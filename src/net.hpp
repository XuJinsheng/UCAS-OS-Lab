#pragma once

#include <common.h>

#define PKT_NUM 32

extern void init_net();

extern void handle_ext_irq();

extern void net_idle_check();