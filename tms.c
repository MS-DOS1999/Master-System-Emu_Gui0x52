#include "tms.h"

void TMS_WriteAddress(byte data)
{
	if(isSecondWrite) //on update le hi
	{
		controlWord &= 0xFF;
		controlWord |= data << 8;
		isSecondWrite = 0;

		byte codeReg = TMS_GetCodeRegister();

		if(codeReg == 0)
		{
			readBuffer = videoMemory[TMS_GetAddressRegister()];
			TMS_IncrementAddress();
		}
		else if(codeReg == 2)
		{
			TMS_SetRegisterData();
		}
	}
	else //on update le lo
	{
		isSecondWrite = 1;
		controlWord &= 0xFF00;
		controlWord |= data;
	}
}

void TMS_IncrementAddress()
{
	if(TMS_GetAddressRegister() == 0x3FFF)
	{
		controlWord &= 0xC000; //on limite l'adresse
	}
	else
	{
		controlWord++;
	}
}

word TMS_GetAddressRegister()
{
	return controlWord & 0x3FFF;
}

void TMS_SetRegisterData()
{
	//le data du reg est le lo byte
	byte data = controlWord & 0xFF;

	//le reg est les 4bit lo du hi byte
	byte reg = controlWord >> 8;
	reg &= 0xF;

	if(reg > 0xA)
	{
		return;
	}

	tmsRegister[reg] = data;

	//si le reg active la vsync, alors on a une irq
	if(reg == 1)
	{
		if(BIT_ByteCheck(tmsStatus, 7) && BIT_ByteCheck(tmsRegister[0x1], 5))
		{
			tmsIrq = 1;
		}
	}
}

byte TMS_GetCodeRegister()
{
	word codeReg = controlWord >> 14;
	return (byte)codeReg;
}

byte TMS_GetStatus()
{
	byte res = tmsStatus;

	if(TMS_GetMode() == 2)
	{
		tmsStatus &= 0x2F; //turn off bits 7 et 5
	}
	else
	{
		tmsStatus &= 0x1F; //turn off top 3 bits
	}

	isSecondWrite = 0;
	tmsIrq = 0;

	return res;
}

void TMS_WriteDataPort(byte data)
{
	isSecondWrite = 0;
	byte code = TMS_GetCodeRegister();

	if(code == 0 || code == 1 || code == 2)
	{
		videoMemory[TMS_GetAddressRegister()] = data;
	}
	else if(code == 3)
	{
		paletteMemory[TMS_GetAddressRegister() & 31] = data;
	}

	readBuffer = data;

	TMS_IncrementAddress();
}

byte TMS_ReadDataPort()
{
	isSecondWrite = 0;

	byte res = readBuffer;

	readBuffer = videoMemory[TMS_GetAddressRegister()];

	TMS_IncrementAddress();

	return res;
}

word TMS_GetSATBase()
{
	byte reg = tmsRegister[0x5];

	BIT_ByteClear(&reg, 7);
	BIT_ByteClear(&reg, 0);

	word res = reg << 7;
	return res;
}

byte TMS_GetColorShade(byte value)
{
	switch(value)
	{
		case 0: return 0;   break;
		case 1: return 85;  break;
		case 2: return 170; break;
		case 3: return 255; break;
	}
}

void TMS_WritePixel(byte x, byte y, byte red, byte green, byte blue)
{
	if(tmsHeight == NUM_RES_VERT_SMALL)
	{
		screenSmall[x][y][0] = red;
		screenSmall[x][y][1] = green;
		screenSmall[x][y][2] = blue;
	}
	else if(tmsHeight == NUM_RES_VERT_MED)
	{
		screenMedium[x][y][0] = red;
		screenMedium[x][y][1] = green;
		screenMedium[x][y][2] = blue;
	}
	else if(tmsHeight == NUM_RES_VERT_HIGH)
	{
		screenHigh[x][y][0] = red;
		screenHigh[x][y][1] = green;
		screenHigh[x][y][2] = blue;
	}
}

