#include "nemu.h"
#include "monitor/diff-test.h"
#include <assert.h>
#define check(name) ((*ref_r).name == cpu.name) 
#define check_f(name) ((*ref_r).eflags.name == cpu.eflags.name)
#define two(name) (*ref_r).name, cpu.name
#define etwo(name) (*ref_r).eflags.name, cpu.eflags.name
static int count = 0;

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {

  
  if(!check(eax)){
    printf("Problem in eax!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", two(eax));
    return false;
  } 
  if(!check(ecx)){
    printf("Problem in ecx!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", two(ecx));
    return false;
  }
  if(!check(edx)){
    printf("Problem in edx!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", two(edx));
    return false;
  }
  if(!check(ebx)){
    printf("Problem in ebx!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", two(ebx));
    return false;
  }
  if(!check(esp)){
    printf("Problem in esp!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", two(esp));
    return false;
  }
  if(!check(ebp)){
    printf("Problem in ebp!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", two(ebp));
    return false;
  }
  if(!check(esi)){
    printf("Problem in esi!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", two(esi));
    return false;
  }
  if(!check(edi)){
    printf("Problem in edi!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", two(edi));
    return false;
  }
  /*if(!check_f(OF)){
    printf("Problem in OF!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", etwo(OF));
    return false;
  }
  if(!check_f(CF)){
    printf("Problem in CF!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", etwo(CF));
    return false;
  }
  if(!check_f(ZF)){
    printf("Problem in ZF!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", etwo(ZF));
    return false;
  }    
  if(!check_f(SF)){
    printf("Problem in SF!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", etwo(SF));
    return false;
  }
  if(!check_f(IF)){
    printf("Problem in IF!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", etwo(IF));
    return false;
  }
  */
  //printf("0x%x\n", (*ref_r).eflags);
  count++;
  printf("the %d instruction,pc: 0x%x \n",count,cpu.pc);
  printf("eflags in nemu:\n");
  printf("CF:0x%x\nZF:0x%x\nSF:0x%x\nIF:0x%x\nOF:0x%x\n",cpu.eflags.CF, cpu.eflags.ZF,cpu.eflags.SF, cpu.eflags.IF,cpu.eflags.OF);
  if(cpu.pc != (*ref_r).pc){
    printf("Problem in pc!\n");
    printf("In qemu:0x%x\nIn nemu:0x%x\n", (*ref_r).pc, cpu.pc);
    return false;
  }
  return true;
  
}

void isa_difftest_attach(void) {
}
