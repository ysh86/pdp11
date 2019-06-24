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
    int operandNum; // 0(setd only), 1, 2, 3, 4, 5(conditional branch), 6(syscall), 7(sob only)
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

instruction_t toFlagName[] = {
    {"nop", 0},
    {"cl" "c", 0},
    {"cl" "v", 0},
    {"cl" "vc", 0},
    {"cl" "z", 0},
    {"cl" "zc", 0},
    {"cl" "zv", 0},
    {"cl" "zvc", 0},
    {"cl" "n", 0},
    {"cl" "nc", 0},
    {"cl" "nv", 0},
    {"cl" "nvc", 0},
    {"cl" "nz", 0},
    {"cl" "nzc", 0},
    {"cl" "nzv", 0},
    {"ccc", 0},

    {"nop", 0},
    {"se" "c", 0},
    {"se" "v", 0},
    {"se" "vc", 0},
    {"se" "z", 0},
    {"se" "zc", 0},
    {"se" "zv", 0},
    {"se" "zvc", 0},
    {"se" "n", 0},
    {"se" "nc", 0},
    {"se" "nv", 0},
    {"se" "nvc", 0},
    {"se" "nz", 0},
    {"se" "nzc", 0},
    {"se" "nzv", 0},
    {"scc", 0},
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
    {"sob", 7},  // 0 111 111b:
};

instruction_t singleOperand0[] = {
    // 004r
    {"jsr", 3},  // 0 000 100 000b: r0
    {"jsr", 3},  // 0 000 100 001b: r1
    {"jsr", 3},  // 0 000 100 010b: r2
    {"jsr", 3},  // 0 000 100 011b: r3
    {"jsr", 3},  // 0 000 100 100b: r4
    {"jsr", 3},  // 0 000 100 101b: r5
    {"jsr", 3},  // 0 000 100 110b: sp
    {"jsr", 3},  // 0 000 100 111b: pc

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
    {"sys", 6},  // 1 000 100 100b:
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
    {"br", 5},   // 0 000 000 1b:
    {"bne", 5},  // 0 000 001 0b:
    {"beq", 5},  // 0 000 001 1b:
    {"bge", 5},  // 0 000 010 0b:
    {"blt", 5},  // 0 000 010 1b:
    {"bgt", 5},  // 0 000 011 0b:
    {"ble", 5},  // 0 000 011 1b:
};

instruction_t conditionalBranch1[] = {
    {"bpl", 5},  // 1 000 000 0b:
    {"bmi", 5},  // 1 000 000 1b:
    {"bhi", 5},  // 1 000 001 0b:
    {"blos", 5}, // 1 000 001 1b:
    {"bvc", 5},  // 1 000 010 0b:
    {"bvs", 5},  // 1 000 010 1b:
    {"bcc or bhis", 5}, // 1 000 011 0b:
    {"bcs or blo", 5},  // 1 000 011 1b:
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

    //
    // CPU internal
    //

    // fetch
    uint16_t addr;
    uint16_t bin;

    // decode
    // double:
    // 4. op(4bits) mode0(3bits) reg0(3bits) mode1(3bits) reg1(3bits)
    // 3. op(7bits)              reg0(3bits) mode1(3bits) reg1(3bits)
    // 7. op(7bits)              reg0(3bits) offset(6bits)
    // single:
    // 2. op(10bits) mode1(3bits) reg1(3bits)
    // 1. op(13bits)              reg1(3bits)
    // 5. op(8bits)  offset(8bits)
    // 6. op(10bits) syscallID(6bits)
    // misc:
    // 0. op(16bits)
    uint16_t op;
    uint8_t mode0;
    uint8_t reg0;
    uint8_t mode1;
    uint8_t reg1;
    uint8_t offset;
    uint8_t syscallID;
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

void syscall_string(machine_t *pm, char *str, size_t size, uint8_t id) {
    uint16_t word0 = 0;
    uint16_t word1 = 0;
    switch (id) {
    case 0:
        // indir
        word0 = fetch(pm);
        snprintf(str, size, "0; 0x%04x", word0);
        break;
    case 1:
        // exit
        snprintf(str, size, "exit");
        break;
    case 4:
        // write
        word0 = fetch(pm);
        word1 = fetch(pm);
        snprintf(str, size, "write; 0x%04x; 0x%04x", word0, word1);
        break;
    default:
        // TODO: not implemented
        assert(0);
        break;
    }
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
            snprintf(str, size, "$0x%04x", word & 0xffff);
        }
        break;
    case 3:
        if (reg != 7) {
            snprintf(str, size, "*(%s)+", rn);
        } else {
            // pc
            word = fetch(pm);
            snprintf(str, size, "%04x", word & 0xffff);
        }
        break;
    case 4:
        snprintf(str, size, "-(%s)", rn);
        break;
    case 5:
        snprintf(str, size, "*-(%s)", rn);
        break;
    case 6:
        word = fetch(pm);
        if (reg != 7) {
            snprintf(str, size, "%d(%s)", word, rn);
        } else {
            // pc
            snprintf(str, size, "%04x", (word + (int16_t)(pm->pc)) & 0xffff);
        }
        break;
    case 7:
        word = fetch(pm);
        if (reg != 7) {
            snprintf(str, size, "*%d(%s)", word, rn);
        } else {
            // pc
            snprintf(str, size, "*0x%04x", (word + (int16_t)(pm->pc)) & 0xffff);
        }
        break;
    default:
        assert(0);
        break;
    }
}

