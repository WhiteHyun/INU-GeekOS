; GeekOS setup code
; Copyright (c) 2001,2004 David H. Hovemeyer <daveho@cs.umd.edu>
; $Revision: 1.10 $

; This is free software.  You are permitted to use,
; redistribute, and modify it as specified in the file "COPYING".

; A lot of this code is adapted from Kernel Toolkit 0.2
; and Linux version 2.2.x, so the following copyrights apply:

; Copyright (C) 1991, 1992 Linus Torvalds
; modified by Drew Eckhardt
; modified by Bruce Evans (bde)
; adapted for Kernel Toolkit by Luigi Sgro

%include "defs.asm"

[BITS 16]
[ORG 0x0]

BeginSetup:
start_setup:

	; Redefine the data segment so we can access variables
	; declared in this file.
	mov	ax, SETUPSEG
	mov	ds, ax

	; Use int 15h to find out size of extended memory in KB.
	; Extended memory is the memory above 1MB.  So by
	; adding 1MB to this amount, we get the total amount
	; of system memory.  We can only detect 64MB this way,
	; but that's OK for now.
	mov	ah, 0x88
	int	0x15
	add	ax, 1024	; 1024 KB == 1 MB
	;; mov	[mem_size_kbytes], ax

        jc short err
	test ax, ax		; size = 0 is an error
	je short err
	cmp ah, 0x86		; unsupported function
	je short err
	cmp ah, 0x80		; invalid command
	je short err
	mov	word [mem_size_kbytes], ax
	jmp ok
err:
	;; failed to get memory that way, try the general purpose routine
	;; mov	word [mem_size_kbytes], (1024*64)-1
	mov	ax, MEMMAPSEG
	mov	es, ax
	xor	di, di
	call	do_e820
	xor	ax, ax
	mov	word [mem_size_kbytes], ax

ok:
	; Kill the floppy motor.
	call	Kill_Motor

	; Block interrupts, since we can't meaningfully handle them yet
	; and we no longer need BIOS services.
	cli

	; Set up IDT and GDT registers
	lidt	[IDT_Pointer]
	lgdt	[GDT_Pointer]

	; Initialize the interrupt controllers, and enable the
	; A20 address line
	call	Init_PIC
	call	Enable_A20

	; Switch to protected mode!
	mov	ax, 0x01
	lmsw	ax

	; Jump to 32 bit code.
	jmp	dword KERNEL_CS:(SETUPSEG << 4) + setup_32

[BITS 32]
setup_32:

	; set up data segment registers
	mov	ax, KERNEL_DS
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	; Create the stack for the initial kernel thread.
	mov	esp, KERN_STACK + 4096

	; Build Boot_Info struct on stack.
	; Note that we push the fields on in reverse order,
	; since the stack grows downwards.
	mov     eax, (MEMMAPSEG<<4)			;; address of memap segments
	push	eax

	xor	eax, eax
	mov	ax, [(SETUPSEG<<4)+mmap_ent]		;; number of segments
	push	eax

	xor     eax, eax
	mov     eax, [(SETUPSEG<<4)+bootDrive]
	push    eax

	xor	eax, eax
	mov	ax, [(SETUPSEG<<4)+mem_size_kbytes]
	push	eax		; memSizeKB

	mov	eax, (INITSEG<<4)
	push	eax		; startKernInfo

	push	dword 24	; bootInfoSize

	; Pass pointer to Boot_Info struct as argument to kernel
	; entry point.
	push	esp

	; Push return address to make this look like a call
	; XXX - untested
	push	dword (SETUPSEG<<4)+.returnAddr

	; Far jump into kernel
	jmp	KERNEL_CS:ENTRY_POINT

.returnAddr:
	; We shouldn't return here.
.here:	jmp .here

[BITS 16]

; Kill the floppy motor.
; This code was shamelessly stolen from Linux.
Kill_Motor:
	mov	dx, 0x3f2
	xor	al, al
	out	dx, al
	ret

