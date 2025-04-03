#include "breakpoint.h"

void Breakpoint::enable() {
    auto data = ptrace(PTRACE_PEEKDATA, pid, addr, nullptr);
    saved_data = static_cast<uint8_t>(data & 0xff);
    uint64_t int3 = 0xcc;
    uint64_t replaced_data = int3 | (data & ~0xff);
    ptrace(PTRACE_POKEDATA, pid, addr, replaced_data);

    enabled = true;
}

void Breakpoint::disable() {
    auto data = ptrace(PTRACE_PEEKDATA, pid, addr, nullptr);
    uint64_t  replaced_data = saved_data | (data & ~0xff);
    ptrace(PTRACE_POKEDATA, pid, addr, replaced_data);

    enabled = false;
}