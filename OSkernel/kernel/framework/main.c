#include <kernel.h>
#include <klib.h>

int main() {
  _ioe_init();
  _cte_init(os->trap);
//  _vme_init(pmm->alloc, pmm->free);
  os->init();
  //printf("\n\n\n\n\n");
  //printf("%x,%x\n",(uintptr_t)_heap.start,_heap.end);
  _mpe_init(os->run);


  return 1;
}
