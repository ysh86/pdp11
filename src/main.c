#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <assert.h>

// for PATH_MAX
#ifdef __linux__
#include <linux/limits.h>
#endif
#ifdef __APPLE__
#include <sys/syslimits.h>
#endif

typedef struct instruction_tag {
    char *mnemonic;
    int operandNum;
} instruction_t;

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

char *toFlagName[] = {
    "nop",
    "cl" "c",
    "cl" "v",
    "cl" "vc"
    "cl" "z",
    "cl" "zc",
    "cl" "zv",
    "cl" "zvc",
    "cl" "n",
    "cl" "nc",
    "cl" "nv",
    "cl" "nvc",
    "cl" "nz",
    "cl" "nzc",
    "cl" "nzv",
    "ccc",

    "nop",
    "se" "c",
    "se" "v",
    "se" "vc"
    "se" "z",
    "se" "zc",
    "se" "zv",
    "se" "zvc",
    "se" "n",
    "se" "nc",
    "se" "nv",
    "se" "nvc",
    "se" "nz",
    "se" "nzc",
    "se" "nzv",
    "scc",
};

instruction_t doubleOperand0[] = {
    {"", 0},     // 0 000b: singleOperand0[], conditionalBranch0[]
    {"mov", 4},  // 0 001b:
    {"cmp", 4},  // 0 010b:
    {"bit", 4},  // 0 011b:
    {"bic", 4},  // 0 100b:
    {"bis", 4},  // 0 101b:
    {"add", 4},  // 0 110b:
    {"", 3},     // 0 111b: doubleOperand1[], floatingPoint0[]

    {"", 0},     // 1 000b: singleOperand1[], conditionalBranch1[]
    {"movb", 4}, // 1 001b:
    {"cmpb", 4}, // 1 010b:
    {"bitb", 4}, // 1 011b:
    {"bicb", 4}, // 1 100b:
    {"bisb", 4}, // 1 101b:
    {"sub", 4},  // 1 110b:
    {"", 4},     // 1 111b: floatingPoint1[]
};

instruction_t doubleOperand1[] = {
    {"mul", 3},  // 0 111 000b:
    {"div", 3},  // 0 111 001b:
    {"ash", 3},  // 0 111 010b:
    {"ashc", 3}, // 0 111 011b:
    {"xor", 3},  // 0 111 100b:
    {"", 0},     // 0 111 101b: floatingPoint0[]
    {NULL, 0},   // 0 111 110b: system?
    {"sob", 3},  // 0 111 111b:
};

instruction_t singleOperand0[] = {
    // 004r
    {"jsr r0", 2},  // 0 000 100
    {"jsr r1", 2},  // 0 000 100
    {"jsr r2", 2},  // 0 000 100
    {"jsr r3", 2},  // 0 000 100
    {"jsr r4", 2},  // 0 000 100
    {"jsr r5", 2},  // 0 000 100
    {"jsr sp", 2},  // 0 000 100
    {"jsr pc", 2},  // 0 000 100

    // 005?
    {"clr", 2},  // 0 000 101 000b:
    {"com", 2},  // 0 000 101 001b:
    {"inc", 2},  // 0 000 101 010b:
    {"dec", 2},  // 0 000 101 011b:
    {"neg", 2},  // 0 000 101 100b:
    {"adc", 2},  // 0 000 101 101b:
    {"sbc", 2},  // 0 000 101 110b:
    {"tst", 2},  // 0 000 101 111b:
    // 006?
    {"ror", 2},  // 0 000 110 000b:
    {"rol", 2},  // 0 000 110 001b:
    {"asr", 2},  // 0 000 110 010b:
    {"asl", 2},  // 0 000 110 011b:
    {"mark", 2}, // 0 000 110 100b:
    {"mfpi", 2}, // 0 000 110 101b:
    {"mtpi", 2}, // 0 000 110 110b:
    {"sxt", 2},  // 0 000 110 111b:
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
    {"trap", 2}, // 1 000 100 100b:
    {"trap", 2}, // 1 000 100 101b:
    {"trap", 2}, // 1 000 100 110b:
    {"trap", 2}, // 1 000 100 111b:

    // 105?
    {"clrb", 2}, // 1 000 101 000b:
    {"comb", 2}, // 1 000 101 001b:
    {"incb", 2}, // 1 000 101 010b:
    {"decb", 2}, // 1 000 101 011b:
    {"negb", 2}, // 1 000 101 100b:
    {"adcb", 2}, // 1 000 101 101b:
    {"sbcb", 2}, // 1 000 101 110b:
    {"tstb", 2}, // 1 000 101 111b:
    // 106?
    {"rorb", 2}, // 1 000 110 000b:
    {"rolb", 2}, // 1 000 110 001b:
    {"asrb", 2}, // 1 000 110 010b:
    {"aslb", 2}, // 1 000 110 011b:
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
    {"br", 1},   // 0 000 000 1b:
    {"bne", 1},  // 0 000 001 0b:
    {"beq", 1},  // 0 000 001 1b:
    {"bge", 1},  // 0 000 010 0b:
    {"blt", 1},  // 0 000 010 1b:
    {"bgt", 1},  // 0 000 011 0b:
    {"ble", 1},  // 0 000 011 1b:
};

