#ifndef BREAKPOINT_H
#define BREAKPOINT_H
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <cstdint>

class Breakpoint {
public:
    Breakpoint(pid_t pid, std::intptr_t addr): pid(pid), addr(addr), enabled(false), saved_data() {}
    ~Breakpoint() {}

    void enable();
    void disable();

    auto is_enabled() const -> bool {return enabled;}
    auto get_address() const -> std::intptr_t {return addr;}

private:
    pid_t pid;
    std::intptr_t addr;
    bool enabled;
    uint8_t saved_data;
};

#endif //BREAKPOINT_H