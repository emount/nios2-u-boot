#include "hush.h"
#include "usb_mailbox.h"
#include <linux/types.h>
#include <common.h>
#include <u-boot/crc.h>

/* RSU peripheral include */
#include "rsu.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0 
#endif

/* Definitions for low-level peripheral register access */
#define READ_REG16(reg) ( *((volatile uint16_t *)reg) )
#define WRITE_REG16(reg, val) ( *((volatile uint16_t *)reg) = val )
#define READ_REG32(reg) ( *((volatile uint32_t *)reg) )
#define WRITE_REG32(reg, val) ( *((volatile uint32_t *)reg) = val )

/* Definitions for the altera_avalon_pio-based pushbutton */
#define BUTTON_GPIO_BASE  (STAT0_REG_BASE)
#  define BUTTON_DATA_REG     (BUTTON_GPIO_BASE + 0x0) /* Data register  */
#  define BUTTON_DIR_REG      (BUTTON_GPIO_BASE + 0x4) /* I/O direction register  */
#  define BUTTON_IRQ_MASK_REG (BUTTON_GPIO_BASE + 0x8) /* IRQ mask register */
#  define BUTTON_EDGE_CAP_REG (BUTTON_GPIO_BASE + 0xc) /* Edge capture register */

#define BUTTON_GPIO_BIT  (0x04)

#define BUTTON_PRESSED ((READ_REG32(BUTTON_DATA_REG) & BUTTON_GPIO_BIT) == 0)

/* Constants for the system ID peripheral */
#define SYSID_BASE  (CONFIG_SYS_SYSID_BASE)
#  define SYSID_ID_REG         (SYSID_BASE + 0x0)  /* ID register */
#  define SYSID_TIMESTAMP_REG  (SYSID_BASE + 0x4)  /* Timestamp register */
#define SYSID_ID_VALUE   READ_REG32(SYSID_ID_REG)
#define SYSID_TIMESTAMP  READ_REG32(SYSID_TIMESTAMP_REG)

/* Data buffer range */
#define USB_MBOX_DATA              (USB_MBOX_BASE + 0x0000)
#  define USB_MBOX_DATA_BYTES (1024)

/* Buffer for storage of USB messages */
static uint8_t messageBuffer[USB_MBOX_DATA_BYTES];

/* Buffer for constructing command strings */
#define CMD_FORMAT_BUF_SZ (256)
static char commandBuffer[CMD_FORMAT_BUF_SZ];

/* System ID signature of the test / bootloader FPGA */
#define BOOT_FPGA_ID (0x01234567)

/* Size of each of the various firmware images */
#define CRC_RECORDS_SIZE  (0x00020000)
#define FPGA_IMAGE_SIZE   (0x00140000)

/* Constant definitions identifying each of the Flash partitions */
#define CRC_RECORDS_ADDR   (CONFIG_SYS_FLASH_BASE)
#define GOLDEN_FPGA_ADDR   (CRC_RECORDS_ADDR + CRC_RECORDS_SIZE)
#define RUNTIME_FPGA_ADDR  (GOLDEN_FPGA_ADDR + FPGA_IMAGE_SIZE)
#define UBOOT_IMAGE_ADDR   (RUNTIME_FPGA_ADDR + FPGA_IMAGE_SIZE)

/*
# 0xCA000000 - 0xCA020000 (  128 KiB / 0x00020000) - Low padding
# 0xCA020000 - 0xCA15FFFF (1,280 KiB / 0x00140000) - "Golden" FPGA image
# 0xCA160000 - 0xCA29FFFF (1,280 KiB / 0x00140000) - Run-time FPGA image
# 0xCA2A0000 - 0xCA2DFFFF (  256 KiB / 0x00040000) - U-Boot
# 0xCA2E0000 - 0xCA45FFFF (1,536 KiB / 0x00180000) - Linux Kernel A
# 0xCA460000 - 0xCA5DFFFF (1,536 KiB / 0x00180000) - Linux Kernel B
# 0xCA5E0000 - 0xCAADFFFF (5,120 KiB / 0x00500000) - Root Filesystem A (SquashFS, only one for now)
# 0xCAAE0000 - 0xCAFF7FFF (5,216 KiB / 0x00518000) - Padding (for now)
# 0xCAFF8000 - 0xCAFFFFFF (   32 KiB / 0x00008000) - U-Boot environment
*/

/* Constant definitions related to the remote system update peripheral */
#define REMOTE_UPDATE_CONTROLLER_BASE  REMOTE_UPDATE_CYCLONEIII_0_BASE

