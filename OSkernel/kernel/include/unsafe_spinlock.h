#include <stdint.h>
#include <stddef.h>
struct unsafe_spinlock
{
    intptr_t locked;
    intptr_t cpu_id;
};
typedef struct unsafe_spinlock unsafe_spinlock_t;
int unsafe_holding(unsafe_spinlock_t *lk);
void unsafe_acquire(unsafe_spinlock_t *lk);
void unsafe_release(unsafe_spinlock_t *lk);
void unsafe_init_lock(unsafe_spinlock_t *lk);


