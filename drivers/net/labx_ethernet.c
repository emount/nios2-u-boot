/*
 *
 * Lab X Ethernet driver for u-boot
 * Adapted from the Xilinx TEMAC driver
 *
 * The Lab X Ethernet peripheral implements a FIFO-based interface which
 * largely, directly, mimics the xps_ll_fifo peripheral but without the
 * unnecessary LocalLink adaptation layer.  A future enhancement may add
 * an optional integrated SGDMA controller.
 *
 * Authors: Yoshio Kashiwagi kashiwagi@co-nss.co.jp
 *          Eldridge M. Mount IV (eldridge.mount@labxtechnologies.com)
 *
 * Copyright (C) 2008 Michal Simek <monstr@monstr.eu>
 * June 2008 Microblaze optimalization, FIFO mode support
 *
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * Copyright (C) 2010 Lab X Technologies, LLC
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <malloc.h>
#include <asm/processor.h>

#ifndef LABX_PRIMARY_ETH_BASEADDR
#  error "Missing board definition for Ethernet peripheral base address"
#endif

/* Ensure that there is a definition for the PHY address and MDIO divisor */
#ifndef LABX_ETHERNET_PHY_ADDR
#  error "Missing board definition of MDIO PHY address"
#endif
#ifndef LABX_ETHERNET_MDIO_DIV
#  error "Missing board definition of MDIO clock divisor"
#endif

/* 
 * Lab X Tri-mode MAC register definitions 
 * The first half of the address space maps to registers used for
 * PHY control and interrupts.  The second half is passed through to the
 * actual MAC registers.
 */
#define MDIO_CONTROL_REG      (0x00000000)
#  define PHY_MDIO_BUSY       (0x80000000)
#  define PHY_REG_ADDR_MASK   (0x01F)
#  define PHY_ADDR_MASK       (0x01F)
#  define PHY_ADDR_SHIFT      (5)
#  define PHY_MDIO_READ       (0x0400)
#  define PHY_MDIO_WRITE      (0x0000)
#define MDIO_DATA_REG         (0x00000004)
#  define PHY_DATA_MASK       (0x0000FFFF)
#define INT_MASK_REG          (0x00000008)
#  define PHY_IRQ_FALLING     (0x00000000)
#  define PHY_IRQ_RISING      (0x80000000)
#define INT_FLAGS_REG         (0x0000000C)
#  define MDIO_IRQ_MASK       (0x00000001)
#  define PHY_IRQ_MASK        (0x00000002)
#define VLAN_MASK_REG         (0x00000010)
#define FILTER_SELECT_REG     (0x00000014)
#define FILTER_CONTROL_REG    (0x00000018)
#  define MATCH_ARCH_ID_SHIFT     24
#    define MATCH_ARCH_XILINX_SRL16E (0x10)
#    define MATCH_ARCH_DIRECT        (0x20)
#  define MAC_ADDRESS_LOAD_ACTIVE 0x00000100
#  define MAC_ADDRESS_LOAD_LAST   0x00000200
#define FILTER_LOAD_REG       (0x0000001C)
#define REVISION_REG          (0x0000003C)
#  define REVISION_MINOR_MASK  0x0000000F
#  define REVISION_MINOR_SHIFT 0
#  define REVISION_MAJOR_MASK  0x000000F0
#  define REVISION_MAJOR_SHIFT 4
#  define REVISION_MATCH_MASK  0x0000FF00
#  define REVISION_MATCH_SHIFT 8

/* Base address for registers internal to the MAC */
#define LABX_MAC_REGS_BASE    (0x00001000)

/* Base address for the Tx & Rx FIFOs */
#define LABX_FIFO_REGS_BASE   (0x00002000)

/* Note - Many of the Rx and Tx config register bits are read-only, and simply
 *        represent the fixed behavior of the simple MAC.  In the future, these
 *        functions may be fleshed out in the hardware.
 */
#ifdef XILINX_HARD_MAC
#define MAC_RX_CONFIG_REG     (LABX_MAC_REGS_BASE + 0x0040)
#else
#define MAC_RX_CONFIG_REG     (LABX_MAC_REGS_BASE + 0x0004)
#endif
#  define RX_SOFT_RESET          (0x80000000)
#  define RX_JUMBO_FRAME_ENABLE  (0x40000000)
#  define RX_IN_BAND_FCS_ENABLE  (0x20000000)
#  define RX_DISABLE             (0x00000000)
#  define RX_ENABLE              (0x10000000)
#  define RX_VLAN_TAGS_ENABLE    (0x08000000)
#  define RX_HALF_DUPLEX_MODE    (0x04000000)
#  define RX_DISABLE_LTF_CHECK   (0x02000000)
#  define RX_DISABLE_CTRL_CHECK  (0x01000000)

