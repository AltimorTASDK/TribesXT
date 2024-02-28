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

class CMDConsumer {
public:
	virtual void consoleCallback(CMDConsole*, const char *consoleLine) = 0;
};

class CMDDataManager {
public:
	virtual void declareDataBlock(const char *className, const char *objectName) = 0;
	virtual const char *getDataField(const char *objectName, const char *slotName, const char *array) = 0;
	virtual void setDataField(const char *objectName, const char *slotName, const char *array, int argc, const char **argv) = 0;
	virtual void beginObject(const char *className, const char *objectName) = 0;
	virtual void endObject() = 0;
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
		using func_t = void(*)(CMDConsole*, const char*, ...);
		((func_t)0x4039B0)(this, format, std::forward<decltype(args)>(args)...);
	}

	void printf(ConsoleColor color, const char *format, auto &&...args)
	{
		using func_t = void(*)(CMDConsole*, ConsoleColor, const char*, ...);
		((func_t)0x403A10)(this, color, format, std::forward<decltype(args)>(args)...);
	}

	void dbprintf(int level, const char *format, auto &&...args)
	{
		using func_t = void(*)(CMDConsole*, int, const char*, ...);
		((func_t)0x403A40)(this, level, format, std::forward<decltype(args)>(args)...);
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

	const char *getVariable(const char *name)
	{
		using func_t = to_static_function_t<decltype(&CMDConsole::getVariable)>;
		return ((func_t)0x403400)(this, name);
	}

	bool getBoolVariable(const char *name, bool def = false)
	{
		using func_t = to_static_function_t<decltype(&CMDConsole::getBoolVariable)>;
		return ((func_t)0x403460)(this, name, def);
	}

	int getIntVariable(const char *name, int def = 0)
	{
		using func_t = to_static_function_t<decltype(&CMDConsole::getIntVariable)>;
		return ((func_t)0x403490)(this, name, def);
	}

	float getFloatVariable(const char *name, float def = 0.0f)
	{
		using func_t = to_static_function_t<decltype(&CMDConsole::getFloatVariable)>;
		return ((func_t)0x4034C0)(this, name, def);
	}

	const char *evaluate(const char *string, bool echo = true,
	                     const char *fileName = nullptr, int privilegeLevel = 0)
	{
		using func_t = to_static_function_t<decltype(&CMDConsole::evaluate)>;
		return ((func_t)0x403640)(this, string, echo, fileName, privilegeLevel);
	}

	void executef(int argc, auto &&...args)
	{
		using func_t = void(*)(CMDConsole*, int, ...);
		((func_t)0x403680)(this, argc, std::forward<decltype(args)>(args)...);
	}
};

inline auto *&Console = *(CMDConsole**)0x6E284C;
