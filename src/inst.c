#include <stdio.h>
#include <assert.h>

#include "inst.h"
#include "machine.h"
#include "util.h"
#include "syscall.h"

instruction_t doubleOperand0[] = {
    {"",    0, NULL},     // 0 000b: singleOperand0[], conditionalBranch0[]
    {"mov", 4, mov},  // 0 001b:
    {"cmp", 4, cmp},  // 0 010b:
    {"bit", 4, bit},  // 0 011b:
    {"bic", 4, bic},  // 0 100b:
    {"bis", 4, bis},  // 0 101b:
    {"add", 4, add},  // 0 110b:
    {"",    3, NULL},     // 0 111b: doubleOperand1[], floatingPoint0[]

    {"",     0, NULL},     // 1 000b: singleOperand1[], conditionalBranch1[]
    {"movb", 4, mov}, // 1 001b:
    {"cmpb", 4, cmp}, // 1 010b:
    {"bitb", 4, bit}, // 1 011b:
    {"bicb", 4, bic}, // 1 100b:
    {"bisb", 4, bis}, // 1 101b:
    {"sub",  4, sub},  // 1 110b:
    {"",     4, NULL},     // 1 111b: floatingPoint1[]
};

instruction_t doubleOperand1[] = {
    {"mul", 8, mul},  // 0 111 000b:
    {"div", 8, mydiv},  // 0 111 001b:
    {"ash", 8, ash},  // 0 111 010b:
    {"ashc", 8, ashc}, // 0 111 011b:
    {"xor", 3, xor},  // 0 111 100b:
    {"", 0},     // 0 111 101b: floatingPoint0[]
    {NULL, 0},   // 0 111 110b: system?
    {"sob", 7, sob},  // 0 111 111b:
};

instruction_t singleOperand0[] = {
    // 004r
    {"jsr", 3, jsr},  // 0 000 100 000b: r0
    {"jsr", 3, jsr},  // 0 000 100 001b: r1
    {"jsr", 3, jsr},  // 0 000 100 010b: r2
    {"jsr", 3, jsr},  // 0 000 100 011b: r3
    {"jsr", 3, jsr},  // 0 000 100 100b: r4
    {"jsr", 3, jsr},  // 0 000 100 101b: r5
    {"jsr", 3, jsr},  // 0 000 100 110b: sp
    {"jsr", 3, jsr},  // 0 000 100 111b: pc

    // 005?
    {"clr", 2, clr},  // 0 000 101 000b:
    {"com", 2, com},  // 0 000 101 001b:
    {"inc", 2, inc},  // 0 000 101 010b:
    {"dec", 2, dec},  // 0 000 101 011b:
    {"neg", 2, neg},  // 0 000 101 100b:
    {"adc", 2, adc},  // 0 000 101 101b:
    {"sbc", 2, sbc},  // 0 000 101 110b:
    {"tst", 2, tst},  // 0 000 101 111b:
    // 006?
    // TODO: not implemented
    {"ror", 2, ror},  // 0 000 110 000b:
    {"rol", 2, rol},  // 0 000 110 001b:
    {"asr", 2, asr},  // 0 000 110 010b:
    {"asl", 2, asl},  // 0 000 110 011b:
    {"mark", 2}, // 0 000 110 100b:
    {"mfpi", 2}, // 0 000 110 101b:
    {"mtpi", 2}, // 0 000 110 110b:
    {"sxt", 2, sxt},  // 0 000 110 111b:
    // 007?
    {NULL, 0},  // 0 000 111 000b:
    {NULL, 0},  // 0 000 111 001b:
    {NULL, 0},  // 0 000 111 010b:
    {NULL, 0},  // 0 000 111 011b:
    {NULL, 0},  // 0 000 111 100b:
    {NULL, 0},  // 0 000 111 101b:
    {NULL, 0},  // 0 000 111 110b:
    {NULL, 0},  // 0 000 111 111b:
};

