#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include "debugger.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <target program>" << std::endl;
        return 1;
    }
    
    auto prog = argv[1];
    std::cout << "Target program: " << prog << std::endl;
    auto pid = fork();
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
        execl(prog, prog, nullptr);
    }
    else if (pid >= 1) {
        std::cout << "Starting debugging process: " << pid << std::endl;
        Debugger debugger(prog, pid);
        debugger.run();
    }
    
}
