#include "unsafe_spinlock.h"
#include <stdint.h>
#define LOCK_SZ (__SIZEOF_POINTER__*2)
#define PAGE_SZ 0x11000
#define HDR_SIZE 0x1000
#define AVAIL_SZ (PAGE_SZ-HDR_SIZE)
#define NUM_BITMAP(x) ((AVAIL_SZ-1)/((x)*8*__SIZEOF_POINTER__)+1)
#define COMMON_MAP NUM_BITMAP(__SIZEOF_POINTER__)

//64位最少分配8字节
//32位最少分配4字节

typedef union my_page {
  struct {
    union my_page *page_list;
    int is_full;
    unsafe_spinlock_t lock; // 锁，用于串行化分配和并发的 free
    size_t type;
    size_t bitmap[NUM_BITMAP(__SIZEOF_POINTER__)];
  }; // 匿名结构体
  struct {
    uint8_t page_header[HDR_SIZE];
    uint8_t data[PAGE_SZ - HDR_SIZE];
  };
} page_t;
#if __SIZEOF_POINTER__==8
#define UP_POW2(x) (((x)>8)?1<<(64-__builtin_clzl((x)-1)):8)
#else  
#define UP_POW2(x) (((x)>4)?1<<(32-__builtin_clz((x)-1)):4)
#endif

#define MASK(x) ((x)&(~(x)+1))//从右向左第一个1