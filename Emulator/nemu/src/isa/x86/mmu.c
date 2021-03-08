#include "nemu.h"
#define PTXSHFT        12 
#define PDXSHFT        22 
typedef uint32_t PTE;
typedef uint32_t PDE;
#define PDX(va)          (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)          (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define PAGEDIFF(va)     ((uint32_t)(va) >> PTXSHFT)
#define OFF(va)          ((uint32_t)(va) & 0xfff)
#define PTE_ADDR(pte)    ((uint32_t)(pte) & ~0xfff)
//从x86.h里面抄了一些宏，美滋滋

paddr_t page_translate(vaddr_t va){//va就是virtual adddress
  if ((cpu.cr0>>31) == 1)
  {
    paddr_t PD_addr=cpu.cr3;
    PDE curPDE = paddr_read((PD_addr+PDX(va)*4),sizeof(PDE));
    assert((curPDE&0x1)==0x1);
    PTE curPTE = paddr_read((PTE_ADDR(curPDE)+PTX(va)*4),sizeof(PTE));
    assert((curPTE&0x1)==0x1);
    return (paddr_t) (PTE_ADDR(curPTE) | OFF(va));
  }
  return va;
}

uint32_t isa_vaddr_read(vaddr_t addr, int len) {//paddr_read奇坑无比，只支持1、2、4字节
  if(PAGEDIFF(addr)!=PAGEDIFF(addr+len-1))
  {
    int i=0;
    for(;i<len-1;i++){
      if(PAGEDIFF(addr+i)!=PAGEDIFF(addr+i+1))
        break;
    }
    assert(len==4);
    if(i == 1)
    {
      uint32_t fore = paddr_read(page_translate(addr),2);
      uint32_t back = paddr_read(page_translate(addr+2),2);
      return back<<16 | fore;//小端，要倒过来
    }
    else if(i == 2)
    {
      uint32_t fore = paddr_read(page_translate(addr),2);
      uint32_t mid = paddr_read(page_translate(addr+2),1);
      uint32_t back = paddr_read(page_translate(addr+3),1);
      return  back<<24 | mid<<16 | fore; //小端，要倒过来
    }
    else if(i == 0)
    {
      uint32_t fore = paddr_read(page_translate(addr),1);
      uint32_t mid = paddr_read(page_translate(addr+1),2);
      uint32_t back = paddr_read(page_translate(addr+3),1);
      return back << 24 | mid << 8 | fore;
    }
    else
    {
      assert(0);
    }
    
  }
  return paddr_read(page_translate(addr), len);
}

void isa_vaddr_write(vaddr_t addr, uint32_t data, int len) {//paddr_write奇坑无比，只支持1、2、4字节
  if(PAGEDIFF(addr)!=PAGEDIFF(addr+len-1))
  {
    int i=0;
    for(;i<len-1;i++){
      if(PAGEDIFF(addr+i)!=PAGEDIFF(addr+i+1))
        break;
    }      
    Log("i:%d\n\n",i);
    assert(i==1);
    assert(len==4);
    uint32_t low = data &0xffff;
    uint32_t high = data >>16;
    paddr_write(page_translate(addr),low,2);
    paddr_write(page_translate(addr+2),high,2);
  }
  paddr_write(page_translate(addr), data, len);
}

