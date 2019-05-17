#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si","Step by step",cmd_si},
  { "info","Print the states of register",cmd_info},
  { "x","Scanning memory",cmd_x},
  { "p","Caculator",cmd_p},
  { "w","Create new watchpoint",cmd_w},
  { "d","Delete the watchpoint",cmd_d},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
  if(args==NULL){
    cpu_exec(1);//这边的参数为1代表只执行一次
  }else{
    char *temp=strtok(NULL," ");
    int n;
    sscanf(temp,"%d",&n);
    if(n<-1){
      printf("The param you input is incorrect!\n");
      return 0;
    }else if(n==0){
      cpu_exec(1);
    }else{
      cpu_exec(n);
    }
  }
  return 0;
}

void printALlRegisters(){
	printf("The following are all hexadecimal(0x)\n");
    for(int i=0;i<8;i++){
      printf("%s:\t%8x\t",regsl[i],cpu.gpr[i]._32);
      if(i%2==1)
        printf("\n");
    }
    for(int i=0;i<8;i++){
      printf("%s:\t%8x\t",regsw[i],cpu.gpr[i]._16);
      if(i%2==1)
        printf("\n");
    }
    for(int i=0,j=0;i<4;i++,j++){
      printf("%s:\t%8x\t",regsb[j],cpu.gpr[i]._8[0]);
      j++;
      printf("%s:\t%8x\t",regsb[j],cpu.gpr[i]._8[1]);
      printf("\n");	
    }
    printf("cpu.CF:%x cpu.OF:%x cpu.SF:%x cpu.ZF:%x\n",cpu.CF,cpu.OF,cpu.SF,cpu.ZF);
}
static int cmd_info(char *args){
  //strtok第一个参数为NULL时
  //该函数默认使用上一次未分割完的字符串的未分割的起始位置作为本次分割的起始位置
    char *temp=strtok(NULL," ");
    if(strcmp(temp,"r")==0)
    	printALlRegisters();
  	else if(strcmp(temp,"w")==0)
    	printAllWatchPoint();
	return 0;
}

static int cmd_x(char *args){
	char *temp=strtok(NULL," ");
	char *temp_2=strtok(NULL," ");
	int n;
	vaddr_t addr;
	sscanf(temp,"%d",&n);//把字符串变成整数
	sscanf(temp_2,"%x",&addr);
  printf("Address               Big-Endian      Little-Endian\n");
  for(int i=0;i<n;i++){
    printf("0x%08x            0x%08x      %02x %02x %02x %02x\n",
    addr+i*4,vaddr_read(addr+i*4,4),vaddr_read(addr+i*4,1),
    vaddr_read(addr+i*4+1,1),vaddr_read(addr+i*4+2,1),vaddr_read(addr+i*4+3,1));
  }
  return 0;
}

static int cmd_p(char *args){
  char *temp=strtok(NULL," ");
  printf("result =%d\n",expr(temp));
  return 1;
}

static int cmd_w(char *args){
  char *temp=strtok(NULL," "); 
  createWatchPoint(temp);
  return 1;
}

static int cmd_d(char *args){
  int num;
  sscanf(args,"%d",&num);
  WP *wp;
  wp=searchWatchPoint(num);
  if(wp)
  	free_wp(wp);
  return 1;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
