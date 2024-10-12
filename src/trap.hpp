#pragma once

#include <common.h>

void init_trap();
__BEGIN_DECLS
void trap_handler(int from_kernel, ptr_t scause, ptr_t stval);
__END_DECLS