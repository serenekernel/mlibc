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
// Misc
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

int Sysdeps<TcbSet>::operator()(void *pointer) {
  long ret;
  if (syscall(SYSCALL_TCB_SET, &ret, (uint64_t)pointer)) {
    return -ret;
  }

  return 0;
}
// File IO

int Sysdeps<Close>::operator()(int fd) {
  long ret;
  bool err = syscall(SYSCALL_CLOSE, &ret, fd);
  if (err) {
    return -ret;
  }
  return 0;
}

int Sysdeps<Open>::operator()(const char *pathname, int flags, mode_t mode,
                              int *fd) {
  long ret;
  bool err = syscall(SYSCALL_OPEN, &ret, (uintptr_t)pathname, strlen(pathname),
                     flags, mode);
  if (err) {
    return -ret;
  }
  *fd = ret;
  return 0;
};

int Sysdeps<Read>::operator()(int fd, void *buff, size_t count,
                              ssize_t *bytes_read) {
  long ret;
  bool err = syscall(SYSCALL_READ, &ret, fd, (uintptr_t)buff, count);
  if (err) {
    return -ret;
  }
  *bytes_read = ret;
  return 0;
}

int Sysdeps<Write>::operator()(int fd, const void *buff, size_t count,
                               ssize_t *bytes_written) {
  long ret;
  bool err = syscall(SYSCALL_WRITE, &ret, fd, (uintptr_t)buff, count);
  if (err) {
    return -ret;
  }
  *bytes_written = ret;
  return 0;
}

int Sysdeps<Seek>::operator()(int fd, off_t offset, int whence,
                              off_t *new_offset) {
  long ret;
  bool err = syscall(SYSCALL_SEEK, &ret, fd, offset, whence);
  if (err) {
    return -ret;
  }
  *new_offset = ret;
  return 0;
}

// Memory
int Sysdeps<VmMap>::operator()(void *hint, size_t size, int prot, int flags,
                               int fd, off_t offset, void **window) {
  long ret;
  bool err = syscall(SYSCALL_VM_MAP, &ret, (uint64_t)hint, size, prot, flags,
                     fd, offset);
  if (err) {
    return -ret;
  }
  *window = (void *)ret;
  return 0;
}

int Sysdeps<VmUnmap>::operator()(void *pointer, size_t size) {
  long ret;
  bool err = syscall(SYSCALL_VM_UNMAP, &ret, (uint64_t)pointer, size);
  if (err) {
    return -ret;
  }
  return 0;
}

int Sysdeps<VmProtect>::operator()(void *pointer, size_t size, int prot) {
  long ret;
  bool err = syscall(SYSCALL_VM_PROTECT, &ret, (uint64_t)pointer, size, prot);
  if (err) {
    return -ret;
  }
  return 0;
}

int Sysdeps<AnonAllocate>::operator()(size_t size, void **pointer) {
  size += 4096 - (size % 4096);
  return sysdep<VmMap>(NULL, size, PROT_READ | PROT_WRITE,
                       MAP_ANON | MAP_PRIVATE, 0, 0, pointer);
}
int Sysdeps<AnonFree>::operator()(void *pointer, size_t size) {
  size += 4096 - (size % 4096);
  return sysdep<VmUnmap>(pointer, size);
}

// Stubs
int Sysdeps<FutexWait>::operator()(int *pointer, int expected,
                                   const struct timespec *time) {
  STUB();
}

int Sysdeps<FutexWake>::operator()(int *pointer, bool all) { STUB(); }
int Sysdeps<Stat>::operator()(fsfd_target fsfdt, int fd, const char *path,
                              int flags, struct stat *statbuf) {
  STUB();
}
int Sysdeps<Isatty>::operator()(int fd) { return 0; }
pid_t Sysdeps<GetTid>::operator()() { STUB(); }
gid_t Sysdeps<GetGid>::operator()() { return 0; }
gid_t Sysdeps<GetEgid>::operator()() { return 0; }
uid_t Sysdeps<GetUid>::operator()() { return 0; }
uid_t Sysdeps<GetEuid>::operator()() { return 0; }

pid_t Sysdeps<GetPid>::operator()() { STUB(); }
pid_t Sysdeps<GetPpid>::operator()() { STUB(); }
int Sysdeps<Dup2>::operator()(int fd, int flags, int newfd) { STUB(); }

int Sysdeps<ClockGet>::operator()(int clock, time_t *secs, long *nanos) {
  STUB();
}
} // namespace mlibc
