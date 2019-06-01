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

instruction_t doubleOperand0[] = {
    {"", 0},     // 0 000b: singleOperand0[], conditionalBranch0[], conditionCode[]
    {"mov", 4},  // 0 001b:
    {"cmp", 4},  // 0 010b:
    {"bit", 4},  // 0 011b:
    {"bic", 4},  // 0 100b:
    {"bis", 4},  // 0 101b:
    {"add", 4},  // 0 110b:
    {"", 3},     // 0 111b: doubleOperand1[]

    {"", 0},     // 1 000b: singleOperand1[], conditionalBranch1[]
    {"movb", 4}, // 1 001b:
    {"cmpb", 4}, // 1 010b:
    {"bitb", 4}, // 1 011b:
    {"bicb", 4}, // 1 100b:
    {"bisb", 4}, // 1 101b:
    {"sub", 4},  // 1 110b:
    {"", 0},     // 1 111b: dummy
};

instruction_t doubleOperand1[] = {
    {"mul", 3},  // 0 111 000b:
    {"div", 3},  // 0 111 001b:
    {"ash", 3},  // 0 111 010b:
    {"ashc", 3}, // 0 111 011b:
    {"xor", 3},  // 0 111 100b:
    {"", 0},     // 0 111 101b: float
    {"", 0},     // 0 111 110b: system
    {"sob", 3},  // 0 111 111b:
};

instruction_t singleOperand0[] = {
    // TODO: 0003 swab
    // 004r
    {"jsr", 2},  // 0 000 100 000b:
    {"jsr", 2},  // 0 000 100 001b:
    {"jsr", 2},  // 0 000 100 010b:
    {"jsr", 2},  // 0 000 100 011b:
    {"jsr", 2},  // 0 000 100 100b:
    {"jsr", 2},  // 0 000 100 101b:
    {"jsr", 2},  // 0 000 100 110b:
    {"jsr", 2},  // 0 000 100 111b:
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
};

instruction_t singleOperand1[] = {
    // 104x
    {"emt", 2},  // 1 000 100 000b:
    {"emt", 2},  // 1 000 100 001b:
    {"emt", 2},  // 1 000 100 010b:
    {"emt", 2},  // 1 000 100 011b:
    {"emt", 2},  // 1 000 100 100b:
    {"emt", 2},  // 1 000 100 101b:
    {"emt", 2},  // 1 000 100 110b:
    {"emt", 2},  // 1 000 100 111b:
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
};

instruction_t conditionalBranch0[] = {
    {"", 0},     // 0 000 000 0b: system instructions?, conditionCode[]
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

instruction_t conditionCode[] = {
    {"", 0}, // 0 000 000 010 100 000b:

    {"", 0}, // 0 000 000 010 110 000b:
};

typedef struct machine_t_tag {
    // env
    char rootdir[PATH_MAX];
    char curdir[PATH_MAX];
    uint16_t aoutHeader[8]; // little endian only

    // memory
    uint8_t virtualMemory[64 * 1024];

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

void push(machine_t *pm, uint16_t value) {
    pm->virtualMemory[pm->sp] = value;
    pm->sp -= 2;
}

uint16_t pop(machine_t *pm) {
    pm->sp += 2;
    return pm->virtualMemory[pm->sp];
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
    // TODO: validate
    assert(machine.aoutHeader[0] == 0x0107);
    assert(machine.aoutHeader[1] > 0);
    assert(0 + machine.aoutHeader[1] + machine.aoutHeader[2] + machine.aoutHeader[3] <= 0xfffe);

    printf("magic:     0x%04x\n", machine.aoutHeader[0]);
    printf("text size: 0x%04x\n", machine.aoutHeader[1]);
    printf("data size: 0x%04x\n", machine.aoutHeader[2]);
    printf("bss  size: 0x%04x\n", machine.aoutHeader[3]);
    printf("symbol:    0x%04x\n", machine.aoutHeader[4]);
    printf("entry:     0x%04x\n", machine.aoutHeader[5]);
    printf("unused:    0x%04x\n", machine.aoutHeader[6]);
    printf("flag:      0x%04x\n", machine.aoutHeader[7]);

    // bss
    size_t begin = 0 + machine.aoutHeader[1] + machine.aoutHeader[2];
    memset(&machine.virtualMemory[begin], 0, machine.aoutHeader[3]);

    // TODO: heap pointer?

    // clear regs
    machine.r0 = 0;
    machine.r1 = 0;
    machine.r2 = 0;
    machine.r3 = 0;
    machine.r4 = 0;
    machine.r5 = 0;
    machine.sp = 0xfffe;
    machine.pc = 0;
    machine.psw = 0;

    // push args
    for (int i = 1; i < argc; i++) {
        argv[i];
    }

    //////////////////////////
    // run
    //////////////////////////
    while (1) {
        // fetch
        uint16_t bin = machine.virtualMemory[machine.pc] | (machine.virtualMemory[machine.pc + 1] << 8);

        // decode
        uint16_t op = bin >> 12; // 4 bits
        uint8_t mode0 = (bin & 0x0e00) >> 9;
        uint8_t reg0 = (bin & 0x01c0) >> 6;
        uint8_t mode1 = (bin & 0x0038) >> 3;
        uint8_t reg1 = bin & 0x0007;
        uint8_t offset = bin & 0x00ff; // 8 bits
        if (op == 7) {
            op = bin >> 9; // (4+3) bits
        }
        if (op == 0 || op == 8) {
            if (bin & 0x0800) {
                // Single-operand
                op = bin >> 6; // (1+4+5) bits
            } else {
                // Conditional branch
                op = bin >> 8; // (1+4+3) bits
            }
        }
        printf("pc:%04x sp:%04x op:%02x asm:%s\n", machine.pc, machine.sp, op, "???");

        // TODO: debug
        //assert(machine.pc < 0xfffe);
        assert(machine.pc < 0x2000);

        machine.pc += 2;
    }

    return EXIT_SUCCESS;
}
