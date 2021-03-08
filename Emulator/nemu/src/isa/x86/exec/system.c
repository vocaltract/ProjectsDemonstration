#include "cpu/exec.h"

make_EHelper(lidt) {
  if(id_dest->width == 4)
  {
    cpu.IDTR.LIMIT = vaddr_read(id_dest->addr, 2);
    cpu.IDTR.BASE = vaddr_read(id_dest->addr+2, 4);
  }
  else
  {
    cpu.IDTR.LIMIT = vaddr_read(id_dest->addr, 2);
    cpu.IDTR.BASE = vaddr_read(id_dest->addr+2, 3);
  }
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  switch (id_dest->reg)
  {
  case 0:
    cpu.cr0=id_src->val;
    break;
  case 3:
    cpu.cr3=id_src->val;
    break;
  default:assert(0);
    break;
  }
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  switch (id_src->reg)
  {
  case 0:
    s0 = cpu.cr0;
    break;
  case 3:
    s0 = cpu.cr3;
    break;
  default:assert(0);
    break;
  }
  rtl_sr(id_dest->reg, &s0, 4);
  /*switch (id_src->reg)
  {
  case 0:
    operand_write(id_dest, &cpu.cr0);
    break;
  case 3:
    operand_write(id_dest, &cpu.cr3);
    break;
  default:assert(0);
    break;
  }*/
  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

  difftest_skip_ref();
}

make_EHelper(int) {
  raise_intr(id_dest->val, *pc);

  print_asm("int %s", id_dest->str);

  difftest_skip_dut(1, 2);
}

make_EHelper(iret) {
  rtl_pop(&s1);
  rtl_j(s1);
  rtl_pop(&cpu.CS);
  rtl_pop(&cpu.eflags.value);
  print_asm("iret");
}

uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void pio_write_l(ioaddr_t, uint32_t);
void pio_write_w(ioaddr_t, uint32_t);
void pio_write_b(ioaddr_t, uint32_t);

make_EHelper(in) {
  if(id_dest->width==4){
    s0 = pio_read_l(id_src->val);
    operand_write(id_dest, &s0);
  }else if(id_dest->width==2){
    s0 = pio_read_w(id_src->val);
    operand_write(id_dest, &s0);
  }else
  {
    s0 = pio_read_b(id_src->val);
    operand_write(id_dest, &s0);
  }
  print_asm_template2(in);
}

make_EHelper(out) {
  if(id_src->width==4)
  {
    pio_write_l(id_dest->val,id_src->val);
  }
  else if(id_src->width==2)
  {
    pio_write_w(id_dest->val,id_src->val);
  }
  else
  {
    pio_write_b(id_dest->val,id_src->val);
  }
  

  print_asm_template2(out);
}