instruction_t singleOperand1[] = {
    // 104x
    {"emt", 2},  // 1 000 100 000b:
    {"emt", 2},  // 1 000 100 001b:
    {"emt", 2},  // 1 000 100 010b:
    {"emt", 2},  // 1 000 100 011b:
    {"sys", 6, sys},  // 1 000 100 100b:
    {"trap", 2}, // 1 000 100 101b:
    {"trap", 2}, // 1 000 100 110b:
    {"trap", 2}, // 1 000 100 111b:

    // 105?
    {"clrb", 2, clr}, // 1 000 101 000b:
    {"comb", 2, com}, // 1 000 101 001b:
    {"incb", 2, inc}, // 1 000 101 010b:
    {"decb", 2, dec}, // 1 000 101 011b:
    {"negb", 2, neg}, // 1 000 101 100b:
    {"adcb", 2, adc}, // 1 000 101 101b:
    {"sbcb", 2, sbc}, // 1 000 101 110b:
    {"tstb", 2, tst}, // 1 000 101 111b:
    // 106?
    // TODO: not implemented
    {"rorb", 2, ror}, // 1 000 110 000b:
    {"rolb", 2, rol}, // 1 000 110 001b:
    {"asrb", 2, asr}, // 1 000 110 010b:
    {"aslb", 2, asl}, // 1 000 110 011b:
    {"mtps", 2}, // 1 000 110 100b:
    {"mfpd", 2}, // 1 000 110 101b:
    {"mtpd", 2}, // 1 000 110 110b:
    {"mfps", 2}, // 1 000 110 111b:
    // 107?
    {NULL, 0},  // 1 000 111 000b:
    {NULL, 0},  // 1 000 111 001b:
    {NULL, 0},  // 1 000 111 010b:
    {NULL, 0},  // 1 000 111 011b:
    {NULL, 0},  // 1 000 111 100b:
    {NULL, 0},  // 1 000 111 101b:
    {NULL, 0},  // 1 000 111 110b:
    {NULL, 0},  // 1 000 111 111b:
};

instruction_t conditionalBranch0[] = {
    {"", 0},     // 0 000 000 0b: systemMisc[]
    {"br", 5, br},   // 0 000 000 1b:
    {"bne", 5, bne},  // 0 000 001 0b:
    {"beq", 5, beq},  // 0 000 001 1b:
    {"bge", 5, bge},  // 0 000 010 0b:
    {"blt", 5, blt},  // 0 000 010 1b:
    {"bgt", 5, bgt},  // 0 000 011 0b:
    {"ble", 5, ble},  // 0 000 011 1b:
};

instruction_t conditionalBranch1[] = {
    {"bpl", 5, bpl},  // 1 000 000 0b:
    {"bmi", 5, bmi},  // 1 000 000 1b:
    {"bhi", 5, bhi},  // 1 000 001 0b:
    {"blos", 5, blos}, // 1 000 001 1b:
    {"bvc", 5, bvc},  // 1 000 010 0b:
    {"bvs", 5, bvs},  // 1 000 010 1b:
    {"bcc", 5, bcc}, // 1 000 011 0b: alias bhis, bec
    {"bcs", 5, bcs},  // 1 000 011 1b: alias blo, bes
};

instruction_t systemMisc[] = {
    // TODO: not implemented
    {"halt", 0},  // 0 000 000 000 000 000b:
    {"wait", 0},  // 0 000 000 000 000 001b:
    {"rti", 0},   // 0 000 000 000 000 010b:
    {"bpt", 0},   // 0 000 000 000 000 011b:
    {"iot", 0},   // 0 000 000 000 000 100b:
    {"reset", 0}, // 0 000 000 000 000 101b:
    {"rtt", 0},   // 0 000 000 000 000 110b:
    {NULL, 0},    // 0 000 000 000 000 111b:

    {"jmp", 2, jmp},   // 0 000 000 001b:

    // subroutine, condition
    {"rts", 1, rts},   // 0 000 000 010 000b:
    {NULL, 0},    // 0 000 000 010 010b:
    {"clear", 0}, // 0 000 000 010 100 000b:
    {"set", 0},   // 0 000 000 010 110 000b:

    {"swab", 2, myswab},  // 0 000 000 011b:
};

instruction_t floatingPoint0[] = {
    // TODO: not implemented
    {"fadd", 1},   // 0 111 101 000b:
    {"fsub", 1},   // 0 111 101 001b:
    {"fmul", 1},   // 0 111 101 010b:
    {"fdiv", 1},   // 0 111 101 011b:
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
};

instruction_t floatingPoint1[] = {
    {NULL, 0},   // 1 111 000 000 000 000b:
    {NULL, 0},   // 1 111 000 000 000 001b:
    {NULL, 0},   // 1 111 000 000 000 010b:
    {NULL, 0},   // 1 111 000 000 000 011b:

    {NULL, 0},   // 1 111 000 000 000 100b:
    {NULL, 0},   // 1 111 000 000 000 101b:
    {NULL, 0},   // 1 111 000 000 000 110b:
    {NULL, 0},   // 1 111 000 000 000 111b:

    {NULL, 0},   // 1 111 000 000 001 000b:
    {"setd", 0, nop}, // 1 111 000 000 001 001b:
    {NULL, 0},   // 1 111 000 000 001 010b:
    {NULL, 0},   // 1 111 000 000 001 011b:

    {NULL, 0},   // 1 111 000 000 001 100b:
    {NULL, 0},   // 1 111 000 000 001 101b:
    {NULL, 0},   // 1 111 000 000 001 110b:
    {NULL, 0},   // 1 111 000 000 001 111b:
};

