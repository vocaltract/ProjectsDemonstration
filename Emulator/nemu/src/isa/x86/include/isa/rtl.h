#ifndef __X86_RTL_H__
#define __X86_RTL_H__

#include "rtl/rtl.h"

/* RTL pseudo instructions */

/*static inline rtlreg_t judge_effective_len(rtlreg_t num){
  int count = 0;
  while(num){
    num = num>>1;
    count++;
  }
  return count;
}
*/

static inline void rtl_lr(rtlreg_t* dest, int r, int width) {//读取
  switch (width) {
    case 4: rtl_mv(dest, &reg_l(r)); return;
    case 1: rtl_host_lm(dest, &reg_b(r), 1); return;
    case 2: rtl_host_lm(dest, &reg_w(r), 2); return;
    default: assert(0);
  }
}

static inline void rtl_sr(int r, const rtlreg_t* src1, int width) {//写入
  switch (width) {
    case 4: rtl_mv(&reg_l(r), src1); return;
    case 1: rtl_host_sm(&reg_b(r), src1, 1); return;
    case 2: rtl_host_sm(&reg_w(r), src1, 2); return;
    default: assert(0);
  }
}

static inline void rtl_push(const rtlreg_t* src1) {
  // esp <- esp - 4
  // M[esp] <- src1
  //TODO();
  cpu.esp -=4;
  vaddr_write(cpu.esp,*src1,4);


}

static inline void rtl_pop(rtlreg_t* dest) {
  // dest <- M[esp]
  // esp <- esp + 4
  *dest = vaddr_read(cpu.esp,4);
  //TODO();
  cpu.esp += 4;
}

static inline void rtl_is_sub_overflow(rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) {
  // dest <- is_overflow(src1 - src2)
  *dest = ~*src2 +1;//特殊情况
  t1 = *src1 >> 31;
  if((t1 == 0)&& *dest == 0x80000000){
    *dest = 1;
    return ;
  }
  else if((t1 == 1)&& *dest == 0x80000000){
    *dest = 0;
    return ;
  }
  rtl_msb(&t0,src1,width);
  rtl_msb(&t1,dest,width);
  if(t0==t1){
    rtl_msb(&t1,res, width);
    if(t0 != t1){
      *dest = 1;
      return ;
    }
  }
  *dest =0;
}

static inline void rtl_is_sub_carry(rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1) {
  // dest <- is_carry(src1 - src2)
    if((*res)>(*src1)){
      *dest =1;
    }
    else{
      *dest=0;
    }
}

static inline void rtl_is_add_overflow(rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) {
  // dest <- is_overflow(src1 + src2)
  rtl_msb(&t0,src1,width);
  rtl_msb(&t1,src2,width);
  if(t0==t1){
    rtl_msb(&t1,res, width);
    if(t0 != t1){
      *dest = 1;
      return ;
    }
  }
  *dest =0;
}

static inline void rtl_is_add_carry(rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1) {
  // dest <- is_carry(src1 + src2)
    if((*res)<(*src1)){//||judge_effective_len(*res)>judge_effective_len(*src1)){
      *dest =1;
    }
    else{
      *dest=0;
    }
  }


#define make_rtl_setget_eflags(f) \
  static inline void concat(rtl_set_, f) (const rtlreg_t* src) { \
    cpu.eflags.f = *src;         \
  } \
  static inline void concat(rtl_get_, f) (rtlreg_t* dest) { \
    *dest = cpu.eflags.f;\
  }

make_rtl_setget_eflags(CF)
make_rtl_setget_eflags(OF)
make_rtl_setget_eflags(ZF)
make_rtl_setget_eflags(SF)

static inline void rtl_update_ZF(const rtlreg_t* result, int width) {
  // eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
  rtl_shli(&ir, result, 32 - width * 8);
	if(ir == 0) cpu.eflags.ZF = 1;
  else cpu.eflags.ZF = 0;
}

static inline void rtl_update_SF(const rtlreg_t* result, int width) {
  // eflags.SF <- is_sign(result[width * 8 - 1 .. 0])
  rtl_shri(&ir,result,width*8-1);
  rtl_set_SF(&ir);
}

static inline void rtl_update_ZFSF(const rtlreg_t* result, int width) {
  rtl_update_ZF(result, width);
  rtl_update_SF(result, width);
}

#endif
