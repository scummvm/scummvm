#include <dmg.h>
#include <ddb.h>
#include <ddb_pal.h>
#include <os_mem.h>
#include <os_lib.h>
#include <os_bito.h>

static void DMG_ExpandPCWByteToPacked(uint8_t value, uint8_t* output)
{
	output[0] = (value & 0x80 ? 0xF0 : 0x00) | (value & 0x40 ? 0x0F : 0x00);
	output[1] = (value & 0x20 ? 0xF0 : 0x00) | (value & 0x10 ? 0x0F : 0x00);
	output[2] = (value & 0x08 ? 0xF0 : 0x00) | (value & 0x04 ? 0x0F : 0x00);
	output[3] = (value & 0x02 ? 0xF0 : 0x00) | (value & 0x01 ? 0x0F : 0x00);
}

static bool DMG_RearrangePCWDecodedToStoredLayout(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, uint32_t outputSize)
{
	uint32_t rowBytes = ((uint32_t)width + 7) >> 3;
	uint32_t monoSize = rowBytes * height;
	if (outputSize < monoSize || (height & 1) != 0)
		return false;

	MemClear(output, monoSize);
	for (uint32_t pair = 0; pair < height / 2; pair++)
	{
		uint32_t sourceTop = pair * rowBytes * 2;
		uint32_t sourceBottom = sourceTop + rowBytes * 2 - 1;
		uint32_t destBase = (pair >> 2) * width + (pair & 3) * 2;
		for (uint32_t xByte = 0; xByte < rowBytes; xByte++)
		{
			uint32_t dest = destBase + (xByte << 3);
			output[dest + 0] = input[sourceTop + xByte];
			output[dest + 1] = input[sourceBottom - xByte];
		}
	}

	return true;
}

bool DMG_ExpandPCWStoredLayoutToPacked(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, uint32_t outputSize)
{
	uint32_t rowBytes = ((uint32_t)width + 7) >> 3;
	uint32_t packedSize = ((uint32_t)width * height + 1) >> 1;
	if (outputSize < packedSize)
		return false;

	for (uint32_t y = 0; y < height; y++)
	{
		uint8_t* rowOut = output + y * (((uint32_t)width + 1) >> 1);
		uint32_t base = (y >> 3) * width + (y & 7);
		for (uint32_t xByte = 0; xByte < rowBytes; xByte++)
			DMG_ExpandPCWByteToPacked(input[base + (xByte << 3)], rowOut + xByte * 4);
	}

	return true;
}

static bool DMG_DecodePCWByteStream(const uint8_t* input, uint32_t inputSize, uint8_t* output, uint32_t outputSize)
{
	if (inputSize < 5)
		return false;

	uint8_t tokenCount = input[0];
	if (tokenCount > 4)
		return false;

	const uint8_t* tokens = input + 1;
	const uint8_t* ptr = input + 5;
	const uint8_t* end = input + inputSize;
	uint32_t out = 0;

	while (ptr < end && out < outputSize)
	{
		uint8_t value = *ptr++;
		uint32_t repeat = 1;
		for (uint8_t n = 0; n < tokenCount; n++)
		{
			if (value == tokens[n])
			{
				if (ptr >= end)
					return false;
				repeat = *ptr++;
				break;
			}
		}

		if (out + repeat > outputSize)
			return false;
		MemSet(output + out, value, repeat);
		out += repeat;
	}

	return out == outputSize && ptr == end;
}

bool DMG_DecodePCWCompressedToPacked(const uint8_t* input, uint32_t inputSize, const DMG_Entry* entry, uint8_t* output, uint32_t outputSize)
{
	uint32_t monoSize = (((uint32_t)entry->width + 7) >> 3) * entry->height;

	uint8_t* decoded = Allocate<uint8_t>("PCW decode", monoSize, false);
	if (decoded == 0)
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}

	uint8_t* stored = Allocate<uint8_t>("PCW layout", monoSize, false);
	if (stored == 0)
	{
		Free(decoded);
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}

	bool success = DMG_DecodePCWByteStream(input, inputSize, decoded, monoSize);
	if (success)
		success = DMG_RearrangePCWDecodedToStoredLayout(decoded, entry->width, entry->height, stored, monoSize) &&
			DMG_ExpandPCWStoredLayoutToPacked(stored, entry->width, entry->height, output, outputSize);

	Free(stored);
	Free(decoded);
	return success;
}

