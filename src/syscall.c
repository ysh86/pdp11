#include <stdio.h>
#include <assert.h>

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
        fprintf(stderr, "exit ");
        break;
    case 4:
        // write
        word0 = fetch(pm);
        word1 = fetch(pm);
        fprintf(stderr, "write; 0x%04x; 0x%04x ", word0, word1);
        break;
    default:
        // TODO: not implemented
        assert(0);
        break;
    }
}
