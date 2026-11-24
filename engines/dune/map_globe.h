/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#ifndef DUNE_MAP_GLOBE_H
#define DUNE_MAP_GLOBE_H

#include "dune/map_renderer.h"

namespace Dune {

/** DOS BA15's 196 signed words. The high and low bytes encode separate sides
 * of the projected globe; they must not be reduced to one signed latitude. */
inline void buildGlobeTiltTable(uint16 *words, int tilt) {
	const int limited = tilt < -98 ? -98 : (tilt > 98 ? 98 : tilt);
	uint16 ax = (uint16)(limited + 98);
	int bx = 98;
	int cx = 196;
	uint written = 0;
	auto setLow = [&](byte value) { ax = (ax & 0xff00) | value; };
	auto low = [&]() { return (int8)(byte)ax; };
	auto put = [&]() { if (written < 196) words[written++] = ax; };

	if ((int16)ax > bx) {
		bx = -bx;
		setLow((byte)(ax + bx + bx - 1));
		put();
		while (low() > bx && --cx != 0) {
			setLow((byte)(ax - 1));
			put();
		}
		bx = -bx;
		ax = (uint16)bx;
		--cx;
	}

	for (;;) {
		put();
		setLow((byte)(ax - 1));
		if (low() < 0 || --cx == 0)
			break;
	}
	if (--cx <= 0)
		return;
	ax = (uint16)-ax;
	for (;;) {
		put();
		setLow((byte)(ax + 1));
		if (low() > bx || --cx == 0)
			break;
	}
	if (--cx <= 0)
		return;
	setLow((byte)-(byte)ax);
	do {
		setLow((byte)(ax + 1));
		put();
	} while (--cx != 0);
}

// DNVGA 1CA6/1CB4: the next geometry run and drawing direction. A completed
// full render leaves these ready for another incremental 148h frame.
struct GlobeRenderState {
	uint16 position = 1;
	int16 row = 54;
	bool lowerHalf = false;
};

// Validate a saved cursor against the actual run stream. No terrain is
// rendered and no saved partial pixels are regenerated during this check.
inline bool validGlobeRenderState(const GlobeRenderState &wanted, const byte *geometry, uint size) {
	if (!geometry || size != 16091 || !wanted.position || wanted.position >= 0xcda ||
			wanted.row < -1 || wanted.row > 109 ||
			(wanted.lowerHalf ? wanted.row < 55 : wanted.row > 54))
		return false;
	GlobeRenderState at;
	for (uint call = 0; call < 110 && at.position < 0xcda; call++) {
		if (wanted.position == at.position && wanted.row == at.row && wanted.lowerHalf == at.lowerHalf)
			return true;
		byte angle = geometry[at.position++];
		if (angle & 0x80) {
			if (at.lowerHalf)
				return false;
			at.lowerHalf = true;
			at.position = 1 - (int8)geometry[0];
			at.row = 55;
			if (at.position >= 0xcda)
				return false;
			angle = geometry[at.position++];
		}
		for (uint x = 0; x < 64 && !(angle & 0x80) && at.position < 0xcda; x++)
			angle = geometry[at.position++];
		at.row += at.lowerHalf ? 1 : -1;
	}
	return false;
}

/** DNVGA 145h/1CB6, using the run stream and radial projection in
 * GLOBDATA.HSQ. The DOS driver paints two pixels at a time from the centre
 * towards the rim, first above and then below the middle of the 129x110 oval.
 */
// With progress, execute just one 148h callback and return its carry flag;
// without it, execute 145h's complete render. The completion callback paints
// nothing and resets the run cursor, exactly as 1D25/1D5A do in the driver.
inline bool drawDosGlobe(byte *screen, int pitch, int left, int top,
		const byte *world, uint worldSize, const MapLatitude *latitudes,
		const byte *geometry, uint geometrySize, int spin, int tilt,
		bool results, GlobeRenderState *progress = nullptr) {
	if (!screen || !world || !latitudes || !geometry || geometrySize != 16091 ||
			worldSize <= 25340 || pitch < left + 128)
		return false;

	uint16 turns[99];
	const uint wrapped = ((spin % 398) + 398) % 398;
	turns[0] = wrapped;
	const uint16 fraction = (uint16)((((uint32)wrapped << 16) | 0x8000) / 398);
	for (uint i = 1; i < 99; i++)
		turns[i] = (uint16)((uint32)latitudes[i].halfWidth * fraction * 2 >> 16);

	uint16 tiltWords[196] = {};
	buildGlobeTiltTable(tiltWords, tilt);
	const int midpoint = (int)worldSize / 2;
	const int lookupBase = 0xcda;
	GlobeRenderState full;
	GlobeRenderState &drawing = progress ? *progress : full;
	auto colour = [results](byte terrain) -> byte {
		const byte ground = terrain & 0x0f;
		const byte owner = terrain & 0x30;
		if (results)
			return (byte)(0x10 + ground + (owner == 0 ? 0 : owner == 0x30 ? 0x20 : 0x10));
		return (byte)(0x10 + ground + (owner == 0x10 && ground < 8 ? 12 : 0));
	};

	for (uint line = 0; line < (progress ? 1U : 110U) && drawing.position < geometrySize; line++) {
		byte angle = geometry[drawing.position++];
		if (angle & 0x80) {
			if (drawing.lowerHalf) {
				drawing = GlobeRenderState();
				return true;
			}
			drawing.lowerHalf = true;
			drawing.position = 1 - (int8)geometry[0];
			drawing.row = 55;
			if (drawing.position >= geometrySize)
				break;
			angle = geometry[drawing.position++];
		}
		const int rowStart = (top + drawing.row) * pitch + left + 64;

		for (uint x = 0; x < 64 && !(angle & 0x80); x++) {
			const int tiltAt = 98 + (drawing.lowerHalf ? -(int)angle : (int)angle);
			if (tiltAt < 0 || tiltAt >= 196)
				break;
			const int16 projection = (int16)tiltWords[tiltAt];
			const int latitudeAt = (int8)(byte)projection;
			const uint fromPole = latitudeAt < 0 ? -latitudeAt : latitudeAt;
			const uint lookupAt = lookupBase + x * 200 + fromPole;
			if (lookupAt + 100 >= geometrySize)
				break;
			const uint lat = geometry[lookupAt] / 2;
			if (lat >= 99)
				break;
			const MapLatitude &row = latitudes[lat];
			const int half = row.halfWidth;
			const int circumference = half * 2;
			if (!circumference)
				break;
			int across = geometry[lookupAt + 100];
			if (latitudeAt < 0)
				across = half - across;
			const int offset = projection < 0 ? -(int)row.offset : row.offset;
			int first = (int)turns[lat] - across;
			if (first < 0)
				first += circumference;
			int second = (int)turns[lat] + across - circumference;
			if (second < 0)
				second += circumference;
			const int firstAt = midpoint + offset + first;
			const int secondAt = midpoint + offset + second;
			if (firstAt < 0 || secondAt < 0 || firstAt >= (int)worldSize ||
					secondAt >= (int)worldSize)
				break;
			screen[rowStart - 1 - (int)x] = colour(world[firstAt]);
			screen[rowStart + (int)x] = colour(world[secondAt]);
			if (drawing.position >= geometrySize)
				break;
			angle = geometry[drawing.position++];
		}
		drawing.row += drawing.lowerHalf ? 1 : -1;
	}
	return false;
}

/** DOS BAF2's lookup projection used before BC0C draws ICONES sprite 54.
 * An off-hemisphere point returns (-1,-1), corresponding to DX=0 in DOS. */
inline Common::Point projectDosGlobeMarker(const MapLatitude *latitudes,
		const byte *geometry, uint geometrySize, int spin, int tilt,
		uint16 longitude, int latitude) {
	if (!latitudes || !geometry || geometrySize != 16091 || latitude < -98 || latitude > 98)
		return Common::Point(-1, -1);

	const uint row = (uint)(latitude < 0 ? -latitude : latitude);
	const int half = latitudes[row].halfWidth;
	if (!half)
		return Common::Point(-1, -1);
	const uint wrapped = ((spin % 398) + 398) % 398;
	const uint16 fraction = (uint16)((((uint32)wrapped << 16) | 0x8000) / 398);
	const int turn = row ? (int)((uint32)half * fraction * 2 >> 16) : wrapped;
	const uint32 longitudeProduct = (uint32)(half * 2) * longitude;
	int distance = (int)(longitudeProduct >> 16) + ((longitudeProduct & 0xffff) >= 0x8000);
	distance -= turn;
	bool leftSide = false;
	if (distance < 0) {
		distance = -distance;
		leftSide = true;
	}
	if (distance >= half) {
		distance = 2 * half - distance;
		leftSide = !leftSide;
	}
	const int quarter = half >> 1;
	bool backSide = false;
	if (distance >= quarter) {
		backSide = true;
		distance = 2 * quarter - distance;
	}
	// BB65 compares DL, not the signed full word. At the narrow polar rows
	// the final fold can be -1; its 0xFF byte keeps scanning until a radial
	// lookup no longer contains that latitude.
	const byte radialDistance = (byte)distance;

	int radial = 0;
	int remaining = 0;
	int column = -1;
	for (; radial < 64; radial++) {
		const uint at = 0xcda + radial * 200;
		column = -1;
		for (int i = 0; i < 100; i++) {
			if (geometry[at + i] == row * 2) {
				column = i;
				break;
			}
		}
		if (column < 0) {
			column = 100 - remaining;
			break;
		}
		remaining = 100 - column;
		if (radialDistance <= geometry[at + column + 100])
			break;
	}
	if (radial >= 64 || column < 0)
		return Common::Point(-1, -1);

	byte vertical = (byte)column;
	if (backSide)
		vertical = (byte)-vertical;
	const uint16 requested = (latitude < 0 ? 0xff00 : 0) | vertical;
	uint16 tiltWords[196] = {};
	buildGlobeTiltTable(tiltWords, tilt);
	int matched = -1;
	for (int i = 34; i < 162; i++) {
		if (tiltWords[i] == requested) {
			matched = i;
			break;
		}
	}
	if (matched < 0)
		return Common::Point(-1, -1);
	const int angle = matched < 98 ? 98 - matched : matched - 98;
	const bool upper = matched < 98;

	uint at = 0;
	int best = 255;
	int bestRow = -1;
	for (int rowNumber = 54; rowNumber > 0; rowNumber--) {
		if (at >= 0xcda)
			break;
		byte run = (byte)(geometry[at] + 1);
		if (!run)
			break;
		run = (byte)-run;
		const uint begin = at + 1;
		at = begin + run;
		if (run <= radial || at > 0xcda)
			break;
		const int difference = (byte)(geometry[begin + radial] - angle);
		if (difference < best) {
			best = difference;
			bestRow = rowNumber;
			if (difference == 0)
				break;
		}
	}
	if (best > 2 || bestRow < 0)
		return Common::Point(-1, -1);
	return Common::Point(160 + (leftSide ? -radial : radial),
			79 + (upper ? 54 - bestRow : bestRow - 54));
}

/** DOS BC4E/BD25: pick the world point below a pixel of the globe.
 * Longitude is an unsigned fraction of a turn, not a Common::Point x.
 * The rectangle excludes its final column/row; the run stream then rejects
 * the corners outside the oval. Keep the hemisphere bytes separate. */
inline bool unprojectDosGlobePoint(const MapLatitude *latitudes,
		const byte *geometry, uint geometrySize, int spin, int tilt,
		const Common::Point &point, uint16 &longitude, int &latitude) {
	if (!latitudes || !geometry || geometrySize != 16091 ||
			point.x < 96 || point.x >= 224 || point.y < 25 || point.y >= 134)
		return false;
	const int x = point.x - 160, y = point.y - 79;
	const uint across = ABS(x);
	uint at = 0, run = 0;
	for (int i = 0; i <= ABS(y); i++) {
		at += run;
		if (at >= 0xcda)
			return false;
		run = (byte)-(byte)(geometry[at++] + 1);
		if (!run)
			return false;
	}
	if (across >= run || at + across >= 0xcda)
		return false;

	const int angle = geometry[at + across];
	uint16 tiltWords[196] = {};
	buildGlobeTiltTable(tiltWords, tilt);
	const int tiltAt = 98 + (y < 0 ? angle : -angle);
	if (tiltAt < 0 || tiltAt >= 196)
		return false;
	const int16 projection = (int16)tiltWords[tiltAt];
	const int band = (int8)(byte)projection;
	const uint lookup = 0xcda + across * 200 + ABS(band);
	if (lookup + 100 >= geometrySize)
		return false;
	const uint row = geometry[lookup] / 2;
	if (row >= 99 || !latitudes[row].halfWidth)
		return false;
	const int half = latitudes[row].halfWidth;
	const int circumference = 2 * half;
	int distance = geometry[lookup + 100];
	if (band < 0)
		distance = half - distance;
	if (x < 0)
		distance = -distance;
	const uint wrapped = ((spin % 398) + 398) % 398;
	const uint16 fraction = (uint16)((((uint32)wrapped << 16) | 0x8000) / 398);
	int turn = row ? (int)((uint32)half * fraction * 2 >> 16) : wrapped;
	turn += distance;
	if (turn < 0)
		turn += circumference;
	if (turn >= circumference)
		turn -= circumference;
	longitude = (uint32)turn * 65536u / circumference;
	latitude = projection < 0 ? -(int)row : (int)row;
	return true;
}

} // End of namespace Dune

#endif