instruction_t clearSet[] = {
    {"nop", 0, myclearset},
    {"cl" "c", 0, myclearset},
    {"cl" "v", 0, myclearset},
    {"cl" "vc", 0, myclearset},
    {"cl" "z", 0, myclearset},
    {"cl" "zc", 0, myclearset},
    {"cl" "zv", 0, myclearset},
    {"cl" "zvc", 0, myclearset},
    {"cl" "n", 0, myclearset},
    {"cl" "nc", 0, myclearset},
    {"cl" "nv", 0, myclearset},
    {"cl" "nvc", 0, myclearset},
    {"cl" "nz", 0, myclearset},
    {"cl" "nzc", 0, myclearset},
    {"cl" "nzv", 0, myclearset},
    {"ccc", 0, myclearset},

    {"nop", 0, myclearset},
    {"se" "c", 0, myclearset},
    {"se" "v", 0, myclearset},
    {"se" "vc", 0, myclearset},
    {"se" "z", 0, myclearset},
    {"se" "zc", 0, myclearset},
    {"se" "zv", 0, myclearset},
    {"se" "zvc", 0, myclearset},
    {"se" "n", 0, myclearset},
    {"se" "nc", 0, myclearset},
    {"se" "nv", 0, myclearset},
    {"se" "nvc", 0, myclearset},
    {"se" "nz", 0, myclearset},
    {"se" "nzc", 0, myclearset},
    {"se" "nzv", 0, myclearset},
    {"scc", 0, myclearset},
};


static inline bool msb32(int32_t v) {
    return (v & 0x80000000);
}
static inline bool msb16(int16_t v) {
    return (v & 0x8000);
}
static inline bool msb8(int8_t v) {
    return (v & 0x80);
}

static uint8_t *operand(machine_t *pm, uint8_t mode, uint8_t reg) {
    uint8_t *ret;

    uint16_t addr;
    int16_t word;
    uint16_t *rn = pm->r[reg];
    switch (mode) {
    case 0:
        return (uint8_t *)rn;
        break;
    case 1:
        return &pm->virtualMemory[*rn];
        break;
    case 2:
        ret = &pm->virtualMemory[*rn];
        if (!pm->isByte || reg == 7) {
            (*rn) += 2;
        } else {
            (*rn) += 1;
        }
        return ret;
        break;
    case 3:
        addr = read16(false, &pm->virtualMemory[*rn]);
        (*rn) += 2;
        return &pm->virtualMemory[addr];
        break;
    case 4:
        if (!pm->isByte) {
            (*rn) -= 2;
        } else {
            (*rn) -= 1;
        }
        return &pm->virtualMemory[*rn];
        break;
    case 5:
        (*rn) -= 2;
        addr = read16(false, &pm->virtualMemory[*rn]);
        return &pm->virtualMemory[addr];
        break;
    case 6:
        word = fetch(pm);
        return &pm->virtualMemory[((int16_t)(*rn) + word) & 0xffff];
        break;
    case 7:
        word = fetch(pm);
        addr = read16(false, &pm->virtualMemory[((int16_t)(*rn) + word) & 0xffff]);
        return &pm->virtualMemory[addr];
        break;
    default:
        assert(0);
        break;
    }
}


