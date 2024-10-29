#pragma once

#include <common.h>

/* E1000 Function Definitions */
void e1000_init(void *base_addr);
int e1000_transmit(void *txpacket, int length);
int e1000_poll(void *rxbuffer);