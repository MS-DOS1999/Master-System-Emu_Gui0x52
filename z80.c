#include "z80.h"

void Z80_UpdateInterrupts()
{
  if(ResetInt && !ExecuteReset)
  {
    Z80_IncRegR();
    ExecuteReset = 1;
    ResetInt = 0;
    IFF1 = 0;
    Halted = 0;
    Z80_PushWord(programCounter);
    programCounter = 0x66;
  }

  if(tmsIrq)
  {
    Z80_IncRegR();
    if(IFF1 && IntMode == 1)
    {
      Halted = 0;
      Z80_PushWord(programCounter);
      programCounter = 0x38;
      IFF1 = 0;
      IFF2 = 0;
    }
  }
}

int Z80_ExecuteInstruction()
{
  int OpcodeClicks = 0;
  byte opcode = EMU_ReadMem(programCounter);
  programCounter++;
  OpcodeClicks = Z80_ExecuteOpcode(opcode);

  if((opcode != 0xFB) && EIPending)
  {
    EIPending = 0;
    IFF1 = 1;
    IFF2 = 1;
  }

  return OpcodeClicks;
}

void Z80_IncRegR()
{
  if((registerR & 127) == 127)
  {
    registerR = registerR & 128;
  }
	else
  {
    registerR++;
  }
}

int Z80_IsEvenParity(byte x)
{
    unsigned int count = 0, i, b = 1;

    for(i = 0; i < 8; i++){
        if( x & (b << i) ){count++;}
    }

    if( (count % 2) ){return 0;}

    return 1;
}


word Z80_FetchWord()
{
  word res = EMU_ReadMem(programCounter+1);
	res = res << 8;
	res |= EMU_ReadMem(programCounter);
	return res;
}

void Z80_PushWord(word value)
{
  byte hi = value >> 8;
  byte lo = value & 0xFF;
  stackPointer--;
  EMU_WriteMem(stackPointer, hi);
  stackPointer--;
  EMU_WriteMem(stackPointer, lo);
}

word Z80_PopWord()
{
  word value = EMU_ReadMem(stackPointer+1) << 8;
  value |= EMU_ReadMem(stackPointer);
  stackPointer += 2;
  return value;
}

byte Z80_FetchByte()
{
  return EMU_ReadMem(programCounter);
}

int Z80_BitsRlc(byte *reg, int OpcodeClicks, int isMem, word *memReg)
{

  if(isMem)
  {
    byte memValue = EMU_ReadMem(*memReg);

    memValue <<= 1;

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
      BIT_ByteSet(&memValue, 0);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
      BIT_ByteClear(&memValue, 0);
    }

    BIT_ByteClear(&registerAF.lo, N_Flag);
    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(Z80_IsEvenParity(memValue))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    if(memValue == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }

    EMU_WriteMem(*memReg, memValue);
  }
  else
  {
    *reg <<= 1;

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
      BIT_ByteSet(&*reg, 0);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
      BIT_ByteClear(&*reg, 0);
    }

    BIT_ByteClear(&registerAF.lo, N_Flag);
    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(Z80_IsEvenParity(*reg))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    if(*reg == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }
  }

  return OpcodeClicks;
}

int Z80_BitsRrc(byte *reg, int OpcodeClicks, int isMem, word *memReg)
{

  if(isMem)
  {
    byte memValue = EMU_ReadMem(*memReg);

    memValue >>= 1;

    if(BIT_ByteCheck(memValue, 0))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
      BIT_ByteSet(&memValue, 7);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
      BIT_ByteClear(&memValue, 7);
    }

    BIT_ByteClear(&registerAF.lo, N_Flag);
    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(Z80_IsEvenParity(memValue))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    if(memValue == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }

    EMU_WriteMem(*memReg, memValue);
  }
  else
  {
    *reg >>= 1;

    if(BIT_ByteCheck(*reg, 0))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
      BIT_ByteSet(&*reg, 7);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
      BIT_ByteClear(&*reg, 7);
    }

    BIT_ByteClear(&registerAF.lo, N_Flag);
    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(Z80_IsEvenParity(*reg))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    if(*reg == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }
  }

  return OpcodeClicks;
}

int Z80_BitsRl(byte *reg, int OpcodeClicks, int isMem, word *memReg)
{

  if(isMem)
  {

    byte memValue = EMU_ReadMem(*memReg);

    memValue <<= 1;

    byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    if(carry)
    {
      BIT_ByteSet(&memValue, 0);
    }
    else
    {
      BIT_ByteClear(&memValue, 0);
    }

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(memValue))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(memValue == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }

    EMU_WriteMem(*memReg, memValue);
  }
  else
  {
    *reg <<= 1;

    byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    if(carry)
    {
      BIT_ByteSet(&*reg, 0);
    }
    else
    {
      BIT_ByteClear(&*reg, 0);
    }

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(*reg))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(*reg == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }
  }

  return OpcodeClicks;
}

int Z80_BitsRr(byte *reg, int OpcodeClicks, int isMem, word *memReg)
{

  if(isMem)
  {

    byte memValue = EMU_ReadMem(*memReg);

    memValue >>= 1;

    byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

    if(BIT_ByteCheck(memValue, 0))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    if(carry)
    {
      BIT_ByteSet(&memValue, 7);
    }
    else
    {
      BIT_ByteClear(&memValue, 7);
    }

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(memValue))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(memValue == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }

    EMU_WriteMem(*memReg, memValue);
  }
  else
  {

    *reg >>= 1;

    byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

    if(BIT_ByteCheck(*reg, 0))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    if(carry)
    {
      BIT_ByteSet(&*reg, 7);
    }
    else
    {
      BIT_ByteClear(&*reg, 7);
    }

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(*reg))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(*reg == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }
  }

  return OpcodeClicks;
}

int Z80_BitsSla(byte *reg, int OpcodeClicks, int isMem, word *memReg)
{

  if(isMem)
  {

    byte memValue = EMU_ReadMem(*memReg);

    memValue <<= 1;

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    BIT_ByteClear(&memValue, 0);

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(memValue))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(memValue == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }

    EMU_WriteMem(*memReg, memValue);
  }
  else
  {

    *reg <<= 1;

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    BIT_ByteClear(&*reg, 0);

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(*reg))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(*reg == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }
  }

  return OpcodeClicks;
}

int Z80_BitsSra(byte *reg, int OpcodeClicks, int isMem, word *memReg)
{

  if(isMem)
  {

    byte memValue = EMU_ReadMem(*memReg);

    byte sevenbit = BIT_ByteCheck(memValue, 7);

    memValue >>= 1;

    if(BIT_ByteCheck(memValue, 0))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    if(sevenbit)
    {
      BIT_ByteSet(&memValue, 7);
    }
    else
    {
      BIT_ByteClear(&memValue, 7);
    } 

    BIT_ByteClear(&memValue, 0);

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(memValue))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(memValue == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }

    EMU_WriteMem(*memReg, memValue);
  }
  else
  {

    byte sevenbit = BIT_ByteCheck(*reg, 7);

    *reg >>= 1;

    if(BIT_ByteCheck(*reg, 0))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    if(sevenbit)
    {
      BIT_ByteSet(&*reg, 7);
    }
    else
    {
      BIT_ByteClear(&*reg, 7);
    }    

    BIT_ByteClear(&*reg, 0);

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(*reg))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(*reg == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }
  }

  return OpcodeClicks;
}

int Z80_BitsSll(byte *reg, int OpcodeClicks, int isMem, word *memReg)
{

  if(isMem)
  {

    byte memValue = EMU_ReadMem(*memReg);

    memValue <<= 1;

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    BIT_ByteSet(&memValue, 0);

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(memValue))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(memValue == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }

    EMU_WriteMem(*memReg, memValue);
  }
  else
  {

    *reg <<= 1;

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    BIT_ByteSet(&*reg, 0);

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(*reg))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(*reg == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }
  }

  return OpcodeClicks;
}

int Z80_BitsSrl(byte *reg, int OpcodeClicks, int isMem, word *memReg)
{

  if(isMem)
  {

    byte memValue = EMU_ReadMem(*memReg);

    memValue >>= 1;

    if(BIT_ByteCheck(memValue, 0))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    BIT_ByteClear(&memValue, 7);

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(memValue))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(memValue == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(memValue, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }

    EMU_WriteMem(*memReg, memValue);
  }
  else
  {

    *reg >>= 1;

    if(BIT_ByteCheck(*reg, 0))
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, C_Flag);
    }

    BIT_ByteClear(&*reg, 7);

    BIT_ByteClear(&registerAF.lo, N_Flag);

    if(Z80_IsEvenParity(*reg))
    {
      BIT_ByteSet(&registerAF.lo, PV_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, PV_Flag);
    }

    BIT_ByteClear(&registerAF.lo, H_Flag);

    if(*reg == 0)
    {
      BIT_ByteSet(&registerAF.lo, Z_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, Z_Flag);
    }

    if(BIT_ByteCheck(*reg, 7))
    {
      BIT_ByteSet(&registerAF.lo, S_Flag);
    }
    else
    {
      BIT_ByteClear(&registerAF.lo, S_Flag);
    }
  }

  return OpcodeClicks;
}

int Z80_BitsBit(byte *reg, int bit, int OpcodeClicks)
{

  byte isTrue = BIT_ByteCheck(*reg, bit);

  BIT_ByteClear(&registerAF.lo, N_Flag);

  BIT_ByteSet(&registerAF.lo, H_Flag);

  if(isTrue)
  {
    BIT_ByteClear(&registerAF.lo, Z_Flag);
  }
  else
  {
    BIT_ByteSet(&registerAF.lo, Z_Flag);
  }

  return OpcodeClicks;
}

int Z80_BitsReset(byte *reg, int bit, int OpcodeClicks)
{

  BIT_ByteClear(&*reg, bit);

  return OpcodeClicks;
}

int Z80_BitsSet(byte *reg, int bit, int OpcodeClicks)
{

  BIT_ByteSet(&*reg, bit);

  return OpcodeClicks;
}


