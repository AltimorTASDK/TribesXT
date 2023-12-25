#pragma once

#include "darkstar/console/console.h"
#include "plugins/netXT/playerXT.h"
#include "util/meta.h"
#include <concepts>
#include <cstdio>
#include <type_traits>

inline __declspec(naked) Player *findPlayerObject(const char *name)
{
	_asm {
		push esi
		mov esi, [esp+8]
		mov eax, 0x489200
		call eax
		pop esi
		ret
	}
}

namespace detail {

inline char scratchBuffer[64];

template<same_as_any<char*, const char*> T>
const T scriptStringToType(const char *arg)
{
	static_assert(!std::is_same_v<T, char*>, "argv pointers must be const");
	return arg;
}

template<std::same_as<int> T>
const T scriptStringToType(const char *arg)
{
	return atoi(arg);
}

template<std::same_as<float> T>
const T scriptStringToType(const char *arg)
{
	return atof(arg);
}

template<std::same_as<bool> T>
const T scriptStringToType(const char *arg)
{
	if (stricmp(arg, "True") == 0)
		return true;
	if (stricmp(arg, "False") == 0)
		return false;
	return atof(arg) != 0;
}

template<same_as_any<Player*, const Player*, PlayerXT*, const PlayerXT*> T>
const T scriptStringToType(const char *arg)
{
	return (T)findPlayerObject(arg);
}

template<same_as_any<Point3F, const Point3F&> T>
const Point3F scriptStringToType(const char *arg)
{
	Point3F p;
	sscanf(arg, "%f %f %f", &p.x, &p.y, &p.z);
	return p;
}

template<same_as_any<EulerF, const EulerF&> T>
const EulerF scriptStringToType(const char *arg)
{
	EulerF e;
	sscanf(arg, "%f %f %f", &e.x, &e.y, &e.z);
	return e;
}

template<typename T>
const T scriptStringToType(const char *arg)
{
	static_assert(always_false<T>(), "Unhandled argument type");
}

inline const char *scriptTypeToString(const char *arg)
{
	return arg;
}

inline const char *scriptTypeToString(int arg)
{
	sprintf_s(scratchBuffer, "%d", arg);
	return scratchBuffer;
}

inline const char *scriptTypeToString(float arg)
{
	sprintf_s(scratchBuffer, "%f", arg);
	return scratchBuffer;
}

inline const char *scriptTypeToString(bool arg)
{
	return arg ? "True" : "False";
}

inline const char *scriptTypeToString(const Point3F &arg)
{
	sprintf_s(scratchBuffer, "%f %f %f", arg.x, arg.y, arg.z);
	return scratchBuffer;
}

inline const char *scriptTypeToString(const EulerF &arg)
{
	sprintf_s(scratchBuffer, "%f %f %f", arg.x, arg.y, arg.z);
	return scratchBuffer;
}

template<typename T>
const char *scriptTypeToString(T &&arg)
{
	static_assert(always_false<T>(), "Unhandled return type");
}

template<bool IsRemote, size_t ArgIndex>
const char *callCommandHandler(auto &&handler, const char *argv[], auto &&...args)
{
	if constexpr (!is_void<decltype(handler(std::forward<decltype(args)>(args)...))>) {
		return scriptTypeToString(handler(std::forward<decltype(args)>(args)...));
	} else {
		handler(std::forward<decltype(args)>(args)...);
		return "";
	}
}

template<bool IsRemote, size_t ArgIndex, typename ArgsHead, typename ...ArgsTail>
const char *callCommandHandler(auto &&handler, const char *argv[], auto &&...args)
{
	decltype(auto) arg = scriptStringToType<ArgsHead>(argv[ArgIndex]);

	if constexpr (std::is_pointer_v<decltype(arg)>) {
		if (arg == nullptr) {
			if constexpr (!IsRemote) {
				Console->printf(
					CON_YELLOW,
					"%s: Invalid object \"%s\"",
					argv[0], argv[ArgIndex]);
			}
			return "";
		}
	}

	return callCommandHandler<IsRemote, ArgIndex + 1, ArgsTail...>(
		handler, argv,
		std::forward<decltype(args)>(args)...,
		std::forward<decltype(arg)>(arg));
}

} // namespace detail

template<string_literal Name, auto Handler>
void addCommandXT(CMDConsole *console)
{
	console->addCommand(0, Name.value, []<typename R, typename ...Args>(R(*)(Args...)) {
		return [](CMDConsole *console, int id, int argc, const char *argv[]) {
			constexpr auto isRemote = Name.starts_with("remote");

			if (argc != sizeof...(Args) + 1) {
				if constexpr (!isRemote) {
					console->printf(
						CON_YELLOW,
						"%s: Wrong number of arguments (should be %d)",
						argv[0], sizeof...(Args));
				}
				return "";
			}

			return detail::callCommandHandler<isRemote, 1, Args...>(Handler, argv);
		};
	}(Handler));
}
