#include "cpu.h"
#include "util.h"

uint16_t pushArgs(cpu_t *pcpu, int argc, uint8_t *args, size_t argsbytes) {
    // argc, argv[0]...argv[na-1], -1, buf
    const uint16_t na = argc;
    const uint16_t vsp = pcpu->sp - (2 + na * 2 + 2 + argsbytes);
    uint8_t *rsp = pcpu->mmuV2R(pcpu->ctx, vsp);
    uint8_t *pbuf = rsp + 2 + na * 2 + 2;

    // argc
    write16(false, rsp, na);
    rsp += 2;

    // argv & buf
    const uint8_t *pa = args;
    for (int i = 0; i < na; i++) {
        uint16_t vaddr = pcpu->mmuR2V(pcpu->ctx, pbuf);
        write16(false, rsp, vaddr);
        rsp += 2;
        do {
            *pbuf++ = *pa;
        } while (*pa++ != '\0');
    }

    uint16_t vaddr = pcpu->mmuR2V(pcpu->ctx, pbuf);
    if (vaddr & 1) {
        *pbuf = '\0'; // alignment
    }

    // -1
    write16(false, rsp, 0xffff);

    pcpu->sp = vsp;
    return vsp;
}
