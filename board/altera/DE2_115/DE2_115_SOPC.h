#ifndef _DE2_115_SOPC_H_
#define _DE2_115_SOPC_H_

/* generated from DE2_115_SOPC.sopcinfo */

/* cpu.data_master is a altera_nios2 */
#define CONFIG_SYS_CLK_FREQ 0
#define CONFIG_SYS_RESET_ADDR 0x00000000
#define CONFIG_SYS_EXCEPTION_ADDR 0x00000000
#define CONFIG_SYS_ICACHE_SIZE 0
#define CONFIG_SYS_ICACHELINE_SIZE 32
#define CONFIG_SYS_DCACHE_SIZE 0
#define CONFIG_SYS_DCACHELINE_SIZE 0
#define IO_REGION_BASE 0x80000000

/* usb.dc is a ISP1362_IF */
#define USB_BASE 0x80000000

/* i2c_scl.s1 is a altera_avalon_pio */
#define I2C_SCL_BASE 0x80000000

/* sd_dat.s1 is a altera_avalon_pio */
#define SD_DAT_BASE 0x80000000

/* sma_in.s1 is a altera_avalon_pio */
#define SMA_IN_BASE 0x80000000

/* sd_cmd.s1 is a altera_avalon_pio */
#define SD_CMD_BASE 0x80000000

/* descriptor_memory.s1 is a altera_avalon_onchip_memory2 */
#define DESCRIPTOR_MEMORY_BASE 0x80000000

/* sd_wp_n.s1 is a altera_avalon_pio */
#define SD_WP_N_BASE 0x80000000

/* seg7.avalon_slave is a SEG7_IF */
#define SEG7_BASE 0x80000000

/* cfi_flash.s1 is a altera_avalon_cfi_flash */
#define CONFIG_SYS_FLASH_BASE 0x80000000
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_CFI_FLASH_STATUS_POLL /* fix amd flash issue */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MAX_FLASH_BANKS 1
#define CONFIG_SYS_MAX_FLASH_SECT 1024

/* jtag_uart.avalon_jtag_slave is a altera_avalon_jtag_uart */
#define CONFIG_SYS_JTAG_UART_BASE 0x80000000

/* timer.s1 is a altera_avalon_timer */
#define CONFIG_SYS_TIMER_BASE 0x80000000
#define CONFIG_SYS_TIMER_IRQ 0
#define CONFIG_SYS_TIMER_FREQ 0

/* sdram.s1 is a altera_avalon_new_sdram_controller */
#define CONFIG_SYS_SDRAM_BASE 0x00000000
#define CONFIG_SYS_SDRAM_SIZE 0x00000000

/* pll.pll_slave is a altpll */
#define PLL_BASE 0x80000000

/* rs232.s1 is a altera_avalon_uart */
#define CONFIG_SYS_UART_BASE 0x80000000
#define CONFIG_SYS_UART_FREQ 0
#define CONFIG_SYS_UART_BAUD 0

/* key.s1 is a altera_avalon_pio */
#define KEY_BASE 0x80000000

/* eep_i2c_scl.s1 is a altera_avalon_pio */
#define EEP_I2C_SCL_BASE 0x80000000

/* lcd.control_slave is a altera_avalon_lcd_16207 */
#define LCD_BASE 0x80000000

/* usb.hc is a ISP1362_IF */
#define USB_BASE 0x80000000

/* sgdma_tx.csr is a altera_avalon_sgdma */
#define CONFIG_SYS_ALTERA_TSE_SGDMA_TX_BASE 0x80000000

/* eep_i2c_sda.s1 is a altera_avalon_pio */
#define EEP_I2C_SDA_BASE 0x80000000

/* sd_clk.s1 is a altera_avalon_pio */
#define SD_CLK_BASE 0x80000000

/* ISP1362_IF_0.dc is a ISP1362_IF */
#define ISP1362_IF_0_BASE 0x80000000

/* ir.s1 is a altera_avalon_pio */
#define IR_BASE 0x80000000

/* tse_mac.control_port is a triple_speed_ethernet */
#define CONFIG_SYS_ALTERA_TSE_MAC_BASE 0x80000000
#define CONFIG_SYS_ALTERA_TSE_RX_FIFO 2048
#define CONFIG_SYS_ALTERA_TSE_TX_FIFO 2048
#define CONFIG_ALTERA_TSE
#define CONFIG_MII
#define CONFIG_CMD_MII
#define CONFIG_SYS_ALTERA_TSE_PHY_ADDR 18
#define CONFIG_SYS_ALTERA_TSE_FLAGS 0

/* ledr.s1 is a altera_avalon_pio */
#define LEDR_BASE 0x80000000

/* sma_out.s1 is a altera_avalon_pio */
#define SMA_OUT_BASE 0x80000000

/* sgdma_rx.csr is a altera_avalon_sgdma */
#define CONFIG_SYS_ALTERA_TSE_SGDMA_RX_BASE 0x80000000

/* sysid.control_slave is a altera_avalon_sysid */
#define CONFIG_SYS_SYSID_BASE 0x80000000

/* ISP1362_IF_0.hc is a ISP1362_IF */
#define ISP1362_IF_0_BASE 0x80000000

/* i2c_sda.s1 is a altera_avalon_pio */
#define I2C_SDA_BASE 0x80000000

/* sram.avalon_slave is a TERASIC_SRAM */
#define SRAM_BASE 0x80000000

/* ledg.s1 is a altera_avalon_pio */
#define LEDG_BASE 0x80000000

/* sw.s1 is a altera_avalon_pio */
#define SW_BASE 0x80000000

/* audio.avalon_slave is a AUDIO_IF */
#define AUDIO_BASE 0x80000000

#endif /* _DE2_115_SOPC_H_ */
