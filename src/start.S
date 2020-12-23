.section .init
.global _start
.global _stack          // top of stack
.global __bss_start     // -> .bss area in RAM
.global __bss_end__     // end of .bss area

_start:
    ldr sp, =_stack  //@ Set up the stack

// Enable Floating point unit
// --------------------------
    mrc p15, 0, r0, c1, c0, #2      // R0 = Access Control Register
    orr r0, #(0x300000 + 0xC00000)  // Enable Single & Double Precision
    mcr p15,0,r0,c1,c0, #2          // Access Control Register = R0
    mov r0, #0x40000000             // R0 = Enable VFP
    vmsr fpexc, r0                  // FPEXC = R0

// Clear .bss
// ----------
    mov   r0,#0                     // get a zero
    ldr   r1,=__bss_start           // -> bss start
    ldr   r2,=__bss_end__           // -> bss end
2:  cmp   r1,r2                     // check if data to clear
    strlo r0,[r1],#4                // clear 4 bytes
    blo   2b                        // loop until done

// Call main with minimum setup
// ----------------------------
    mov   r0,#0                     // no arguments (argc = 0)
    mov   r1,r0
    mov   r2,r0
    mov   fp,r0                     // null frame pointer
    b main

/*
* This command tells the assembler to put this code with the rest.
*/
.section .text

