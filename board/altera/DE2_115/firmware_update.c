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

/* Buffer for storage of USB messages */
static uint8_t messageBuffer[USB_MBOX_DATA_BYTES];

/* Checks to see whether a firmware update is being requested */
void CheckFirmwareUpdate(void)
{
  int readStatus;
  uint32_t messageLength;

  printf("Checking for firmware update...\n");

  /* For the moment, unconditionally perform an update */

  /* Set up the USB mailbox, allowing the device to enumerate */
  SetupUsbMailbox();

  /* Still working to get messages into the peripheral... */
#if 0
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
#endif
}