int Z80_ExecuteOpcode(byte opcode)
{
  Z80_IncRegR();

  int OpcodeClicks = 0;

  switch(opcode)
  {
    case 0x00:
    {
      //nop, donc on fait rien ^^
      OpcodeClicks = 4;
      break;
    }
    case 0x01:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      registerBC.reg = ldValue;
      OpcodeClicks = 10;
      break;
    }
    case 0x02:
    {
      EMU_WriteMem(registerBC.reg, registerAF.hi);
      OpcodeClicks = 7;
      break;
    }
    case 0x03:
    {
      registerBC.reg+=1;
      OpcodeClicks = 6;
      break;
    }
    case 0x04:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerBC.hi;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerBC.hi & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerBC.hi+=1;

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x05:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerBC.hi;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerBC.hi & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerBC.hi-=1;

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x06:
    {
      byte ldValue = Z80_FetchByte();
      programCounter+=1;
      registerBC.hi = ldValue;
      OpcodeClicks = 7;
      break;
    }
    case 0x07:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);
      int copy = BIT_ByteCheck(registerAF.hi, 7);
      registerAF.hi <<= 1;

      if(copy)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerAF.hi, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerAF.hi, 0);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x08:
    {
      word exAF = registerAF.reg;
      word exAFshadow = registerAFShadow.reg;
      registerAF.reg = exAFshadow;
      registerAFShadow.reg = exAF;
      OpcodeClicks = 4;
      break;
    }
    case 0x09:
    {
      if(registerHL.reg + registerBC.reg > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerHL.reg & 0x0FFF) + (registerBC.reg & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.reg += registerBC.reg;

      OpcodeClicks = 11;
      break;
    }
    case 0x0A:
    {
      registerAF.hi = EMU_ReadMem(registerBC.reg);
      OpcodeClicks = 7;
      break;
    }
    case 0x0B:
    {
      registerBC.reg-=1;
      OpcodeClicks = 6;
      break;
    }
    case 0x0C:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerBC.lo;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerBC.lo & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerBC.lo+=1;

      if(registerBC.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x0D:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerBC.lo;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerBC.lo & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerBC.lo-=1;

      if(registerBC.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x0E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter+=1;
      registerBC.lo = ldValue;
      OpcodeClicks = 7;
      break;
    }
    case 0x0F:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);
      int copy = BIT_ByteCheck(registerAF.hi, 0);
      registerAF.hi >>= 1;

      if(copy)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerAF.hi, 7);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerAF.hi, 7);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x10:
    {
      signed_byte ldValue = (signed_byte)Z80_FetchByte();
      registerBC.hi -=1;
      OpcodeClicks = 8;
      if(registerBC.hi != 0)
      {
        programCounter += ldValue;
        OpcodeClicks = 13;
      }
      programCounter+=1;
      break;
    }
    case 0x11:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      registerDE.reg = ldValue;
      OpcodeClicks = 10;
      break;
    }
    case 0x12:
    {
      EMU_WriteMem(registerDE.reg, registerAF.hi);
      OpcodeClicks = 7;
      break;
    }
    case 0x13:
    {
      registerDE.reg+=1;
      OpcodeClicks = 6;
      break;
    }
    case 0x14:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerDE.hi;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerDE.hi & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerDE.hi+=1;

      if(registerDE.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerDE.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x15:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerDE.hi;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerDE.hi & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerDE.hi-=1;

      if(registerDE.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerDE.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x16:
    {
      byte ldValue = Z80_FetchByte();
      programCounter+=1;
      registerDE.hi = ldValue;
      OpcodeClicks = 7;
      break;
    }
    case 0x17:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);
      int copy = BIT_ByteCheck(registerAF.hi, 7);
      int carryCopy = BIT_ByteCheck(registerAF.lo, C_Flag);
      registerAF.hi <<= 1;

      if(copy)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      if(carryCopy)
      {
        BIT_ByteSet(&registerAF.hi, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.hi, 0);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x18:
    {
      signed_byte ldValue = (signed_byte)Z80_FetchByte();
      programCounter+=1;
      programCounter+=ldValue;
      OpcodeClicks = 12;
      break;
    }
    case 0x19:
    {
      if(registerHL.reg + registerDE.reg > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerHL.reg & 0x0FFF) + (registerDE.reg & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.reg += registerDE.reg;

      OpcodeClicks = 11;
      break;
    }
    case 0x1A:
    {
      registerAF.hi = EMU_ReadMem(registerDE.reg);
      OpcodeClicks = 7;
      break;
    }
    case 0x1B:
    {
      registerDE.reg-=1;
      OpcodeClicks = 6;
      break;
    }
    case 0x1C:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerDE.lo;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerDE.lo & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerDE.lo+=1;

      if(registerDE.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerDE.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x1D:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerDE.lo;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerDE.lo & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerDE.lo-=1;

      if(registerDE.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerDE.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x1E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter+=1;
      registerDE.lo = ldValue;
      OpcodeClicks = 7;
      break;
    }
    case 0x1F:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);
      int copy = BIT_ByteCheck(registerAF.hi, 0);
      int carryCopy = BIT_ByteCheck(registerAF.lo, C_Flag);
      registerAF.hi >>= 1;

      if(copy)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      if(carryCopy)
      {
        BIT_ByteSet(&registerAF.hi, 7);
      }
      else
      {
        BIT_ByteClear(&registerAF.hi, 7);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x20:
    {
      signed_byte ldValue = (signed_byte)Z80_FetchByte();
      programCounter+=1;
      OpcodeClicks = 7;
      int ZFlag = BIT_ByteCheck(registerAF.lo, Z_Flag);
      if(ZFlag == 0)
      {
        programCounter += ldValue;
        OpcodeClicks = 12;
      }
      break;
    }
    case 0x21:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      registerHL.reg = ldValue;
      OpcodeClicks = 10;
      break;
    }
    case 0x22:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      EMU_WriteMem(ldValue, registerHL.lo);
      EMU_WriteMem(ldValue+1, registerHL.hi);
      OpcodeClicks = 16;
      break;
    }
    case 0x23:
    {
      registerHL.reg+=1;
      OpcodeClicks = 6;
      break;
    }
    case 0x24:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerHL.hi;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerHL.hi & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.hi+=1;

      if(registerHL.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x25:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerHL.hi;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerHL.hi & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.hi-=1;

      if(registerHL.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x26:
    {
      byte ldValue = Z80_FetchByte();
      programCounter+=1;
      registerHL.hi = ldValue;
      OpcodeClicks = 7;
      break;
    }
    case 0x27:
    {
      int a = registerAF.hi;

      if(!BIT_ByteCheck(registerAF.lo, N_Flag))
      {
        if(BIT_ByteCheck(registerAF.lo, H_Flag) || (a & 0xF) > 9)
        {
          a+=0x06;
        }

        if(BIT_ByteCheck(registerAF.lo, C_Flag) || a > 0x9F)
        {
          a+=0x60;
        }
      }
      else
      {
        if(BIT_ByteCheck(registerAF.lo, H_Flag))
        {
          a = (a - 6) & 0xFF;
        }

        if(BIT_ByteCheck(registerAF.lo, C_Flag))
        {
          a-=0x60;
        }
      }

      registerAF.lo &= ~(H_Flag | Z_Flag);

      if((a & 0x100) == 0x100)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      a&=0xFF;

      if(a == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      registerAF.hi = (byte)a;

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x28:
    {
      if(BIT_ByteCheck(registerAF.lo, Z_Flag) == 1)
      {
        signed_byte ldValue = (signed_byte)Z80_FetchByte();
        programCounter+=1;
        programCounter+=ldValue;
        OpcodeClicks = 12;
      }
      else
      {
        programCounter++;
        OpcodeClicks = 7;
      }
      
      break;
    }
    case 0x29:
    {
      if(registerHL.reg + registerHL.reg > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerHL.reg & 0x0FFF) + (registerHL.reg & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.reg += registerHL.reg;

      OpcodeClicks = 11;
      break;
    }
    case 0x2A:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerHL.reg = EMU_ReadMem(ldValue+1) << 8;
      registerHL.reg |= EMU_ReadMem(ldValue);
      OpcodeClicks = 16;
      break;
    }
    case 0x2B:
    {
      registerHL.reg-=1;
      OpcodeClicks = 6;
      break;
    }
    case 0x2C:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerHL.lo;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerHL.lo & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.lo+=1;

      if(registerHL.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x2D:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerHL.lo;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerHL.lo & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.lo-=1;

      if(registerHL.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x2E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerHL.lo = ldValue;
      OpcodeClicks = 7;
      break;
    }
    case 0x2F:
    {
      registerAF.hi ^= 0xFF;
      BIT_ByteSet(&registerAF.lo, N_Flag);
      BIT_ByteSet(&registerAF.lo, H_Flag);

      OpcodeClicks = 4;
      break;
    }
    case 0x30:
    {
      if(BIT_ByteCheck(registerAF.lo, C_Flag) == 0)
      {
        signed_byte ldValue = (signed_byte)Z80_FetchByte();
        programCounter+=1;
        programCounter+=ldValue;
        OpcodeClicks = 12;
      }
      else
      {
        programCounter++;
        OpcodeClicks = 7;
      }
      
      break;
    }
    case 0x31:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      stackPointer = ldValue;
      OpcodeClicks = 10;
      break;
    }
    case 0x32:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      EMU_WriteMem(ldValue, registerAF.hi);
      OpcodeClicks = 13;
      break;
    }
    case 0x33:
    {
      stackPointer++;
      OpcodeClicks = 6;
      break;
    }
    case 0x34:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      byte memValue = EMU_ReadMem(registerHL.reg);

      signed_byte svalue = (signed_byte)memValue;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((memValue & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      memValue+=1;

      if(memValue == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(memValue, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem(registerHL.reg, memValue);

      OpcodeClicks = 11;
      break;
    }
    case 0x35:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      byte memValue = EMU_ReadMem(registerHL.reg);

      signed_byte svalue = (signed_byte)memValue;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((memValue & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      memValue-=1;

      if(memValue == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(memValue, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem(registerHL.reg, memValue);

      OpcodeClicks = 11;
      break;
    }
    case 0x36:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem(registerHL.reg, ldValue);
      OpcodeClicks = 10;
      break;
    }
    case 0x37:
    {
      BIT_ByteSet(&registerAF.lo, C_Flag);
      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      OpcodeClicks = 4;
      break;

    }
    case 0x38:
    {
      if(BIT_ByteCheck(registerAF.lo, C_Flag) == 1)
      {
        signed_byte ldValue = (signed_byte)Z80_FetchByte();
        programCounter+=1;
        programCounter+=ldValue;
        OpcodeClicks = 12;
      }
      else
      {
        programCounter++;
        OpcodeClicks = 7;
      }
      
      break;
    }
    case 0x39:
    {
      word result = registerHL.reg + stackPointer;

      if((registerHL.reg + stackPointer) > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerHL.hi & 0xF0) + ((stackPointer >> 8) & 0xF0)) & 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.reg = result;

      OpcodeClicks = 11;
      break;

    }
    case 0x3A:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      registerAF.hi = EMU_ReadMem(ldValue);
      OpcodeClicks = 13;
      break;
    }
    case 0x3B:
    {
      stackPointer--;
      OpcodeClicks = 6;
      break;
    }
    case 0x3C:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerAF.hi;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi+=1;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x3D:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerAF.hi;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi-=1;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 4;
      break;
    }
    case 0x3E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerAF.hi = ldValue;
      OpcodeClicks = 7;
      break;
    }
    case 0x3F:
    {
      if(BIT_ByteCheck(registerAF.lo, C_Flag))
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      OpcodeClicks = 4;
      break;
    }
    case 0x40:
    {
      registerBC.hi = registerBC.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x41:
    {
      registerBC.hi = registerBC.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x42:
    {
      registerBC.hi = registerDE.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x43:
    {
      registerBC.hi = registerDE.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x44:
    {
      registerBC.hi = registerHL.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x45:
    {
      registerBC.hi = registerHL.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x46:
    {
      registerBC.hi = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = 7;
      break;
    }
    case 0x47:
    {
      registerBC.hi = registerAF.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x48:
    {
      registerBC.lo = registerBC.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x49:
    {
      registerBC.lo = registerBC.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x4A:
    {
      registerBC.lo = registerDE.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x4B:
    {
      registerBC.lo = registerDE.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x4C:
    {
      registerBC.lo = registerHL.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x4D:
    {
      registerBC.lo = registerHL.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x4E:
    {
      registerBC.lo = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = 7;
      break;
    }
    case 0x4F:
    {
      registerBC.lo = registerAF.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x50:
    {
      registerDE.hi = registerBC.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x51:
    {
      registerDE.hi = registerBC.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x52:
    {
      registerDE.hi = registerDE.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x53:
    {
      registerDE.hi = registerDE.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x54:
    {
      registerDE.hi = registerHL.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x55:
    {
      registerDE.hi = registerHL.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x56:
    {
      registerDE.hi = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = 7;
      break;
    }
    case 0x57:
    {
      registerDE.hi = registerAF.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x58:
    {
      registerDE.lo = registerBC.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x59:
    {
      registerDE.lo = registerBC.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x5A:
    {
      registerDE.lo = registerDE.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x5B:
    {
      registerDE.lo = registerDE.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x5C:
    {
      registerDE.lo = registerHL.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x5D:
    {
      registerDE.lo = registerHL.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x5E:
    {
      registerDE.lo = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = 7;
      break;
    }
    case 0x5F:
    {
      registerDE.lo = registerAF.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x60:
    {
      registerHL.hi = registerBC.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x61:
    {
      registerHL.hi = registerBC.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x62:
    {
      registerHL.hi = registerDE.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x63:
    {
      registerHL.hi = registerDE.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x64:
    {
      registerHL.hi = registerHL.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x65:
    {
      registerHL.hi = registerHL.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x66:
    {
      registerHL.hi = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = 7;
      break;
    }
    case 0x67:
    {
      registerHL.hi = registerAF.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x68:
    {
      registerHL.lo = registerBC.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x69:
    {
      registerHL.lo = registerBC.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x6A:
    {
      registerHL.lo = registerDE.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x6B:
    {
      registerHL.lo = registerDE.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x6C:
    {
      registerHL.lo = registerHL.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x6D:
    {
      registerHL.lo = registerHL.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x6E:
    {
      registerHL.lo = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = 7;
      break;
    }
    case 0x6F:
    {
      registerHL.lo = registerAF.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x70:
    {
      EMU_WriteMem(registerHL.reg, registerBC.hi);
      OpcodeClicks = 7;
      break;
    }
    case 0x71:
    {
      EMU_WriteMem(registerHL.reg, registerBC.lo);
      OpcodeClicks = 7;
      break;
    }
    case 0x72:
    {
      EMU_WriteMem(registerHL.reg, registerDE.hi);
      OpcodeClicks = 7;
      break;
    }
    case 0x73:
    {
      EMU_WriteMem(registerHL.reg, registerDE.lo);
      OpcodeClicks = 7;
      break;
    }
    case 0x74:
    {
      EMU_WriteMem(registerHL.reg, registerHL.hi);
      OpcodeClicks = 7;
      break;
    }
    case 0x75:
    {
      EMU_WriteMem(registerHL.reg, registerHL.lo);
      OpcodeClicks = 7;
      break;
    }
    case 0x77:
    {
      EMU_WriteMem(registerHL.reg, registerAF.hi);
      OpcodeClicks = 7;
      break;
    }
    case 0x78:
    {
      registerAF.hi = registerBC.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x79:
    {
      registerAF.hi = registerBC.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x7A:
    {
      registerAF.hi = registerDE.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x7B:
    {
      registerAF.hi = registerDE.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x7C:
    {
      registerAF.hi = registerHL.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x7D:
    {
      registerAF.hi = registerHL.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x7E:
    {
      registerAF.hi = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = 7;
      break;
    }
    case 0x7F:
    {
      registerAF.hi = registerAF.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x80:
    {

      if(registerAF.hi + registerBC.hi > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sB = (signed_byte)registerBC.hi;
      if(sA + sB > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerBC.hi & 0xF0)) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerBC.hi;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x81:
    {

      if(registerAF.hi + registerBC.lo > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sC = (signed_byte)registerBC.lo;
      if(sA + sC > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerBC.lo & 0xF0)) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerBC.lo;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x82:
    {

      if(registerAF.hi + registerDE.hi > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sD = (signed_byte)registerDE.hi;
      if(sA + sD > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerDE.hi & 0xF0)) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerDE.hi;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x83:
    {

      if(registerAF.hi + registerDE.lo > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sE = (signed_byte)registerDE.lo;
      if(sA + sE > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerDE.lo & 0xF0)) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerDE.lo;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x84:
    {

      if(registerAF.hi + registerHL.hi > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sH = (signed_byte)registerHL.hi;
      if(sA + sH > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerHL.hi & 0xF0)) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerHL.hi;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x85:
    {

      if(registerAF.hi + registerHL.lo > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sL = (signed_byte)registerHL.lo;
      if(sA + sL > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerHL.lo & 0xF0)) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerHL.lo;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x86:
    {

      byte memValue = EMU_ReadMem(registerHL.reg);

      if(registerAF.hi + memValue > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if(sA + sM > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (memValue & 0xF0)) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += memValue;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0x87:
    {

      if(registerAF.hi + registerAF.hi > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerAF.hi;
      if(svalue + svalue > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerAF.hi & 0xF0)) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerAF.hi;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x88:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerBC.hi + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sB = (signed_byte)registerBC.hi;
      if((sA + sB + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerBC.hi & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerBC.hi;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x89:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerBC.lo + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sC = (signed_byte)registerBC.lo;
      if((sA + sC + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerBC.lo & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerBC.lo;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x8A:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerDE.hi + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sD = (signed_byte)registerDE.hi;
      if((sA + sD + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerDE.hi & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerDE.hi;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x8B:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerDE.lo + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sE = (signed_byte)registerDE.lo;
      if((sA + sE + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerDE.lo & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerDE.lo;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x8C:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerHL.hi + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sH = (signed_byte)registerHL.hi;
      if((sA + sH + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerHL.hi & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerHL.hi;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x8D:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + memValue + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if((sA + sM + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (memValue & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += memValue;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0x8E:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + memValue + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if((sA + sM + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (memValue & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += memValue;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0x8F:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerAF.hi + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      if((sA + sA + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerAF.hi & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerAF.hi;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0x90:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sB = (signed_byte)registerBC.hi;

      byte result = registerAF.hi - registerBC.hi;

      if((registerAF.hi - registerBC.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sB) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerBC.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x91:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sC = (signed_byte)registerBC.lo;

      byte result = registerAF.hi - registerBC.lo;

      if((registerAF.hi - registerBC.lo) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sC) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerBC.lo & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x92:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sD = (signed_byte)registerDE.hi;

      byte result = registerAF.hi - registerDE.hi;

      if((registerAF.hi - registerDE.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sD) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerDE.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x93:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sE = (signed_byte)registerDE.lo;

      byte result = registerAF.hi - registerDE.lo;

      if((registerAF.hi - registerDE.lo) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sE) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerDE.lo & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x94:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sH = (signed_byte)registerHL.hi;

      byte result = registerAF.hi - registerHL.hi;

      if((registerAF.hi - registerHL.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sH) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerHL.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x95:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sL = (signed_byte)registerHL.lo;

      byte result = registerAF.hi - registerHL.lo;

      if((registerAF.hi - registerHL.lo) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sL) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerHL.lo & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x96:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;

      byte result = registerAF.hi - memValue;

      if((registerAF.hi - memValue) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sM) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 7;
      break;
    }
    case 0x97:
    {
      signed_byte sA = (signed_byte)registerAF.hi;

      byte result = registerAF.hi - registerAF.hi;

      if((registerAF.hi - registerAF.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sA) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerAF.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x98:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sB = (signed_byte)registerBC.hi;

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      byte result = (registerAF.hi - registerBC.hi) - carry;

      if(((registerAF.hi - registerBC.hi) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sA - sB) - carry) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerBC.hi & 0xF0)) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x99:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sC = (signed_byte)registerBC.lo;

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      byte result = (registerAF.hi - registerBC.lo) - carry;

      if(((registerAF.hi - registerBC.lo) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sA - sC) - carry) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerBC.lo & 0xF0)) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x9A:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sD = (signed_byte)registerDE.hi;

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      byte result = (registerAF.hi - registerDE.hi) - carry;

      if(((registerAF.hi - registerDE.hi) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sA - sD) - carry) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerDE.hi & 0xF0)) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x9B:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sE = (signed_byte)registerDE.lo;

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      byte result = (registerAF.hi - registerDE.lo) - carry;

      if(((registerAF.hi - registerDE.lo) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sA - sE) - carry) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerDE.lo & 0xF0)) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x9C:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sH = (signed_byte)registerHL.hi;

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      byte result = (registerAF.hi - registerHL.hi) - carry;

      if(((registerAF.hi - registerHL.hi) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sA - sH) - carry) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerHL.hi & 0xF0)) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x9D:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sL = (signed_byte)registerHL.lo;

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      byte result = (registerAF.hi - registerHL.lo) - carry;

      if(((registerAF.hi - registerHL.lo) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sA - sL) - carry) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerHL.lo & 0xF0)) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0x9E:
    {

      byte memValue = EMU_ReadMem(registerHL.reg);
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;

      byte result = (registerAF.hi - memValue) - carry;

      if(((registerAF.hi - memValue) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sA - sM) - (signed_byte)carry) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerBC.hi & 0xF0)) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 7;
      break;
    }
    case 0x9F:
    {
      signed_byte sA = (signed_byte)registerAF.hi;

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      byte result = (registerAF.hi - registerAF.hi) - carry;

      if(((registerAF.hi - registerAF.hi) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sA - sA) - carry) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerAF.hi & 0xF0)) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 4;
      break;
    }
    case 0xA0:
    {
      registerAF.hi &= registerBC.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xA1:
    {
      registerAF.hi &= registerBC.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xA2:
    {
      registerAF.hi &= registerDE.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xA3:
    {
      registerAF.hi &= registerDE.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xA4:
    {
      registerAF.hi &= registerHL.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xA5:
    {
      registerAF.hi &= registerHL.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xA6:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);

      registerAF.hi &= memValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xA7:
    {
      registerAF.hi &= registerAF.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xA8:
    {
      registerAF.hi ^= registerBC.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xA9:
    {
      registerAF.hi ^= registerBC.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xAA:
    {
      registerAF.hi ^= registerDE.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xAB:
    {
      registerAF.hi ^= registerDE.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xAC:
    {
      registerAF.hi ^= registerHL.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xAD:
    {
      registerAF.hi ^= registerHL.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xAE:
    {
      registerAF.hi ^= EMU_ReadMem(registerHL.reg);

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xAF:
    {
      registerAF.hi ^= registerAF.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xB0:
    {
      registerAF.hi |= registerBC.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xB1:
    {
      registerAF.hi |= registerBC.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xB2:
    {
      registerAF.hi |= registerDE.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xB3:
    {
      registerAF.hi |= registerDE.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xB4:
    {
      registerAF.hi |= registerHL.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xB5:
    {
      registerAF.hi |= registerHL.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xB6:
    {
      registerAF.hi |= EMU_ReadMem(registerHL.reg);

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xB7:
    {
      registerAF.hi |= registerAF.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xB8:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sB = (signed_byte)registerBC.hi;

      byte result = registerAF.hi - registerBC.hi;

      if((registerAF.hi - registerBC.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sB) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerBC.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xB9:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sC = (signed_byte)registerBC.lo;

      byte result = registerAF.hi - registerBC.lo;

      if((registerAF.hi - registerBC.lo) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sC) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerBC.lo & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xBA:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sD = (signed_byte)registerDE.hi;

      byte result = registerAF.hi - registerDE.lo;

      if((registerAF.hi - registerDE.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sD) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerDE.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xBB:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sE = (signed_byte)registerDE.lo;

      byte result = registerAF.hi - registerDE.lo;

      if((registerAF.hi - registerDE.lo) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sE) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerDE.lo & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xBC:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sH = (signed_byte)registerHL.hi;

      byte result = registerAF.hi - registerHL.hi;

      if((registerAF.hi - registerHL.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sH) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerHL.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xBD:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sL = (signed_byte)registerHL.lo;

      byte result = registerAF.hi - registerHL.lo;

      if((registerAF.hi - registerHL.lo) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sL) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerHL.lo & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xBE:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;

      byte result = registerAF.hi - memValue;

      if((registerAF.hi - memValue) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sM) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xBF:
    {
      signed_byte sA = (signed_byte)registerAF.hi;

      byte result = registerAF.hi - registerAF.hi;

      if((registerAF.hi - registerAF.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sA) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerAF.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 4;
      break;
    }
    case 0xC0:
    {
      if(BIT_ByteCheck(registerAF.lo, Z_Flag) == 0)
      {
        programCounter = Z80_PopWord();
        OpcodeClicks = 11;
      }
      else
      {
        OpcodeClicks = 5;
      }
      break;
    }
    case 0xC1:
    {
      registerBC.reg = Z80_PopWord();
      OpcodeClicks = 10;
      break;
    }
    case 0xC2:
    {
      if(BIT_ByteCheck(registerAF.lo, Z_Flag) == 0)
      {
        word ldValue = Z80_FetchWord();
        programCounter += 2;
        programCounter = ldValue;
      }
      else
      {
        programCounter += 2;
      }
      OpcodeClicks = 11;
      break;
    }
    case 0xC3:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      programCounter = ldValue;
      OpcodeClicks = 10;
      break;
    }
    case 0xC4:
    {
      if(BIT_ByteCheck(registerAF.lo, Z_Flag) == 0)
      {
        word ldValue = Z80_FetchWord();
        programCounter+=2;
        Z80_PushWord(programCounter);
        programCounter = ldValue;
        OpcodeClicks = 17;
      }
      else
      {
        OpcodeClicks = 10;
        programCounter += 2;
      }
      break;
    }
    case 0xC5:
    {
      Z80_PushWord(registerBC.reg);
      OpcodeClicks = 11;
      break;
    }
    case 0xC6:
    {

      byte ldValue = Z80_FetchByte();
      programCounter++;

      if(registerAF.hi + ldValue > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sLd = (signed_byte)ldValue;
      if(sA + sLd > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) + (ldValue & 0xF0)) & 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += ldValue;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xC8:
    {
      if(BIT_ByteCheck(registerAF.lo, Z_Flag) == 1)
      {
        programCounter = Z80_PopWord();
        OpcodeClicks = 11;
      }
      else
      {
        OpcodeClicks = 5;
      }
      break;
    }
    case 0xC9:
    {
      programCounter = Z80_PopWord();
      OpcodeClicks = 10;
      break;
    }
    case 0xCA:
    {
      if(BIT_ByteCheck(registerAF.lo, Z_Flag) == 1)
      {
        word ldValue = Z80_FetchWord();
        programCounter += 2;
        programCounter = ldValue;
      }
      else
      {
        programCounter += 2;
      }
      OpcodeClicks = 10;
      break;
    }
    case 0xCB:
    {
      OpcodeClicks = Z80_ExecuteBITSOpcode();
      break;
    }
    case 0xCC:
    {
      if(BIT_ByteCheck(registerAF.lo, Z_Flag) == 1)
      {
        word ldValue = Z80_FetchWord();
        programCounter+=2;
        Z80_PushWord(programCounter);
        programCounter = ldValue;
        OpcodeClicks = 17;
      }
      else
      {
        OpcodeClicks = 10;
        programCounter += 2;
      }
      break;
    }
    case 0xCD:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      Z80_PushWord(programCounter);
      programCounter = ldValue;
      OpcodeClicks = 17;
      break;
    }
    case 0xCE:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + ldValue + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sLd = (signed_byte)ldValue;
      if((sA + sLd + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (ldValue & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += ldValue;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xCF:
    {
      Z80_PushWord(programCounter);
      programCounter = 0x8;
      OpcodeClicks = 11;
      break;
    }
    case 0xD0:
    {
      if(BIT_ByteCheck(registerAF.lo, C_Flag) == 0)
      {
        programCounter = Z80_PopWord();
        OpcodeClicks = 11;
      }
      else
      {
        OpcodeClicks = 5;
      }
      break;
    }
    case 0xD1:
    {
      registerDE.reg = Z80_PopWord();
      OpcodeClicks = 10;
      break;
    }
    case 0xD2:
    {
      if(BIT_ByteCheck(registerAF.lo, C_Flag) == 0)
      {
        word ldValue = Z80_FetchWord();
        programCounter += 2;
        programCounter = ldValue;
      }
      else
      {
        programCounter += 2;
      }
      OpcodeClicks = 10;
      break;
    }
    case 0xD3:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteIO(ldValue, registerAF.hi);
      OpcodeClicks = 11;
      break;
    }
    case 0xD4:
    {
      if(BIT_ByteCheck(registerAF.lo, C_Flag) == 0)
      {
        word ldValue = Z80_FetchWord();
        programCounter+=2;
        Z80_PushWord(programCounter);
        programCounter = ldValue;
        OpcodeClicks = 17;
      }
      else
      {
        OpcodeClicks = 10;
        programCounter += 2;
      }
      break;
    }
    case 0xD5:
    {
      Z80_PushWord(registerDE.reg);
      OpcodeClicks = 11;
      break;
    }
    case 0xD6:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sV = (signed_byte)ldValue;

      byte result = registerAF.hi - ldValue;

      if((registerAF.hi - ldValue) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sV) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (ldValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 7;
      break;
    }
    case 0xD7:
    {
      Z80_PushWord(programCounter);
      programCounter = 0x10;
      OpcodeClicks = 11;
      break;
    }
    case 0xD8:
    {
      if(BIT_ByteCheck(registerAF.lo, C_Flag) == 1)
      {
        programCounter = Z80_PopWord();
        OpcodeClicks = 11;
      }
      else
      {
        OpcodeClicks = 5;
      }
      break;
    }
    case 0xD9:
    {
      word bc = registerBC.reg;
      word de = registerDE.reg;
      word hl = registerHL.reg;
      word bcs = registerBCShadow.reg;
      word des = registerDEShadow.reg;
      word hls = registerHLShadow.reg;

      registerBC.reg = bcs;
      registerDE.reg = des;
      registerHL.reg = hls;

      registerBCShadow.reg = bc;
      registerDEShadow.reg = de;
      registerHLShadow.reg = hl;

      OpcodeClicks = 4;
      break;
    }
    case 0xDA:
    {
      if(BIT_ByteCheck(registerAF.lo, C_Flag) == 1)
      {
        word ldValue = Z80_FetchWord();
        programCounter += 2;
        programCounter = ldValue;
      }
      else
      {
        programCounter += 2;
      }
      OpcodeClicks = 10;
      break;
    }
    case 0xDB:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerAF.hi = EMU_ReadIO(ldValue);
      OpcodeClicks = 11;
      break;
    }
    case 0xDC:
    {
      if(BIT_ByteCheck(registerAF.lo, C_Flag) == 1)
      {
        word ldValue = Z80_FetchWord();
        programCounter+=2;
        Z80_PushWord(programCounter);
        programCounter = ldValue;
        OpcodeClicks = 17;
      }
      else
      {
        OpcodeClicks = 10;
        programCounter += 2;
      }
      break;
    }
    case 0xDD:
    {
      OpcodeClicks = Z80_ExecuteIXOpcode();
      break;
    }
    case 0xDE:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sLd = (signed_byte)ldValue;

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      byte result = (registerAF.hi - ldValue) - carry;

      if(((registerAF.hi - ldValue) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sA - sLd) - carry) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (ldValue & 0xF0)) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerAF.hi = result;

      OpcodeClicks = 7;
      break;
    }
    case 0xDF:
    {
      Z80_PushWord(programCounter);
      programCounter = 0x18;
      OpcodeClicks = 11;
      break;
    }
    case 0xE1:
    {
      registerHL.reg = Z80_PopWord();
      OpcodeClicks = 10;
      break;
    }
    case 0xE2:
    {
      if(BIT_ByteCheck(registerAF.lo, PV_Flag) == 0)
      {
        word ldValue = Z80_FetchWord();
        programCounter += 2;
        programCounter = ldValue;
      }
      else
      {
        programCounter += 2;
      }
      OpcodeClicks = 10;
      break;
    }
    case 0xE3:
    {
      byte regL = registerHL.lo;
      byte regH = registerHL.hi;

      registerHL.lo = EMU_ReadMem(stackPointer);
      registerHL.hi = EMU_ReadMem(stackPointer+1);

      EMU_WriteMem(stackPointer, regL);
      EMU_WriteMem(stackPointer+1, regH);

      OpcodeClicks = 19;
      break;
    }
    case 0xE4:
    {
      if(BIT_ByteCheck(registerAF.lo, PV_Flag) == 0)
      {
        word ldValue = Z80_FetchWord();
        programCounter+=2;
        Z80_PushWord(programCounter);
        programCounter = ldValue;
        OpcodeClicks = 17;
      }
      else
      {
        OpcodeClicks = 10;
        programCounter += 2;
      }
      break;
    }
    case 0xE5:
    {
      Z80_PushWord(registerHL.reg);
      OpcodeClicks = 11;
      break;
    }
    case 0xE6:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerAF.hi &= ldValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xE7:
    {
      Z80_PushWord(programCounter);
      programCounter = 0x20;
      OpcodeClicks = 11;
      break;
    }
    case 0xE8:
    {
      if(BIT_ByteCheck(registerAF.lo, PV_Flag) == 1)
      {
        programCounter = Z80_PopWord();
        OpcodeClicks = 11;
      }
      else
      {
        OpcodeClicks = 5;
      }
      break;
    }
    case 0xE9:
    {
      programCounter = registerHL.reg;
      OpcodeClicks = 4;
      break;
    }
    case 0xEA:
    {
      if(BIT_ByteCheck(registerAF.lo, PV_Flag) == 1)
      {
        word ldValue = Z80_FetchWord();
        programCounter += 2;
        programCounter = ldValue;
      }
      else
      {
        programCounter += 2;
      }
      OpcodeClicks = 10;
      break;
    }
    case 0xEB:
    {
      word de = registerDE.reg;
      word hl = registerHL.reg;
      registerDE.reg = hl;
      registerHL.reg = de;
      OpcodeClicks = 4;
      break;
    }
    case 0xEC:
    {
      if(BIT_ByteCheck(registerAF.lo, PV_Flag) == 1)
      {
        word ldValue = Z80_FetchWord();
        programCounter+=2;
        Z80_PushWord(programCounter);
        programCounter = ldValue;
        OpcodeClicks = 17;
      }
      else
      {
        OpcodeClicks = 10;
        programCounter += 2;
      }
      break;
    }
    case 0xED:
    {
      OpcodeClicks = Z80_ExecuteEXTDOpcode();
      break;
    }
    case 0xEE:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerAF.hi ^= ldValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xEF:
    {
      Z80_PushWord(programCounter);
      programCounter = 0x28;
      OpcodeClicks = 11;
      break;
    }
    case 0xF0:
    {
      if(BIT_ByteCheck(registerAF.lo, S_Flag) == 0)
      {
        programCounter = Z80_PopWord();
        OpcodeClicks = 11;
      }
      else
      {
        OpcodeClicks = 5;
      }
      break;
    }
    case 0xF1:
    {
      registerAF.reg = Z80_PopWord();
      OpcodeClicks = 10;
      break;
    }
    case 0xF2:
    {
      if(BIT_ByteCheck(registerAF.lo, S_Flag) == 0)
      {
        word ldValue = Z80_FetchWord();
        programCounter += 2;
        programCounter = ldValue;
      }
      else
      {
        programCounter += 2;
      }
      OpcodeClicks = 10;
      break;
    }
    case 0xF3:
    {
      IFF1 = 0;
      IFF2 = 0;
      OpcodeClicks = 4;
      break;
    }
    case 0xF5:
    {
      Z80_PushWord(registerAF.reg);
      OpcodeClicks = 11;
      break;
    }
    case 0xF6:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerAF.hi |= ldValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xF7:
    {
      Z80_PushWord(programCounter);
      programCounter = 0x30;
      OpcodeClicks = 11;
      break;
    }
    case 0xF8:
    {
      if(BIT_ByteCheck(registerAF.lo, S_Flag) == 1)
      {
        programCounter = Z80_PopWord();
        OpcodeClicks = 11;
      }
      else
      {
        OpcodeClicks = 5;
      }
      break;
    }
    case 0xF9:
    {
      stackPointer = registerHL.reg;
      OpcodeClicks = 6;
      break;
    }
    case 0xFA:
    {
      if(BIT_ByteCheck(registerAF.lo, S_Flag) == 1)
      {
        word ldValue = Z80_FetchWord();
        programCounter += 2;
        programCounter = ldValue;
      }
      else
      {
        programCounter += 2;
      }
      OpcodeClicks = 10;
      break;
    }
    case 0xFB:
    {
      EIPending = 1;
      OpcodeClicks = 4;
      break;
    }
    case 0xFC:
    {
      if(BIT_ByteCheck(registerAF.lo, S_Flag) == 1)
      {
        word ldValue = Z80_FetchWord();
        programCounter+=2;
        Z80_PushWord(programCounter);
        programCounter = ldValue;
        OpcodeClicks = 17;
      }
      else
      {
        OpcodeClicks = 10;
        programCounter += 2;
      }
      break;
    }
    case 0xFD:
    {
      OpcodeClicks = Z80_ExecuteIYOpcode();
      break;
    }
    case 0xFE:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      if(registerAF.hi - ldValue < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerAF.hi - ldValue < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (ldValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if((registerAF.hi - ldValue) == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      byte result = registerAF.hi - ldValue;

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 7;
      break;
    }
    case 0xFF:
    {
      Z80_PushWord(programCounter);
      programCounter = 0x38;
      OpcodeClicks = 11;
      break;
    }

    default:
    {
      if(UNOP)
      {
        printf("Unimplemented Opcode : 0x%X\n", opcode);
        getchar();
      }
      break;
    }

  }

  if(DEBUG)
  {
    printf("Opcode : 0x%X\n", opcode);
    printf("AF = 0x%X\n", registerAF.reg);
    printf("BC = 0x%X\n", registerBC.reg);
    printf("DE = 0x%X\n", registerDE.reg);
    printf("HL = 0x%X\n", registerHL.reg);
    printf("PC = 0x%X\n", programCounter);
    printf("SP = 0x%X\n", stackPointer);
    printf("C_Flag = %d\n", BIT_ByteCheck(registerAF.lo, C_Flag));
    printf("N_Flag = %d\n", BIT_ByteCheck(registerAF.lo, N_Flag));
    printf("PV_Flag = %d\n", BIT_ByteCheck(registerAF.lo, PV_Flag));
    printf("H_Flag = %d\n", BIT_ByteCheck(registerAF.lo, H_Flag));
    printf("Z_Flag = %d\n", BIT_ByteCheck(registerAF.lo, Z_Flag));
    printf("S_Flag = %d\n", BIT_ByteCheck(registerAF.lo, S_Flag));
    printf("VCounter = %d\n", VCounter);
    printf("OpClicks = %d\n", OpcodeClicks);
    //getchar();
  }
  
  return OpcodeClicks;
}

int Z80_ExecuteEXTDOpcode()
{
  byte opcode = EMU_ReadMem(programCounter);

  Z80_IncRegR();

  programCounter++;

  int OpcodeClicks = 0;

  switch(opcode)
  {
    case 0x40:
    {
      registerBC.hi = EMU_ReadIO(registerBC.lo);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerBC.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 12;
      break;
    }
    case 0x41:
    {
      EMU_WriteIO(registerBC.lo, registerBC.hi);
      OpcodeClicks = 12;
      break;
    }
    case 0x42:
    {
      signed_word sHL = (signed_word)registerHL.reg;
      signed_word sBC = (signed_word)registerBC.reg;

      word carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      word result = registerHL.reg - registerBC.reg;
      result -= carry;

      if(((registerHL.reg - registerBC.reg) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sHL - sBC) - carry) < -32768)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerHL.hi & 0xF0) - (registerBC.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_WordCheck(result, 15))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerHL.reg = result;

      OpcodeClicks = 15;
      break;
    }
    case 0x43:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      EMU_WriteMem(ldValue, registerBC.lo);
      EMU_WriteMem(ldValue+1, registerBC.hi);
      OpcodeClicks = 20;
      break;
    }
    case 0x44:
    {
      byte oldA = registerAF.hi;

      registerAF.lo = 0;

      registerAF.hi = 0 - registerAF.hi;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerAF.hi > 127)
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }

      signed_word htest = 0;
      htest -= (oldA & 0xF);

      if(htest < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }

      if(oldA == 128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(oldA != 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }

      OpcodeClicks = 8;
      break;

    }
    case 0x45:
    {
      programCounter = Z80_PopWord();
      IFF1 = IFF2;
      ExecuteReset = 0;
      OpcodeClicks = 14;
      break;
    }
    case 0x46:
    {
      IntMode = 0;
      OpcodeClicks = 8;
      break;
    }
    case 0x47:
    {
      registerI = registerAF.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x48:
    {
      registerBC.lo = EMU_ReadIO(registerBC.lo);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerBC.lo))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerBC.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 12;
      break;
    }
    case 0x49:
    {
      EMU_WriteIO(registerBC.lo, registerBC.lo);
      OpcodeClicks = 12;
      break;
    }
    case 0x4A:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if(registerHL.reg + registerBC.reg + carry> 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerHL.reg & 0x0FFF) + (registerBC.reg & 0x0FFF)) + carry > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.reg += registerBC.reg;
      registerHL.reg += (word)carry;

      if(registerHL.reg == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.reg, 15))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 15;
      break;
    }
    case 0x4B:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerBC.reg = EMU_ReadMem(ldValue+1) << 8;
      registerBC.reg |= EMU_ReadMem(ldValue);
      OpcodeClicks = 20;
      break;
    }
    case 0x4C:
    {
      byte oldA = registerAF.hi;

      registerAF.lo = 0;

      registerAF.hi = 0 - registerAF.hi;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerAF.hi > 127)
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }

      signed_word htest = 0;
      htest -= (oldA & 0xF);

      if(htest < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }

      if(oldA == 128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(oldA != 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }

      OpcodeClicks = 8;
      break;

    }
    case 0x4D:
    {
      IFF1 = IFF2;
      ExecuteReset = 0;
      programCounter = Z80_PopWord();
      OpcodeClicks = 14;
      break;
    }
    case 0x4F:
    {
      registerR = registerAF.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x50:
    {
      registerDE.hi = EMU_ReadIO(registerBC.lo);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerDE.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerDE.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerDE.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 12;
      break;
    }
    case 0x51:
    {
      EMU_WriteIO(registerBC.lo, registerBC.lo);
      OpcodeClicks = 12;
      break;
    }
    case 0x52:
    {
      signed_word sHL = (signed_word)registerHL.reg;
      signed_word sDE = (signed_word)registerDE.reg; 

      word carry = (word)BIT_ByteCheck(registerAF.lo, C_Flag);

      word result = (registerHL.reg - registerDE.reg) - carry;

      if(((registerHL.reg - registerDE.reg) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sHL - sDE) - carry) < -32768)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerHL.hi & 0xF0) - (registerDE.hi & 0xF0)) - ((byte)carry)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_WordCheck(result, 15))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerHL.reg = result;

      OpcodeClicks = 15;
      break;
    }
    case 0x53:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      EMU_WriteMem(ldValue, registerDE.lo);
      EMU_WriteMem(ldValue+1, registerDE.hi);
      OpcodeClicks = 20;
      break;
    }
    case 0x54:
    {
      byte oldA = registerAF.hi;

      registerAF.lo = 0;

      registerAF.hi = 0 - registerAF.hi;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerAF.hi > 127)
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }

      signed_word htest = 0;
      htest -= (oldA & 0xF);

      if(htest < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }

      if(oldA == 128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(oldA != 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }

      OpcodeClicks = 8;
      break;

    }
    case 0x55:
    {
      programCounter = Z80_PopWord();
      IFF1 = IFF2;
      ExecuteReset = 0;
      OpcodeClicks = 14;
      break;
    }
    case 0x56:
    {
      IntMode = 1;
      OpcodeClicks = 8;
      break;
    }
    case 0x57:
    {
      registerAF.hi = registerI;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);
      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(IFF2)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 9;
      break;
    }
    case 0x58:
    {
      registerDE.lo = EMU_ReadIO(registerBC.lo);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerDE.lo))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerDE.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerDE.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 12;
      break;
    }
    case 0x59:
    {
      EMU_WriteIO(registerBC.lo, registerDE.lo);
      OpcodeClicks = 12;
      break;
    }
    case 0x5A:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if(registerHL.reg + registerDE.reg + carry> 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerHL.reg & 0x0FFF) + (registerDE.reg & 0x0FFF)) + carry > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.reg += registerDE.reg;
      registerHL.reg += (word)carry;

      if(registerHL.reg == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.reg, 15))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 15;
      break;
    }
    case 0x5B:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerDE.reg = EMU_ReadMem(ldValue+1) << 8;
      registerDE.reg |= EMU_ReadMem(ldValue);
      OpcodeClicks = 20;
      break;
    }
    case 0x5C:
    {
      byte oldA = registerAF.hi;

      registerAF.lo = 0;

      registerAF.hi = 0 - registerAF.hi;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerAF.hi > 127)
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }

      signed_word htest = 0;
      htest -= (oldA & 0xF);

      if(htest < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }

      if(oldA == 128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(oldA != 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }

      OpcodeClicks = 8;
      break;

    }
    case 0x5D:
    {
      programCounter = Z80_PopWord();
      IFF1 = IFF2;
      ExecuteReset = 0;
      OpcodeClicks = 14;
      break;
    }
    case 0x5E:
    {
      IntMode = 2;
      OpcodeClicks = 8;
      break;
    }
    case 0x5F:
    {
      registerAF.hi = registerR;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);
      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(IFF2)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 9;
      break;
    }
    case 0x60:
    {
      registerHL.hi = EMU_ReadIO(registerBC.lo);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerHL.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerHL.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 12;
      break;
    }
    case 0x61:
    {
      EMU_WriteIO(registerBC.lo, registerHL.hi);
      OpcodeClicks = 12;
      break;
    }
    case 0x62:
    {
      signed_word sHL = (signed_word)registerHL.reg;

      word carry = (word)BIT_ByteCheck(registerAF.lo, C_Flag);

      word result = (registerHL.reg - registerHL.reg) - carry;

      if(((registerHL.reg - registerHL.reg) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sHL - sHL) - carry) < -32768)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerHL.hi & 0xF0) - (registerHL.hi & 0xF0)) - ((byte)carry)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_WordCheck(result, 15))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerHL.reg = result;

      OpcodeClicks = 15;
      break;
    }
    case 0x63:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      EMU_WriteMem(ldValue, registerHL.lo);
      EMU_WriteMem(ldValue+1, registerHL.hi);
      OpcodeClicks = 20;
      break;
    }
    case 0x64:
    {
      byte oldA = registerAF.hi;

      registerAF.lo = 0;

      registerAF.hi = 0 - registerAF.hi;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerAF.hi > 127)
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }

      signed_word htest = 0;
      htest -= (oldA & 0xF);

      if(htest < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }

      if(oldA == 128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(oldA != 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }

      OpcodeClicks = 8;
      break;

    }
    case 0x65:
    {
      programCounter = Z80_PopWord();
      IFF1 = IFF2;
      ExecuteReset = 0;
      OpcodeClicks = 14;
      break;
    }
    case 0x66:
    {
      IntMode = 0;
      OpcodeClicks = 8;
      break;
    }
    case 0x67:
    {
      byte hldata = EMU_ReadMem(registerHL.reg);
      byte nibbleloA = registerAF.hi & 0xF;
      byte nibbleloHL = hldata & 0xF;
      byte nibblehiHL = hldata >> 4;

      registerAF.hi &= 0xF0;
      registerAF.hi |= nibbleloHL;

      hldata = nibbleloA << 4;
      hldata |= nibblehiHL;

      EMU_WriteMem(registerHL.reg, hldata);

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);
      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      OpcodeClicks = 18;
      break;
    }
    case 0x68:
    {
      registerHL.lo = EMU_ReadIO(registerBC.lo);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerHL.lo))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerHL.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 12;
      break;
    }
    case 0x69:
    {
      EMU_WriteIO(registerBC.lo, registerHL.lo);
      OpcodeClicks = 12;
      break;
    }
    case 0x6A:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if(registerHL.reg + registerHL.reg + carry> 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerHL.reg & 0x0FFF) + (registerHL.reg & 0x0FFF)) + carry > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.reg += registerHL.reg;
      registerHL.reg += (word)carry;

      if(registerHL.reg == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.reg, 15))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 15;
      break;
    }
    case 0x6B:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerHL.reg = EMU_ReadMem(ldValue+1) << 8;
      registerHL.reg |= EMU_ReadMem(ldValue);
      OpcodeClicks = 20;
      break;
    }
    case 0x6C:
    {
      byte oldA = registerAF.hi;

      registerAF.lo = 0;

      registerAF.hi = 0 - registerAF.hi;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerAF.hi > 127)
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }

      signed_word htest = 0;
      htest -= (oldA & 0xF);

      if(htest < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }

      if(oldA == 128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(oldA != 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }

      OpcodeClicks = 8;
      break;

    }
    case 0x6D:
    {
      programCounter = Z80_PopWord();
      IFF1 = IFF2;
      ExecuteReset = 0;
      OpcodeClicks = 14;
      break;
    }
    case 0x6F:
    {
      byte hldata = EMU_ReadMem(registerHL.reg);
      byte nibbleloA = registerAF.hi & 0xF;
      byte nibbleloHL = hldata & 0xF;
      byte nibblehiHL = hldata >> 4;

      registerAF.hi &= 0xF0;
      registerAF.hi |= nibblehiHL;

      hldata = nibbleloHL << 4;
      hldata |= nibbleloA;

      EMU_WriteMem(registerHL.reg, hldata);

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);
      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      OpcodeClicks = 18;
      break;
    }
    case 0x70:
    {
      byte iomem = EMU_ReadIO(registerBC.lo);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(iomem))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(iomem == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(iomem, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 12;
      break;
    }
    case 0x71:
    {
      EMU_WriteIO(registerBC.lo, 0);
      OpcodeClicks = 12;
      break;
    }
    case 0x72:
    {
      signed_word sHL = (signed_word)registerHL.reg;
      signed_word sSP = (signed_word)stackPointer;

      word carry = (word)BIT_ByteCheck(registerAF.lo, C_Flag);

      word result = (registerHL.reg - stackPointer) - carry;

      if(((registerHL.reg - stackPointer) - carry) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if(((sHL - sSP) - carry) < -32768)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerHL.hi & 0xF0) - ((stackPointer >> 8) & 0xF0)) - ((byte)carry)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_WordCheck(result, 15))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerHL.reg = result;

      OpcodeClicks = 15;
      break;
    }
    case 0x73:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      EMU_WriteMem(ldValue, stackPointer & 0xFF);
      EMU_WriteMem(ldValue+1, (stackPointer >> 8) & 0xFF);
      OpcodeClicks = 20;
      break;
    }
    case 0x74:
    {
      byte oldA = registerAF.hi;

      registerAF.lo = 0;

      registerAF.hi = 0 - registerAF.hi;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerAF.hi > 127)
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }

      signed_word htest = 0;
      htest -= (oldA & 0xF);

      if(htest < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }

      if(oldA == 128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(oldA != 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }

      OpcodeClicks = 8;
      break;

    }
    case 0x75:
    {
      programCounter = Z80_PopWord();
      IFF1 = IFF2;
      ExecuteReset = 0;
      OpcodeClicks = 14;
      break;
    }
    case 0x76:
    {
      IntMode = 1;
      OpcodeClicks = 8;
      break;
    }
    case 0x79:
    {
      EMU_WriteIO(registerBC.lo, registerAF.hi);
      OpcodeClicks = 12;
      break;
    }
    case 0x7A:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if(registerHL.reg + stackPointer + carry> 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerHL.reg & 0x0FFF) + (stackPointer & 0x0FFF)) + carry > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerHL.reg += stackPointer;
      registerHL.reg += (word)carry;

      if(registerHL.reg == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.reg, 15))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 15;
      break;
    }
    case 0x7B:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      stackPointer = EMU_ReadMem(ldValue+1) << 8;
      stackPointer |= EMU_ReadMem(ldValue);
      OpcodeClicks = 20;
      break;
    }
    case 0x7C:
    {
      byte oldA = registerAF.hi;

      registerAF.lo = 0;

      registerAF.hi = 0 - registerAF.hi;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerAF.hi > 127)
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }

      signed_word htest = 0;
      htest -= (oldA & 0xF);

      if(htest < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }

      if(oldA == 128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(oldA != 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }

      OpcodeClicks = 8;
      break;

    }
    case 0x7D:
    {
      programCounter = Z80_PopWord();
      IFF1 = IFF2;
      ExecuteReset = 0;
      OpcodeClicks = 14;
      break;
    }
    case 0x7E:
    {
      IntMode = 2;
      OpcodeClicks = 8;
      break;
    }
    case 0xA0:
    {
      EMU_WriteMem(registerDE.reg, EMU_ReadMem(registerHL.reg));
      registerHL.reg++;
      registerDE.reg++;
      registerBC.reg--;

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(registerBC.reg == 0)
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      OpcodeClicks = 16;
    
      break;
    }
    case 0xA1:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;

      byte result = registerAF.hi - memValue;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerHL.reg++;
      registerBC.reg--;

      if(registerBC.reg == 0)
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      OpcodeClicks = 16;
      break;
    }
    case 0xA2:
    {
      EMU_WriteMem(registerHL.reg, EMU_ReadIO(registerBC.lo));
      registerHL.reg++;
      registerBC.hi--;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      OpcodeClicks = 16;
      break;

    }
    case 0xA3:
    {
      EMU_WriteIO(registerBC.lo, EMU_ReadMem(registerHL.reg));
      registerHL.reg++;
      registerBC.hi--;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      OpcodeClicks = 16;
      break;
    }
    case 0xA8:
    {
      EMU_WriteMem(registerDE.reg, EMU_ReadMem(registerHL.reg));
      registerHL.reg--;
      registerDE.reg--;
      registerBC.reg--;

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(registerBC.reg == 0)
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      OpcodeClicks = 16;
    
      break;
    }
    case 0xA9:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;

      byte result = registerAF.hi - memValue;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerHL.reg--;
      registerBC.reg--;

      if(registerBC.reg == 0)
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      OpcodeClicks = 16;
      break;
    }
    case 0xAA:
    {
      EMU_WriteMem(registerHL.reg, EMU_ReadIO(registerBC.lo));
      registerHL.reg--;
      registerBC.hi--;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      OpcodeClicks = 16;
      break;

    }
    case 0xAB:
    {
      EMU_WriteIO(registerBC.lo, EMU_ReadMem(registerHL.reg));
      registerHL.reg--;
      registerBC.hi--;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      OpcodeClicks = 16;
      break;
    }
    case 0xB0:
    {
      EMU_WriteMem(registerDE.reg, EMU_ReadMem(registerHL.reg));
      registerHL.reg++;
      registerDE.reg++;
      registerBC.reg--;

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, PV_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerBC.reg != 0)
      {
        programCounter -= 2;
        OpcodeClicks = 21;
      }
      else
      {
        OpcodeClicks = 16;
      }
      break;
    }
    case 0xB1:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;

      byte result = registerAF.hi - memValue;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerHL.reg++;
      registerBC.reg--;

      if(registerBC.reg == 0)
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(registerBC.reg != 0 && memValue != registerAF.hi)
      {
        OpcodeClicks = 21;
        programCounter-=2;
      }
      else
      {
        OpcodeClicks = 16;
      }
      break;
    }
    case 0xB2:
    {
      EMU_WriteMem(registerHL.reg, EMU_ReadIO(registerBC.lo));
      registerHL.reg++;
      registerBC.hi--;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerBC.hi != 0)
      {
        OpcodeClicks = 21;
        programCounter-=2;
      }
      else
      {
        OpcodeClicks = 16;
      }
      break;

    }
    case 0xB3:
    {
      EMU_WriteIO(registerBC.lo, EMU_ReadMem(registerHL.reg));
      registerHL.reg++;
      registerBC.hi--;

      BIT_ByteSet(&registerAF.lo, N_Flag);
      BIT_ByteSet(&registerAF.lo, Z_Flag);

      if(registerBC.hi != 0)
      {
        programCounter -= 2;
        OpcodeClicks = 21;
      }
      else
      {
        OpcodeClicks = 16;
      }
      break;
    }
    case 0xB8:
    {
      EMU_WriteMem(registerDE.reg, EMU_ReadMem(registerHL.reg));
      registerHL.reg--;
      registerDE.reg--;
      registerBC.reg--;

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, PV_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerBC.reg != 0)
      {
        programCounter -= 2;
        OpcodeClicks = 21;
      }
      else
      {
        OpcodeClicks = 16;
      }
      break;
    }
    case 0xB9:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;

      byte result = registerAF.hi - memValue;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      registerHL.reg--;
      registerBC.reg--;

      if(registerBC.reg == 0)
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }

      if(registerBC.reg != 0 && memValue != registerAF.hi)
      {
        OpcodeClicks = 21;
        programCounter-=2;
      }
      else
      {
        OpcodeClicks = 16;
      }
      break;
    }
    case 0xBA:
    {
      EMU_WriteMem(registerHL.reg, EMU_ReadIO(registerBC.lo));
      registerHL.reg--;
      registerBC.hi--;

      BIT_ByteSet(&registerAF.lo, N_Flag);

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }

      if(registerBC.hi != 0)
      {
        OpcodeClicks = 21;
        programCounter-=2;
      }
      else
      {
        OpcodeClicks = 16;
      }
      break;

    }
    case 0xBB:
    {
      EMU_WriteIO(registerBC.lo, EMU_ReadMem(registerHL.reg));
      registerHL.reg--;
      registerBC.hi--;

      BIT_ByteSet(&registerAF.lo, N_Flag);
      BIT_ByteSet(&registerAF.lo, Z_Flag);

      if(registerBC.hi != 0)
      {
        programCounter -= 2;
        OpcodeClicks = 21;
      }
      else
      {
        OpcodeClicks = 16;
      }
      break;
    }

    default:
    {
      if(UNOP)
      {
        printf("Unimplemented EXTD Opcode : 0x%X\n", opcode);
        getchar();
      }
      break;
    }
  }

  if(DEBUG)
  {
    printf("EXTD Opcode : 0x%X\n", opcode);
  }

  return OpcodeClicks;
}

int Z80_ExecuteIYOpcode()
{
  byte opcode = EMU_ReadMem(programCounter);

  Z80_IncRegR();

  programCounter++;

  int OpcodeClicks = 0;

  switch(opcode)
  {
    case 0x09:
    {
      if(registerIY.reg + registerBC.reg > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerIY.reg & 0x0FFF) + (registerBC.reg & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIY.reg += registerBC.reg;

      OpcodeClicks = 15;
      break;
    }
    case 0x19:
    {
      if(registerIY.reg + registerDE.reg > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerIY.reg & 0x0FFF) + (registerDE.reg & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIY.reg += registerDE.reg;

      OpcodeClicks = 15;
      break;
    }
    case 0x21:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerIY.reg = ldValue;
      OpcodeClicks = 14;
      break;
    }
    case 0x22:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      EMU_WriteMem(ldValue, registerIY.lo);
      EMU_WriteMem(ldValue+1, registerIY.hi);
      OpcodeClicks = 22;
      break;
    }
    case 0x23:
    {
      registerIY.reg++;
      OpcodeClicks = 10;
      break;
    }
    case 0x24:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerIY.hi;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerIY.hi & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIY.hi+=1;

      if(registerIY.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerIY.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 8;
      break;
    }
    case 0x25:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerIY.hi;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerIY.hi & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIY.hi-=1;

      if(registerIY.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerIY.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 8;
      break;
    }
    case 0x26:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerIY.hi = ldValue;
      OpcodeClicks = 11;
      break;
    }
    case 0x29:
    {
      if(registerIY.reg + registerIY.reg > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerIY.reg & 0x0FFF) + (registerIY.reg & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIY.reg += registerIY.reg;

      OpcodeClicks = 15;
      break;
    }
    case 0x2A:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerIY.reg = EMU_ReadMem(ldValue+1) << 8;
      registerIY.reg |= EMU_ReadMem(ldValue);
      OpcodeClicks = 20;
      break;
    }
    case 0x2B:
    {
      registerIY.reg--;
      OpcodeClicks = 10;
      break;
    }
    case 0x2C:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerIY.lo;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerIY.lo & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIY.lo+=1;

      if(registerIY.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerIY.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 8;
      break;
    }
    case 0x2D:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerIY.lo;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerIY.lo & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIY.lo-=1;

      if(registerIY.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerIY.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 8;
      break;
    }
    case 0x2E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerIY.lo = ldValue;
      OpcodeClicks = 11;
      break;
    }
    case 0x34:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)memValue;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((memValue & 0xF0) + 1) > 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      memValue+=1;

      if(memValue == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(memValue, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), memValue);

      OpcodeClicks = 23;
      break;
    }
    case 0x35:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);

      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)memValue;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((memValue & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      memValue-=1;

      if(memValue == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(memValue, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), memValue);

      OpcodeClicks = 23;
      break;
    }
    case 0x36:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte ldValue2 = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIY.reg + (word)ldValue), ldValue2);
      OpcodeClicks = 19;
      break;
    }
    case 0x39:
    {
      if(registerIY.reg + stackPointer > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerIY.reg & 0x0FFF) + (stackPointer & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIY.reg += stackPointer;

      OpcodeClicks = 15;
      break;
    }
    case 0x44:
    {
      registerBC.hi = registerIY.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x45:
    {
      registerBC.hi = registerIY.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x46:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerBC.hi = EMU_ReadMem(registerIY.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x4C:
    {
      registerBC.lo = registerIY.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x4D:
    {
      registerBC.lo = registerIY.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x4E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerBC.lo = EMU_ReadMem(registerIY.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x54:
    {
      registerDE.hi = registerIY.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x55:
    {
      registerDE.hi = registerIY.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x56:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerDE.hi = EMU_ReadMem(registerIY.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x5C:
    {
      registerDE.lo = registerIY.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x5D:
    {
      registerDE.lo = registerIY.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x5E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerDE.lo = EMU_ReadMem(registerIY.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x60:
    {
      registerIY.hi = registerBC.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x61:
    {
      registerIY.hi = registerBC.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x62:
    {
      registerIY.hi = registerDE.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x63:
    {
      registerIY.hi = registerDE.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x64:
    {
      registerIY.hi = registerIY.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x65:
    {
      registerIY.hi = registerIY.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x66:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerHL.hi = EMU_ReadMem(registerIY.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x67:
    {
      registerIY.hi = registerAF.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x68:
    {
      registerIY.lo = registerBC.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x69:
    {
      registerIY.lo = registerBC.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x6A:
    {
      registerIY.lo = registerDE.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x6B:
    {
      registerIY.lo = registerDE.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x6C:
    {
      registerIY.lo = registerIY.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x6D:
    {
      registerIY.lo = registerIY.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x6E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerHL.lo = EMU_ReadMem(registerIY.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x6F:
    {
      registerIY.lo = registerAF.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x70:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIY.reg + (word)ldValue), registerBC.hi);
      OpcodeClicks = 19;
      break;
    }
    case 0x71:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIY.reg + (word)ldValue), registerBC.lo);
      OpcodeClicks = 19;
      break;
    }
    case 0x72:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIY.reg + (word)ldValue), registerDE.hi);
      OpcodeClicks = 19;
      break;
    }
    case 0x73:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIY.reg + (word)ldValue), registerDE.lo);
      OpcodeClicks = 19;
      break;
    }
    case 0x74:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIY.reg + (word)ldValue), registerHL.hi);
      OpcodeClicks = 19;
      break;
    }
    case 0x75:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIY.reg + (word)ldValue), registerHL.lo);
      OpcodeClicks = 19;
      break;
    }
    case 0x77:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIY.reg + (word)ldValue), registerAF.hi);
      OpcodeClicks = 19;
      break;
    }
    case 0x7C:
    {
      registerAF.hi = registerIY.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x7D:
    {
      registerAF.hi = registerIY.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x7E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerAF.lo = EMU_ReadMem(registerIY.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x84:
    {

      if(registerAF.hi + registerIY.hi > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.hi;
      if(sA + sXY > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) + (registerIY.hi & 0xF0)) & 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerIY.hi;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x85:
    {

      if(registerAF.hi + registerIY.lo > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.lo;
      if(sA + sXY > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) + (registerIY.lo & 0xF0)) & 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerIY.lo;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x86:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);
      
      if(registerAF.hi + memValue > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if(sA + sM > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) + (memValue & 0xF0)) & 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += memValue;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0x8C:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerIY.hi + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.hi;
      if((sA + sXY + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerIY.hi & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerIY.hi;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x8D:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerIY.lo + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.lo;
      if((sA + sXY + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerIY.lo & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerIY.lo;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x8E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + memValue + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if((sA + sM + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (memValue & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += memValue;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0x94:
    {      
      if(registerAF.hi - registerIY.hi < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.hi;
      if(sA - sXY < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerIY.hi & 0xF0)) < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= registerIY.hi;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x95:
    {      
      if(registerAF.hi - registerIY.lo < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.lo;
      if(sA - sXY < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerIY.lo & 0xF0)) < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= registerIY.lo;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x96:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);
      
      if(registerAF.hi - memValue < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if(sA - sM < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= memValue;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0x9C:
    { 
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if((registerAF.hi - registerIY.hi) - carry < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.hi;
      if((sA - sXY) - (signed_byte)carry < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerIY.hi & 0xF0)) - carry < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= registerIY.hi;
      registerAF.hi -= carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x9D:
    { 
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if((registerAF.hi - registerIY.lo) - carry < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.lo;
      if((sA - sXY) - (signed_byte)carry < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerIY.lo & 0xF0)) - carry < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= registerIY.lo;
      registerAF.hi -= carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x9E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);
      
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if((registerAF.hi - memValue) - carry < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if((sA - sM) - (signed_byte)carry < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (memValue & 0xF0)) - carry < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= memValue;
      registerAF.hi -= carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xA4:
    {

      registerAF.hi &= registerIY.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xA5:
    {

      registerAF.hi &= registerIY.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xA6:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerAF.hi &= memValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xAC:
    {
      registerAF.hi ^= registerIY.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xAD:
    {
      registerAF.hi ^= registerIY.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xAE:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerAF.hi ^= memValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xB4:
    {

      registerAF.hi |= registerIY.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xB5:
    {

      registerAF.hi |= registerIY.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xB6:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerAF.hi |= memValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xBC:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.hi;

      byte result = registerAF.hi - registerIY.hi;

      if((registerAF.hi - registerIY.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sXY) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerIY.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xBD:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIY.lo;

      byte result = registerAF.hi - registerIY.lo;

      if((registerAF.hi - registerIY.lo) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sXY) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerIY.lo & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xBE:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIY.reg + (word)ldValue);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;

      byte result = registerAF.hi - memValue;

      if((registerAF.hi - memValue) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sM) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xCB:
    {
      OpcodeClicks = Z80_ExecuteIYBITSOpcode();
      break;
    }
    case 0xE1:
    {
      registerIY.reg = Z80_PopWord();
      OpcodeClicks = 14;
      break;
    }
    case 0xE3:
    {
      byte sp = EMU_ReadMem(stackPointer);
      byte spone = EMU_ReadMem(stackPointer+1);

      byte tempRegHi = registerIY.hi;
      byte tempRegLo = registerIY.lo;

      registerIY.hi = spone;
      registerIY.lo = sp;

      EMU_WriteMem(stackPointer, tempRegLo);
      EMU_WriteMem(stackPointer+1, tempRegHi);

      OpcodeClicks = 23;
      break;
    }
    case 0xE5:
    {
      Z80_PushWord(registerIY.reg);
      OpcodeClicks = 15;
      break;
    }
    case 0xE9:
    {
      programCounter = registerIY.reg;
      OpcodeClicks = 8;
      break;
    }
    case 0xF9:
    {
      stackPointer = registerIY.reg;
      OpcodeClicks = 10;
      break;
    }
    default:
    {
      if(UNOP)
      {
        printf("Unimplemented IY Opcode : 0x%X\n", opcode);
        getchar();
      }
      break;
    }
  }

  if(DEBUG)
  {
    printf("IY Opcode : 0x%X\n", opcode);
  }

  return OpcodeClicks;
}

int Z80_ExecuteBITSOpcode()
{
  byte opcode = EMU_ReadMem(programCounter);

  Z80_IncRegR();

  programCounter++;

  int OpcodeClicks = 0;

  switch(opcode)
  {
    case 0x00:
    {
      OpcodeClicks = Z80_BitsRlc(&registerBC.hi, 8, 0, NULL);
      break;
    }
    case 0x01:
    {
      OpcodeClicks = Z80_BitsRlc(&registerBC.lo, 8, 0, NULL);
      break;
    }
    case 0x02:
    {
      OpcodeClicks = Z80_BitsRlc(&registerDE.hi, 8, 0, NULL);
      break;
    }
    case 0x03:
    {
      OpcodeClicks = Z80_BitsRlc(&registerDE.lo, 8, 0, NULL);
      break;
    }
    case 0x04:
    {
      OpcodeClicks = Z80_BitsRlc(&registerHL.hi, 8, 0, NULL);
      break;
    }
    case 0x05:
    {
      OpcodeClicks = Z80_BitsRlc(&registerHL.lo, 8, 0, NULL);
      break;
    }
    case 0x06:
    {
      OpcodeClicks = Z80_BitsRlc(NULL, 15, 1, &registerHL.reg);
      break;
    }
    case 0x07:
    {
      OpcodeClicks = Z80_BitsRlc(&registerAF.hi, 8, 0, NULL);
      break;
    }
    case 0x08:
    {
      OpcodeClicks = Z80_BitsRrc(&registerBC.hi, 8, 0, NULL);
      break;
    }
    case 0x09:
    {
      OpcodeClicks = Z80_BitsRrc(&registerBC.lo, 8, 0, NULL);
      break;
    }
    case 0x0A:
    {
      OpcodeClicks = Z80_BitsRrc(&registerDE.hi, 8, 0, NULL);
      break;
    }
    case 0x0B:
    {
      OpcodeClicks = Z80_BitsRrc(&registerDE.lo, 8, 0, NULL);
      break;
    }
    case 0x0C:
    {
      OpcodeClicks = Z80_BitsRrc(&registerHL.hi, 8, 0, NULL);
      break;
    }
    case 0x0D:
    {
      OpcodeClicks = Z80_BitsRrc(&registerHL.lo, 8, 0, NULL);
      break;
    }
    case 0x0E:
    {
      OpcodeClicks = Z80_BitsRrc(NULL, 15, 1, &registerHL.reg);
      break;
    }
    case 0x0F:
    {
      OpcodeClicks = Z80_BitsRrc(&registerAF.hi, 8, 0, NULL);
      break;
    }
    case 0x10:
    {
      OpcodeClicks = Z80_BitsRl(&registerBC.hi, 8, 0, NULL);
      break;
    }
    case 0x11:
    {
      OpcodeClicks = Z80_BitsRl(&registerBC.lo, 8, 0, NULL);
      break;
    }
    case 0x12:
    {
      OpcodeClicks = Z80_BitsRl(&registerDE.hi, 8, 0, NULL);
      break;
    }
    case 0x13:
    {
      OpcodeClicks = Z80_BitsRl(&registerDE.lo, 8, 0, NULL);
      break;
    }
    case 0x14:
    {
      OpcodeClicks = Z80_BitsRl(&registerHL.hi, 8, 0, NULL);
      break;
    }
    case 0x15:
    {
      OpcodeClicks = Z80_BitsRl(&registerHL.lo, 8, 0, NULL);
      break;
    }
    case 0x16:
    {
      OpcodeClicks = Z80_BitsRl(NULL, 15, 1, &registerHL.reg);
      break;
    }
    case 0x17:
    {
      OpcodeClicks = Z80_BitsRl(&registerAF.hi, 8, 0, NULL);
      break;
    }
    case 0x18:
    {
      OpcodeClicks = Z80_BitsRr(&registerBC.hi, 8, 0, NULL);
      break;
    }
    case 0x19:
    {
      OpcodeClicks = Z80_BitsRr(&registerBC.lo, 8, 0, NULL);
      break;
    }
    case 0x1A:
    {
      OpcodeClicks = Z80_BitsRr(&registerDE.hi, 8, 0, NULL);
      break;
    }
    case 0x1B:
    {
      OpcodeClicks = Z80_BitsRr(&registerDE.lo, 8, 0, NULL);
      break;
    }
    case 0x1C:
    {
      OpcodeClicks = Z80_BitsRr(&registerHL.hi, 8, 0, NULL);
      break;
    }
    case 0x1D:
    {
      OpcodeClicks = Z80_BitsRr(&registerHL.lo, 8, 0, NULL);
      break;
    }
    case 0x1E:
    {
      OpcodeClicks = Z80_BitsRr(NULL, 15, 1, &registerHL.reg);
      break;
    }
    case 0x1F:
    {
      OpcodeClicks = Z80_BitsRr(&registerAF.hi, 8, 0, NULL);
      break;
    }
    case 0x20:
    {
      OpcodeClicks = Z80_BitsSla(&registerBC.hi, 8, 0, NULL);
      break;
    }
    case 0x21:
    {
      OpcodeClicks = Z80_BitsSla(&registerBC.lo, 8, 0, NULL);
      break;
    }
    case 0x22:
    {
      OpcodeClicks = Z80_BitsSla(&registerDE.hi, 8, 0, NULL);
      break;
    }
    case 0x23:
    {
      OpcodeClicks = Z80_BitsSla(&registerDE.lo, 8, 0, NULL);
      break;
    }
    case 0x24:
    {
      OpcodeClicks = Z80_BitsSla(&registerHL.hi, 8, 0, NULL);
      break;
    }
    case 0x25:
    {
      OpcodeClicks = Z80_BitsSla(&registerHL.lo, 8, 0, NULL);
      break;
    }
    case 0x26:
    {
      OpcodeClicks = Z80_BitsSla(NULL, 15, 1, &registerHL.reg);
      break;
    }
    case 0x27:
    {
      OpcodeClicks = Z80_BitsSla(&registerAF.hi, 8, 0, NULL);
      break;
    }
    case 0x28:
    {
      OpcodeClicks = Z80_BitsSra(&registerBC.hi, 8, 0, NULL);
      break;
    }
    case 0x29:
    {
      OpcodeClicks = Z80_BitsSra(&registerBC.lo, 8, 0, NULL);
      break;
    }
    case 0x2A:
    {
      OpcodeClicks = Z80_BitsSra(&registerDE.hi, 8, 0, NULL);
      break;
    }
    case 0x2B:
    {
      OpcodeClicks = Z80_BitsSra(&registerDE.lo, 8, 0, NULL);
      break;
    }
    case 0x2C:
    {
      OpcodeClicks = Z80_BitsSra(&registerHL.hi, 8, 0, NULL);
      break;
    }
    case 0x2D:
    {
      OpcodeClicks = Z80_BitsSra(&registerHL.lo, 8, 0, NULL);
      break;
    }
    case 0x2E:
    {
      OpcodeClicks = Z80_BitsSra(NULL, 15, 1, &registerHL.reg);
      break;
    }
    case 0x2F:
    {
      OpcodeClicks = Z80_BitsSra(&registerAF.hi, 8, 0, NULL);
      break;
    }
    case 0x30:
    {
      OpcodeClicks = Z80_BitsSll(&registerBC.hi, 8, 0, NULL);
      break;
    }
    case 0x31:
    {
      OpcodeClicks = Z80_BitsSll(&registerBC.lo, 8, 0, NULL);
      break;
    }
    case 0x32:
    {
      OpcodeClicks = Z80_BitsSll(&registerDE.hi, 8, 0, NULL);
      break;
    }
    case 0x33:
    {
      OpcodeClicks = Z80_BitsSll(&registerDE.lo, 8, 0, NULL);
      break;
    }
    case 0x34:
    {
      OpcodeClicks = Z80_BitsSll(&registerHL.hi, 8, 0, NULL);
      break;
    }
    case 0x35:
    {
      OpcodeClicks = Z80_BitsSll(&registerHL.lo, 8, 0, NULL);
      break;
    }
    case 0x36:
    {
      OpcodeClicks = Z80_BitsSll(NULL, 15, 1, &registerHL.reg);
      break;
    }
    case 0x37:
    {
      OpcodeClicks = Z80_BitsSll(&registerAF.hi, 8, 0, NULL);
      break;
    }
    case 0x38:
    {
      OpcodeClicks = Z80_BitsSrl(&registerBC.hi, 8, 0, NULL);
      break;
    }
    case 0x39:
    {
      OpcodeClicks = Z80_BitsSrl(&registerBC.lo, 8, 0, NULL);
      break;
    }
    case 0x3A:
    {
      OpcodeClicks = Z80_BitsSrl(&registerDE.hi, 8, 0, NULL);
      break;
    }
    case 0x3B:
    {
      OpcodeClicks = Z80_BitsSrl(&registerDE.lo, 8, 0, NULL);
      break;
    }
    case 0x3C:
    {
      OpcodeClicks = Z80_BitsSrl(&registerHL.hi, 8, 0, NULL);
      break;
    }
    case 0x3D:
    {
      OpcodeClicks = Z80_BitsSrl(&registerHL.lo, 8, 0, NULL);
      break;
    }
    case 0x3E:
    {
      OpcodeClicks = Z80_BitsSrl(NULL, 15, 1, &registerHL.reg);
      break;
    }
    case 0x3F:
    {
      OpcodeClicks = Z80_BitsSrl(&registerAF.hi, 8, 0, NULL);
      break;
    }
    case 0x40:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.hi, 0, 8);
      break;
    }
    case 0x41:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.lo, 0, 8);
      break;
    }
    case 0x42:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.hi, 0, 8);
      break;
    }
    case 0x43:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.lo, 0, 8);
      break;
    }
    case 0x44:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.hi, 0, 8);
      break;
    }
    case 0x45:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.lo, 0, 8);
      break;
    }
    case 0x46:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsBit(&memValue, 0, 12);
      break;
    }
    case 0x47:
    {
      OpcodeClicks = Z80_BitsBit(&registerAF.hi, 0, 8);
      break;
    }
    case 0x48:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.hi, 1, 8);
      break;
    }
    case 0x49:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.lo, 1, 8);
      break;
    }
    case 0x4A:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.hi, 1, 8);
      break;
    }
    case 0x4B:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.lo, 1, 8);
      break;
    }
    case 0x4C:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.hi, 1, 8);
      break;
    }
    case 0x4D:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.lo, 1, 8);
      break;
    }
    case 0x4E:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsBit(&memValue, 1, 12);
      break;
    }
    case 0x4F:
    {
      OpcodeClicks = Z80_BitsBit(&registerAF.hi, 1, 8);
      break;
    }
    case 0x50:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.hi, 2, 8);
      break;
    }
    case 0x51:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.lo, 2, 8);
      break;
    }
    case 0x52:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.hi, 2, 8);
      break;
    }
    case 0x53:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.lo, 2, 8);
      break;
    }
    case 0x54:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.hi, 2, 8);
      break;
    }
    case 0x55:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.lo, 2, 8);
      break;
    }
    case 0x56:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsBit(&memValue, 2, 12);
      break;
    }
    case 0x57:
    {
      OpcodeClicks = Z80_BitsBit(&registerAF.hi, 2, 8);
      break;
    }
    case 0x58:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.hi, 3, 8);
      break;
    }
    case 0x59:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.lo, 3, 8);
      break;
    }
    case 0x5A:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.hi, 3, 8);
      break;
    }
    case 0x5B:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.lo, 3, 8);
      break;
    }
    case 0x5C:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.hi, 3, 8);
      break;
    }
    case 0x5D:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.lo, 3, 8);
      break;
    }
    case 0x5E:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsBit(&memValue, 3, 12);
      break;
    }
    case 0x5F:
    {
      OpcodeClicks = Z80_BitsBit(&registerAF.hi, 3, 8);
      break;
    }
    case 0x60:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.hi, 4, 8);
      break;
    }
    case 0x61:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.lo, 4, 8);
      break;
    }
    case 0x62:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.hi, 4, 8);
      break;
    }
    case 0x63:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.lo, 4, 8);
      break;
    }
    case 0x64:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.hi, 4, 8);
      break;
    }
    case 0x65:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.lo, 4, 8);
      break;
    }
    case 0x66:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsBit(&memValue, 4, 12);
      break;
    }
    case 0x67:
    {
      OpcodeClicks = Z80_BitsBit(&registerAF.hi, 4, 8);
      break;
    }
    case 0x68:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.hi, 5, 8);
      break;
    }
    case 0x69:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.lo, 5, 8);
      break;
    }
    case 0x6A:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.hi, 5, 8);
      break;
    }
    case 0x6B:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.lo, 5, 8);
      break;
    }
    case 0x6C:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.hi, 5, 8);
      break;
    }
    case 0x6D:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.lo, 5, 8);
      break;
    }
    case 0x6E:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsBit(&memValue, 5, 12);
      break;
    }
    case 0x6F:
    {
      OpcodeClicks = Z80_BitsBit(&registerAF.hi, 5, 8);
      break;
    }
    case 0x70:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.hi, 6, 8);
      break;
    }
    case 0x71:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.lo, 6, 8);
      break;
    }
    case 0x72:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.hi, 6, 8);
      break;
    }
    case 0x73:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.lo, 6, 8);
      break;
    }
    case 0x74:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.hi, 6, 8);
      break;
    }
    case 0x75:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.lo, 6, 8);
      break;
    }
    case 0x76:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsBit(&memValue, 6, 12);
      break;
    }
    case 0x77:
    {
      OpcodeClicks = Z80_BitsBit(&registerAF.hi, 6, 8);
      break;
    }
    case 0x78:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.hi, 7, 8);
      break;
    }
    case 0x79:
    {
      OpcodeClicks = Z80_BitsBit(&registerBC.lo, 7, 8);
      break;
    }
    case 0x7A:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.hi, 7, 8);
      break;
    }
    case 0x7B:
    {
      OpcodeClicks = Z80_BitsBit(&registerDE.lo, 7, 8);
      break;
    }
    case 0x7C:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.hi, 7, 8);
      break;
    }
    case 0x7D:
    {
      OpcodeClicks = Z80_BitsBit(&registerHL.lo, 7, 8);
      break;
    }
    case 0x7E:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsBit(&memValue, 7, 12);
      break;
    }
    case 0x7F:
    {
      OpcodeClicks = Z80_BitsBit(&registerAF.hi, 7, 8);
      break;
    }
    case 0x80:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.hi, 0, 8);
      break;
    }
    case 0x81:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.lo, 0, 8);
      break;
    }
    case 0x82:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.hi, 0, 8);
      break;
    }
    case 0x83:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.lo, 0, 8);
      break;
    }
    case 0x84:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.hi, 0, 8);
      break;
    }
    case 0x85:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.lo, 0, 8);
      break;
    }
    case 0x86:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsReset(&memValue, 0, 15);
      break;
    }
    case 0x87:
    {
      OpcodeClicks = Z80_BitsReset(&registerAF.hi, 0, 8);
      break;
    }
    case 0x88:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.hi, 1, 8);
      break;
    }
    case 0x89:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.lo, 1, 8);
      break;
    }
    case 0x8A:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.hi, 1, 8);
      break;
    }
    case 0x8B:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.lo, 1, 8);
      break;
    }
    case 0x8C:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.hi, 1, 8);
      break;
    }
    case 0x8D:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.lo, 1, 8);
      break;
    }
    case 0x8E:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsReset(&memValue, 1, 15);
      break;
    }
    case 0x8F:
    {
      OpcodeClicks = Z80_BitsReset(&registerAF.hi, 1, 8);
      break;
    }
    case 0x90:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.hi, 2, 8);
      break;
    }
    case 0x91:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.lo, 2, 8);
      break;
    }
    case 0x92:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.hi, 2, 8);
      break;
    }
    case 0x93:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.lo, 2, 8);
      break;
    }
    case 0x94:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.hi, 2, 8);
      break;
    }
    case 0x95:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.lo, 2, 8);
      break;
    }
    case 0x96:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsReset(&memValue, 2, 15);
      break;
    }
    case 0x97:
    {
      OpcodeClicks = Z80_BitsReset(&registerAF.hi, 2, 8);
      break;
    }
    case 0x98:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.hi, 3, 8);
      break;
    }
    case 0x99:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.lo, 3, 8);
      break;
    }
    case 0x9A:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.hi, 3, 8);
      break;
    }
    case 0x9B:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.lo, 3, 8);
      break;
    }
    case 0x9C:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.hi, 3, 8);
      break;
    }
    case 0x9D:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.lo, 3, 8);
      break;
    }
    case 0x9E:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsReset(&memValue, 3, 15);
      break;
    }
    case 0x9F:
    {
      OpcodeClicks = Z80_BitsReset(&registerAF.hi, 3, 8);
      break;
    }
    case 0xA0:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.hi, 4, 8);
      break;
    }
    case 0xA1:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.lo, 4, 8);
      break;
    }
    case 0xA2:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.hi, 4, 8);
      break;
    }
    case 0xA3:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.lo, 4, 8);
      break;
    }
    case 0xA4:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.hi, 4, 8);
      break;
    }
    case 0xA5:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.lo, 4, 8);
      break;
    }
    case 0xA6:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsReset(&memValue, 4, 15);
      break;
    }
    case 0xA7:
    {
      OpcodeClicks = Z80_BitsReset(&registerAF.hi, 4, 8);
      break;
    }
    case 0xA8:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.hi, 5, 8);
      break;
    }
    case 0xA9:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.lo, 5, 8);
      break;
    }
    case 0xAA:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.hi, 5, 8);
      break;
    }
    case 0xAB:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.lo, 5, 8);
      break;
    }
    case 0xAC:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.hi, 5, 8);
      break;
    }
    case 0xAD:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.lo, 5, 8);
      break;
    }
    case 0xAE:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsReset(&memValue, 5, 15);
      break;
    }
    case 0xAF:
    {
      OpcodeClicks = Z80_BitsReset(&registerAF.hi, 5, 8);
      break;
    }
    case 0xB0:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.hi, 6, 8);
      break;
    }
    case 0xB1:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.lo, 6, 8);
      break;
    }
    case 0xB2:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.hi, 6, 8);
      break;
    }
    case 0xB3:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.lo, 6, 8);
      break;
    }
    case 0xB4:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.hi, 6, 8);
      break;
    }
    case 0xB5:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.lo, 6, 8);
      break;
    }
    case 0xB6:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsReset(&memValue, 6, 15);
      break;
    }
    case 0xB7:
    {
      OpcodeClicks = Z80_BitsReset(&registerAF.hi, 6, 8);
      break;
    }
    case 0xB8:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.hi, 7, 8);
      break;
    }
    case 0xB9:
    {
      OpcodeClicks = Z80_BitsReset(&registerBC.lo, 7, 8);
      break;
    }
    case 0xBA:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.hi, 7, 8);
      break;
    }
    case 0xBB:
    {
      OpcodeClicks = Z80_BitsReset(&registerDE.lo, 7, 8);
      break;
    }
    case 0xBC:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.hi, 7, 8);
      break;
    }
    case 0xBD:
    {
      OpcodeClicks = Z80_BitsReset(&registerHL.lo, 7, 8);
      break;
    }
    case 0xBE:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsReset(&memValue, 7, 15);
      break;
    }
    case 0xBF:
    {
      OpcodeClicks = Z80_BitsSet(&registerAF.hi, 7, 8);
      break;
    }
    case 0xC0:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.hi, 0, 8);
      break;
    }
    case 0xC1:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.lo, 0, 8);
      break;
    }
    case 0xC2:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.hi, 0, 8);
      break;
    }
    case 0xC3:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.lo, 0, 8);
      break;
    }
    case 0xC4:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.hi, 0, 8);
      break;
    }
    case 0xC5:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.lo, 0, 8);
      break;
    }
    case 0xC6:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsSet(&memValue, 0, 15);
      break;
    }
    case 0xC7:
    {
      OpcodeClicks = Z80_BitsSet(&registerAF.hi, 0, 8);
      break;
    }
    case 0xC8:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.hi, 1, 8);
      break;
    }
    case 0xC9:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.lo, 1, 8);
      break;
    }
    case 0xCA:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.hi, 1, 8);
      break;
    }
    case 0xCB:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.lo, 1, 8);
      break;
    }
    case 0xCC:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.hi, 1, 8);
      break;
    }
    case 0xCD:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.lo, 1, 8);
      break;
    }
    case 0xCE:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsSet(&memValue, 1, 15);
      break;
    }
    case 0xCF:
    {
      OpcodeClicks = Z80_BitsSet(&registerAF.hi, 1, 8);
      break;
    }
    case 0xD0:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.hi, 2, 8);
      break;
    }
    case 0xD1:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.lo, 2, 8);
      break;
    }
    case 0xD2:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.hi, 2, 8);
      break;
    }
    case 0xD3:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.lo, 2, 8);
      break;
    }
    case 0xD4:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.hi, 2, 8);
      break;
    }
    case 0xD5:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.lo, 2, 8);
      break;
    }
    case 0xD6:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsSet(&memValue, 2, 15);
      break;
    }
    case 0xD7:
    {
      OpcodeClicks = Z80_BitsSet(&registerAF.hi, 2, 8);
      break;
    }
    case 0xD8:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.hi, 3, 8);
      break;
    }
    case 0xD9:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.lo, 3, 8);
      break;
    }
    case 0xDA:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.hi, 3, 8);
      break;
    }
    case 0xDB:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.lo, 3, 8);
      break;
    }
    case 0xDC:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.hi, 3, 8);
      break;
    }
    case 0xDD:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.lo, 3, 8);
      break;
    }
    case 0xDE:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsSet(&memValue, 3, 15);
      break;
    }
    case 0xDF:
    {
      OpcodeClicks = Z80_BitsSet(&registerAF.hi, 3, 8);
      break;
    }
    case 0xE0:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.hi, 4, 8);
      break;
    }
    case 0xE1:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.lo, 4, 8);
      break;
    }
    case 0xE2:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.hi, 4, 8);
      break;
    }
    case 0xE3:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.lo, 4, 8);
      break;
    }
    case 0xE4:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.hi, 4, 8);
      break;
    }
    case 0xE5:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.lo, 4, 8);
      break;
    }
    case 0xE6:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsSet(&memValue, 4, 15);
      break;
    }
    case 0xE7:
    {
      OpcodeClicks = Z80_BitsSet(&registerAF.hi, 4, 8);
      break;
    }
    case 0xE8:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.hi, 5, 8);
      break;
    }
    case 0xE9:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.lo, 5, 8);
      break;
    }
    case 0xEA:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.hi, 5, 8);
      break;
    }
    case 0xEB:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.lo, 5, 8);
      break;
    }
    case 0xEC:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.hi, 5, 8);
      break;
    }
    case 0xED:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.lo, 5, 8);
      break;
    }
    case 0xEE:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsSet(&memValue, 5, 15);
      break;
    }
    case 0xEF:
    {
      OpcodeClicks = Z80_BitsSet(&registerAF.hi, 5, 8);
      break;
    }
    case 0xF0:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.hi, 6, 8);
      break;
    }
    case 0xF1:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.lo, 6, 8);
      break;
    }
    case 0xF2:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.hi, 6, 8);
      break;
    }
    case 0xF3:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.lo, 6, 8);
      break;
    }
    case 0xF4:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.hi, 6, 8);
      break;
    }
    case 0xF5:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.lo, 6, 8);
      break;
    }
    case 0xF6:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsSet(&memValue, 6, 15);
      break;
    }
    case 0xF7:
    {
      OpcodeClicks = Z80_BitsSet(&registerAF.hi, 6, 8);
      break;
    }
    case 0xF8:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.hi, 7, 8);
      break;
    }
    case 0xF9:
    {
      OpcodeClicks = Z80_BitsSet(&registerBC.lo, 7, 8);
      break;
    }
    case 0xFA:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.hi, 7, 8);
      break;
    }
    case 0xFB:
    {
      OpcodeClicks = Z80_BitsSet(&registerDE.lo, 7, 8);
      break;
    }
    case 0xFC:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.hi, 7, 8);
      break;
    }
    case 0xFD:
    {
      OpcodeClicks = Z80_BitsSet(&registerHL.lo, 7, 8);
      break;
    }
    case 0xFE:
    {
      byte memValue = EMU_ReadMem(registerHL.reg);
      OpcodeClicks = Z80_BitsSet(&memValue, 7, 15);
      break;
    }
    case 0xFF:
    {
      OpcodeClicks = Z80_BitsSet(&registerAF.hi, 7, 8);
      break;
    }
    default:
    {
      if(UNOP)
      {
        printf("Unimplemented BITS Opcode : 0x%X\n", opcode);
        getchar();
      }
      break;
    }
  }

  if(DEBUG)
  {
    printf("BITS Opcode : 0x%X\n", opcode);
  }

  return OpcodeClicks;
}

