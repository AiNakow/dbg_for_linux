#include "register.h"

namespace edb
{
    uint64_t get_register_value(pid_t pid, const reg r)
    {
        user_regs_struct regs;
        ptrace(PTRACE_GETREGS, pid, nullptr, &regs);

        auto it = std::find_if(
            g_register_descriptors.begin(),
            g_register_descriptors.end(),
            [r](auto &&rd)
            {
                return rd.r == r;
            });

        return *(reinterpret_cast<uint64_t *>(&regs) + (it - g_register_descriptors.begin()));
    }

    uint64_t get_register_value_from_dwarf_register(pid_t pid, int regnum)
    {
        auto it = std::find_if(
            g_register_descriptors.begin(),
            g_register_descriptors.end(),
            [regnum](auto &&rd)
            {
                return rd.dwarf_r == regnum;
            });

        if (it == g_register_descriptors.end())
        {
            throw std::out_of_range("Unknown dwarf register");
        }

        return get_register_value(pid, it->r);
    }

    void set_register_value(pid_t pid, const reg r, uint64_t value)
    {
        user_regs_struct regs;
        ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
        auto it = std::find_if(
            g_register_descriptors.begin(),
            g_register_descriptors.end(),
            [r](auto &&rd)
            {
                return rd.r == r;
            });

        *(reinterpret_cast<uint64_t *>(&regs) + (it - g_register_descriptors.begin())) = value;
        ptrace(PTRACE_SETREGS, pid, nullptr, &regs);
    }

    std::string get_register_name(const reg r)
    {
        auto it = std::find_if(
            g_register_descriptors.begin(),
            g_register_descriptors.end(),
            [r](auto &&rd)
            {
                return rd.r == r;
            });

        return it->name;
    }

    reg get_register_from_name(std::string &name)
    {
        auto it = std::find_if(
            g_register_descriptors.begin(),
            g_register_descriptors.end(),
            [name](auto &&rd)
            {
                return rd.name == name;
            });

        if (it == g_register_descriptors.end())
        {
            throw std::out_of_range("Unknown register name");
        }

        return it->r;
    }
}