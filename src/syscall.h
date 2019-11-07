#pragma once

struct machine_tag;
#ifndef _MACHINE_T_
#define _MACHINE_T_
typedef struct machine_tag machine_t;
#endif

void mysyscall(machine_t *pm);
