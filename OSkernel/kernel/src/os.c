#define DEBUG
#define LOCALTEST

#include <common.h>
#include <debug.h>
#include <am.h>
typedef struct handler_wrapper
{
    handler_t handler;
    int seq;
    int event;
    struct handler_wrapper *next;
}handler_wrapper_t;

//global variables
static handler_wrapper_t wrapper_head;
//static spinlock_t trap_lk={.locked=0,.name="trap_lk"};



static _Context * wrapper_panic(_Event ev, _Context *context)
{
  panic("should never be here!");
  return NULL;
}

static void check_seq()
{
  int last_seq =INT_MIN;
  for(handler_wrapper_t *cur=wrapper_head.next;cur!=&wrapper_head;cur=cur->next)
  {
    assert(cur->seq>=last_seq);
    last_seq=cur->seq;
  }
}

static handler_wrapper_t wrapper_head={
  .handler=wrapper_panic,
  .event=_EVENT_NULL,
  .seq=INT_MIN,
  .next = &wrapper_head
};
#ifdef LOCALTEST
spinlock_t *local_lk;
int count=0;
void yielder(void *arg) { while (1) {_putc((char)((intptr_t)(arg))+'A');_yield(); } }
void adder(void *arg) { 
  while (1) 
  {
    kmt->spin_lock(local_lk);
    if(count<90000000)count++;
    printf("%d,",count);
    kmt->spin_unlock(local_lk);
  } 
}
sem_t empty, fill;
#define P kmt->sem_wait
#define V kmt->sem_signal
void producer(void *arg) { while (1) { P(&empty); _putc('('); V(&fill);  } }
void consumer(void *arg) { while (1) { P(&fill);  _putc(')'); V(&empty); } }

#endif
static void os_init() {//after pmm->init os_on_irq can be used
  assert(_ncpu()<=MAX_CPU);
  pmm->init();
  kmt->init();
#ifdef LOCALTEST
//   local_lk=pmm->alloc(sizeof(spinlock_t));
//   kmt->spin_init(local_lk,"local");
#define N_t 8
void *tasks[N_t];
  for(int i=0;i<N_t;i++)
  {
    tasks[i]=pmm->alloc(sizeof(task_t));
    //kmt->create(tasks[i], "yielder1", yielder, (void*)i);
   // kmt->create(tasks[i], "adder", adder, NULL);
  }
  kmt->sem_init(&empty, "empty", 20);  // 缓冲区大小为 5
  kmt->sem_init(&fill,  "fill",  0);
  kmt->create(tasks[0], "producerA", producer, NULL);
  kmt->create(tasks[1], "consumerA", consumer, NULL);
  kmt->create(tasks[2], "consumerB", consumer, NULL);


#endif
}


static void os_run() {
  _intr_write(1);
  while (1);
}
//spinlock_t debug_lk = {.locked=0,.name="debug_lk"};
void p_tks();
static _Context * os_trap(_Event ev, _Context *context)//due to context might be buggy
{
//  kmt->spin_lock(&debug_lk);
  //p_tks();

  if(ev.event!=_EVENT_IRQ_TIMER && ev.event!=_EVENT_YIELD)
  {
    printf("%s",ev.msg);
    assert(0);
  }
  _Context *next = NULL;
  for(handler_wrapper_t *cur=wrapper_head.next;cur!=&wrapper_head;cur=cur->next)
  {
    if(cur->event==_EVENT_NULL || cur->event==ev.event)
    {
      _Context *r = cur->handler(ev,context);
      panic_on(r&&next,"returning multiple contexts!");
      if(r) next=r;
    }
  }
  panic_on(!next, "returning NULL context");

//  kmt->spin_unlock(&debug_lk);
  return next;
}

static void os_on_irq(int seq, int event, handler_t handler)
{
  // printf("de!");
  // printf("%d\n",seq);
  //printf("%d,",seq);
#ifdef DEBUG  
  check_seq();
#endif
  handler_wrapper_t *new_wrapper = (handler_wrapper_t*)(pmm->alloc(sizeof(handler_wrapper_t)));
  new_wrapper->event=event;
  new_wrapper->handler=handler;
  new_wrapper->seq = seq;
  if(wrapper_head.next == &wrapper_head)
  {
    wrapper_head.next=new_wrapper;
    new_wrapper->next = &wrapper_head;
  }
  else
  {
    for(handler_wrapper_t *cur=wrapper_head.next;cur!=&wrapper_head;cur=cur->next)
    {
        if((seq>=cur->seq&&seq<=cur->next->seq)||(seq>=cur->seq&&cur->next==&wrapper_head))
        {
            new_wrapper->next = cur->next;
            cur->next = new_wrapper;
            break;
        }
    }
  }
  //printf("finished!\n");
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq,
};





void profile(void (*func)(),const char* fname, int rounds)
{
    uint32_t st=uptime();
    for(int i=0;i<rounds;i++)
    {
      __sync_synchronize();
      func();
      __sync_synchronize();
    }
    printf("Test %s.Passed time:%d\n",fname,(uptime()-st)/rounds);
}

