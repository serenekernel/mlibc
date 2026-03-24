#include <bits/ensure.h>
#include <dirent.h>
#include <errno.h>
#include <mlibc/all-sysdeps.hpp>
#include <mlibc/debug.hpp>
#include <poll.h>
#include <serene/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>

#define STUB()                                                                 \
  ({                                                                           \
    __ensure(!"STUB function was called");                                     \
    __builtin_unreachable();                                                   \
  })

namespace mlibc {

[[noreturn]] void Sysdeps<Exit>::operator()(int status) {
  syscall(SYSCALL_EXIT, NULL, status);
  __builtin_unreachable();
}

void Sysdeps<LibcLog>::operator()(const char *message) {
  long ret;
  syscall(SYSCALL_DEBUG_LOG, &ret, (uint64_t)message, strlen(message));
}

[[noreturn]] void Sysdeps<LibcPanic>::operator()() {
  sysdep<LibcLog>("mlibc: panic");
  sysdep<Exit>(1);
}

int Sysdeps<Close>::operator()(int fd) { STUB(); }

int Sysdeps<Open>::operator()(const char *pathname, int flags, mode_t mode,
                              int *fd) {
  STUB();
};

int Sysdeps<Read>::operator()(int fd, void *buff, size_t count,
                              ssize_t *bytes_read) {
  STUB();
}

int Sysdeps<Write>::operator()(int fd, const void *buff, size_t count,
                               ssize_t *bytes_written) {
  long ret;
  bool err = syscall(SYSCALL_WRITE, &ret, fd, (uintptr_t)buff, count);
  if (err)
    return -1;
  *bytes_written = ret;
  return 0;
}

int Sysdeps<Seek>::operator()(int fd, off_t offset, int whence,
                              off_t *new_offset) {
  return 0;
}

int Sysdeps<ClockGet>::operator()(int clock, time_t *secs, long *nanos) {
  STUB();
}

int Sysdeps<VmMap>::operator()(void *hint, size_t size, int prot, int flags,
                               int fd, off_t offset, void **window) {
  STUB();
}

int Sysdeps<VmUnmap>::operator()(void *pointer, size_t size) { STUB(); }

int Sysdeps<TcbSet>::operator()(void *pointer) {
  long ret;
  syscall(SYSCALL_TCB_SET, &ret, (uint64_t)pointer);
  return ret;
}

int Sysdeps<FutexWait>::operator()(int *pointer, int expected,
                                   const struct timespec *time) {
  STUB();
}

int Sysdeps<FutexWake>::operator()(int *pointer, bool all) { STUB(); }
int Sysdeps<AnonAllocate>::operator()(size_t size, void **pointer) {
  long ret;
  bool err = syscall(SYS_MEM_ANON_ALLOC, &ret, size, size);
  if (err) {
    return -1;
  }
  *pointer = (void *)ret;
  return 0;
}
int Sysdeps<AnonFree>::operator()(void *pointer, size_t size) {
  long ret;
  bool err = syscall(SYS_MEM_ANON_FREE, &ret, (uintptr_t)pointer);
  return err ? -1 : 0;
}
int Sysdeps<Isatty>::operator()(int fd) { return 0; }

} // namespace mlibc
