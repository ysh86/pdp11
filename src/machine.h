#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <dirent.h>

// for PATH_MAX
#ifdef __linux__
#include <linux/limits.h>
#endif
#ifdef __APPLE__
#include <sys/syslimits.h>
#endif

struct instruction_tag;
#ifndef _INSTRUCTION_T_
#define _INSTRUCTION_T_
typedef struct instruction_tag instruction_t;
#endif

struct machine_tag {
    // env
    char rootdir[PATH_MAX];
    char curdir[PATH_MAX];
    int argc;
    uint8_t args[512];
    uint16_t argsbytes;
    uint16_t aoutHeader[8];

    // emulate opendir, closedir and readdir
    int dirfd;
    DIR *dirp;

    // memory
    uint8_t virtualMemory[64 * 1024];
    uint16_t textStart;
    uint16_t textEnd;
    uint16_t dataStart;
    uint16_t dataEnd;
    uint16_t bssStart;
    uint16_t bssEnd;
    uint16_t brk;

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
#ifndef _MACHINE_T_
#define _MACHINE_T_
typedef struct machine_tag machine_t;
#endif

void setRegPtr(machine_t *pm);
extern char *toRegName[];

#define PSW_N (1<<3) // negative
#define PSW_Z (1<<2) // zero
#define PSW_V (1<<1) // overflow
#define PSW_C (1<<0) // carry
static inline void setN(machine_t *pm) {
    pm->psw |= PSW_N;
}
static inline void setZ(machine_t *pm) {
    pm->psw |= PSW_Z;
}
static inline void setV(machine_t *pm) {
    pm->psw |= PSW_V;
}
static inline void setC(machine_t *pm) {
    pm->psw |= PSW_C;
}

static inline void clearN(machine_t *pm) {
    pm->psw &= ~PSW_N;
}
static inline void clearZ(machine_t *pm) {
    pm->psw &= ~PSW_Z;
}
static inline void clearV(machine_t *pm) {
    pm->psw &= ~PSW_V;
}
static inline void clearC(machine_t *pm) {
    pm->psw &= ~PSW_C;
}

static inline bool isN(const machine_t *pm) {
    return (pm->psw & PSW_N);
}
static inline bool isZ(const machine_t *pm) {
    return (pm->psw & PSW_Z);
}
static inline bool isV(const machine_t *pm) {
    return (pm->psw & PSW_V);
}
static inline bool isC(const machine_t *pm) {
    return (pm->psw & PSW_C);
}

bool load(machine_t *pm, const char *src);
uint16_t fetch(machine_t *pm);
