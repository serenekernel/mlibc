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
                          Isatty {};

template <typename Tag> using Sysdeps = SysdepOf<SereneSysdepTags, Tag>;

} // namespace mlibc
