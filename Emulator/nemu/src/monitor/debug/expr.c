#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <string.h>//为了memcpy
#include <stdlib.h>//为了atoi
#include <stdio.h>//为了sscanf
#include <string.h>//为了strcmp

#define NUM_LEN 32
#define EXPR_LEN 64//从32变为64

extern uint32_t paddr_read(paddr_t addr, int len);//在ics2019/nemu/src/memory/memory.c里面

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUMBERS, DEREF, TK_AND, TK_UNEQ, TK_HEX, TK_REG
  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {//长度为9的结构数组

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"\\)", ')'}, //right parenthesis
  {"\\(", '('},// left parenthesis
  {"\\*", '*'},//multiply
  {"\\-", '-'},// minus
  {"/", '/'},// divide
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus一直没找到相关资料，为什么两个反斜杠表示转义？
  {"==", TK_EQ},         // equal
  {"0[xX][0-9a-f]+", TK_HEX},//16进制数,位置必须在下面十进制数前匹配
  {"[0-9]+", TK_NUMBERS},//匹配正数
  {"&&", TK_AND},//与
  {"!=", TK_UNEQ},//不等于
  {"\\$[a-z]{2,3}", TK_REG}//寄存器
 

  /* TODO: Add more token types */

  /* TODO: Add more token types */
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )//规则数目

static regex_t re[NR_REGEX] = {};//与rules元素类型、个数一样的数组
//用来放编译好的正则表达式


/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {//做rules个数次循环
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);//指向编译后的正则表达式的指针;需要被筛选的字符;ERE模式;成功返回0,失败返回错误编码
//编译正则表达式
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);//错误码；经过regcomp编译的正则表达式；存储错误信息的buffer；buffer的大小
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);//报错
    }
  }
}


typedef struct token {
  int type;
  char str[NUM_LEN];
} Token;


