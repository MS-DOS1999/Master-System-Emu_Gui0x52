#include "tms.h"

void TMS_ResetScreen()
{
	if (tmsHeight == NUM_RES_VERT_SMALL)
	{
		memset(&screenSmall, 1, sizeof(screenSmall));
	}
	else if (tmsHeight == NUM_RES_VERT_MED)
	{
		memset(&screenMedium, 1, sizeof(screenMedium));
	}
	else if (tmsHeight == NUM_RES_VERT_HIGH)
	{
		memset(&screenHigh, 1, sizeof(screenHigh));
	}
}

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

	byte code = TMS_GetCodeRegister();

	if(code == 0 || code == 1)
	{
		readBuffer = videoMemory[TMS_GetAddressRegister()];
	}

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

word TMS_GetNTBase()
{
	byte reg = tmsRegister[0x2];
	reg &= 0xF;
	BIT_ByteClear(&reg, 0);

	if(tmsHeight != NUM_RES_VERT_SMALL)
	{
		reg >>= 2;
		reg &= 0x3;

		switch(reg)
		{
			case 0: return 0x700;  break;
			case 1: return 0x1700; break;
			case 2: return 0x2700; break;
			case 3: return 0x3700; break;
		}
	}

	word res = reg << 10;
	return res;
}

byte TMS_GetColorShade(byte value)
{
	byte res = 0;
	switch(value)
	{
		case 0: res = 0;   break;
		case 1: res = 85;  break;
		case 2: res = 170; break;
		case 3: res = 255; break;
	}
	return res;
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
	return 0;
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
	return 0;
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
	return 0;
}

void TMS_GetOldColor(byte color, byte* red, byte* green, byte* blue)
{
	switch (color)
	{
		case 0x0: *red = 0;   *green = 0;   *blue = 0;   break; // transparent
		case 0x1: *red = 0;   *green = 0;   *blue = 0;   break; // black
		case 0x2: *red = 33;  *green = 200; *blue = 66;  break; // medium green
		case 0x3: *red = 94;  *green = 220; *blue = 120; break; // light green
		case 0x4: *red = 84;  *green = 85;  *blue = 237; break; // dark blue
		case 0x5: *red = 125; *green = 118; *blue = 252; break; // light blue
		case 0x6: *red = 212; *green = 82;  *blue = 77;  break; // dark red
		case 0x7: *red = 66;  *green = 235; *blue = 245; break; // cyan
		case 0x8: *red = 252; *green = 85;  *blue = 84;  break; // medium red
		case 0x9: *red = 255; *green = 121; *blue = 120; break; // light red
		case 0xA: *red = 212; *green = 193; *blue = 84;  break; // dark yellow
		case 0xB: *red = 230; *green = 206; *blue = 84;  break; // light yellow
		case 0xC: *red = 33;  *green = 176; *blue = 59;  break; // dark green
		case 0xD: *red = 201; *green = 91;  *blue = 186; break; // Magenta
		case 0xE: *red = 204; *green = 204; *blue = 204; break; // Gray
		case 0xF: *red = 255; *green = 255; *blue = 255; break; // White
	}
}

byte TMS_GetRefresh()
{
	if(refresh)
	{
		refresh = 0;
		return 1;
	}
	return 0;
}

void TMS_Init()
{
	
	isSecondWrite = 0;
	tmsStatus = 0;
	tmsIrq = 0;
	VScroll = 0;
	readBuffer = 0;
	tmsWidth = NUM_RES_HORIZONTAL;
	tmsHeight = NUM_RES_VERT_SMALL;

	memset(&videoMemory, 0, sizeof(videoMemory));
	memset(&paletteMemory, 0, sizeof(paletteMemory));
	memset(&tmsRegister, 0, sizeof(tmsRegister));

	tmsRegister[0x2] = 0xFF;
	tmsRegister[0x3] = 0xFF;
	tmsRegister[0x4] = 0x07;
	tmsRegister[0x5] = 0xFF;
	tmsRegister[0xA] = 0xFF;

	controlWord = 0;
	HCounter = 0;
	VCounter = 0;
	VCounterFirst = 1;
	lineInterrupt = 0xFF;
	ScreenDisabled = 1;
	refresh = 0;

	memset(&screenSmall, 1, sizeof(screenSmall));
	memset(&screenMedium, 1, sizeof(screenMedium));
	memset(&screenHigh, 1, sizeof(screenHigh));
}

