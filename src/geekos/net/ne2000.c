/*
 * NE2000 PCI Network Interface Card Driver
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */


#include <geekos/net/ne2000.h>
#include <geekos/io.h>
#include <geekos/irq.h>
#include <geekos/net/net.h>
#include <geekos/malloc.h>
#include <geekos/idt.h>
#include <geekos/net/net.h>

// #define DEBUG_NE2K(x...) Print("NE2k: " x)
#define DEBUG_NE2K(x...)

/* called when an interrupt tells us to. */
static void NE2000_Do_Receive(struct Net_Device *device) {
    uchar_t currentBuffer;
    ulong_t baseAddr = device->baseAddr;
    ushort_t ringBufferPage;


    Out_Byte(baseAddr + NE2K_CR,
             NE2K_CR_PAGE1 + NE2K_CR_NODMA + NE2K_CR_STA);
    currentBuffer = In_Byte(baseAddr + NE2K1W_CURR);
    Out_Byte(baseAddr + NE2K_CR,
             NE2K_CR_PAGE0 + NE2K_CR_NODMA + NE2K_CR_STA);

    while (currentBuffer != In_Byte(baseAddr + NE2K0R_BNRY)) {
        ringBufferPage = In_Byte(baseAddr + NE2K0R_BNRY) << 8;

        /* Receive as a packet; enqueue for further processing */
        Net_Device_Receive(device, ringBufferPage);

        /* Read the current buffer register */
        Out_Byte(baseAddr + NE2K_CR,
                 NE2K_CR_PAGE1 + NE2K_CR_NODMA + NE2K_CR_STA);
        currentBuffer = In_Byte(baseAddr + NE2K1W_CURR);
        Out_Byte(baseAddr + NE2K_CR,
                 NE2K_CR_PAGE0 + NE2K_CR_NODMA + NE2K_CR_STA);

        //Print("Current buffer: %x\n", currentBuffer);
        //Print("Boundary pointer: %x\n", In_Byte(baseAddr + NE2K0R_BNRY));
    }
}

static void NE2000_Interrupt_Handler(struct Interrupt_State *state) {
    struct Net_Device *device;
    ulong_t baseAddr;
    unsigned char isrMask;
    int rc;

    Begin_IRQ(state);
    DEBUG_NE2K("Handling NE2000 interrupt\n");

    // 2014 rc = Get_Net_Device_By_IRQ(state->intNum - FIRST_EXTERNAL_INT, &device);
    rc = Get_Net_Device_By_IRQ(state->intNum, &device);
    if(rc != 0) {
        Print("NE2000: Could not identify interrupt number %d (rc=%d)\n",
              state->intNum, rc);
        goto fail;
    }

    baseAddr = device->baseAddr;
    isrMask = In_Byte(NE2K0R_ISR + baseAddr);

    // DEBUG_NE2K("ISR Mask: %x\n" , isrMask);

    if(isrMask & NE2K_ISR_RXE) {
        //Print("RSR: %x\n", In_Byte(NE2K0R_RSR + baseAddr));
        ++device->rxPacketErrors;
    }

    if(isrMask & NE2K_ISR_TXE) {
        //Print("TSR: %x\n", In_Byte(NE2K0R_TSR + baseAddr));
        ++device->txPacketErrors;
    }

    if(isrMask & NE2K_ISR_OVW) {
        //Print("Ring Buffer Overflow Encountered!!\n");
        NE2000_Handle_Ring_Buffer_Overflow(device);
    }

    if(isrMask & NE2K_ISR_PRX) {
        DEBUG_NE2K("Receiving packet\n");
        NE2000_Do_Receive(device);
        ++device->rxPackets;
    }

    if(isrMask & NE2K_ISR_PTX) {
        //Print("Packet transmitted\n");
        DEBUG_NE2K("Transmitted.\n");
        //Print("TSR: %x\n", In_Byte(NE2K0R_TSR + baseAddr));
        ++device->txPackets;
    }

    Out_Byte(baseAddr + NE2K0R_ISR, isrMask);

  fail:
    End_IRQ(state);
}

