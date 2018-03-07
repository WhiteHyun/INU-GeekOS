/*
 * NE2000 PCI Network Interface Card Driver
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */


#ifndef _NE2000_H_
#define _NE2000_H_

#include <geekos/ktypes.h>
#include <geekos/list.h>
#include <geekos/net/net.h>


#define NE2K_OFFSET_ADDR(reg) (reg)

/* Command Register - On all Pages */
#define NE2K_CR           NE2K_OFFSET_ADDR(0x00)

/* Page 0 Registers */
#define NE2K0R_CLDA0      NE2K_OFFSET_ADDR(0x01)
#define NE2K0W_PSTART     NE2K_OFFSET_ADDR(0x01)
#define NE2K0R_CLDA1      NE2K_OFFSET_ADDR(0x02)
#define NE2K0W_PSTOP      NE2K_OFFSET_ADDR(0x02)
#define NE2K0R_BNRY       NE2K_OFFSET_ADDR(0x03)
#define NE2K0W_BNRY       NE2K_OFFSET_ADDR(0x03)
#define NE2K0R_TSR        NE2K_OFFSET_ADDR(0x04)
#define NE2K0W_TPSR       NE2K_OFFSET_ADDR(0x04)
#define NE2K0R_NCR        NE2K_OFFSET_ADDR(0x05)
#define NE2K0W_TBCR0      NE2K_OFFSET_ADDR(0x05)
#define NE2K0R_FIFO       NE2K_OFFSET_ADDR(0x06)
#define NE2K0W_TBCR1      NE2K_OFFSET_ADDR(0x06)
#define NE2K0R_ISR        NE2K_OFFSET_ADDR(0x07)
#define NE2K0W_ISR        NE2K_OFFSET_ADDR(0x07)
#define NE2K0R_CRDA0      NE2K_OFFSET_ADDR(0x08)
#define NE2K0W_RSAR0      NE2K_OFFSET_ADDR(0x08)
#define NE2K0R_CRDA1      NE2K_OFFSET_ADDR(0x09)
#define NE2K0W_RSAR1      NE2K_OFFSET_ADDR(0x09)
#define NE2K0W_RBCR0      NE2K_OFFSET_ADDR(0x0A)
#define NE2K0W_RBCR1      NE2K_OFFSET_ADDR(0x0B)
#define NE2K0R_RSR        NE2K_OFFSET_ADDR(0x0C)
#define NE2K0W_RCR        NE2K_OFFSET_ADDR(0x0C)
#define NE2K0R_CNTR0      NE2K_OFFSET_ADDR(0x0D)
#define NE2K0W_TCR        NE2K_OFFSET_ADDR(0x0D)
#define NE2K0R_CNTR1      NE2K_OFFSET_ADDR(0x0E)
#define NE2K0W_DCR        NE2K_OFFSET_ADDR(0x0E)
#define NE2K0R_CNTR2      NE2K_OFFSET_ADDR(0x0F)
#define NE2K0W_IMR        NE2K_OFFSET_ADDR(0x0F)

/* Page 1 Registers */
#define NE2K1R_PAR0       NE2K_OFFSET_ADDR(0x01)
#define NE2K1W_PAR0       NE2K_OFFSET_ADDR(0x01)
#define NE2K1R_PAR1       NE2K_OFFSET_ADDR(0x02)
#define NE2K1W_PAR1       NE2K_OFFSET_ADDR(0x02)
#define NE2K1R_PAR2       NE2K_OFFSET_ADDR(0x03)
#define NE2K1W_PAR2       NE2K_OFFSET_ADDR(0x03)
#define NE2K1R_PAR3       NE2K_OFFSET_ADDR(0x04)
#define NE2K1W_PAR3       NE2K_OFFSET_ADDR(0x04)
#define NE2K1R_PAR4       NE2K_OFFSET_ADDR(0x05)
#define NE2K1W_PAR4       NE2K_OFFSET_ADDR(0x05)
#define NE2K1R_PAR5       NE2K_OFFSET_ADDR(0x06)
#define NE2K1W_PAR5       NE2K_OFFSET_ADDR(0x06)
#define NE2K1R_CURR       NE2K_OFFSET_ADDR(0x07)
#define NE2K1W_CURR       NE2K_OFFSET_ADDR(0x07)
#define NE2K1R_MAR0       NE2K_OFFSET_ADDR(0x08)
#define NE2K1W_MAR0       NE2K_OFFSET_ADDR(0x08)
#define NE2K1R_MAR1       NE2K_OFFSET_ADDR(0x09)
#define NE2K1W_MAR1       NE2K_OFFSET_ADDR(0x09)
#define NE2K1R_MAR2       NE2K_OFFSET_ADDR(0x0A)
#define NE2K1W_MAR2       NE2K_OFFSET_ADDR(0x0A)
#define NE2K1R_MAR3       NE2K_OFFSET_ADDR(0x0B)
#define NE2K1W_MAR3       NE2K_OFFSET_ADDR(0x0B)
#define NE2K1R_MAR4       NE2K_OFFSET_ADDR(0x0C)
#define NE2K1W_MAR4       NE2K_OFFSET_ADDR(0x0C)
#define NE2K1R_MAR5       NE2K_OFFSET_ADDR(0x0D)
#define NE2K1W_MAR5       NE2K_OFFSET_ADDR(0x0D)
#define NE2K1R_MAR6       NE2K_OFFSET_ADDR(0x0E)
#define NE2K1W_MAR6       NE2K_OFFSET_ADDR(0x0E)
#define NE2K1R_MAR7       NE2K_OFFSET_ADDR(0x0F)
#define NE2K1W_MAR7       NE2K_OFFSET_ADDR(0x0F)

