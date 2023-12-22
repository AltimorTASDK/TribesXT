#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

void patch_code(void *target, const void *patch, size_t size);

template<size_t N>
void patch_code(void *target, const char (&patch)[N])
{
	patch_code(target, patch, N - 1);
}

inline int32_t make_rel32(const void *from, const void *to, size_t instruction_size)
{
	return (int32_t)((std::byte*)to - ((std::byte*)from + instruction_size));
}

namespace detail {

template<size_t Index, typename T>
struct call_virtual_impl;

template<size_t Index, typename ReturnType, typename ThisType, typename ...ArgTypes>
struct call_virtual_impl<Index, ReturnType(ThisType::*)(ArgTypes...)> {
	static ReturnType call(ThisType *object, ArgTypes ...args)
	{
		using func_t = ReturnType(__thiscall*)(ThisType*, ArgTypes...);
		auto **vtable = *(func_t**)object;
		return vtable[Index](object, std::forward<ArgTypes>(args)...);
	}
};

template<size_t Index, typename ReturnType, typename ThisType, typename ...ArgTypes>
struct call_virtual_impl<Index, ReturnType(ThisType::*)(ArgTypes...) const> {
	static ReturnType call(const ThisType *object, ArgTypes ...args)
	{
		using func_t = ReturnType(__thiscall*)(const ThisType*, ArgTypes...);
		auto **vtable = *(func_t**)object;
		return vtable[Index](object, std::forward<ArgTypes>(args)...);
	}
};

} // namespace detail

template<size_t Index, typename T>
inline auto call_virtual(auto *object, auto &&...args)
{
	return detail::call_virtual_impl<Index, T>::call(
		object, std::forward<decltype(args)>(args)...);
}