int Init_NE2000(struct Net_Device *device) {

    unsigned char saProm[32];
    int i;
    ulong_t baseAddr = device->baseAddr;

    Print("Initializing ne2000 nic...\n");

    /* Read the PROM, taken from ne2k-pci.c in linux kernel */
    Out_Byte(baseAddr + NE2K_CR, 0x21);
    Out_Byte(baseAddr + NE2K0W_DCR, 0x49);
    Out_Byte(baseAddr + NE2K0W_RBCR0, 0x00);
    Out_Byte(baseAddr + NE2K0W_RBCR1, 0x00);
    Out_Byte(baseAddr + NE2K0W_IMR, 0x00);
    Out_Byte(baseAddr + NE2K0R_ISR, 0xFF);
    Out_Byte(baseAddr + NE2K0W_RCR, 0x20);
    Out_Byte(baseAddr + NE2K0W_TCR, 0x02);
    Out_Byte(baseAddr + NE2K0W_RBCR0, 32);
    Out_Byte(baseAddr + NE2K0W_RBCR1, 0x00);
    Out_Byte(baseAddr + NE2K0W_RSAR0, 0x00);
    Out_Byte(baseAddr + NE2K0W_RSAR1, 0x00);
    Out_Byte(baseAddr + NE2K_CR, NE2K_CR_STA + NE2K_CR_DMA_RREAD);

    for(i = 0; i < 32; ++i) {
        saProm[i] = (unsigned char)In_Word(baseAddr + NE2K_IO_PORT);
    }

    /* Ensure this is an ne2000. Ne2000 cards have the byte values
       0x57 in the 0x0e and 0x0f bytes in the station prom */
    if(saProm[0x0e] != 0x57 || saProm[0x0f] != 0x57) {
        return -1;
    }

    /* Print out the mac address */
    device->addrLength = 6;
    Print("Mac address: ");
    for(i = 0; i < 6; ++i) {
        device->devAddr[i] = saProm[i];
        Print("%02x", saProm[i]);
        if(i < 5)
            Print(":");
    }

    Print("\n");

    /* Install interrupt handler */
    Install_IRQ(device->irq, NE2000_Interrupt_Handler);

    /* Enable IRQ */
    Enable_IRQ(device->irq);

    /* Program Command Register for Page 0 */
    Out_Byte(baseAddr + NE2K_CR, 0x21);

    /* Initialize Data Configuration Register */
    Out_Byte(baseAddr + NE2K0W_DCR, 0x49);

    /* Clear remote byte count registers */
    Out_Byte(baseAddr + NE2K0W_RBCR0, 0x00);
    Out_Byte(baseAddr + NE2K0W_RBCR1, 0x00);

    /* Initialize Receive Configuration Register */
    Out_Byte(baseAddr + NE2K0W_RCR, 0x0C);

    /* Place the NIC in LOOPBACK mode 1 or 2 */
    Out_Byte(baseAddr + NE2K0W_TCR, 0x02);

    /* Initialize Transmit page start register */
    Out_Byte(baseAddr + NE2K0W_TPSR, NE2K_TRANSMIT_PAGE);

    /* Clear Interrupt Status Register ISR by writing 0xFF to it */
    Out_Byte(baseAddr + NE2K0R_ISR, 0xFF);

    /* Initialize Interrupt Mask Register */
    Out_Byte(baseAddr + NE2K0W_IMR, 0x0F + NE2K_ISR_OVW);

    /* Initialize the Receive Buffer Ring (BNDRY, PSTART, PSTOP) */
    Out_Byte(baseAddr + NE2K0W_PSTART, NE2K_RB_START);
    Out_Byte(baseAddr + NE2K0W_BNRY, NE2K_RB_START);
    Out_Byte(baseAddr + NE2K0W_PSTOP, NE2K_RB_STOP);

    /* Go to Page 1 */
    Out_Byte(baseAddr + NE2K_CR, 0x61);

    /* Initialize Physical Address Registers */
    Out_Byte(baseAddr + NE2K1W_PAR0, saProm[0]);
    Out_Byte(baseAddr + NE2K1W_PAR1, saProm[1]);
    Out_Byte(baseAddr + NE2K1W_PAR2, saProm[2]);
    Out_Byte(baseAddr + NE2K1W_PAR3, saProm[3]);
    Out_Byte(baseAddr + NE2K1W_PAR4, saProm[4]);
    Out_Byte(baseAddr + NE2K1W_PAR5, saProm[5]);

    /* Initialize Multicast Address Registers */
    Out_Byte(baseAddr + NE2K1W_MAR0, 0xFF);
    Out_Byte(baseAddr + NE2K1W_MAR1, 0xFF);
    Out_Byte(baseAddr + NE2K1W_MAR2, 0xFF);
    Out_Byte(baseAddr + NE2K1W_MAR3, 0xFF);
    Out_Byte(baseAddr + NE2K1W_MAR4, 0xFF);
    Out_Byte(baseAddr + NE2K1W_MAR5, 0xFF);
    Out_Byte(baseAddr + NE2K1W_MAR6, 0xFF);
    Out_Byte(baseAddr + NE2K1W_MAR7, 0xFF);

    /* Write the CURRENT buffer address */
    Out_Byte(baseAddr + NE2K1W_CURR, NE2K_RB_START);
    Print("Current buffer Address: %x\n",
          In_Byte(baseAddr + NE2K1W_CURR));

    /* Go back to Page 0 and Start */
    Out_Byte(baseAddr + NE2K_CR, 0x22);

    /* Initialize the Transmit Configuration Register */
    Out_Byte(baseAddr + NE2K0W_TCR, 0x00);

    return 0;

}

