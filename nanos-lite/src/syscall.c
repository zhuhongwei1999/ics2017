#include "common.h"
#include "syscall.h"

extern char _end;

extern ssize_t fs_read(int fd, void *buf, size_t len);
extern ssize_t fs_write(int fd, const void *buf, size_t len);
extern int fs_open(const char *pathname, int flags, int mode);
extern off_t fs_lseek(int fd, off_t offset, int whence);
extern int fs_close(int fd);

extern int mm_brk(uint32_t new_brk);

uintptr_t sys_write(int fd, const void *buf, size_t len) {
	int i=0;
	if (fd==1||fd==2){
    Log("it's log\n");
		for(;len>0;len--,i++){
			_putc(((char*)buf)[i]);
		}
	}
	return i;
}


_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4],ret=-1;
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
	a[2] = SYSCALL_ARG3(r);
	a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:{
      ret=1; 
      break;
    }
    case SYS_exit:{
      _halt(a[1]);
      break;
    }
    case SYS_write:{
      ret=fs_write(a[1],(void *)a[2],a[3]);
			break;  
    }
    case SYS_brk:{
      // ret=0;
      ret=mm_brk(a[1]);
			break;
    }
		case SYS_read:{
      ret=fs_read(a[1],(void *)a[2],a[3]);
			break;  
    }
    case SYS_open:{
      ret=fs_open((char *)a[1],a[2],a[3]);
      break;
    }
		case SYS_close:{
      ret=fs_close(a[1]);
      break;
    }
		case SYS_lseek:{
      ret=fs_lseek(a[1],a[2],a[3]);
      break;
    }
			
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  SYSCALL_ARG1(r)=ret;
  
  return NULL;
}
