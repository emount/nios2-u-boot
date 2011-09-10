/*
 * (C) Copyright 2005, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * BOARD/CPU
 */
#include "../board/entexp/eeColor/eeColor.h"
#define CONFIG_BOARD_NAME "eeColor"
#define CONFIG_BOARD_EARLY_INIT_F	/* enable early board-spec. init */
#define CONFIG_SYS_NIOS_SYSID_BASE	CONFIG_SYS_SYSID_BASE

/* Perform the normal bootdelay checking */
#define CONFIG_BOOTDELAY 1

/* Perform a test for firmware update once initialized */
#define CONFIG_FIRMWARE_UPDATE

/*
 * SERIAL
 *
 * NOTE - Using the actual RS-232 serial port; uncomment the following line
 *        if it is ever desirable to use the JTAG UART.  The JTAG UART will,
 *        however, hang the boot process unless a ByteBlaster is actually
 *        connected and nios2-terminal running!
 * #define CONFIG_ALTERA_JTAG_UART
 */
#if defined(CONFIG_ALTERA_JTAG_UART)
# define CONFIG_SYS_NIOS_CONSOLE	CONFIG_SYS_JTAG_UART_BASE
#else
# define CONFIG_ALTERA_UART
# define CONFIG_SYS_NIOS_CONSOLE	CONFIG_SYS_UART_BASE
#endif

#define CONFIG_ALTERA_JTAG_UART_BYPASS
#define CONFIG_SYS_NIOS_FIXEDBAUD
#define CONFIG_BAUDRATE		CONFIG_SYS_UART_BAUD
#define CONFIG_SYS_BAUDRATE_TABLE	{CONFIG_BAUDRATE}
#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* Suppress console info */

/*
 * TIMER
 */
#define CONFIG_SYS_NIOS_TMRBASE	CONFIG_SYS_TIMER_BASE
#define CONFIG_SYS_NIOS_TMRIRQ		CONFIG_SYS_TIMER_IRQ
#define CONFIG_SYS_HZ			1000	/* Always 1000 */
#define CONFIG_SYS_NIOS_TMRMS		10	/* Desired period (msec)*/
#define CONFIG_SYS_NIOS_TMRCNT \
	(CONFIG_SYS_NIOS_TMRMS * (CONFIG_SYS_TIMER_FREQ / 1000) - 1)

/*
 * STATUS LED
 */
/* Revisit this once basic functionality is working */
#if 0
#define CONFIG_STATUS_LED		/* Enable status driver */
#define CONFIG_GPIO_LED		/* Enable gpioled driver */

#define STATUS_LED_BIT			2	/* Bit-2 on GPIO */
#define STATUS_LED_STATE		1	/* Blinking */
#define STATUS_LED_PERIOD	(500 / CONFIG_SYS_NIOS_TMRMS) /* 500 msec */
#endif

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BOOTD
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_ITEST
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_XIMG

#ifdef CONFIG_CMD_NET
# define CONFIG_NET_MULTI
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_PING
#endif

/* Define the width of the Flash for the CFI driver */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

/*
 * ENVIRONMENT -- Put environment in the very first bottom-boot sector
 *                of the Spansion S29GL064 device.  This part has eight
 *                8-KiB bottom boot block sectors, followed by 127 64-KiB
 *                sectors.
 */
#define CONFIG_ENV_IS_IN_FLASH

#define CONFIG_ENV_SIZE		0x8000	/* 32k, 1 top-boot sector */
#define CONFIG_ENV_OVERWRITE		/* Serial change Ok	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00FF8000)

/*
 * MEMORY ORGANIZATION
 *	-Monitor at top of sdram.
 *	-The heap is placed below the monitor
 *	-Global data is placed below the heap.
 *	-The stack is placed below global data (&grows down).
 */
#define CONFIG_MONITOR_IS_IN_RAM
#define CONFIG_SYS_MONITOR_LEN		0x80000	/* Reserve 512k */
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_SDRAM_SIZE - \
					 CONFIG_SYS_MONITOR_LEN)
#define CONFIG_SYS_GBL_DATA_SIZE	256	/* Global data size rsvd */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 0x20000)
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_MONITOR_BASE - \
					 CONFIG_SYS_MALLOC_LEN)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_MALLOC_BASE - \
					 CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP		CONFIG_SYS_GBL_DATA_OFFSET

/*
 * MISC
 */
#define CONFIG_SYS_LONGHELP		/* Provide extended help */
#define CONFIG_SYS_PROMPT		"==> "	/* Command prompt	*/
#define CONFIG_SYS_CBSIZE		256	/* Console I/O buf size */
#define CONFIG_SYS_MAXARGS		16	/* Max command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Bootarg buf size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + \
					 16)	/* Print buf size */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_INIT_SP - 0x20000)
#define CONFIG_CMDLINE_EDITING

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#endif /* __CONFIG_H */
