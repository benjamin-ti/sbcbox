#define push(a...) stmdb r13!,{a}
#define pop(a...) ldmia r13!,{a}
#define PRINTK(STR)\
    push(r3,r4,r5,r14); \
    adr r4,1f; \
    bl  debugprint; \
    bl  2f; \
    1: .ASCIZ STR; \
    .ALIGN 4; \
    2: pop(r14,r5,r4,r3)
#define SERIAL0_BASE 0x10009000
#define SERIAL0_FLAG 0x10009018


.text
PRINTK("TEST\r\n")
1: b 1b

debugprint:
    ldr     r3,=SERIAL0_FLAG
1:  ldr     r5,[r3]
    ands    r5,#(1<<5)
    bne     1b
    ldr     r3,=SERIAL0_BASE
    ldrb    r5,[r4],#1
    cmp     r5,#0
    moveq   r15,r14
    str     r5,[r3]
    b       debugprint
