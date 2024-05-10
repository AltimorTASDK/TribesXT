#include "darkstar/Ts3/ts_vertex.h"
#include "plugins/starFix/starFix.h"

__declspec(naked) void StarFixPlugin::hook_StarField_render_setZ_rect()
{
	__asm
	{
		// v.fPoint.z
		fld [esp+0x6C]
		// Overwritten instruction
		mov ecx, [esp+0x28]
		mov edx, 0x4DD12C
		jmp edx
	}
}

__declspec(naked) void StarFixPlugin::hook_StarField_render_setZ_point()
{
	__asm
	{
		// v.fPoint.z
		fld [esp+0x6C]
		// Overwritten instruction
		mov ecx, [esp+0x28]
		mov edx, 0x4DD15C
		jmp edx
	}
}

void StarFixPlugin::hook_Processor_SSE_transformProject_setZ(CpuState &cs)
{
	// Fix Z being overwritten by inv W
	auto *vertex = (TS::TransformedVertex*)(cs.reg.ecx - 0x18);
	vertex->fPoint.z *= vertex->fTransformedPoint.z;
}