#ifdef XILINX_HARD_MAC
#define MAC_TX_CONFIG_REG     (LABX_MAC_REGS_BASE + 0x0080)
#else
#define MAC_TX_CONFIG_REG     (LABX_MAC_REGS_BASE + 0x0008)
#endif
#  define TX_SOFT_RESET          (0x80000000)
#  define TX_JUMBO_FRAME_ENABLE  (0x40000000)
#  define TX_IN_BAND_FCS_ENABLE  (0x20000000)
#  define TX_DISABLE             (0x00000000)
#  define TX_ENABLE              (0x10000000)
#  define TX_VLAN_TAGS_ENABLE    (0x08000000)
#  define TX_HALF_DUPLEX_MODE    (0x04000000)
#  define TX_IFG_ADJUST_ENABLE   (0x02000000)

#ifdef XILINX_HARD_MAC
#define MAC_SPEED_SELECT_REG  (LABX_MAC_REGS_BASE + 0x0100)
#else
#define MAC_SPEED_SELECT_REG  (LABX_MAC_REGS_BASE + 0x0010)
#endif
#  define MAC_SPEED_1_GBPS    (0x80000000)
#  define MAC_SPEED_100_MBPS  (0x40000000)
#  define MAC_SPEED_10_MBPS   (0x00000000)

#ifdef XILINX_HARD_MAC
#define MAC_MDIO_CONFIG_REG   (LABX_MAC_REGS_BASE + 0x0140)
#else
#define MAC_MDIO_CONFIG_REG   (LABX_MAC_REGS_BASE + 0x0014)
#endif
#  define MDIO_DIVISOR_MASK  (0x0000003F)
#  define MDIO_ENABLED       (0x00000040)

/* SGMII PCS registers, mapped into the MAC register set for SGMII shims */
#define LABX_SGMII_REGS_BASE  (LABX_MAC_REGS_BASE + 0x0100)
#define SGMII_CTRL_REG             (LABX_SGMII_REGS_BASE + 0x00)
#define SGMII_STATUS_REG           (LABX_SGMII_REGS_BASE + 0x04)
#define SGMII_PHY_ID_HIGH_REG      (LABX_SGMII_REGS_BASE + 0x08)
#define SGMII_PHY_ID_LOW_REG       (LABX_SGMII_REGS_BASE + 0x0C)
#define SGMII_DEV_ABILITY_REG      (LABX_SGMII_REGS_BASE + 0x10)
#define SGMII_PARTNER_ABILITY_REG  (LABX_SGMII_REGS_BASE + 0x14)
#define SGMII_AN_EXPANSION_REG     (LABX_SGMII_REGS_BASE + 0x18)
#define SGMII_SCRATCH_REG          (LABX_SGMII_REGS_BASE + 0x40)
#define SGMII_IF_MODE_REG          (LABX_SGMII_REGS_BASE + 0x50)

/* Maximum Ethernet MTU (with VLAN tag extension) */
#define ETHER_MTU		1520

/* PHY register definitions */
#define MII_BMCR            0x00        /* Basic mode control register */
#define MII_ADVERTISE       0x04
#define MII_EXADVERTISE 	0x09

/* Basic mode control register. */
#define BMCR_RESV               0x003f  /* Unused...                   */
#define BMCR_SPEED1000          0x0040  /* MSB of Speed (1000)         */
#define BMCR_CTST               0x0080  /* Collision test              */
#define BMCR_FULLDPLX           0x0100  /* Full duplex                 */
#define BMCR_ANRESTART          0x0200  /* Auto negotiation restart    */
#define BMCR_ISOLATE            0x0400  /* Disconnect DP83840 from MII */
#define BMCR_PDOWN              0x0800  /* Powerdown the DP83840       */
#define BMCR_ANENABLE           0x1000  /* Enable auto negotiation     */
#define BMCR_SPEED100           0x2000  /* Select 100Mbps              */
#define BMCR_LOOPBACK           0x4000  /* TXD loopback bits           */
#define BMCR_RESET              0x8000  /* Reset the DP83840           */

/* Advertisement control register. */
#define ADVERTISE_SLCT          0x001f  /* Selector bits               */
#define ADVERTISE_CSMA          0x0001  /* Only selector supported     */
#define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL     0x0020  /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF     0x0040  /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE    0x0080  /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM 0x0100  /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4      0x0200  /* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP     0x0400  /* Try for pause               */
#define ADVERTISE_PAUSE_ASYM    0x0800  /* Try for asymetric pause     */
#define ADVERTISE_RESV          0x1000  /* Unused...                   */
#define ADVERTISE_RFAULT        0x2000  /* Say we can detect faults    */
#define ADVERTISE_LPACK         0x4000  /* Ack link partners response  */
#define ADVERTISE_NPAGE         0x8000  /* Next page bit               */

/* 1000BASE-T Control register */
#define ADVERTISE_1000FULL      0x0200  /* Advertise 1000BASE-T full duplex */
#define ADVERTISE_1000HALF      0x0100  /* Advertise 1000BASE-T half duplex */

#define ADVERTISE_FULL (ADVERTISE_100FULL | ADVERTISE_10FULL |	\
                        ADVERTISE_CSMA)
#define ADVERTISE_ALL (ADVERTISE_10HALF | ADVERTISE_10FULL |	\
                       ADVERTISE_100HALF | ADVERTISE_100FULL)

/* Special stuff for setting up a BCM5481.  Note that LS nibble of low ID
 * is revision number, and will vary.
 */
