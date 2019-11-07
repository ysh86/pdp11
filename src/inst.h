#pragma once

#include <stdlib.h>

struct machine_tag;
#ifndef _MACHINE_T_
#define _MACHINE_T_
typedef struct machine_tag machine_t;
#endif

struct instruction_tag {
    char *mnemonic;
    int operandNum; // 0(setd only), 1, 2, 3, 4, 5(conditional branch), 6(syscall), 7(sob only), 8(mul,div,ash,ashc)
    void (*exec)(machine_t *);
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

void exec(machine_t *pm);

void nop(machine_t *pm);

void mov(machine_t *pm);
void cmp(machine_t *pm);
void bit(machine_t *pm);
void bic(machine_t *pm);
void bis(machine_t *pm);
void add(machine_t *pm);
void sub(machine_t *pm);
void mul(machine_t *pm);
void mydiv(machine_t *pm);
void ash(machine_t *pm);
void ashc(machine_t *pm);
void xor(machine_t *pm);
void sob(machine_t *pm);

void br(machine_t *pm);
void bne(machine_t *pm);
void beq(machine_t *pm);
void bge(machine_t *pm);
void blt(machine_t *pm);
void bgt(machine_t *pm);
void ble(machine_t *pm);
void bpl(machine_t *pm);
void bmi(machine_t *pm);
void bhi(machine_t *pm);
void blos(machine_t *pm);
void bvc(machine_t *pm);
void bvs(machine_t *pm);
void bcc(machine_t *pm);
void bcs(machine_t *pm);

void clr(machine_t *pm);
void com(machine_t *pm);
void inc(machine_t *pm);
void dec(machine_t *pm);
void neg(machine_t *pm);
void adc(machine_t *pm);
void sbc(machine_t *pm);
void tst(machine_t *pm);

void jmp(machine_t *pm);
void jsr(machine_t *pm);
void rts(machine_t *pm);

void sys(machine_t *pm);