instruction_t conditionalBranch1[] = {
    {"bpl", 1},  // 1 000 000 0b:
    {"bmi", 1},  // 1 000 000 1b:
    {"bhi", 1},  // 1 000 001 0b:
    {"blos", 1}, // 1 000 001 1b:
    {"bvc", 1},  // 1 000 010 0b:
    {"bvs", 1},  // 1 000 010 1b:
    {"bcc or bhis", 1}, // 1 000 011 0b:
    {"bcs or blo", 1},  // 1 000 011 1b:
};

instruction_t systemMisc[] = {
    {"halt", 0},  // 0 000 000 000 000 000b:
    {"wait", 0},  // 0 000 000 000 000 001b:
    {"rti", 0},   // 0 000 000 000 000 010b:
    {"bpt", 0},   // 0 000 000 000 000 011b:
    {"iot", 0},   // 0 000 000 000 000 100b:
    {"reset", 0}, // 0 000 000 000 000 101b:
    {"rtt", 0},   // 0 000 000 000 000 110b:
    {NULL, 0},    // 0 000 000 000 000 111b:

    {"jmp", 2},   // 0 000 000 001b:

    // subroutine, condition
    {"rts", 1},   // 0 000 000 010 000b:
    {NULL, 0},    // 0 000 000 010 010b:
    {"clear", 0}, // 0 000 000 010 100 000b:
    {"set", 0},   // 0 000 000 010 110 000b:

    {"swab", 2},  // 0 000 000 011b:
};

instruction_t floatingPoint0[] = {
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
    {"setd", 0}, // 1 111 000 000 001 001b:
    {NULL, 0},   // 1 111 000 000 001 010b:
    {NULL, 0},   // 1 111 000 000 001 011b:

    {NULL, 0},   // 1 111 000 000 001 100b:
    {NULL, 0},   // 1 111 000 000 001 101b:
    {NULL, 0},   // 1 111 000 000 001 110b:
    {NULL, 0},   // 1 111 000 000 001 111b:
};

typedef struct machine_t_tag {
    // env
    char rootdir[PATH_MAX];
    char curdir[PATH_MAX];
    uint16_t aoutHeader[8]; // little endian only

    // memory
    uint8_t virtualMemory[64 * 1024];
    uint16_t textStart;
    uint16_t textEnd;
    uint16_t dataStart;
    uint16_t dataEnd;
    uint16_t bssStart;
    uint16_t bssEnd;

    // regs
    uint16_t r0;
    uint16_t r1;
    uint16_t r2;
    uint16_t r3;
    uint16_t r4;
    uint16_t r5;
    uint16_t sp; // r6
    uint16_t pc; // r7
    uint16_t psw;
} machine_t;

uint16_t fetch(machine_t *pm) {
    uint16_t bin = pm->virtualMemory[pm->pc] | (pm->virtualMemory[pm->pc + 1] << 8);
    pm->pc += 2;
    return bin;
}

