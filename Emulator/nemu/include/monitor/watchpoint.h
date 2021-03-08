#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__
#define EXPR_LEN 64
#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char content[EXPR_LEN];//存放表达式
  int former_value;
  int latter_value;
  /* TODO: Add more members if necessary */

} WP;

WP* new_wp(void);
void free_wp(WP *wp);
void delete_w (int num);
bool update_all_w(void);
void print_all_watchpoints(void);
#endif
