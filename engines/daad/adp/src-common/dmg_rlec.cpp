#include <dmg.h>
#include <ddb.h>
#include <ddb_pal.h>
#include <os_mem.h>
#include <os_lib.h>
#include <os_bito.h>

bool DMG_DecompressNewRLE (const uint8_t* d, uint16_t rleMask, uint16_t dataLength, uint8_t* buffer, int pixels, bool littleEndian)
{
	#if !DMG_SUPPORT_CROSS_ENDIAN_SOURCES
	(void)littleEndian;
	#endif
	uint32_t nibbles = 0;
	uint8_t color = 0;
	uint8_t repetitions;
	const uint32_t* data = (const uint32_t*) d;
	const uint32_t* start = data;
	const uint32_t* end = (const uint32_t*)(d + dataLength);
	int totalPixels = pixels;
	int nibbleCount = 0;

	uint8_t outc = 0;
	bool outcm = false;

	DMG_SetError(DMG_ERROR_NONE);

	while (pixels > 0)
	{
		if (nibbleCount == 0)
		{
			if (data >= end)
			{
				DMG_Warning("Early end of data stream at offset %lu/%lu (%d pixels remaining)",
					(unsigned long)(data - start), (unsigned long)(end - start), pixels);
				DMG_SetError(DMG_ERROR_TRUNCATED_DATA_STREAM);
				while (pixels-- > 0)
				{
					if (outcm)
						*buffer++ = color | (outc << 4);
					else 
						outc = color;
					outcm = !outcm;
				}
				break;
			}

			#if DMG_SUPPORT_CROSS_ENDIAN_SOURCES
			nibbles = fix32(*data++, littleEndian);
			#else
			nibbles = *data++;
			#endif
			nibbleCount = 8;
		}

		color = nibbles & 0x0F;
		nibbles >>= 4;
		nibbleCount--;
		if (outcm)
			*buffer++ = color | (outc << 4);
		else 
			outc = color;
		outcm = !outcm;

		pixels--;

		if ((rleMask & (1 << color)) != 0) 
		{
			if (nibbleCount == 0)
			{
				if (data >= end)
				{
					DMG_Warning("Early end of data stream at offset %lu/%lu (%d pixels remaining)",
						(unsigned long)(data - start), (unsigned long)(end - start), pixels);
					while (pixels-- > 0)
					{
						if (outcm)
							*buffer++ = color | (outc << 4);
						else 
							outc = color;
						outcm = !outcm;
					}
					DMG_SetError(DMG_ERROR_TRUNCATED_DATA_STREAM);
					break;
				}
				#if DMG_SUPPORT_CROSS_ENDIAN_SOURCES
				nibbles = fix32(*data++, littleEndian);
				#else
				nibbles = *data++;
				#endif
				nibbleCount = 8;
			}

			repetitions = nibbles & 0x0F;
			nibbles >>= 4;
			nibbleCount--;
			
			if (pixels < repetitions)
			{
				DMG_Warning("Data stream wrote %d pixels past image size (at offset %lu/%lu, already read %d pixels)",
					repetitions - pixels, (unsigned long)(data - start), (unsigned long)(end - start), totalPixels);
				DMG_SetError(DMG_ERROR_DATA_STREAM_TOO_LONG);
				repetitions = pixels;
			}
			pixels -= repetitions;
			while (repetitions-- > 0)
			{
				if (outcm)
					*buffer++ = color | (outc << 4);
				else 
					outc = color;
				outcm = !outcm;
			}
		}
	}

	if (outcm)
		*buffer++ = (outc << 4);

	if (data < end-4)
	{
		DMG_Warning("Data stream contains %u extra bytes (at offset %lu)",
			(unsigned)dataLength, (unsigned long)(data - start));
		DMG_SetError(DMG_ERROR_DATA_STREAM_TOO_LONG);
	}
	return true;
	//return DMG_GetError() == DMG_ERROR_NONE;
}
