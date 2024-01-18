#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#include "cpu.h"
#include "inst.h"
#include "util.h"

static void operand_string(cpu_t *pcpu, char *str, size_t size, uint8_t mode, uint8_t reg) {
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
            word = fetch(pcpu);
            snprintf(str, size, "$0x%04x", word & 0xffff);
        }
        break;
    case 3:
        if (reg != 7) {
            snprintf(str, size, "*(%s)+", rn);
        } else {
            // pc
            word = fetch(pcpu);
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
        word = fetch(pcpu);
        if (reg != 7) {
            snprintf(str, size, "%d(%s)", word, rn);
        } else {
            // pc
            snprintf(str, size, "%04x", (word + (int16_t)(pcpu->pc)) & 0xffff); // TODO: disasm
        }
        break;
    case 7:
        word = fetch(pcpu);
        if (reg != 7) {
            snprintf(str, size, "*%d(%s)", word, rn);
        } else {
            // pc
            snprintf(str, size, "*0x%04x", (word + (int16_t)(pcpu->pc)) & 0xffff); // TODO: disasm
        }
        break;
    default:
        assert(0);
        break;
    }
}

void disasm(cpu_t *pcpu) {
    char operand0[32] = {'\0'};
    char operand1[32] = {'\0'};
    char *sep = "";
    char *tabs = "";

    if (pcpu->inst->operandNum == 4) {
        // doubleOperand0
        operand_string(pcpu, operand0, sizeof(operand0), pcpu->mode0, pcpu->reg0);
        operand_string(pcpu, operand1, sizeof(operand1), pcpu->mode1, pcpu->reg1);
        sep = ",";
        tabs = "\t";
    } else if (pcpu->inst->operandNum == 3) {
        // doubleOperand1, jsr
        operand_string(pcpu, operand0, sizeof(operand0), 0, pcpu->reg0);
        operand_string(pcpu, operand1, sizeof(operand1), pcpu->mode1, pcpu->reg1);
        sep = ",";
        tabs = "\t\t";
    } else if (pcpu->inst->operandNum == 7) {
        // doubleOperand1 sob only
        operand_string(pcpu, operand0, sizeof(operand0), 0, pcpu->reg0);
        snprintf(operand1, sizeof(operand1), "%04x", pcpu->pc - (pcpu->offset << 1));
        sep = ",";
        tabs = "\t\t";
    } else if (pcpu->inst->operandNum == 8) {
        // doubleOperand1 mul,div,ash,ashc
        operand_string(pcpu, operand0, sizeof(operand0), pcpu->mode1, pcpu->reg1);
        operand_string(pcpu, operand1, sizeof(operand1), 0, pcpu->reg0);
        sep = ",";
        tabs = "\t\t";
    } else if (pcpu->inst->operandNum == 2) {
        // singleOperand0, singleOperand1, jmp, swab
        operand_string(pcpu, operand1, sizeof(operand1), pcpu->mode1, pcpu->reg1);
        sep = "";
        tabs = "\t\t";
    } else if (pcpu->inst->operandNum == 1) {
        // subroutine
        operand_string(pcpu, operand1, sizeof(operand1), 0, pcpu->reg1);
        sep = "";
        tabs = "\t\t";
    } else if (pcpu->inst->operandNum == 5) {
        // conditionalBranch0, conditionalBranch1
        snprintf(operand1, sizeof(operand1), "%04x", pcpu->pc + ((int8_t)pcpu->offset << 1));
        sep = "";
        tabs = "\t\t";
    } else if (pcpu->inst->operandNum == 6) {
        // syscall
        syscallString16(pcpu->ctx, operand1, sizeof(operand1), pcpu->syscallID);
        sep = "";
        tabs = "\t\t";
    } else if (pcpu->inst->operandNum == 0) {
        // floatingPoint1, systemMisc
        sep = "";
        tabs = "\t\t\t";
    } else {
        // TODO: unknown op
        assert(0);
    }

    printf("%04x: %s\t%s%s%s%s/ bin:%04x, bin:%06o, pc:%04x, sp:%04x\n",
        pcpu->addr,
        pcpu->inst->mnemonic,
        operand0,
        sep,
        operand1,
        tabs,
        pcpu->bin,
        pcpu->bin,
        pcpu->pc,
        pcpu->sp);
    while (pcpu->pc > pcpu->addr + 2) {
        pcpu->addr += 2;
        pcpu->bin = read16(false, pcpu->mmuV2R(pcpu->ctx, pcpu->addr));
        printf("%04x: /\t%04x\n", pcpu->addr, pcpu->bin);
    }

    // syscall indir
    if (pcpu->inst->operandNum == 6 && pcpu->syscallID == 0) {
        uint16_t oldpc = pcpu->pc;
        pcpu->pc = oldpc - 2;
        uint16_t addr = fetch(pcpu);
        {
            pcpu->pc = addr;
            pcpu->bin = fetch(pcpu);
            pcpu->syscallID = pcpu->bin & 0x3f;
            assert(pcpu->bin - pcpu->syscallID == 0104400);
            syscallString16(pcpu->ctx, operand1, sizeof(operand1), pcpu->syscallID);
        }
        pcpu->pc = oldpc;

        printf(".data\n");
        printf("%04x: sys\t%s\n", addr, operand1);
        printf(".text\n");
    }
}
