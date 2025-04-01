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