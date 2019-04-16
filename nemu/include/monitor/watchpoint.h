#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char *expr; //被监视的表达式
  int new_val, old_val; //表达式的新值和旧值
} WP;

int set_watchPoint(char *e);
bool delete_watchpoint(int no);
void list_watchpoint();

#endif
