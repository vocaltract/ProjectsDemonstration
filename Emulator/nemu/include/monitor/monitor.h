#ifndef __MONITOR_H__
#define __MONITOR_H__
#define NUM_LEN 32
#define EXPR_LEN 64//从32变为64
#include "common.h"

enum { NEMU_STOP, NEMU_RUNNING, NEMU_END, NEMU_ABORT };

typedef struct {
  int state;
  vaddr_t halt_pc;
  uint32_t halt_ret;
} NEMUState;

extern NEMUState nemu_state;

#endif
