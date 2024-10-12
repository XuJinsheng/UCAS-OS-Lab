#pragma once

#include <common.h>

#define TIMER_INTERVAL 150000

extern void init_timer();

extern uint64_t get_timer(void);
extern uint64_t get_ticks(void);
extern uint64_t get_time_base(void);
extern void latency(uint64_t time);
extern void handle_irq_timer();
