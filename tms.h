#ifndef TMS_H
#define TMS_H

#include "oldSize.h"
#include "bitUtils.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "emu.h"

/*
0x0000 - 0x1FFF = Sprite / tile patters (numbers 0 to 255) 
0x2000 - 0x37FF = Sprite / tile patters (numbers 256 to 447)
0x3800 - 0x3EFF = Name Table 
0x3F00 - 0x3FFF = Sprite Info Table
*/

/*
BIT 7 = VSync Interrupt Pending
BIT 6 = Sprite Overflow
BIT 5 = Sprite Collision
BIT 4-0 = Unused
*/

/*
0x7E = VCounter (read only)
0x7F = HCounter( read only)
0xBE = Data Port (read/write)
0xBF = Control Port ( read/write).
*/

/*
NTSC 256x192 (small)
0-191 = active display
192-255 = inactive display
Vcounter values = 0x0-0xDA, 0xD5-0xFF

NTSC 256x224(medium)
0-223 = active display
224-255 = inactive display
VCounter values = 0x0-0xEA, 0x0E5-0xFF

NTSC 256x240(large)
doesnt work in NTSC

PAL 256x192(small)
0 - 191 = active display 
192 - 255 = inactive display
VCounter Values = 0x0-0xF2,0xBA-0xFF

PAL 256-224(medium)
0-223 = active display
224-255 = inactive display
VCounter values = 0x0-0xFF, 0x0-0x02, 0xCA-0xFF

PAL 256x240(large)
0-239 = active display
240-255 = inactive display
VCounter Values = 0x0-0xFF,0-0x0A,0xD2,0xFF
*/

#define NUM_RES_HORIZONTAL 256
#define NUM_RES_VERT_SMALL 192
#define NUM_NTSC_VERTICAL  262
#define NUM_PAL_VERTICAL   313
#define NUM_RES_VERT_MED   224
#define NUM_RES_VERT_HIGH  240

byte screenSmall[NUM_RES_HORIZONTAL][NUM_RES_VERT_SMALL][3];
byte screenMedium[NUM_RES_HORIZONTAL][NUM_RES_VERT_MED][3];
byte screenHigh[NUM_RES_HORIZONTAL][NUM_RES_VERT_HIGH][3];

/*
Register 0x0:
Bit7 = If set then vertial scrolling for columns 24-31 are disabled
Bit6 = If set then horizontal scrolling for colums 0-1 are disabled
Bit5 = If set then column 0 is set to the colour of register 0x7
Bit4 = If set then line interrupt is enabled
Bit3 = If set sprites are moved left by 8 pixels
Bit2 = If set use Mode 4
Bit1 = If set use Mode 2. Must also be set for mode4 to change screen resolution

Register 0x1:
Bit6 = If set the screen is enabled
Bit5 = If set vsync interrupts are enabled
Bit4 = If set active display has 224 (medium) scanlines. Reg 0 bit1 must be set
Bit3 = If set active display has 240 (large) scanlines. Reg0 bit1 must be set
Bit1 = If set sprites are 16x16 otherwise 8x8
Bit0 = If set sprites are zoomed (double in size)

Register 0x2:
Bit3 = Bit13 of the name base table address
Bit2 = Bit12 of the name base table address
Bit1 = Bit11 of the name base table address if resolution is "small" otherwise unused

As I mentioned earlier both the name table and the sprite info table can be moved and this is the register that sets where the name table is. To convert from this register to the name table you need to logically and this register with 0xE which will get the results of bits 3-1 (including bit 0 which is "off"), you then shift this 10 times so Bit3 aligns with Bit 13. So if bits 3-0 are 1110, then this would get shifted left 10 times to give 11100000000000 which gives the name table address of 0x3800. However it works slightly differently if you are not using the "small" resolution (meaning register 1 has bits 3 or 4 set). You need to logically and register 2 with 0xC and shift it left 10 places. You then need to logically or this with 0x700 to get the name table. For example if bits 3-0 is 1110 you and this with 0xC to give 1100, left shift 10 and logically or with 0x700 to give name table address 0x3700. 

Register 0x3 and 0x4:
Unused

Register 0x5:
Bit 6 = Bit13 of sprite info base table
Bit 5 = Bit12 of sprite info base table
Bit 4 = Bit11 of sprite info base table
Bit 3 = Bit10 of sprite info base table
Bit 2 = Bit9 of sprite info base table
Bit 1 = Bit8 of sprite info base table

This register gives the base address of the sprite attribute table. As bits 7 and 0 are ignored you need to logical and this register with binary value 01111110 (hex 0x7E) and then shift it left 7 places so the bits align

Reister 0x6:
Bit 2 = If set sprites use tiles in memory 0x2000 (tiles 256..511), else memory 0x0 (tiles 0 - 256)

Register 0x7:
Bits 3-0 = Defines the colour to use for the overscan order

Register 0x8:
The entire 8 bit register is the Background X Scrolling position (explained later)

Register 0x9:
The entire 8 bit register is the Background Y Scrolling position (explained later).

Register 0xA:
The entire 8 bit register is what the line counter should be set to (explained later)
*/

byte videoMemory[0x4000];
byte paletteMemory[32];

byte tmsRegister[0xB];

word controlWord;
byte isSecondWrite;
byte readBuffer;

byte tmsStatus;

word HCounter;
byte VCounter;
byte VCounterFirst;

byte lineInterrupt;
byte VScroll;

byte tmsIrq;

float tmsRunningCycles;

word tmsHeight;
word tmsWidth;

byte tmsIsPal;

byte ScreenDisabled;

void TMS_ResetScreen();
void TMS_WriteAddress(byte data);
void TMS_IncrementAddress();
word TMS_GetAddressRegister();
void TMS_SetRegisterData();
byte TMS_GetCodeRegister();
byte TMS_GetStatus();
void TMS_WriteDataPort(byte data);
byte TMS_ReadDataPort();
word TMS_GetSATBase();
word TMS_GetNTBase();
byte TMS_GetColorShade(byte value);
void TMS_WritePixel(byte x, byte y, byte red, byte green, byte blue);
byte TMS_GetPixelColor(byte x, byte y, int color);
byte TMS_GetVJump();
byte TMS_GetVJumpTo();
void TMS_GetOldColor(byte color, byte* red, byte* green, byte* blue);
void TMS_Init();
void TMS_Update(float nextCycle);
byte TMS_GetMode();
void TMS_Render();
void TMS_Sprite2();
void TMS_IllegalSprites2(byte sprite);
void TMS_DrawSprite2(word address, byte x, byte line, byte color);
void TMS_Sprite4();
void TMS_Background2();
void TMS_Background4();

#endif

