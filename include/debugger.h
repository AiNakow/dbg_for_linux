#ifndef DEBUGGER_H
#define DEBUGGER_H
#include <string>
#include <iostream>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include "linenoise.h"
#include "utils.h"

class Debugger {
public:
    Debugger(std::string prog_name, pid_t pid): prog_name(prog_name), pid(pid) {}
    ~Debugger() {}

    void run();
    void handle_command(const std::string& line);
    void continue_execution();

private:
    std::string prog_name;
    pid_t pid;
};

#endif // DEBUGGER_H