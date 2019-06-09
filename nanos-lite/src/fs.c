#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = _screen.width * _screen.height * sizeof(uint32_t);
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}

void ramdisk_read(void *, uint32_t, uint32_t);
void ramdisk_write(const void *, uint32_t, uint32_t);
size_t events_read(void *buf, size_t len);
void dispinfo_read(void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);

int fs_open(const char *pathname, int flags, int mode) {
  int i;
  for (i = 0; i < NR_FILES; i ++) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }

  panic("No such file: %s", pathname);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
	ssize_t fs_size = fs_filesz(fd);
	//Log("in the read, fd = %d, file size = %d, len = %d, file open_offset = %d\n", fd, fs_size, len, file_table[fd].open_offset);
	switch(fd) {
		case FD_STDOUT:
		case FD_FB:
			//Log("in the fs_read fd_fb\n");
			break;
		case FD_EVENTS:
			len = events_read((void *)buf, len);
			break;
		case FD_DISPINFO:
			if (file_table[fd].open_offset >= file_table[fd].size)
				return 0;
			if (file_table[fd].open_offset + len > file_table[fd].size)
				len = file_table[fd].size - file_table[fd].open_offset;
			dispinfo_read(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;	
			break;
		default:
			if(file_table[fd].open_offset >= fs_size || len == 0)
				return 0;
			if(file_table[fd].open_offset + len > fs_size)
				len = fs_size - file_table[fd].open_offset;
			ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
	ssize_t fs_size = fs_filesz(fd);
	//Log("in the write, fd = %d, file size = %d, len = %d, file open_offset = %d\n", fd, fs_size, len, file_table[fd].open_offset);
	switch(fd) {
		case FD_STDOUT:
		case FD_STDERR:
			// call _putc()
			for(int i = 0; i < len; i++) {
					_putc(((char*)buf)[i]);
			}
			break;
		case FD_FB:
			// write to frame buffer
			fb_write(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		default:
			// write to ramdisk
			if(file_table[fd].open_offset >= fs_size)
				return 0;	
			if(file_table[fd].open_offset + len > fs_size)
				len = fs_size - file_table[fd].open_offset;
			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  Finfo *f = file_table + fd;
  int new_offset = f->open_offset;
  int file_size = file_table[fd].size;
  switch (whence) {
    case SEEK_CUR: new_offset += offset; break;
    case SEEK_SET: new_offset = offset; break;
    case SEEK_END: new_offset = file_size + offset; break;
    default: assert(0);
  }
  if (new_offset < 0) {
    new_offset = 0;
  }
  else if (new_offset > file_size) {
    new_offset = file_size;
  }
  f->open_offset = new_offset;

  return new_offset;
}

int fs_close(int fd) {
  return 0;
}