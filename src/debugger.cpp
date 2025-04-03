#include "debugger.h"

void Debugger::run() {
    int wait_status;
    auto options = 0;
    waitpid(pid, &wait_status, options);
    
    char* line = nullptr;
    while ((line = linenoise("debugger> ")) != nullptr) {
        handle_command(line);
        linenoiseHistoryAdd(line);
        linenoiseFree(line);
    
    }
}

void Debugger::handle_command(const std::string& line) {
    auto args = split(line, " ");
    auto command = args[0];

    if (is_prefix("continue", command)) {
        continue_execution();
    }
    else if (is_prefix("break", command)) {
        if (args.size() < 2)
        {
            std::cout << "Target address lost." << std::endl << "eg: break 0x12345678" << std::endl;
            return;
        }
        
        std::regex pattern("^0x[0-9a-f]+$", std::regex::icase);
        if (!std::regex_match(args[1], pattern)) {
            std::cout << "Invalid address: " << args[1] << std::endl;
            return;
        }
        std::string addr(args[1], 2);
        set_breakpoint_at_address(std::stol(addr, nullptr, 16));
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
    }
}

void Debugger::continue_execution() {
    ptrace(PTRACE_CONT, pid, nullptr, nullptr);

    int wait_status;
    auto options = 0;
    waitpid(pid, &wait_status, options);
}

void Debugger::set_breakpoint_at_address(std::intptr_t addr) {
    std::cout << "Set breakpoint at 0x" << std::hex << addr << std::endl;
    Breakpoint breakpoint(pid, addr);
    breakpoint.enable();
    breakpoint_map.insert({addr, breakpoint});
}