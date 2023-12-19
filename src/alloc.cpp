#include <cstddef>

void *operator new(size_t count)
{
	using func_t = void*(*)(size_t);
	return ((func_t)0x5A8E54)(count);
}

void operator delete(void *ptr)
{
	using func_t = void(*)(void*);
	return ((func_t)0x5A94EE)(ptr);
}