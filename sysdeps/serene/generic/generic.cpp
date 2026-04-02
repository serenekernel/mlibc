#include <asm/ioctls.h>
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

#define STUB_WARN()                                                            \
  ({                                                                           \
    __ensure_warn("STUB function was called", __FILE__, __LINE__,              \
                  __PRETTY_FUNCTION__);                                        \
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
    return ret;
  }

  return 0;
}
// File IO

int Sysdeps<Close>::operator()(int fd) {
  long ret;
  bool err = syscall(SYSCALL_CLOSE, &ret, fd);
  if (err) {
    return ret;
  }
  return 0;
}

int Sysdeps<Open>::operator()(const char *pathname, int flags, mode_t mode,
                              int *fd) {
  long ret;
  bool err = syscall(SYSCALL_OPEN, &ret, (uintptr_t)pathname, strlen(pathname),
                     flags, mode);
  if (err) {
    return ret;
  }
  *fd = ret;
  return 0;
};

int Sysdeps<Read>::operator()(int fd, void *buff, size_t count,
                              ssize_t *bytes_read) {
  long ret;
  bool err = syscall(SYSCALL_READ, &ret, fd, (uintptr_t)buff, count);
  if (err) {
    return ret;
  }
  *bytes_read = ret;
  return 0;
}

int Sysdeps<Write>::operator()(int fd, const void *buff, size_t count,
                               ssize_t *bytes_written) {
  long ret;
  bool err = syscall(SYSCALL_WRITE, &ret, fd, (uintptr_t)buff, count);
  if (err) {
    return ret;
  }
  *bytes_written = ret;
  return 0;
}

int Sysdeps<Seek>::operator()(int fd, off_t offset, int whence,
                              off_t *new_offset) {
  long ret;
  bool err = syscall(SYSCALL_SEEK, &ret, fd, offset, whence);
  if (err) {
    return ret;
  }
  *new_offset = ret;
  return 0;
}

// @todo: move this somewhere???
typedef struct {
  int64_t st_size;
  uint64_t st_nlink;
  uint32_t st_mode;
} serene_stat_t;

int Sysdeps<Stat>::operator()(fsfd_target fsfdt, int fd, const char *path,
                              int flags, struct stat *statbuf) {
  serene_stat_t serene_stat;
  long ret;
  bool err;
  switch (fsfdt) {
  case fsfd_target::path:
    err = syscall(SYSCALL_STATAT, &ret, AT_FDCWD, (uint64_t)path, strlen(path),
                  (uint64_t)&serene_stat, flags);
    break;
  case fsfd_target::fd:
    err = syscall(SYSCALL_STAT, &ret, fd, (uint64_t)&serene_stat);
    break;
  case fsfd_target::fd_path:
    err = syscall(SYSCALL_STATAT, &ret, fd, (uint64_t)path, strlen(path),
                  (uint64_t)&serene_stat, flags);
    break;
  default:
    mlibc::infoLogger() << "mlibc: stat: Unknown fsfd_target: " << (int)fsfdt
                        << frg::endlog;
    return ENOSYS;
  }

  if (err) {
    return ret;
  }
  memset(statbuf, 0, sizeof(struct stat));
  statbuf->st_size = serene_stat.st_size;
  statbuf->st_nlink = serene_stat.st_nlink;
  statbuf->st_mode = serene_stat.st_mode;

  return 0;
}
int Sysdeps<Isatty>::operator()(int fd) {
  long ret;
  bool err = syscall(SYSCALL_ISATTY, &ret, fd);
  if (err) {
    return ret;
  }
  return ret;
}

int Sysdeps<GetCwd>::operator()(char *buf, size_t size) {

  mlibc::infoLogger() << "mlibc: getcwd: CALLED" << frg::endlog;
  long ret;
  bool err = syscall(SYSCALL_GETCWD, &ret, (uint64_t)buf, size);
  if (err) {
    return ret;
  }
  return 0;
}
// Memory
int Sysdeps<VmMap>::operator()(void *hint, size_t size, int prot, int flags,
                               int fd, off_t offset, void **window) {
  long ret;
  bool err = syscall(SYSCALL_VM_MAP, &ret, (uint64_t)hint, size, prot, flags,
                     fd, offset);
  if (err) {
    return ret;
  }
  *window = (void *)ret;
  return 0;
}

int Sysdeps<VmUnmap>::operator()(void *pointer, size_t size) {
  long ret;
  bool err = syscall(SYSCALL_VM_UNMAP, &ret, (uint64_t)pointer, size);
  if (err) {
    return ret;
  }
  return 0;
}

