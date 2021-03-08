#include "cpu/exec.h"
#include "cc.h"
#include<stdio.h>

make_EHelper(test) {
  rtl_and(&s0,&(id_dest->val),&(id_src->val));
  rtl_update_ZFSF(&s0,id_dest->width);
  s0 = 0;
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);
  print_asm_template2(and);
  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&s0,&(id_dest->val),&(id_src->val));
  operand_write(id_dest,&s0);
  rtl_update_ZFSF(&s0,id_dest->width);
  s0 = 0;
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);
  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&s0,&(id_dest->val),&(id_src->val));
  operand_write(id_dest,&s0);
  /*if(id_src->type == OP_TYPE_REG){
    rtl_sr(id_dest->reg, &s0, id_dest->width);
  }
  else{
    rtl_sm(&(id_dest->addr),&s0, id_dest->width);
  } */
  rtl_update_ZFSF(&s0,id_dest->width);
  s0 = 0;
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);
  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&s0,&(id_dest->val),&(id_src->val));
  operand_write(id_dest,&s0);
  rtl_update_ZFSF(&s0,id_dest->width);
  s0 = 0;
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);

  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_sar(&s0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&s0);
  rtl_update_ZFSF(&s0,id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_shl(&s0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&s0);
  rtl_update_ZFSF(&s0,id_dest->width);

  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&s0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&s0);
  rtl_update_ZFSF(&s0,id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}


make_EHelper(rol) {
  s1 = id_dest->val;
 
  if(id_src->val==1)
  {
    while(id_src->val>0){
      rtl_msb(&s0, &s1, id_dest->width);
      rtl_set_CF(&s0);
      rtl_shli(&s1, &s1, 1);
      rtl_add(&s1, &s1, &s0);
      id_src->val--;
    }
    operand_write(id_dest, &s1);
    rtl_msb(&s0, &s1, id_dest->width);
    rtl_get_CF(&s1);
    if(s0 == s1)
    {
      s0 = 0;
      rtl_set_OF(&s0);
    }
    else
    {
      s0 = 1;
      rtl_set_OF(&s0);
    }
  }
  else
  {
    while(id_src->val>0){
      rtl_msb(&s0, &s1, id_dest->width);
      rtl_set_CF(&s0);
      rtl_shli(&s1, &s1, 1);
      rtl_add(&s1, &s1, &s0);
      id_src->val--;
    }
    operand_write(id_dest, &s1);
  }
  
}

make_EHelper(setcc) {
  uint32_t cc = decinfo.opcode & 0xf;

  rtl_setcc(&s0, cc);
  operand_write(id_dest, &s0);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
  s0 = 0xffffffff;
  rtl_xor(&s0, &s0, &id_dest->val);
  operand_write(id_dest, &s0);//会根据宽度进行截断，所以不用担心

  print_asm_template1(not);
}
