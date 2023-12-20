#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace detail {

class JmpHookImpl {
	size_t size = 0;
	std::byte *target;
protected:
	std::unique_ptr<std::byte[]> original;

public:
	JmpHookImpl(std::byte *target, const void *hook);

	~JmpHookImpl()
	{
		memcpy(target, original.get(), size);
	}
};

} // namespace detail

template<typename HookType, typename OriginalType, typename ...ArgTypes>
class JmpHook : public detail::JmpHookImpl {
public:
	JmpHook(void *target, HookType hook) : JmpHookImpl((std::byte*)target, (void*)hook)
	{
	}

	JmpHook(std::integral auto target, HookType hook) : JmpHook((void*)target, hook)
	{
	}

	auto callOriginal(ArgTypes ...args) const
	{
		return ((OriginalType)original.get())(std::forward<ArgTypes>(args)...);
	}
};

template<typename ReturnType, typename ...ArgTypes>
JmpHook(auto, ReturnType(__cdecl *hook)(ArgTypes...))
	-> JmpHook<decltype(hook), decltype(hook), ArgTypes...>;

template<typename ReturnType, typename ...ArgTypes>
JmpHook(auto, ReturnType(__stdcall *hook)(ArgTypes...))
	-> JmpHook<decltype(hook), decltype(hook), ArgTypes...>;

// __fastcall for __thiscall hooks
enum class edx_t : uintptr_t {};

template<typename ReturnType, typename ThisType, typename ...ArgTypes>
JmpHook(auto, ReturnType(__fastcall *hook)(ThisType*, edx_t, ArgTypes...))
	-> JmpHook<decltype(hook),
	           ReturnType(__thiscall*)(ThisType*, ArgTypes...),
	           ThisType*, ArgTypes...>;

template<typename ReturnType, typename ThisType>
JmpHook(auto, ReturnType(__fastcall *hook)(ThisType*))
	-> JmpHook<decltype(hook),
	           ReturnType(__thiscall*)(ThisType*),
	           ThisType*>;

template<auto Target, auto Hook>
	requires requires { JmpHook(Target, Hook); }
class StaticJmpHook : public decltype(JmpHook(Target, Hook)) {
	using super = decltype(JmpHook(Target, Hook));
public:
	StaticJmpHook() : super(Target, Hook) {}
};