#define BCM5481_ID_HIGH 0x0143
#define BCM5481_ID_LOW 0xBCA0
#define BCM5481_ID_LOW_MASK 0xFFF0
#define BCM5481_RX_SKEW_REGISTER_SEL         0x7007
#define BCM5481_RX_SKEW_ENABLE               0x0100
#define BCM5481_CLOCK_ALIGNMENT_REGISTER_SEL 0x0C00
#define BCM5481_SHADOW_WRITE                 0x8000
#define BCM5481_XMIT_CLOCK_DELAY             0x0200
#define BCM5481_AUTO_NEGOTIATE_ENABLE        0x1000
#define BCM5481_HIGH_PERFORMANCE_ENABLE      0x0040
#define BCM5481_HPE_REGISTER_SELECT          0x0002

/* Structure emulating the register file of the Cpu_Ethernet peripheral */
typedef struct ll_fifo_s {
  int isr;  /* Interrupt Status Register 0x0 */
  int ier;  /* Interrupt Enable Register 0x4 */
  int tdfr; /* Transmit data FIFO reset 0x8 */
  int tdfv; /* Transmit data FIFO Vacancy 0xC */
  int tdfd; /* Transmit data FIFO 32bit wide data write port 0x10 */
  int tlf;  /* Write Transmit Length FIFO 0x14 */
  int rdfr; /* Read Receive data FIFO reset 0x18 */
  int rdfo; /* Receive data FIFO Occupancy 0x1C */
  int rdfd; /* Read Receive data FIFO 32bit wide data read port 0x20 */
  int rlf;  /* Read Receive Length FIFO 0x24 */
  int fctl; /* FIFO control 0x28 */
} ll_fifo_s;

/* Masks, etc. for use with the register file */
#define RLF_MASK 0x000007FF

/* Interrupt status register mnemonics */
#define FIFO_ISR_RPURE  0x80000000
#define FIFO_ISR_RPORE  0x40000000
#define FIFO_ISR_RPUE   0x20000000
#  define FIFO_ISR_RX_ERR (FIFO_ISR_RPURE | FIFO_ISR_RPORE | FIFO_ISR_RPUE)
#define FIFO_ISR_TPOE   0x10000000
#define FIFO_ISR_TC     0x08000000
#define FIFO_ISR_RC     0x04000000
#  define FIFO_ISR_ALL     0xFC000000

/* "Magic" value for FIFO reset operations, and timeout, in msec */
#define FIFO_RESET_MAGIC    0x000000A5
#define FIFO_RESET_TIMEOUT  500

/* FIFO control register bits */
#define FIFO_TX_ALIGN32  0x00000000
#define FIFO_TX_ALIGN16  0x00000001

/* Locate the FIFO structure offset from the Ethernet base address */
ll_fifo_s *ll_fifo = (ll_fifo_s *) (LABX_PRIMARY_ETH_BASEADDR + LABX_FIFO_REGS_BASE);


#if !defined(CONFIG_NET_MULTI)
static struct eth_device *xps_ll_dev = NULL;
#endif

struct labx_eth_private {
  int           idx;
  unsigned char dev_addr[6];

  /* Number of match units */
  uint32_t numMatchUnits;

  /* Number of words needed to fully load a match unit */
  uint32_t matchLoadWords;

  /* Device-specific hook function for loading match units */
  void (*loadMatcher)(const uint8_t*);
};

static struct labx_eth_private priv_data = {
  .idx      = 0,
  .dev_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  
};

/* Performs a register write to a PHY */
static void write_phy_register(int phy_addr, int reg_addr, int phy_data)
{
  uint32_t addr;

  /* Write the data first, then the control register */
  addr = (LABX_MDIO_ETH_BASEADDR + MDIO_DATA_REG);
  *((volatile uint32_t *) addr) = phy_data;
  addr = (LABX_MDIO_ETH_BASEADDR + MDIO_CONTROL_REG);
  *((volatile uint32_t *) addr) = 
    (PHY_MDIO_WRITE | ((phy_addr & PHY_ADDR_MASK) << PHY_ADDR_SHIFT) |
     (reg_addr & PHY_REG_ADDR_MASK));

  /* Wait for the MDIO unit to go busy, then idle again */
  while((*((volatile uint32_t *) addr) & PHY_MDIO_BUSY) == 0);
  while((*((volatile uint32_t *) addr) & PHY_MDIO_BUSY) != 0);
}

/* Performs a register read from a PHY */
static uint32_t read_phy_register(int phy_addr, int reg_addr)
{
  uint32_t addr;
  uint32_t readValue;

  /* Write to the MDIO control register to initiate the read */
  addr = (LABX_MDIO_ETH_BASEADDR + MDIO_CONTROL_REG);
  *((volatile uint32_t *) addr) = 
    (PHY_MDIO_READ | ((phy_addr & PHY_ADDR_MASK) << PHY_ADDR_SHIFT) |
     (reg_addr & PHY_REG_ADDR_MASK));

  /* Wait for the MDIO unit to go busy, then idle again */
  while((*((volatile uint32_t *) addr) & PHY_MDIO_BUSY) == 0);
  while((*((volatile uint32_t *) addr) & PHY_MDIO_BUSY) != 0);

  addr = (LABX_MDIO_ETH_BASEADDR + MDIO_DATA_REG);
  readValue = *((volatile uint32_t *) addr);
  return(readValue);
}

