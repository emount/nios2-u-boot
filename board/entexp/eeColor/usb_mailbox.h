#ifndef USB_MAILBOX
#define USB_MAILBOX

// #include "xparameters.h"
#include <linux/types.h> 

/* USB Mailbox Register Map */
#define USB_MBOX_BASE              (USB_MAILBOX_0_BASE)

/* Data buffer range */
#define USB_MBOX_DATA              (USB_MBOX_BASE + 0x0000)
#  define USB_MBOX_DATA_BYTES (1024)

/* Register range */
#define USB_MBOX_REGS_BASE         (USB_MBOX_BASE + 0x1000)

/* Control Register 0 */
#define USB_MBOX_CTRL0             (USB_MBOX_REGS_BASE + 0x00)
#define USB_MBOX_DISABLE           (0x00000000)
#define USB_MBOX_ENABLE            (0x00000001)

/* Control Register 1 */
#define USB_MBOX_CTRL1             (USB_MBOX_REGS_BASE + 0x02)
#define USB_MBOX_TXBUF_SEND        (0x8000)
#define USB_MBOX_LENGTH_MASK       (0x07FF)

/* Status Register */
#define USB_MBOX_STAT              (USB_MBOX_REGS_BASE + 0x04)

/* IRQ Flags / Mask Registers */
#define USB_MBOX_IRQ_FLAGS         (USB_MBOX_REGS_BASE + 0x06)
#define USB_MBOX_IRQ_MASK          (USB_MBOX_REGS_BASE + 0x08)
#  define USB_MBOX_NO_IRQS           0x00000000
#  define USB_MBOX_RX_IRQ            0x00000001
#  define USB_MBOX_OVFLW_IRQ         0x00000002
#  define USB_MBOX_TX_IRQ            0x00000004
#  define USB_MBOX_ALL_IRQS          0x00000007

/* Public functions */
extern void SetupUsbMailbox(void);
extern int ReadUsbMailbox(uint8_t *buffer, uint32_t *size);
extern void WriteUsbMailbox(uint8_t *buffer, uint32_t size);

#endif