/* Page 2 Register */
#define NE2K2R_PSTART     NE2K_OFFSET_ADDR(0x01)
#define NE2K2W_CLDA0      NE2K_OFFSET_ADDR(0x01)
#define NE2K2R_PSTOP      NE2K_OFFSET_ADDR(0x02)
#define NE2K2W_CLDA1      NE2K_OFFSET_ADDR(0x02)
#define NE2K2R_RNPP       NE2K_OFFSET_ADDR(0x03)
#define NE2K2W_RNPP       NE2K_OFFSET_ADDR(0x03)
#define NE2K2R_TPSR       NE2K_OFFSET_ADDR(0x04)
#define NE2K2R_LNPP       NE2K_OFFSET_ADDR(0x05)
#define NE2K2W_LNPP       NE2K_OFFSET_ADDR(0x05)
#define NE2K2R_AC_HI      NE2K_OFFSET_ADDR(0x06)
#define NE2K2W_AC_HI      NE2K_OFFSET_ADDR(0x06)
#define NE2K2R_AC_LO      NE2K_OFFSET_ADDR(0x07)
#define NE2K2W_AC_LO      NE2K_OFFSET_ADDR(0x07)
#define NE2K2R_RCR        NE2K_OFFSET_ADDR(0x0C)
#define NE2K2R_TCR        NE2K_OFFSET_ADDR(0x0D)
#define NE2K2R_DCR        NE2K_OFFSET_ADDR(0x0E)
#define NE2K2R_IMR        NE2K_OFFSET_ADDR(0x0F)

/* Command Register (CR, RW) bits */
#define NE2K_CR_STP       0x01
#define NE2K_CR_STA       0x02
#define NE2K_CR_TXP       0x04
#define NE2K_CR_RD0       0x08
#define NE2K_CR_RD1       0x10
#define NE2K_CR_RD2       0x20
#define NE2K_CR_PS0       0x40
#define NE2K_CR_PS1       0x80

/* Command Register (CR, RW) pages */
#define NE2K_CR_PAGE0     0x00
#define NE2K_CR_PAGE1     NE2K_CR_PS0
#define NE2K_CR_PAGE2     NE2K_CR_PS1

/* Command Register Remove DMA Command */
#define NE2K_CR_DMA_NOT_ALLOWED       0x00
#define NE2K_CR_DMA_RREAD             NE2K_CR_RD0
#define NE2K_CR_DMA_RWRITE            NE2K_CR_RD1
#define NE2K_CR_DMA_SEND_PACKET       (NE2K_CR_RD0 | NE2K_CR_RD1)
#define NE2K_CR_NODMA                 NE2K_CR_RD2

/* Interrupt Status Register (ISR, RW) */
#define NE2K_ISR_PRX       0x01
#define NE2K_ISR_PTX       0x02
#define NE2K_ISR_RXE       0x04
#define NE2K_ISR_TXE       0x08
#define NE2K_ISR_OVW       0x10
#define NE2K_ISR_CNT       0x20
#define NE2K_ISR_RDC       0x40
#define NE2K_ISR_RST       0x80

