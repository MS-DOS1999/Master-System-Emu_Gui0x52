#include "bitUtils.h"

//set
void BIT_ByteSet(byte *value, int bit)
{
  *value |= 1 << bit;
}

void BIT_UnByteSet(unsigned_byte *value, int bit)
{
  *value |= 1 << bit;
}

void BIT_WordSet(word *value, int bit)
{
  *value |= 1 << bit;
}

void BIT_UnWordSet(unsigned_word *value, int bit)
{
  *value |= 1 << bit;
}

//clear
void BIT_ByteClear(byte *value, int bit)
{
  *value &= ~(1 << bit);
}

void BIT_UnByteClear(unsigned_byte *value, int bit)
{
  *value &= ~(1 << bit);
}

void BIT_WordClear(word *value, int bit)
{
  *value &= ~(1 << bit);
}

void BIT_UnWordClear(unsigned_word *value, int bit)
{
  *value &= ~(1 << bit);
}

//check
int BIT_ByteCheck(byte value, int bit)
{
  int result = (value >> bit) & 1;

  return result;
}

int BIT_UnByteCheck(unsigned_byte value, int bit)
{
  int result = (value >> bit) & 1;

  return result;
}

int BIT_WordCheck(word value, int bit)
{
  int result = (value >> bit) & 1;

  return result;
}

int BIT_UnWordCheck(unsigned_word value, int bit)
{
  int result = (value >> bit) & 1;

  return result;
}
