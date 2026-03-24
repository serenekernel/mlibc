#ifndef _SERENE_SYSCALL_H
#define _SERENE_SYSCALL_H

#include <stddef.h>
#include <stdint.h>

#define SYSCALL_EXIT 0
#define SYSCALL_WRITE 1
#define SYSCALL_DEBUG_LOG 2
#define SYSCALL_TCB_SET 3
#define SYS_MEM_ANON_ALLOC 4
#define SYS_MEM_ANON_FREE 5

#ifndef __MLIBC_ABI_ONLY

static long syscall(long func, long *ret, uint64_t p1 = 0, uint64_t p2 = 0,
                    uint64_t p3 = 0, uint64_t p4 = 0, uint64_t p5 = 0,
                    uint64_t p6 = 0) {
  volatile long err;

  register uint64_t r4 asm("r10") = p4;
  register uint64_t r5 asm("r8") = p5;
  register uint64_t r6 asm("r9") = p6;

  asm volatile("syscall"
               : "=a"(*ret), "=d"(err)
               : "a"(func), "D"(p1), "S"(p2), "d"(p3), "r"(r4), "r"(r5), "r"(r6)
               : "memory", "rcx", "r11", "r15");
  return err;
}

#endif /* !__MLIBC_ABI_ONLY */

#endif /* _SERENE_SYSCALL_H */
