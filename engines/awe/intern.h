/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AWE_INTERN_H
#define AWE_INTERN_H

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/language.h"
#include "common/rect.h"

namespace Awe {

using Common::Language;

enum {
	kPartCopyProtection = 16000,
	kPartIntro = 16001,
	kPartWater = 16002,
	kPartPrison = 16003,
	kPartCite = 16004,
	kPartArene = 16005,
	kPartLuxe = 16006,
	kPartFinal = 16007,
	kPartPassword = 16008
};

enum {
	kPaulaFreq = 7159092
};

struct Ptr {
	uint8 *pc = nullptr;
	bool byteSwap = false;

	uint8 fetchByte() {
		return *pc++;
	}

	uint16 fetchWord() {
		const uint16 i = byteSwap ? READ_LE_UINT16(pc) : READ_BE_UINT16(pc);
		pc += 2;
		return i;
	}
};

struct Point : public Common::Point {
	Point() : Common::Point() {}
	Point(int16 xx, int16 yy) : Common::Point(xx, yy) {}
	Point(const Point &p) : Common::Point(p) {}

	void scale(int u, int v) {
		x = (x * u) >> 16;
		y = (y * v) >> 16;
	}
};

struct QuadStrip {
	enum {
		MAX_VERTICES = 70
	};

	uint8 numVertices = 0;
	Point vertices[MAX_VERTICES];
};

struct Color {
	byte rgb[3];
	uint8 &r = rgb[0];
	uint8 &g = rgb[1];
	uint8 &b = rgb[2];

	Color() {
		clear();
	}
	void clear() {
		r = g = b = 0;
	}
};

struct Frac {
	static const int BITS = 16;
	static const int MASK = (1 << BITS) - 1;
	uint32 inc = 0;
	uint64 offset = 0;

	void reset(int n, int d) {
		inc = (((int64)n) << BITS) / d;
		offset = 0;
	}

	uint32 getInt() const {
		return offset >> BITS;
	}
	uint32 getFrac() const {
		return offset & MASK;
	}
	int interpolate(int sample1, int sample2) const {
		const int fp = getFrac();
		return (sample1 * (MASK - fp) + sample2 * fp) >> BITS;
	}
};

} // namespace Awe

#endif
