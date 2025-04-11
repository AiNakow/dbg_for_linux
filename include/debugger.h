#ifndef DEBUGGER_H
#define DEBUGGER_H
#include "breakpoint.h"
#include "linenoise.h"
#include "dwarf/dwarf++.hh"
#include "elf/elf++.hh"
#include "register.h"
#include "utils.h"
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unordered_map>

namespace edb
{
    class Debugger
    {
    public:
        Debugger(std::string prog_name, pid_t pid) : prog_name(prog_name), pid(pid)
        {
            auto fd = open(prog_name.c_str(), O_RDONLY);
            m_elf = elf::elf(elf::create_mmap_loader(fd));
            m_dwarf = dwarf::dwarf(dwarf::elf::create_loader(m_elf));
        }
        ~Debugger() {}

        void run();
        void handle_command(const std::string &line);
        void continue_execution();
        void set_breakpoint_at_address(std::uintptr_t addr);
        void dump_registers();
        uint64_t read_memory(std::uintptr_t addr);
        void write_memory(std::uintptr_t addr, uint64_t value);
        uint64_t get_pc();
        void set_pc(uint64_t pc);
        void step_over_breakpoint();
        void wait_for_signal();
        dwarf::die get_function_from_pc(uint64_t pc);
        dwarf::line_table::iterator get_line_entry_from_pc(uint64_t pc);
        void init_load_address();
        uint64_t offset_load_address(uint64_t addr);
        void print_source(const std::string& filename, uint line, uint n_line_context);
        siginfo_t get_signal_info();
        void handle_sigtrap(siginfo_t siginfo);

    private:
        std::string prog_name;
        pid_t pid;
        std::unordered_map<std::intptr_t, Breakpoint> breakpoint_map;
        uint64_t load_address;
        dwarf::dwarf m_dwarf;
        elf::elf m_elf;
    };
}

#endif // DEBUGGER_H