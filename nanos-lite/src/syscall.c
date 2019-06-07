#include "common.h"
#include "syscall.h"

uintptr_t sys_write(int fd, const void *buf, size_t len) {
	int i = 0;
	if (fd==1 || fd==2){
		for(; len>0; len--,i++){
			_putc(((char*)buf)[i]);
		}
	}
	return i;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4], ret = -1;
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
	a[2] = SYSCALL_ARG3(r);
	a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:{
      ret = 1; 
      break;
    }
    case SYS_exit:{
      _halt(a[1]);
      break;
    }
    case SYS_write:{
      ret = sys_write(a[1], (void *)a[2], a[3]);
      break;
    }
    case SYS_brk:{
      ret = 0;
      break;
    }

    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  SYSCALL_ARG1(r)=ret;

  return NULL;
}