void NE2000_Transmit(struct Net_Device *device, void *buffer,
                     ulong_t length) {

    ulong_t baseAddr = device->baseAddr;

    KASSERT(!Interrupts_Enabled());

    unsigned int i;
    unsigned int newLength = length >> 1;
    unsigned short *newBuffer = (unsigned short *)buffer;

    /* Make sure we aren't currently transmitting */
    if(In_Byte(baseAddr + NE2K_CR) & NE2K_CR_TXP) {
        Print("ERROR - Currently transmitting\n");
        return;
    }

    /* Reset the card */
    NE2000_Reset(device);

    /* Set the Command Register */
    Out_Byte(baseAddr + NE2K_CR, 0x22);

    /* Handle the read before write bug */
    Out_Byte(baseAddr + NE2K0W_RBCR0, 0x42);
    Out_Byte(baseAddr + NE2K0W_RBCR1, 0x00);
    Out_Byte(baseAddr + NE2K0W_RSAR0, 0x42);
    Out_Byte(baseAddr + NE2K0W_RSAR1, 0x00);
    Out_Byte(baseAddr + NE2K_CR, NE2K_CR_DMA_RREAD + NE2K_CR_STA);
    Out_Byte(baseAddr + NE2K0R_ISR, NE2K_ISR_RDC);

    /* Load the packet size into the registers */
    Out_Byte(baseAddr + NE2K0W_RBCR0, length & 0xFF);
    Out_Byte(baseAddr + NE2K0W_RBCR1, length >> 8);

    /* Load the page start  into the RSARX registers */
    Out_Byte(baseAddr + NE2K0W_RSAR0, 0x00);
    Out_Byte(baseAddr + NE2K0W_RSAR1, NE2K_TRANSMIT_PAGE);

    /* Start the remote write */
    Out_Byte(baseAddr + NE2K_CR, NE2K_CR_DMA_RWRITE | NE2K_CR_STA);

    /* Write the data to the I/O port */
    for(i = 0; i < newLength; ++i) {
        Out_Word(baseAddr + NE2K_IO_PORT, newBuffer[i]);
    }

    /* Send the last byte of data if we have an odd length */
    if(length & 0x1) {
        Out_Word(baseAddr + NE2K_IO_PORT,
                 ((uchar_t *) buffer)[length - 1]);
    }


    /* Wait for transmit remote DMA */
    while ((In_Byte(baseAddr + NE2K0R_ISR) & NE2K_ISR_RDC) == 0) {
        Print("Waiting on remote DMA for transmit \n");
    }

    /* Ack the interrupt */
    Out_Byte(baseAddr + NE2K0R_ISR, NE2K_ISR_RDC);

    /* Now that Remote DMA is complete, set up the transmit */

    /* Store the transmit page in the transmit register */
    Out_Byte(baseAddr + NE2K0W_TPSR, NE2K_TRANSMIT_PAGE);

    /* Set transmit byte count */
    Out_Byte(baseAddr + NE2K0W_TBCR0, length & 0xFF);
    Out_Byte(baseAddr + NE2K0W_TBCR1, length >> 8);

    /* Issue transmit command */
    Out_Byte(baseAddr + NE2K_CR,
             NE2K_CR_NODMA | NE2K_CR_STA | NE2K_CR_TXP);

    device->txBytes += length;
}

/* actually reads the data out of the device */
void NE2000_Receive(struct Net_Device *device, void *buffer,
                    ulong_t length, ulong_t pageOffset) {
    ulong_t baseAddr = device->baseAddr;

    KASSERT(!Interrupts_Enabled());

    int i;
    int newLength = length >> 1;
    unsigned short *newBuffer = (unsigned short *)buffer;

    /* Set the Command Register */
    Out_Byte(baseAddr + NE2K_CR, 0x22);

    Out_Byte(baseAddr + NE2K0W_RCR, 0x0C);

    /* Load the packet size into the registers */
    Out_Byte(baseAddr + NE2K0W_RBCR0, length & 0xFF);
    Out_Byte(baseAddr + NE2K0W_RBCR1, length >> 8);

    /* Load the page start  into the RSARX registers */
    Out_Byte(baseAddr + NE2K0W_RSAR0, pageOffset & 0xFF);
    Out_Byte(baseAddr + NE2K0W_RSAR1, pageOffset >> 8);

    /* Start the remote write */
    Out_Byte(baseAddr + NE2K_CR, NE2K_CR_DMA_RREAD | NE2K_CR_STA);

    /* Read the data in through the I/O port */
    for(i = 0; i < newLength; ++i) {
        newBuffer[i] = In_Word(baseAddr + NE2K_IO_PORT);
    }

    /* Receive the last byte of data if we have an odd length */
    if(length & 0x1) {
        ((uchar_t *) buffer)[length - 1] =
            In_Byte(baseAddr + NE2K_IO_PORT);
    }

    /* Ack the remote DMA interrupt */
    Out_Byte(baseAddr + NE2K0R_ISR, NE2K_ISR_RDC);

    device->rxBytes += length;
}

