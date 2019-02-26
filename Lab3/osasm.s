;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 3 starter file
; March 2, 2016




        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXPORT  StartOS
        EXPORT  SysTick_Handler
        IMPORT  Scheduler


SysTick_Handler                ; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                  ; 2) Prevent interrupt during switch
    PUSH    {R4-R11}		   ; 3) Save the remaining registers
	LDR     R0, =RunPt         ; 4) R0 points to the current thread
	LDR     R1, [R0]           ;     R1 -> RunPt
	STR     SP, [R1]           ; 5) Save stack pointer into TCB
	 	;LDR     R1, [R1, #4]	   ; R1 = RunPt->next
	    ;STR     R1, [R0]           ; RunPt = R1
	PUSH    {R0, LR}           ;     
	BL      Scheduler          ;     Call outside c function (Scheduler)
	POP     {R0, LR}
	LDR     R1, [R0]           ; 6) R1 = RunPt new thread
	LDR     SP, [R1]           ; 7) SP -> new thread to launch, SP = RunPt->sp
	POP     {R4-R11}           ; 8) Pull out the 8 registers from the stack
    CPSIE   I                  ; 9) tasks run with interrupts enabled
    BX      LR                 ; 10) restore R0-R3,R12,LR,PC,PSR

StartOS
    LDR     R0, =RunPt         ; LOAD R0 WITH THE CURRENT RUNNUNG THREAD pointer
	LDR     R2, [R0]
	LDR     SP, [R2]           ; NOW THE ACTUAL SP POINTS TO THREAD
	POP     {R4-R11}
	POP     {R0-R3}
	POP     {R12}
	ADD     SP, SP, #4	       ; DISCARD LR FROM INITIAL STACK
	POP     {LR}
	ADD     SP, SP, #4         ; DISCARD PSR
    CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread

    ALIGN
    END
