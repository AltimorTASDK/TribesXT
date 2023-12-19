#pragma once

#include "util/meta.h"

enum ConsoleColor {
	CON_WHITE  = 0,
	CON_RED    = 1,
	CON_YELLOW = 2,
	CON_GREEN  = 3,
	CON_BLUE   = 4,
	CON_BLACK  = 5,
	CON_PINK   = 6
};

class CMDConsole;

class CMDCallback {
public:
	using Callback = const char*(*)(CMDConsole*, int id, int argc, const char *argv[]);
	virtual const char *consoleCallback(CMDConsole*, int id, int argc, const char *argv[]) = 0;
};

class CMDConsole : public CMDCallback {
public:
	void printf(const char *format, auto &&...args)
	{
		using func_t = void(__stdcall*)(CMDConsole*, const char*, ...);
		((func_t)0x4039B0)(this, format, std::forward<decltype(args)>(args)...);
	}

	void printf(ConsoleColor color, const char *format, auto &&...args)
	{
		using func_t = void(__stdcall*)(CMDConsole*, ConsoleColor, const char*, ...);
		((func_t)0x403A10)(this, color, format, std::forward<decltype(args)>(args)...);
	}
};

inline auto *&Console = *(CMDConsole**)0x6E284C;