void NE2000_Complete_Receive(struct Net_Device *device,
                             struct Net_Device_Header *hdr) {
    /* Advance the boundary pointer */
    Out_Byte(device->baseAddr + NE2K0W_BNRY, hdr->next);
}

void NE2000_Reset(struct Net_Device *device) {
    ulong_t baseAddr = device->baseAddr;

    KASSERT(!Interrupts_Enabled());

    Out_Byte(baseAddr + NE2K_RESET_PORT, In_Byte(NE2K_RESET_PORT));

    while (In_Byte(baseAddr + NE2K0R_ISR) & NE2K_ISR_RST) {
        Print("NIC has not reset yet\n");
    }
}

void NE2000_Get_Dev_Hdr(struct Net_Device *device,
                        struct Net_Device_Header *hdr,
                        ulong_t pageOffset) {
    unsigned int i;
    ulong_t baseAddr = device->baseAddr;
    ulong_t size = sizeof(struct Net_Device_Header) >> 1;
    ushort_t *buffer = (ushort_t *) hdr;

    /* Set the Command Register */
    Out_Byte(baseAddr + NE2K_CR, 0x22);

    /* Load the packet size into the registers */
    Out_Byte(baseAddr + NE2K0W_RBCR0, sizeof(struct Net_Device_Header));
    Out_Byte(baseAddr + NE2K0W_RBCR1, 0);

    /* Load the page start  into the RSARX registers */
    Out_Byte(baseAddr + NE2K0W_RSAR0, 0x00);
    Out_Byte(baseAddr + NE2K0W_RSAR1, pageOffset);

    /* Start the remote write */
    Out_Byte(baseAddr + NE2K_CR, NE2K_CR_DMA_RREAD | NE2K_CR_STA);

    /* Read the data in through the I/O port */
    for(i = 0; i < size; ++i) {
        buffer[i] = In_Word(baseAddr + NE2K_IO_PORT);
    }

}

void NE2000_Handle_Ring_Buffer_Overflow(struct Net_Device *device) {
    int txp, resend;
    ulong_t baseAddr = device->baseAddr;
    /* 1. Read and store the value of the TXP bit */
    txp = In_Byte(baseAddr + NE2K_CR) & NE2K_CR_TXP;

    /* 2. Issue the STOP command to the NIC */
    Out_Byte(baseAddr + NE2K_CR, 0x21);

    /* 3. Wait for at least 1.6 ms */
    IO_Delay();

    /* 4. Clear the NIC’s Remote Byte Count */
    Out_Byte(baseAddr + NE2K0W_RBCR0, 0x00);
    Out_Byte(baseAddr + NE2K0W_RBCR1, 0x00);

    /* 5. Handle resend */
    if(!txp) {
        resend = 0;
    } else {
        int isr = In_Byte(baseAddr + NE2K0R_ISR);
        if((isr & NE2K_ISR_PTX) || (isr & NE2K_ISR_TXE)) {
            resend = 0;
        } else {
            resend = 1;
        }
    }

    /* 6. Place the NIC into loopback mode */
    Out_Byte(baseAddr + NE2K0W_TCR, 0x02);

    /* 7. Issue START command to the NIC */
    Out_Byte(baseAddr + NE2K_CR, 0x22);

    /* 8. Remove one or more packets from the receive buffer ring */
    NE2000_Do_Receive(device);

    /* 9. Reset the overwrite warning (OVW) bit in the ISR */
    Out_Byte(baseAddr + NE2K0R_ISR, NE2K_ISR_OVW);

    /* 10. Take the NIC out of loopback */
    Out_Byte(baseAddr + NE2K0W_TCR, 0x00);

    /* 11. Restart the interrupted transmit if resend = 1 */
    if(resend) {
        Out_Byte(baseAddr + NE2K_CR, 0x26);
    }
}


struct Net_Device_Capabilities g_ne2000Capabilities = {
    "ne2000",
    Init_NE2000,
    NE2000_Transmit,
    NE2000_Receive,
    NE2000_Reset,
    NE2000_Get_Dev_Hdr,
    NE2000_Complete_Receive
};
