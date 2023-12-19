#pragma once

#include <cstddef>
#include <type_traits>
 
template<typename T>
using array_elem_type = std::remove_reference_t<decltype(std::declval<T>()[0])>;
 
template<typename T>
constexpr auto array_elem_count = sizeof(std::declval<T>()) / sizeof(std::declval<T>()[0]);

#define FIELD(offset, type, name) \
	void set_##offset(std::add_lvalue_reference_t<std::add_const_t<type>> value) \
	{ \
		*(std::add_pointer_t<type>)((std::byte*)this + offset) = value; \
	} \
	\
	void set_##offset(std::add_rvalue_reference_t<type> value) \
	{ \
		*(std::add_pointer_t<type>)((std::byte*)this + offset) = std::move(value); \
	} \
	\
	std::add_lvalue_reference_t<type> get_##offset() const \
	{ \
		return *(std::add_pointer_t<type>)((std::byte*)this + offset); \
	} \
	__declspec(property(get=get_##offset, put=set_##offset)) type name

#define ARRAY_FIELD(offset, type, name) \
	void set_##offset(int index, std::add_const_t<std::add_lvalue_reference_t<array_elem_type<type>>> value) \
	{ \
		((std::decay_t<type>)((std::byte*)this + offset))[index] = value; \
	} \
	\
	void set_##offset(int index, std::add_rvalue_reference_t<array_elem_type<type>> value) \
	{ \
		((std::decay_t<type>)((std::byte*)this + offset))[index] = std::move(value); \
	} \
	\
	std::add_lvalue_reference_t<array_elem_type<type>> get_##offset(int index) const \
	{ \
		return ((std::decay_t<type>)((std::byte*)this + offset))[index]; \
	} \
	__declspec(property(get=get_##offset, put=set_##offset)) array_elem_type<type> name[array_elem_count<type>]

#define BIT_FIELD(offset, type, mask, name) \
	void set_##offset_##mask(bool value) \
	{ \
		if (value) \
			*(type*)((std::byte*)this + offset) |= mask; \
		else \
			*(type*)((std::byte*)this + offset) &= ~mask; \
	} \
	\
	bool get_##offset_##mask() const \
	{ \
		return (*(type*)((std::byte*)this + offset) & mask) != 0; \
	} \
	__declspec(property(get=get_##offset_##mask, put=set_##offset_##mask)) bool name