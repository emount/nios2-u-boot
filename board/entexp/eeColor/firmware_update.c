#include "hush.h"
#include "usb_mailbox.h"
#include <linux/types.h>
#include <common.h>
#include <u-boot/crc.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0 
#endif

/* Definitions for the altera_avalon_pio-based pushbutton */
#define BUTTON_GPIO_BASE  (STAT0_REG_BASE)
#  define BUTTON_DATA_REG     (BUTTON_GPIO_BASE + 0x0) /* Data register  */
#  define BUTTON_DIR_REG      (BUTTON_GPIO_BASE + 0x4) /* I/O direction register  */
#  define BUTTON_IRQ_MASK_REG (BUTTON_GPIO_BASE + 0x8) /* IRQ mask register */
#  define BUTTON_EDGE_CAP_REG (BUTTON_GPIO_BASE + 0xc) /* Edge capture register */

#define BUTTON_GPIO_BIT  (0x04)

#define BUTTON_READ_REG(reg) ( *((volatile uint16_t *)reg) )
#define BUTTON_WRITE_REG(reg, val) ( *((volatile uint16_t *)reg) = val )

#define BUTTON_PRESSED ((BUTTON_READ_REG(BUTTON_DATA_REG) & BUTTON_GPIO_BIT) == 0)

/* Data buffer range */
#define USB_MBOX_DATA              (USB_MBOX_BASE + 0x0000)
#  define USB_MBOX_DATA_BYTES (1024)

/* Buffer for storage of USB messages */
static uint8_t messageBuffer[USB_MBOX_DATA_BYTES];

/* Checks to see whether a firmware update is being requested */
void CheckFirmwareUpdate(void)
{
  int readStatus;
  uint32_t messageLength;
  uint16_t regValue;

  printf("Checking for firmware update...\n");

  /* For the moment, test to see if the pushbutton is being held */
  if(BUTTON_PRESSED) {
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
}