void push(machine_t *pm, uint16_t value) {
    pm->sp -= 2;
    pm->virtualMemory[pm->sp] = value;
}

uint16_t pop(machine_t *pm) {
    uint16_t value = pm->virtualMemory[pm->sp];
    pm->sp += 2;
    return value;
}

void operand_string(machine_t *pm, char *str, size_t size, uint8_t mode, uint8_t reg) {
    int16_t word = 0;
    const char *rn = toRegName[reg];
    switch (mode) {
    case 0:
        snprintf(str, size, "%s", rn);
        break;
    case 1:
        snprintf(str, size, "(%s)", rn);
        break;
    case 2:
        if (reg != 7) {
            snprintf(str, size, "(%s)+", rn);
        } else {
            // pc
            word = fetch(pm);
            snprintf(str, size, "#%06o / 0x%04x", word, word);
        }
        break;
    case 3:
        if (reg != 7) {
            snprintf(str, size, "@(%s)+", rn);
        } else {
            // pc
            word = fetch(pm);
            snprintf(str, size, "@#%06o / 0x%04x", word, word);
        }
        break;
    case 4:
        snprintf(str, size, "-(%s)", rn);
        break;
    case 5:
        snprintf(str, size, "@-(%s)", rn);
        break;
    case 6:
        word = fetch(pm);
        if (reg != 7) {
            snprintf(str, size, "%d(%s)", word, rn);
        } else {
            // pc
            if (word > 0) {
                snprintf(str, size, "%06o / 0x%04x", word, word);
            } else {
                snprintf(str, size, "%d / 0x%04x", word, word & 0xffff);
            }
        }
        break;
    case 7:
        word = fetch(pm);
        if (reg != 7) {
            snprintf(str, size, "@%d(%s)", word, rn);
        } else {
            // pc
            snprintf(str, size, "@%06o / 0x%04x", word, word);
        }
        break;
    default:
        assert(0);
        break;
    }
}