/* Writes a value to a MAC register */
static void labx_eth_write_mac_reg(uint32_t reg_offset, uint32_t reg_data)
{
  //printf("REG %04X = %08X (was %08X)\n", reg_offset, reg_data, *(volatile uint32_t *)(LABX_PRIMARY_ETH_BASEADDR + reg_offset));
  /* Apply the register offset to the base address */
  *(volatile uint32_t *)(LABX_PRIMARY_ETH_BASEADDR + reg_offset) = reg_data;
}

/* Writes a value to the MDIO configuration register within the register
 * file of the controller being used for MDIO (may or may not be the same
 * as the primary!
 */
static void labx_eth_write_mdio_config(int config_data)
{
  /* Apply the MDIO register offset to the base address */
  *(volatile uint32_t *)(LABX_MDIO_ETH_BASEADDR + MAC_MDIO_CONFIG_REG) = config_data;
}

/* Reads a value from a MAC register */
uint32_t labx_eth_read_mac_reg(uint32_t reg_offset)
{
  uint32_t val = *(volatile uint32_t *)(LABX_PRIMARY_ETH_BASEADDR + reg_offset);
  return(val);
}

static int phy_addr = LABX_ETHERNET_PHY_ADDR;
static int link = 0;
static int first = 1;

/* setting ll_temac and phy to proper setting */
static int labx_eth_phy_ctrl(void)
{
  uint32_t result;

  if(first) {
    uint32_t id_high;
    uint32_t id_low;

    /* Read and report the PHY */
    id_high = read_phy_register(phy_addr, 2);
    id_low = read_phy_register(phy_addr, 3);
    printf("PHY ID at address 0x%02X: 0x%04X%04X\n", phy_addr, id_high, id_low);

    /* Perform RXC and TXC phase adjustment specific to some PHYs */
    if (id_high == BCM5481_ID_HIGH &&
        (id_low & BCM5481_ID_LOW_MASK) == BCM5481_ID_LOW) {
      /* RGMII Transmit Clock Delay: The RGMII transmit timing can be adjusted
       * by software control. TXD-to-GTXCLK clock delay time can be increased
       * by approximately 1.9 ns for 1000BASE-T mode, and between 2 ns to 6 ns
       * when in 10BASE-T or 100BASE-T mode by setting Register 1ch, SV 00011,
       * bit 9 = 1. Enabling this timing adjustment eliminates the need for
       * board trace delays as required by the RGMII specification.
       */
      write_phy_register(phy_addr, 0x1C, BCM5481_CLOCK_ALIGNMENT_REGISTER_SEL);
      result = read_phy_register(phy_addr, 0x1C);
      write_phy_register(phy_addr, 0x1C,
                         BCM5481_SHADOW_WRITE | BCM5481_CLOCK_ALIGNMENT_REGISTER_SEL | BCM5481_XMIT_CLOCK_DELAY);
      write_phy_register(phy_addr, 0x1C, BCM5481_CLOCK_ALIGNMENT_REGISTER_SEL);
      printf("RGMII Transmit Clock Delay: %d (0x%04X) => %d\n",
             ((result & BCM5481_XMIT_CLOCK_DELAY) != 0), result,
             ((read_phy_register(phy_addr, 0x1C)& BCM5481_XMIT_CLOCK_DELAY) != 0));

      result = read_phy_register(phy_addr, 0x00);
      result = result | BCM5481_AUTO_NEGOTIATE_ENABLE;
      write_phy_register(phy_addr, 0x00, result);
      printf("Auto-Negotiate Enable: %d (0x%04X) => %d\n",
             ((result & BCM5481_AUTO_NEGOTIATE_ENABLE) != 0), result,
             ((read_phy_register(phy_addr, 0x00) & BCM5481_AUTO_NEGOTIATE_ENABLE) != 0));
      result = read_phy_register(phy_addr, 0x18);
      result = result | BCM5481_SHADOW_WRITE | BCM5481_HPE_REGISTER_SELECT;
      result = result & ~BCM5481_HIGH_PERFORMANCE_ENABLE;
      printf("High-Performance Enable: %d (0x%04X) => %d\n",
             ((result & BCM5481_HIGH_PERFORMANCE_ENABLE) != 0), result,
             ((read_phy_register(phy_addr, 0x18) & BCM5481_HIGH_PERFORMANCE_ENABLE) != 0));

      write_phy_register(phy_addr, 0x18, BCM5481_RX_SKEW_REGISTER_SEL);
      result = read_phy_register(phy_addr, 0x18);
      write_phy_register(phy_addr, 0x18,
                         result | BCM5481_SHADOW_WRITE | BCM5481_RX_SKEW_REGISTER_SEL | BCM5481_RX_SKEW_ENABLE);
      write_phy_register(phy_addr, 0x18, BCM5481_RX_SKEW_REGISTER_SEL);
      printf("RGMII Receive Clock Skew: %d (0x%04X) => %d\n",
             ((result & BCM5481_RX_SKEW_ENABLE) != 0), result,
             ((read_phy_register(phy_addr, 0x18)& BCM5481_RX_SKEW_ENABLE) != 0));
	
    }

    /* Probe the SGMII PCS/PMA layer if present */
#if LABX_ETH_ALTERA_SGMII
    result = labx_eth_read_mac_reg(SGMII_PHY_ID_HIGH_REG);
    printf("SGMII PHY ID : 0x%08X\n", 
           (result | (labx_eth_read_mac_reg(SGMII_PHY_ID_LOW_REG) << 16)));
#endif
  } /* if(first call) */

  if(!link) {
    int timeout = 50;
    /* Requery the PHY general status register */
    /* Wait up to 5 secs for a link */
    while(timeout--) {
      result = read_phy_register(phy_addr, 1);
      if(result & 0x24) {
        printf("Link up!\n");
        break;
      }
      mdelay(100);
    }
  }
  
  result = read_phy_register(phy_addr, 1);
  if((result & 0x24) != 0x24) {
    printf("No link!\n");
    if(link) {
      link = 0;
      printf("Link has gone down\n");
    }
    return 0;
  }
  if(link == 0) {
    link = 1;
  } else {
    return 1;
  }
  
  result = read_phy_register(phy_addr, 10);
  if((result & 0x0800) == 0x0800) {
    labx_eth_write_mac_reg(MAC_SPEED_SELECT_REG, MAC_SPEED_1_GBPS);
    printf("1000BASE-T/FD\n");
    return 1;
  }
  result = read_phy_register(phy_addr, 5);
  if((result & 0x0100) == 0x0100) {
    labx_eth_write_mac_reg(MAC_SPEED_SELECT_REG, MAC_SPEED_100_MBPS);
    printf("100BASE-T/FD\n");
  } else if((result & 0x0040) == 0x0040) {
    labx_eth_write_mac_reg(MAC_SPEED_SELECT_REG, MAC_SPEED_10_MBPS);
    printf("10BASE-T/FD\n");
  } else {
    printf("Half Duplex not supported\n");
  }
  return 1;
}

