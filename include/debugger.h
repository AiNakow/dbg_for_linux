#ifndef DEBUGGER_H
#define DEBUGGER_H
#include <string>
#include <unordered_map>
#include <cstdint>
#include <regex>
#include <iostream>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include "linenoise.h"
#include "utils.h"
#include "breakpoint.h"

class Debugger {
public:
    Debugger(std::string prog_name, pid_t pid): prog_name(prog_name), pid(pid) {}
    ~Debugger() {}

    void run();
    void handle_command(const std::string& line);
    void continue_execution();
    void set_breakpoint_at_address(std::intptr_t addr);

private:
    std::string prog_name;
    pid_t pid;
    std::unordered_map<std::intptr_t, Breakpoint> breakpoint_map;
};

#endif // DEBUGGER_H