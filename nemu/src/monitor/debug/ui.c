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

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute one or more steps", cmd_si},
  { "info", "Print the register's information", cmd_info},
  { "x", "Scan memory", cmd_x},

  /* TODO: Add more commands */

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
  /* extract the first argument  */
  char *arg = strtok(NULL, " ");
  int step;

  if(arg != NULL){
	sscanf(arg, "%d", &step);
  }
  else step = 1; /* no argument given, default step = 1 */

  /* step is -1, the same function as command 'c' */ 
  if(step == -1){
	cpu_exec(-1);
	return 0;	
  }
  /* step is positive, execute N times using a for loop */
  else if(step >= 1){
	for(int i=1; i<=step; i++){
	  cpu_exec(1);
	}
  }
  /* illegal argument, send error */
  else printf("Unknown command '%s'\n", arg);
  return 1;
}

static int cmd_info(char *args){
  /* exract the first argument */
  char *arg = strtok(NULL, " ");
  /* argument is 'r', print current registers' value */
  if(strcmp(arg, "r") == 0){
    /* print 32-bit registers */
	for(int i=0; i<8; i++) printf("%s\t0x%x\t%d\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
	/* print 16-bit registers */
	for(int i=0; i<8; i++) printf("%s\t0x%x\t%d\n", regsw[i], cpu.gpr[i]._16, cpu.gpr[i]._16);
	/* print 8-bit registers */
	for(int i=0; i<8; i++){
	  for(int j=0; j<2; j++){
		printf("%s\t0x%x\t%d\n", regsb[i], cpu.gpr[i]._8[j], cpu.gpr[i]._8[j]);
      }
    }
  }
  return 1; 
}

static int cmd_x(char *args){
  char *arg1 = strtok(NULL, " ");
  char *arg2 = strtok(NULL, " ");
  printf("%s,%s\n", arg1, arg2);
  int len;
  vaddr_t addr;
  /* convert string to numerical value */
  sscanf(arg1, "%d", &len);
  sscanf(arg2, "%x", &addr);
  printf("Address Dword block Byte sequence\n");
  for(int i=0; i<len; i++){
	printf("0x%x ", addr);
	printf("0x%x   ", vaddr_read(addr, 4));
	printf("%02x %02x %02x %02x", vaddr_read(addr, 1), vaddr_read(addr+1, 1), vaddr_read(addr+2, 1), vaddr_read(addr+3, 1));
	printf("\n");
	addr += 4; 
  }
  
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
