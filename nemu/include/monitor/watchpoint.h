#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__
#include "common.h"

typedef struct watchpoint {
  int NO;
  int value;
  int type;
  char expr[33];
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
} WP;

WP* new_wp();
void init_wp_pool();
void createWatchPoint(char *args);
WP* searchWatchPoint(int num);
bool judgeWatchPoint();
void printAllWatchPoint();
void free_wp(WP *wp);
#endif
