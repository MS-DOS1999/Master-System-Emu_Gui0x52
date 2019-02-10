#include "bitUtils.h"

//set
void BIT_ByteSet(byte *value, int bit)
{
  *value |= 1 << bit;
}

void BIT_SByteSet(signed_byte *value, int bit)
{
  *value |= 1 << bit;
}

void BIT_WordSet(word *value, int bit)
{
  *value |= 1 << bit;
}

void BIT_SWordSet(signed_word *value, int bit)
{
  *value |= 1 << bit;
}

//clear
void BIT_ByteClear(byte *value, int bit)
{
  *value &= ~(1 << bit);
}

void BIT_SByteClear(signed_byte *value, int bit)
{
  *value &= ~(1 << bit);
}

void BIT_WordClear(word *value, int bit)
{
  *value &= ~(1 << bit);
}

void BIT_SWordClear(signed_word *value, int bit)
{
  *value &= ~(1 << bit);
}

//check
int BIT_ByteCheck(byte value, int bit)
{
  int result = (value >> bit) & 1;

  return result;
}

int BIT_SByteCheck(signed_byte value, int bit)
{
  int result = (value >> bit) & 1;

  return result;
}

int BIT_WordCheck(word value, int bit)
{
  int result = (value >> bit) & 1;

  return result;
}

int BIT_SWordCheck(signed_word value, int bit)
{
  int result = (value >> bit) & 1;

  return result;
}
