#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "machine.h"
#include "inst.h"
#include "util.h"

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
            snprintf(str, size, "%04x", word & 0xffff); // TODO: disasm
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
            snprintf(str, size, "%04x", (word + (int16_t)(pm->pc)) & 0xffff); // TODO: disasm
        }
        break;
    case 7:
        word = fetch(pm);
        if (reg != 7) {
            snprintf(str, size, "*%d(%s)", word, rn);
        } else {
            // pc
            snprintf(str, size, "*0x%04x", (word + (int16_t)(pm->pc)) & 0xffff); // TODO: disasm
        }
        break;
    default:
        assert(0);
        break;
    }
}

void disasm(machine_t *pm) {
    char operand0[32] = {'\0'};
    char operand1[32] = {'\0'};
    char *sep = "";
    char *tabs = "";

    if (pm->inst->operandNum == 4) {
        // doubleOperand0
        operand_string(pm, operand0, sizeof(operand0), pm->mode0, pm->reg0);
        operand_string(pm, operand1, sizeof(operand1), pm->mode1, pm->reg1);
        sep = ",";
        tabs = "\t";
    } else if (pm->inst->operandNum == 3) {
        // doubleOperand1, jsr
        operand_string(pm, operand0, sizeof(operand0), 0, pm->reg0);
        operand_string(pm, operand1, sizeof(operand1), pm->mode1, pm->reg1);
        sep = ",";
        tabs = "\t\t";
    } else if (pm->inst->operandNum == 7) {
        // doubleOperand1 sob only
        operand_string(pm, operand0, sizeof(operand0), 0, pm->reg0);
        snprintf(operand1, sizeof(operand1), "%04x", pm->pc - (pm->offset << 1));
        sep = ",";
        tabs = "\t\t";
    } else if (pm->inst->operandNum == 8) {
        // doubleOperand1 mul,div,ash,ashc
        operand_string(pm, operand0, sizeof(operand0), pm->mode1, pm->reg1);
        operand_string(pm, operand1, sizeof(operand1), 0, pm->reg0);
        sep = ",";
        tabs = "\t\t";
    } else if (pm->inst->operandNum == 2) {
        // singleOperand0, singleOperand1, jmp, swab
        operand_string(pm, operand1, sizeof(operand1), pm->mode1, pm->reg1);
        sep = "";
        tabs = "\t\t";
    } else if (pm->inst->operandNum == 1) {
        // subroutine
        operand_string(pm, operand1, sizeof(operand1), 0, pm->reg1);
        sep = "";
        tabs = "\t\t";
    } else if (pm->inst->operandNum == 5) {
        // conditionalBranch0, conditionalBranch1
        snprintf(operand1, sizeof(operand1), "%04x", pm->pc + ((int8_t)pm->offset << 1));
        sep = "";
        tabs = "\t\t";
    } else if (pm->inst->operandNum == 6) {
        // syscall
        syscall_string(pm, operand1, sizeof(operand1), pm->syscallID);
        sep = "";
        tabs = "\t\t";
    } else if (pm->inst->operandNum == 0) {
        // floatingPoint1, systemMisc
        sep = "";
        tabs = "\t\t\t";
    } else {
        // TODO: unknown op
        assert(0);
    }

    printf("%04x: %s\t%s%s%s%s/ bin:%04x, bin:%06o, pc:%04x, sp:%04x\n",
        pm->addr,
        pm->inst->mnemonic,
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
    if (pm->inst->operandNum == 6 && pm->syscallID == 0) {
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
    setRegPtr(&machine);

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
    // TODO: endian

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

    // calc size of args
    uint16_t na = argc - 1;
    uint16_t nc = 0;
    for (int i = 1; i < argc; i++) {
        char *pa = argv[i];
        do {
            nc++;
        } while (*pa++ != '\0');
    }
    if (nc & 1) {
        nc++;
    }
    machine.sp -= 2 + na * 2 + 2 + nc; // argc, argv[0]...argv[na-1], -1, buf

    // push args
    uint8_t *sp = &machine.virtualMemory[machine.sp];
    char *pbuf = (char *)(sp + 2 + na * 2 + 2);
    write16(false, sp, na); // argc
    sp += 2;
    for (int i = 1; i < argc; i++) { // argv & buf
        char *pa = argv[i];
        uint16_t addr = (uintptr_t)pbuf - (uintptr_t)machine.virtualMemory;
        write16(false, sp, addr);
        sp += 2;
        do {
            *pbuf++ = *pa;
        } while (*pa++ != '\0');
    }
    uint16_t addr = (uintptr_t)pbuf - (uintptr_t)machine.virtualMemory;
    if (addr & 1) {
        *pbuf = '\0'; // alignment
    }
    write16(false, sp, 0xffff); // -1

#if 0
    // debug dump
    printf("/ stack: sp = %04x\n", machine.sp);
    int maxj = sizeof(machine.virtualMemory);
    for (int j = maxj - 256; j < maxj; j += 16) {
        printf("/ %04x:", j);
        for (int i = 0; i < 16; i++) {
            printf(" %02x", machine.virtualMemory[j + i]);
        }
        printf("\n");
    }
    printf("\n");
#endif

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
        machine.inst = NULL;
        machine.isByte = false;
        machine.isEven = false;
        machine.mode0 = (machine.bin & 0x0e00) >> 9;
        machine.reg0 = (machine.bin & 0x01c0) >> 6;
        machine.mode1 = (machine.bin & 0x0038) >> 3;
        machine.reg1 = machine.bin & 0x0007;
        machine.offset = machine.bin & 0x00ff;
        machine.syscallID = machine.bin & 0x003f;
        uint16_t op = machine.bin >> 12;
        instruction_t *table = NULL;
        do {
            uint8_t op_temp = op & 7;
            if (op_temp != 0 && op_temp != 7) {
                table = doubleOperand0;
                if (op & 8 && op_temp != 6) {
                    machine.isByte = true;
                }
                break;
            }
            if (op == 7) {
                if (machine.mode0 != 5) {
                    table = doubleOperand1;
                    op = (machine.bin >> 9) & 7; // (4+3) bits
                    if (op == 7) {
                        // sob
                        machine.offset &= 0x3f; // 6 bits positive num
                    } else {
                        if ((machine.reg0 & 1) == 0) {
                            machine.isEven = true;
                        }
                    }
                } else {
                    table = floatingPoint0;
                    op = machine.bin >> 9; // (4+3) bits
                    // TODO: not implemented
                    assert(0);
                }
                break;
            }
            if (op == 15) {
                table = floatingPoint1;
                op = machine.offset & 0xf; // 4 bits
                break;
            }
            if (op == 0 || op == 8) {
                if (machine.mode0 & 4) {
                    if (op == 0) {
                        table = singleOperand0;
                    } else {
                        table = singleOperand1;
                        if (machine.mode0 == 5) {
                            machine.isByte = true;
                        }
                    }
                    op = ((machine.mode0 & 3) << 3) | machine.reg0; // (2+3) bits
                } else {
                    // conditionalBranch
                    if (op == 0) {
                        table = conditionalBranch0;
                        op = ((machine.mode0 & 3) << 1) | (machine.reg0 >> 2); // (2+1) bits
                        if (op == 0) {
                            // systemMisc
                            table = systemMisc;
                            if (machine.reg0 == 0 && machine.mode1 == 0) {
                                // interrupt, misc
                                op = machine.reg1;
                            } else if (machine.reg0 == 1) {
                                // jmp
                                op = 8;
                            } else if (machine.reg0 == 2) {
                                op = 9 + (machine.mode1 >> 1);
                                if (op == 9) {
                                    // subroutine
                                } else {
                                    // condition
                                    table = clearSet;
                                    op = machine.bin & 0x1f;
                                }
                            } else if (machine.reg0 == 3) {
                                // swab
                                op = 13;
                            } else {
                                // TODO: unknown op
                                assert(0);
                            }
                        }
                    } else {
                        table = conditionalBranch1;
                        op = ((machine.mode0 & 3) << 1) | (machine.reg0 >> 2); // (2+1) bits
                    }
                }
                break;
            }
        } while(0);

        if (table == NULL || table[op].mnemonic == NULL) {
            // TODO: unknown op or not implemented
            assert(0);
        }
        machine.inst = &table[op];

        // exec
        exec(&machine);
        //disasm(&machine);
    }

    return EXIT_SUCCESS;
}