int Sysdeps<VmProtect>::operator()(void *pointer, size_t size, int prot) {
  long ret;
  bool err = syscall(SYSCALL_VM_PROTECT, &ret, (uint64_t)pointer, size, prot);
  if (err) {
    return ret;
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

// Process info
pid_t Sysdeps<GetPid>::operator()() {
  long ret;
  bool err = syscall(SYSCALL_GET_PROC_INFO, &ret, SYSCALL_GET_PROC_INFO_PID);
  if (err) {
    return ret;
  }
  return ret;
}

gid_t Sysdeps<GetGid>::operator()() {
  long ret;
  bool err = syscall(SYSCALL_GET_PROC_INFO, &ret, SYSCALL_GET_PROC_INFO_GID);
  if (err) {
    return ret;
  }
  return ret;
}
gid_t Sysdeps<GetEgid>::operator()() {
  long ret;
  bool err = syscall(SYSCALL_GET_PROC_INFO, &ret, SYSCALL_GET_PROC_INFO_EGID);
  if (err) {
    return ret;
  }
  return ret;
}
uid_t Sysdeps<GetUid>::operator()() {
  long ret;
  bool err = syscall(SYSCALL_GET_PROC_INFO, &ret, SYSCALL_GET_PROC_INFO_UID);
  if (err) {
    return ret;
  }
  return ret;
}
uid_t Sysdeps<GetEuid>::operator()() {
  long ret;
  bool err = syscall(SYSCALL_GET_PROC_INFO, &ret, SYSCALL_GET_PROC_INFO_EUID);
  if (err) {
    return ret;
  }
  return ret;
}

pid_t Sysdeps<GetPpid>::operator()() {
  long ret;
  bool err = syscall(SYSCALL_GET_PROC_INFO, &ret, SYSCALL_GET_PROC_INFO_PPID);
  if (err) {
    return ret;
  }
  return ret;
}

int Sysdeps<GetPgid>::operator()(pid_t pid, pid_t *pgid) {
  long ret;
  bool err =
      syscall(SYSCALL_GET_PROC_INFO, &ret, SYSCALL_GET_PROC_INFO_GET_PGID, pid);
  if (err) {
    return ret;
  }
  *pgid = ret;
  return 0;
}
int Sysdeps<SetPgid>::operator()(pid_t pid, pid_t pgid) {
  long ret;
  bool err = syscall(SYSCALL_GET_PROC_INFO, &ret,
                     SYSCALL_GET_PROC_INFO_SET_PGID, pid, pgid);
  if (err) {
    return ret;
  }
  return ret;
}

// tc get attr

int Sysdeps<Ioctl>::operator()(int fd, unsigned long request, void *arg,
                               int *result) {
  long ret;
  bool err = syscall(SYSCALL_IOCTL, &ret, request, (uint64_t)arg);
  if (err) {
    return ret;
  }
  if (result)
    *result = ret;
  return 0;
}

int Sysdeps<Tcgetattr>::operator()(int fd, struct termios *attr) {
  int res;
  return sysdep<Ioctl>(fd, TCGETS, (void *)attr, &res);
}

int Sysdeps<Tcsetattr>::operator()(int fd, int act,
                                   const struct termios *attr) {
  (void)act;
  int res;
  return sysdep<Ioctl>(fd, TCSETS, (void *)attr, &res);
}

int Sysdeps<Fcntl>::operator()(int fd, int cmd, va_list args, int *result) {
  long arg = va_arg(args, uint64_t);
  long ret;
  long err = syscall(SYSCALL_FCNTL, &ret, fd, cmd, arg);
  if (err) {
    return ret;
  }
  *result = ret;
  return 0;
}

// Stub signals
int Sysdeps<Sigaction>::operator()(int signum, const struct sigaction *act,
                                   struct sigaction *oldact) {
  STUB_WARN();
  return 0;
}

static sigset_t fake_sigmask;
int Sysdeps<Sigprocmask>::operator()(int how, const sigset_t *set,
                                     sigset_t *oldset) {
  STUB_WARN();
  if (oldset) {
    *oldset = fake_sigmask;
  }
  if (set) {
    switch (how) {
    case SIG_BLOCK:
      for (size_t i = 0; i < sizeof(sigset_t); i++) {
        ((unsigned char *)&fake_sigmask)[i] |= ((const unsigned char *)set)[i];
      }

      break;
    case SIG_UNBLOCK:
      for (size_t i = 0; i < sizeof(sigset_t); i++) {
        ((unsigned char *)&fake_sigmask)[i] &= ~((const unsigned char *)set)[i];
      }
      break;
    case SIG_SETMASK:
      fake_sigmask = *set;
      break;
    default:
      return EINVAL;
    }
  }
  return 0;
}

// Stubs
int Sysdeps<FutexWait>::operator()(int *pointer, int expected,
                                   const struct timespec *time) {
  STUB();
}

int Sysdeps<FutexWake>::operator()(int *pointer, bool all) { STUB(); }

pid_t Sysdeps<GetTid>::operator()() { STUB(); }

int Sysdeps<Dup2>::operator()(int fd, int flags, int newfd) { STUB(); }

int Sysdeps<ClockGet>::operator()(int clock, time_t *secs, long *nanos) {

  STUB_WARN();
  *secs = 0;
  *nanos = 0;
  return 0;
}
} // namespace mlibc