uint8_t* DMG_GetEntryDataChunky (DMG* dmg, uint8_t index)
{
	bool success;
	DMG_Entry* entry = DMG_GetEntry(dmg, index);
	if (entry == 0)
		return 0;
	if (entry->type == DMGEntry_Empty)
		return 0;

	uint8_t* buffer = 0;
	uint8_t* fileData;
	uint32_t bufferSize = 0;

	unsigned requiredSize = DMG_CalculateRequiredSize(entry, ImageMode_Packed);

	DMG_Cache* cache = DMG_GetImageCache(dmg, index, entry, requiredSize);
	if (cache)
	{
		buffer = (uint8_t*)(cache + 1);
		if (cache->populated && cache->imageMode == ImageMode_Packed)
			return buffer;
		bufferSize = cache->size;
	}

	if (buffer == 0 || bufferSize < requiredSize)
	{
		buffer = DMG_GetTemporaryBuffer(ImageMode_Packed);
		bufferSize = DMG_GetTemporaryBufferSize();
		if (bufferSize < requiredSize)
		{
			DMG_Warning("Entry %d: Internal buffer too small for entry (%d bytes required)", index, requiredSize);
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return 0;
		}
	}
	if (buffer == 0)
	{
		// PANIC: No temporary buffer
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		return 0;
	}

	uint32_t dataOffset = entry->fileOffset + 6;
	fileData = (uint8_t*)DMG_GetFromFileCache(dmg, dataOffset, entry->length);
	if (fileData == 0)
	{
		if (entry->length > bufferSize)
		{
			DMG_Warning("Entry %d: Invalid entry data (size too big for image dimensions)", index);
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return 0;
		}

		fileData = buffer + bufferSize - entry->length;
		if (DMG_ReadFromFile(dmg, dataOffset, fileData, entry->length) != entry->length)
		{
			DMG_SetError(DMG_ERROR_READING_FILE);
			return 0;
		}
	}

	if (entry->type == DMGEntry_Audio)
		return fileData;

	if ((entry->flags & DMG_FLAG_COMPRESSED) != 0)
	{
		if (false)
		{
		}
		else if (dmg->version == DMG_Version1_PCW)
		{
			success = DMG_DecodePCWCompressedToPacked(fileData, entry->length, entry, buffer, requiredSize);
		}
		#if DMG_SUPPORT_EGA_SOURCES
		else if (dmg->version == DMG_Version1_EGA)
		{
			success = DMG_DecompressEGA(fileData, entry->length, buffer, entry->width, entry->height);
		}
		#endif
		#if DMG_SUPPORT_CGA_SOURCES
		else if (dmg->version == DMG_Version1_CGA)
		{
			success = DMG_DecompressCGA(fileData, entry->length, buffer, entry->width, entry->height);
		}
		#endif
		else
		{
			#if !DMG_SUPPORT_CROSS_ENDIAN_SOURCES
			if (!DMG_IsClassicNativeDATByteOrder(dmg->littleEndian))
			{
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				return 0;
			}
			#endif
			uint16_t mask = read16(fileData, dmg->littleEndian);
			if (dmg->version != DMG_Version2)
			{
				success = DMG_DecompressOldRLE(fileData+2, mask, 
					entry->length-2, buffer, requiredSize*2, dmg->littleEndian);
			}
			else
			{
				success = DMG_DecompressNewRLE(fileData+2, mask, 
					entry->length-2, buffer, requiredSize*2, dmg->littleEndian);
				if (success)
				{
					if (cache != 0)
					{
						cache->imageMode = ImageMode_Packed;
						cache->populated = true;
					}
					return buffer;
				}
			}
		}
	}
	else
	{
		uint32_t expectedSize = DMG_CalculateRequiredSize(entry, ImageMode_Raw);
		uint32_t packedSize = DMG_CalculateRequiredSize(entry, ImageMode_Packed);
		uint8_t* scratch = 0;
		uint32_t scratchSize = 0;
		if (entry->length < expectedSize)
			success = false;
		else if (dmg->version == DMG_Version1_PCW)
			success = DMG_ExpandPCWStoredLayoutToPacked(fileData, entry->width, entry->height, buffer, packedSize);
		#if DMG_SUPPORT_EGA_SOURCES
		else if (dmg->version == DMG_Version1_EGA)
		{
			scratch = DMG_GetScratchBuffer(dmg, packedSize);
			scratchSize = DMG_GetScratchBufferSize(dmg);
			success = DMG_UncEGAToPacked(fileData, entry->width, entry->height, buffer, packedSize, scratch, scratchSize);
		}
		#endif
		#if DMG_SUPPORT_CGA_SOURCES
		else if (dmg->version == DMG_Version1_CGA)
		{
			scratch = DMG_GetScratchBuffer(dmg, entry->length);
			scratchSize = DMG_GetScratchBufferSize(dmg);
			success = DMG_UncCGAToPacked(fileData, entry->width, entry->height, buffer, packedSize, scratch, scratchSize);
		}
		#endif
		#if DMG_SUPPORT_CROSS_ENDIAN_SOURCES
		else if (dmg->littleEndian)
			success = DMG_CopyImageData(fileData, entry->length, buffer, packedSize);
		#endif
		else 
			success = DMG_Planar8ToPacked(fileData, expectedSize, buffer, packedSize, entry->width);
	}
	if (!success)
	{
		if (cache != 0)
			cache->populated = false;
		if (DMG_GetError() == DMG_ERROR_NONE)
			DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
		return 0;
	}

	if (cache != 0)
	{
		cache->imageMode = ImageMode_Packed;
		cache->populated = true;
	}

	return buffer;
}
