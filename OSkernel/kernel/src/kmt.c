#define DEBUG
//#define DEBUG_CTX
#include <common.h>
#include <debug.h>
#include <am.h>

//declearation
void p_semtks();
//cpulocal
static void init_cpu_local();
static void init_idles();
static struct cpu_local* mycpu();
//spinlock
static int holding(struct spinlock *lock);
static void popcli();
static void pushcli();

//handlers
_Context* kmt_context_save(_Event ev, _Context* context);
_Context* kmt_schedule(_Event ev, _Context* context);

//task queue
static void tasks_enque_lk(task_t *task);
static void tasks_enque_ulk(task_t *task);
static task_t* tasks_deque_lk();
static task_t* tasks_deque_ulk();
static int tasks_check_ulk(task_t *task);// should be used into a queue_lk
static void tasks_throw_lk(task_t* task);
static void tasks_throw_ulk(task_t* task);
static int check_running_lk();
static int check_running_ulk();
/////////////////////////////////////////////////////////////////////
//definition
static spinlock_t queue_lk = {.locked=0,.name="queue_lk",.cpu=NULL};
struct cpu_local cpu_local[MAX_CPU];
task_t idles[MAX_CPU];
//static spinlock_t sem_init_lk={.name="sem_init_lk",.locked=0,.cpu=NULL};
task_t task_head={
    .name="task_head",
    .stack={.start=NULL,.end=NULL},
    .next = &task_head,
    .prev = &task_head,
    .context = NULL,
    .status = HEADER
};

_Context* kmt_test(_Event ev, _Context* context)
{
    //placeholder
    check_running_lk();
    tasks_throw_ulk(NULL);
    tasks_deque_lk();
    holding(NULL);
    return NULL;
}



static void kmt_init()
{
    init_cpu_local();
    init_idles();
    os->on_irq(INT_MIN,_EVENT_NULL,kmt_context_save);
    os->on_irq(INT_MAX,_EVENT_NULL,kmt_schedule);
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg)
{
    int intr = _intr_read();
    _intr_write(0);
    task->name = name;
    uintptr_t begin = (uintptr_t)pmm->alloc(4096);
    task->stack.start=(void*)begin;
    task->stack.end=(void*)(begin+4096);
    task->status = RUNNING;
    task->context=_kcontext(task->stack,entry,arg);
    legal_context(task->context);
    tasks_enque_lk(task);
    if(intr) _intr_write(1);
    return 0;
}

static void kmt_teardown(task_t *task)//memory leaking!!!!
{
    task->status = DEAD;
    tasks_throw_lk(task);
    //pmm->free(task->stack.start);
}

static void kmt_spin_init(spinlock_t *lk, const char *name)
{
    lk->name = name;
    lk->locked = 0;
    lk->cpu = NULL;
}
static void kmt_spin_lock(spinlock_t *lk)
{
    pushcli();
    //if(holding(lk))   panic("acquire");
    if(holding(lk))
    {
        printf("%s",lk->name);
        while(1);
    }
    
    while(_atomic_xchg(&lk->locked, 1));
    __sync_synchronize();
    lk->cpu = mycpu();
}
static void kmt_spin_unlock(spinlock_t *lk)
{
    if(!holding(lk))  panic("release");
    lk->cpu=NULL;
    __sync_synchronize();
    _atomic_xchg(&lk->locked, 0);
    popcli();
}

