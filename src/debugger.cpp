#include "debugger.h"

void Debugger::run() {
	int wait_status;
	auto options = 0;
	waitpid(pid, &wait_status, options);

	char *line = nullptr;
	while ((line = linenoise("debugger> ")) != nullptr) {
		handle_command(line);
		linenoiseHistoryAdd(line);
		linenoiseFree(line);
	}
}

void Debugger::handle_command(const std::string &line) {
	auto args = split(line, " ");
	auto command = args[0];

	if (is_prefix("continue", command)) {
		continue_execution();
	} else if (is_prefix("break", command)) {
		if (args.size() < 2) {
		std::cout << "Target address lost." << std::endl
					<< "eg: break 0x12345678" << std::endl;
		return;
		}

		std::regex pattern("^0x[0-9a-f]+$", std::regex::icase);
		if (!std::regex_match(args[1], pattern)) {
		std::cout << "Invalid address: " << args[1] << std::endl;
		return;
		}
		std::string addr(args[1], 2);
		set_breakpoint_at_address(std::stol(addr, nullptr, 16));
	} else if (is_prefix("register", command)) {
		if (is_prefix("dump", args[1])) {
		dump_registers();
		} else {
		if (args.size() < 3) {
			std::cout << "Need register name." << std::endl
					<< "eg: register read rax" << std::endl;
		}
		reg r;
		try {
			r = get_register_from_name(args[2]);
		} catch (const std::exception &e) {
			std::cerr << e.what() << '\n';
			return;
		}
		if (is_prefix("read", args[1])) {
			std::cout << std::setfill('0') << std::setw(16) << std::hex
					<< get_register_value(pid, r);
		}
		if (is_prefix("write", args[1])) {
			if (args.size() < 4) {
			std::cout << "Need register value." << std::endl
						<< "eg: register write rax 0x12345678" << std::endl;
			}
			std::regex pattern("^0x[0-9a-f]+$", std::regex::icase);
			if (!std::regex_match(args[3], pattern)) {
			std::cout << "Invalid register value: " << args[3] << std::endl;
			return;
			}
			std::string value(args[3], 2);
			set_register_value(pid, r, std::stol(value, nullptr, 16));
		}
		}
	} else {
		std::cerr << "Unknown command: " << command << std::endl;
	}
}

void Debugger::continue_execution() {
	ptrace(PTRACE_CONT, pid, nullptr, nullptr);

	int wait_status;
	auto options = 0;
	waitpid(pid, &wait_status, options);
}

void Debugger::set_breakpoint_at_address(std::uintptr_t addr) {
	std::cout << "Set breakpoint at 0x" << std::hex << addr << std::endl;
	Breakpoint breakpoint(pid, addr);
	breakpoint.enable();
	breakpoint_map.insert({addr, breakpoint});
}

void Debugger::dump_registers() {
	for (const auto &rd : g_register_descriptors) {
		std::cout << rd.name << "0x" << std::setfill('0') << std::setw(16)
				<< std::hex << get_register_value(pid, rd.r) << std::endl;
	}
}

uint64_t Debugger::read_memory(std::uintptr_t addr) {
	void *remote_addr = reinterpret_cast<void *>(addr);

	uint64_t value;
	struct iovec local[1] = {{&value, sizeof(value)}};
	struct iovec remote[1] = {{remote_addr, sizeof(value)}};

	ssize_t byte_read = process_vm_readv(pid, local, 1, remote, 1, 0);
	if (byte_read != sizeof(value)) {
		throw std::runtime_error("Failed to read memory: " + std::string(strerror(errno)));
	}

	return value;
}

void Debugger::write_memory(std::uintptr_t addr, uint64_t value) {

}