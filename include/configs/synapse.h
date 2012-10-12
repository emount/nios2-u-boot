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
#include "../board/axon/synapse/synapse.h"
#define CONFIG_BOARD_NAME "Axon - Synapse - Stratix-IV GX"
#define CONFIG_BOARD_EARLY_INIT_F	/* enable early board-spec. init */
#define CONFIG_SYS_NIOS_SYSID_BASE	CONFIG_SYS_SYSID_BASE

/* Perform the normal bootdelay checking */
#define CONFIG_BOOTDELAY 1

/* Perform a test for firmware update once initialized */
/* #define CONFIG_FIRMWARE_UPDATE */

/*
 * SERIAL
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
 *
 * CONFIG_SYS_HZ sets the time granularity of the system timestamp;
 * keep this set to 1000 to use msec.  The CONFIG_SYS_NIOS_TMRMS is
 * then used to configure the period at which the interval timer
 * actually interrupts the processor.  The value of one millisecond
 * is appropriate to ensure that brief timeouts, such as write timeouts
 * for Flash memory, are checked with sufficient precision.
 */
#define CONFIG_SYS_NIOS_TMRBASE	CONFIG_SYS_TIMER_BASE
#define CONFIG_SYS_NIOS_TMRIRQ		CONFIG_SYS_TIMER_IRQ
#define CONFIG_SYS_HZ               1000  /* Always 1000 */
#define CONFIG_SYS_NIOS_TMRMS		1     /* Desired period (msec)*/
#define CONFIG_SYS_NIOS_TMRCNT \
	(CONFIG_SYS_NIOS_TMRMS * (CONFIG_SYS_TIMER_FREQ / 1000) - 1)

/* Lab X Ethernet controller 
 *
 * MDIO divisor is set to produce (100 MHz / 100) = 1 MHz
 */
#define CONFIG_SYS_ENET            1
#define CONFIG_LABX_ETHERNET       1
#define LABX_ETHERNET_MDIO_DIV     100
#define LABX_PRIMARY_ETH_BASEADDR  LABX_ETHERNET_0_BASE
#define LABX_MDIO_ETH_BASEADDR     LABX_ETHERNET_0_BASE
#define LABX_ETH_ALTERA_SGMII      1
#define LABX_ETHERNET_PHY_ADDR     0x00
#define WHICH_ETH_PORT             0

/* Default MAC, IP address, and server IP */
#define CONFIG_ETHADDR   00:03:41:00:50:F0
#define CONFIG_IPADDR    192.168.1.1
#define CONFIG_SERVERIP  192.168.1.100

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

/* Flash driver options */
#define CONFIG_FLASH_SHOW_PROGRESS 1

/*
 * ENVIRONMENT -- Put environment in the very first bottom-boot sector
 *                of the Numonyx PC28F512P30BF device.  This part has four
 *                32-KiB bottom boot block sectors, followed by 511 128-KiB
 *                sectors.
 */
#define CONFIG_ENV_IS_IN_FLASH 1
#define CONFIG_ENV_SECT_SIZE   0x20000
#define CONFIG_ENV_SIZE		   0x20000  /* 128k, 1 sector */
#define CONFIG_ENV_OVERWRITE           /* Serial change Ok	*/
#define CONFIG_ENV_ADDR		   (CONFIG_SYS_FLASH_BASE + 0x01F00000)

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
