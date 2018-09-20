#ifndef Z80_H
#define Z80_H

#include <stdio.h>
#include "oldSize.h"
#include "emu.h"

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

word programCounter;
word stackPointer;

word registerI;
word registerR;

typedef union
{
  word reg;
  struct
  {
    byte lo;
    byte hi;
  };
}Z80_register;

Z80_register registerAF;
Z80_register registerBC;
Z80_register registerDE;
Z80_register registerHL;
Z80_register registerAFShadow;
Z80_register registerBCShadow;
Z80_register registerDEShadow;
Z80_register registerHLShadow;

//registre d'index
Z80_register registerIX;
Z80_register registerIY;

int Z80_ExecuteInstruction();
void Z80_IncRegR();
int Z80_IsEvenParity(byte x);
word Z80_FetchWord();
byte Z80_FetchByte();
int Z80_ExecuteOpcode(byte opcode);

#endif