/* Rx buffer is also used by FIFO mode */
static unsigned char rx_buffer[ETHER_MTU] __attribute((aligned(32)));

void debugll(int count)
{
  printf ("%d fifo isr 0x%08x, fifo_ier 0x%08x, fifo_rdfr 0x%08x, fifo_rdfo 0x%08x fifo_rlr 0x%08x\n",count, ll_fifo->isr, \
	  ll_fifo->ier, ll_fifo->rdfr, ll_fifo->rdfo, ll_fifo->rlf);
}

static int labx_eth_send_fifo(unsigned char *buffer, int length)
{
  uint32_t *buf = (uint32_t*) buffer;
  uint32_t len, i, val;

  /* U-Boot always provides buffer with 32-bit word alignment.  This still
   * needs to be explicitly set in the peripheral to ensure it is in the
   * correct alignment mode, or the first word may be dropped.
   */
  ll_fifo->fctl = FIFO_TX_ALIGN32;

  /* Compute the word count needed to account for the full packet */
  len = ((length + 3) / 4);

  /* Write to the data FIFO, enqueuing the packet's contents */
  for (i = 0; i < len; i++) {
    val = *buf++;
    ll_fifo->tdfd = val;
  }

  /* Write the length, in bytes, to commence transmission, and then
   * return the length transmitted.
   */
  ll_fifo->tlf = length;

  return(length);
}

static int labx_eth_recv_fifo(void)
{
  int len, len2, i, val;
  int *buf = (int*) &rx_buffer[0];

  if (ll_fifo->isr & FIFO_ISR_RC) {
    /* One or more packets have been received.  Check the read occupancy register
     * to see how much data is ready to be processed.
     */
    len = ll_fifo->rlf & RLF_MASK;
    len2 = ((len + 3) / 4);

    for (i = 0; i < len2; i++) {
      val = ll_fifo->rdfd;
      *buf++ = val ;
    }

    /* Re-check the occupancy register; if there are still FIFO contents
     * remaining, they are for valid packets.  Not sure if it's okay to call
     * NetReceive() more than once for each invocation, so we'll just leave
     * the ISR flag set instead and let the NetLoop invoke us again.
     */
    if(ll_fifo->rdfo == 0) ll_fifo->isr = FIFO_ISR_RC;

    /* Enqueue the received packet! */
    NetReceive (&rx_buffer[0], len);
  } else if(ll_fifo->isr & FIFO_ISR_RX_ERR) {
    printf("Rx error 0x%08X\n", ll_fifo->isr);

    /* A receiver error has occurred, reset the Rx logic */
    ll_fifo->isr = FIFO_ISR_ALL;
    ll_fifo->rdfr = FIFO_RESET_MAGIC;
  }

  return(0);
}

/* Constants for match unit loading */
#define MAC_MATCH_NONE 0
#define MAC_MATCH_ALL  1

