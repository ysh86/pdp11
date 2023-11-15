#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SIZE_OF_VECTORS 0

struct instruction_tag;
#ifndef _INSTRUCTION_T_
#define _INSTRUCTION_T_
typedef struct instruction_tag instruction_t;
#endif

typedef void * context_t;
typedef uint8_t *(*mmu_v2r_t)(context_t ctx, uint16_t vaddr);
typedef uint16_t (*mmu_r2v_t)(context_t ctx, uint8_t *raddr);
typedef void (*syscall_t)(context_t ctx);
typedef void (*syscall_string_t)(context_t ctx, char *str, size_t size, uint8_t id);

struct cpu_tag {
    // machine
    context_t ctx;
    mmu_v2r_t mmuV2R;
    mmu_r2v_t mmuR2V;
    syscall_t syscallHook;
    syscall_string_t syscallStringHook;

    // regs
    uint16_t r0;
    uint16_t r1;
    uint16_t r2;
    uint16_t r3;
    uint16_t r4;
    uint16_t r5;
    uint16_t sp; // r6
    uint16_t pc; // r7
    uint16_t psw; // N:negative, Z:zero, V:overflow, C:carry

    // reg ptr
    uint16_t *r[8]; // r[6]:sp, r[7]:pc

    //
    // internal
    //

    // fetch
    uint16_t addr;
    uint16_t bin;

    // decode
    // double:
    // 4. op(4bits) mode0(3bits) reg0(3bits) mode1(3bits) reg1(3bits)
    // 3. op(7bits)              reg0(3bits) mode1(3bits) reg1(3bits)
    // 7. op(7bits)              reg0(3bits) offset(6bits)
    // 8. op(7bits)              reg0(3bits) mode1(3bits) reg1(3bits)
    // single:
    // 2. op(10bits) mode1(3bits) reg1(3bits)
    // 1. op(13bits)              reg1(3bits)
    // 5. op(8bits)  offset(8bits)
    // 6. op(10bits) syscallID(6bits)
    // misc:
    // 0. op(16bits)
    instruction_t *inst;
    bool isByte;
    bool isEven;
    uint8_t mode0;
    uint8_t reg0;
    uint8_t mode1;
    uint8_t reg1;
    uint8_t offset;
    uint8_t syscallID;

    // exec
    uint8_t *operand0;
    uint8_t *operand1;
};
#ifndef _CPU_T_
#define _CPU_T_
typedef struct cpu_tag cpu_t;
#endif

extern char *toRegName[];

#define PSW_N (1<<3) // negative
#define PSW_Z (1<<2) // zero
#define PSW_V (1<<1) // overflow
#define PSW_C (1<<0) // carry
static inline void setN(cpu_t *pcpu) {
    pcpu->psw |= PSW_N;
}
static inline void setZ(cpu_t *pcpu) {
    pcpu->psw |= PSW_Z;
}
static inline void setV(cpu_t *pcpu) {
    pcpu->psw |= PSW_V;
}
static inline void setC(cpu_t *pcpu) {
    pcpu->psw |= PSW_C;
}

static inline void clearN(cpu_t *pcpu) {
    pcpu->psw &= ~PSW_N;
}
static inline void clearZ(cpu_t *pcpu) {
    pcpu->psw &= ~PSW_Z;
}
static inline void clearV(cpu_t *pcpu) {
    pcpu->psw &= ~PSW_V;
}
static inline void clearC(cpu_t *pcpu) {
    pcpu->psw &= ~PSW_C;
}

static inline bool isN(const cpu_t *pcpu) {
    return (pcpu->psw & PSW_N);
}
static inline bool isZ(const cpu_t *pcpu) {
    return (pcpu->psw & PSW_Z);
}
static inline bool isV(const cpu_t *pcpu) {
    return (pcpu->psw & PSW_V);
}
static inline bool isC(const cpu_t *pcpu) {
    return (pcpu->psw & PSW_C);
}

void init(
    cpu_t *pcpu,
    context_t ctx,
    mmu_v2r_t v2r,
    mmu_r2v_t r2v,
    syscall_t syscallHook,
    syscall_string_t syscallStringHook,
    uint16_t sp, uint16_t pc);

uint16_t getSP(cpu_t *pcpu);
uint16_t getPC(cpu_t *pcpu);

uint16_t fetch(cpu_t *pcpu);
void decode(cpu_t *pcpu);
void exec(cpu_t *pcpu);

void disasm(cpu_t *pcpu);
