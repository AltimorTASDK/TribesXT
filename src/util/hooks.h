#pragma once

#include "util/memory.h"
#include "util/meta.h"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

template<size_t Size>
class CodePatch {
	void *target;
	std::byte original[Size];

public:
	CodePatch(void *target, const void *patch) :
		target(target)
	{
		memcpy(original, target, Size);
		patch_code(target, patch, Size);
	}

	CodePatch(void *target, std::integral auto patch) :
		CodePatch(target, &patch)
	{
	}

	CodePatch(void *target, std::floating_point auto patch) :
		CodePatch(target, &patch)
	{
	}

	CodePatch(std::integral auto target, auto patch) :
		CodePatch((void*)target, patch)
	{
	}

	~CodePatch()
	{
		patch_code(target, original, Size);
	}
};

template<size_t N>
CodePatch(auto target, const char (&patch)[N]) -> CodePatch<N - 1>;
template<std::integral T>
CodePatch(auto target, T patch) -> CodePatch<sizeof(T)>;
template<std::floating_point T>
CodePatch(auto target, T patch) -> CodePatch<sizeof(T)>;

namespace detail {

// Allow StaticCodePatch to receive both primitives and string literals
template<typename T, size_t N>
struct PatchType {
	static constexpr auto size = sizeof(T) * (N - 1);
	T value[N];
	constexpr PatchType(const T (&string)[N]) { std::copy_n(string, N, value); }
};

template<typename T>
struct PatchType<T, 0> {
	static constexpr auto size = sizeof(T);
	T value[1];
	constexpr PatchType(const T &in) { value[0] = in; }
};

template<std::integral T>
PatchType(T in) -> PatchType<T, 0>;
template<std::floating_point T>
PatchType(T in) -> PatchType<T, 0>;

} // namespace detail

// Used to avoid explicit template args in class declarations
template<auto Target, detail::PatchType Patch>
class StaticCodePatch : public CodePatch<Patch.size> {
public:
	using T = CodePatch<Patch.size>;
	StaticCodePatch() : CodePatch<Patch.size>(Target, Patch.value) {}
};

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
		patch_code(target, original.get(), size);
	}
};

} // namespace detail

template<typename HookType, typename OriginalType, typename ...ArgTypes>
class JmpHook : public detail::JmpHookImpl {
public:
	JmpHook(void *target, HookType hook) :
		JmpHookImpl((std::byte*)target, (void*)hook)
	{
	}

	JmpHook(std::integral auto target, HookType hook) :
		JmpHook((void*)target, hook)
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

// Used to avoid explicit template args in class declarations
template<auto Target, auto Hook>
	requires requires { JmpHook(Target, Hook); }
class StaticJmpHook : public decltype(JmpHook(Target, Hook)) {
	using super = decltype(JmpHook(Target, Hook));
public:
	StaticJmpHook() : super(Target, Hook) {}
};
