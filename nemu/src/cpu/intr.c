#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  rtl_push(&cpu.val);
  rtl_push(&cpu.CS);
  rtl_push(&ret_addr);

  uint32_t temp1,temp2,jumpTarget;
  temp1 = vaddr_read(cpu.IDTR.Base + 8 * NO, 4);
  temp2 = vaddr_read(cpu.IDTR.Base + 8 * NO + 4, 4);
  jumpTarget = ((temp1 & 0x0000ffff) | (temp2 & 0xffff0000));
  
  decoding.is_jmp = 1;
  decoding.jmp_eip = jumpTarget;

  cpu.eflags.IF=0;
}

void dev_raise_intr() {
}
