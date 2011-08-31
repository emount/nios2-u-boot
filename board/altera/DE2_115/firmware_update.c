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

#define MAX_CMD_LENGTH 256

/* Checks to see whether a firmware update is being requested */
void CheckFirmwareUpdate(void)
{
  printf("Checking for firmware update...\n");

  /* For the moment, unconditionally perform an update */
  SetupUsbMbox();
}
