#include <Windows.h>
#include "x86.h"

// Lightweight x86 instruction interpreter
unsigned int x86Copy(void *dest, const void *address, int count)
{
  unsigned char *pDest = (unsigned char *)dest;
  const unsigned char *pSrc = (unsigned char *)address;

  while(count--)
  {
    const unsigned char *pStart = pSrc;

    unsigned char opcode = NULL,
                  modRM = 0xFF;

    unsigned int operandSize = sizeof(DWORD),
                 addressSize = sizeof(DWORD),
                 immediate = 0;

    // Decode Prefix
    /////////////////////////////////////////////////////////
    while(*pSrc == 0xF0 ||            // LOCK
          *pSrc == 0xF2 ||            // REPNE/REPNZ
          *pSrc == 0xF3 ||            // REP/REPZ
          *pSrc == 0x2E ||            // CS segment
          *pSrc == 0x36 ||            // SS segment
          *pSrc == 0x3E ||            // DS segment
          *pSrc == 0x26 ||            // ES segment
          *pSrc == 0x64 ||            // FS segment
          *pSrc == 0x65 ||            // GS segment
          *pSrc == 0x66 ||            // Operand-size override
          *pSrc == 0x67)              // Address-size override
    {
      if(*pSrc == 0x66)               // Operand-size override
        operandSize = sizeof(WORD);
      else if(*pSrc == 0x67)          // Address-size override
        addressSize = sizeof(WORD);

      pSrc++;
    }

    // Decode Opcode
    /////////////////////////////////////////////////////////

    // Secondary Opcode
    unsigned char secondaryOpcode = NULL;
    if(*pSrc == 0x0F ||               // Two-Byte Opcode
      (*pSrc & 0xF8) == 0xD8)         // FPU
      secondaryOpcode = *pSrc++;

    // Opcode
    opcode = *pSrc++;

    // Decode ModRM
    //////////////////////////////////////////////////////////
    if((opcode & 0xC0) != 0xC0 && secondaryOpcode)
      modRM = opcode;
    else if(!secondaryOpcode)
    {
      if((opcode & 0xC4) == 0x00 ||
          (opcode & 0xF4) == 0x60 && ((opcode & 0x0A) == 0x02 ||
          (opcode & 0x09) == 0x9) ||
          (opcode & 0xF0) == 0x80 ||
          (opcode & 0xF8) == 0xC0 && (opcode & 0x0E) != 0x02 ||
          (opcode & 0xFC) == 0xD0 ||
          (opcode & 0xF6) == 0xF6)
        modRM = *pSrc++;
    }
    else
    {
      if((opcode & 0xF0) == 0x00 && (opcode & 0x0F) >= 0x04 && (opcode & 0x0D) != 0x0D ||
        (opcode & 0xF0) == 0x30 ||
        (opcode & 0xF0) == 0x80 ||
        (opcode & 0xF0) == 0xA0 && (opcode & 0x07) <= 0x02 ||
        (opcode & 0xF8) == 0xC8 ||
        (opcode & 0xF8) == 0xE8 ||
         opcode == 0x77)
      {
        // No mod R/M byte
      }
      else
        modRM = *pSrc++;
    }

    // Decode SIB
    /////////////////////////////////////////////////////
    if((modRM & 0x07) == 0x04 &&
       (modRM & 0xC0) != 0xC0)
      pSrc++;

    // Decode Displacement
    /////////////////////////////////////////////////////
    if((modRM & 0xC5) == 0x05 || (modRM & 0xC0) == 0x80)
      pSrc += sizeof(DWORD);
    if((modRM & 0xC0) == 0x40) // byte
      pSrc++;

    // Decode Immediate
    /////////////////////////////////////////////////////
    if(!secondaryOpcode)
    {
      if((opcode & 0xC7) == 0x04 ||
         (opcode & 0xFE) == 0x6A ||           // PUSH/POP/IMUL
         (opcode & 0xF0) == 0x70 ||           // Jcc
         opcode == 0x80 ||
         opcode == 0x83 ||
         (opcode & 0xFD) == 0xA0 ||           // MOV
         opcode == 0xA8 ||                    // TEST
         (opcode & 0xF8) == 0xB0 ||           // MOV
         (opcode & 0xFE) == 0xC0 ||           // RCL
         opcode == 0xC6 ||                    // MOV
         opcode == 0xCD ||                    // INT
         (opcode & 0xFE) == 0xD4 ||           // AAD/AAM
         (opcode & 0xF8) == 0xE0 ||           // LOOP/JCXZ
         opcode == 0xEB ||
         opcode == 0xF6 && (modRM & 0x30) == 0x00)   // TEST
      {
        pSrc++;
      }
      else if((opcode & 0xF7) == 0xC2)
      {
        pSrc += sizeof(WORD);
      }
      else if((opcode & 0xFC) == 0x80 ||
              (opcode & 0xC7) == 0x05 ||      // ADD
              (opcode & 0xF8) == 0xB8 ||      // MOV
              (opcode & 0xFE) == 0xE8 ||      // CALL/JMP
              (opcode & 0xFE) == 0x68 ||
              (opcode & 0xFC) == 0xA0 ||
              (opcode & 0xEE) == 0xA8 ||
              opcode == 0xC7 ||
              opcode == 0xF7 && (modRM & 0x30) == 0x00)
      {
        if(operandSize == sizeof(DWORD))
        {
          // adjust immediate relative to 'dest'
          if((opcode & 0xFE) == 0xE8)       // CALL / JMP
          {
            if(pDest)
            {
              immediate = (DWORD)pSrc + *(PDWORD)(PBYTE)pSrc;
            }
          }
        }
        pSrc += operandSize;
      }
    }
    else
    {
      if(opcode == 0xBA ||            // BT
         opcode == 0x0F ||            // 3DNow!
         (opcode & 0xFC) == 0x70 ||   // PSLLW
         (opcode & 0xF7) == 0xA4 ||   // SHLD
         opcode == 0xC2 ||
         opcode == 0xC4 ||
         opcode == 0xC5 ||
         opcode == 0xC6)
        pSrc++;
      else if((opcode & 0xF0) == 0x80)
        pSrc += operandSize;
    }

    if(secondaryOpcode)
      pSrc++;

    if(pDest)
    {
      // copy instruction
      while(pStart != pSrc)
        *pDest++ = *pStart++;

      // update immediate value
      if(immediate)
        *(PDWORD)(pDest - sizeof(DWORD)) = immediate - (DWORD)pDest + sizeof(DWORD);
    }
  }

  return pSrc - (unsigned char *) address;
}