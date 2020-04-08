;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 2 starter file
; February 10, 2016
; Implemented OS by Abdulmaguid Eissa, April 1, 2020


        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXPORT  StartOS
        EXPORT  SysTick_Handler
        IMPORT  Scheduler

; ************************************************************************
; SysTick_Handler
; Context Switcher in assembly for high performance.
; 1. Disable interrupts
; 2. Save the current context by pushing the 8 registers R4-R11
; 3. Load the RunPtr to SP
; 4. Skip to the next thread, RunPtr = RunPtr->next
; 5. Restore the 8 registers R4-R11
; 6. Enable interrupts back
; 7. Branch to LR, CPU restores the remaining 8 registers automatically

SysTick_Handler                ; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                  ; 2) Prevent interrupt during switch
    PUSH    {R4-R11}           ; 3) Save the current context, remaining registers R4-R11
    LDR     R0, =RunPt        ; 4) Load the current thread, R0 = RunPtr
    LDR     R1, [R0]           ;    R1 = RunPtr
    STR     SP, [R1]           ; 5) Save SP into TCB
    ;LDR     R1, [R1, #4]       ; 6) R1 = RunPtr->next
    ;STR     R1, [R0]           ;    RunPtr = R1
    PUSH    {R0, LR}           ; R0 and LR must be saved before branching to Scheduler written in C.
    BL      Scheduler          ; Extending the capabilities of scheduler using C.
    POP     {R0, LR}
    LDR     R1, [R0]
    LDR     SP, [R1]           ; 7) SP = RunPtr->sp
    POP     {R4-R11}           ; 8) Restore the 8 registers, R4-R11
    CPSIE   I                  ; 9) tasks run with interrupts enabled
    BX      LR                 ; 10) restore R0-R3,R12,LR,PC,PSR

; ************************************************************************
; StartOS
; Setting the SP to the value of the first thread.
; Pulling all registers off the stack explicitly.
; The stack is set as if it had been running previously, was interrupted
; - 8 registers were pushed - and suspended - another 8 registers pushed.
; When launch the first thread for the first time we do not execute a 
; return from interrupt (we just pull 16 registers from its stack). 
;Thus, the state of the thread is initialized and is now ready to run.

StartOS
    LDR     R0, =RunPt        ; Currently running thread
    LDR     R2, [R0]           ; R1 has the value of RunPtr
    LDR     SP, [R2]           ; SP = RunPtr->sp
    POP     {R4-R11}           ; Restore registers R4-R11
    POP     {R0-R3}
    POP     {R12}
    ADD     SP, SP, #4         ; Discard LR from initial stack
    POP     {LR}               ; Start location
    ADD     SP, SP, #4         ; Discard PSR
    CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread

    ALIGN
    END
