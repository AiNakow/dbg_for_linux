#include "debugger.h"

namespace edb
{
    void Debugger::run()
    {
        wait_for_signal();
        try
        {
            init_load_address();
            std::cout << "loaded address: 0x" << std::hex << load_address << std::dec << std::endl;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return;
        }

        char *line = nullptr;
        while ((line = linenoise("edb> ")) != nullptr)
        {
            handle_command(line);
            linenoiseHistoryAdd(line);
            linenoiseFree(line);
        }
    }

    void Debugger::handle_command(const std::string &line)
    {
        auto args = split(line, " ");
        auto command = args[0];

        if (is_prefix("continue", command))
        {
            continue_execution();
        }
        else if (is_prefix("break", command))
        {
            if (args.size() < 2)
            {
                std::cout << "Target address lost." << std::endl
                          << "eg: break 0x12345678" << std::endl;
                return;
            }

            std::regex pattern("^0x[0-9a-f]+$", std::regex::icase);
            if (!std::regex_match(args[1], pattern))
            {
                std::cout << "Invalid address: " << args[1] << std::endl;
                return;
            }
            std::string addr(args[1], 2);
            std::cout << "Set breakpoint at " << args[1] << std::endl;
            set_breakpoint_at_address(std::stol(addr, nullptr, 16));
        }
        else if (is_prefix("register", command))
        {
            if (is_prefix("dump", args[1]))
            {
                dump_registers();
            }
            else
            {
                if (args.size() < 3)
                {
                    std::cout << "Need register name." << std::endl
                              << "eg: register read rax" << std::endl;
                    return;
                }
                reg r;
                try
                {
                    r = get_register_from_name(args[2]);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                    return;
                }
                if (is_prefix("read", args[1]))
                {
                    std::cout << "0x" << std::setfill('0') << std::setw(16) << std::hex
                              << get_register_value(pid, r) << std::endl;
                }
                if (is_prefix("write", args[1]))
                {
                    if (args.size() < 4)
                    {
                        std::cout << "Need register value." << std::endl
                                  << "eg: register write rax 0x12345678"
                                  << std::endl;
                        return;
                    }
                    std::regex pattern("^0x[0-9a-f]+$", std::regex::icase);
                    if (!std::regex_match(args[3], pattern))
                    {
                        std::cout << "Invalid register value: " << args[3]
                                  << std::endl;
                        return;
                    }
                    std::string value(args[3], 2);
                    set_register_value(pid, r, std::stol(value, nullptr, 16));
                }
            }
        }
        else if (is_prefix("memory", command))
        {
            if (args.size() < 3)
            {
                std::cout << "Command args lost" << std::endl
                          << "eg: memory read 0x12345678" << std::endl;
                return;
            }

            std::regex pattern("^0x[0-9a-f]+$", std::regex::icase);
            if (!std::regex_match(args[2], pattern))
            {
                std::cout << "Invalid address: " << args[2] << std::endl;
                return;
            }
            std::uintptr_t addr = std::stol(std::string(args[2], 2), nullptr, 16);
            if (is_prefix("read", args[1]))
            {
                uint64_t value;
                try
                {
                    value = read_memory(addr);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }

                std::cout << "0x" << std::setfill('0') << std::setw(16) << std::hex << value << std::endl;
            }
            else if (is_prefix("write", args[1]))
            {
                if (args.size() < 4)
                {
                    std::cout << "Need value to write" << std::endl
                              << "eg: memory write 0x12345678 0x87654321" << std::endl;
                    return;
                }
                if (!std::regex_match(args[3], pattern))
                {
                    std::cout << "Invalid memory value: " << args[3] << std::endl;
                    return;
                }

                uint64_t value = std::stol(std::string(args[3], 2), nullptr, 16);
                try
                {
                    write_memory(addr, value);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
        else if (is_prefix(command, "stepi"))
        {
            single_step_in_instructions_with_bp();
            auto offset = offset_load_address(get_pc());
            auto line_entry = get_line_entry_from_pc(offset);
            print_source(line_entry->file->path, line_entry->line);
        }
        else if (is_prefix(command, "step"))
        {
            step_in();
        }
        else if (is_prefix(command, "next"))
        {
            step_over();
        }
        else if (is_prefix(command, "finish"))
        {
            step_out();
        }
        
        else
        {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    }

    void Debugger::continue_execution()
    {
        step_over_breakpoint();
        ptrace(PTRACE_CONT, pid, nullptr, nullptr);
        wait_for_signal();
    }

    void Debugger::set_breakpoint_at_address(std::uintptr_t addr)
    {
        Breakpoint breakpoint(pid, addr);
        breakpoint.enable();
        breakpoint_map.insert({addr, breakpoint});
    }

    void Debugger::dump_registers()
    {
        int count = -1;
        for (const auto &rd : g_register_descriptors)
        {
            std::cout << std::setfill(' ') << std::setw(10) << std::left << rd.name << "0x" << std::setfill('0') << std::setw(16)
                      << std::right << std::hex << get_register_value(pid, rd.r) << " | ";
            ++count;
            if (count % 4 == 3 || count == g_register_descriptors.size() - 1)
            {
                std::cout << std::endl;
            }
        }
    }

    uint64_t Debugger::read_memory(std::uintptr_t addr)
    {
        void *remote_addr = reinterpret_cast<void *>(addr);

        uint64_t value;
        struct iovec local[1] = {{&value, sizeof(value)}};
        struct iovec remote[1] = {{remote_addr, sizeof(value)}};

        ssize_t byte_read = process_vm_readv(pid, local, 1, remote, 1, 0);
        if (byte_read != sizeof(value))
        {
            throw std::runtime_error("Failed to read memory: " + std::string(strerror(errno)));
        }

        return value;
    }

    void Debugger::write_memory(std::uintptr_t addr, uint64_t value)
    {
        void *remote_addr = reinterpret_cast<void *>(addr);

        struct iovec local[1] = {{&value, sizeof(value)}};
        struct iovec remote[1] = {{remote_addr, sizeof(value)}};
        ssize_t byte_written = process_vm_writev(pid, local, 1, remote, 1, 0);
        if (byte_written != sizeof(value))
        {
            throw std::runtime_error("Failed to write memory: " + std::string(strerror(errno)));
        }
    }

    uint64_t Debugger::get_pc()
    {
        return get_register_value(pid, reg::rip);
    }

    void Debugger::set_pc(uint64_t pc)
    {
        set_register_value(pid, reg::rip, pc);
    }

    void Debugger::step_over_breakpoint()
    {
        auto it = breakpoint_map.find(get_pc());
        if (it != breakpoint_map.end())
        {
            auto &bp = it->second;
            if (bp.is_enabled())
            {
                bp.disable();
                ptrace(PTRACE_SINGLESTEP, pid, nullptr, nullptr);
                wait_for_signal();
                bp.enable();
            }
        }
    }

    void Debugger::wait_for_signal()
    {
        int wait_status;
        auto options = 0;
        waitpid(pid, &wait_status, options);

        auto siginfo = get_signal_info();
        switch (siginfo.si_signo)
        {
        case SIGTRAP:
            handle_sigtrap(siginfo);
            break;
        case SIGSEGV:
            std::cout << "Got segfault. Reason: " << siginfo.si_code << std::endl;
            break;;
        default:
            std::cout << "Got signal " << strsignal(siginfo.si_signo) << std::endl;
            break;
        }
    }

    dwarf::die Debugger::get_function_from_pc(uint64_t pc)
    {
        for (auto& cu : m_dwarf.compilation_units())
        {
            if (!dwarf::die_pc_range(cu.root()).contains(pc))
            {
                continue;
            }
            for (const auto& die : cu.root())
            {
                if (die.tag != dwarf::DW_TAG::subprogram || !die.has(dwarf::DW_AT::low_pc))
                {
                    continue;
                }
                
                if (dwarf::die_pc_range(die).contains(pc))
                {
                    return die;
                }   
            }            
        }
        
        throw std::out_of_range("Function not found");
    }

    dwarf::line_table::iterator Debugger::get_line_entry_from_pc(uint64_t pc) 
    {
        for (auto& cu : m_dwarf.compilation_units())
        {
            if (!dwarf::die_pc_range(cu.root()).contains(pc))
            {
                continue;
            }
            
            auto &lt = cu.get_line_table();
            auto it = lt.find_address(pc);
            if (it == lt.end())
            {
                throw std::out_of_range("Line entry not found");
            }
            else
            {
                return it;
            }
        }
        
        throw std::out_of_range("Line entry not found");
    }

    void Debugger::init_load_address() 
    {
        if (m_elf.get_hdr().type == elf::et::dyn)
        {
            std::string map_file_path = "/proc/" + std::to_string(pid) + "/maps";
            std::ifstream map(map_file_path);
            if (!map.is_open())
            {
                throw std::runtime_error("Failed to open map file: " + map_file_path);
            }
            std::string addr;
            std::getline(map, addr, '-');

            load_address = std::stol(addr, nullptr, 16);
        }
    }

    uint64_t Debugger::offset_load_address(uint64_t addr) 
    {
        return addr - load_address;
    }

    void Debugger::print_source(const std::string &filename, uint line, uint n_line_context) 
    {
        std::ifstream file(filename);
        auto start_line = line <= n_line_context ? 1 : line - n_line_context;
        auto end_line = line + n_line_context + (line < n_line_context ? n_line_context - line : 0) + 1;
        
        char ch;
        uint current_line = 1;
        while (current_line < start_line && file.get(ch))
        {
            if (ch == '\n')
            {
                ++current_line;
            }
            
        }
        std::cout << "Source file: " << filename << std::endl;
        std::cout << (current_line == line ? ">" : " ") << std::dec << current_line << " ";
        while (current_line < end_line && file.get(ch))
        {
            std::cout << ch;
            if (ch == '\n')
            {
                ++current_line;
                std::cout << (current_line == line ? ">" : " ") << std::dec << current_line << " ";
            }
        }
        
        std::cout << std::endl;
    }

    siginfo_t Debugger::get_signal_info() 
    {
        siginfo_t siginfo;
        ptrace(PTRACE_GETSIGINFO, pid, nullptr, &siginfo);
        return siginfo;
    }

    void Debugger::handle_sigtrap(siginfo_t siginfo) 
    {
        switch (siginfo.si_code)
        {
        case SI_KERNEL:
        case TRAP_BRKPT:
        {   
            set_pc(get_pc() - 1);
            std::cout << "Hit breakpoint at 0x" << std::setfill('0') << std::setw(16) << std::right << std::hex << get_pc() << std::endl;
            auto offset = offset_load_address(get_pc());
            auto line_entry = get_line_entry_from_pc(offset);
            print_source(line_entry->file->path, line_entry->line);
            break;
        }
        case TRAP_TRACE:
            break;
        default:
            std::cout << "Unknown SIGTRAP code: " << siginfo.si_code << std::endl;
            break;
        }
    }

    void Debugger::single_step_in_instructions()
    {
        ptrace(PTRACE_SINGLESTEP, pid, nullptr, nullptr);
        wait_for_signal();
    }

    void Debugger::single_step_in_instructions_with_bp()
    {
        if (breakpoint_map.count(get_pc()))
        {
            step_over_breakpoint();
        }
        else
        {
            single_step_in_instructions();
        }
    }

    void Debugger::step_in()
    {
        auto line = get_line_entry_from_pc(offset_load_address(get_pc()))->line;
        while (get_line_entry_from_pc(offset_load_address(get_pc()))->line == line)
        {
            std::cout << "0x" << std::setfill('0') << std::setw(16) << std::hex << get_pc() << std::endl;
            auto pc = offset_load_address(get_pc());
            single_step_in_instructions_with_bp();
        }
        
        auto line_entry = get_line_entry_from_pc(offset_load_address(get_pc()));
        print_source(line_entry->file->path, line_entry->line);
    }

    void Debugger::step_out()
    {
        uint64_t stack_frame_pointer;
        uint64_t return_addr;
        switch (test_prologue())
        {
        case prologue_state::before:
            stack_frame_pointer = get_register_value(pid, reg::rsp);
            return_addr = read_memory(stack_frame_pointer);
            break;
        case prologue_state::during:
            stack_frame_pointer = get_register_value(pid, reg::rsp);
            return_addr = read_memory(stack_frame_pointer + 0x8);
            break;
        case prologue_state::after:
            stack_frame_pointer = get_register_value(pid, reg::rbp);
            return_addr = read_memory(stack_frame_pointer + 0x8);
        default:
            break;
        }

        bool is_temp_bp = false;
        if (breakpoint_map.count(return_addr) == 0)
        {
            set_breakpoint_at_address(return_addr);
            is_temp_bp = true;
        }
        
        continue_execution();

        if (is_temp_bp)
        {
            remove_bp(return_addr);
        }
    }

    void Debugger::step_over()
    {
        auto func = get_function_from_pc(offset_load_address(get_pc()));
        auto func_start = dwarf::at_low_pc(func);
        auto func_end = dwarf::at_high_pc(func);

        std::vector<uint64_t> temp_breakpoint;
        auto line = get_line_entry_from_pc(func_start);
        auto start_line = get_line_entry_from_pc(offset_load_address(get_pc()));
        while(line->address < func_end)
        {
            auto real_address = offset_dwarf_address(line->address);
            if (line->address != start_line->address && breakpoint_map.count(real_address) == 0)
            {
                set_breakpoint_at_address(real_address);
                temp_breakpoint.push_back(real_address);
            }
            ++line;
        }

        auto stack_frame_pointer = get_register_value(pid, reg::rbp);
        auto return_address = read_memory(stack_frame_pointer + 0x8);
        if (breakpoint_map.count(return_address) == 0)
        {
            set_breakpoint_at_address(return_address);
            temp_breakpoint.push_back(return_address);
        }

        continue_execution();

        for (auto addr : temp_breakpoint)
        {
            remove_bp(addr);
        }
    }

    void Debugger::remove_bp(std::uintptr_t addr)
    {
        auto it = breakpoint_map.find(addr);
        if (it != breakpoint_map.end() && it->second.is_enabled())
        {
            it->second.disable();
        }
        
        breakpoint_map.erase(addr);
    }

    uint64_t Debugger::offset_dwarf_address(uint64_t addr)
    {
        return addr + load_address;
    }

    // 重构判断序言前中后状态的条件
    prologue_state Debugger::test_prologue()
    {
        uint64_t value;
        auto pc = get_register_value(pid, reg::rip);
        value = read_memory(pc);

        std::array<uint8_t, 8> code;
        bool flag = false;
        for (size_t i = 0; i < 5; i++)
        {
            code[i] = static_cast<uint8_t>(value & 0xff);
            if (code[i] == 0x55)
            {
                flag = true;
            }
            
            value >>= 8;
        }
        if (code[0] == 0x48 && code[1] == 0x89 && code[2] == 0xE5)
        {
            return prologue_state::during;
        }
        else if (flag)
        {
            return prologue_state::before;
        }
        
        return prologue_state::after;
    }
}