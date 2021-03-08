#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>
#define MAX_CPU 8
#define INT_MAX 0x7fffffff
#define INT_MIN 0x80000000
enum TaskStatus{
    RUNNING=1,
    WAITING,
    HEADER,
    DEAD,
    IDLE
};



struct task{
    const char *name;
    _Context *context;//might be buggy!!!!
    _Area stack;
    struct task *prev;
    struct task *next;
    enum TaskStatus status;
};

struct cpu_local {
  task_t *current;
  task_t *last;
  int ncli;
  int intena;
  int id;
}; 
extern struct cpu_local cpu_local[MAX_CPU];

struct spinlock{
    intptr_t locked;
    const char *name;
    struct cpu_local *cpu;
};
typedef struct sem_task{
  task_t *task;
  struct sem_task *prev;
  struct sem_task *next;
}sem_task_t;


struct semaphore{
  int value;
  spinlock_t lk;
  const char *name;
  sem_task_t sem_head;
};





// union kstack{ 
//   int32_t magic1;
//   uint8_t data[4096];
//   struct{
//     uint8_t aligns[4092];
//     int32_t magic2;
//   }__attribute__((packed));
// }__attribute__((packed));
// typedef union kstack kstack_t;