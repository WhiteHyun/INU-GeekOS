; Low level interrupt/thread handling code for GeekOS.
; Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
; Copyright (c) 2003,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
; $Revision: 1.18 $

; All rights reserved.
; This code may not be resdistributed without the permission of the copyright holders.

; This is 32 bit code to be linked into the kernel.
; It defines low level interrupt handler entry points that we'll use
; to populate the IDT.  It also contains the interrupt handling
; and thread context switch code.

%include "defs.asm"
%include "symbol.asm"

[BITS 32]

; ----------------------------------------------------------------------
; Definitions
; ----------------------------------------------------------------------

; This is the size of the Interrupt_State struct in int.h
INTERRUPT_STATE_SIZE equ 64     

; Save registers prior to calling a handler function.
; This must be kept up to date with:
;   - Interrupt_State struct in int.h
;   - Setup_Kernel_Thread(), Setup_User_Thread() in kthread.c
;   - REG_SKIP, EFLAGS_SKIP below (count of registers pushed * 4) 
;   - INTERRUPT_STATE_SIZE above (count of registers pushed * 4) 

%macro Save_Registers 0
	push	eax
	push	ebx
	push	ecx
	push	edx
	push	esi
	push	edi
	push	ebp
	push	ds
	push	es
	push	fs
	push	gs                  
%endmacro

; Restore registers and clean up the stack after calling a handler function
; (i.e., just before we return from the interrupt via an iret instruction).
%macro Restore_Registers 0
	pop	gs
	pop	fs
	pop	es
	pop	ds
	pop	ebp
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	eax
	add	esp, 8	; skip int num and error code
%endmacro

; Code to activate a new user context (if necessary), before returning
; to executing a thread.  Should be called just before restoring
; registers (because the interrupt context is used).
%macro Activate_User_Context 0
	; If the new thread has a user context which is not the current
	; one, activate it.
	push    esp                     ; Interrupt_State pointer
	Push_Current_Thread_PTR
	call    Switch_To_User_Context
	add     esp, 8                  ; clear 2 arguments
%endmacro

; Code to rearrange the stack to activate a signal handler
%macro Process_Signal 1
	; Check if a signal is pending.  If so, we need to
	; rearrange the stack so that when the thread restarts
	; it will enter the signal handler and then afterward
	; return to its original spot
	push	esp
	Push_Current_Thread_PTR
	call	Check_Pending_Signal
	add	esp, 8
	cmp	eax, dword 0	
	je	%1
	; We have a pending signal, so we must arrange to
	; call its signal handler
; 	push	esp
; 	call	Print_IS
; 	add	esp, 4
 	push	esp
	Push_Current_Thread_PTR
 	call	Setup_Frame
 	add	esp, 8
; 	push	esp
; 	call	Print_IS
; 	add	esp, 4
%endmacro
	
; Number of bytes between the top of the stack and
; the interrupt number after the general-purpose and segment
; registers have been saved.
REG_SKIP equ 	(11*4)          

; Number of bytes from top of the stack the the saved EFLAGS after 
; GP and seg registers have been saved
EFLAGS_SKIP equ	(REG_SKIP + (4*4))

; bit mask for Interrupt enable bit of EFLAGS
EFLAGS_IF equ	word 0x200

; Template for entry point code for interrupts that have
; an explicit processor-generated error code.
; The argument is the interrupt number.
%macro Int_With_Err 1
align 16
	push	dword %1	; push interrupt number
	jmp	Handle_Interrupt ; jump to common handler
%endmacro

; Template for entry point code for interrupts that do not
; generate an explicit error code.  We push a dummy error
; code on the stack, so the stack layout is the same
; for all interrupts.
%macro Int_No_Err 1
align 16
	push	dword 0		; fake error code
	push	dword %1	; push interrupt number
	jmp	Handle_Interrupt ; jump to common handler
%endmacro


; ----------------------------------------------------------------------
; Symbol imports and exports
; ----------------------------------------------------------------------

IMPORT unlockKernel
IMPORT lockKernel
IMPORT kthreadLock              ; to avoid preemption when the kthread spinlock is held.

; This symbol is defined in idt.c, and is a table of addresses
; of C handler functions for interrupts.
IMPORT g_interruptTable

; Global variable pointing to context struct for current thread.
IMPORT g_currentThreads

; Set to non-zero when we need to choose a new thread
; in the interrupt return code.
IMPORT g_needReschedule

; Set to non-zero when preemption is disabled.
IMPORT g_preemptionDisabled

; This is the function that returns the next runnable thread.
IMPORT Get_Next_Runnable

; Function to put a thread on the run queue.
IMPORT Make_Runnable