static void kmt_sem_init(sem_t *sem, const char *name, int value)
{
    sem->name = name;
    sem->value=value;
    kmt->spin_init(&sem->lk,"sem_lk");
    sem->sem_head=(sem_task_t){.task=NULL,.prev=&sem->sem_head,.next=&sem->sem_head};
}
static void kmt_sem_wait(sem_t *sem)
{
    kmt->spin_lock(&sem->lk);
    sem->value--;
    int sem_value = sem->value;
    if(sem_value<0)
    {
        sem_task_t *new_sem_task = pmm->alloc(sizeof(sem_task_t)); 
        //sem_task_t *new_sem_task = pmm->alloc(sizeof(new_sem_task)); 

        task_t* cur_task = mycpu()->current;
        cur_task->status = WAITING;
        //add cur_task to wait_list
        new_sem_task->task =cur_task;
        sem->sem_head.prev->next = new_sem_task;
        new_sem_task->prev = sem->sem_head.prev;
        new_sem_task->next = &sem->sem_head;
        sem->sem_head.prev = new_sem_task;
    }
    //p_semtks(sem);
    kmt->spin_unlock(&sem->lk);
    if(sem_value<0) _yield();
}
static void kmt_sem_signal(sem_t *sem)
{
    kmt->spin_lock(&sem->lk);
    sem->value++;
    task_t *set_thread=NULL;
    sem_task_t *to_throw=NULL;
    if(sem->sem_head.prev !=&sem->sem_head)
    {
        to_throw = sem->sem_head.prev;
        assert(to_throw->prev);
        sem->sem_head.prev = to_throw->prev;
        
        to_throw->prev->next = &sem->sem_head;
        //_putc('0'+_intr_read());
        set_thread = to_throw->task;
    }
    //p_semtks(sem);
    kmt->spin_unlock(&sem->lk);
    if(to_throw) pmm->free(to_throw);
    if(set_thread)
    {
        set_thread->status= RUNNING;
    }
}





MODULE_DEF(kmt)={
    .init=kmt_init,
    .create=kmt_create,
    .teardown=kmt_teardown,
    .spin_init=kmt_spin_init,
    .spin_lock=kmt_spin_lock,
    .spin_unlock=kmt_spin_unlock,
    .sem_init=kmt_sem_init,
    .sem_wait=kmt_sem_wait,
    .sem_signal=kmt_sem_signal
};


_Context* kmt_context_save(_Event ev, _Context* context)//call when locked
{
    legal_context(context);
    assert(mycpu()->current);
    if(!mycpu()->current->context)
    {
        //this is an idle
        assert(mycpu()->current==&idles[_cpu()]);
        assert(mycpu()->last==NULL);
        assert(mycpu()->current->status==IDLE);
    }
    else
    {
        assert(mycpu()->last);
    }
    //put last into queue. 
    //Remember that idles should never be into the queue
    if(mycpu()->last&&mycpu()->last->status!=IDLE) tasks_enque_lk(mycpu()->last);

    mycpu()->current->context = context;
    return NULL;
}
_Context* kmt_schedule(_Event ev, _Context* context)//call when locked
{
    //set last
    __attribute__((unused)) volatile task_t *last = mycpu()->last;
    mycpu()->last=mycpu()->current;
    //Now last has been set as current
    legal_context(context);
    kmt->spin_lock(&queue_lk);
    if(check_running_ulk())//whether queue is empty and whether there is running
    {
        task_t *transfer = tasks_deque_ulk();
        while(transfer->status!=RUNNING)//at least one thread is running
        {
            tasks_enque_ulk(transfer);
            transfer=tasks_deque_ulk();
        }
        mycpu()->current = transfer;
        //tasks_enque(transfer);
        //selected should not be into the queue
    }
    else    
    {
        mycpu()->current = &idles[_cpu()];
    }
    kmt->spin_unlock(&queue_lk);
    legal_context(mycpu()->current->context);
    return mycpu()->current->context;    
}


static void init_cpu_local()
{
    for(int i=0;i<_ncpu();i++)
    {
        cpu_local[i].current=&idles[i];//assume first interrupt must be idles
        cpu_local[i].intena=1;//might be trouble!!!!!!!
        cpu_local[i].ncli=0;
        cpu_local[i].id=i;
        cpu_local[i].last=NULL;
    }
}

static void init_idles()
{
    for(int i=0;i<MAX_CPU;i++)
    {
        idles[i].status=IDLE;
        idles[i].context=NULL;
        idles[i].name="idle";
        idles[i].next=NULL;
        idles[i].next=NULL;
    }
}


static struct cpu_local* mycpu()
{
    return &cpu_local[_cpu()];
}

static void pushcli()
{
    int intr=_intr_read();
    _intr_write(0);
    if(mycpu()->ncli == 0)  mycpu()->intena = intr;
    mycpu()->ncli += 1;
}

