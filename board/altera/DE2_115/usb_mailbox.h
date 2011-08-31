#ifndef USB_MAILBOX
#define USB_MAILBOX

// #include "xparameters.h"
#include <linux/types.h> 

/* SPI Mailbox Register Map */
#define USB_MBOX_BASE              (XPAR_BIAMP_USB_MAILBOX_0_BASEADDR)

/* Control Register */
#define USB_MBOX_CTRL              (USB_MBOX_BASE + 0x00)
#define USB_MBOX_DISABLE           (0x00000000)
#define USB_MBOX_ENABLE            (0x00000001)
#define USB_MBOX_MSG_CONSUMED      (0x00000002)         /* Notify host we have consumed a message            */
#define USB_MBOX_MSG_RDY           (0x00000004)         /* Notifies us that a message from the host is ready */

/* IRQ Mask Register */
#define USB_MBOX_IRQ_MASK          (USB_MBOX_BASE + 0x04)

/* IRQ Flags Register */
#define USB_MBOX_FLAGS             (USB_MBOX_BASE + 0x08)
#define USB_MBOX_NO_IRQS           0x00000000
#define USB_MBOX_HOST2SLAVE        0x00000001           /* Notify slave that there is a msg */
#define USB_MBOX_SLAVE2HOST        0x00000002

/* 
 * Mailbox Message Length Register 
 * On Write - How big message to send to host is.
 * On Read  - How bit incoming message is
 */
#define USB_MBOX_MSG_LENGTH        (USB_MBOX_BASE + 0x0C)

/* Mailbox Data Register (Send/Receive) 
 * Send - Write data to this area
 * Rcv  - Read incoming messages from this location
 */
#define USB_MBOX_DATA              (USB_MBOX_BASE + 0x0400)

/* Low-level macros for read and write from / to the mailbox */
#define USB_MBOX_READ_REG(reg) ( *((volatile unsigned long *)reg) )
#define USB_MBOX_WRITE_REG(reg,val) ( *((volatile unsigned long *)reg) = val )

/* Number of dummy bytes returned by the mailbox */
#define MAILBOX_DUMMY_BYTES (4)

/* Public functions */
extern void SetupSPIMbox(void);
extern int ReadSPIMailbox(uint8_t *buffer, uint32_t *size);
extern void WriteSPIMailbox(uint8_t *buffer, uint32_t size);

#endif