static Token tokens[EXPR_LEN] __attribute__((used)) = {};//长度为32的token类型结构
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;
/*typedef struct {

　　regoff_t  rm_so;

　　regoff_t  rm_eo;

　　} regmatch_t;

　　rm_so

如果-1==rm_so，表示没有匹配到。

如果-1!=rm_so，表示string中下一个最大子字符串的偏移量（距离string开头的偏移量）。

　　rm_eo即为子字符串的长度。*/
  nr_token = 0;

  while (e[position] != '\0') {
    if(nr_token>=EXPR_LEN)
    {
      Log("Your expression is too long!");
      assert(0);
    }
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {//经过regcomp编译的正则表达式。;待匹配的字符串;匹配到的字符串的个数;　匹配到的数组; flag
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;
        if(rules[i].token_type==TK_NOTYPE)
          break;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        tokens[nr_token].type=rules[i].token_type;
        if (substr_len>NUM_LEN-1)//还要存\0呢
        { 
          Log("Your number is too large!");
          return false;
        }
        switch (rules[i].token_type) {
          case TK_NUMBERS: {memcpy(tokens[nr_token].str, substr_start, substr_len);tokens[nr_token].str[substr_len]='\0';}; break;
          case TK_HEX: {memcpy(tokens[nr_token].str, substr_start, substr_len);tokens[nr_token].str[substr_len]='\0';}; break;
          case TK_REG: {memcpy(tokens[nr_token].str, substr_start+1, substr_len-1);tokens[nr_token].str[substr_len-1]='\0';}; break;
          //for(int j=0;j<substr_len;j++) tokens[nr_token].str[j]=substr_start[j];       
        }
    
        nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}
bool checkparentheses(int p, int q)//true需要表达式被一对括号包裹
{
  int count_stack=0;
  if(tokens[p].type!='('&& tokens[q].type== ')')//不被括号包围，直接跳出
    return false;
  while(p<q)//程序的正确性由至少一个左括号未匹配保证。如果没有要求子表达式被左右括号包围，这个check是错的
  {
    if (tokens[p].type=='(')
      ++count_stack; 
    else if(tokens[p].type==')') 
      --count_stack; 
    if(count_stack==0)
      return false;
  ++p;
  }
  if(count_stack==1)// 应对（（（（） 这种情况
    return true;
  else
    return false;
}

bool my_checkparentheses(int p, int q)//表达式括号匹配就返回true，不要求被一对括号包围
{
  int count_stack=0;
    while(p<=q)
  {
    if (tokens[p].type=='(')
      ++count_stack; 
    else if(tokens[p].type==')') 
      --count_stack; 
    if(count_stack<0)
      return false;
  ++p;
  }
  if(count_stack==0)
    return true;
  else
    return false;//如(((((()
}
int find_the_main_op(int p, int q)
{
    char operator[30];
    int count_operator = 0;
    int count_parenthesis=0;//为0的时候已完成
    for(int i=p; i<=q; i++)
    {
        if(tokens[i].type == '(')
          ++count_parenthesis;
        else if(tokens[i].type == ')')
          --count_parenthesis;
        if(count_parenthesis!=0||tokens[i].type == ')')// || 保证完成匹配的那次可以再次重做循环,顺便短路
          continue;
        if(tokens[i].type != TK_NUMBERS  && tokens[i].type != TK_REG && tokens[i].type != TK_HEX)
        { 
          operator[count_operator]= tokens[i].type;
          ++count_operator;
        }
    }
    /*
    逻辑如下：
    先检查是否为纯粹 &&
    再检查是否为纯粹 ==， ！=
    再检查是否为纯粹 +, -
    再检查是否为纯粹 *, /
    如果出现了纯粹的，取最后一个运算符为主运算符
    否则，取最后一个出现的&&
    若不出现，取最后一个出现的 == ！=
    若不出现，取最后一个出现的 + -
    若不出现，取最后一个出现的 * /
    */
    bool flag_of_pure_and = true;
    bool flag_of_pure_equal_unequal = true;
    bool flag_of_pure_add_minus = true;
    bool flag_of_pure_multiply_and_divide = true;
    for(int j=count_operator-1; j>=0; j--)
    {
        if (operator[j] != TK_AND)
          {
            flag_of_pure_and = false;
            break;
          }
    }
    for(int j=count_operator-1; j>=0; j--)
    {
        if (operator[j] != TK_EQ && operator[j] != TK_UNEQ)
          {
            flag_of_pure_equal_unequal = false;
            break;
          }
    }
    for(int j=count_operator-1; j>=0; j--)
    {
        if (operator[j] != '+' && operator[j] != '-')
          {
            flag_of_pure_add_minus = false;
            break;
          }
    }
    for(int j=count_operator-1; j>=0; j--)
    {
        if (operator[j] != '*' && operator[j] != '/')
          {
            flag_of_pure_multiply_and_divide = false;
            break;
          }
    }    
    int main_op_count=0;
    if(flag_of_pure_and || flag_of_pure_equal_unequal || flag_of_pure_add_minus || flag_of_pure_multiply_and_divide)//纯就取最后一个符号
      main_op_count = count_operator - 1 ;
    else
    {
      for(int j=count_operator-1;j>=0;j--)
      {
        if (operator[j]== TK_AND)
        {
          main_op_count = j;
          goto Reintialization;
        }  
      }
      for(int j=count_operator-1;j>=0;j--)
      {
        if (operator[j]== TK_EQ || operator[j]==TK_UNEQ)
        {
          main_op_count = j;
          goto Reintialization;
        }  
      }
      for(int j=count_operator-1;j>=0;j--)
      {
        if (operator[j]== '+' || operator[j]=='-')
        {
          main_op_count = j;
         goto Reintialization;
        }  
      }
      for(int j=count_operator-1;j>=0;j--)
      {
        if (operator[j]== '*' || operator[j]=='/')
        {
          main_op_count = j;
          goto Reintialization;
        }  
      }
    
    }//通过再次遍历找到OP在原表达式中的位置
    Reintialization:
    count_operator =count_parenthesis= 0;//重新初始化
     for(int i=p; i<=q; i++)
    {
        if(tokens[i].type == '(')
          ++count_parenthesis;
        else if(tokens[i].type == ')')
          --count_parenthesis;
        if(count_parenthesis!=0||tokens[i].type == ')')// || 保证完成匹配的那次可以再次重做循环,顺便短路
          continue;
        if(tokens[i].type != TK_NUMBERS  && tokens[i].type != TK_REG && tokens[i].type != TK_HEX)
        { 
          operator[count_operator]= tokens[i].type;
          if(count_operator == main_op_count)
            return i;
          ++count_operator;
        }
    }
  Log("There is something wrong in the search of main op");
  return 0;
}
int eval(int p,int q)//实现一个只能计算自然数的表达式计算器,unsigned int 
{
  if(p>q)
    {
      Log("There must be something wrong for the expressions!");
      return 0;
    }
  else if(p == q)//是一个数或寄存器
    {
      int temp_one_number = 0;
      if((tokens[p].str[1] == 'x' || tokens[p].str[1] == 'X')&& tokens[p].type== TK_HEX)
        sscanf(tokens[p].str, "%x",&temp_one_number);
      else
      {
        sscanf(tokens[p].str, "%d",&temp_one_number);
      }
      if(tokens[p].type== TK_REG)
      {
        char reg_type[5];
        sscanf(tokens[p].str, "%s", reg_type);
        if(strcmp(tokens[p].str, "eax")==0)//0表示字符串相同
          temp_one_number = cpu.gpr[0]._32;
        else if(strcmp(tokens[p].str, "ax")==0)
          temp_one_number = cpu.gpr[0]._16;
        else if(strcmp(tokens[p].str, "al")==0)
          temp_one_number = cpu.gpr[0]._8[0];
        else if(strcmp(tokens[p].str, "ah")==0)
          temp_one_number = cpu.gpr[0]._8[1];
        else if(strcmp(tokens[p].str, "ecx")==0)//0表示字符串相同
          temp_one_number = cpu.gpr[1]._32;
        else if(strcmp(tokens[p].str, "cx")==0)
          temp_one_number = cpu.gpr[1]._16;
        else if(strcmp(tokens[p].str, "cl")==0)
          temp_one_number = cpu.gpr[1]._8[0];
        else if(strcmp(tokens[p].str, "ch")==0)
          temp_one_number = cpu.gpr[1]._8[1]; 
        else if(strcmp(tokens[p].str, "edx")==0)//0表示字符串相同
          temp_one_number = cpu.gpr[2]._32;
        else if(strcmp(tokens[p].str, "dx")==0)
          temp_one_number = cpu.gpr[2]._16;
        else if(strcmp(tokens[p].str, "dl")==0)
          temp_one_number = cpu.gpr[2]._8[0];
        else if(strcmp(tokens[p].str, "dh")==0)
          temp_one_number = cpu.gpr[2]._8[1];
        else if(strcmp(tokens[p].str, "ebx")==0)//0表示字符串相同
          temp_one_number = cpu.gpr[3]._32;
        else if(strcmp(tokens[p].str, "bx")==0)
          temp_one_number = cpu.gpr[3]._16;
        else if(strcmp(tokens[p].str, "bl")==0)
          temp_one_number = cpu.gpr[3]._8[0];
        else if(strcmp(tokens[p].str, "bh")==0)
          temp_one_number = cpu.gpr[3]._8[1];
        else if(strcmp(tokens[p].str, "esp")==0)//0表示字符串相同
          temp_one_number = cpu.gpr[4]._32;
        else if(strcmp(tokens[p].str, "sp")==0)
          temp_one_number = cpu.gpr[4]._16;
        else if(strcmp(tokens[p].str, "ebp")==0)//0表示字符串相同
          temp_one_number = cpu.gpr[5]._32;
        else if(strcmp(tokens[p].str, "bp")==0)
          temp_one_number = cpu.gpr[5]._16;
        else if(strcmp(tokens[p].str, "esi")==0)//0表示字符串相同
          temp_one_number = cpu.gpr[6]._32;
        else if(strcmp(tokens[p].str, "si")==0)
          temp_one_number = cpu.gpr[6]._16;
        else if(strcmp(tokens[p].str, "edi")==0)//0表示字符串相同
          temp_one_number = cpu.gpr[7]._32;
        else if(strcmp(tokens[p].str, "di")==0)
          temp_one_number = cpu.gpr[7]._16;
        else if(strcmp(tokens[p].str, "pc")==0)//加一个pc以实现断点的功能
          temp_one_number = cpu.pc;                                 
      }

      return temp_one_number;
    }
  else if(checkparentheses(p, q) == true)
      return eval(p+1,q-1);
  else if (tokens[p].type == DEREF)
  {
    if(!my_checkparentheses(p,q))
      {
        Log("It's an illegal expression due to unmatched parentheses!");
        return 0;
      }
    return paddr_read((uint32_t)(eval(p+1, q)), 4);
  }
  
  else
  {
    if(!my_checkparentheses(p,q))
      {
        Log("It's an illegal expression due to unmatched parentheses!");
        return 0;
      }
    int op = find_the_main_op(p,q);
    int val1 = eval(p, op - 1);
    int val2 = eval(op + 1, q);
  //处理除以0
    if(val2==0 && tokens[op].type=='/')
      {
          Log("ZeroDivisionError!Your expression is wrong!");
          return 0;
      }

    switch (tokens[op].type) {
      case '+': if (val1+val2>=0) return (val1 + val2); else {printf("Negative number in subexpression!Or overflow!\n"); return (val1 + val2);};break;
      case '-': if (val1-val2>=0) return (val1 - val2); else {printf("Negative number in subexpression!Or overflow!\n"); return (val1 - val2);};break;
      case '*': if (val1*val2>=0) return (val1 * val2); else {printf("Negative number in subexpression!Or overflow!\n"); return (val1 * val2);};break;
      case '/': if (val1/val2>=0) return (val1 / val2); else {printf("Negative number in subexpression!Or overflow!\n"); return (val1 / val2);};break;
      case TK_AND: if (val1>=0 && val2>=0) return (val1 && val2); else {printf("Negative number in subexpression!Or overflow!\n"); return (val1 && val2);};break;
      case TK_UNEQ: if (val1>=0 && val2>=0) return (val1 != val2); else {printf("Negative number in subexpression!Or overflow!\n"); return (val1 != val2);};break;
      case TK_EQ: if (val1>=0 && val2>=0) return (val1 == val2); else {printf("Negative number in subexpression!Or overflow!\n"); return (val1 == val2);};break;
      default: assert(0);
    }      
  }
}



__attribute__((unused)) uint32_t expr(char *e, bool* success)  {
  if (!make_token(e)) {
    success = false;
    return 0;
  }
  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '*' && (i == 0 || tokens[i - 1].type == '+'|| tokens[i - 1].type == '-'|| tokens[i - 1].type == '*'|| tokens[i - 1].type == '/'|| tokens[i - 1].type == '(') ) {
      tokens[i].type = DEREF;}
  }
  int temp = eval(0, nr_token-1);//调试用
 return (uint32_t)temp; 

}
