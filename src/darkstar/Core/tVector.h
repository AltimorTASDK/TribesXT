#pragma once

template<typename T>
class Vector {
	enum Flags {
		Flags_OwnArray = 0x1,
		Flags_ReadOnly = 0x2,
	};

	int element_count;
	int array_size;
	short flags;
	void *unaligned_buffer;
	T *array;

public:
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;

	using iterator = T*;
	using const_iterator = const T*;
	using difference_type = int;
	using size_type = int;

	iterator begin() { return &array[0]; }
	const_iterator begin() const { return &array[0]; }
	iterator end() { return &array[element_count]; }
	const_iterator end() const { return &array[element_count]; }
};
