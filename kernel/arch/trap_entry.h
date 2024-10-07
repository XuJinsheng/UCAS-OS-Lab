#pragma once

#include <common.h>

// these are in trap_entry.S, assume 4-byte aligned
__BEGIN_DECLS
void user_trap_entry();
void user_trap_return();
void kernel_trap_entry();
void switch_context_entry(ptr_t from_reg[14], ptr_t to_reg[14]);
__END_DECLS