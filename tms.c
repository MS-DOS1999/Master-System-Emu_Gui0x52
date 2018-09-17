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

byte TMS_ReadDataPort()
{
	isSecondWrite = 0;

	byte res = readBuffer;

	readBuffer = videoMemory[TMS_GetAddressRegister()];

	TMS_IncrementAddress();

	return res;
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
		else if((vcount == GetVJump()) && VCounterFirst)
		{
			VCounterFirst = 0;
			VCounter = GetVJumpTo();
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