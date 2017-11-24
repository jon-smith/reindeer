#pragma once

#include <functional>

namespace obelisk
{
	class Command
	{
	public:
		Command() = default;
		virtual ~Command() = default;
		virtual void execute() = 0;
	};

	// Generic function command
	class GenericCommand : public Command
	{
	public:
		GenericCommand() = delete;
		GenericCommand(std::function<void(void)> func) : func(func){};
		void execute() override
		{
			func();
		}
	private:
		std::function<void(void)> func;
	};

} // End obelisk namespace
