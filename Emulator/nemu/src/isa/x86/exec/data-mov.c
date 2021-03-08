#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(xchg){



  print_asm_template1(dec);
}


make_EHelper(pop) {
  rtl_pop(&s0);
  operand_write(id_dest,&s0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  s0 = cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&s0);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);
  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&s0);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);
  print_asm("popa");
}

make_EHelper(leave) {
  rtl_lr(&s0, R_EBP, id_dest->width);//对于BP和SP的情况编号恰好是一样的
  rtl_sr(R_ESP, &s0, id_dest->width);
  rtl_pop(&s0);
  rtl_sr(R_EBP, &s0, id_dest->width);
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decinfo.isa.is_operand_size_16) {
    s1 = 15;
    rtl_lr(&s0, R_AX, 2);
    rtl_shr(&s0,&s0,&s1);
    if(s0== 1){
      s0 = 0xffff;
      rtl_sr(R_DX,&s0,2);
    }
    else{
      s0 = 0;
      rtl_sr(R_DX,&s0,2);
    }
  }
  else {
    s1 =31;
    rtl_lr(&s0, R_EAX, 4);
    rtl_shr(&s0,&s0,&s1);
    if(s0==1){
      s0 = 0xffffffff;
      rtl_sr(R_EDX,&s0,4);
    }
    else{
      s0 = 0;
      rtl_sr(R_EDX,&s0,4);
    }    
  }

  print_asm(decinfo.isa.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decinfo.isa.is_operand_size_16) {
    rtl_lr(&s0, R_AL,2);
    rtl_sext(&s0, &s0, 2);
    rtl_sr(R_AX, &s0, 2);
  }
  else {
    rtl_lr(&s0, R_AX,2);
    rtl_sext(&s0, &s0, 2);
    rtl_sr(R_EAX, &s0, 4);
  }

  print_asm(decinfo.isa.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  rtl_sext(&s0, &id_src->val, id_src->width);
  operand_write(id_dest, &s0);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}
