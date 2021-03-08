#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

//怎么把这个宏和expr.c的连起来呢。。。
#define NUM_LEN 32
#define EXPR_LEN 64
int judge_bit(int);

// this should be enough
static char buf[65536];
static int pos=0;//数buf有效的位置
static int count_tokens = 0;//数tokens
int judge_bit(int num)//判断整数有几位
{
  int count = 0;
  if(num == 0) return 1;
  while(num!=0)
  {
    num/= 10;
    count++;
  }
  return count;
}
void gen_num(void)
{
  int temp = rand()%400;
  sprintf(&buf[pos], "%d", temp);
  pos += judge_bit(temp);
  count_tokens++;
  buf[pos]='\0';
}
void gen(char paren)
{
  buf[pos] = paren;
  pos++;
  if(paren == '(')
    count_tokens += 2;
  buf[pos]='\0';
}

void gen_rand_op(void)
{
  switch(rand()%4)
  {
    case 0: buf[pos] = '+';break;
    case 1: buf[pos] = '-';break;
    case 2: buf[pos] = '*';break;
    case 3: buf[pos] = '/';break;
  }
  int temp = rand()%3;
  for(int i=1;i<=temp;i++)//随机插入空格
  {
    buf[pos+i] = ' ';
  }
  pos += temp +1;
  count_tokens+= temp +1;
  buf[pos] = '\0';
}


static inline void gen_rand_expr() {
    if(count_tokens<500)//反正大于tokens长度的都要重做，这里是防止buf溢出
      switch (rand()%3) {
      case 0: gen_num(); break;
      case 1: gen('('); gen_rand_expr(); gen(')'); break;
      case 2: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
      }

}

static char code_buf[65536*2];//大跃进，翻一倍！不然sprintf有warning! 盲猜max(code_format+buf)小于等于max(code_buf)才能不warning
static const char *code_format = //我加了个 const
"#include <stdio.h>\n"//自动拼接双引号，最后应该只有“......”
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);//返回从epoch至今的秒数
  srand(seed);//设置随机数发生器的种子
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);//盲猜argv[0]是空格
  }
  int i;
  for (i = 0; i < loop; i ++) {//循环loop次
  negative:
    do
    {
      //清空并重新初始化buf及count
      for(int i=0 ; i<strlen(buf);i++)
        buf[i]='\0';
      count_tokens=pos=0;
      gen_rand_expr();
    }while(count_tokens>30||count_tokens<6);
    sprintf(code_buf, code_format, buf);//把buf按code_format的格式打进code_buf,会在codebuf的有效字符的最后加一个\0，buf必须以'\0’结束才能正常工作
    
    //文件读写
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);//code_buf必须以\0结束
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -Werror -o /tmp/.expr");//.expr是可执行文件，把code.c编译好（广义的编译）
    if (ret != 0) continue;
    fp = popen("/tmp/.expr", "r");//失败则返回null
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);
    if((unsigned int) (result)>500000000)//负数或超大正数
      goto negative;
    printf("%u %s\n", result, buf);
  }
  return 0;
}