Init_PIC:
	; Initialize master and slave PIC!
	mov	al, ICW1
	out	0x20, al		; ICW1 to master
	call	Delay
	out	0xA0, al		; ICW1 to slave
	call	Delay
	mov	al, ICW2_MASTER
	out	0x21, al		; ICW2 to master
	call	Delay
	mov	al, ICW2_SLAVE
	out	0xA1, al		; ICW2 to slave
	call	Delay
	mov	al, ICW3_MASTER
	out	0x21, al		; ICW3 to master
	call	Delay
	mov	al, ICW3_SLAVE
	out	0xA1, al		; ICW3 to slave
	call	Delay
	mov	al, ICW4
	out	0x21, al		; ICW4 to master
	call	Delay
	out	0xA1, al		; ICW4 to slave
	call	Delay
	mov	al, 0xff		; mask all ints in slave
	out	0xA1, al		; OCW1 to slave
	call	Delay
	mov	al, 0xfb		; mask all ints but 2 in master
	out	0x21, al		; OCW1 to master
	call	Delay
	ret


;
; Code from http://wiki.osdev.org/How_Do_I_Determine_The_Amount_Of_RAM#Getting_an_E820_Memory_Map
;   Adapted for GeekOS by Jeff Hollingsworth (2/4/10)
;;
do_e820:
        xor ebx, ebx            ; ebx must be 0 to start
        xor bp, bp              ; keep an entry count in bp
        mov edx, 0x0534D4150    ; Place "SMAP" into edx
        mov eax, 0xe820
        mov [es:di + 20], dword 1       ; force a valid ACPI 3.X entry
        mov ecx, 24             ; ask for 24 bytes
        int 0x15
        jc short .failed        ; carry set on first call means "unsupported function"
        mov edx, 0x0534D4150    ; Some BIOSes apparently trash this register?
        cmp eax, edx            ; on success, eax must have been reset to "SMAP"
        jne short .failed
        test ebx, ebx           ; ebx = 0 implies list is only 1 entry long (worthless)
        je short .failed
        jmp short .jmpin
.e820lp:
        mov eax, 0xe820         ; eax, ecx get trashed on every int 0x15 call
        mov [es:di + 20], dword 1       ; force a valid ACPI 3.X entry
        mov ecx, 24             ; ask for 24 bytes again
        int 0x15
        jc short .e820f         ; carry set means "end of list already reached"
        mov edx, 0x0534D4150    ; repair potentially trashed register
.jmpin:
        jcxz .skipent           ; skip any 0 length entries
        cmp cl, 20              ; got a 24 byte ACPI 3.X response?
        jbe short .notext
        test byte [es:di + 20], 1       ; if so: is the "ignore this data" bit clear?
        je short .skipent
.notext:
        mov ecx, [es:di + 8]    ; get lower dword of memory region length
        test ecx, ecx           ; is the qword == 0?
        jne short .goodent
        mov ecx, [es:di + 12]   ; get upper dword of memory region length
        jecxz .skipent          ; if length qword is 0, skip entry
.goodent:
        inc bp                  ; got a good entry: ++count, move to next storage spot
        add di, 24
.skipent:
        test ebx, ebx           ; if ebx resets to 0, list is complete
        jne short .e820lp
.e820f:
        mov [mmap_ent], bp      ; store the entry count
        ret                     ; test opcode cleared carry flag
.failed:
        stc                     ; "function unsupported" error exit
        ret

mmap_ent:       dw      0


; Linux uses this code.
; The idea is that some systems issue port I/O instructions
; faster than the device hardware can deal with them.
Delay:
	jmp	.done
.done:	ret

; Enable the A20 address line, so we can correctly address
; memory above 1MB.
Enable_A20:
	mov	al, 0xD1
	out	0x64, al
	call	Delay
	mov	al, 0xDF
	out	0x60, al
	call	Delay
	ret