byte TMS_GetPixelColor(byte x, byte y, int color)
{
	if(tmsHeight == NUM_RES_VERT_SMALL)
	{
		return screenSmall[x][y][color];
	}
	else if(tmsHeight == NUM_RES_VERT_MED)
	{
		return screenMedium[x][y][color];
	}
	else if(tmsHeight == NUM_RES_VERT_HIGH)
	{
		return screenHigh[x][y][color];
	}
}

byte TMS_GetVJump()
{
	if(tmsIsPal)
	{
		if(tmsHeight == NUM_RES_VERT_SMALL)
		{
			return 0xF2;
		}
		else if(tmsHeight == NUM_RES_VERT_MED)
		{
			return 0xFF;
		}
		else if(tmsHeight == NUM_RES_VERT_HIGH)
		{
			return 0xFF;
		}
	}
	else
	{
		if(tmsHeight == NUM_RES_VERT_SMALL)
		{
			return 0xDA;
		}
		else if(tmsHeight == NUM_RES_VERT_MED)
		{
			return 0xEA;
		}
		else if(tmsHeight == NUM_RES_VERT_HIGH)
		{
			return 0xFF;
		}
	}
}

byte TMS_GetVJumpTo()
{
	if(tmsIsPal)
	{
		if(tmsHeight == NUM_RES_VERT_SMALL)
		{
			return 0xBA;
		}
		else if(tmsHeight == NUM_RES_VERT_MED)
		{
			return 0xC7;
		}
		else if(tmsHeight == NUM_RES_VERT_HIGH)
		{
			return 0xC1;
		}
	}
	else
	{
		if(tmsHeight == NUM_RES_VERT_SMALL)
		{
			return 0xD5;
		}
		else if(tmsHeight == NUM_RES_VERT_MED)
		{
			return 0xE5;
		}
		else if(tmsHeight == NUM_RES_VERT_HIGH)
		{
			return 0xFF;
		}
	}
}

void TMS_GetOldColor(byte color, byte* red, byte* green, byte* blue)
{
	switch (color)
	{
		case 0:   *red = 0;   *green = 0;   *blue = 0;   break; // transparent
		case 1:   *red = 0;   *green = 0;   *blue = 0;   break; // black
		case 2:   *red = 33;  *green = 200; *blue = 66;  break; // medium green
		case 3:   *red = 94;  *green = 220; *blue = 120; break; // light green
		case 4:   *red = 84;  *green = 85;  *blue = 237; break; // dark blue
		case 5:   *red = 125; *green = 118; *blue = 252; break; // light blue
		case 6:   *red = 212; *green = 82;  *blue = 77;  break; // dark red
		case 7:   *red = 66;  *green = 235; *blue = 245; break; // cyan
		case 8:   *red = 252; *green = 85;  *blue = 84;  break; // medium red
		case 9:   *red = 255; *green = 121; *blue = 120; break; // light red
		case 0xA: *red = 212; *green = 193; *blue = 84;  break; // dark yellow
		case 0xB: *red = 230; *green = 206; *blue = 84;  break; // light yellow
		case 0xC: *red = 33;  *green = 176; *blue = 59;  break; // dark green
		case 0xD: *red = 201; *green = 91;  *blue = 186; break; // Magenta
		case 0xE: *red = 204; *green = 204; *blue = 204; break; // Gray
		case 0xF: *red = 255; *green = 255; *blue = 255; break; // White
	}
}

