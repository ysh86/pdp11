#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

#include "syscall.h"
#include "machine.h"

void syscall(machine_t *pm) {
    uint16_t word0 = 0;
    uint16_t word1 = 0;
    switch (pm->syscallID) {
    case 0:
        // indir
        assert(0);
        break;
    case 1:
        // exit
        _exit((int16_t)pm->r0);
        break;
    case 4:
        // write
        word0 = fetch(pm);
        word1 = fetch(pm);
        pm->r0 = write((int16_t)pm->r0, &pm->virtualMemory[word0], word1) & 0xffff;
        break;
    default:
        // TODO: not implemented
        assert(0);
        break;
    }
}
