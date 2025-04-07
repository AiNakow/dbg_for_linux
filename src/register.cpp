#include "register.h"

uint64_t get_register_value(pid_t pid, const reg r) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);

    auto it = std::find_if(
        g_register_descriptors.begin(), 
        g_register_descriptors.end(), 
        [r](auto&& rd) {
            return rd.r == r;
        }
    );

    return *(reinterpret_cast<uint64_t*>(&regs) + (it - g_register_descriptors.begin()));
}

void set_register_value(const reg r, uint64_t value) {

}

std::string get_register_name(const reg r) {

}