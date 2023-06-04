#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

static inline uint16_t read16(bool isReg, const uint8_t *p) {
    if (isReg) {
        return *(const uint16_t *)p;
    } else {
        return p[0] | (p[1] << 8);
    }
}

static inline void write16(bool isReg, uint8_t *p, uint16_t data) {
    if (isReg) {
        *(uint16_t *)p = data;
    } else {
        p[0] = data & 0xff;
        p[1] = data >> 8;
    }
}

static inline uint8_t read8(bool isReg, const uint8_t *p) {
    if (isReg) {
        return *(const uint16_t *)p & 0xff;
    } else {
        return p[0];
    }
}

static inline void write8(bool isReg, uint8_t *p, uint8_t data) {
    if (isReg) {
        *(int16_t *)p = (int8_t)data;
    } else {
        p[0] = data;
    }
}
