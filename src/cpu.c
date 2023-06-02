#include <stdio.h>
#include <assert.h>

#include "cpu.h"
#include "inst.h"
#include "util.h"

char *toRegName[] = {
    "r0",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "sp", //"r6",
    "pc", //"r7",
};

void init(
    cpu_t *pcpu,
    context_t ctx,
    mmu_v2r_t v2r,
    mmu_r2v_t r2v,
    syscall_t syscallHook,
    syscall_string_t syscallStringHook,
    uint16_t sp, uint16_t pc) {
    // machine
    pcpu->ctx = ctx;
    pcpu->mmuV2R = v2r;
    pcpu->mmuR2V = r2v;
    pcpu->syscallHook = syscallHook;
    pcpu->syscallStringHook = syscallStringHook;

    // set reg ptr
    pcpu->r[0] = &pcpu->r0;
    pcpu->r[1] = &pcpu->r1;
    pcpu->r[2] = &pcpu->r2;
    pcpu->r[3] = &pcpu->r3;
    pcpu->r[4] = &pcpu->r4;
    pcpu->r[5] = &pcpu->r5;
    pcpu->r[6] = &pcpu->sp;
    pcpu->r[7] = &pcpu->pc;

    // clear regs
    pcpu->r0 = 0;
    pcpu->r1 = 0;
    pcpu->r2 = 0;
    pcpu->r3 = 0;
    pcpu->r4 = 0;
    pcpu->r5 = 0;
    pcpu->sp = sp;
    pcpu->pc = pc;
    pcpu->psw = 0;
}

uint16_t fetch(cpu_t *pcpu) {
    pcpu->addr = pcpu->pc;
    pcpu->bin = read16(false, pcpu->mmuV2R(pcpu->ctx, pcpu->pc));
    pcpu->pc += 2;
    return pcpu->bin;
}

void decode(cpu_t *pcpu) {
    pcpu->inst = NULL;

    pcpu->isByte = false;
    pcpu->isEven = false;
    pcpu->mode0 = (pcpu->bin & 0x0e00) >> 9;
    pcpu->reg0 = (pcpu->bin & 0x01c0) >> 6;
    pcpu->mode1 = (pcpu->bin & 0x0038) >> 3;
    pcpu->reg1 = pcpu->bin & 0x0007;
    pcpu->offset = pcpu->bin & 0x00ff;
    pcpu->syscallID = pcpu->bin & 0x003f;

    uint16_t op = pcpu->bin >> 12;
    instruction_t *table = NULL;
    do {
        uint8_t op_temp = op & 7;
        if (op_temp != 0 && op_temp != 7) {
            table = doubleOperand0;
            if (op & 8 && op_temp != 6) {
                pcpu->isByte = true;
            }
            break;
        }
        if (op == 7) {
            if (pcpu->mode0 != 5) {
                table = doubleOperand1;
                op = (pcpu->bin >> 9) & 7; // (4+3) bits
                if (op == 7) {
                    // sob
                    pcpu->offset &= 0x3f; // 6 bits positive num
                } else {
                    if ((pcpu->reg0 & 1) == 0) {
                        pcpu->isEven = true;
                    }
                }
            } else {
                table = floatingPoint0;
                op = pcpu->bin >> 9; // (4+3) bits
                // TODO: not implemented
                assert(0);
            }
            break;
        }
        if (op == 15) {
            table = floatingPoint1;
            op = pcpu->offset & 0xf; // 4 bits
            break;
        }
        if (op == 0 || op == 8) {
            if (pcpu->mode0 & 4) {
                if (op == 0) {
                    table = singleOperand0;
                } else {
                    table = singleOperand1;
                    if (pcpu->mode0 == 5 || (pcpu->mode0 == 6 && pcpu->reg0 < 4)) {
                        pcpu->isByte = true;
                    }
                }
                op = ((pcpu->mode0 & 3) << 3) | pcpu->reg0; // (2+3) bits
            } else {
                // conditionalBranch
                if (op == 0) {
                    table = conditionalBranch0;
                    op = ((pcpu->mode0 & 3) << 1) | (pcpu->reg0 >> 2); // (2+1) bits
                    if (op == 0) {
                        // systemMisc
                        table = systemMisc;
                        if (pcpu->reg0 == 0 && pcpu->mode1 == 0) {
                            // interrupt, misc
                            op = pcpu->reg1;
                        } else if (pcpu->reg0 == 1) {
                            // jmp
                            op = 8;
                        } else if (pcpu->reg0 == 2) {
                            op = 9 + (pcpu->mode1 >> 1);
                            if (op == 9) {
                                // subroutine
                            } else {
                                // condition
                                table = clearSet;
                                op = pcpu->bin & 0x1f;
                            }
                        } else if (pcpu->reg0 == 3) {
                            // swab
                            op = 13;
                        } else {
                            // TODO: unknown op
                            assert(0);
                        }
                    }
                } else {
                    table = conditionalBranch1;
                    op = ((pcpu->mode0 & 3) << 1) | (pcpu->reg0 >> 2); // (2+1) bits
                }
            }
            break;
        }
    } while(0);

    if (table == NULL || table[op].mnemonic == NULL) {
        // TODO: unknown op or not implemented
        fprintf(stderr, "/ [ERR] Unknown: %04x: %04x\n", pcpu->addr, pcpu->bin);
        assert(0);
    }
    pcpu->inst = &table[op];
}
