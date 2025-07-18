#include "autoconf.h"
#include "asm/asmdefs.h"
#include "asm/processor.h"


#define ULONG_SIZE 4
#define STACKFRAME_MINSIZE 16
#define STKOFF 8
#define SAVE_SPACE 144

/*
 * Switch execution context
 * This saves registers in the stack, then
 * switches the stack, and restores everything from the new stack.
 * This function takes no argument. New stack pointer is
 * taken from global variable __context, and old stack pointer
 * is also saved to __context. This way we can just jump to
 * this routine to get back to the original context.
 */

_GLOBAL(__switch_context):
    /* save internal stack pointer */
    PPC_STL  r1, -SAVE_SPACE + STKOFF(r1)
    PPC_STLU r1, -SAVE_SPACE(r1) /* fits within alignment */

    /* r4, r5 */
    PPC_STL r4, (STKOFF + 9 * ULONG_SIZE)(r1)
    PPC_STL r5, (STKOFF + 10 * ULONG_SIZE)(r1)

    /* link register */
    mflr	r4
    PPC_STL  r4, PPC_LR_STKOFF(r1)
    PPC_STL  r4,  (STKOFF + ULONG_SIZE)(r1)

    PPC_STL  r3,  (STKOFF + 5 * ULONG_SIZE)(r1)
    PPC_STL  r2,  (STKOFF + 4 * ULONG_SIZE)(r1)
    PPC_STL  r0,  (STKOFF + 3 * ULONG_SIZE)(r1)

    /* ctr, cr and xer */
    mfctr	r4
    PPC_STL  r4,  (STKOFF + 6 * ULONG_SIZE)(r1)
    mfcr	r4
    PPC_STL  r4,  (STKOFF + 7 * ULONG_SIZE)(r1)
    mfxer	r4
    PPC_STL  r4,  (STKOFF + 8 * ULONG_SIZE)(r1)

    /* r6-r31 */
    PPC_STL  r6,  (STKOFF + 11 * ULONG_SIZE)(r1)
    PPC_STL  r7,  (STKOFF + 12 * ULONG_SIZE)(r1)
    PPC_STL  r8,  (STKOFF + 13 * ULONG_SIZE)(r1)
    PPC_STL  r9,  (STKOFF + 14 * ULONG_SIZE)(r1)
    PPC_STL  r10,  (STKOFF + 15 * ULONG_SIZE)(r1)
    PPC_STL  r11,  (STKOFF + 16 * ULONG_SIZE)(r1)
    PPC_STL  r12,  (STKOFF + 17 * ULONG_SIZE)(r1)
    PPC_STL  r13,  (STKOFF + 18 * ULONG_SIZE)(r1)
    PPC_STL  r14,  (STKOFF + 19 * ULONG_SIZE)(r1)
    PPC_STL  r15,  (STKOFF + 20 * ULONG_SIZE)(r1)
    PPC_STL  r16,  (STKOFF + 21 * ULONG_SIZE)(r1)
    PPC_STL  r17,  (STKOFF + 22 * ULONG_SIZE)(r1)
    PPC_STL  r18,  (STKOFF + 23 * ULONG_SIZE)(r1)
    PPC_STL  r19,  (STKOFF + 24 * ULONG_SIZE)(r1)
    PPC_STL  r20,  (STKOFF + 25 * ULONG_SIZE)(r1)
    PPC_STL  r21,  (STKOFF + 26 * ULONG_SIZE)(r1)
    PPC_STL  r22,  (STKOFF + 27 * ULONG_SIZE)(r1)
    PPC_STL  r23,  (STKOFF + 28 * ULONG_SIZE)(r1)
    PPC_STL  r24,  (STKOFF + 29 * ULONG_SIZE)(r1)
    PPC_STL  r25,  (STKOFF + 30 * ULONG_SIZE)(r1)
    PPC_STL  r26,  (STKOFF + 31 * ULONG_SIZE)(r1)
    PPC_STL  r27,  (STKOFF + 32 * ULONG_SIZE)(r1)
    PPC_STL  r28,  (STKOFF + 33 * ULONG_SIZE)(r1)
    PPC_STL  r29,  (STKOFF + 34 * ULONG_SIZE)(r1)
    PPC_STL  r30,  (STKOFF + 35 * ULONG_SIZE)(r1)
    PPC_STL  r31,  (STKOFF + 36 * ULONG_SIZE)(r1)

    /* swap context */
    LOAD_REG_IMMEDIATE(r4, __context)
    PPC_LL   r5, 0(r4)
    PPC_STL  r1, 0(r4)
    mr	r4, r5

    b	__set_context

