/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AUDIO_SOFTSYNTH_OPL_MAME_FAKE_EMU_H
#define AUDIO_SOFTSYNTH_OPL_MAME_FAKE_EMU_H

#include <stdint.h>

// emu/emucore.h
#include <assert.h>

#define NAME(x) x, #x

#if __cplusplus < 201103L
#define constexpr
#endif

template <typename T, typename U> constexpr T BIT(T x, U n) { return (x >> n) & T(1); }

#if __cplusplus < 201103L
#undef constexpr
#endif

// In MAME this is actually 32-bits, but FMOPL generates 16-bit samples anyway
typedef int16_t stream_sample_t;

// emu/eminline.h
inline int64_t mul_32x32(int32_t a, int32_t b) {
	return int64_t(a) * int64_t(b);
}

inline uint64_t mulu_32x32(uint32_t a, uint32_t b) {
	return uint64_t(a) * uint64_t(b);
}

inline uint32_t divu_64x32(uint64_t a, uint32_t b) {
	return a / uint64_t(b);
}

inline uint32_t divu_64x32_rem(uint64_t a, uint32_t b, uint32_t *remainder) {
	uint32_t const res = divu_64x32(a, b);
	*remainder = a - (uint64_t(b) * res);
	return res;
}

// osd/osdcomm.h
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;

#include "emu/attotime.h"

#include "fake-device.h"

#endif // AUDIO_SOFTSYNTH_OPL_MAME_EMU_H