void exec(machine_t *pm) {
    // debug
    //uint16_t addr0 = pm->sp;
    //uint16_t sp0 = read16(false, &pm->virtualMemory[pm->sp]);
    //fprintf(stderr, "%04x: %s ", pm->addr, pm->inst->mnemonic);

    if (pm->inst->operandNum == 4) {
        // doubleOperand0
        pm->operand0 = operand(pm, pm->mode0, pm->reg0);
        pm->operand1 = operand(pm, pm->mode1, pm->reg1);
    } else if (pm->inst->operandNum == 3 || pm->inst->operandNum == 8) {
        // doubleOperand1, mul,div,ash,ashc, jsr
        pm->operand0 = operand(pm, 0, pm->reg0);
        pm->operand1 = operand(pm, pm->mode1, pm->reg1);
    } else if (pm->inst->operandNum == 7) {
        // doubleOperand1 sob only
        pm->operand0 = operand(pm, 0, pm->reg0);
        //pm->operand1 = &pm->offset;
    } else if (pm->inst->operandNum == 2) {
        // singleOperand0, singleOperand1, jmp, swab
        pm->operand1 = operand(pm, pm->mode1, pm->reg1);
    } else if (pm->inst->operandNum == 1) {
        // subroutine
        pm->operand1 = operand(pm, 0, pm->reg1);
    } else if (pm->inst->operandNum == 5) {
        // conditionalBranch0, conditionalBranch1
        //pm->operand0 = &pm->offset;
    } else if (pm->inst->operandNum == 6) {
        // syscall
    } else if (pm->inst->operandNum == 0) {
        // floatingPoint1, systemMisc
    } else {
        // TODO: unknown op
        assert(0);
    }

    if (pm->inst->exec == NULL) {
        fprintf(stderr, "/ [ERR] Not implemented: %s, %04x: %04x\n", pm->inst->mnemonic, pm->addr, pm->bin);
        assert(pm->inst->exec);
    }

    pm->inst->exec(pm);

    // debug
    //uint16_t addr1 = pm->sp;
    //uint16_t sp1 = read16(false, &pm->virtualMemory[pm->sp]);
    //fprintf(stderr, "(sp: %04x:%04x -> %04x:%04x, pc: %04x)\n", addr0, sp0, addr1, sp1, pm->pc);
}

void nop(machine_t *pm) {
    // nothing to do
}

void mov(machine_t *pm) {
    int signedSrc;
    if (!pm->isByte) {
        uint16_t src = read16((pm->mode0 == 0), pm->operand0);
        write16((pm->mode1 == 0), pm->operand1, src);
        signedSrc = (int16_t)src;
    } else {
        uint8_t src = read8((pm->mode0 == 0), pm->operand0);
        write8((pm->mode1 == 0), pm->operand1, src);
        signedSrc = (int8_t)src;
    }

    if (signedSrc < 0) {
        setN(pm);
    } else {
        clearN(pm);
    }
    if (signedSrc == 0) {
        setZ(pm);
    } else {
        clearZ(pm);
    }
    clearV(pm);
}

void cmp(machine_t *pm) {
    if (!pm->isByte) {
        int16_t src16 = read16((pm->mode0 == 0), pm->operand0);
        int16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        int16_t result16 = src16 - dst16;

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (msb16(src16) != msb16(dst16) && msb16(dst16) == msb16(result16)) {
            setV(pm);
        } else {
            clearV(pm);
        }
        uint32_t c = (uint16_t)src16 + ~(uint16_t)dst16 + 1;
        if (c & 0x10000) {
            setC(pm);
        } else {
            clearC(pm);
        }
    } else {
        int8_t src8 = read8((pm->mode0 == 0), pm->operand0);
        int8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        int8_t result8 = src8 - dst8;

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (msb8(src8) != msb8(dst8) && msb8(dst8) == msb8(result8)) {
            setV(pm);
        } else {
            clearV(pm);
        }
        uint16_t c = (uint8_t)src8 + ~(uint8_t)dst8 + 1;
        if (c & 0x100) {
            setC(pm);
        } else {
            clearC(pm);
        }
    }
}

void bit(machine_t *pm) {
    if (!pm->isByte) {
        uint16_t src16 = read16((pm->mode0 == 0), pm->operand0);
        uint16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        uint16_t result16 = src16 & dst16;

        if (msb16(result16)) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    } else {
        uint8_t src8 = read8((pm->mode0 == 0), pm->operand0);
        uint8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        uint8_t result8 = src8 & dst8;

        if (msb8(result8)) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    }

    clearV(pm);
}