#define RSU_CONFIG_OFFSET_ADDR (0x04)
#define RSU_CTRL_STAT_ADDR     (0x20)
#  define RSU_CTRL_RESET_WDOG (0x00000002)
#  define RSU_CTRL_RECONFIG   (0x00000001)

/* The highest 12 bits of the 27-bit watchdog cycle counter (~10 MHz) */
#define WATCHDOG_TIMEOUT_NONE (0x0000)
#define WATCHDOG_TIMEOUT_MAX  (0x0FFF)

/* The RSU expects only the 22 highest-order bits of a 24-bit offset;
 * beyond that we shift an additional bit to compensate for the 16-bit
 * data bus width of the CFI Flash device.
 */
#define RSU_ADDRESS_SHIFT  (3)

/* Checks to see whether a firmware update is being requested */
void CheckFirmwareUpdate(void)
{
  int readStatus;
  uint32_t messageLength;
  uint32_t timestamp;
  uint32_t runtimeImage;
  char *kernelStart = NULL;
  char *rootfsPartition = NULL;

  /* Print values from the status GPIO peripheral */
  timestamp = READ_REG32(REVISION_REG_BASE + 0x00);
  printf("FPGA Build Date: %02X-%02X-20%02X, %02X:00\n",
         ((timestamp >> 16) & 0x0FF),
         ((timestamp >>  8) & 0x0FF),
         ((timestamp >> 24) & 0x0FF),
         (timestamp & 0x0FF));

  /* Determine whether this is the bootloader or the run-time FPGA */
  runtimeImage = (SYSID_ID_VALUE != BOOT_FPGA_ID);
  printf("System ID: 0x%08X (%s FPGA)\n", 
         SYSID_ID_VALUE,
         (runtimeImage ? "run-time" : "bootloader"));

  /* Perform run-time or bootloader processing */
  if(runtimeImage) {
    /* Select the run-time Linux image for boot */
    printf("Booting run-time Linux kernel 'A'...\n");
    kernelStart     = "kernel_a_start";
    rootfsPartition = "mtdblock8";
  } else {
    printf("Checking for test mode...\n");

    if(BUTTON_PRESSED) {
      /* Select the run-time Linux image for boot */
      printf("<<< TEST MODE >>>\n");
      printf("Booting golden Linux kernel...\n");
      kernelStart     = "golden_kernel_start";
      rootfsPartition = "mtdblock7";
    } else {
      /* Determine which run-time FPGA image to reconfigure into */
      printf("Reconfiguring to run-time FPGA\n");

      /* Use the Remote System Update (RSU) peripheral to reconfigure.
       * TODO - Need to dynamically select a runtime image
       */
      rsu_factory_trigger_reconfig(REMOTE_UPDATE_CONTROLLER_BASE,
                                   (((RUNTIME_FPGA_ADDR - CONFIG_SYS_FLASH_BASE)) >> RSU_ADDRESS_SHIFT), // MSB 22-bits of 24-bit address
                                   WATCHDOG_TIMEOUT_NONE // MSB 12-bits of 29-bit timeout count
                                   );

      /* Should never reach here */
      while(1);
    }
  }

  /* Alter the boot string for the kernel selection */
  sprintf(commandBuffer, "set bootcmd 'bootm ${%s}'", kernelStart);
  commandBuffer[CMD_FORMAT_BUF_SZ - 1] = '\0';

  if(parse_string_outer(commandBuffer,
                        (FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP)) != 0) {
    printf("Set of bootcmd \"%s\" failed\n", commandBuffer);
  }

  /* Alter the kernel arguments accordingly */
  sprintf(commandBuffer, 
          "set bootargs 'console=ttyAL0,115200 root=/dev/%s rootfstype=squashfs init=/bin/init'",
          rootfsPartition);
  commandBuffer[CMD_FORMAT_BUF_SZ - 1] = '\0';

  if(parse_string_outer(commandBuffer,
                        (FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP)) != 0) {
    printf("Set of bootargs \"%s\" failed\n", commandBuffer);
  }

#if 0

  /* For the moment, test to see if the pushbutton is being held */
  if(0) { // BUTTON_PRESSED) {
    printf("Entering firmware update mode\n");

    /* Set up the USB mailbox, allowing the device to enumerate */
    SetupUsbMailbox();

    /* Loop, polling for and responding to messages */
    while(1) {
      printf("Polling for a message...\n");

      /* Specify the size of the buffer and attempt to read */
      messageLength = USB_MBOX_DATA_BYTES;
      readStatus = ReadUsbMailbox(messageBuffer, &messageLength);
      if(readStatus) {
        printf("Got %d-byte message\n", messageLength);
      } else {
        printf("Message read failed\n");
      }
    }
  } /* if(button press) */
#endif
}