; Function to activate a new user context (if needed).
IMPORT Switch_To_User_Context

; Function that checks if the current thread has a signal pending.
IMPORT Check_Pending_Signal

; Function that sets up the stack frame to invoke a signal handler
IMPORT Setup_Frame

;; Function to manipulate APIC
IMPORT APIC_Write

; In case of error, just terminate.
IMPORT Hardware_Shutdown

; Sizes of interrupt handler entry points for interrupts with
; and without error codes.  The code in idt.c uses this
; information to infer the layout of the table of interrupt
; handler entry points, without needing a separate linker
; symbol for each one (which is quite tedious to type :-)
EXPORT g_handlerSizeNoErr
EXPORT g_handlerSizeErr

; Simple functions to load the IDTR, GDTR, and LDTR.
EXPORT Load_IDTR
EXPORT Load_GDTR
EXPORT Load_LDTR

; Beginning and end of the table of interrupt entry points.
EXPORT g_entryPointTableStart
EXPORT g_entryPointTableEnd

; Thread context switch function.
EXPORT Switch_To_Thread

; Return current value of eflags register.
EXPORT Get_Current_EFLAGS

; Virtual memory support.
EXPORT Enable_Paging
EXPORT Set_PDBR
EXPORT Get_PDBR
EXPORT Flush_TLB

; Spin Lock
EXPORT Spin_Lock_INTERNAL
EXPORT Spin_Unlock_INTERNAL
EXPORT Send_Timer_INT

; ----------------------------------------------------------------------
; Code
; ----------------------------------------------------------------------

[SECTION .text]

; Load IDTR with 6-byte pointer whose address is passed as
; the parameter.
align 8
Load_IDTR:
	mov	eax, [esp+4]
	lidt	[eax]
	ret

;  Load the GDTR with 6-byte pointer whose address is
; passed as the parameter.  Assumes that interrupts
; are disabled.
align 8
Load_GDTR:
	mov	eax, [esp+4]
	lgdt	[eax]
	; Reload segment registers
	mov	ax, KERNEL_DS
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax
	jmp	KERNEL_CS:.here
.here:
	ret

; Load the LDT whose selector is passed as a parameter.
align 8
Load_LDTR:
	mov	eax, [esp+4]
	lldt	ax
	ret

;
; Start paging
;	load crt3 with the passed page directory pointer
;	enable paging bit in cr2
align 8
Enable_Paging:
	mov	eax, [esp+4]
	mov	cr3, eax
	mov	eax, cr3
	mov	cr3, eax
	mov	ebx, cr0
	or	ebx, 0x80000000
	mov	cr0, ebx
	ret


;
; Change PDBR 
;	load cr3 with the passed page directory pointer
align 8
Set_PDBR:
	mov	eax, [esp+4]
	mov	cr3, eax
	; mov	eax, [esp+4]
	; mov	cr3, eax
	ret

;
; Get the current PDBR.
; This is useful for lazily switching address spaces;
; only switch if the new thread has a different address space.
;
align 8
Get_PDBR:
	mov	eax, cr3
	ret

;
; Flush TLB - just need to re-load cr3 to force this to happen
;
; - XXXX do we need to shoot down the TLBs on other cores with an IPI??
;   - might need to just make sure we don't take pages from other processes, since the TLB clearing is currently
;     only used to clear the TLB when we take page out a page from a process, need to ensure the TLB entry (if any) 
;     for it is cleared.  When we start to have shared memory between cores, will need to look at a full shootdown
;     strategy.
;
align 8
Flush_TLB:
	mov	eax, cr3
	mov	cr3, eax
	ret


APIC_BASE	equ	0xFEE00000
APIC_ID		equ	0x20

;;
;; eax = &g_currentThreads[Get_CPU_ID()];
;;
%macro Mov_EAX_Current_Thread_PTR 0
	mov	eax, [APIC_BASE+APIC_ID]		;; load id of local APC (which is cpuid)
	shr	eax, 24-2				;; id is in high 24 bits of register, but need id <<2
	add	eax, g_currentThreads
%endmacro 

;; eax = *( &g_currentThreads[Get_CPU_ID()] )
%macro Get_Current_Thread_To_EAX 0
	Mov_EAX_Current_Thread_PTR
	mov	eax, [eax]              
%endmacro
%macro Set_Current_Thread_From_EBX 0
	Mov_EAX_Current_Thread_PTR
	mov	[eax], ebx
%endmacro
%macro Push_Current_Thread_PTR 0
	Mov_EAX_Current_Thread_PTR
	push	dword [eax]
%endmacro

%include "percpu.asm"

