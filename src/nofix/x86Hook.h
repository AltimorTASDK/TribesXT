#ifndef _X86HOOK_H_
#define _X86HOOK_H_

#include "nofix/x86.h"
#include <cstdint>

#define __x86Hook __stdcall

struct CpuState
{
	struct EFlagStruct
	{
		unsigned cf:1; //carry
		unsigned u1:1;
		unsigned pf:1; //parity
		unsigned u2:1;
		unsigned af:1; //aux
		unsigned u3:1;
		unsigned zf:1; //zero
		unsigned sf:1; //sign
		unsigned tf:1; //trap
		unsigned nf:1; //interrupt
		unsigned df:1; //directionflag
		unsigned of:1; //overflow
		unsigned emp:19;
	}eflag;
	struct RegisterStruct
	{
		uint32_t edi;
		uint32_t esi;
		uint32_t ebp;
		uint32_t esp;
		uint32_t ebx;
		uint32_t edx;
		uint32_t ecx;
		uint32_t eax;
	}reg;
	/*
	struct FpuStruct
	{
		uint16_t control;
		uint16_t unused;
		uint16_t status;
		uint16_t unused1;
		uint16_t tag;
		uint16_t unused2;
		uint32_t instruction;
		uint16_t codeseg;
		uint16_t unused3;
		uint32_t operand;
		uint16_t dataseg;
		uint16_t unused4;
		uint8_t st_0[10];
		uint8_t st_1[10];
		uint8_t st_2[10];
		uint8_t st_3[10];
		uint8_t st_4[10];
		uint8_t st_5[10];
		uint8_t st_6[10];
		uint8_t st_7[10]; 
	}fpu;*/
};

extern "C" void CallStubHeader(void);
extern "C" void CallStubFooter(void);

class x86Hook
{
public:
	enum x86HookOpt
	{
		X86H_NULL = 0,
		X86H_BEFORE = 0x1,
		X86H_PRESERVEFPU = 0x2,
		X86H_NOINSTRUCITON = 0x4
	};

	x86Hook(void *pFunction, uint32_t pAddress, int nInstructions = 0, uint32_t ret = 0, x86HookOpt opt = X86H_NULL, bool bEnable = true) :
			_pAddress((void*)pAddress),
			_pFunction(pFunction),
			_nInstruction(nInstructions),
			_return(ret),
			_opt(opt)
	{   
		_enabled = _initialized = false;
		_size = 0;

		_pBackup = _pCallStub = _pJmpPatch = NULL;

		if(bEnable)
			Initialize(true);
	}

	x86Hook(void *pFunction, void *pAddress, int nInstructions = 0, uint32_t ret = 0, x86HookOpt opt = X86H_NULL, bool bEnable = true) :
			_pAddress(pAddress),
			_pFunction(pFunction),
			_nInstruction(nInstructions),
			_return(ret),
			_opt(opt)
	{
		_enabled = _initialized = false;
		_size = 0;

		_pBackup = _pCallStub = _pJmpPatch = NULL;

		if(bEnable)
			Initialize(true);
	}

	~x86Hook()
	{
		Destroy();
	}

	bool Disable(void);
	bool Enable(void);
	bool Initialize(bool bEnable);
private:
	void Destroy(void);

	bool BuildCallStub(void);
	bool BuildJmpPatch(void);
	bool CreateBackup(void);
	unsigned char *WriteImmediate(unsigned char *dst, unsigned char val, uint32_t from, uint32_t to);

private:
	bool _enabled;
	bool _initialized;
	unsigned int _size;

	unsigned char *_pCallStub;
	unsigned char *_pJmpPatch;
	unsigned char *_pBackup;
	void * const _pFunction;
	void * const _pAddress;
	int _nInstruction;
	uint32_t _opt;
	uint32_t _return;
};

#endif