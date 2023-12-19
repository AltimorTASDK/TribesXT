#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <Windows.h>

void patch_code(void *target, const void *patch, size_t size)
{
	DWORD old_protect;
	VirtualProtect(target, size, PAGE_EXECUTE_READWRITE, &old_protect);
	memcpy(target, patch, size);
	VirtualProtect(target, size, old_protect, &old_protect);
}
