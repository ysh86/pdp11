#pragma once

struct cpu_tag;
#ifndef _CPU_T_
#define _CPU_T_
typedef struct cpu_tag cpu_t;
#endif

struct instruction_tag {
    char *mnemonic;
    int operandNum; // 0(setd only), 1, 2, 3, 4, 5(conditional branch), 6(syscall), 7(sob only), 8(mul,div,ash,ashc)
    void (*exec)(cpu_t *);
};
#ifndef _INSTRUCTION_T_
#define _INSTRUCTION_T_
typedef struct instruction_tag instruction_t;
#endif

extern instruction_t doubleOperand0[];
extern instruction_t doubleOperand1[];
extern instruction_t singleOperand0[];
extern instruction_t singleOperand1[];
extern instruction_t conditionalBranch0[];
extern instruction_t conditionalBranch1[];
extern instruction_t systemMisc[];
extern instruction_t floatingPoint0[];
extern instruction_t floatingPoint1[];
extern instruction_t clearSet[];
