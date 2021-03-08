//#define DEBUG
#include <common.h>
#include "pmm.h"
#include "debug.h"

static inline void init_page(page_t *page,size_t type);
static inline size_t loc(size_t map, size_t type);
static inline void *kalloc_unsafe(size_t size);
static inline void kfree_unsafe(void* ptr); 
unsafe_spinlock_t global_lk;
//Node_t sentinel;

page_t *page_senti[MAX_CPU];//max smp==8
page_t *tail_page[MAX_CPU];
uintptr_t last_page_addr=0;


static void* kalloc(size_t size)
{
  int intr_val = _intr_read();
  _intr_write(0);
  void *ret = kalloc_unsafe(size);
  if (intr_val) _intr_write(1);
  return ret;
}
static void kfree(void *ptr)
{
  int intr_val = _intr_read();
  _intr_write(0);
  kfree_unsafe(ptr);
  if (intr_val) _intr_write(1);
}
static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
  unsafe_init_lock(&global_lk);
  printf("Got %d MiB heap: [%x, %x)\n", pmsize >> 20, _heap.start, _heap.end);
  for(int i=0;i<_ncpu();i++)
  {
    tail_page[i]=page_senti[i] = (page_t *)((uintptr_t)_heap.start+i*PAGE_SZ);
    init_page(page_senti[i],__SIZEOF_POINTER__);
  }
  last_page_addr = (uintptr_t)_heap.start+_ncpu()*PAGE_SZ;
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};


static inline void *kalloc_unsafe(size_t size) 
{
  if(!size) return NULL;
  void *res =NULL;
  size = UP_POW2(size);//content size
  int select_cpu = _cpu();
  page_t *page=page_senti[select_cpu];
  for(;page!=NULL;page=page->page_list)
  {
    unsafe_acquire(&page->lock);
    if(page->type==size &&!page->is_full)
    {
      for(int i=0;i<NUM_BITMAP(size);i++)
      {
        if(MASK(page->bitmap[i]))
        {
          res = (void *)(loc(page->bitmap[i],size)*size+(uintptr_t)&page->data+i*__SIZEOF_POINTER__*8*size);
          Assert(((((uintptr_t)res)%size)==0));
          page->bitmap[i] &= ~(MASK(page->bitmap[i]));
          if(i==NUM_BITMAP(size)-1)
          {
#if __SIZEOF_POINTER__==8
            switch (size)
            {
            case 4096:
            {
              if((loc(page->bitmap[i],size))==63-48)
              {
                page->is_full=1;
              }
            }
            break;
            case 2048:
            {
              if((loc(page->bitmap[i],size))==63-32)
              {
                page->is_full=1;
              }
            }
            break;
            default:
            {
              if((loc(page->bitmap[i],size))==63)
              {
                page->is_full=1;
              }
            }  
            break;
            }
#else
            switch (size)
            {
            case 4096:
            {
              if((loc(page->bitmap[i],size))==31-16)
              {
                page->is_full=1;
              }
            }
            break;
            default:
            {
              if((loc(page->bitmap[i],size))==31)
              {
                page->is_full=1;
              }
            }  
            break;
            }          
#endif    
          }
          break;
        }
      }
    } 
    unsafe_release(&page->lock);
    if(res) break;
  }
  if(!res)
  {
    unsafe_acquire(&global_lk);
    page_t *new_page = (page_t *)last_page_addr;
    if(last_page_addr>=(uintptr_t)_heap.end-PAGE_SZ)
    {
      unsafe_release(&global_lk);
      return NULL;
    }
    last_page_addr += PAGE_SZ;
    unsafe_release(&global_lk); 
    init_page(new_page,size);
    res = (void *)(loc(new_page->bitmap[0],size)*size+(uintptr_t)&new_page->data+0*__SIZEOF_POINTER__*8*size);
    new_page->bitmap[0] &= ~(MASK(new_page->bitmap[0]));
    tail_page[select_cpu]->page_list=new_page;
    tail_page[select_cpu] = new_page;
  }
  return res;
}