; Common interrupt handling code.
; Save registers, call C handler function,
; possibly choose a new thread to run, restore
; registers, return from the interrupt.
align 8
Handle_Interrupt:
	; Save registers (general purpose and segment)
	Save_Registers



	; Ensure that we're using the kernel data segment
	mov	ax, KERNEL_DS
	mov	ds, ax
	mov	es, ax

	; get the kernel lock if interrupts were on before int
 	test	word [esp+EFLAGS_SKIP], EFLAGS_IF
	jz	.skipLock
	call	lockKernel
.skipLock:

	; Get the address of the C handler function from the
	; table of handler functions.
	mov	eax, g_interruptTable	; get address of handler table
	mov	esi, [esp+REG_SKIP]	; get interrupt number
	mov	ebx, [eax+esi*4]	; get address of handler function
    
    test ebx,ebx
    jz .bail_no_handler

	; Call the handler.
	; The argument passed is a pointer to an Interrupt_State struct,
	; which describes the stack layout for all interrupts.
	push	esp
	call	ebx
	add	esp, 4			; clear 1 argument

	; If preemption is disabled, then the current thread
	; keeps running.

	mov	ebx, [APIC_BASE+APIC_ID]		;; load id of local APC (which is cpuid)
	shr	ebx, 24-2				;; id is in high 24 bits of register, but need id <<2
	cmp	[g_preemptionDisabled+ebx], dword 0
	jne	.tramp_restore

        ;;  nspring - check if kthreadLock is; if so, skip preemption.
        ;;  this is a hack.  it can help, but is not reliable (we are
        ;;  not acquiring the lock, but another thread might.
	mov	ebx, [kthreadLock]		;; the lock value at the front of the spinlock.
	jne	.tramp_restore

	; See if we need to choose a new thread to run.
	mov	ebx, [APIC_BASE+APIC_ID]		;; load id of local APC (which is cpuid)
	shr	ebx, 24-2				;; id is in high 24 bits of register, but need id <<2
	cmp	[g_needReschedule+ebx], dword 0
	je	.tramp_restore

	; Put current thread back on the run queue
	Push_Current_Thread_PTR
	call	Make_Runnable
	add	esp, 4			; clear 1 argument

	; Save stack pointer in current thread context, and
	; clear numTicks field.
    Get_Current_Thread_To_EAX
    test eax,eax
    jne .ok 
    jmp .bail_null_current_thread
.tramp_restore: 
    jmp .restore
.bail_no_handler:
    call Hardware_Shutdown
.ok:
	mov	[eax+0], esp		; esp field
	mov	[eax+4], dword 0	; numTicks field

	; Pick a new thread to run, and switch to its stack
	call	Get_Next_Runnable
	mov	ebx, eax               ; save new thread into ebx
    test eax, eax              ; possibly redundant setting of the flags.
    jne .ok2
    jmp .bail_null_runnable_thread
.ok2:
    Set_Current_Thread_From_EBX
	mov	esp, [ebx+0]		   ; load esp from new thread

	; Clear "need reschedule" flag
	mov	ebx, [APIC_BASE+APIC_ID]		;; load id of local APC (which is cpuid)
	shr	ebx, 24-2				;; id is in high 24 bits of register, but need id <<2
	mov	[g_needReschedule+ebx], dword 0

.restore:
	; Activate the user context, if necessary.
	Activate_User_Context

	Process_Signal .finish
.finish:	

	; clear APIC Interrupt info
	mov	[APIC_BASE+APIC_EOI], dword 0

	; releasee the kernel lock if interrupts will be re-enabled
 	test	word [esp+EFLAGS_SKIP], EFLAGS_IF
	jz	.skipUnlock
	call	unlockKernel
.skipUnlock:
        mov eax, esp            ; debug ns

	; Restore registers
	Restore_Registers

	; Return from the interrupt.
	iret
.bail_null_current_thread:
    call Hardware_Shutdown
.bail_null_runnable_thread:
    call Hardware_Shutdown


; ----------------------------------------------------------------------
; Switch_To_Thread()
;   Save context of currently executing thread, and activate
;   the thread whose context object is passed as a parameter.
; 
; Parameter: 
;   - ptr to Kernel_Thread whose state should be restored and made active
;
; Notes:
; Called with interrupts disabled.
; This must be kept up to date with definition of Kernel_Thread
; struct, in kthread.h.
; ----------------------------------------------------------------------
align 16
Switch_To_Thread:
	; Modify the stack to allow a later return via an iret instruction.
	; We start with a stack that looks like this:
	;
	;            thread_ptr
	;    esp --> return addr
	;
	; We change it to look like this:
	;
	;            thread_ptr
	;            eflags
	;            cs
	;    esp --> return addr

	push	eax		; save eax
	mov	eax, [esp+4]	; get return address
	mov	[esp-4], eax	; move return addr down 8 bytes from orig loc
	add	esp, 8		; move stack ptr up
	pushfd			; put eflags where return address was
	mov	eax, [esp-4]	; restore saved value of eax
	push	dword KERNEL_CS	; push cs selector
	sub	esp, 4		; point stack ptr at return address

	; Push fake error code and interrupt number
	push	dword 0
	push	dword 0

	; Save general purpose registers.
	Save_Registers

	; get the kernel lock if interrupts were on before int
 	test	word [esp+EFLAGS_SKIP], EFLAGS_IF
	jz	.skipLock
	call	lockKernel
.skipLock:

	; Save stack pointer in the thread context struct (at offset 0).
    Get_Current_Thread_To_EAX
	mov	[eax+0], esp

	; Clear numTicks field in thread context, since this
	; thread is being suspended.
	mov	[eax+4], dword 0

	; Load the pointer to the new thread context into eax.
	; We skip over the Interrupt_State struct on the stack to
	; get the parameter.
	mov	eax, [esp+INTERRUPT_STATE_SIZE]

	; Make the new thread current, and switch to its stack.
	mov	ebx, eax
    Set_Current_Thread_From_EBX
	mov	esp, [ebx+0]

	; Activate the user context, if necessary.
	Activate_User_Context

	; clear APIC Interrupt info
	mov	[APIC_BASE+APIC_EOI], dword 0

	Process_Signal .complete
.complete:	

	; release the kernel lock, if enabling interrupts on iret
 	test	word [esp+EFLAGS_SKIP], EFLAGS_IF
	jz	.skipUnlock
	call	unlockKernel
.skipUnlock:
        mov eax, esp            ; debug ns; get esp into a register that is dumped if there's a trap.

	; Restore general purpose and segment registers, and clear interrupt
	; number and error code.
	Restore_Registers

	; We'll return to the place where the thread was
	; executing last.
	iret

; Return current contents of eflags register.
align 16
Get_Current_EFLAGS:
	pushfd			; push eflags
	pop	eax		; pop contents into eax
	ret

lockops:                      
     dd      0

        ;; nspring - ecx is caller save, ebx is callee save. 
        ;; http://www.cs.virginia.edu/~evans/cs216/guides/x86.html
Spin_Lock_INTERNAL:
     mov     ecx, [esp+4]

.still_locked_early:
     mov eax, [ecx]
     test eax, eax
     jnz .still_locked_early
        
.seems_unlocked:
     mov     eax, 1          
     lock xchg    eax, [ecx]   
     test    eax, eax        
     jnz     Spin_Lock_INTERNAL       
     inc     dword [lockops]
     ret                     
                             
        ;; nspring - ecx is caller save, ebx is callee save. 
        ;; http://www.cs.virginia.edu/~evans/cs216/guides/x86.html
Spin_Unlock_INTERNAL:
     mov     ecx, [esp+4]
     mov     eax, 0          
     xchg    eax, [ecx]   
     ret            

Send_Timer_INT:
	int	0h
	ret

; ----------------------------------------------------------------------
; Generate interrupt-specific entry points for all interrupts.
; We also define symbols to indicate the extend of the table
; of entry points, and the size of individual entry points.
; ----------------------------------------------------------------------
align 16
g_entryPointTableStart:

; Handlers for processor-generated exceptions, as defined by
; Intel 486 manual.
Int_No_Err 0
align 16
Before_No_Err:
Int_No_Err 1
align 16
After_No_Err:
Int_No_Err 2	; FIXME: not described in 486 manual
Int_No_Err 3
Int_No_Err 4
Int_No_Err 5
Int_No_Err 6
Int_No_Err 7
align 16
Before_Err:
Int_With_Err 8
align 16
After_Err:
Int_No_Err 9	; FIXME: not described in 486 manual
Int_With_Err 10
Int_With_Err 11
Int_With_Err 12
Int_With_Err 13
Int_With_Err 14
Int_No_Err 15	; FIXME: not described in 486 manual
Int_No_Err 16
Int_With_Err 17

; The remaining interrupts (18 - 255) do not have error codes.
; We can generate them all in one go with nasm's %rep construct.
%assign intNum 18
%rep (256 - 18)
Int_No_Err intNum
%assign intNum intNum+1
%endrep

align 16
g_entryPointTableEnd:

[SECTION .data]

; Exported symbols defining the size of handler entry points
; (both with and without error codes).
align 4
g_handlerSizeNoErr: dd (After_No_Err - Before_No_Err)
align 4
g_handlerSizeErr: dd (After_Err - Before_Err)