void disasm(machine_t *pm, const instruction_t *instTable) {
    char operand0[32] = {'\0'};
    char operand1[32] = {'\0'};
    char *sep = "";
    char *tabs = "";

    if (instTable[pm->op].operandNum == 4) {
        // doubleOperand0
        operand_string(pm, operand0, sizeof(operand0), pm->mode0, pm->reg0);
        operand_string(pm, operand1, sizeof(operand1), pm->mode1, pm->reg1);
        sep = ",";
        tabs = "\t";
    } else if (instTable[pm->op].operandNum == 3) {
        // doubleOperand1, jsr
        operand_string(pm, operand0, sizeof(operand0), 0, pm->reg0);
        operand_string(pm, operand1, sizeof(operand1), pm->mode1, pm->reg1);
        sep = ",";
        tabs = "\t\t";
    } else if (instTable[pm->op].operandNum == 7) {
        // doubleOperand1 sob only
        operand_string(pm, operand0, sizeof(operand0), 0, pm->reg0);
        snprintf(operand1, sizeof(operand1), "%04x", pm->pc - (pm->offset << 1));
        sep = ",";
        tabs = "\t\t";
    } else if (instTable[pm->op].operandNum == 2) {
        // singleOperand0, singleOperand1, jmp, swab
        operand_string(pm, operand1, sizeof(operand1), pm->mode1, pm->reg1);
        sep = "";
        tabs = "\t\t";
    } else if (instTable[pm->op].operandNum == 1) {
        // subroutine
        operand_string(pm, operand1, sizeof(operand1), 0, pm->reg1);
        sep = "";
        tabs = "\t\t";
    } else if (instTable[pm->op].operandNum == 5) {
        // conditionalBranch0, conditionalBranch1
        snprintf(operand1, sizeof(operand1), "%04x", pm->pc + ((int8_t)pm->offset << 1));
        sep = "";
        tabs = "\t\t";
    } else if (instTable[pm->op].operandNum == 6) {
        // syscall
        syscall_string(pm, operand1, sizeof(operand1), pm->syscallID);
        sep = "";
        tabs = "\t\t";
    } else if (instTable[pm->op].operandNum == 0) {
        // floatingPoint1, systemMisc
        sep = "";
        tabs = "\t\t\t";
    } else {
        // TODO: unknown op
        assert(0);
    }

    printf("%04x: %s\t%s%s%s%s/ bin:%04x, bin:%06o, pc:%04x, sp:%04x\n",
        pm->addr,
        instTable[pm->op].mnemonic,
        operand0,
        sep,
        operand1,
        tabs,
        pm->bin,
        pm->bin,
        pm->pc,
        pm->sp);
    while (pm->pc > pm->addr + 2) {
        pm->addr += 2;
        pm->bin = pm->virtualMemory[pm->addr] | (pm->virtualMemory[pm->addr+1] << 8);
        printf("%04x: /\t%04x\n", pm->addr, pm->bin);
    }

    // syscall indir
    if (instTable[pm->op].operandNum == 6 && pm->syscallID == 0) {
        uint16_t oldpc = pm->pc;
        pm->pc = oldpc - 2;
        uint16_t addr = fetch(pm);
        {
            pm->pc = addr;
            pm->bin = fetch(pm);
            pm->syscallID = pm->bin & 0x3f;
            assert(pm->bin - pm->syscallID == 0104400);
            syscall_string(pm, operand1, sizeof(operand1), pm->syscallID);
        }
        pm->pc = oldpc;

        printf(".data\n");
        printf("%04x: sys\t%s\n", addr, operand1);
        printf(".text\n");
    }
}

