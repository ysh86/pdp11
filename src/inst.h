#pragma once

#include <stdlib.h>

struct machine_tag;
typedef struct machine_tag* pmachine_t;

typedef struct instruction_tag {
    char *mnemonic;
    int operandNum; // 0(setd only), 1, 2, 3, 4, 5(conditional branch), 6(syscall), 7(sob only), 8(mul,div,ash,ashc)
    void (*exec)(pmachine_t);
} instruction_t;

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

void exec(pmachine_t pm);

void nop(pmachine_t pm);

void mov(pmachine_t pm);
void cmp(pmachine_t pm);
void bit(pmachine_t pm);
void bic(pmachine_t pm);
void bis(pmachine_t pm);
void add(pmachine_t pm);
void sub(pmachine_t pm);
void mul(pmachine_t pm);
void mydiv(pmachine_t pm);
void ash(pmachine_t pm);
void ashc(pmachine_t pm);
void xor(pmachine_t pm);
void sob(pmachine_t pm);

void br(pmachine_t pm);
void bne(pmachine_t pm);
void beq(pmachine_t pm);
void bge(pmachine_t pm);
void blt(pmachine_t pm);
void bgt(pmachine_t pm);
void ble(pmachine_t pm);
void bpl(pmachine_t pm);
void bmi(pmachine_t pm);
void bhi(pmachine_t pm);
void blos(pmachine_t pm);
void bvc(pmachine_t pm);
void bvs(pmachine_t pm);
void bcc(pmachine_t pm);
void bcs(pmachine_t pm);

void clr(pmachine_t pm);
void com(pmachine_t pm);
void inc(pmachine_t pm);
void dec(pmachine_t pm);
void neg(pmachine_t pm);
void adc(pmachine_t pm);
void sbc(pmachine_t pm);
void tst(pmachine_t pm);

void jmp(pmachine_t pm);
void jsr(pmachine_t pm);
void rts(pmachine_t pm);

void sys(pmachine_t pm);
