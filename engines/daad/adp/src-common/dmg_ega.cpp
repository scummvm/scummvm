#include <dmg.h>
#include <os_mem.h>

static bool DMG_RangesOverlap(const uint8_t* a, uint32_t aSize, const uint8_t* b, uint32_t bSize)
{
	return aSize != 0 && bSize != 0 && a < b + bSize && b < a + aSize;
}

bool DMG_UncEGAToPacked(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, int outputSize, uint8_t* scratch, uint32_t scratchSize)
{
	uint32_t inputSize = (uint32_t)width * (uint32_t)height / 2u;
	if ((uint32_t)outputSize < inputSize)
	{
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		return false;
	}
	if (DMG_RangesOverlap(input, inputSize, output, (uint32_t)outputSize))
	{
		if (scratch == 0 || scratchSize < inputSize ||
			DMG_RangesOverlap(scratch, inputSize, output, (uint32_t)outputSize))
		{
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return false;
		}
		MemCopy(scratch, input, inputSize);
		input = scratch;
	}

	const uint8_t* end = input + inputSize;
	uint8_t* outputEnd = output + outputSize;
	uint32_t bitPlaneStride = ((uint32_t)width * (uint32_t)height) >> 3;

	while (height > 0 && input < end && output < outputEnd) {
		for (int x = 0; x < width; x += 2)
		{
			int offset = x >> 3;
			int mask = 0x80 >> (x & 7);
			uint8_t color0 = 
				((input[offset                   ] & mask) != 0 ? 0x01 : 0) |
				((input[offset + bitPlaneStride  ] & mask) != 0 ? 0x02 : 0) |
				((input[offset + bitPlaneStride*2] & mask) != 0 ? 0x04 : 0) |
				((input[offset + bitPlaneStride*3] & mask) != 0 ? 0x08 : 0);
			mask >>= 1;
			uint8_t color1 = 
				((input[offset                   ] & mask) != 0 ? 0x01 : 0) |
				((input[offset + bitPlaneStride  ] & mask) != 0 ? 0x02 : 0) |
				((input[offset + bitPlaneStride*2] & mask) != 0 ? 0x04 : 0) |
				((input[offset + bitPlaneStride*3] & mask) != 0 ? 0x08 : 0);
			*output++ = color1 | (color0 << 4);
		}
		input += width/8;
		height--;
	}
	
	return true;// input == end && output == outputEnd;
}

static void ProcessEGABuffer (const uint8_t* buffer, int width, int height, uint8_t* output)
{
	int x, y;
	uint32_t bitPlaneStride = ((uint32_t)width * (uint32_t)height) >> 3;
	uint32_t rowStride = (uint32_t)width >> 3;

	for (y = height-1; y >= 0; y--)
	{
		uint32_t base = (uint32_t)y * rowStride;
		uint8_t outc = 0;
		bool outcm = false;

		if (y & 1)
		{
			uint8_t* ptr = output + (y * width)/2;
			for (x = width-1; x >= 0; x--)
			{
				int offset = base + (x >> 3);
				int mask = 0x1 << (x & 7);
				uint8_t color = 
					((buffer[offset                   ] & mask) != 0 ? 0x01 : 0) |
					((buffer[offset + bitPlaneStride  ] & mask) != 0 ? 0x02 : 0) |
					((buffer[offset + bitPlaneStride*2] & mask) != 0 ? 0x04 : 0) |
					((buffer[offset + bitPlaneStride*3] & mask) != 0 ? 0x08 : 0);

				if (outcm)
					*ptr++ = outc | color;
				else
					outc = color << 4;
				outcm = !outcm;
			}
		}
		else
		{
			uint8_t* ptr = output + (y * width + width)/2 - 1;
			for (x = width-1; x >= 0; x--)
			{
				int offset = base + (x >> 3);
				int mask = 0x80 >> (x & 7);
				uint8_t color = 
					((buffer[offset                   ] & mask) != 0 ? 0x01 : 0) |
					((buffer[offset + bitPlaneStride  ] & mask) != 0 ? 0x02 : 0) |
					((buffer[offset + bitPlaneStride*2] & mask) != 0 ? 0x04 : 0) |
					((buffer[offset + bitPlaneStride*3] & mask) != 0 ? 0x08 : 0);
				
				if (outcm)
					*ptr-- = outc | (color << 4);
				else
					outc = color;
				outcm = !outcm;
			}
		}
	}
}

bool DMG_DecompressEGA (const uint8_t* data, uint16_t dataLength, uint8_t* buffer, int width, int height)
{
	uint8_t  color;
	uint8_t  repetitions;
	uint8_t  rleColorCount = *data;

	const uint8_t* rleColors = data+1;
	const uint8_t* dataEnd = data + dataLength;

	uint32_t packedSize = ((uint32_t)width * (uint32_t)height) >> 1;
	uint8_t* tempBuffer = Allocate<uint8_t>("EGA decompression buffer", packedSize);
	if (tempBuffer == 0)
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}
	uint8_t* ptr = tempBuffer;
	uint8_t* end = tempBuffer + packedSize;

	if (dataLength < 6 || rleColorCount > 4)
	{
		DMG_Warning("Compressed data is invalid");
		return false;
	}
	data += 5;

	while (ptr < end && data < dataEnd)
	{
		color = *data++;
		*ptr++ = color;
		if (ptr == end) break;
		if (data == dataEnd) break;
		for (int n = 0; n < rleColorCount; n++)
		{
			if (color == rleColors[n])
			{
				repetitions = *data++;
				if (repetitions > 0)
				{
					repetitions--;
					while (repetitions-- > 0 && ptr < end)
						*ptr++ = color;
				}
				break;
			}
		}
	}

	ProcessEGABuffer(tempBuffer, width, height, buffer);
	Free(tempBuffer);
	return true;
}