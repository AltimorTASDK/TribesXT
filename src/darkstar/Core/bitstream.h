#pragma once

#include "darkstar/Core/streamio.h"

class BitStream : public StreamIO {
	uint8_t *dataPtr;
	int bitNum;
	int bufSize;
	bool error;
	int maxReadBitNum;
	int maxWriteBitNum;

public:
	void writeInt(int value, int bitCount);
	int readInt(int bitCount);

	void writeSignedInt(int value, int bitCount);
	int readSignedInt(int bitCount);

	// read and write floats... floats are 0 to 1 inclusive, signed floats are -1 to 1 inclusive

	float readFloat(int bitCount);
	float readSignedFloat(int bitCount);

	void writeFloat(float f, int bitCount);
	void writeSignedFloat(float f, int bitCount);

	void writeBits(int bitCount, const void *bitPtr);
	void readBits(int bitCount, void *bitPtr);
	bool writeFlag(bool val);
	bool readFlag();
};

inline bool BitStream::readFlag()
{
	if(bitNum > maxReadBitNum)
	{
		error = true;
		return false;
	}
	int mask = 1 << (bitNum & 0x7);
	bool ret = (*(dataPtr + (bitNum >> 3)) & mask) != 0;
	bitNum++;
	return ret;
}

inline bool BitStream::writeFlag(bool val)
{
	if(bitNum + 1 > maxWriteBitNum)
	{
		error = true;
		return false;
	}
	if(val)
		*(dataPtr + (bitNum >> 3)) |= (1 << (bitNum & 0x7));
	else
		*(dataPtr + (bitNum >> 3)) &= ~(1 << (bitNum & 0x7));
	bitNum++;
	return (val);
}

inline void BitStream::writeBits(int bitCount, const void *bitPtr)
{
	if(!bitCount)
		return;

	if(bitCount + bitNum > maxWriteBitNum)
	{
		error = true;
		return;
	}
	const uint8_t *ptr = (uint8_t *) bitPtr;
	uint8_t *stPtr = dataPtr + (bitNum >> 3);
	uint8_t *endPtr = dataPtr + ((bitCount + bitNum - 1) >> 3);

	int upShift = bitNum & 0x7;
	int downShift = 8 - upShift;
	uint8_t lastMask = 0xFF >> (7 - ((bitNum + bitCount - 1) & 0x7));
	uint8_t startMask = 0xFF >> downShift;

	uint8_t curB = *ptr++;
	*stPtr = (curB << upShift) | (*stPtr & startMask);

	stPtr++;
	while(stPtr <= endPtr)
	{
		uint8_t nextB = *ptr++;
		*stPtr++ = (curB >> downShift) | (nextB << upShift);
		curB = nextB;
	}
	*endPtr &= lastMask;

	bitNum += bitCount;
}

inline void BitStream::readBits(int bitCount, void *bitPtr)
{
	if(!bitCount)
		return;
	if(bitCount + bitNum > maxReadBitNum)
	{
		error = true;
		return;
	}
	uint8_t *stPtr = dataPtr + (bitNum >> 3);
	int byteCount = (bitCount + 7) >> 3;

	uint8_t *ptr = (uint8_t *) bitPtr;

	int downShift = bitNum & 0x7;
	int upShift = 8 - downShift;

	uint8_t curB = *stPtr;
	while(byteCount--)
	{
		uint8_t nextB = *++stPtr;
		*ptr++ = (curB >> downShift) | (nextB << upShift);
		curB = nextB;
	}

	bitNum += bitCount;
}

inline int BitStream::readInt(int bitCount)
{
	int ret = 0;
	readBits(bitCount, &ret);
	if(bitCount == 32)
		return ret;
	else
		ret &= (1 << bitCount) - 1;
	return ret;
}

inline void BitStream::writeInt(int val, int bitCount)
{
	writeBits(bitCount, &val);
}

inline void BitStream::writeFloat(float f, int bitCount)
{
	writeInt((int)(f * ((1 << bitCount) - 1)), bitCount);
}

inline float BitStream::readFloat(int bitCount)
{
	return readInt(bitCount) / float((1 << bitCount) - 1);
}

inline void BitStream::writeSignedFloat(float f, int bitCount)
{
	writeInt((int)(((f + 1) * .5f) * ((1 << bitCount) - 1)), bitCount);
}

inline float BitStream::readSignedFloat(int bitCount)
{
	return readInt(bitCount) * 2 / float((1 << bitCount) - 1) - 1.0f;
}

inline void BitStream::writeSignedInt(int value, int bitCount)
{
	if(writeFlag(value < 0))
		writeInt(-value, bitCount - 1);
	else
		writeInt(value, bitCount - 1);
}

inline int BitStream::readSignedInt(int bitCount)
{
	if(readFlag())
		return -readInt(bitCount - 1);
	else
		return readInt(bitCount - 1);
}