/* Interrupt Mask Register (IMR, W) */
#define NE2K_IMR_PRXE       0x01
#define NE2K_IMR_PTXE       0x02
#define NE2K_IMR_RXEE       0x04
#define NE2K_IMR_TXEE       0x08
#define NE2K_IMR_OVWE       0x10
#define NE2K_IMR_CNTE       0x20
#define NE2K_IMR_RDCE       0x40

/* Data Configuration Register (DCR, W) */
#define NE2K_DCR_WTS        0x01
#define NE2K_DCR_BOS        0x02
#define NE2K_DCR_LAS        0x04
#define NE2K_DCR_LS         0x08
#define NE2K_DCR_ARM        0x10
#define NE2K_DCR_FT0        0x20
#define NE2K_DCR_FT1        0x40

/* Data Configuration Register (DCR, W) FIFO Thresholds */
#define NE2K_DCR_FIFO1      0x00
#define NE2K_DCR_FIFO2      NE2K_DCR_FT0
#define NE2K_DCR_FIFO4      NE2K_DCR_FT1
#define NE2K_DCR_FIFO6      (NE2K_DCR_FT0 | NE2K_DCR_FT1)

/* Transmit Configuration Register (TCR, W) */
#define NE2K_TCR_CRC       0x01
#define NE2K_TCR_LB0       0x02
#define NE2K_TCR_LB1       0x04
#define NE2K_TCR_ATD       0x08
#define NE2K_TCR_OFST      0x10

/* Transmit Configuration Register (TCR, W) Loopback Controls */
#define NE2K_TCR_LB_M0     0x00
#define NE2K_TCR_LB_M1     NE2K_TCR_LB0
#define NE2K_TCR_LB_M2     NE2K_TCR_LB1
#define NE2K_TCR_LB_M3     (NE2K_TCR_LB0 | NE2K_TCR_LB1)

/* Transmit Status Register (TSR, R) */
#define NE2K_TSR_PTX       0x01
#define NE2K_TSR_COL       0x04
#define NE2K_TSR_ABT       0x08
#define NE2K_TSR_CRS       0x10
#define NE2K_TSR_FU        0x20
#define NE2K_TSR_CDH       0x40
#define NE2K_TSR_OWC       0x80

/* Receive Configuration Register (RCR, W) */
#define NE2K_RCR_SEP       0x01
#define NE2K_RCR_AR        0x02
#define NE2K_RCR_AB        0x04
#define NE2K_RCR_AM        0x08
#define NE2K_RCR_PRO       0x10
#define NE2K_RCR_MON       0x20

/* Receive Status Register (RSR, R) */
#define NE2K_RSR_PRX       0x01
#define NE2K_RSR_CRC       0x02
#define NE2K_RSR_FAE       0x04
#define NE2K_RSR_FO        0x08
#define NE2K_RSR_MPA       0x10
#define NE2K_RSR_PHY       0x20
#define NE2K_RSR_DIS       0x40
#define NE2K_RSR_DFR       0x80

/* Memory addresses of ring buffers */
#define NE2K_TRANSMIT_PAGE 0x40
#define NE2K_TB_START      0x40
#define NE2K_TB_STOP       0x52
#define NE2K_RB_START      0x52
#define NE2K_RB_STOP       0x92

#define NE2K_IO_PORT       NE2K_OFFSET_ADDR(0x10)
#define NE2K_RESET_PORT    NE2K_OFFSET_ADDR(0x1F)

extern struct Net_Device_Capabilities g_ne2000Capabilities;

/* Public Functions*/
extern int Init_NE2000(struct Net_Device *device);
extern void NE2000_Transmit(struct Net_Device *device, void *buffer,
                            ulong_t length);
extern void NE2000_Receive(struct Net_Device *device, void *buffer,
                           ulong_t length, ulong_t pageOffset);
extern void NE2000_Reset(struct Net_Device *device);
extern void NE2000_Get_Dev_Hdr(struct Net_Device *device,
                               struct Net_Device_Header *hdr,
                               ulong_t pageOffset);

extern void NE2000_Receive_Thread(struct Net_Device *device);
extern void NE2000_Transmit_Thread(struct Net_Device *device);
extern void NE2000_Complete_Receive(struct Net_Device *device,
                                    struct Net_Device_Header *hdr);
void NE2000_Handle_Ring_Buffer_Overflow(struct Net_Device *device);

#endif
