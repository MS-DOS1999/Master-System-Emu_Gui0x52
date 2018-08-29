#include "z80.h"

int Z80_ExecuteInstruction()
{
  int OpcodeClicks = 0;
  byte opcode = EMU_ReadMem(programCounter);
  programCounter++;
  OpcodeClicks = Z80_ExecuteOpcode(opcode);
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

byte Z80_FetchByte()
{
  return EMU_ReadMem(programCounter);
}

int Z80_ExecuteOpcode()
{
  Z80Z80_IncRegR();

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
      BIT_ByteClear(registerAF.lo, N_Flag);

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
      BIT_ByteSet(registerAF.lo, N_Flag);

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

      BIT_ByteClear(registerAF.lo, N_Flag);

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
      BIT_ByteClear(registerAF.lo, N_Flag);

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
      BIT_ByteSet(registerAF.lo, N_Flag);

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
      BIT_ByteClear(registerAF.lo, N_Flag);

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
      BIT_ByteSet(registerAF.lo, N_Flag);

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
      programCounter+=ldvalue;
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

      BIT_ByteClear(registerAF.lo, N_Flag);

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
      BIT_ByteClear(registerAF.lo, N_Flag);

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
      BIT_ByteSet(registerAF.lo, N_Flag);

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
      BIT_ByteClear(registerAF.lo, N_Flag);

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
      BIT_ByteSet(registerAF.lo, N_Flag);

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
  }

  return OpcodeClicks;
}