void TMS_Update(float nextCycle)
{
	tmsIrq = 0;
	word hcount = HCounter;
	byte nextline = 0;
	isVblank = 0;
	tmsRefresh = 0;

	tmsRunningCycles += nextCycle;

	unsigned int clockInfo = floorf(tmsRunningCycles);

	//le hcounter avance à la vitesse de la machine et pas celui du tms
	int cycles = clockInfo * 2;

	if((hcount + cycles) > 684)
	{
		nextline = 1;
	}

	HCounter = (HCounter + cycles) % 685;

	if(nextline)
	{
		byte vcount = VCounter;
		VCounter++;

		if(vcount == 255)
		{
			VCounter = 0;
			VCounterFirst = 1;
			TMS_Render();
			tmsRefresh = 1;
		}
		else if((vcount == TMS_GetVJump()) && VCounterFirst)
		{
			VCounterFirst = 0;
			VCounter = TMS_GetVJumpTo();
		}
		else if(VCounter == tmsHeight) //vert refresh ?
		{
			isVblank = 1;
			BIT_ByteSet(&tmsStatus, 7); //irq pending
		}

		if(VCounter >= tmsHeight)
		{
			if(VCounter != tmsHeight)
			{
				lineInterrupt = tmsRegister[0xA];
			}

			//on update le VScroll
			VScroll = tmsRegister[0x9];
			byte mode = TMS_GetMode();

			//on check si la res change
			if(mode == 11)
			{
				tmsHeight = NUM_RES_VERT_MED;
			}
			else if(mode == 14)
			{
				tmsHeight = NUM_RES_VERT_HIGH;
			}
			else
			{
				tmsHeight = NUM_RES_VERT_SMALL;
			}
		}

		if(VCounter < tmsHeight)
		{
			screenDisabled = !BIT_ByteCheck(tmsRegister[0x1], 6);
			TMS_Render();
		}

		if(VCounter <= tmsHeight)
		{
			byte underflow = 0;

			if(lineInterrupt == 0)
			{
				underflow = 1;
			}

			if(underflow)
			{
				lineInterrupt = tmsRegister[0xA];
				if(BIT_ByteCheck(tmsRegister[0x0], 4))
				{
					tmsIrq = 1;
				}
			}
		}
	}

	if(BIT_ByteCheck(tmsStatus, 7) && BIT_ByteCheck(tmsRegister[0x1], 5))
	{
		tmsIrq = 1;
	}
}

byte TMS_GetMode()
{
	byte res = 0;
	res |= BIT_ByteCheck(tmsRegister[0x0], 2) << 3;
	res |= BIT_ByteCheck(tmsRegister[0x1], 3) << 2;
	res |= BIT_ByteCheck(tmsRegister[0x0], 1) << 1;
	res |= BIT_ByteCheck(tmsRegister[0x1], 4);
	return res;
}

void TMS_Render()
{
	byte mode = TMS_GetMode();

	if(mode == 2)
	{
		TMS_Sprite2():
		TMS_Background2();
	}
	else
	{
		TMS_Sprite4():
		TMS_Background4();
	}
}

void TMS_Sprite2()
{
	word satbase = TMS_GetSATBase();
	word sgtable = tmsRegister[0x6] & 7;
	sgtable <<= 11;

	byte size = BIT_ByteCheck(tmsRegister[0x1], 1) ? 16 : 8;
	byte isZoomed = BIT_ByteCheck(tmsRegister[0x1], 0);

	int spriteCount = 0;

	for(int sprite = 0; sprite < 32; sprite++)
	{
		word location = satbase + (sprite * 4);

		int y = videoMemory[location];

		if(y == 0xD0) //on ne dessine pas les sprites
		{
			if(!BIT_ByteCheck(tmsStatus, 6))
			{
				tmsStatus &= 0xE0;
				tmsStatus |= sprite;
			}
			return;
		}

		if(y > 0xD0)
		{
			y -= 0x100;
		}

		y++;

		if((VCounter >= y) && (VCounter < (y+size)))
		{
			byte x = videoMemory[location+1];
			byte pattern = videoMemory[location+2];
			byte color = videoMemory[location+3];
			byte ec = BIT_ByteCheck(color, 7);

			if(ec)
			{
				x-=32;
			}

			color &= 0xF;

			if(color == 0)
			{
				continue;
			}

			spriteCount++:

			if(spriteCount > 4)
			{
				TMS_IllegalSprites2(sprite);
				return;
			}
			else
			{
				BIT_ByteClear(&tmsStatus, 6);
			}

			int line = VCounter - y;

			if(size == 8)
			{
				TMS_DrawSprite2(sgtable + (pattern * 8), x, line, color);
			}
			else
			{
				word address = sgtable + ((pattern & 252) * 8)
				TMS_DrawSprite2(address, x, line, color);
				TMS_DrawSprite2(address, x+8, line+16, color);
			}
		}
	}

	tmsStatus &= 0xE0;
	tmsStatus |= 31;
}

