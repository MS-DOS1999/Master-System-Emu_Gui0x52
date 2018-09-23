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

      if(((registerBC.hi & 0xF0) + (1 & 0xF0) & 0x10))
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

      if((registerBC.hi & 0xF0) - (1 & 0xF0) < 0)
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

      if((registerHL.reg & 0x0FFF) + (registerBC.reg & 0x0FFF) > 0x0FFF)
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

      if(((registerBC.lo & 0xF0) + (1 & 0xF0) & 0x10))
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

      if((registerBC.lo & 0xF0) - (1 & 0xF0) < 0)
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
      programCounter+=1;
      registerBC.hi -=1;
      OpcodeClicks = 8;
      if(registerBC.hi != 0)
      {
        programCounter += ldValue;
        OpcodeClicks = 13;
      }
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

      if(((registerDE.hi & 0xF0) + (1 & 0xF0) & 0x10))
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

      if((registerDE.hi & 0xF0) - (1 & 0xF0) < 0)
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

      if((registerHL.reg & 0x0FFF) + (registerDE.reg & 0x0FFF) > 0x0FFF)
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

      if(((registerDE.lo & 0xF0) + (1 & 0xF0) & 0x10))
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

      if((registerDE.lo & 0xF0) - (1 & 0xF0) < 0)
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

      if(((registerHL.hi & 0xF0) + (1 & 0xF0) & 0x10))
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

      if((registerHL.hi & 0xF0) - (1 & 0xF0) < 0)
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
    case 0x2A:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerHL.reg = EMU_ReadMem(ldValue+1) << 8;
      registerHL.reg |= EMU_ReadMem(ldValue);
      OpcodeClicks = 16;
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
    case 0x36:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      EMU_WriteMem(registerHL.reg, ldValue);
      OpcodeClicks = 10;
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

      if((registerHL.hi & 0xF0) - ((stackPointer >> 8) & 0xF0) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

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

      if(((registerAF.hi & 0xF0) + (1 & 0xF0) & 0x10))
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

      if((registerAF.hi & 0xF0) - (1 & 0xF0) < 0)
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
    case 0x54:
    {
      registerDE.hi = registerHL.hi;
      OpcodeClicks = 4;
      break;
    }
    case 0x5D:
    {
      registerDE.lo = registerHL.lo;
      OpcodeClicks = 4;
      break;
    }
    case 0x5F:
    {
      registerDE.lo = registerAF.hi;
      OpcodeClicks = 4;
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
      EMU_WriteMem(registerHL.reg, registerAF.lo);
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

      if((registerAF.hi & 0xF0) - (registerBC.hi & 0xF0) < 0)
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

      if((registerAF.hi & 0xF0) - (registerBC.lo & 0xF0) < 0)
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

      if((registerAF.hi & 0xF0) - (registerAF.hi & 0xF0) < 0)
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

      if((registerAF.hi & 0xF0) - (registerBC.hi & 0xF0) < 0)
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
    case 0xC9:
    {
      programCounter = Z80_PopWord();
      OpcodeClicks = 10;
      break;
    }
    case 0xCB:
    {
      OpcodeClicks = Z80_ExecuteBITSOpcode();
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
    case 0xCF:
    {
      Z80_PushWord(programCounter);
      programCounter = 0x8;
      OpcodeClicks = 11;
      break;
    }
    case 0xD1:
    {
      registerDE.reg = Z80_PopWord();
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

      if((registerAF.hi & 0xF0) - (ldValue & 0xF0) < 0)
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
    case 0xDB:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerAF.hi = EMU_ReadIO(ldValue);
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
    case 0xED:
    {
      OpcodeClicks = Z80_ExecuteEXTDOpcode();
      break;
    }
    case 0xF1:
    {
      registerAF.reg = Z80_PopWord();
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
    case 0xF9:
    {
      stackPointer = registerHL.reg;
      OpcodeClicks = 6;
      break;
    }
    case 0xFB:
    {
      EIPending = 1;
      OpcodeClicks = 4;
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

      if((registerAF.hi & 0xF0) - (ldValue & 0xF0) < 0)
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

    default:
    {
      printf("Unimplemented Opcode : 0x%X\n", opcode);
      getchar();
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

      if((registerHL.hi & 0xF0) - (registerBC.hi & 0xF0) < 0)
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
    case 0x4D:
    {
      IFF1 = IFF2;
      ExecuteReset = 0;
      programCounter = Z80_PopWord();
      OpcodeClicks = 14;
      break;
    }
    case 0x56:
    {
      IntMode = 1;
      OpcodeClicks = 8;
      break;
    }
    case 0x61:
    {
      EMU_WriteIO(registerBC.lo, registerHL.hi);
      OpcodeClicks = 12;
      break;
    }
    case 0x69:
    {
      EMU_WriteIO(registerBC.lo, registerHL.lo);
      OpcodeClicks = 12;
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

    default:
    {
      printf("Unimplemented EXTD Opcode : 0x%X\n", opcode);
      getchar();
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
    case 0x21:
    {
      word ldValue = Z80_FetchWord();
      programCounter += 2;
      registerIY.reg = ldValue;
      OpcodeClicks = 11;
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
      word result = registerIY.reg + stackPointer;

      if((registerIY.reg + stackPointer) > 0xFFFF)
      {
        BIT_ByteSet(&registerAF.lo, C_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, C_Flag);
      }

      BIT_ByteClear(&registerAF.lo, N_Flag);

      if((registerIY.hi & 0xF0) - ((stackPointer >> 8) & 0xF0) < 0)
      {
        BIT_ByteSet(&registerAF.lo, H_Flag);
      }
      else
      {
        BIT_ByteClear(&registerAF.lo, H_Flag);
      }

      OpcodeClicks = 15;
      break;

    }
    case 0x6E:
    {
      byte ldValue = Z80_FetchByte();
      programCounter++;
      registerHL.lo = EMU_ReadMem((registerIY.reg + (word)ldValue));
      OpcodeClicks = 19;
      break;
    }
    default:
    {
      printf("Unimplemented IY Opcode : 0x%X\n", opcode);
      getchar();
      break;
    }
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
    case 0x40:
    {
      byte isTrue = BIT_ByteCheck(registerBC.hi, 0);

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
      OpcodeClicks = 8;
      break;
    }
    case 0xFF:
    {
      BIT_ByteSet(&registerAF.hi, 7);
      OpcodeClicks = 8;
      break;
    }
    default:
    {
      printf("Unimplemented BITS Opcode : 0x%X\n", opcode);
      getchar();
      break;
    }
  }

  return OpcodeClicks;
}