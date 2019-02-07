#ifndef Z80_H
#define Z80_H

#include <stdio.h>
#include <windows.h>
#include "oldSize.h"
#include "emu.h"
#include "tms.h"

#define C_Flag 0
#define N_Flag 1
#define PV_Flag 2
#define H_Flag 4
#define Z_Flag 6
#define S_Flag 7

/*
A: Accumulator
B: General Purpose
C: General Purpose
D: General Purpose
E: General Purpose
F: Flag Register
H: Address register
L: Address register
I: Interrupt vector
R: Refresh counter
*/

/*
Bit 7 = (S)Sign Flag
Bit 6 = (Z)Zero Flag
Bit 5 = Not used
Bit 4 = (H)Half Carry Flag
Bit 3 = Not used
Bit 2 = (P/V)Parity or Overflow Flag
Bit 1 = (N)Subrtract Flag
Bit 0 = (C)Carry Flag
*/

/*The sign flag is simply a copy of the 7th bit of the result of a math Z80 instruction. However with some 16bit instructions it will be a copy of the 15th bit of the resulting operation.
The zero flag is set when the result of an operation is zero
The Half carry flag is set if a subtract operation carried from bit 4 to bit 3 or, or with an 16 bit operation carried from bit 12 to 11. With addition operations it is set if carried from bit 3 to 4 or with 16 bit addition 11 to 12
The Parity or Overflow flag has two meanings. Some instructions use the parity flag which means it gets set if the result of the operation has an even number of bits set. The overflow flag is used by some instructions when the 2-complement of the result does not fit within the register
The subtract flag is simply set when the instruction is a subtraction
The carry flag is set when the instruction overflows its upper or lower limits.*/

extern word programCounter;
extern word stackPointer;

extern byte registerI;
extern byte registerR;

typedef union
{
  word reg;
  struct
  {
    byte lo;
    byte hi;
  };
}Z80_register;

extern Z80_register registerAF;
extern Z80_register registerBC;
extern Z80_register registerDE;
extern Z80_register registerHL;
extern Z80_register registerAFShadow;
extern Z80_register registerBCShadow;
extern Z80_register registerDEShadow;
extern Z80_register registerHLShadow;

//registre d'index
extern Z80_register registerIX;
extern Z80_register registerIY;

extern byte ResetInt;
extern byte ExecuteReset;
extern int IntMode;
extern byte IFF1;
extern byte IFF2;
extern byte Halted;
extern byte EIPending;

extern word DAATable[0x800];
extern byte ZSPTable[256];

void InitDAATable();
void Z80_UpdateInterrupts();
int Z80_ExecuteInstruction();
void Z80_IncRegR();
int Z80_IsEvenParity(byte x);
word Z80_FetchWord();
void Z80_PushWord(word value);
word Z80_PopWord();
byte Z80_FetchByte();

int Z80_BitsRlc(byte *reg, int OpcodeClicks, int isMem, word *memReg);
int Z80_BitsRrc(byte *reg, int OpcodeClicks, int isMem, word *memReg);
int Z80_BitsRl(byte *reg, int OpcodeClicks, int isMem, word *memReg);
int Z80_BitsRr(byte *reg, int OpcodeClicks, int isMem, word *memReg);
int Z80_BitsSla(byte *reg, int OpcodeClicks, int isMem, word *memReg);
int Z80_BitsSra(byte *reg, int OpcodeClicks, int isMem, word *memReg);
int Z80_BitsSll(byte *reg, int OpcodeClicks, int isMem, word *memReg);
int Z80_BitsSrl(byte *reg, int OpcodeClicks, int isMem, word *memReg);
int Z80_BitsBit(byte *reg, int bit, int OpcodeClicks);
int Z80_BitsReset(byte *reg, int bit, int OpcodeClicks);
int Z80_BitsSet(byte *reg, int bit, int OpcodeClicks);

int Z80_ExecuteOpcode(byte opcode);
int Z80_ExecuteEXTDOpcode();
int Z80_ExecuteIYOpcode();
int Z80_ExecuteBITSOpcode();
int Z80_ExecuteIYBITSOpcode();
int Z80_ExecuteIXOpcode();
int Z80_ExecuteIXBITSOpcode();

#endif
