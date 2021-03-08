#include "unsafe_spinlock.h"
#include "common.h"

void unsafe_acquire(struct unsafe_spinlock *lk)
{
    //if(holding(lk))
    //    panic("Lock Error!");
    while(_atomic_xchg(&(lk->locked), 1));
    //__sync_synchronize();
    //lk->cpu_id = _cpu();
}

int unsafe_holding(struct unsafe_spinlock *lk)
{
   return lk->locked && lk->cpu_id == _cpu();
}

void unsafe_release(struct unsafe_spinlock *lk)
{
    //if(!holding(lk))
    //    panic("Unlock Error!");
    //lk->cpu_id=-1;
    //__sync_synchronize();
    _atomic_xchg(&lk->locked, 0);
}
void unsafe_init_lock(struct unsafe_spinlock *lk)
{
  lk->locked = 0;
  lk->cpu_id = -1;
}
