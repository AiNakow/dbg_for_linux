#ifndef DEBUGGER_H
#define DEBUGGER_H
#include "breakpoint.h"
#include "linenoise.h"
#include "register.h"
#include "utils.h"
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unordered_map>

class Debugger
{
public:
	Debugger(std::string prog_name, pid_t pid) : prog_name(prog_name), pid(pid) {}
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

private:
	std::string prog_name;
	pid_t pid;
	std::unordered_map<std::intptr_t, Breakpoint> breakpoint_map;
};

#endif // DEBUGGER_H