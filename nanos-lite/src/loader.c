#include "common.h"
#define DEFAULT_ENTRY ((void *)0x4000000)
extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);
uintptr_t loader(_Protect *as, const char *filename) {
//将ramdisk 中从0 开始的所有内容放置在0x4000000,并
//把这个地址作为程序的入口返回即可.
ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());
return (uintptr_t)DEFAULT_ENTRY;
}