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

#include "dune/map_region.h"
#include "common/util.h"

namespace Dune {

void liberateMapDeposits(byte *world, uint worldSize, const byte *deposits,
		uint depositSize, const bool *released) {
	const uint count = MIN<uint>(0xc5f9, MIN(worldSize, depositSize));
	for (uint i = 0; i < count; i++) {
		if (deposits[i] && released[deposits[i]] && (world[i] & 0x30) == 0x30)
			world[i] &= 0xef;
	}
}

uint16 decayMapVegetation(byte *world, uint worldSize, uint16 seed, bool &changed) {
	changed = false;
	for (uint i = 0; i < 70; i++) {
		seed = (seed >> 1) ^ ((seed & 1) ? 0x402 : 0);
		for (uint at = seed; at < worldSize; at += 0x7ff) {
			if ((world[at] & 0x30) == 0x10) {
				world[at] = (world[at] & 0xcf) | 0x20;
				changed = true;
			}
		}
	}
	return seed;
}

void paintMapRegion(byte *world, uint worldSize, const MapLatitude *latitudes,
		uint16 longitude, int latitude, uint radius, byte owner,
		VegetationPlaceCallback onPlace, void *context, MapRegionPaintState *sharedState) {
	if (!world || !latitudes || !worldSize || !radius || radius > 12)
		return;
	if (owner != 0 && owner != 0x20 && owner != 0x30)
		return;
	MapRegionPaintState localState;
	MapRegionPaintState &paint = sharedState ? *sharedState : localState;
	paint.longitude = longitude;
	paint.latitude = latitude;
	paint.owner = owner;
	if (!owner)
		paint.pattern = 0x44; // 0x6518: restarted for each vegetation footprint.
	auto span = [&](int y, int left, int count) {
		// 6500/6508 read the shared callback afresh for each paired span.
		// A capture switches subsequent spans to ownership at the fort's
		// centre, but the active vegetation span must still finish its row.
		const byte spanOwner = paint.owner;
		y += paint.latitude;
		const int limit = spanOwner ? 93 : 86;
		if (y < -limit || y > limit)
			return;
		const MapLatitude &row = latitudes[ABS(y)];
		const int width = 2 * row.halfWidth;
		if (!width)
			return;
		const int start = (int)(worldSize / 2) + (y < 0 ? -(int)row.offset : row.offset);
		// 0xB58B rounds the longitude product before adding the span offset.
		int x = (int)(((uint32)paint.longitude * width + 0x8000) >> 16) + left;
		while (x < 0)
			x += width;
		for (int i = 0; i < count; i++) {
			const int offset = start + x;
			if (offset >= 0 && (uint)offset < worldSize) {
				const byte was = world[offset];
				if ((was & 0x30) != 0x10) {
					byte pixel = (was & 0xcf) | (spanOwner ? spanOwner : 0x20);
					if (!spanOwner) {
						if ((was & 0x40) && onPlace)
							onPlace(offset, context);
						if ((was & 0x0e) < 8) {
							const bool green = (paint.pattern & 0x80) != 0;
							paint.pattern = (byte)((paint.pattern << 1) | (paint.pattern >> 7));
							if (green)
								pixel = (pixel & 0xcf) | 0x10;
						}
					}
					world[offset] = pixel;
				}
			}
			if (++x >= width)
				x -= width;
		}
	};

	// Preserve the original paired-span order, including overlapping rows:
	// those repeated visits affect vegetation's rotating 0x44 pattern.
	int top = -(int)radius, bottom = radius - 1;
	int right = 0, left = 0, slope = 0;
	uint16 balance = radius, remaining = radius;
	auto pair = [&]() {
		span(top, left, right - left + 1);
		span(bottom, left, right - left + 1);
	};
	for (;;) {
		balance -= slope;
		while ((int16)balance >= 0) {
			++slope;
			++right;
			--left;
			balance -= slope;
		}
		pair();
		++top;
		--bottom;
		balance += remaining;
		if ((int16)balance < 0 || --remaining == 0)
			break;
		++slope;
		++right;
		--left;
	}
	++slope;
	--remaining;
	for (;;) {
		pair();
		++top;
		--bottom;
		const uint sum = (uint)balance + remaining;
		balance = (uint16)sum;
		if (sum <= 0xffff && --remaining != 0)
			continue;
		if ((int16)--remaining < 0)
			break;
		++right;
		--left;
		balance -= slope;
		++slope;
	}
}

} // End of namespace Dune
