#ifndef _X86HOOK_H_
#define _X86HOOK_H_

#include <Windows.h>
#include "nofix/x86.h"

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
    DWORD edi;
    DWORD esi;
    DWORD ebp;
    DWORD esp;
    DWORD ebx;
    DWORD edx;
    DWORD ecx;
    DWORD eax;
  }reg;
  /*
  struct FpuStruct
  {
    WORD control;
    WORD unused;
    WORD status;
    WORD unused1;
    WORD tag;
    WORD unused2;
    DWORD instruction;
    WORD codeseg;
    WORD unused3;
    DWORD operand;
    WORD dataseg;
    WORD unused4;
    BYTE st_0[10];
    BYTE st_1[10];
    BYTE st_2[10];
    BYTE st_3[10];
    BYTE st_4[10];
    BYTE st_5[10];
    BYTE st_6[10];
    BYTE st_7[10]; 
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

  x86Hook(void *pFunction, DWORD pAddress, int nInstructions = 0, DWORD ret = 0, x86HookOpt opt = X86H_NULL, bool bEnable = true) :
      _pAddress(LongToPtr(pAddress)),
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

  x86Hook(void *pFunction, void *pAddress, int nInstructions = 0, DWORD ret = 0, x86HookOpt opt = X86H_NULL, bool bEnable = true) :
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
  unsigned char *WriteImmediate(unsigned char *dst, unsigned char val, DWORD from, DWORD to);

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
  DWORD _opt;
  DWORD _return;
};

#endif