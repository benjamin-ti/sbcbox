#ifndef MAKRO_H_
#define MAKRO_H_

.extern debugprint

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

#endif /* MAKRO_H_ */
