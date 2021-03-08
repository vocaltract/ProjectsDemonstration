#include "rtl/rtl.h"

#define IRQ_TIMER 32

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  //vaddr_read的神秘现象告破
  //原本地址存储的为fa06 0800 008f 1000，最左边为cpu.IDTR.BASE+NO*8+0位置对应的东西
  //由于是小端存储，我们在解释的时候，需要把它反过来。
  //如果我们用vaddr_read(cpu.IDTR.BASE+NO*8,4)，它得到fa060800对应应该解释为000806fa
  //如果我们用vaddr_read(cpu.IDTR.BASE+NO*8+2,2), 它得到0800,解释为0008
  //IDT数组一个元素8字节所以NO要乘以8
  //观察结构体的定义可知，排在最前面的是低16位，最后面是高16位
    rtl_push(&cpu.eflags.value);
    cpu.eflags.IF=0;
    rtl_push(&cpu.CS);
    rtl_push(&ret_addr);
    s0 = vaddr_read(cpu.IDTR.BASE+NO*8,2);//低位 06fa
    s1 = vaddr_read(cpu.IDTR.BASE+NO*8+6,2);//高位 0010
    s0 = (s1<<16)|s0;
    rtl_j(s0);

}

bool isa_query_intr(void) {
  if (cpu.eflags.IF==1 && cpu.INTR==1) {
    cpu.INTR = false;
    raise_intr(IRQ_TIMER, cpu.pc);
    return true;
  }
  return false;
}