int main(int argc, char *argv[]) {
    //////////////////////////
    // usage
    //////////////////////////
    if (argc < 2) {
        fprintf(stderr, "Usage: pdp11 exe_file\n");
        return EXIT_FAILURE;
    }

    machine_t machine;

    //////////////////////////
    // load
    //////////////////////////
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        fprintf(stderr, "Invalid input file\n");
        return EXIT_FAILURE;
    }

    size_t n;
    size_t size;
    size = sizeof(machine.aoutHeader);
    n = fread(machine.aoutHeader, 1, size, fp);
    if (n != size) {
        fprintf(stderr, "Can't read file\n");
        fclose(fp);
        return EXIT_FAILURE;
    }

    size = sizeof(machine.virtualMemory);
    n = fread(machine.virtualMemory, 1, size, fp);
    if (n <= 0) {
        fprintf(stderr, "Can't read file\n");
        fclose(fp);
        return EXIT_FAILURE;
    }
    // TODO: support 0x0108(0410) mode (8KB alignment)
    fclose(fp);
    fp = NULL;

    //////////////////////////
    // prepare
    //////////////////////////
    machine.textStart = 0;
    machine.textEnd = machine.textStart + machine.aoutHeader[1];
    machine.dataStart = machine.textEnd;
    machine.dataEnd = machine.dataStart + machine.aoutHeader[2];
    machine.bssStart = machine.dataEnd;
    machine.bssEnd = machine.bssStart + machine.aoutHeader[3];

    // TODO: validate
    assert(machine.aoutHeader[0] == 0x0107);
    assert(machine.aoutHeader[1] > 0);
    assert(machine.bssEnd <= 0xfffe);

    printf("magic:     0x%04x\n", machine.aoutHeader[0]);
    printf("text size: 0x%04x\n", machine.aoutHeader[1]);
    printf("data size: 0x%04x\n", machine.aoutHeader[2]);
    printf("bss  size: 0x%04x\n", machine.aoutHeader[3]);
    printf("symbol:    0x%04x\n", machine.aoutHeader[4]);
    printf("entry:     0x%04x\n", machine.aoutHeader[5]);
    printf("unused:    0x%04x\n", machine.aoutHeader[6]);
    printf("flag:      0x%04x\n", machine.aoutHeader[7]);

    // bss
    memset(&machine.virtualMemory[machine.bssStart], 0, machine.aoutHeader[3]);

    // clear regs
    machine.r0 = 0;
    machine.r1 = 0;
    machine.r2 = 0;
    machine.r3 = 0;
    machine.r4 = 0;
    machine.r5 = 0;
    machine.sp = 0;
    machine.pc = machine.textStart;
    machine.psw = 0;

    // push args
    for (int i = 1; i < argc; i++) {
        //argv[i];
    }

    //////////////////////////
    // run
    //////////////////////////
    while (1) {
        // TODO: debug
        assert(machine.pc < machine.textEnd);

        // fetch
        uint16_t addr = machine.pc;
        uint16_t bin = fetch(&machine);

        // decode
        uint16_t op = bin >> 12; // 4 bits
        uint8_t mode0 = (bin & 0x0e00) >> 9;
        uint8_t reg0 = (bin & 0x01c0) >> 6;
        uint8_t mode1 = (bin & 0x0038) >> 3;
        uint8_t reg1 = bin & 0x0007;
        uint8_t offset = bin & 0x00ff; // 8 bits
        char operand0[32];
        char operand1[32];
        if (op == 0 || op == 8) {
            if (mode0 & 4) {
                // singleOperand
                instruction_t *table;
                if (op == 0) {
                    table = singleOperand0;
                } else {
                    table = singleOperand1;
                }
                op = ((mode0 & 3) << 3) | reg0; // (2+3) bits
                operand_string(&machine, operand1, sizeof(operand1), mode1, reg1);
                printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s %s\n",
                    addr, bin,
                    machine.pc,
                    machine.sp,
                    bin,
                    table[op].mnemonic,
                    operand1);
                while (machine.pc > addr + 2) {
                    addr += 2;
                    bin = machine.virtualMemory[addr] | (machine.virtualMemory[addr+1] << 8);
                    printf("%04x %04x:\n", addr, bin);
                }
                continue;
            } else {
                // conditionalBranch
                instruction_t *table;
                if (op == 0) {
                    table = conditionalBranch0;
                    op = ((mode0 & 3) << 1) | (reg0 >> 2); // (2+1) bits
                    if (op == 0) {
                        // systemMisc
                        table = systemMisc;
                        if (reg0 == 0 && mode1 == 0) {
                            // interrupt, misc
                            op = reg1;
                            printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s\n",
                                addr, bin,
                                machine.pc,
                                machine.sp,
                                bin,
                                table[op].mnemonic);
                            continue;
                        } else if (reg0 == 1) {
                            // jmp
                            op = 8;
                            operand_string(&machine, operand1, sizeof(operand1), mode1, reg1);
                            printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s %s\n",
                                addr, bin,
                                machine.pc,
                                machine.sp,
                                bin,
                                table[op].mnemonic,
                                operand1);
                            while (machine.pc > addr + 2) {
                                addr += 2;
                                bin = machine.virtualMemory[addr] | (machine.virtualMemory[addr+1] << 8);
                                printf("%04x %04x:\n", addr, bin);
                            }
                            continue;
                        } else if (reg0 == 2) {
                            op = 9 + (mode1 >> 1);
                            if (op == 9) {
                                // subroutine
                                operand_string(&machine, operand1, sizeof(operand1), 0, reg1);
                                printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s %s\n",
                                    addr, bin,
                                    machine.pc,
                                    machine.sp,
                                    bin,
                                    table[op].mnemonic,
                                    operand1);
                                continue;
                            } else {
                                // condition
                                op = bin & 0x1f;
                                printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s\n",
                                    addr, bin,
                                    machine.pc,
                                    machine.sp,
                                    bin,
                                    toFlagName[op]);
                                continue;
                            }
                        } else if (reg0 == 3) {
                            // swab
                            op = 13;
                            operand_string(&machine, operand1, sizeof(operand1), mode1, reg1);
                            printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s %s\n",
                                addr, bin,
                                machine.pc,
                                machine.sp,
                                bin,
                                table[op].mnemonic,
                                operand1);
                            while (machine.pc > addr + 2) {
                                addr += 2;
                                bin = machine.virtualMemory[addr] | (machine.virtualMemory[addr+1] << 8);
                                printf("%04x %04x:\n", addr, bin);
                            }
                            continue;
                        } else {
                            // TODO: unknown op
                            printf("%04x %04x: pc:%04x sp:%04x bin:%06o op:%03o, %s\n",
                                addr, bin,
                                machine.pc,
                                machine.sp,
                                bin,
                                op,
                                "???");
                            //assert(0);
                            continue;
                        }
                    }
                } else {
                    table = conditionalBranch1;
                    op = ((mode0 & 3) << 1) | (reg0 >> 2); // (2+1) bits
                }
                printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s %03o / 0x%02x\n",
                    addr, bin,
                    machine.pc,
                    machine.sp,
                    bin,
                    table[op].mnemonic,
                    offset, offset << 1);
                continue;
            }

            // TODO: unknown op
            printf("%04x %04x: pc:%04x sp:%04x bin:%06o op:%03o, %s\n",
                addr, bin,
                machine.pc,
                machine.sp,
                bin,
                op,
                "???");
            assert(0);

            continue;
        }
        if (op == 7) {
            if (mode0 != 5) {
                // doubleOperand1
                op = (bin >> 9) & 7; // (4+3) bits
                if (op != 7) {
                    operand_string(&machine, operand0, sizeof(operand0), 0, reg0);
                    operand_string(&machine, operand1, sizeof(operand1), mode1, reg1);
                    printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s %s %s\n",
                        addr, bin,
                        machine.pc,
                        machine.sp,
                        bin,
                        doubleOperand1[op].mnemonic,
                        operand0,
                        operand1);
                    while (machine.pc > addr + 2) {
                        addr += 2;
                        bin = machine.virtualMemory[addr] | (machine.virtualMemory[addr+1] << 8);
                        printf("%04x %04x:\n", addr, bin);
                    }
                } else {
                    offset &= 0x3f; // 6 bits
                    operand_string(&machine, operand0, sizeof(operand0), 0, reg0);
                    printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s %s %02o / 0x%02x\n",
                        addr, bin,
                        machine.pc,
                        machine.sp,
                        bin,
                        doubleOperand1[op].mnemonic,
                        operand0,
                        offset, offset << 1);
                }
            } else {
                // floatingPoint0
                op = bin >> 9; // (4+3) bits
                printf("%04x %04x: pc:%04x sp:%04x bin:%06o op:%03o mode1:%o, %s %s %s\n",
                    addr, bin,
                    machine.pc,
                    machine.sp,
                    bin,
                    op,
                    mode1,
                    floatingPoint0[reg0].mnemonic,
                    toRegName[reg0],
                    toRegName[reg1]);
            }
            continue;
        }
        if (op == 15) {
            // floatingPoint1
            op = offset & 0xf;
            printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s\n",// %s %s\n",
                addr, bin,
                machine.pc,
                machine.sp,
                bin,
                floatingPoint1[op].mnemonic);
                //toRegName[reg0],
                //toRegName[reg1]);
            continue;
        }
        {
            // doubleOperand0
            operand_string(&machine, operand0, sizeof(operand0), mode0, reg0);
            operand_string(&machine, operand1, sizeof(operand1), mode1, reg1);
            printf("%04x %04x: pc:%04x sp:%04x bin:%06o, %s %s %s\n",
                addr, bin,
                machine.pc,
                machine.sp,
                bin,
                doubleOperand0[op].mnemonic,
                operand0,
                operand1);
            while (machine.pc > addr + 2) {
                addr += 2;
                bin = machine.virtualMemory[addr] | (machine.virtualMemory[addr+1] << 8);
                printf("%04x %04x:\n", addr, bin);
            }
            continue;
        }
    }

    return EXIT_SUCCESS;
}
