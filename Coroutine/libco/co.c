#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#ifdef LIBCO_DEBUG
#define Assert(expr) assert(expr)
#else
#define Assert(expr)
#endif
#define STACK_SIZE (64*1024*64)
#define MAX_NAME_LEN 48

enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};
struct co {
  char name[MAX_NAME_LEN];
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;
  struct co *prev;
  struct co *next;
  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
};

static struct co *current = NULL;
static struct co *sentinel = NULL;

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg);
// Caution! What was called by stack_switch_call should never return!
static inline void list_insert_end(struct co *co);
static inline void list_remove_one_nofree(struct co* co);
static inline void list_remove_one(struct co *co);
struct co * rand_select(int round_max);

// sp 栈顶（rsp的内容）  entry 函数的参数  arg rip的位置
//stack_switch_call is a calling 保证携程在指定的stack上


static void wrapper(uintptr_t placeholder)
{
  current->status=CO_RUNNING;
  current->func(current->arg);
  current->status=CO_DEAD;
  if(current->waiter)
  {
    current->waiter->status=CO_RUNNING;
  }
  list_remove_one_nofree(current);
  co_yield();//dead coroutine will never be selected again
  assert(0);//should never return
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  struct co *fresh_co = malloc(sizeof(struct co));
  fresh_co->arg=arg;
  fresh_co->func=func;
  strcpy(fresh_co->name,name);
  fresh_co->waiter=NULL;
  fresh_co->status=CO_NEW;
  list_insert_end(fresh_co);
  return fresh_co;
}

void co_wait(struct co *co) {
  if(co->status==CO_DEAD)
  {
    // Do nothing
  }
  else //CO_NEW or CO_RUNNING or CO_WAITING
  {
    current->status=CO_WAITING;
    co->waiter=current;
    co_yield();
  }
  free(co);
}

void co_yield() {
  int val = setjmp(current->context);
  if(val == 0)
  {
    //random select a NEW or RUNNING coroutine
    int round_max = rand()%129+1;
    current = rand_select(round_max);
    if(current->status==CO_RUNNING)
    {
      longjmp(current->context,1);//recover
    }
    else
    {
      Assert(current->status==CO_NEW);
      uintptr_t sp = (uintptr_t)&current->stack+sizeof(current->stack);
      stack_switch_call((void *)sp,(void *)&wrapper,(uintptr_t)0);
    }
  }
  //recovered
}



__attribute__((constructor)) static void init_co()
{
#ifdef LIBCO_DEBUG
  srand(250);
#else
  srand(time(NULL));
#endif
  sentinel = (struct co *)malloc(sizeof(struct co));
  strcpy(sentinel->name,"main");
  sentinel->next=sentinel;
  sentinel->prev=sentinel;
  sentinel->status=CO_RUNNING;
  sentinel->arg=NULL;//Assume main's arg is NULL
  sentinel->waiter=NULL;
  current = sentinel;
}

static inline void list_insert_end(struct co *co)// insert at the tail
{
  struct co *tail=sentinel->prev;
  tail->next = co;
  sentinel->prev = co;
  co->prev = tail;
  co->next = sentinel;
  Assert(co->prev->next==co);
  Assert(co->next->prev==co);
}

static inline void list_remove()
{
  for(struct co *co=sentinel->next;co!=sentinel;co=co->next)
  {
    if(co->status==CO_DEAD)
    {
      co->prev->next = co->next;
      co->next->prev = co->prev;
      free(co);
    }
  }
}

static inline void list_remove_one(struct co *co)
{
  Assert(co->status==CO_DEAD);
  co->prev->next = co->next;
  co->next->prev = co->prev;
  free(co);
}


static inline void list_remove_one_nofree(struct co* co)
{
  assert(co->status==CO_DEAD);
  co->prev->next = co->next;
  co->next->prev = co->prev;
}


struct co * rand_select(int round_max)// select a new or running
{
    struct co *selected = sentinel;
    for(int i=0;i<round_max;i++)
    {
      if(selected->status!=CO_NEW && selected->status!=CO_RUNNING)
      {
        i--;
      }
      selected = selected->next;
    }
    while(selected->status!=CO_NEW && selected->status!=CO_RUNNING)
    {
      selected=selected->next;
    }
    return selected;
}



static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      : : "b"((uintptr_t)sp),     "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
}