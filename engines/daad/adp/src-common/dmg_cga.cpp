#include <dmg.h>
#include <os_mem.h>

static bool DMG_RangesOverlap(const uint8_t* a, uint32_t aSize, const uint8_t* b, uint32_t bSize)
{
	return aSize != 0 && bSize != 0 && a < b + bSize && b < a + aSize;
}

bool DMG_UncCGAToPacked(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, int pixels, uint8_t* scratch, uint32_t scratchSize)
{
	uint32_t inputSize = ((uint32_t)width * (uint32_t)height) >> 2;
	if ((uint32_t)pixels < (((uint32_t)width * (uint32_t)height) >> 1))
	{
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		return false;
	}
	if (DMG_RangesOverlap(input, inputSize, output, (uint32_t)pixels))
	{
		if (scratch == 0 || scratchSize < inputSize ||
			DMG_RangesOverlap(scratch, inputSize, output, (uint32_t)pixels))
		{
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return false;
		}
		MemCopy(scratch, input, inputSize);
		input = scratch;
	}

	const uint8_t* end = input + inputSize;
	uint8_t* outputEnd = output + pixels;

	while (height > 0 && input < end && output < outputEnd) 
	{
		const uint8_t* src = input;
		for (int x = 0; x < width; x += 4)
		{
			uint8_t c = *src++;
			*output++ = ((c & 0xC0) >> 2) | ((c & 0x30) >> 4);
			*output++ = ((c & 0x0C) << 2) | ((c & 0x03)     );
		}
		input += width/4;
		height--;
	}
	return true;// input == end && output == outputEnd;
}

static void ProcessCGABuffer (const uint8_t* buffer, int width, int height, uint8_t* output)
{
	int x, y;
	uint32_t rowStride = (uint32_t)width >> 2;

	for (y = 0; y < height; y++)
	{
		uint32_t base = (uint32_t)y * rowStride;
		uint32_t rowOffset = ((uint32_t)y * (uint32_t)width) >> 1;
		if (y & 1)
		{
			uint8_t* ptr = output + rowOffset + (((uint32_t)width >> 1) - 1);
			for (x = 0; x < width; x += 4)
			{
				*ptr-- = ( buffer[base] & 0x03)       | ((buffer[base] & 0x0C) << 2);
				*ptr-- = ((buffer[base] & 0x30) >> 4) | ((buffer[base] & 0xC0) >> 2);
				base++;
			}
		}
		else
		{
			uint8_t* ptr = output + rowOffset;
			for (x = 0; x < width; x += 4)
			{
				*ptr++ = ((buffer[base] & 0x30) >> 4) | ((buffer[base] & 0xC0) >> 2);
				*ptr++ =  (buffer[base] & 0x03)       | ((buffer[base] & 0x0C) << 2);
				base++;
			}
		}
	}
}

bool DMG_DecompressCGA (const uint8_t* data, uint16_t dataLength, uint8_t* buffer, int width, int height)
{
	uint8_t  color;
	uint8_t  repetitions;
	uint8_t  rleColorCount = *data;

	const uint8_t* rleColors = data+1;
	const uint8_t* dataEnd = data + dataLength;
	uint32_t tempBufferSize = ((uint32_t)width * (uint32_t)height) >> 2;

	uint8_t* tempBuffer = Allocate<uint8_t>("CGA decompression buffer", tempBufferSize);
	if (tempBuffer == 0)
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}
	uint8_t* ptr = tempBuffer;
	uint8_t* end = tempBuffer + tempBufferSize;

	if (dataLength < 6 || rleColorCount > 4)
	{
		DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
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

	ProcessCGABuffer(tempBuffer, width, height, buffer);
	Free(tempBuffer);
	return true;
}