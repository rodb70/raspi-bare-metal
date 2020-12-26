.section .init
.global _start
.global _halt
.global _stack          // top of stack
.global __bss_start     // -> .bss area in RAM
.global __bss_end__     // end of .bss area

_start:
    ldr sp, =_stack  //@ Set up the stack

/* Enable Floating point unit
 * --------------------------
 */
    /*  Read Coprocessor Access Control Register */
    mrc p15, 0, r0, c1, c0, #2
    /* CP10 & CP11 co-processors enable */
    orr r0, #(0x300000 + 0xC00000)  /* Enable Single & Double Precision */
    /*  Write Coprocessor Access Control Register */
    mcr p15,0,r0,c1,c0, #2          /* Access Control Register = R0 */
    mov r0, #0x40000000             /* R0 = Enable VFP */
    vmsr fpexc, r0                  /* FPEXC = R0 */

/* set the cpu to SVC32 mode
 * ----------
 */
    mrs r0, cpsr
    bic r0, r0, #0x3f
    orr r0, r0, #0xd3
    msr cpsr, r0

/* Clear .bss
 * ----------
 */
    mov   r0,#0                     /* get a zero */
    ldr   r1,=__bss_start           /* -> bss start */
    ldr   r2,=__bss_end__           /* -> bss end */
2:  cmp   r1,r2                     /* check if data to clear */
    strlo r0,[r1],#4                /* clear 4 bytes */
    blo   2b                        /* loop until done */

/* Call main with minimum setup
 * ----------------------------
 */
    mov   r0,#0                     /* no arguments (argc = 0) */
    mov   r1,r0                     /* No argument values */
    mov   r2,r0                     /* No environment vars */
    mov   fp,r0                     /* null frame pointer */
    b main
_halt:
    b .                             /* If main ever returns stop here */

/* MMU enable routine
 * --------------------------
 */
#define SCTLR_ICACHE_ENABLE             (1<<12)
#define SCTLR_BRANCH_PREDICTOR_ENABLE   (1<<11)
#define SCTLR_CACHE_ENABLE              (1<<2)
#define SCTLR_MMU_ENABLE                (1<<0)

/*
 * This command tells the assembler to put this code with the rest.
 */
.section .text

/*
/* Provide "C" function: void EnableMmuTables ( void* map1to1, void* virtualmap);
 */
.section .text.EnableMmuTables, "ax", %progbits
.balign 4
.globl EnableMmuTables;
.type EnableMmuTables, %function
EnableMmuTables:
    /* Write Translation Table Base Register 0 */
    mcr p15, 0, r0, c2, c0, 0                   /* Set map1to1 to TBBR0 */
    /* Write Translation Table Base Register 1 */
    mcr p15, 0, r1, c2, c0, 1                   /* Write virtualmap to TTBR1 */

    /* Read Translation Table Base Control Register */
    mrc p15, 0, r0, c2, c0, 2                   /* Read TTBCR into R0 */
    orr r0, r0, #1                              /* Set N = 1 */
    /* Write Translation Table Base Control Register */
    mcr p15, 0, r0, c2, c0, 2                   /* Write R0 to TTBCR */

    /* Set the access control to all-supervisor */
    mov r0, #~0
    /* Write Domain Access Control Register */
    mcr p15, 0, r0, c3, c0, 0

    /* Read Auxiliary Control Register */
    mrc p15, 0, r0, c1, c0,  1
    /* Set CZ bit, restrict cache size to 16KB */
    orr r0, r0, #(1 << 6)
    /* Write Auxiliary Control Register */
    mcr p15, 0, r0, c1, c0,  1

    /* Accesses are checked against the access permission bits in the TLB entry. */
    mov r0, #1
    /* Write Domain Access Control Register */
    mcr p15, 0, r0, c3, c0,  0

    /* Read Control Register configuration data */
    mrc p15, 0,  r0, c1, c0, 0
    /* Enable MMU ! */
    orr r0, r0, #SCTLR_MMU_ENABLE
    /* Enable branch predictor */
    orr r0, r0, #SCTLR_BRANCH_PREDICTOR_ENABLE
    /* Enable caches! */
    orr r0, r0, #SCTLR_CACHE_ENABLE
    orr r0, r0, #SCTLR_ICACHE_ENABLE
    /* Write Control Register configuration data */
    mcr p15, 0, r0, c1, c0, 0
/* barrier
 * ----------------------------
 */
    mov r0, #0
    /* Invalidate Entire Instruction Cache */
    mcr p15, 0, r0, c7, c5, 0
    /* Flush Entire Branch Target Cache */
    mcr p15, 0, r0, c7, c5, 6
    /* drain write buffer */
    mcr p15, 0, r0, c7, c10, 4
    /* Flush Prefetch Buffer */
    mcr p15, 0, r0, c7, c5, 4
    mov pc, lr
    bx lr

