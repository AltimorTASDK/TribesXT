
#include "nofix/x86.h"
#include "util/hooks.h"
#include "util/memory.h"
#include "util/platform.h"
#include <cstddef>
#include <memory>
#include <Windows.h>

detail::JmpHookImpl::JmpHookImpl(std::byte *target, const void *hook) :
	target(target),
	original(std::make_unique<std::byte[]>(PAGE_SIZE))
{
	PACKED (struct JmpInstruction {
		uint8_t op = 0xE9;
		int32_t rva;

		JmpInstruction(const void *from, const void *to) :
			rva(make_rel32(from, to, sizeof(JmpInstruction)))
		{
		}
	});

	DWORD oldProtect;
	VirtualProtect(original.get(), PAGE_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);

	do {
		size += x86Copy(original.get() + size, target + size, 1);
	} while (size < sizeof(JmpInstruction));

	// jmp to original after clobbered instructions
	const auto jmpStub = JmpInstruction(original.get() + size, target + size);
	memcpy(original.get() + size, &jmpStub, sizeof(jmpStub));

	const auto jmpHook = JmpInstruction(target, hook);
	patch_code(target, &jmpHook, sizeof(jmpHook));
}