/* Common MAC addresses */
#define MAC_ADDR_BYTES (6)
static const u8 MAC_BROADCAST[MAC_ADDR_BYTES] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static const u8 MAC_ZERO[MAC_ADDR_BYTES]      = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* Busy loops until the match unit configuration logic is idle.  The hardware goes 
 * idle very quickly and deterministically after a configuration word is written, 
 * so this should not consume very much time at all.
 */
static void wait_match_config(void) {
  uint32_t addr;
  uint32_t statusWord;
  uint32_t timeout = 10000;


  addr = (LABX_PRIMARY_ETH_BASEADDR + FILTER_CONTROL_REG);
  do {
    statusWord = *((volatile uint32_t *) addr);
    if (timeout-- == 0)
      {
        printf("labx_ethernet : wait_match_config timeout!\n");
        break;
      }
  } while(statusWord & MAC_ADDRESS_LOAD_ACTIVE);
}

/* Selects a set of match units for subsequent configuration loads */
typedef enum { SELECT_NONE, SELECT_SINGLE, SELECT_ALL } SelectionMode;
static void select_matchers(SelectionMode selectionMode,
                            uint32_t matchUnit) {
  uint32_t addr;

  addr = (LABX_PRIMARY_ETH_BASEADDR + FILTER_SELECT_REG);

  switch(selectionMode) {
  case SELECT_NONE:
    /* De-select all the match units */
    //printk("MAC SELECT %08X\n", 0);
    *((volatile uint32_t *) addr) = 0x00000000;
    break;

  case SELECT_SINGLE:
    /* Select a single unit */
    //printk("MAC SELECT %08X\n", 1 << matchUnit);
    *((volatile uint32_t *) addr) = (1 << matchUnit);
    break;

  default:
    /* Select all match units at once */
    //printk("MAC SELECT %08X\n", 0xFFFFFFFF);
    *((volatile uint32_t *) addr) = 0xFFFFFFFF;
    break;
  }
}

/* Sets the loading mode for any selected match units.  This revolves around
 * automatically disabling the match units' outputs while they're being
 * configured so they don't fire false matches, and re-enabling them as their
 * last configuration word is loaded.
 */
typedef enum { LOADING_MORE_WORDS, LOADING_LAST_WORD } LoadingMode;
static void set_matcher_loading_mode(LoadingMode loadingMode) {
  uint32_t addr;
  uint32_t controlWord;

  addr = (LABX_PRIMARY_ETH_BASEADDR + FILTER_CONTROL_REG);

  controlWord = *((volatile uint32_t *) addr);

  if(loadingMode == LOADING_MORE_WORDS) {
    /* Clear the "last word" bit to suppress false matches while the units are
     * only partially cleared out
     */
    controlWord &= ~MAC_ADDRESS_LOAD_LAST;
  } else {
    /* Loading the final word, flag the match unit(s) to enable after the
     * next configuration word is loaded.
     */
    controlWord |= MAC_ADDRESS_LOAD_LAST;
  }
  //printk("CONTROL WORD %08X\n", controlWord);

  *((volatile uint32_t *) addr) = controlWord;
}

/**
 * Clears any selected match units, preventing them from matching any packets 
 */
static void clear_selected_matchers(struct labx_eth_private *lp) {
  uint32_t addr;
  uint32_t wordIndex;
  
  /* Ensure the unit(s) disable as the first word is load to prevent erronous
   * matches as the units become partially-cleared
   */
  set_matcher_loading_mode(LOADING_MORE_WORDS);

  for(wordIndex = 0; wordIndex < lp->matchLoadWords; wordIndex++) {
    /* Assert the "last word" flag on the last word required to complete the clearing
     * of the selected unit(s).
     */
    if(wordIndex == (lp->matchLoadWords - 1)) {
      set_matcher_loading_mode(LOADING_LAST_WORD);
    }

    //printk("MAC LOAD %08X\n", 0);
    addr = (LABX_PRIMARY_ETH_BASEADDR + FILTER_LOAD_REG);
    *((volatile uint32_t *) addr) = 0x00000000;
  }
}

/* Constants common to all match architectures */
#define MAC_ADDRESS_LOAD_SHIFT 8

/* Constants for Xilinx SRL16E implementation */
#define NUM_SRL16E_CONFIG_WORDS 8
#define NUM_SRL16E_INSTANCES    12

/* Loads truth tables into a match unit using the Xilinx "unified" match
 * architecture.  This is SRL16E based (not cascaded) due to the efficient
 * packing of these primitives into Xilinx LUT6-based architectures.
 */
