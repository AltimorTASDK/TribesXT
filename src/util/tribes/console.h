#pragma once

#include "darkstar/console/console.h"
#include "plugins/netset/playerXT.h"
#include "util/meta.h"
#include "util/platform.h"
#include <cstdio>
#include <tuple>
#include <type_traits>
#include <utility>

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

template<same_as_any<char*, const char*> T>
[[msvc::forceinline]]
const T parseCommandArgument(const char *arg)
{
	static_assert(!std::is_same_v<T, char*>, "argv pointers must be const");
	return arg;
}

template<std::same_as<int> T>
[[msvc::forceinline]]
const T parseCommandArgument(const char *arg)
{
	return atoi(arg);
}

template<std::same_as<float> T>
[[msvc::forceinline]]
const T parseCommandArgument(const char *arg)
{
	return atof(arg);
}

template<same_as_any<Player*, const Player*, PlayerXT*, const PlayerXT*> T>
[[msvc::forceinline]]
const T parseCommandArgument(const char *arg)
{
	return (T)findPlayerObject(arg);
}

template<same_as_any<Point3F, const Point3F&> T>
[[msvc::forceinline]]
const T parseCommandArgument(const char *arg)
{
	Point3F p;
	sscanf(arg, "%f %f %f", &p.x, &p.y, &p.z);
	return p;
}

template<same_as_any<EulerF, const EulerF&> T>
[[msvc::forceinline]]
const T parseCommandArgument(const char *arg)
{
	EulerF e;
	sscanf(arg, "%f %f %f", &e.x, &e.y, &e.z);
	return e;
}

template<typename T>
[[msvc::forceinline]]
const T parseCommandArgument(const char *arg)
{
	static_assert(always_false<T>(), "Unhandled argument type");
}

template<bool IsRemote, size_t ArgIndex>
[[msvc::forceinline]]
const char *callCommandHandler(auto &&handler, const char *argv[], auto &&...args)
{
	return handler(std::forward<decltype(args)>(args)...);
}

template<bool IsRemote, size_t ArgIndex, typename ArgsHead, typename ...ArgsTail>
[[msvc::forceinline]]
const char *callCommandHandler(auto &&handler, const char *argv[], auto &&...args)
{
	decltype(auto) arg = parseCommandArgument<ArgsHead>(argv[ArgIndex]);

	if constexpr (std::is_pointer_v<decltype(arg)>) {
		if (arg == nullptr) {
			if constexpr (IsRemote) {
				Console->printf("%s: Invalid object \"%s\"",
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
	const auto callback = []<typename ...Args>(const char*(*)(Args...)) {
		return [](CMDConsole *console, int id, int argc, const char *argv[]) {
			constexpr auto isRemote = Name.starts_with("remote");

			if (argc != sizeof...(Args) + 1) {
				if constexpr (!isRemote) {
					console->printf(
						"%s: Wrong number of arguments (should be %d)",
						argv[0], sizeof...(Args));
				}
				return "";
			}

			return detail::callCommandHandler<isRemote, 1, Args...>(Handler, argv);
		};
	}(Handler);

	console->addCommand(0, Name.value, callback);
}