static void popcli()
{
    if(_intr_read())  panic("popcli - interruptible");
    if(--mycpu()->ncli < 0)   panic("popcli");
    if(mycpu()->ncli == 0 && mycpu()->intena) _intr_write(1);
}

static int holding(struct spinlock *lock)
{
  int r;
  pushcli();
  r = lock->locked && lock->cpu == mycpu();
  popcli();
  return r;
}

static void tasks_enque_lk(task_t *task)
{
    kmt->spin_lock(&queue_lk);
    task->prev = task_head.prev;
    task->next = &task_head;
    task->prev->next = task;
    task->next->prev = task;
    Assert(task->prev->next==task);
    Assert(task->next->prev==task);
    kmt->spin_unlock(&queue_lk);
}
static void tasks_enque_ulk(task_t *task)
{
    task->prev = task_head.prev;
    task->next = &task_head;
    task->prev->next = task;
    task->next->prev = task;
    Assert(task->prev->next==task);
    Assert(task->next->prev==task);
}

static task_t* tasks_deque_lk()
{   
    kmt->spin_lock(&queue_lk);
    task_t *res = task_head.next;
    if(task_head.next==&task_head)
    {
        kmt->spin_unlock(&queue_lk);
        return NULL;
    }
    else
    {
        task_head.next = res->next;
        res->next->prev = &task_head;    
    }
    Assert(task_head.next->prev==&task_head);
    Assert(task_head.prev->next==&task_head);
    kmt->spin_unlock(&queue_lk);
    res->prev=res->next=NULL;
    return res;
}


static task_t* tasks_deque_ulk()
{   
    task_t *res = task_head.next;
    if(task_head.next==&task_head)
    {
        return NULL;
    }
    else
    {
        task_head.next = res->next;
        res->next->prev = &task_head;    
    }
    Assert(task_head.next->prev==&task_head);
    Assert(task_head.prev->next==&task_head);
    res->prev=res->next=NULL;
    return res;
}


static void tasks_throw_lk(task_t* task)
{
    kmt->spin_lock(&queue_lk);
    Assert(tasks_check_ulk(task));
    Assert(task->status==DEAD);
    task_t *prev = task->prev;
    task_t *next = task->next;
    prev->next = next;
    next->prev = prev;
    kmt->spin_unlock(&queue_lk);
}

static void tasks_throw_ulk(task_t* task)
{
    Assert(tasks_check_ulk(task));
    Assert(task->status==DEAD);
    task_t *prev = task->prev;
    task_t *next = task->next;
    prev->next = next;
    next->prev = prev;
}

static int tasks_check_ulk(task_t *task)// should be used into a queue_lk
{
    for(task_t *cur = task_head.next;cur!=&task_head;cur=cur->next)
    {
        if(cur==task) return 1;
    }
    return 0;
}

static int check_running_lk()
{
    kmt->spin_lock(&queue_lk);
    for(task_t *cur = task_head.next;cur!=&task_head;cur=cur->next)
    {
        if(cur->status==RUNNING)
        {
            kmt->spin_unlock(&queue_lk);
            return 1;
        }
    }
    kmt->spin_unlock(&queue_lk);
    return 0;    
}


static int check_running_ulk()
{
    for(task_t *cur = task_head.next;cur!=&task_head;cur=cur->next)
    {
        if(cur->status==RUNNING)
        {
            return 1;
        }
    }
    return 0;    
}

void p_tks()
{
    for(task_t *cur = task_head.next;cur!=&task_head;cur=cur->next)
    {
        printf("%s:%d->",cur->name,cur->status);
    }
    printf("%s\n",task_head.name);
    printf("cur:%s,%d\n",mycpu()->current->name,mycpu()->current->status); 
    if(mycpu()->last)printf("last:%s,%d\n",mycpu()->last->name,mycpu()->last->status);   
}

void p_semtks(sem_t *sem)
{
    for(sem_task_t *cur = sem->sem_head.next;cur!=&sem->sem_head;cur=cur->next)
    {
        printf("%s:%d",cur->task->name,cur->task->status);
    }   
    _putc('\n');
}