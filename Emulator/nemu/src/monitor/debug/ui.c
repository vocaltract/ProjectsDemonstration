#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

extern uint32_t paddr_read(paddr_t addr, int len);//defined in src/memory/memory.c
extern void isa_reg_display(void);//defined in x86/reg.c
void cpu_exec(uint64_t);
extern uint32_t expr(char *e, bool* success);

int my_scanf(char *args)
{
  if(args==0x0)//没有数字的情况，args没有初始化
    return 1;
  char temp_num[11];
  int result=0, prop=1,count=0,count_loop=0,flag=1;
  while(args[count_loop]!='\0')
  {
    if(47<(int)(args[count_loop]) && (int)(args[count_loop]<58))
    {
      temp_num[count]=args[count_loop];
      count++;
    }
    if(args[count_loop]=='-')
      flag = -1;
    count_loop++;
  }
  while(count!=0)
  {
    count--;
    result += ((int)(temp_num[count])-48)*prop;
    prop *=10;
  }
  if (result == 0)
    return 1;
  else
    return (result*flag>0)?result*flag:-1;
  //gdb告诉我他已经分好了。多写了好多废话。我好苦啊,我以为它si和数字没分开
}

int my_scan_x_number(char *args)
{
  int count;
  if(sscanf(args, "%d ", &count))
    return count;
  else
  {
    printf("Something wrong!");
    return 0; 
  }
  
}

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
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

static int cmd_si(char *args){
  cpu_exec(my_scanf(args));
  return 0;
}

static int cmd_info(char *args){
  if(args[0]=='r')
    isa_reg_display();
  else if (args[0]=='w')
    print_all_watchpoints();
  return 0;
}

static int cmd_x(char *args){
  int count = my_scan_x_number(args);
  do
 {
   args++;
 } while (*args != ' ');
  args++;
  bool judge = true;
  bool* p_judge = &judge;
  unsigned int my_addr = expr(args, p_judge);
  unsigned int result;
  if(*p_judge)
  {
    while(count!=0)//源码的位操作实在看不懂，功能是我猜的
    {
      result = paddr_read(my_addr, 4);
      count--;
      printf("%#8x: ", my_addr);
      my_addr += 4; 
      printf("%#8x\n", result);
    }
  }
  else
    printf("Failure to read the expression!");
  return 0;
}

static int cmd_p(char* arg){
  bool judge=true;
  bool* p_judge= &judge; 
  if(arg == NULL)
    {
      Log("Input without expression!");
      return -1;
    }
  unsigned int temp = expr(arg, p_judge);
  if(*p_judge)
    {printf("My p_command's result is 0x%x\n", temp);
      return temp;//cmd_test依赖该语句    
    }
  else
  {
    Log("Maybe there exists something wrong！");
    assert(0);
  }
  return 0;
}

static int cmd_test(char *arg)
{
  int count = 100;//input中的行数
  int resul_c = 0;
  //FILE * fp = fopen("../../../tools/gen-expr/input", "r");
  FILE * fp = fopen("/home/revolution/ics2019/nemu/tools/gen-expr/input", "r");
  if (fp == NULL)
    assert(0);
  char buff[110];//随便设的数字
  char expression[80];//随便设置的数字
  while(count>0)
  {
    int begin_of_expr = 0;
    int end_of_expr = 0;
    assert(fgets(buff,100,fp));
    sscanf(buff, "%d", &resul_c);
    while(buff[begin_of_expr]!=' ')
      begin_of_expr++;
    begin_of_expr++;
      //此时为表达式开头
    while(buff[end_of_expr]!='\n')
      end_of_expr++;
    memcpy(&expression[0],&buff[begin_of_expr],end_of_expr-begin_of_expr+1);
    expression[end_of_expr-begin_of_expr]='\0';
    printf("The expression is : %s\n", expression);
    if(resul_c != cmd_p(expression))
      {
      printf("Unequal!\n The number of the expression is %x.\n", 101-count);
      }
    else
    {
      printf("Equal!\n");
    }
    count--;
  }
  fclose(fp);
  return 0;
}

static int cmd_w(char* arg)
{
  bool judge = true;
  bool * p_judge = & judge;
  int temp = expr(arg, p_judge);
  WP* new_node;
  if(*p_judge)
  {
    new_node = new_wp();
    memcpy(&new_node->content[0], arg, strlen(arg));//算\0
    new_node -> former_value = temp;
    new_node -> latter_value = temp;
    return 0;
  }
  else
  {
    printf("Wrong expression!\n");
    return -1;
  }
}

static int cmd_d(char* arg)
{
  int num = atoi(arg);
  delete_w (num);
  return 0;
}



static int cmd_help(char *args);


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si", "Execute an instruction", cmd_si},
  {"info", "r Display the reg\n     - w Display the information from the watchpoint", cmd_info},
  {"x", "Scan the RAM", cmd_x},
  {"p","Calculate the value of the expression", cmd_p},
  {"test","Test the p function with input from tools/gen-expr/input", cmd_test},
  {"w", "Set the watchpoint with the expression", cmd_w},
  {"d", "Delete the watchpoint named after number, beginning with 0", cmd_d}
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))//total number of kinds of acceptable instrucions

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

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
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