void TMS_Update(float nextCycle)
{

	if(BIT_ByteCheck(tmsStatus, 7) && BIT_ByteCheck(tmsRegister[0x1], 5))
	{
		tmsIrq = 1;
	}
	else
	{
		tmsIrq = 0;
	}

	word hcount = HCounter;
	byte nextline = 0;
	refresh = 0;

	tmsRunningCycles += nextCycle;

	unsigned int clockInfo = floorf(tmsRunningCycles);

	//le hcounter avance à la vitesse de la machine et pas celui du tms
	int cycles = clockInfo * 2;

	if(DEBUG)
	{
		printf("hcount + cycles = %d\n\n", (hcount + cycles));
	}

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

			refresh = 1;
		}
		else if((VCounter == TMS_GetVJump()) && VCounterFirst)
		{
			VCounterFirst = 0;
			VCounter = TMS_GetVJumpTo();
		}
		else if(VCounter == tmsHeight) //vert refresh ?
		{
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
			ScreenDisabled = !BIT_ByteCheck(tmsRegister[1], 6);
			TMS_Render();
		}

		if(VCounter <= tmsHeight)
		{
			byte underflow = 0;

			if(lineInterrupt == 0)
			{
				underflow = 1;
			}

			lineInterrupt--;

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

	tmsRunningCycles -= clockInfo;
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
		TMS_Sprite2();
		TMS_Background2();
	}
	else
	{
		TMS_Sprite4();
		TMS_Background4();
	}
}