int main(int argc, char *argv[]) {
    //////////////////////////
    // usage
    //////////////////////////
    if (argc < 2) {
        fprintf(stderr, "Usage: pdp11 aout\n");
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

    printf("/ aout header\n");
    printf("/\n");
    printf("/ magic:     0x%04x\n", machine.aoutHeader[0]);
    printf("/ text size: 0x%04x\n", machine.aoutHeader[1]);
    printf("/ data size: 0x%04x\n", machine.aoutHeader[2]);
    printf("/ bss  size: 0x%04x\n", machine.aoutHeader[3]);
    printf("/ symbol:    0x%04x\n", machine.aoutHeader[4]);
    printf("/ entry:     0x%04x\n", machine.aoutHeader[5]);
    printf("/ unused:    0x%04x\n", machine.aoutHeader[6]);
    printf("/ flag:      0x%04x\n", machine.aoutHeader[7]);
    printf("\n");

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
        //assert(machine.pc < machine.textEnd);
        if (machine.pc >= machine.textEnd) {
            break;
        }

        // fetch
        machine.addr = machine.pc;
        machine.bin = fetch(&machine);

        // decode
        machine.op = machine.bin >> 12;
        machine.mode0 = (machine.bin & 0x0e00) >> 9;
        machine.reg0 = (machine.bin & 0x01c0) >> 6;
        machine.mode1 = (machine.bin & 0x0038) >> 3;
        machine.reg1 = machine.bin & 0x0007;
        machine.offset = machine.bin & 0x00ff;
        machine.syscallID = machine.bin & 0x003f;
        uint8_t op_temp = machine.op & 7;
        if (op_temp != 0 && op_temp != 7) {
            disasm(&machine, doubleOperand0);
            continue;
        }
        if (machine.op == 7) {
            if (machine.mode0 != 5) {
                machine.op = (machine.bin >> 9) & 7; // (4+3) bits
                if (machine.op != 7) {
                    disasm(&machine, doubleOperand1);
                } else {
                    // sob
                    machine.offset &= 0x3f; // 6 bits
                    disasm(&machine, doubleOperand1);
                }
            } else {
                machine.op = machine.bin >> 9; // (4+3) bits
                //disasm(&machine, floatingPoint0);
                // TODO: not implemented
                assert(0);
            }
            continue;
        }
        if (machine.op == 15) {
            machine.op = machine.offset & 0xf; // 4 bits
            disasm(&machine, floatingPoint1);
            continue;
        }
        if (machine.op == 0 || machine.op == 8) {
            if (machine.mode0 & 4) {
                instruction_t *table;
                if (machine.op == 0) {
                    table = singleOperand0;
                } else {
                    table = singleOperand1;
                }
                machine.op = ((machine.mode0 & 3) << 3) | machine.reg0; // (2+3) bits
                disasm(&machine, table);
                continue;
            } else {
                // conditionalBranch
                instruction_t *table;
                if (machine.op == 0) {
                    table = conditionalBranch0;
                    machine.op = ((machine.mode0 & 3) << 1) | (machine.reg0 >> 2); // (2+1) bits
                    if (machine.op == 0) {
                        // systemMisc
                        table = systemMisc;
                        if (machine.reg0 == 0 && machine.mode1 == 0) {
                            // interrupt, misc
                            machine.op = machine.reg1;
                        } else if (machine.reg0 == 1) {
                            // jmp
                            machine.op = 8;
                        } else if (machine.reg0 == 2) {
                            machine.op = 9 + (machine.mode1 >> 1);
                            if (machine.op == 9) {
                                // subroutine
                            } else {
                                // condition
                                table = toFlagName;
                                machine.op = machine.bin & 0x1f;
                            }
                        } else if (machine.reg0 == 3) {
                            // swab
                            machine.op = 13;
                        } else {
                            // TODO: unknown op
                            assert(0);
                        }
                    }
                } else {
                    table = conditionalBranch1;
                    machine.op = ((machine.mode0 & 3) << 1) | (machine.reg0 >> 2); // (2+1) bits
                }
                disasm(&machine, table);
                continue;
            }

            // TODO: unknown op
            assert(0);

            continue;
        }
    }

    return EXIT_SUCCESS;
}