_GLOBAL(__switch_context_nosave):
    LOAD_REG_IMMEDIATE(r4, __context)
    PPC_LL  r4, 0(r4)

__set_context:
    /* link register */
    PPC_LL  r5, (STKOFF + ULONG_SIZE)(r4)
    mtlr	r5

    PPC_LL r3,  (STKOFF + 5 * ULONG_SIZE)(r4)
    PPC_LL r2,  (STKOFF + 4 * ULONG_SIZE)(r4)
    PPC_LL r0,  (STKOFF + 3 * ULONG_SIZE)(r4)

    /* ctr, cr and xer */
    PPC_LL r5,  (STKOFF + 6 * ULONG_SIZE)(r4)
    mtctr	r5
    PPC_LL r5,  (STKOFF + 7 * ULONG_SIZE)(r4)
    mtcr	r5
    PPC_LL r5,  (STKOFF + 8 * ULONG_SIZE)(r4)
    mtxer	r5

    /* r5-r31 */
    PPC_LL  r5,  (STKOFF + 10 * ULONG_SIZE)(r4)
    PPC_LL  r6,  (STKOFF + 11 * ULONG_SIZE)(r4)
    PPC_LL  r7,  (STKOFF + 12 * ULONG_SIZE)(r4)
    PPC_LL  r8,  (STKOFF + 13 * ULONG_SIZE)(r4)
    PPC_LL  r9,  (STKOFF + 14 * ULONG_SIZE)(r4)
    PPC_LL  r10,  (STKOFF + 15 * ULONG_SIZE)(r4)
    PPC_LL  r11,  (STKOFF + 16 * ULONG_SIZE)(r4)
    PPC_LL  r12,  (STKOFF + 17 * ULONG_SIZE)(r4)
    PPC_LL  r13,  (STKOFF + 18 * ULONG_SIZE)(r4)
    PPC_LL  r14,  (STKOFF + 19 * ULONG_SIZE)(r4)
    PPC_LL  r15,  (STKOFF + 20 * ULONG_SIZE)(r4)
    PPC_LL  r16,  (STKOFF + 21 * ULONG_SIZE)(r4)
    PPC_LL  r17,  (STKOFF + 22 * ULONG_SIZE)(r4)
    PPC_LL  r18,  (STKOFF + 23 * ULONG_SIZE)(r4)
    PPC_LL  r19,  (STKOFF + 24 * ULONG_SIZE)(r4)
    PPC_LL  r20,  (STKOFF + 25 * ULONG_SIZE)(r4)
    PPC_LL  r21,  (STKOFF + 26 * ULONG_SIZE)(r4)
    PPC_LL  r22,  (STKOFF + 27 * ULONG_SIZE)(r4)
    PPC_LL  r23,  (STKOFF + 28 * ULONG_SIZE)(r4)
    PPC_LL  r24,  (STKOFF + 29 * ULONG_SIZE)(r4)
    PPC_LL  r25,  (STKOFF + 30 * ULONG_SIZE)(r4)
    PPC_LL  r26,  (STKOFF + 31 * ULONG_SIZE)(r4)
    PPC_LL  r27,  (STKOFF + 32 * ULONG_SIZE)(r4)
    PPC_LL  r28,  (STKOFF + 33 * ULONG_SIZE)(r4)
    PPC_LL  r29,  (STKOFF + 34 * ULONG_SIZE)(r4)
    PPC_LL  r30,  (STKOFF + 35 * ULONG_SIZE)(r4)
    PPC_LL  r31,  (STKOFF + 36 * ULONG_SIZE)(r4)

    /* r4, r1 */
    PPC_LL  r1,  STKOFF(r4)
    PPC_LL  r4,  (STKOFF + 9 * ULONG_SIZE)(r4)

    LOAD_REG_IMMEDIATE(r0, MSR_FP | MSR_ME | MSR_DR | MSR_IR)
    MTMSRD(r0)

    blrl

_GLOBAL(__exit_context):
    /* Get back to the original context */
    b	__switch_context