static void load_xilinx_matcher(const uint8_t matchMac[6]) {
  uint32_t addr;
  int32_t wordIndex;
  int32_t lutIndex;
  uint32_t configWord = 0x00000000;
  uint32_t matchChunk;
  
  /* All local writes will be to the MAC filter load register */
  addr = (LABX_PRIMARY_ETH_BASEADDR + FILTER_LOAD_REG);

  /* In this architecture, all of the SRL16Es are loaded in parallel, with each
   * configuration word supplying two bits to each.  Only one of the two bits can
   * ever be set, so there is just an explicit check for one.
   */
  for(wordIndex = (NUM_SRL16E_CONFIG_WORDS - 1); wordIndex >= 0; wordIndex--) {
    for(lutIndex = (NUM_SRL16E_INSTANCES - 1); lutIndex >= 0; lutIndex--) {
      matchChunk = ((matchMac[5 - (lutIndex / 2)] >> ((lutIndex & 1) << 2)) & 0x0F);
      configWord <<= 2;
      if(matchChunk == (wordIndex << 1)) configWord |= 0x01;
      if(matchChunk == ((wordIndex << 1) + 1)) configWord |= 0x02;
    }

    /* 12 nybbles are packed to the most-significant bytes */
    configWord <<= MAC_ADDRESS_LOAD_SHIFT;

    /* Two bits of truth table have been determined for each SRL16E, load the
     * word and wait for the configuration to occur.  Be sure to flag the last
     * word to automatically re-enable the match unit(s) as the last word completes.
     */
    if(wordIndex == 0) set_matcher_loading_mode(LOADING_LAST_WORD);
    *((volatile uint32_t *) addr) = configWord;
    wait_match_config();
  }
}

/* Constants for Altera / direct match implementation */
#define NUM_DIRECT_CONFIG_WORDS  2
#define NUM_DIRECT_CHAINS       12

static void load_direct_matcher(const uint8_t matchMac[6]) {
  uint32_t addr;
  int32_t wordIndex;
  int32_t chainIndex;
  uint32_t configWord;
  uint32_t loadBits;
  
  /* All local writes will be to the MAC filter load register */
  addr = (LABX_PRIMARY_ETH_BASEADDR + FILTER_LOAD_REG);

  /* In the direct-matching scheme, the raw MAC address is loaded in parallel.
   * The scan-in mechanism is identical to that used by the Xilinx architecture;
   * the MAC address bits are loaded into twelve independent chains of four bits
   * each.  Every word loaded supplies all twelve chains with two bits.
   */
  for(wordIndex = (NUM_DIRECT_CONFIG_WORDS - 1); wordIndex >= 0; wordIndex--) {
    configWord = 0x00000000;

    /* Iterate through each of the chains; all are partially loaded with each word */
    for(chainIndex = (NUM_DIRECT_CHAINS - 1); chainIndex >= 0; chainIndex--) {
      /* Resolve the byte of the MAC address the chain represents */
      loadBits = matchMac[5 - (chainIndex / 2)];

      /* Narrow down to which nybble of the byte */
      if((chainIndex % 2) == 0) {
        loadBits &= 0x0F;
      } else {
        loadBits >>= 4;
      }

      /* Resolve which dibit of the nybble; most-significant shifts in first,
       * and the loop counts down
       */
      if(wordIndex == 1) {
        loadBits >>= 2;
      } else {
        loadBits &= 0x03;
      }

      /* Shift the chain's bits into the word */
      configWord = ((configWord << 2) | loadBits);
    }

    /* 24 bits are packed into the most-significant bytes */
    configWord <<= MAC_ADDRESS_LOAD_SHIFT;

    /* Two bits of raw MAC address have been sliced for each chain, load the
     * word and wait for the configuration to occur.  Be sure to flag the last
     * word to automatically re-enable the match unit(s) as the last word completes.
     */
    if(wordIndex == 0) set_matcher_loading_mode(LOADING_LAST_WORD);
    *((volatile uint32_t *) addr) = configWord;
    wait_match_config();
  }
}

static void configure_mac_filter(struct labx_eth_private *lp,
                                 int unitNum, 
                                 const u8 mac[6], 
                                 int mode) {
  //printk("CONFIGURE MAC MATCH %d (%d), %02X:%02X:%02X:%02X:%02X:%02X\n", unitNum, mode,
  //	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  /* Ascertain that the configuration logic is ready, then select the matcher */
  wait_match_config();
  select_matchers(SELECT_SINGLE, unitNum);

  if (mode == MAC_MATCH_NONE) {
    clear_selected_matchers(lp);
  } else {
    /* Set the loading mode to disable as we load the first word */
    set_matcher_loading_mode(LOADING_MORE_WORDS);

    /* Call the platform-specific hook to load the selected match unit */
    lp->loadMatcher(mac);
  }

  /* De-select the match unit */
  select_matchers(SELECT_NONE, 0);
}

/* setup mac addr */
static int labx_eth_addr_setup(struct labx_eth_private *lp)
{
  char * env_p;
  char * end;
  int i;
  
  env_p = getenv("ethaddr");
  if (env_p == NULL) {
    printf("cannot get enviroment for \"ethaddr\".\n");
    return -1;
  }

  for (i = 0; i < 6; i++) {
    lp->dev_addr[i] = env_p ? simple_strtoul(env_p, &end, 16) : 0;
    if (env_p) env_p = (*end) ? end + 1 : end;
  }

  /* Configure for our unicast MAC address first, and the broadcast MAC
   * address second, provided there are enough MAC address filters.
   */
  if(lp->numMatchUnits >= 1) {
    configure_mac_filter(lp, 0, lp->dev_addr, MAC_MATCH_ALL);
  }

  if(lp->numMatchUnits >= 2) {
    configure_mac_filter(lp, 1, MAC_BROADCAST, MAC_MATCH_ALL);
  }
  
  return(0);
}

