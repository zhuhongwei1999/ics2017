#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP *new_wp(){
  WP *wp = free_;
  // 没有空闲监视点结构的情况,通过 assert(0) 马上终止程序
  if(!free_){
    printf("Memory allocation Failed!");
    assert(0);
  }
  else{
    // 将空闲监视点向后移动一个位置
    free_ = free_->next;
    // 将新监视点插入head链表，并将head指向当前节点
    wp->next = NULL;
    if(head) wp->next = head;
    head = wp;
  }
  return wp;
}

void free_wp(WP* wp){
  WP* p;
  // 如果正在使用的监视点指向头结点，则直接将head指针指向下一位
  if (wp == head){
    head = head->next;
    wp->next = free_;
  }
  else{
    p = head;
    // 遍历链表，找到要回收的监视点
    while(p->next){
      if (p->next == wp) {
        // 删除head链表中的节点，并添加到free_链表中
        p->next = wp->next;
        wp->next = free_;
      }
    }
  }
  free_ = wp;
}

int set_watchPoint(char *e){
  printf("normal");
	// WP *p = new_wp();
  // bool *success = false;
	// strcpy(p->expr, e);
	// p->old_val = expr(p->expr, success);
  // return p->NO;
  return 1;
}

bool delete_watchpoint(int no){
  if(!head){
    printf("No watchpoints.");
    return false;
  }
  WP *wp = &wp_pool[no];
  free_wp(wp);
  return true;
}

void list_watchpoint(){
  WP *p = head;
  if(!head){
    printf("No watchpoints.");
    return;
  }
  else{
    printf("NO    Expr    Old Value");
    while(p){
      printf("%d   %s    0x%x\n", p->NO, p->expr, p->old_val);
      p = p->next;
    }
  }
}
