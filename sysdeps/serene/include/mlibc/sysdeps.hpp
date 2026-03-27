#pragma once

#include <mlibc/sysdep-signatures.hpp>

namespace mlibc {

struct SereneSysdepTags : Exit,
                          FutexWait,
                          FutexWake,
                          Open,
                          Read,
                          Write,
                          Seek,
                          Close,
                          ClockGet,
                          LibcLog,
                          LibcPanic,
                          AnonAllocate,
                          AnonFree,
                          VmMap,
                          VmUnmap,
                          TcbSet,
                          Isatty,
                          GetTid,
                          GetGid,
                          GetEgid,
                          GetUid,
                          GetEuid,
                          GetPid,
                          GetPpid,
                          Stat,
                          Dup2,
                          VmProtect,
                          GetCwd,
                          GetPgid,
                          SetPgid {};

template <typename Tag> using Sysdeps = SysdepOf<SereneSysdepTags, Tag>;

} // namespace mlibc
