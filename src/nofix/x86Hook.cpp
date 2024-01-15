#include <Windows.h>
#include "x86Hook.h"

bool x86Hook::Disable(void)
{
	if(_enabled && _initialized)
	{
		memcpy(_pAddress, _pBackup, _size);
		_enabled = false;

		return true;
	}

	return false;
}

bool x86Hook::Enable(void)
{
	if(!_enabled && _initialized)
	{
		memcpy(_pAddress, _pJmpPatch, _size);
		_enabled = true;

		return true;
	}

	return false;
}

bool x86Hook::Initialize(bool bEnable)
{
	if(_initialized)
		return true;
	
	// default size
	if(_nInstruction < 1)
	{
		do
			++_nInstruction;
		while( (_size += x86Copy(NULL, (unsigned char*)_pAddress + _size, 1)) < 5);
	}
	else
		_size = x86Copy(NULL, (unsigned char*)_pAddress, _nInstruction);  

	if(!BuildCallStub() || !BuildJmpPatch()  || !CreateBackup())
		return false;
	
	_initialized = true;

	if(bEnable)
		_initialized = Enable();

	return _initialized;
}

void x86Hook::Destroy(void)
{
	Disable();

	delete[] _pCallStub;
	delete[] _pJmpPatch;
	delete[] _pBackup;
}

bool x86Hook::BuildCallStub(void)
{
	unsigned char *pStub;
	unsigned int stubSize = _size + 64;
	
	pStub = _pCallStub = new unsigned char[stubSize];  

	if(!_pCallStub)
		return false;

	// if this isn't a post hook, 
	if(!(_opt & X86H_BEFORE) && !(_opt & X86H_NOINSTRUCITON))
		pStub += x86Copy(pStub, _pAddress, _nInstruction);  

	// attach header
	memcpy(pStub, CallStubHeader, 3);
	pStub += 3;  

	// attach hook call
	pStub = WriteImmediate(pStub, 0xE8, 
		PtrToLong(pStub), PtrToLong(_pFunction));
	
	// attach footer
	memcpy(pStub, CallStubFooter, 2);
	pStub += 2;

	// attach instructions from hooked function
	if((_opt & X86H_BEFORE) && !(_opt & X86H_NOINSTRUCITON))
		pStub += x86Copy(pStub, _pAddress, _nInstruction);

	// attach jmp instruction for return back home
	unsigned char *retJmp;
	retJmp = _pCallStub + _size + 10;
	
	// 
	if((_opt & X86H_NOINSTRUCITON))
		retJmp -= _size;
	
	pStub = WriteImmediate(retJmp, 0xE9, PtrToLong(retJmp), 
		_return ? PtrToLong(_return) : PtrToLong(_pAddress) + _size);

	// update protections
	DWORD protect;
	if(!(VirtualProtect(_pCallStub, (pStub - _pCallStub),
			PAGE_EXECUTE_READWRITE, &protect)))
			return false;

	return true;
}

bool x86Hook::BuildJmpPatch(void)
{
	if(!_pCallStub)
		return false;

	_pJmpPatch = new unsigned char[_size];
	if(!_pJmpPatch)
		return false;

	// nop-sled, and write jmp
	memset(_pJmpPatch, 0x90, _size);
	WriteImmediate(_pJmpPatch, 0xE9, PtrToLong(_pAddress), 
		PtrToLong(_pCallStub));

	DWORD protect;
	if(!(VirtualProtect(_pAddress, 512, 
			PAGE_EXECUTE_READWRITE, &protect)))
			return false;

	return true;
}

bool x86Hook::CreateBackup(void)
{
	if(_size && !_initialized) {
		_pBackup = new unsigned char[_size];

		if(!_pBackup)
			return false;

		memcpy(_pBackup, _pAddress, _size);
		return true;
	}

	return false;
}

unsigned char *x86Hook::WriteImmediate(unsigned char *dst, unsigned char val, uint32_t from, uint32_t to)
{
	*dst++ = val;
	*reinterpret_cast<PDWORD>(dst) = to - from - 5;

	return dst + 4;
}

extern "C"
{
	void __declspec(naked) CallStubHeader(void)
	{
		__asm
		{
			pushad                  // [ebp+8] registers
			pushfd                  // [ebp+4] flags
			push esp                // [px86Reg] ( not explicit )      
		}
	}

	void __declspec(naked) CallStubFooter(void)
	{
		__asm
		{
			popfd
			popad
		}
	}

	void __declspec(naked) CallStubHeaderFPU(void)
	{
		__asm
		{
			sub esp, 0x6C           // sizeof fpu struct            
			pushfd                  // [ebp+8] flags
			pushad                  // [ebp+4] registers
			fnsave [esp+0x24]
			push esp                // [px86Reg] ( not explicit )    
		}
	}

	void __declspec(naked) CallStubFooterFPU(void)
	{
		__asm
		{
			frstor [esp+0x24]
			popad                   // 10 restore registers                  
			popfd                   // 9 restore flags
			add esp, 0x6c
		}
	}
}