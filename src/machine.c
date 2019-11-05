#include "machine.h"
#include "util.h"

void setRegPtr(machine_t *pm) {
    pm->r[0] = &pm->r0;
    pm->r[1] = &pm->r1;
    pm->r[2] = &pm->r2;
    pm->r[3] = &pm->r3;
    pm->r[4] = &pm->r4;
    pm->r[5] = &pm->r5;
    pm->r[6] = &pm->sp;
    pm->r[7] = &pm->pc;
}

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

uint16_t fetch(machine_t *pm) {
    uint16_t bin = read16(false, &pm->virtualMemory[pm->pc]);
    pm->pc += 2;
    return bin;
}