int Z80_ExecuteIYBITSOpcode()
{
  byte opcode = EMU_ReadMem(programCounter);

  Z80_IncRegR();

  programCounter++;

  int OpcodeClicks = 0;

  switch(opcode)
  {
    case 0x00:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerBC.hi = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerBC.hi <<= 1;

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerBC.hi, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerBC.hi, 0);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerBC.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), registerBC.hi);

      OpcodeClicks = 23;
      break;
    }
    case 0x01:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerBC.lo = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerBC.lo <<= 1;

      if(BIT_ByteCheck(registerBC.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerBC.lo, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerBC.lo, 0);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerBC.lo))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerBC.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), registerBC.lo);

      OpcodeClicks = 23;
      break;
    }
    case 0x02:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerDE.hi = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerDE.hi <<= 1;

      if(BIT_ByteCheck(registerDE.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerDE.hi, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerDE.hi, 0);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerDE.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerDE.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerDE.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), registerDE.hi);

      OpcodeClicks = 23;
      break;
    }
    case 0x03:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerDE.lo = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerDE.lo <<= 1;

      if(BIT_ByteCheck(registerDE.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerDE.lo, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerDE.lo, 0);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerDE.lo))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerDE.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerDE.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), registerDE.lo);

      OpcodeClicks = 23;
      break;
    }
    case 0x05:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerHL.lo = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerHL.lo <<= 1;

      if(BIT_ByteCheck(registerHL.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerHL.lo, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerHL.lo, 0);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerHL.lo))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerHL.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerHL.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), registerHL.lo);

      OpcodeClicks = 23;
      break;
    }
    case 0x06:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      byte res = EMU_ReadMem(registerIY.reg + (word)ldValue);

      res <<= 1;

      if(BIT_ByteCheck(res, 7))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&res, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&res, 0);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(res))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(res == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(res, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), res);

      OpcodeClicks = 23;
      break;
    }
    case 0x07:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerAF.hi = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerAF.hi <<= 1;

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerAF.hi, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerAF.hi, 0);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), registerAF.hi);

      OpcodeClicks = 23;
      break;
    }
    case 0x08:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerBC.hi = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerBC.hi >>= 1;

      if(BIT_ByteCheck(registerBC.hi, 0))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerBC.hi, 7);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerBC.hi, 7);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerBC.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), registerBC.hi);

      OpcodeClicks = 23;
      break;
    }
    case 0x09:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerBC.lo = EMU_ReadMem(registerIY.reg + (word)ldValue);

      registerBC.lo >>= 1;

      if(BIT_ByteCheck(registerBC.lo, 0))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerBC.lo, 7);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerBC.lo, 7);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerBC.lo))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerBC.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIY.reg + (word)ldValue), registerBC.lo);

      OpcodeClicks = 23;
      break;
    }
    default:
    {
      if(UNOP)
      {
        printf("Unimplemented IY BITS Opcode : 0x%X\n", opcode);
        getchar();
      }
      break;
    }
  }

  if(DEBUG)
  {
    printf("IY BITS Opcode : 0x%X\n", opcode);
  }

  return OpcodeClicks;
}