void TMS_Sprite2()
{
	word satbase = TMS_GetSATBase();
	word sgtable = tmsRegister[0x6] & 7;
	sgtable <<= 11;

	byte size = BIT_ByteCheck(tmsRegister[0x1], 1) ? 16 : 8;

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

			spriteCount++;

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
				word address = sgtable + ((pattern & 252) * 8);
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
	for(int i = 0; i < 8; i++, invert--)
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

void TMS_Sprite4()
{

	int spriteCount = 0;
	word satbase = TMS_GetSATBase();

	byte is8x16 = 0;
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

			for(int i = 0; i < 8; i++, col--)
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

				if(palette == 0)
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

void TMS_Background2()
{
	// reg 3 contains colour table info
	// reg 4 contains the pattern table info
	// if bit 2 is set of reg 4 the pattern table address starts at 0x2000 otherwise 0x0
	// if bit 7 is set of reg 3 the colour table address starts at 0x2000 otherwise 0x0
	// bits 0 to 6 of reg 3 get anded over bits 1-7 of the character number
	// bit 0-6 get anded over bit 1-7 of character number so shift left 1

	word ntBase = TMS_GetNTBase();

	word ptaBase = BIT_ByteCheck(tmsRegister[0x4], 2) ? 0x2000 : 0x0;

	word ctaBase = BIT_ByteCheck(tmsRegister[0x3], 7) ? 0x2000 : 0x0;

	byte ctaAND = tmsRegister[0x3] & 127;

	ctaAND <<= 1;

	BIT_ByteSet(&ctaAND, 0);

	int row = VCounter / 8;

	int patternTable = 0;

	if(row > 7)
	{
		if(row > 15)
		{
			if(BIT_ByteCheck(tmsRegister[0x4], 1))
			{
				patternTable = 2;
			}
		}
		else
		{
			if(BIT_ByteCheck(tmsRegister[0x4], 0))
			{
				patternTable = 1;
			}
		}
	}

	word patternTableOffset = 0;

	if(patternTable == 1)
	{
		patternTableOffset = 256 * 8;
	}
	else if(patternTable == 2)
	{
		patternTableOffset = 256 * 2 * 8;
	}

	int line = VCounter % 8;

	for(int column = 0; column < 32; column++)
	{
		word ntBaseCopy = ntBase + ((row * 32) + column);

		byte pattern = videoMemory[ntBaseCopy];
		word ptAddress = ptaBase + (pattern * 8);

		ptAddress += patternTableOffset;
		ptAddress += line;

		byte pixelLine = videoMemory[ptAddress];
		byte colIndex = pattern & ctaAND;

		byte color = videoMemory[ctaBase + (colIndex * 8) + patternTableOffset + line];
		byte fore = color >> 4;
		byte back = color & 0xF;

		int invert = 7;
		for(int x = 0; x < 8; x++, invert--)
		{
			byte colNum = BIT_ByteCheck(pixelLine, invert) ? fore : back;

			if(colNum == 0)
			{
				continue;
			}

			byte red = 0;
			byte green = 0;
			byte blue = 0;

			TMS_GetOldColor(colNum, &red, &green, &blue);

			int xpos = (column * 8) + x;

			if(TMS_GetPixelColor(xpos, VCounter, 0) != 1)
			{
				continue;
			}

			if(xpos >= NUM_RES_HORIZONTAL)
			{
				continue;
			}

			TMS_WritePixel(xpos, VCounter, red, green, blue);
		}
	}
}

void TMS_Background4()
{

	word ntBase = TMS_GetNTBase();
	byte HScroll = tmsRegister[0x8];

	byte vStartingRow = VScroll >> 3;
	byte vFineScroll = VScroll & 0x7;
	byte hStartingCol = HScroll >> 3;
	byte hFineScroll = HScroll & 0x7;

	byte limitVScroll = BIT_ByteCheck(tmsRegister[0x0], 7);
	byte limitHScroll = BIT_ByteCheck(tmsRegister[0x0], 6);

	int row = VCounter;
	row /= 8;

	byte maskFirstColumn = BIT_ByteCheck(tmsRegister[0x0], 5);

	for(int column = 0; column < 32; column++)
	{
		int invert = 7;

		for(int x = 0; x < 8; x++, invert--)
		{
			int xpixel = x;

			byte allowHScroll = ((row > 1) || !limitHScroll) ? 1 : 0;
			
			int i = x;

			i += column * 8;

			int xpos = i;

			if(allowHScroll)
			{
				xpos = hStartingCol;
				xpos *= 8;
				xpos += xpixel + hFineScroll;
				xpos = xpos % tmsWidth;
			}

			byte allowVScroll = (((xpos/8) > 23) && limitVScroll) ? 0 : 1;

			int vrow = row;

			if(allowVScroll)
			{
				vrow += vStartingRow;

				int bumpRow = VCounter % 8;

				if((bumpRow + vFineScroll) > 7)
				{
					vrow++;
				}

				int mod = (tmsHeight == NUM_RES_VERT_SMALL) ? 28 : 32;

				vrow = vrow % mod;
			}

			int col = column;

			word ntBaseOffset = ntBase;
			ntBaseOffset += vrow * 64;
			ntBaseOffset += col * 2;

			word tileData = videoMemory[ntBaseOffset+1] << 8;
			tileData |= videoMemory[ntBaseOffset];

			byte hiPriority = BIT_WordCheck(tileData, 12);
			byte useSpritePalette = BIT_WordCheck(tileData, 11);
			byte vertFlip = BIT_WordCheck(tileData, 10);
			byte horzFlip = BIT_WordCheck(tileData, 9);
			word tileDefinition = tileData & 0x1FF;

			int offset = VCounter;

			if(allowVScroll)
			{
				offset += VScroll;
			}

			offset = offset % 8;

			if(vertFlip)
			{
				offset *= -1;
				offset += 7;
			}

			tileDefinition *= 32;
			tileDefinition += 4 * offset;

			byte data1 = videoMemory[tileDefinition];
			byte data2 = videoMemory[tileDefinition+1];
			byte data3 = videoMemory[tileDefinition+2];
			byte data4 = videoMemory[tileDefinition+3];

			int colorbit = invert;

			if(horzFlip)
			{
				colorbit = x;
			}

			byte palette = 0;
			byte bit = BIT_ByteCheck(data4, colorbit);
			palette = (bit << 3);
			bit = BIT_ByteCheck(data3, colorbit);
			palette |= (bit << 2);
			bit = BIT_ByteCheck(data2, colorbit);
			palette |= (bit << 1);
			bit = BIT_ByteCheck(data1, colorbit);
			palette |= bit;

			byte masking = 0;

			if((xpos < 8) && maskFirstColumn)
			{
				masking = 1;
				palette = tmsRegister[0x7] & 15;
				useSpritePalette = 1;
			}

			if(palette == 0)
			{
				hiPriority = 0;
			}

			if(useSpritePalette)
			{
				palette += 16;
			}

			byte color = paletteMemory[palette];

			byte red = color & 0x3;
	   		color >>= 2;
	   		byte green = color & 0x3;
	   		color >>= 2;
	   		byte blue = color & 0x3;
	   		
	   		if(!masking && !hiPriority && (TMS_GetPixelColor(xpos, VCounter, 0) != 1))
	   		{	
	   			continue;
	   		}

	   		if(xpos >= NUM_RES_HORIZONTAL)
	   		{
	   			continue;
	   		}

	   		TMS_WritePixel(xpos, VCounter, TMS_GetColorShade(red), TMS_GetColorShade(green), TMS_GetColorShade(blue));
		}
		hStartingCol = (hStartingCol + 1) % 32;
	}
}