; ----------------------------------------------------------------------
; Setup data
; ----------------------------------------------------------------------

mem_size_kbytes: dw 0


; ----------------------------------------------------------------------
; The GDT.  Creates flat 32-bit address space for the kernel
; code, data, and stack.  Note that this GDT is just used
; to create an environment where we can start running 32 bit
; code.  The kernel will create and manage its own GDT.
; ----------------------------------------------------------------------

; GDT initialization stuff
NUM_GDT_ENTRIES equ 3		; number of entries in GDT
GDT_ENTRY_SZ equ 8		; size of a single GDT entry

align 8, db 0
GDT:
	; Descriptor 0 is not used
	dw 0
	dw 0
	dw 0
	dw 0

	; Descriptor 1: kernel code segment
	dw 0xFFFF	; bytes 0 and 1 of segment size
	dw 0x0000	; bytes 0 and 1 of segment base address
	db 0x00		; byte 2 of segment base address
	db 0x9A		; present, DPL=0, non-system, code, non-conforming,
			;   readable, not accessed
	db 0xCF		; granularity=page, 32 bit code, upper nibble of size
	db 0x00		; byte 3 of segment base address

	; Descriptor 2: kernel data and stack segment
	; NOTE: what Intel calls an "expand-up" segment
	; actually means that the stack will grow DOWN,
	; towards lower memory.  So, we can use this descriptor
	; for both data and stack references.
	dw 0xFFFF	; bytes 0 and 1 of segment size
	dw 0x0000	; bytes 0 and 1 of segment base address
	db 0x00		; byte 2 of segment base address
	db 0x92		; present, DPL=0, non-system, data, expand-up,
			;   writable, not accessed
	db 0xCF		; granularity=page, big, upper nibble of size
	db 0x00		; byte 3 of segment base address

GDT_Pointer:
	dw NUM_GDT_ENTRIES*GDT_ENTRY_SZ	; limit
	dd (SETUPSEG<<4) + GDT		; base address

IDT_Pointer:
	dw 0
	dd 00
EndSetup:

        FillSector  times (508 - (EndSetup - BeginSetup)) db 0

bootDrive:
	dw      0
	dw      0

;;
;; start up a seconday processor
;;     This code needs to start on a 4KB page boundry.
;;
align 4096, db 0
[BITS 16]
start_secondary_cpu:

	mov     ax, SETUPSEG
        mov     ds, ax

	; Block interrupts, since we can't meaningfully handle them yet
	; and we no longer need BIOS services.
	cli

	mov sp,TRAMPOLINE_STACK

	; Set up IDT and GDT registers
	lidt	[IDT_Pointer]
	lgdt	[GDT_Pointer]

	; Initialize the interrupt controllers, and enable the
	; A20 address line
	;; call	Init_PIC
	;; call	Enable_A20

	; Switch to protected mode!
	mov	ax, 0x01
	lmsw	ax


	; Jump to 32 bit code.
	jmp	dword KERNEL_CS:(SETUPSEG << 4) + setup_2nd_32

[BITS 32]
setup_2nd_32:

	; set up data segment registers
	mov	ax, KERNEL_DS
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	; Create the stack for the initial kernel thread.
	;;mov	esp, [endSecondStack]
	;;mov	esp, TRAMPOLINE_STACK

	;; secondary stack is the page we allocated, stacks grow down so move the stack pointer to the end of the page
	mov	esp, dword [SECONDARY_STACK]
	add	esp, 4096
	push 	esp

	; Far jump into kernel
	push	dword (SETUPSEG<<4)+.returnAddr2

	jmp	KERNEL_CS:SECONDARY_ENTRY_POINT
.returnAddr2:
.there:	jmp .there

;;
;; XXXX This depends on the above function being less than 4KB since we use a fixed offset to this next location
;;
align 4096, db 0
endSecondStack:
	dw 0

align 4096, db 0
TRAMPOLINE_STACK:
