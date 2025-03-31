#include <string>

class Debugger {
public:
    Debugger(std::string prog_name, pid_t pid): prog_name(prog_name), pid(pid) {};
    ~Debugger();

    void run();

private:
    std::string prog_name;
    pid_t pid;
    int status;
};