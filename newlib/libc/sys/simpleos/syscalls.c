/* note these headers are all provided by newlib - you don't need to provide them */
#include <string.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/time.h>
#include <stdio.h>

#include <sys/stat.h>

#include <errno.h>
#undef errno
extern int errno;

#include <syscall.h>
#include <kernel/stat.h>

static inline _syscall1(SYS_SBRK, void*, sys_sbrk, int, size_delta)
static inline _syscall3(SYS_READ, int, sys_read, int, fd, void*, buf, uint, size)
static inline _syscall3(SYS_WRITE, int, sys_write, int, fd, const void*, buf, uint, size)
static inline _syscall2(SYS_OPEN, int, sys_open, const char*, path, int, flags)
static inline _syscall1(SYS_CLOSE, int, sys_close, int, fd)
static inline _syscall1(SYS_EXIT, int, sys_exit, int, exit_code)
static inline _syscall0(SYS_FORK, int, sys_fork)
static inline _syscall2(SYS_GETATTR_PATH, int, sys_getattr_path, const char*, path, fs_stat*, st)
static inline _syscall2(SYS_GETATTR_FD, int, sys_getattr_fd, int, fd, fs_stat*, st)
static inline _syscall2(SYS_EXEC, int, sys_exec, const char*, path, char* const *, argv);
static inline _syscall0(SYS_GET_PID, int, sys_get_pid);
static inline _syscall3(SYS_SEEK, int, sys_seek, int, fd, int, offset, int, whence)
static inline _syscall1(SYS_CURR_DATE_TIME, int, sys_curr_date_time, date_time*, dt)
static inline _syscall1(SYS_WAIT, int, sys_wait, int*, wait_status)
static inline _syscall1(SYS_UNLINK, int, sys_unlink, const char*, path)
static inline _syscall2(SYS_LINK, int, sys_link, const char*, old_path, const char*, new_path)
static inline _syscall3(SYS_RENAME, int, sys_rename, const char*, old_path, const char*, new_path, uint, flag)
static inline _syscall1(SYS_CHDIR, int, sys_chdir, const char *, path)
static inline _syscall2(SYS_GETCWD, int, sys_getcwd, char *, buf, size_t, buf_size)

static void fs_stat2stat(fs_stat* fs_st, struct stat* st)
{
  memset(st, 0, sizeof(*st));
  st->st_dev = (dev_t) fs_st->mount_point_id;
  st->st_ino = (ino_t) fs_st->inum;
  st->st_nlink = (nlink_t) fs_st->nlink;
  st->st_mode = (mode_t) fs_st->mode;
  st->st_size = (off_t) fs_st->size;
  st->st_blocks = (blkcnt_t) fs_st->blocks;
  st->st_blksize = 512;
  st->st_mtim = (struct timespec) { 
    .tv_sec = mktime((struct tm*) &fs_st->mtime)
    };
  st->st_ctim =  (struct timespec) { 
    .tv_sec = mktime((struct tm*) &fs_st->ctime)
  };
}


// Exit a program without cleaning up files. 
// If your system doesn’t provide this, it is best to avoid linking with subroutines that require it (exit, system).
void _exit(int return_code) {
  sys_exit(return_code);
  __builtin_unreachable(); // shall not return to here
}

// Close a file
int close(int file) {
  return sys_close(file);
}

// A pointer to a list of environment variables and their values. 
// For a minimal environment, this empty list is adequate:
char *__env[1] = { 0 };
char **environ = __env;

// Transfer control to a new process
int execve (const char *path, char * const argv[], char * const envp[]) {
  return sys_exec(path, argv);
}

// Create a new process
int fork(void) {
  return sys_fork();
}

// Status of an open file
// For consistency with other minimal implementations in these examples, all files are regarded as character special devices.
int fstat(int file, struct stat *st) {
  fs_stat fs_st = {0};
  int res = sys_getattr_fd(file, &fs_st);
  if(res < 0) {
    return res;
  }
  fs_stat2stat(&fs_st, st);
  return res;
}

// Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes.
int getpid(void) {
  return sys_get_pid();
}

// Query whether output stream is a terminal
// For consistency with the other minimal implementations, which only support output to stdout
int isatty(int file) {
  struct stat st = {0};
  int res = fstat(file, &st);
  if(res < 0) {
    return st.st_mode == S_IFCHR;
  } else {
    return 0;
  }
}

// Send a signal
int kill(int pid, int sig) {
  errno = EINVAL;
  return -1;
}

// Establish a new name for an existing file
int link(const char *old, const char *new) {
  return sys_link(old, new);
}

int _rename(const char* old, const char* new)
{
  return sys_rename(old, new, 0);
}

// Set position in a file
off_t lseek(int file, off_t offset, int whence) {
  return (off_t) sys_seek(file, (int) offset, whence);
}

// Open a file
int open(const char *name, int flags, ...) {
  return sys_open(name, flags);
}

// Read from a file
int read(int file, void *ptr, size_t len) {
  return sys_read(file, ptr, (uint) len);
}

// Increase program data space
void* sbrk(ptrdiff_t incr) {
  return sys_sbrk((int) incr);
}


// Status of a file (by name)
int stat(const char *file, struct stat *st) {
  fs_stat fs_st = {0};
  int res = sys_getattr_path(file, &fs_st);
  if(res < 0) {
    return res;
  }
  fs_stat2stat(&fs_st, st);
  return 0;
}

// Timing information for current process
clock_t times(struct tms *buf) {
  return -1;
}

// Remove a file’s directory entry
int unlink(const char *name) {
  return sys_unlink(name);
}

// Wait for a child process
int wait(int *status) {
  return sys_wait(status);
}

// Write to a file. libc subroutines will use this system routine for output to all files, including stdout
int write(int file, const void *ptr, size_t len) {
  return sys_write(file, ptr, (uint) len);
}

// Get current clock time
int gettimeofday(struct timeval* tp, void* tz)
{
  date_time dt = {0};
  int res = sys_curr_date_time(&dt);
  if(res < 0) {
    return res;
  }
  tp->tv_sec = mktime((struct tm*) &dt);
  tp->tv_usec = 0;
  return 0;
}

int chdir(const char* path)
{
  return sys_chdir(path);
}

char* getcwd(char* buf, size_t size)
{
  int r = sys_getcwd(buf, size);
  if(r < 0) {
    return NULL;
  } else {
    return buf;
  }
}