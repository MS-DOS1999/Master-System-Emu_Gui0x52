#include "sn.h"

SN::SN()
{

	const int maxVolume = 8000;
	const double TwodBScalingFactor = 0.79432823;

	double vol = maxVolume;

	for(int i = 0; i < 15; i++)
	{
		volumeTable[i] = (int)vol;
		vol *= TwodBScalingFactor;
	}

	volumeTable[15] = 0;

	SN_Reset();
	SDL_PauseAudio(1);

	const float sdlCallBackFreq = (FREQUENCY / BUFFERSIZE) + 1;

	const float clockSpeed = 220000;

	updateBufferLimit = clockSpeed / sdlCallBackFreq;
	updateBufferLimit /= BUFFERSIZE;
}

void SN::SN_Reset()
{
	bufferUpdateCount = 0;
	memset(buffer, 0, sizeof(buffer));
	memset(tones, 0, sizeof(tones));
	memset(counters, 0, sizeof(counters));

	for(int p = 0; p < 4; p++)
	{
		volume[p] = 0xF;
		polarity[p] = 1;
	}

	latchedChannel = CHANNEL_ZERO;
	isToneLatched = 1;
	currentBufferPos = 0;
	cycles = 0;
	LFSR = 0x8000;

	SDL_CloseAudio();

	SDL_AudioSpec as;
	as.freq = FREQUENCY;
	as.format = AUDIO_S16SYS;
	as.channels = 1;
	as.silence = 0;
	as.samples = BUFFERSIZE;
	as.size = 0;
	as.callback = SN_HandleSDLCallback;
	as.userdata = this;
	SDL_OpenAudio(&as, 0);
	
	SDL_PauseAudio(0);
}

void SN::SN_WriteData(unsigned long int cycles, byte data)
{

	if(BIT_ByteCheck(data, 7))
	{

		int channel = data;
		channel >>= 5;

		channel &= 0x3;
		latchedChannel = (CHANNEL)channel;

		isToneLatched = BIT_ByteCheck(data, 4) ? 0 : 1;

		byte channelData = data & 0xF;

		if(isToneLatched)
		{
			if(latchedChannel == TONES_NOISE)
			{
				tones[TONES_NOISE] = channelData;
				LFSR = 0x8000;
			}
			else
			{
				word currentValue = tones[latchedChannel];

				currentValue &= 0xFFF0;

				currentValue |= channelData;

				tones[latchedChannel] = currentValue;
			}
		}
		else
		{
			byte currentValue = volume[latchedChannel];

			currentValue &= 0xF0;

			currentValue |= channelData;

			volume[latchedChannel] = currentValue;
		}
	}
	else
	{
		word channelData = 0;

		channelData = data & 0xF3;

		if(isToneLatched)
		{
			if(latchedChannel == TONES_NOISE)
			{
				tones[TONES_NOISE] = data & 0xF;
				LFSR = 0x8000;
			}
			else
			{
				word currentValue = tones[latchedChannel];
				byte currentLowNibble = currentValue & 0xF;

				channelData <<= 4;

				channelData |= currentLowNibble;
				tones[latchedChannel] = channelData;
			}
		}
		else
		{
			volume[latchedChannel] = data & 0xF;
		}
	}
}

void SN::SN_HandleSDLCallback(void* userData, Uint8* altbuffer, int len)
{
	SN* data = (SN*)userData;
	data->SN_HandleSDLCallback(altbuffer, len);
}

void SN::SN_HandleSDLCallback(Uint8* altbuffer, int len)
{
	memcpy(altbuffer, buffer, len);
	currentBufferPos = 0;
}

int SN::SN_Parity(byte data)
{
	int bitCount = 0;
	for(int i = 0; i < 4; i++)
	{
		if(BIT_ByteCheck(data, i))
		{
			bitCount++;
		}
	}

	if((bitCount % 2) == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}

}

void SN::SN_Update(float cyclesMac)
{
	const int sampleRate = 16;
	cyclesMac /= sampleRate;

	cycles += cyclesMac;

	float floor = floorf(cycles);

	cycles -= floor;

	bufferUpdateCount += floor;

	signed short int tone = 0;

	for(int i = 0; i < CHANNEL_THREE; i++)
	{
		if(tones[i] == 0)
		{
			continue;
		}

		counters[i] -= floor;

		if(counters[i] <= 0)
		{
			counters[i] = tones[i];
			polarity[i] *= -1;
		}

		tone += volumeTable[volume[i]] * polarity[i];
	}

	if(tones[TONES_NOISE] != 0)
	{
		counters[TONES_NOISE] -= floor;

		if(counters[TONES_NOISE] <= 0)
		{
			word freq = tones[TONES_NOISE];
			freq &= 0x3;

			int count = 0;
			switch(freq)
			{
				case 0:
				count = 0x10;
				break;

				case 1:
				count = 0x20;
				break;

				case 2:
				count = 0x40;
				break;

				case 3:
				count = tones[CHANNEL_TWO];
				break;
			}

			counters[TONES_NOISE] = count;
			polarity[TONES_NOISE] *= -1;

			if(polarity[TONES_NOISE] == 1)
			{
				byte isWhiteNoise = BIT_WordCheck(tones[TONES_NOISE], 2);
				byte tappedBits = BIT_WordCheck(tones[TONES_NOISE], 0);
				tappedBits |= (BIT_WordCheck(tones[TONES_NOISE], 3) << 3);

				LFSR = (LFSR >> 1) | ((isWhiteNoise ? SN_Parity(LFSR & tappedBits) : LFSR & 1) << 15);
			}
		}

		tone += volumeTable[volume[TONES_NOISE]] * (LFSR & 1);
	}

	if(bufferUpdateCount >= updateBufferLimit)
	{
		if(currentBufferPos < BUFFERSIZE)
		{
			buffer[currentBufferPos] = tone;
		}

		currentBufferPos++;
		bufferUpdateCount = updateBufferLimit - bufferUpdateCount;
	}
	
}