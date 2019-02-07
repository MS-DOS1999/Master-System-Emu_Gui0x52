#ifndef SN_H
#define SN_H

#include <SDL/SDL.h>
#include <math.h>
#include "oldSize.h"
#include "bitUtils.h"

#define BUFFERSIZE 1024

#define FREQUENCY 44100

class SN
{

public:
	SN();
	void SN_Reset();
	void SN_WriteData(unsigned long int cycles, byte data);
	void SN_Update(float cyclesMac);

private:
	typedef enum
	{
		CHANNEL_ZERO,
		CHANNEL_ONE,
		CHANNEL_TWO,
		CHANNEL_THREE,
		CHANNEL_NUM
	}CHANNEL;

	typedef enum
	{
		TONES_ZERO,
		TONES_ONE,
		TONES_TWO,
		TONES_NOISE,
		TONES_NUM
	}TONES;

	typedef enum 
	{
		VOLUME_ZERO,
		VOLUME_ONE,
		VOLUME_TWO,
		VOLUME_THREE,
		VOLUME_NUM
	}VOLUME;

	signed short int buffer[BUFFERSIZE];
	word tones[TONES_NUM];
	byte volume[VOLUME_NUM];
	int counters[CHANNEL_NUM];
	int polarity[CHANNEL_NUM];
	CHANNEL latchedChannel;
	byte isToneLatched;
	int volumeTable[16];
	int currentBufferPos;
	float cycles;
	word LFSR;
	float bufferUpdateCount;
	float updateBufferLimit;

	static void SN_HandleSDLCallback(void* userData, Uint8* altbuffer, int len);
	void SN_HandleSDLCallback(Uint8* altbuffer, int len);
	int SN_Parity(byte data);
};

#endif