static void kfree_unsafe(void *ptr) {
  page_t *cur_page =(page_t *)(((uintptr_t)ptr-(uintptr_t)_heap.start)/PAGE_SZ*PAGE_SZ+(uintptr_t)_heap.start);
  size_t num_bits = ((uintptr_t)ptr-(uintptr_t)&cur_page->data)/cur_page->type;
  size_t n_p = num_bits/8/__SIZEOF_POINTER__;
  size_t n_b = num_bits%(8*__SIZEOF_POINTER__);
  switch (cur_page->type)
  {
  case 4096:
    {
      Assert(n_p==0);
      unsafe_acquire(&cur_page->lock);
      cur_page->is_full=0;
#if __SIZEOF_POINTER__==8
      cur_page->bitmap[0] |= 1UL<< (n_b+48);
#else
      cur_page->bitmap[0] |= 1UL<< (n_b+16);
#endif
      unsafe_release(&cur_page->lock);
    }
  break;
  case 2048:
    {
      Assert(n_p==0);
      unsafe_acquire(&cur_page->lock);
      cur_page->is_full=0;
#if __SIZEOF_POINTER__==8
      cur_page->bitmap[0] |= 1UL<< (n_b+32);
#else
      cur_page->bitmap[0] |= 1UL<< (n_b);
#endif
      unsafe_release(&cur_page->lock);
    }
  break;  
  case 1024:
    {
      Assert(n_p==0);
      unsafe_acquire(&cur_page->lock);
      cur_page->is_full=0;
#if __SIZEOF_POINTER__==8
      cur_page->bitmap[0] |= 1UL<< (n_b);
#else 
      cur_page->bitmap[0] |= 1UL<< (n_b);
#endif
      unsafe_release(&cur_page->lock);
    }
  break;
  case 512:
    {
      unsafe_acquire(&cur_page->lock);
      cur_page->is_full=0;
#if __SIZEOF_POINTER__==8
      Assert(n_p==0);
#endif
      cur_page->bitmap[n_p] |= 1UL<< (n_b);
      unsafe_release(&cur_page->lock);
    }
  break;
  default:
    {
      unsafe_acquire(&cur_page->lock);
      cur_page->is_full=0;
      cur_page->bitmap[n_p] |= 1UL<< (n_b);
      unsafe_release(&cur_page->lock);      
    }
    break;
  }
}


static inline void init_page(page_t *page,size_t type)//没有把page串起来
{
  unsafe_init_lock(&page->lock);
  page->is_full=0;
  page->page_list=NULL;
  page->type = type;
  switch (type)
  {
#if __SIZEOF_POINER__==4
  case 4:
  {
    for(int i=0;i<NUM_BITMAP(4);i++)
    {
      page->bitmap[i]=0xffffffff;
    }
  }
  break;
#endif
  case 8:
  {
    for(int i=0;i<NUM_BITMAP(8);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffffffffffffffff;
#else
      page->bitmap[i]=0xffffffff;
#endif
    }
  }
  break;
  case 16:
  {
    for(int i=0;i<NUM_BITMAP(16);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffffffffffffffff;
#else
      page->bitmap[i]=0xffffffff;
#endif
    }
  }
  break;
  case 32:
  {
    for(int i=0;i<NUM_BITMAP(32);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffffffffffffffff;
#else
      page->bitmap[i]=0xffffffff;
#endif
    }
  }
  break;
  case 64:
  {
    for(int i=0;i<NUM_BITMAP(64);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffffffffffffffff;
#else
      page->bitmap[i]=0xffffffff;
#endif
    }
  }
  break;
  case 128:
  {
    for(int i=0;i<NUM_BITMAP(128);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffffffffffffffff;
#else
      page->bitmap[i]=0xffffffff;
#endif
    }
  }
  break;
  case 256:
  {
    for(int i=0;i<NUM_BITMAP(256);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffffffffffffffff;
#else
      page->bitmap[i]=0xffffffff;
#endif
    }
  }
  break;
  case 512:
  {
    for(int i=0;i<NUM_BITMAP(512);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffffffffffffffff;
#else
      page->bitmap[i]=0xffffffff;
#endif
    }
  }
  break;
  case 1024://!!!!!
  {
    for(int i=0;i<NUM_BITMAP(1024);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffffffffffffffff;
#else
      page->bitmap[i]=0xffffffff;
#endif
    }
  }
  break;
  case 2048://!!!!!
  {
    for(int i=0;i<NUM_BITMAP(2048);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffffffff00000000;
#else
      page->bitmap[i]=0xffffffff;
#endif
    }
  }
  break;
  case 4096://!!!!!
  {
    for(int i=0;i<NUM_BITMAP(4096);i++)
    {
#if __SIZEOF_POINTER__==8
      page->bitmap[i]=0xffff000000000000;
#else
      page->bitmap[i]=0xffff0000;
#endif
    }
  }
  break;
  default:Assert(0);
    break;
  }

}

static inline size_t loc(size_t map, size_t type)
{
  Assert(map!=0);
#if __SIZEOF_POINTER__==8
  if(type==4096)
  {
    return __builtin_ctzl(map)-48;
  }
  else if(type==2048)
  {
    return __builtin_ctzl(map)-32;
  }
  else
  {
    return __builtin_ctzl(map);
  }
  

#else
  if(type==4096)
  {
    return __builtin_ctz(map)-16;
  }
  else
  {
    return __builtin_ctz(map);
  }

#endif
  return 0;
}