void bic(machine_t *pm) {
    if (!pm->isByte) {
        uint16_t src16 = read16((pm->mode0 == 0), pm->operand0);
        uint16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        dst16 &= ~src16;
        write16((pm->mode1 == 0), pm->operand1, dst16);

        if (msb16(dst16)) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (dst16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    } else {
        uint8_t src8 = read8((pm->mode0 == 0), pm->operand0);
        uint8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        dst8 &= ~src8;
        write8((pm->mode1 == 0), pm->operand1, dst8);

        if (msb16(dst8)) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (dst8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    }

    clearV(pm);
}

void bis(machine_t *pm) {
    if (!pm->isByte) {
        uint16_t src16 = read16((pm->mode0 == 0), pm->operand0);
        uint16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        dst16 |= src16;
        write16((pm->mode1 == 0), pm->operand1, dst16);

        if (msb16(dst16)) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (dst16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    } else {
        uint8_t src8 = read8((pm->mode0 == 0), pm->operand0);
        uint8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        dst8 |= src8;
        write8((pm->mode1 == 0), pm->operand1, dst8);

        if (msb16(dst8)) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (dst8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    }

    clearV(pm);
}

void add(machine_t *pm) {
    if (!pm->isByte) {
        int16_t src16 = read16((pm->mode0 == 0), pm->operand0);
        int16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        int16_t result16 = src16 + dst16;
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (msb16(src16) == msb16(dst16) && msb16(src16) != msb16(result16)) {
            setV(pm);
        } else {
            clearV(pm);
        }
        uint32_t c = (uint16_t)src16 + (uint16_t)dst16;
        if (c & 0x10000) {
            setC(pm);
        } else {
            clearC(pm);
        }
    } else {
        int8_t src8 = read8((pm->mode0 == 0), pm->operand0);
        int8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        int8_t result8 = src8 + dst8;
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (msb8(src8) == msb8(dst8) && msb8(src8) != msb8(result8)) {
            setV(pm);
        } else {
            clearV(pm);
        }
        uint16_t c = (uint8_t)src8 + (uint8_t)dst8;
        if (c & 0x100) {
            setC(pm);
        } else {
            clearC(pm);
        }
    }
}

void sub(machine_t *pm) {
    if (!pm->isByte) {
        int16_t src16 = read16((pm->mode0 == 0), pm->operand0);
        int16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        int16_t result16 = dst16 - src16;
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (msb16(src16) != msb16(dst16) && msb16(src16) == msb16(result16)) {
            setV(pm);
        } else {
            clearV(pm);
        }
        uint32_t c = (uint16_t)dst16 + ~(uint16_t)src16 + 1;
        if (c & 0x10000) {
            setC(pm);
        } else {
            clearC(pm);
        }
    } else {
        int8_t src8 = read8((pm->mode0 == 0), pm->operand0);
        int8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        int8_t result8 = dst8 - src8;
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (msb8(src8) != msb8(dst8) && msb8(src8) == msb8(result8)) {
            setV(pm);
        } else {
            clearV(pm);
        }
        uint16_t c = (uint8_t)dst8 + ~(uint8_t)src8 + 1;
        if (c & 0x100) {
            setC(pm);
        } else {
            clearC(pm);
        }
    }
}

void mul(machine_t *pm) {
    int32_t m = (int16_t)read16(true, pm->operand0) * (int16_t)read16((pm->mode1 == 0), pm->operand1);
    uint32_t temp = m;
    if (pm->isEven) {
        write16(true, pm->operand0, temp >> 16);
        write16(true, pm->operand0 + 2, temp & 0x0000ffff);
    } else {
        write16(true, pm->operand0, temp & 0x0000ffff);
    }

    if (m < 0) {
        setN(pm);
    } else {
        clearN(pm);
    }
    if (m == 0) {
        setZ(pm);
    } else {
        clearZ(pm);
    }
    clearV(pm);
    if (m < -(1<<15) || (1<<15) - 1 <= m) {
        setC(pm);
    } else {
        clearC(pm);
    }
}

void mydiv(machine_t *pm) {
    assert(pm->isEven);

    int16_t reg = read16(true, pm->operand0);
    int32_t v32 = (reg << 16) | read16(true, pm->operand0 + 2);
    int16_t d = read16((pm->mode1 == 0), pm->operand1);

    clearV(pm);
    clearC(pm);
    if (d == 0) {
        setV(pm);
        setC(pm);
        return;
    }
    if (abs(reg) > abs(d)) {
        setV(pm);
        return;
    }

    int16_t q = v32 / d;
    int16_t r = v32 % d;
    write16(true, pm->operand0, q);
    write16(true, pm->operand0 + 2, r);

    if (q < 0) {
        setN(pm);
    } else {
        clearN(pm);
    }
    if (q == 0) {
        setZ(pm);
    } else {
        clearZ(pm);
    }
}

void ash(machine_t *pm) {
    int16_t reg = (int16_t)read16(true, pm->operand0);
    int16_t shift6 = (int16_t)((read16((pm->mode1 == 0), pm->operand1) & 0x3f) << 10) >> 10;

    int16_t result;
    bool c;
    if (shift6 > 16) {
        result = 0;
        c = 0;
    } else if (shift6 > 0) {
        result = reg << shift6;
        c = reg & (1 << (16 - shift6));
    } else if (shift6 == 0) {
        result = reg;
        c = 0;
    } else {
        result = reg >> -shift6;
        c = (int32_t)reg & (1 << (-shift6 - 1));
    }
    write16(true, pm->operand0, result);

    if (result < 0) {
        setN(pm);
    } else {
        clearN(pm);
    }
    if (result == 0) {
        setZ(pm);
    } else {
        clearZ(pm);
    }
    if (msb16(reg) != msb16(result)) {
        setV(pm);
    } else {
        clearV(pm);
    }
    if (c) {
        setC(pm);
    } else {
        clearC(pm);
    }
}

void ashc(machine_t *pm) {
    uint16_t reg = read16(true, pm->operand0);
    int32_t v32 = (reg << 16) | ((pm->isEven) ? read16(true, pm->operand0 + 2) : reg);
    int16_t shift6 = (int16_t)((read16((pm->mode1 == 0), pm->operand1) & 0x3f) << 10) >> 10;

    int32_t result;
    bool c;
    if (shift6 > 0) {
        result = v32 << shift6;
        c = v32 & (1 << (32 - shift6));
    } else if (shift6 == 0) {
        result = v32;
        c = 0;
    } else {
        result = v32 >> -shift6;
        c = v32 & (1 << (-shift6 - 1));
    }
    if (pm->isEven) {
        write16(true, pm->operand0, result >> 16);
        write16(true, pm->operand0 + 2, result & 0x0000ffff);
    } else {
        write16(true, pm->operand0, result & 0x0000ffff);
    }

    if (result < 0) {
        setN(pm);
    } else {
        clearN(pm);
    }
    if (result == 0) {
        setZ(pm);
    } else {
        clearZ(pm);
    }
    if (msb32(v32) != msb32(result)) {
        setV(pm);
    } else {
        clearV(pm);
    }
    if (c) {
        setC(pm);
    } else {
        clearC(pm);
    }
}

void xor(machine_t *pm) {
    uint16_t result = read16(true, pm->operand0) ^ read16((pm->mode1 == 0), pm->operand1);
    write16((pm->mode1 == 0), pm->operand1, result);

    if (result < 0) {
        setN(pm);
    } else {
        clearN(pm);
    }
    if (result == 0) {
        setZ(pm);
    } else {
        clearZ(pm);
    }
    clearV(pm);
}

void sob(machine_t *pm) {
    int16_t reg = (int16_t)read16(true, pm->operand0) - 1;
    write16(true, pm->operand0, reg);

    if (reg != 0) {
        pm->pc -= (pm->offset << 1);
    }
}

void br(machine_t *pm) {
    pm->pc += ((int8_t)pm->offset << 1);
}

void bne(machine_t *pm) {
    if (!isZ(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void beq(machine_t *pm) {
    if (isZ(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void bge(machine_t *pm) {
    if (!(isN(pm) ^ isV(pm))) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void blt(machine_t *pm) {
    if (isN(pm) ^ isV(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void bgt(machine_t *pm) {
    if (!(isZ(pm) || (isN(pm) ^ isV(pm)))) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void ble(machine_t *pm) {
    if (isZ(pm) || (isN(pm) ^ isV(pm))) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void bpl(machine_t *pm) {
    if (!isN(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void bmi(machine_t *pm) {
    if (isN(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void bhi(machine_t *pm) {
    if (!(isC(pm) || isZ(pm))) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void blos(machine_t *pm) {
    if (isC(pm) || isZ(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void bvc(machine_t *pm) {
    if (!isV(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void bvs(machine_t *pm) {
    if (isV(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

// bhis
void bcc(machine_t *pm) {
    if (!isC(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

// blo
void bcs(machine_t *pm) {
    if (isC(pm)) {
        pm->pc += ((int8_t)pm->offset << 1);
    }
}

void clr(machine_t *pm) {
    if (!pm->isByte) {
        write16((pm->mode1 == 0), pm->operand1, 0);
    } else {
        write8((pm->mode1 == 0), pm->operand1, 0);
    }

    clearN(pm);
    setZ(pm);
    clearV(pm);
    clearC(pm);
}

void com(machine_t *pm) {
    if (!pm->isByte) {
        uint16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        dst16 = ~dst16;
        write16((pm->mode1 == 0), pm->operand1, dst16);

        if (msb16(dst16)) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (dst16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        clearV(pm);
        setC(pm);
    } else {
        uint8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        dst8 = ~dst8;
        write8((pm->mode1 == 0), pm->operand1, dst8);

        if (msb8(dst8)) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (dst8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        clearV(pm);
        setC(pm);
    }
}

void inc(machine_t *pm) {
    if (!pm->isByte) {
        int16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        int16_t result16 = dst16 + 1;
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (dst16 == 0x7fff) {
            setV(pm);
        } else {
            clearV(pm);
        }
    } else {
        int8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        int8_t result8 = dst8 + 1;
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (dst8 == 0x7f) {
            setV(pm);
        } else {
            clearV(pm);
        }
    }
}

void dec(machine_t *pm) {
    if (!pm->isByte) {
        int16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        int16_t result16 = dst16 - 1;
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if ((uint16_t)dst16 == 0x8000) {
            setV(pm);
        } else {
            clearV(pm);
        }
    } else {
        int8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        int8_t result8 = dst8 - 1;
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if ((uint8_t)dst8 == 0x80) {
            setV(pm);
        } else {
            clearV(pm);
        }
    }
}

void neg(machine_t *pm) {
    if (!pm->isByte) {
        int16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        int16_t result16 = -dst16;
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if ((uint16_t)result16 == 0x8000) {
            setV(pm);
        } else {
            clearV(pm);
        }
        if (result16 == 0) {
            clearC(pm);
        } else {
            setC(pm);
        }
    } else {
        int8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        int8_t result8 = -dst8;
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if ((uint8_t)result8 == 0x80) {
            setV(pm);
        } else {
            clearV(pm);
        }
        if (result8 == 0) {
            clearC(pm);
        } else {
            setC(pm);
        }
    }
}

void adc(machine_t *pm) {
    if (!pm->isByte) {
        int16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        int16_t result16 = dst16 + (isC(pm) ? 1 : 0);
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (dst16 == 0x7fff && isC(pm)) {
            setV(pm);
        } else {
            clearV(pm);
        }
        if ((uint16_t)dst16 == 0xffff && isC(pm)) {
            setC(pm);
        } else {
            clearC(pm);
        }
    } else {
        int8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        int8_t result8 = dst8 + (isC(pm) ? 1 : 0);
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if (dst8 == 0x7f && isC(pm)) {
            setV(pm);
        } else {
            clearV(pm);
        }
        if ((uint8_t)dst8 == 0xff && isC(pm)) {
            setC(pm);
        } else {
            clearC(pm);
        }
    }
}

void sbc(machine_t *pm) {
    if (!pm->isByte) {
        int16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
        int16_t result16 = dst16 - (isC(pm) ? 1 : 0);
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if ((uint16_t)dst16 == 0x8000) {
            setV(pm);
        } else {
            clearV(pm);
        }
        if (dst16 == 0 && isC(pm)) {
            clearC(pm);
        } else {
            setC(pm);
        }
    } else {
        int8_t dst8 = read8((pm->mode1 == 0), pm->operand1);
        int8_t result8 = dst8 - (isC(pm) ? 1 : 0);
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
        if ((uint8_t)dst8 == 0x80) {
            setV(pm);
        } else {
            clearV(pm);
        }
        if (dst8 == 0 && isC(pm)) {
            clearC(pm);
        } else {
            setC(pm);
        }
    }
}

void tst(machine_t *pm) {
    if (!pm->isByte) {
        int16_t dst16 = read16((pm->mode1 == 0), pm->operand1);

        if (dst16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (dst16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    } else {
        int8_t dst8 = read8((pm->mode1 == 0), pm->operand1);

        if (dst8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (dst8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    }

    clearV(pm);
    clearC(pm);
}

void ror(machine_t *pm) {
    bool c;
    if (!pm->isByte) {
        int16_t dst16 = (int16_t)read16((pm->mode1 == 0), pm->operand1);

        c = dst16 & 1;
        int16_t result16 = dst16 >> 1;
        if (isC(pm)) {
            result16 |= 0x8000;
        } else {
            result16 &= 0x7fff;
        }
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    } else {
        int8_t dst8 = (int8_t)read8((pm->mode1 == 0), pm->operand1);

        c = dst8 & 1;
        int8_t result8 = dst8 >> 1;
        if (isC(pm)) {
            result8 |= 0x80;
        } else {
            result8 &= 0x7f;
        }
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    }

    if (isN(pm) ^ c) {
        setV(pm);
    } else {
        clearV(pm);
    }
    if (c) {
        setC(pm);
    } else {
        clearC(pm);
    }
}

void rol(machine_t *pm) {
    bool c;
    if (!pm->isByte) {
        int16_t dst16 = (int16_t)read16((pm->mode1 == 0), pm->operand1);

        c = msb16(dst16);
        int16_t result16 = dst16 << 1;
        if (isC(pm)) {
            result16 |= 1;
        } else {
            result16 &= 0xfffe;
        }
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    } else {
        int8_t dst8 = (int8_t)read8((pm->mode1 == 0), pm->operand1);

        c = msb8(dst8);
        int8_t result8 = dst8 << 1;
        if (isC(pm)) {
            result8 |= 1;
        } else {
            result8 &= 0xfe;
        }
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    }

    if (isN(pm) ^ c) {
        setV(pm);
    } else {
        clearV(pm);
    }
    if (c) {
        setC(pm);
    } else {
        clearC(pm);
    }
}

void asr(machine_t *pm) {
    bool c;
    if (!pm->isByte) {
        int16_t dst16 = (int16_t)read16((pm->mode1 == 0), pm->operand1);

        c = dst16 & 1;
        int16_t result16 = dst16 >> 1;
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    } else {
        int8_t dst8 = (int8_t)read8((pm->mode1 == 0), pm->operand1);

        c = dst8 & 1;
        int8_t result8 = dst8 >> 1;
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    }

    if (isN(pm) ^ c) {
        setV(pm);
    } else {
        clearV(pm);
    }
    if (c) {
        setC(pm);
    } else {
        clearC(pm);
    }
}

void asl(machine_t *pm) {
    bool c;
    if (!pm->isByte) {
        int16_t dst16 = (int16_t)read16((pm->mode1 == 0), pm->operand1);

        c = msb16(dst16);
        int16_t result16 = dst16 << 1;
        write16((pm->mode1 == 0), pm->operand1, result16);

        if (result16 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result16 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    } else {
        int8_t dst8 = (int8_t)read8((pm->mode1 == 0), pm->operand1);

        c = msb8(dst8);
        int8_t result8 = dst8 << 1;
        write8((pm->mode1 == 0), pm->operand1, result8);

        if (result8 < 0) {
            setN(pm);
        } else {
            clearN(pm);
        }
        if (result8 == 0) {
            setZ(pm);
        } else {
            clearZ(pm);
        }
    }

    if (isN(pm) ^ c) {
        setV(pm);
    } else {
        clearV(pm);
    }
    if (c) {
        setC(pm);
    } else {
        clearC(pm);
    }
}

void sxt(machine_t *pm) {
    uint16_t result = 0;
    if (isN(pm)) {
        result = 0xffff;
    }
    write16((pm->mode1 == 0), pm->operand1, result);

    if (!isN(pm)) {
        setZ(pm);
    }
}

void myswab(machine_t *pm) {
    uint16_t dst16 = read16((pm->mode1 == 0), pm->operand1);
    uint16_t result16 = (dst16 << 8) | (dst16 >> 8);
    write16((pm->mode1 == 0), pm->operand1, result16);

    if (result16 & 0x80) {
        setN(pm);
    } else {
        clearN(pm);
    }
    if ((result16 & 0xff) == 0) {
        setZ(pm);
    } else {
        clearZ(pm);
    }
    clearV(pm);
    clearC(pm);
}

void jmp(machine_t *pm) {
    if (pm->mode1 == 0) {
        assert(0); // illegal instruction
    }

    uint16_t addr = (pm->operand1 - pm->virtualMemory) & 0xffff;
    if (addr & 1) {
        assert(0); // boundary error
    }

    pm->pc = addr;
}

void jsr(machine_t *pm) {
    uint16_t addr = (pm->operand1 - pm->virtualMemory) & 0xffff;
    uint16_t reg = read16(true, pm->operand0);
    pm->sp -= 2;
    write16(false, &pm->virtualMemory[pm->sp], reg);
    write16(true, pm->operand0, pm->pc);
    pm->pc = addr;
}

void rts(machine_t *pm) {
    pm->pc = read16(true, pm->operand1);
    uint16_t temp = read16(false, &pm->virtualMemory[pm->sp]);
    pm->sp += 2;
    write16(true, pm->operand1, temp);
}

void sys(machine_t *pm) {
    if (pm->syscallID == 0) {
        // indir
        uint16_t addr = fetch(pm);
        //fprintf(stderr, "0; 0x%04x ", addr);

        uint16_t oldpc = pm->pc;
        {
            pm->pc = addr;
            pm->bin = fetch(pm);
            pm->syscallID = pm->bin & 0x3f;
            assert(pm->bin - pm->syscallID == 0104400);
            mysyscall(pm);
        }
        pm->pc = oldpc;
    } else {
        mysyscall(pm);
    }
}

void myclearset(machine_t *pm) {
    bool isSet = pm->bin & 0x10;
    bool N = pm->bin & 8;
    bool Z = pm->bin & 4;
    bool V = pm->bin & 2;
    bool C = pm->bin & 1;

    if (isSet) {
        if (N) setN(pm);
        if (Z) setZ(pm);
        if (V) setV(pm);
        if (C) setC(pm);
    } else {
        if (N) clearN(pm);
        if (Z) clearZ(pm);
        if (V) clearV(pm);
        if (C) clearC(pm);
    }
}