void TMS_IllegalSprites2(byte sprite)
{
	BIT_ByteSet(&tmsStatus, 6);
	tmsStatus &= 0xE0;
	tmsStatus |= sprite;
}

void TMS_DrawSprite2(word address, byte x, byte line, byte color)
{
	byte red = 0;
	byte green = 0;
	byte blue = 0;

	TMS_GetOldColor(color, &red, &green, &blue);
	byte invert = 7;
	for(int i = 0; i < 8; i++; invert--)
	{
		byte drawLine = videoMemory[address + line];
		byte xpos = x + i;

		//on check la collision du sprite
		if(TMS_GetPixelColor(xpos, VCounter, 0) != 1)
		{
			BIT_ByteSet(&tmsStatus, 5);
			continue;
		}

		if(!BIT_ByteCheck(drawLine, invert))
		{
			continue;
		}

		TMS_WritePixel(xpos, VCounter, red, green, blue);
	}
}

void Sprite4()
{
	int spriteCount = 0;
	word satbase = TMS_GetSATBase();

	byte is8x16 = 0;
	byte isZoomed = 0;
	byte size = 8;

	byte shiftX = BIT_ByteCheck(tmsRegister[0x0], 3);
	byte useSecondPattern = BIT_ByteCheck(tmsRegister[0x6], 2);

	if(BIT_ByteCheck(tmsRegister[0x1], 1))
	{
		is8x16 = 1;
		size = 16;
	}

	if(BIT_ByteCheck(tmsRegister[0x1], 0))
	{
		isZoomed = 1;
		size = 16;
	}

	for(int sprite = 0; sprite < 64; sprite++)
	{
		int y = videoMemory[satbase + sprite];

		if((tmsHeight == NUM_RES_VERT_SMALL) && (y == 0xD0))
		{
			break;
		}

		if(y > 0xD0)
		{
			y -= 0x100;
		}

		y++;

		if((VCounter >= y) && (VCounter < (y+size))) 
		{
			spriteCount++;
			int x = videoMemory[satbase + 128 + (sprite * 2)];
			word tileNumber = videoMemory[satbase + 129 + (sprite * 2)];

			if(spriteCount > 8)
			{
				BIT_ByteSet(&tmsStatus, 6);
				break;
			}

			if(shiftX)
			{
				x -= 8;
			}

			if(useSecondPattern)
			{
				tileNumber += 256;
			}

			if(is8x16)
			{
				if(y < (VCounter + 9))
				{
					BIT_WordClear(&tileNumber, 0);
				}
			}

			int startAddress = tileNumber * 32;
			startAddress += (4 * (VCounter - y));

			byte data1 = videoMemory[startAddress];
			byte data2 = videoMemory[startAddress + 1];
			byte data3 = videoMemory[startAddress + 2];
			byte data4 = videoMemory[startAddress + 3];

			int col = 7;

			for(int i = 0; i < 8; i++; col--;)
			{
				if((x+i) >= NUM_RES_HORIZONTAL)
				{
					continue;
				}

				if(TMS_GetPixelColor(x+i, VCounter, 0) != 1)
				{
					BIT_ByteSet(&tmsStatus, 5);
					continue;
				}

				byte palette = 0;
				byte bit = BIT_ByteCheck(data4, col);
				palette = (bit << 3);
				bit = BIT_ByteCheck(data3, col);
				palette |= (bit << 2);
				bit = BIT_ByteCheck(data2, col);
				palette |= (bit << 1);
				bit = BIT_ByteCheck(data1, col);
				palette |= bit;

				if(palette = 0)
				{
					continue;
				}

				byte color = paletteMemory[palette+16];

				byte red = color & 0x3;
				color >>= 2;
				byte green = color & 0x3;
				color >>= 2;
				byte blue = color & 0x3;

				TMS_WritePixel(x+i, VCounter, TMS_GetColorShade(red), TMS_GetColorShade(green), TMS_GetColorShade(blue));
			}
		}
	}
}