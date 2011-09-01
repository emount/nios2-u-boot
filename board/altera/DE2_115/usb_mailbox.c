#include <common.h>
#include "usb_mailbox.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/* Low-level macros for read and write from / to the mailbox */
#define USB_MBOX_READ_REG(reg) ( *((volatile uint16_t *)reg) )
#define USB_MBOX_WRITE_REG(reg, val) ( *((volatile uint16_t *)reg) = val )

uint8_t bPollingUSBMbox=FALSE;
uint8_t bWaitingForUSBMsg=FALSE;

/* Number of microseconds to hold the USB transceiver in reset */
#define USB_RESET_USECS (10000)

/* Static buffer to hold packet data in */
static uint8_t packetBuffer[USB_MBOX_DATA_BYTES];

void SetupUsbMailbox(void) {
  volatile uint16_t *dataPtr;
  uint32_t byteIndex;
  uint32_t errorCount;
    
  printf("Initializing USB Mailbox\n");

  /* Ensure the USB chip receives a solid minimum reset */
  USB_MBOX_WRITE_REG(USB_MBOX_CTRL0, USB_MBOX_DISABLE);
  udelay(USB_RESET_USECS);
  USB_MBOX_WRITE_REG(USB_MBOX_CTRL0, USB_MBOX_ENABLE);

  /* Perform a quick test of the mailbox data RAM */
  dataPtr = (volatile uint16_t *) USB_MBOX_DATA;
  for(byteIndex = 0; byteIndex < USB_MBOX_DATA_BYTES; byteIndex++) {
    *dataPtr++ = (uint16_t) ~byteIndex;
  }
  
  errorCount = 0;
  dataPtr = (volatile uint16_t *) USB_MBOX_DATA;
  for(byteIndex = 0; byteIndex < USB_MBOX_DATA_BYTES; byteIndex++) {
    if(*dataPtr++ != (~byteIndex & 0x0FF)) errorCount++;
  }

  printf("USB mailbox tested with %d errors\n", errorCount);

  printf("USB Mailbox initialized\n");
}

/**
 * Reads a message out of the USB Mailbox. Controlled by
 * global variable bPollingUSBMbox which can be used to
 * force return
 *
 * Paramaters:
 *       buffer - Buffer to read data into
 *       size   - [IN]  - Size of input buffer
 *              - [OUT] - Number of bytes read
 *
 * Returns:
 *       TRUE  - Message was read
 *       FALSE - No message was read
 */
int ReadUsbMailbox(uint8_t *buffer, uint32_t *size) {
#if 0
  int bStatus = FALSE;
  int idx;

  bWaitingForUSBMsg = TRUE;
  while(bWaitingForUSBMsg)
  {
    /* Use the IRQ flags register to determine when a message has been received,
     * despite the fact that we don't actually use interrupts.  Write the flags
     * back each time to clear any pending events prior to servicing.
     */
    uint32_t flags_reg = USB_MBOX_READ_REG(USB_MBOX_FLAGS);
    USB_MBOX_WRITE_REG(USB_MBOX_FLAGS, flags_reg);
    if(flags_reg & USB_MBOX_HOST2SLAVE)
    {
      /* A message has been received from the host, obtain its length */
      uint32_t bufLen = USB_MBOX_READ_REG(USB_MBOX_MSG_LENGTH);
      if (*size < bufLen) {
        /* We received a message longer than the requested message size; bail
         * out and return FALSE as an indication
         */
        break;
      }

      /* Received a message, inform the caller of its length */
      *size = bufLen;

      /* Retrieve and buffer the message words for the client */
      for (idx = 0; idx < ((*size + 3) / 4); idx++) {
        ((uint32_t *) buffer)[idx] = ((uint32_t *) USB_MBOX_DATA)[idx];
      }

      USB_MBOX_WRITE_REG(USB_MBOX_CTRL, (USB_MBOX_MSG_CONSUMED | USB_MBOX_ENABLE));
      bStatus = TRUE;
      bWaitingForUSBMsg = FALSE;
    }
  }
  return bStatus;
#endif
  return(0);
}

/**
 * Writes a message out to the USB mailbox
 *
 * buffer - Pointer to data buffer to copy message from
 * size   - size of buffer to write
 */
void WriteUsbMailbox(uint8_t *buffer, uint32_t size) {
#if 0
  int idx;

  /* Write the response words into the data buffer */
  for(idx = 0; idx < (size + 3) / 4; idx++) {
    ((uint32_t *) USB_MBOX_DATA)[idx] = ((uint32_t *) buffer)[idx];
  }

  /* Commit the response message to the host */
  USB_MBOX_WRITE_REG(USB_MBOX_MSG_LENGTH, size);
#endif
}
