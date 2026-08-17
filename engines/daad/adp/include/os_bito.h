#pragma once

#include <os_types.h>

static inline uint16_t read16LE(const uint8_t* ptr)
{
	return (uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8);
}

static inline uint32_t read32LE(const uint8_t* ptr)
{
	return (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8) | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3] << 24);
}

static inline uint16_t read16BE(const uint8_t* ptr)
{
	return (uint16_t)ptr[1] | ((uint16_t)ptr[0] << 8);
}

static inline uint32_t read32BE(const uint8_t* ptr)
{
	return (uint32_t)ptr[3] | ((uint32_t)ptr[2] << 8) | ((uint32_t)ptr[1] << 16) | ((uint32_t)ptr[0] << 24);
}

static inline void write16(uint8_t* ptr, uint16_t n, bool littleEndian)
{
	if (littleEndian)
	{
		ptr[0] = n & 0xFF;
		ptr[1] = (n >> 8) & 0xFF;
	}
	else
	{
		ptr[1] = n & 0xFF;
		ptr[0] = (n >> 8) & 0xFF;
	}
}

static inline void write32(uint8_t* ptr, uint32_t n, bool littleEndian)
{
	if (littleEndian)
	{
		ptr[0] = n & 0xFF;
		ptr[1] = (n >> 8) & 0xFF;
		ptr[2] = (n >> 16) & 0xFF;
		ptr[3] = (n >> 24) & 0xFF;
	}
	else
	{
		ptr[3] = n & 0xFF;
		ptr[2] = (n >> 8) & 0xFF;
		ptr[1] = (n >> 16) & 0xFF;
		ptr[0] = (n >> 24) & 0xFF;
	}
}

static inline uint16_t read16(const uint8_t* ptr, bool littleEndian)
{
	if (littleEndian)
		return (uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8);
	else
		return (uint16_t)ptr[1] | ((uint16_t)ptr[0] << 8);
}

static inline uint32_t read32(const uint8_t* ptr, bool littleEndian)
{
	if (littleEndian)
		return (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8) | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3] << 24);
	else
		return (uint32_t)ptr[3] | ((uint32_t)ptr[2] << 8) | ((uint32_t)ptr[1] << 16) | ((uint32_t)ptr[0] << 24);
}

static inline uint16_t fix16(uint16_t value, bool littleEndian)
{
	union
	{
		uint16_t as16;
		uint8_t  as8[2];
	}
	v;

	v.as16 = value;
	return read16(v.as8, littleEndian);
}

static inline uint32_t fix32(uint32_t value, bool littleEndian)
{
	union
	{
		uint32_t as32;
		uint8_t  as8[4];
	}
	v;

	v.as32 = value;
	return read32(v.as8, littleEndian);
}

static inline uint32_t fix32(uint32_t value)
{
#ifdef _BIG_ENDIAN
	return value;
#else
	return fix32(value, false);
#endif
}
