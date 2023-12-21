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
	enum Type {
		Default,
		Bool,
		Int,
		Float,
		Double,
		Unit,
		Point3F,
	};

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

	void addVariable(int id, const char *name, Callback cb, const char *value = nullptr)
	{
		using func_t = void(__thiscall*)(CMDConsole*, int, const char*,
		                                 Callback, const char*);
		((func_t)0x403500)(this, id, name, cb, value);
	}

	void addVariable(int id, const char *name, CMDCallback *cb, const char *value = nullptr)
	{
		using func_t = void(__thiscall*)(CMDConsole*, int, const char*,
		                                 CMDCallback*, const char*);
		((func_t)0x403530)(this, id, name, cb, value);
	}

	void addVariable(int id, const char *name, Type t, void *dp)
	{
		using func_t = void(__thiscall*)(CMDConsole*, int, const char*, Type, void*);
		((func_t)0x403560)(this, id, name, t, dp);
	}

	void addCommand(int id, const char *name, Callback cb, int privilegeLevel = 0)
	{
		using func_t = void(__thiscall*)(CMDConsole*, int, const char*, Callback, int);
		((func_t)0x403600)(this, id, name, cb, privilegeLevel);
	}

	void addCommand(int id, const char *name, CMDCallback *cb, int privilegeLevel = 0)
	{
		using func_t = void(__thiscall*)(CMDConsole*, int, const char*, CMDCallback*, int);
		((func_t)0x403640)(this, id, name, cb, privilegeLevel);
	}

	const char *evaluate(const char *string, bool echo = true,
	                     const char *fileName = nullptr, int privilegeLevel = 0)
	{
		using func_t = to_static_function_t<decltype(&CMDConsole::evaluate)>;
		((func_t)0x403640)(this, string, echo, fileName, privilegeLevel);
	}
};

inline auto *&Console = *(CMDConsole**)0x6E284C;