int Z80_ExecuteIXOpcode()
{
  byte opcode = EMU_ReadMem(programCounter);

  Z80_IncRegR();

  programCounter++;

  int OpcodeClicks = 0;

  switch(opcode)
  {
    case 0x09:
    {
      if(registerIX.reg + registerBC.reg > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerIX.reg & 0x0FFF) + (registerBC.reg & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIX.reg += registerBC.reg;

      OpcodeClicks = 15;
      break;
    }
    case 0x19:
    {
      if(registerIX.reg + registerDE.reg > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerIX.reg & 0x0FFF) + (registerDE.reg & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIX.reg += registerDE.reg;

      OpcodeClicks = 15;
      break;
    }
    case 0x21:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerIX.reg = ldValue;
      OpcodeClicks = 14;
      break;
    }
    case 0x22:
    {
      word ldValue = Z80_FetchWord();
      programCounter+=2;
      EMU_WriteMem(ldValue, registerIX.lo);
      EMU_WriteMem(ldValue+1, registerIX.hi);
      OpcodeClicks = 22;
      break;
    }
    case 0x23:
    {
      registerIX.reg++;
      OpcodeClicks = 10;
      break;
    }
    case 0x24:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerIX.hi;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerIX.hi & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIX.hi+=1;

      if(registerIX.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerIX.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 8;
      break;
    }
    case 0x25:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerIX.hi;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerIX.hi & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIX.hi-=1;

      if(registerIX.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerIX.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 8;
      break;
    }
    case 0x26:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerIX.hi = ldValue;
      OpcodeClicks = 11;
      break;
    }
    case 0x29:
    {
      if(registerIX.reg + registerIX.reg > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerIX.reg & 0x0FFF) + (registerIX.reg & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIX.reg += registerIX.reg;

      OpcodeClicks = 15;
      break;
    }
    case 0x2A:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerIX.reg = EMU_ReadMem(ldValue+1) << 8;
      registerIX.reg |= EMU_ReadMem(ldValue);
      OpcodeClicks = 20;
      break;
    }
    case 0x2B:
    {
      registerIX.reg--;
      OpcodeClicks = 10;
      break;
    }
    case 0x2C:
    {
      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerIX.lo;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerIX.lo & 0xF0) + 1) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIX.lo+=1;

      if(registerIX.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerIX.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 8;
      break;
    }
    case 0x2D:
    {
      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)registerIX.lo;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerIX.lo & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIX.lo-=1;

      if(registerIX.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerIX.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }
      OpcodeClicks = 8;
      break;
    }
    case 0x2E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerIX.lo = ldValue;
      OpcodeClicks = 11;
      break;
    }
    case 0x34:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)memValue;
      if(svalue == 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((memValue & 0xF0) + 1) > 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      memValue+=1;

      if(memValue == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(memValue, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIX.reg + (word)ldValue), memValue);

      OpcodeClicks = 23;
      break;
    }
    case 0x35:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);

      BIT_ByteSet(&registerAF.lo, N_Flag);

      signed_byte svalue = (signed_byte)memValue;
      if(svalue == -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((memValue & 0xF0) - 1) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      memValue-=1;

      if(memValue == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(memValue, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIX.reg + (word)ldValue), memValue);

      OpcodeClicks = 23;
      break;
    }
    case 0x36:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte ldValue2 = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIX.reg + (word)ldValue), ldValue2);
      OpcodeClicks = 19;
      break;
    }
    case 0x39:
    {
      if(registerIX.reg + stackPointer > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(((registerIX.reg & 0x0FFF) + (stackPointer & 0x0FFF)) > 0x0FFF)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerIX.reg += stackPointer;

      OpcodeClicks = 15;
      break;
    }
    case 0x44:
    {
      registerBC.hi = registerIX.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x45:
    {
      registerBC.hi = registerIX.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x46:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerBC.hi = EMU_ReadMem(registerIX.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x4C:
    {
      registerBC.lo = registerIX.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x4D:
    {
      registerBC.lo = registerIX.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x4E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerBC.lo = EMU_ReadMem(registerIX.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x54:
    {
      registerDE.hi = registerIX.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x55:
    {
      registerDE.hi = registerIX.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x56:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerDE.hi = EMU_ReadMem(registerIX.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x5C:
    {
      registerDE.lo = registerIX.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x5D:
    {
      registerDE.lo = registerIX.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x5E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerDE.lo = EMU_ReadMem(registerIX.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x60:
    {
      registerIX.hi = registerBC.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x61:
    {
      registerIX.hi = registerBC.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x62:
    {
      registerIX.hi = registerDE.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x63:
    {
      registerIX.hi = registerDE.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x64:
    {
      registerIX.hi = registerIX.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x65:
    {
      registerIX.hi = registerIX.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x66:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerHL.hi = EMU_ReadMem(registerIX.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x67:
    {
      registerIX.hi = registerAF.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x68:
    {
      registerIX.lo = registerBC.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x69:
    {
      registerIX.lo = registerBC.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x6A:
    {
      registerIX.lo = registerDE.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x6B:
    {
      registerIX.lo = registerDE.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x6C:
    {
      registerIX.lo = registerIX.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x6D:
    {
      registerIX.lo = registerIX.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x6E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerHL.lo = EMU_ReadMem(registerIX.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x6F:
    {
      registerIX.lo = registerAF.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x70:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIX.reg + (word)ldValue), registerBC.hi);
      OpcodeClicks = 19;
      break;
    }
    case 0x71:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIX.reg + (word)ldValue), registerBC.lo);
      OpcodeClicks = 19;
      break;
    }
    case 0x72:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIX.reg + (word)ldValue), registerDE.hi);
      OpcodeClicks = 19;
      break;
    }
    case 0x73:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIX.reg + (word)ldValue), registerDE.lo);
      OpcodeClicks = 19;
      break;
    }
    case 0x74:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIX.reg + (word)ldValue), registerHL.hi);
      OpcodeClicks = 19;
      break;
    }
    case 0x75:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIX.reg + (word)ldValue), registerHL.lo);
      OpcodeClicks = 19;
      break;
    }
    case 0x77:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem((registerIX.reg + (word)ldValue), registerAF.hi);
      OpcodeClicks = 19;
      break;
    }
    case 0x7C:
    {
      registerAF.hi = registerIX.hi;
      OpcodeClicks = 8;
      break;
    }
    case 0x7D:
    {
      registerAF.hi = registerIX.lo;
      OpcodeClicks = 8;
      break;
    }
    case 0x7E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerAF.lo = EMU_ReadMem(registerIX.reg + (word)ldValue);
      OpcodeClicks = 19;
      break;
    }
    case 0x84:
    {

      if(registerAF.hi + registerIX.hi > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.hi;
      if(sA + sXY > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) + (registerIX.hi & 0xF0)) & 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerIX.hi;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x85:
    {

      if(registerAF.hi + registerIX.lo > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.lo;
      if(sA + sXY > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) + (registerIX.lo & 0xF0)) & 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerIX.lo;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x86:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);
      
      if(registerAF.hi + memValue > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if(sA + sM > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) + (memValue & 0xF0)) & 0x10)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += memValue;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0x8C:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerIX.hi + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.hi;
      if((sA + sXY + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerIX.hi & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerIX.hi;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x8D:
    {
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + registerIX.lo + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.lo;
      if((sA + sXY + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (registerIX.lo & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += registerIX.lo;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x8E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);
      
      if(registerAF.hi + memValue + carry > 0xFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if((sA + sM + (signed_byte)carry) > 127)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) + (memValue & 0xF0) + carry) & 0x10))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi += memValue;
      registerAF.hi += carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0x94:
    {      
      if(registerAF.hi - registerIX.hi < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.hi;
      if(sA - sXY < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerIX.hi & 0xF0)) < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= registerIX.hi;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x95:
    {      
      if(registerAF.hi - registerIX.lo < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.lo;
      if(sA - sXY < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerIX.lo & 0xF0)) < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= registerIX.lo;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x96:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);
      
      if(registerAF.hi - memValue < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if(sA - sM < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= memValue;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0x9C:
    { 
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if((registerAF.hi - registerIX.hi) - carry < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.hi;
      if((sA - sXY) - (signed_byte)carry < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerIX.hi & 0xF0)) - carry < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= registerIX.hi;
      registerAF.hi -= carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x9D:
    { 
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if((registerAF.hi - registerIX.lo) - carry < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.lo;
      if((sA - sXY) - (signed_byte)carry < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (registerIX.lo & 0xF0)) - carry < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= registerIX.lo;
      registerAF.hi -= carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0x9E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);
      
      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      if((registerAF.hi - memValue) - carry < 0x0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;
      if((sA - sM) - (signed_byte)carry < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if((((registerAF.hi & 0xF0) - (memValue & 0xF0)) - carry < 0x0))
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      registerAF.hi -= memValue;
      registerAF.hi -= carry;

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xA4:
    {

      registerAF.hi &= registerIX.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xA5:
    {

      registerAF.hi &= registerIX.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xA6:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);

      registerAF.hi &= memValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xAC:
    {
      registerAF.hi ^= registerIX.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xAD:
    {
      registerAF.hi ^= registerIX.lo;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xAE:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);

      registerAF.hi ^= memValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xB4:
    {

      registerAF.hi |= registerIX.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xB5:
    {

      registerAF.hi |= registerIX.hi;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xB6:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);

      registerAF.hi |= memValue;

      BIT_ByteClear(&registerAF.lo, C_Flag);

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if(Z80_IsEvenParity(registerAF.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(registerAF.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerAF.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xBC:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.hi;

      byte result = registerAF.hi - registerIX.hi;

      if((registerAF.hi - registerIX.hi) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sXY) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerIX.hi & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xBD:
    {
      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sXY = (signed_byte)registerIX.lo;

      byte result = registerAF.hi - registerIX.lo;

      if((registerAF.hi - registerIX.lo) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sXY) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (registerIX.lo & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 8;
      break;
    }
    case 0xBE:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      byte memValue = EMU_ReadMem(registerIX.reg + (word)ldValue);

      signed_byte sA = (signed_byte)registerAF.hi;
      signed_byte sM = (signed_byte)memValue;

      byte result = registerAF.hi - memValue;

      if((registerAF.hi - memValue) < 0)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteSet(&registerAF.lo, N_Flag);


      if((sA - sM) < -128)
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(((registerAF.hi & 0xF0) - (memValue & 0xF0)) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      if(result == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(result, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      OpcodeClicks = 19;
      break;
    }
    case 0xCB:
    {
      OpcodeClicks = Z80_ExecuteIXBITSOpcode();
      break;
    }
    case 0xE1:
    {
      registerIX.reg = Z80_PopWord();
      OpcodeClicks = 14;
      break;
    }
    case 0xE3:
    {
      byte sp = EMU_ReadMem(stackPointer);
      byte spone = EMU_ReadMem(stackPointer+1);

      byte tempRegHi = registerIX.hi;
      byte tempRegLo = registerIX.lo;

      registerIX.hi = spone;
      registerIX.lo = sp;

      EMU_WriteMem(stackPointer, tempRegLo);
      EMU_WriteMem(stackPointer+1, tempRegHi);

      OpcodeClicks = 23;
      break;
    }
    case 0xE5:
    {
      Z80_PushWord(registerIX.reg);
      OpcodeClicks = 15;
      break;
    }
    case 0xE9:
    {
      programCounter = registerIX.reg;
      OpcodeClicks = 8;
      break;
    }
    case 0xF9:
    {
      stackPointer = registerIX.reg;
      OpcodeClicks = 10;
      break;
    }
    default:
    {
      if(UNOP)
      {
        printf("Unimplemented IX Opcode : 0x%X\n", opcode);
        getchar();
      }
      break;
    }
  }

  if(DEBUG)
  {
    printf("IX Opcode : 0x%X\n", opcode);
  }

  return OpcodeClicks;
}

int Z80_ExecuteIXBITSOpcode()
{
  byte opcode = EMU_ReadMem(programCounter);

  Z80_IncRegR();

  programCounter++;

  int OpcodeClicks = 0;

  switch(opcode)
  {
    case 0x00:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerBC.hi = EMU_ReadMem(registerIX.reg + (word)ldValue);

      registerBC.hi <<= 1;

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerBC.hi, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerBC.hi, 0);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerBC.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIX.reg + (word)ldValue), registerBC.hi);

      OpcodeClicks = 23;
      break;
    }
    case 0x01:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerBC.lo = EMU_ReadMem(registerIX.reg + (word)ldValue);

      registerBC.lo <<= 1;

      if(BIT_ByteCheck(registerBC.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
        BIT_ByteSet(&registerBC.lo, 0);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
        BIT_ByteClear(&registerBC.lo, 0);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerBC.lo))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerBC.lo == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.lo, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIX.reg + (word)ldValue), registerBC.lo);

      OpcodeClicks = 23;
      break;
    }
    case 0x18:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerBC.hi = EMU_ReadMem(registerIX.reg + (word)ldValue);

      byte carry = BIT_ByteCheck(registerAF.lo, C_Flag);

      registerBC.hi >>= 1;

      if(BIT_ByteCheck(registerBC.hi, 0))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      if(carry)
      {
        BIT_ByteSet(&registerBC.hi, 7);
      }
      else
      {
        BIT_ByteClear(&registerBC.hi, 7);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerBC.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIX.reg + (word)ldValue), registerBC.hi);

      OpcodeClicks = 23;
      break;
    }
    case 0x28:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;

      registerBC.hi = EMU_ReadMem(registerIX.reg + (word)ldValue);

      byte bit7 = BIT_ByteCheck(registerBC.hi, 7);

      registerBC.hi >>= 1;

      if(BIT_ByteCheck(registerBC.hi, 0))
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      if(bit7)
      {
        BIT_ByteSet(&registerBC.hi, 7);
      }
      else
      {
        BIT_ByteClear(&registerBC.hi, 7);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);
      BIT_ByteClear(&registerAF.lo, H_Flag);

      if(Z80_IsEvenParity(registerBC.hi))
      {
        BIT_ByteSet(&registerAF.lo, PV_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, PV_Flag);
      }

      if(registerBC.hi == 0)
      {
        BIT_ByteSet(&registerAF.lo, Z_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, Z_Flag);
      }

      if(BIT_ByteCheck(registerBC.hi, 7))
      {
        BIT_ByteSet(&registerAF.lo, S_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, S_Flag);
      }

      EMU_WriteMem((registerIX.reg + (word)ldValue), registerBC.hi);

      OpcodeClicks = 23;
      break;
    }
    default:
    {
      if(UNOP)
      {
        printf("Unimplemented IX BITS Opcode : 0x%X\n", opcode);
        getchar();
      }
      break;
    }
  }

  if(DEBUG)
  {
    printf("IX BITS Opcode : 0x%X\n", opcode);
  }

  return OpcodeClicks;
}
