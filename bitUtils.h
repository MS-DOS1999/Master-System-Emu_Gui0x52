#ifndef BITUTIlS
#define BITUTILS

#include "oldSize.h"
//set
void BIT_ByteSet(byte *value, int bit);
void BIT_UnByteSet(unsigned_byte *value, int bit);
void BIT_WordSet(word *value, int bit);
void BIT_UnWordSet(unsigned_word *value, int bit);

//clear
void BIT_ByteClear(byte *value, int bit);
void BIT_UnByteClear(unsigned_byte *value, int bit);
void BIT_WordClear(word *value, int bit);
void BIT_UnWordClear(unsigned_word *value, int bit);

//check
int BIT_ByteCheck(byte value, int bit);
int BIT_UnByteCheck(unsigned_byte value, int bit);
int BIT_WordCheck(word value, int bit);
int BIT_UnWordCheck(unsigned_word value, int bit);

#endif
