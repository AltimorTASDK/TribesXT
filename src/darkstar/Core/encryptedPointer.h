#pragma once

#include <cstdint>

template<typename T>
class EncryptedPointer {
	static constexpr auto Key = 0xDEADBEEF;

	uintptr_t value;

public:
	T *get() const
	{
		return (T*)(value ^ Key);
	}

	T &operator*() const
	{
		return *get();
	}

	T *operator->() const
	{
		return get();
	}
};
