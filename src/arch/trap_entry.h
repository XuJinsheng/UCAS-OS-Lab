#pragma once

#include <common.h>

// these are in trap_entry.S, assume 4-byte aligned
__BEGIN_DECLS
struct Thread;
void user_trap_entry();
void user_trap_return();
void kernel_trap_entry();
void switch_context_entry(Thread *from_thread, Thread *to_thread); // will not update current_running
__END_DECLS