#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool()
{
	int i;
	for (i = 0; i < NR_WP; i++)
	{
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp()
{
	WP *p = free_;
	if (!free_)
		assert(0);
	free_ = free_->next;
	if (!head)
	{
		head = p;
		head->next = NULL;
		p->NO = 1;
	}
	else
	{
		int cnt = 2;
		WP *q = head;
		while (q->next)
		{
			cnt++;
			q = q->next;
		}
		p->NO = cnt;
		q->next = p;
		p->next = NULL;
	}
	return p;
}
void createWatchPoint(char *args)
{
	WP *p = new_wp();
	strcpy(p->expr, args);
	p->type = 1;
	p->value = expr(p->expr);
}
void free_wp(WP *wp)
{
	WP *p = head;
	WP *q = head;
	if (q == wp)
	{ //头结点
		head = head->next;
	}
	else
	{
		while (q != wp)
		{
			p = q;
			q = q->next;
		}
		p->next = q->next;
	}
	if (!free_)
	{
		free_ = wp;
	}
	else
	{
		wp->next = free_;
		free_ = wp;
	}
}
WP *searchWatchPoint(int num)
{
	WP *p = head;
	num--;
	while (num--)
	{
		if (!p)
		{
			printf("NO.%d is not exist!\n", num);
			return NULL;
		}
		p = p->next;
	}
	return p;
}
bool judgeWatchPoint()
{
	bool flag = false;
	int value;
	WP *p = head;
	while (p)
	{
		value = expr(p->expr);
		if (value != p->value)
		{
			printf("The following watchpoint is changed!\n");
			printf("Num	Expr	OldValue	NewValue\n");
			printf("%d	%s	0x%x	0x%x\n", p->NO, p->expr, p->value, value);
			p->value = value;
			flag = true;
		}
		p = p->next;
	}
	return flag;
}
void printAllWatchPoint()
{
	WP *p = head;
	if (!head)
	{
		printf("There is no watchpoint!\n");
		return;
	}
	else
	{
		printf("Num	Type	Expr	Value\n");
		while (p)
		{
			if (p->type == 1)
			{
				printf("%d	%d	%s	0x%x\n", p->NO, p->type, p->expr, p->value);
				p = p->next;
			}
		}
	}
}