static void labx_eth_restart(void)
{
  /* Enable the receiver and transmitter */
  labx_eth_write_mac_reg(MAC_RX_CONFIG_REG, RX_ENABLE | RX_VLAN_TAGS_ENABLE);
  labx_eth_write_mac_reg(MAC_TX_CONFIG_REG, TX_ENABLE | TX_VLAN_TAGS_ENABLE);
}


static int labx_eth_init(struct eth_device *dev, bd_t *bis)
{
  struct labx_eth_private *lp = (struct labx_eth_private *)dev->priv;
  uint32_t addr;
  uint32_t matchArchId;

  if(!first) {
    /* Short-circuit some of the setup; still return no error to permit
     * the client code to keep going
     */
    labx_eth_restart();
    labx_eth_phy_ctrl();
    return(0);
  }

  /* Determine how many MAC address filters the instance supports, as reported
   * by a field within the revision register
   */
  addr = (LABX_PRIMARY_ETH_BASEADDR + REVISION_REG);
  lp->numMatchUnits = ((*((volatile uint32_t *) addr) & REVISION_MATCH_MASK) >>
                       REVISION_MATCH_SHIFT);
  
  /* Determine the proper loading function for the MAC match units
   * based upon the match architecture reported by the filtering hardware 
   * itself.
   */
  addr = (LABX_PRIMARY_ETH_BASEADDR + FILTER_CONTROL_REG);
  matchArchId = (*((volatile uint32_t *) addr) >> MATCH_ARCH_ID_SHIFT);

  printf("Lab X Ethernet has %d MAC filters, ", lp->numMatchUnits);
  switch(matchArchId) {
  case MATCH_ARCH_XILINX_SRL16E:
    /* Xilinx architecture */
    printf("SRL16E");
    lp->loadMatcher = &load_xilinx_matcher;
    break;

  case MATCH_ARCH_DIRECT:
    /* Altera or other direct matcher architecture */
    printf("DIRECT");
    lp->loadMatcher = &load_direct_matcher;
    break;

  default:
    printf("UNRECOGNIZED-(0x%02X)\n", matchArchId);
  }

  printf(" architecture\n");

  /* Clear ISR flags and reset both transmit and receive FIFO logic */
  ll_fifo->isr = FIFO_ISR_ALL;
  ll_fifo->tdfr = FIFO_RESET_MAGIC;
  ll_fifo->rdfr = FIFO_RESET_MAGIC;
  // printf ("fifo isr 0x%08x, fifo_ier 0x%08x, fifo_tdfv 0x%08x, fifo_rdfo 0x%08x fifo_rlf 0x%08x\n", ll_fifo->isr, ll_fifo->ier, ll_fifo->tdfv, ll_fifo->rdfo,ll_fifo->rlf);

  /* Configure the MDIO divisor and enable the interface to the PHY.
   * XILINX_HARD_MAC Note: The hard MDIO controller must be configured or
   * the SGMII autonegotiation won't happen. Even if we never use that MDIO controller.
   * This operation must access the register file of the controller being used
   * for MDIO access, which may or may not be the same as that used for the actual
   * communications! 
   */
  labx_eth_write_mdio_config((LABX_ETHERNET_MDIO_DIV & MDIO_DIVISOR_MASK) |
                             MDIO_ENABLED);
  
  /* Set up the MAC address in the hardware */
  labx_eth_addr_setup(lp);

  /* Enable the receiver and transmitter */
  labx_eth_write_mac_reg(MAC_RX_CONFIG_REG, RX_ENABLE | RX_VLAN_TAGS_ENABLE);
  labx_eth_write_mac_reg(MAC_TX_CONFIG_REG, TX_ENABLE | TX_VLAN_TAGS_ENABLE);

  /* Configure the PHY */
  labx_eth_phy_ctrl();
  first = 0;

  return(0);
}

static void labx_eth_halt(struct eth_device *dev)
{
  labx_eth_write_mac_reg(MAC_RX_CONFIG_REG, RX_DISABLE);
  labx_eth_write_mac_reg(MAC_TX_CONFIG_REG, TX_DISABLE);
}

int labx_eth_send(struct eth_device *dev, volatile void *packet, int length)
{
  if(!link)
    if(labx_eth_phy_ctrl() == 0)
      return 0;

  return(labx_eth_send_fifo((unsigned char *)packet, length));
}

int labx_eth_recv(struct eth_device *dev)
{
  return labx_eth_recv_fifo();
}

int labx_eth_initialize(bd_t *bis)
{
  struct eth_device *dev;
  
  dev = malloc(sizeof(*dev));
  if (dev == NULL)
    hang();
  
  memset(dev, 0, sizeof(*dev));
  sprintf(dev->name, "labx_ethernet%d", WHICH_ETH_PORT);
  dev->name[NAMESIZE - 1] = '\0';
  
  dev->iobase = LABX_PRIMARY_ETH_BASEADDR;
  dev->priv   = &priv_data;
  dev->init   =  labx_eth_init;
  dev->halt   =  labx_eth_halt;
  dev->send   =  labx_eth_send;
  dev->recv   =  labx_eth_recv;
  
  eth_register(dev);
  
  return(0);
}
