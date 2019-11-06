#pragma once

struct machine_tag;
typedef struct machine_tag* pmachine_t;

void syscall(pmachine_t pm);
