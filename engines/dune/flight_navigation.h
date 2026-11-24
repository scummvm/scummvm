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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DUNE_FLIGHT_NAVIGATION_H
#define DUNE_FLIGHT_NAVIGATION_H

#include "common/util.h"

namespace Dune {

// B7D2/B5F9: invert a compact-map pixel using the prepared integer row
// centre. Adding a reciprocal pixel offset to the continuous longitude,
// or converting through an equatorial column, changes the flight bearing.
inline uint16 insetTargetLongitude(uint16 centreLongitude, uint width, int offset) {
	if (!width)
		return centreLongitude;
	int column = (int)((uint32)centreLongitude * width / 65536u) + offset;
	column = (column % (int)width + (int)width) % (int)width;
	return (uint32)column * 65536u / width;
}

// 514E/45A3: DOS labels only a narrow wedge around each of the eight
// bearings. Between wedges the cabin says just DESERT, without a direction.
inline int cabinDirectionIndex(int dx, int dy) {
	byte bearing = 0;
	if (ABS(dx) >= ABS(dy)) {
		if (dx != 0)
			bearing = (byte)(32 * dy / dx + (dx >= 0 ? 64 : 192));
	} else {
		bearing = (byte)(-(32 * dx / dy - (dy >= 0 ? 128 : 0)));
	}
	const byte adjusted = bearing + 3;
	return (adjusted & 31) < 6 ? adjusted >> 5 : -1;
}

// DOS 5133: a square bearing, with longitude wrapping and latitude scaled
// by 256. Zero is north; 64 east. False means both coordinates coincide.
inline bool flightBearing(uint16 longitude, int latitude, uint16 targetLongitude,
		int targetLatitude, byte &heading) {
	int dx = (int16)(targetLongitude - longitude);
	int dy = (int16)(targetLatitude - latitude);
	if (dy < -128 || dy >= 128) {
		dx >>= 1;
		dy >>= 1;
	}
	dy = (int16)((uint16)(byte)dy << 8);
	if (!dx && !dy)
		return false;
	if (ABS(dx) >= ABS(dy))
		heading = (byte)(32 * dy / dx + (dx >= 0 ? 64 : 192));
	else
		heading = (byte)(-(32 * dx / dy - (dy >= 0 ? 128 : 0)));
	return true;
}

// DOS 5198/5206. Keep the fractional latitude in DS:11CC; rounding the
// heading to a walking direction loses small turns and all diagonal motion.
inline void advanceFlightPosition(uint16 &longitude, int16 &latitude, byte &heading,
		byte &fraction, uint step, bool manual) {
	if (manual && (latitude < -77 || latitude > 77) &&
			!(((byte)(heading - 64) ^ (byte)(latitude >> 8)) & 128))
		heading = (heading & 128) | 64; // 51CB: steer away from a pole.

	byte angle = heading;
	const byte quadrant = angle + 32;
	int dx, dy;
	if ((quadrant & 127) >= 64) {
		dx = 32;
		angle -= 64;
		if (quadrant & 128) {
			dx = -32;
			angle = -(byte)(angle - 128);
		}
		dy = (int8)angle;
	} else {
		dy = -32;
		if (quadrant & 128) {
			dy = 32;
			angle = -(byte)(angle - 128);
		}
		dx = (int8)angle;
	}
	dx = dx * (int)step / 32;
	dy = dy * (int)step / 32;
	uint accumulated = ABS(dy) + fraction;
	if (accumulated >= 512) {
		dx = dx * 256 / (int)accumulated;
		accumulated = 256;
	}
	fraction = (byte)accumulated;
	latitude += dy < 0 ? -(int)(accumulated >> 8) : (int)(accumulated >> 8);
	longitude += dx;
	if ((uint16)(latitude + 96) >= 192) {
		heading += 128;
		longitude += 32768;
	}
}

// 413A: accept [-96, 96), excluding the rear quarter of the view.
inline bool flightCanSee(byte heading, byte bearing) {
	return (byte)(bearing - heading + 96) < 192;
}

// 4EC2..4F03: requested DOS movie, switched by CB61 at a loop boundary.
// The table has a reserved slot: 1=DFL2, 2=MNT1, 3=MNT2, 4=MNT3, 5=MNT4.
// MNT2 enters rocky terrain, MNT3 continues it, MNT4 returns to open ground.
inline uint16 flightTerrainMovie(byte sixth, byte seventh, uint16 &base, uint16 current) {
	uint16 wanted = base;
	if (base >= 2) {
		const byte average = (byte)(sixth + seventh) >> 1;
		if ((average & 15) < 8) {
			if (current > 2) {
				wanted = current <= 4 ? 5 : 2;
				if (current > 4)
					base = 2; // 4EE9; the MNT4 request bypasses this store.
			}
		} else {
			wanted = current <= 2 || current == 5 ? 3 : 4;
		}
	}
	return wanted;
}

} // namespace Dune

#endif
