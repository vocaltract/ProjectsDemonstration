#include "cpu/exec.h"
//#include<stdio.h>
CPU_state cpu;

rtlreg_t s0, s1, t0, t1, ir;

/* shared by all helper functions */
DecodeInfo decinfo;
bool isa_query_intr(void);
void decinfo_set_jmp(bool is_jmp) {
  decinfo.is_jmp = is_jmp;
}

void isa_exec(vaddr_t *pc);

vaddr_t exec_once(void) {
  decinfo.seq_pc = cpu.pc;
  isa_exec(&decinfo.seq_pc);//src/isa/x86/exec/exec.c:195
  update_pc();//include/cpu/exec.h decinfo.is_jmp==0就update,否则不update 把decinfo.is_jmp置成0
  if (isa_query_intr()) update_pc();
  return decinfo.seq_pc;
}
