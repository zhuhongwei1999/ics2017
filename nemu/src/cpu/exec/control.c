#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  // printf("decoding.is_jmp:%d\n", t2);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  rtl_push(eip);//当前eip入栈
  rtl_addi(&decoding.jmp_eip,eip,id_dest->val);//用jmp_eip记录新的eip值
  decoding.is_jmp = 1;//修改标志
  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  decoding.is_jmp=1;
  rtl_pop(&t0);
  rtl_mv(&decoding.jmp_eip,&t0);
  print_asm("ret");
}

make_EHelper(call_rm) {
  rtl_push(eip);
  rtl_mv(&decoding.jmp_eip,&id_dest->val);
  decoding.is_jmp=1;
  print_asm("call *%s", id_dest->str);
}
