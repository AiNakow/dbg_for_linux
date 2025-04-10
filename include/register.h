#ifndef REGISTER_H
#define REGISTER_H
#include <array>
#include <string>
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <sys/ptrace.h>
#include <sys/user.h>

namespace edb
{
    enum class reg
    {
        rax,
        rbx,
        rcx,
        rdx,
        rdi,
        rsi,
        rbp,
        rsp,
        r8,
        r9,
        r10,
        r11,
        r12,
        r13,
        r14,
        r15,
        rip,
        rflags,
        cs,
        orig_rax,
        fs_base,
        gs_base,
        fs,
        gs,
        ss,
        ds,
        es
    };

    constexpr std::size_t n_registers = 27;

    struct reg_descriptor
    {
        reg r;
        int dwarf_r;
        std::string name;
    };

    const std::array<reg_descriptor, n_registers> g_register_descriptors{{
        {reg::r15, 15, "r15"},
        {reg::r14, 14, "r14"},
        {reg::r13, 13, "r13"},
        {reg::r12, 12, "r12"},
        {reg::rbp, 6, "rbp"},
        {reg::rbx, 3, "rbx"},
        {reg::r11, 11, "r11"},
        {reg::r10, 10, "r10"},
        {reg::r9, 9, "r9"},
        {reg::r8, 8, "r8"},
        {reg::rax, 0, "rax"},
        {reg::rcx, 2, "rcx"},
        {reg::rdx, 1, "rdx"},
        {reg::rsi, 4, "rsi"},
        {reg::rdi, 5, "rdi"},
        {reg::orig_rax, -1, "orig_rax"},
        {reg::rip, -1, "rip"},
        {reg::cs, 51, "cs"},
        {reg::rflags, 49, "eflags"},
        {reg::rsp, 7, "rsp"},
        {reg::ss, 52, "ss"},
        {reg::fs_base, 58, "fs_base"},
        {reg::gs_base, 59, "gs_base"},
        {reg::ds, 53, "ds"},
        {reg::es, 50, "es"},
        {reg::fs, 54, "fs"},
        {reg::gs, 55, "gs"},
    }};

    uint64_t get_register_value(pid_t pid, const reg r);
    uint64_t get_register_value_from_dwarf_register(pid_t pid, int regnum);
    void set_register_value(pid_t pid, const reg r, uint64_t value);
    std::string get_register_name(const reg r);
    reg get_register_from_name(std::string &name);
}

#endif // REGISTER_H