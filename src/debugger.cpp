#include "debugger.h"

void Debugger::run()
{
	int wait_status;
	auto options = 0;
	waitpid(pid, &wait_status, options);

	char *line = nullptr;
	while ((line = linenoise("debugger> ")) != nullptr)
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
				std::cout << std::setfill('0') << std::setw(16) << std::hex
						  << get_register_value(pid, r);
			}
			if (is_prefix("write", args[1]))
			{
				if (args.size() < 4)
				{
					std::cout << "Need register value." << std::endl
							  << "eg: register write rax 0x12345678"
							  << std::endl;
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
			std::cout << "Command args lost" << std::endl << "eg: memory read 0x12345678" << std::endl;
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
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
			
			std::cout << "0x" << std::setfill('0') << std::setw(16) << std::hex << value << std::endl;
		}
		else if (is_prefix("write", args[1]))
		{
			if (args.size() < 4)
			{
				std::cout << "Need value to write" << std::endl << "eg: memory write 0x12345678 0x87654321" << std::endl;
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
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
		}
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
	std::cout << "Set breakpoint at 0x" << std::hex << addr << std::endl;
	Breakpoint breakpoint(pid, addr);
	breakpoint.enable();
	breakpoint_map.insert({addr, breakpoint});
}

void Debugger::dump_registers()
{
	for (const auto &rd : g_register_descriptors)
	{
		std::cout << rd.name << "0x" << std::setfill('0') << std::setw(16)
				  << std::hex << get_register_value(pid, rd.r) << std::endl;
	}
}

uint64_t
Debugger::read_memory(std::uintptr_t addr)
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
	auto previous_pc = get_pc() - 1;
	if (breakpoint_map.count(previous_pc))
	{
		auto& bp = breakpoint_map[previous_pc];
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
}