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

#include "dune/map_renderer.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/util.h"

namespace Dune {

namespace {

// A sample is a 4.4 fixed-point height, centred within its colour band.
byte heightAt(const byte *world, uint size, int index) {
	return index >= 0 && (uint)index < size ? (byte)(world[index] * 16 + 8) : 0;
}

// DNVGA 0x2025 expands 89 successive samples into quarter-step heights.
// Each latitude retains its own width and fractional longitude alignment.
void prepareRow(byte *buffer, const byte *world, uint size,
		const MapLatitude *latitudes, uint16 longitude, int latitude) {
	const MapLatitude &row = latitudes[ABS(latitude)];
	const int start = size / 2 + (latitude < 0 ? -(int)row.offset : row.offset);
	const int width = 2 * row.halfWidth;
	const uint32 product = (uint32)longitude * width;
	const int count = MIN(width, 88);
	int x = ((product >> 16) + width - count / 2) % width;
	int out = 140 - ((product >> 14) & 3) + MAX(88 - width, 0) * 2;
	byte previous = heightAt(world, size, start + x - 1);
	for (int i = 0; i <= count; i++) {
		const byte current = heightAt(world, size, start + x);
		const int delta = (int8)(current - previous) >> 2;
		for (int quarter = 0; quarter < 4; quarter++)
			buffer[out++] = (byte)(previous + quarter * delta);
		previous = current;
		x = (x + 1) % width;
	}
}

// 0x2123/0x2153 interpolate from the narrower row towards the wider one.
// Fractional accumulators expand each intermediate line by its own amount.
void interpolateRows(byte *buffer, const MapLatitude *latitudes, const byte *edges, int latitude) {
	const int bottom = latitudes[ABS(latitude)].halfWidth;
	const int top = latitudes[ABS(latitude - 1)].halfWidth;
	const bool north = latitude <= 0;
	const int narrow = north ? top : bottom;
	const int wide = north ? bottom : top;
	const uint ratio = ((wide - narrow) << 16) / narrow;
	const int quarter = ratio >> 2, half = ratio >> 1;
	const int jumps[3] = { (quarter + 128) >> 8, (half + 128) >> 8,
		(quarter + half + 128) >> 8 };
	const int rowA = north ? 0 : 4, rowB = 4 - rowA;
	const int intermediate[3] = { north ? 1 : 3, 2, north ? 3 : 1 };
	for (int direction = 1; direction >= -1; direction -= 2) {
		int ax = direction > 0 ? 316 : 315, bx = ax;
		int destinations[3] = { ax, ax, ax };
		int phases[3] = { jumps[0], jumps[1], jumps[2] };
		uint phase = ratio;
		for (int remaining = 176; remaining > 0; remaining--) {
			const byte a = buffer[rowA * 400 + ax];
			const byte b = buffer[rowB * 400 + bx];
			ax += direction;
			bx += direction;
			const int delta = (int8)(b - a) >> 2;
			for (int i = 0; i < 3; i++) {
				const byte value = (byte)(a + (i + 1) * delta);
				buffer[intermediate[i] * 400 + destinations[i]] = value;
				destinations[i] += direction;
				phases[i] += jumps[i];
				if (phases[i] >= 256) {
					phases[i] -= 256;
					buffer[intermediate[i] * 400 + destinations[i]] = value;
					destinations[i] += direction;
				}
			}
			phase += ratio;
			if (phase >= 65536) {
				phase -= 65536;
				bx += direction;
				// Preserve the original's asymmetric edge walk (0x2202/0x2285).
				if (direction > 0)
					remaining--;
			}
		}
	}

	// 0x22A0 masks the last polar rows with the driver asset's curved edge.
	const int first = north ? 86 : 87;
	if (ABS(latitude) >= first) {
		const int position = (north ? 8 : 64) + (ABS(latitude) - first) * 8;
		for (int y = 0; y < 4; y++) {
			const int border = edges[position + y * 2];
			const int halfWidth = edges[position + y * 2 + 1];
			byte *out = buffer + y * 400 + 160;
			const int zeroes = MAX(border - 4, 0), colours = MIN(border, 4);
			memset(out, 0, zeroes);
			memcpy(out + zeroes, edges + 4 - colours, colours);
			out += border + 2 * halfWidth;
			memcpy(out, edges + 4, colours);
			memset(out + colours, 0, zeroes);
		}
	}
}

} // End of anonymous namespace

void drawStrategicMap(byte *screen, int pitch, const byte *world, uint worldSize,
		const MapLatitude *latitudes, const byte *edges, uint16 longitude, int latitude) {
	latitude = CLIP(latitude, -75, 75);
	byte buffer[2800] = {};
	prepareRow(buffer, world, worldSize, latitudes, longitude, latitude - 18);
	for (int strip = 0; strip < 36; strip++) {
		const int row = latitude - 17 + strip;
		prepareRow(buffer + 1600, world, worldSize, latitudes, longitude, row);
		interpolateRows(buffer, latitudes, edges, row);
		for (int y = 0; y < 4; y++) {
			byte *out = screen + (4 + strip * 4 + y) * pitch + 4;
			for (int x = 0; x < 312; x++)
				out[x] = (buffer[y * 400 + 160 + x] >> 4) + 16;
		}
		memcpy(buffer + 140, buffer + 1740, 700);
	}
}

Common::Point strategicMapPosition(const MapLatitude *latitudes,
		uint16 centreLongitude, int centreLatitude, uint16 longitude, int latitude) {
	if (latitude < -98 || latitude > 98)
		return Common::Point(-32768, -32768);
	const int32 delta = (int16)(longitude - centreLongitude);
	const int32 product = delta * latitudes[ABS(latitude)].halfWidth * 8;
	// IMUL's signed high word rounds negative products down, not toward zero.
	const int x = product >= 0 ? product / 65536 : -((-product + 65535) / 65536);
	return Common::Point(160 + x, 76 + (latitude - CLIP(centreLatitude, -75, 75)) * 4);
}

void visitMapVegetation(const byte *world, uint worldSize, const MapLatitude *latitudes,
		uint16 longitude, int latitude, MapDecorationCallback draw, void *context) {
	if (!world || !latitudes || !draw)
		return;
	latitude = CLIP(latitude, -75, 75);
	for (int row = latitude - 18; row < latitude + 18; row++) {
		const MapLatitude &band = latitudes[ABS(row)];
		const int width = 2 * band.halfWidth;
		if (!width)
			continue;
		const int start = worldSize / 2 + (row < 0 ? -(int)band.offset : band.offset);
		const int middle = ((uint32)longitude * width + 0x8000) >> 16;
		const int y = 76 + (row - latitude) * 4;
		auto scan = [&](int direction) {
			int sample = middle, at = start + sample;
			for (int x = 160; x >= 4 && x < 316; x += direction * 4) {
				// B58B rounds the initial sample without wrapping it. The
				// two-byte lookahead also crosses physical row boundaries.
				if (at >= 0 && (uint)at + 1 < worldSize && (world[at] & 0x30) == 0x10) {
					const uint sprite = (world[at + 1] & 0x30) == 0x10 ? 121 : 120;
					draw(sprite, x + (((width + at) >> 2) & 3), y + (at & 3), context);
				}
				at += direction;
				sample += direction;
				if (direction > 0 && sample >= width) {
					sample -= width;
					at -= width;
				} else if (direction < 0 && sample < 0) {
					sample += width;
					at += width;
				}
			}
		};
		// 634D scans right and then left, drawing the centre twice.
		scan(1);
		scan(-1);
	}
}

void buildSpiceDensityColours(byte *colours, const byte *state, uint stateSize) {
	if (!colours)
		return;
	memset(colours, 0x70, 256);
	if (!state)
		return;
	const byte unsurveyed = stateSize > 0x2942 && (state[0x2942] & 2) ? 0x78 : 0x75;
	for (uint record = 0x100; record < 0x8a8 && record + 28 <= stateSize; record += 28) {
		if (state[record] == 0xff)
			break;
		const byte flags = state[record + 10];
		if (flags & 0x80)
			continue;
		// The ID is recorded by map initialization, not resampled through a
		// screen projection. Like DOS, later known places sharing it overwrite
		// earlier ones, even when the later place has not been surveyed.
		colours[state[record + 16]] = flags & 0x40
			? 0x50 + (state[record + 18] >> 4) : unsurveyed;
	}
}

SurveyTroopCounts countSurveyTroops(const byte *state, uint stateSize, uint16 place) {
	SurveyTroopCounts counts = {};
	if (!state || place < 0x100 || place >= 0x8a8 || (place - 0x100) % 28 ||
			(uint)place + 28 > stateSize)
		return counts;
	auto count = [&counts](byte status) {
		const uint order = status & 15;
		// 6906 returns CF for state < 80; ownership, held and prisoner flags
		// are not separate exclusions in the survey's callback.
		if (status < 0x80 && order != 2)
			counts.occupation[MIN(order >> 2, 2u)]++;
	};
	bool visited[67] = {};
	for (uint id = state[place + 9]; id && id <= 67 && !visited[id - 1];) {
		const uint troop = 0x8aa + (id - 1) * 27;
		if (troop + 27 > stateSize)
			break;
		visited[id - 1] = true;
		count(state[troop + 3]);
		id = state[troop + 1];
	}
	for (uint troop = 0x8aa; troop < 0xfbb && troop + 27 <= stateSize; troop += 27) {
		const byte status = state[troop + 3];
		// Collectors name their return site at +0C, not the outward destination.
		const uint destination = (status & 3) == 3 ? 12 : 4;
		if ((status & 0x40) && READ_LE_UINT16(state + troop + destination) == place)
			count(status);
	}
	return counts;
}

byte surveyTroopPattern(const SurveyTroopCounts &counts) {
	const byte a = counts.occupation[0], b = counts.occupation[1], c = counts.occupation[2];
	const uint present = (a ? 1 : 0) | (b ? 2 : 0) | (c ? 4 : 0);
	switch (present) {
	case 0: return 0;
	case 1: return 0x55;
	case 2: return 0xaa;
	case 3: return a == b ? 0x66 : (a < b ? 0x9a : 0x65);
	case 4: return 0xff;
	case 5: return a == c ? 0x77 : (a < c ? 0xdf : 0x75);
	case 6: return b == c ? 0xbb : (b < c ? 0xef : 0xba);
	default:
		// Three occupations share the rows 2:1:1. Ties favour mining, then
		// military, exactly as the original's ordered comparisons do.
		return a < b ? (b < c ? 0xde : 0x9b) : (a < c ? 0x7b : 0x67);
	}
}

void buildTroopDistributionColours(byte *colours, const byte *state, uint stateSize) {
	if (!colours)
		return;
	memset(colours, 0x70, 256);
	if (!state)
		return;
	for (uint place = 0x100; place < 0x8a8 && place + 28 <= stateSize; place += 28) {
		if (state[place] == 0xff)
			break;
		if (!(state[place + 10] & 0x80))
			colours[state[place + 16]] = surveyTroopPattern(countSurveyTroops(state, stateSize, place));
	}
}

int clampSurveyLatitude(int latitude, int height) {
	const int limit = MAX(86 - (height - 1) / 2, 0);
	return CLIP(latitude, -limit, limit);
}

Common::Point surveyMapPosition(const MapLatitude *latitudes,
		uint16 centreLongitude, int centreLatitude, uint16 longitude, int latitude,
		const Common::Rect &view) {
	if (latitude < -98 || latitude > 98)
		return Common::Point(-32768, -32768);
	const int32 product = (int16)(longitude - centreLongitude) *
		(int32)latitudes[ABS(latitude)].halfWidth * 2;
	const int x = product >= 0 ? product / 65536 : -((-product + 65535) / 65536);
	return Common::Point(view.left + view.width() / 2 + x,
		view.top + (view.height() - 1) / 2 + latitude - clampSurveyLatitude(centreLatitude, view.height()));
}

void drawSurveyMap(byte *screen, int pitch, const byte *world, uint worldSize,
		const MapLatitude *latitudes, const byte *edges, const byte *colours,
		uint16 longitude, int latitude, const Common::Rect &view, bool troopMode,
		byte *scratch, uint scratchSize) {
	const int width = view.width(), height = view.height();
	if (width < 2 || width > 200 || height < 1 || height > 173)
		return;
	latitude = clampSurveyLatitude(latitude, height) - (height - 1) / 2;
	// The DOS scratch rows have a 200-byte pitch, even in the 160-pixel inset.
	// 24C0/24D2 also read the retained row BELOW the prepared rectangle.
	// C085's previous drawing can leave actual pixels there. Only isolated
	// callers without a shared scratch page start with a zero neighbour.
	const uint required = height * 200 + width;
	const bool retained = scratch && scratchSize >= required;
	Common::Array<byte> buffer(retained ? 0 : required, 0);
	byte *prepared = retained ? scratch : buffer.begin();
	for (int y = 0; y < height; y++) {
		const int atLatitude = latitude + y;
		const MapLatitude &row = latitudes[ABS(atLatitude)];
		const int rowWidth = row.halfWidth * 2;
		if (!rowWidth)
			continue;
		const int start = worldSize / 2 + (atLatitude < 0 ? -(int)row.offset : row.offset);
		const int count = MIN(width, rowWidth);
		int column = (((uint32)longitude * rowWidth >> 16) + rowWidth - count / 2) % rowWidth;
		byte *out = prepared + y * 200 + (width - count) / 2;
		for (int x = 0; x < count; x++) {
			const int index = start + column;
			out[x] = index >= 0 && (uint)index < worldSize ? world[index] : 0;
			column = (column + 1) % rowWidth;
		}
	}
	for (int y = 0; y < height; y++) {
		byte *out = screen + (view.top + y) * pitch + view.left;
		const byte *row = prepared + y * 200;
		const int polar = ABS(latitude + y) - 70;
		const int edge = polar >= 0 ? edges[polar] : 0;
		if (2 * edge >= width) {
			memset(out, 0, width);
			continue;
		}
		if (edge) {
			memset(out, 0, edge);
			memset(out + width - edge, 0, edge);
			out[edge - 1] = 23;
			out[width - edge] = 23;
			if (edge >= 2) {
				out[edge - 2] = 24;
				out[width - edge + 1] = 24;
			}
			if (edge >= 4) {
				out[edge - 4] = 28;
				out[edge - 3] = 25;
				out[width - edge + 2] = 25;
				out[width - edge + 3] = 28;
			}
		}
		for (int x = edge; x < width - edge; x++) {
			const byte deposit = row[x];
			const bool boundary = deposit != row[x + 200] ||
				(x + 1 < width - edge && deposit != row[x + 1]);
			byte colour = colours[deposit];
			if (troopMode && colour != 0x70) {
				// 24A8 rotates every non-unknown entry left two bits after each
				// row. Compute the same phase without modifying the lookup.
				const uint shift = (y & 3) * 2;
				colour = 0x71 + (((colour << shift) | (colour >> (8 - shift))) & 3);
			}
			out[x] = boundary ? 0x70 : colour;
		}
	}
}

void drawTerrainInset(byte *screen, int pitch, const byte *world, uint worldSize,
		const MapLatitude *latitudes, const byte *edges,
		uint16 longitude, int latitude, const Common::Rect &view, byte *scratch, uint scratchSize) {
	const int width = view.width(), height = view.height();
	if (!screen || !world || !latitudes || !edges ||
			width < 2 || width > 200 || height < 1 || height > 173)
		return;

	// B6C3 prepares a 200-byte scratch row for each displayed latitude.
	// DNVGA 0x142 walks those rows in reverse and adds 0x10 to the low nibble.
	const int firstLatitude = clampSurveyLatitude(latitude, height) - (height - 1) / 2;
	const uint required = height * 200;
	const bool retained = scratch && scratchSize >= required;
	Common::Array<byte> buffer(retained ? 0 : required, 0);
	byte *prepared = retained ? scratch : buffer.begin();
	for (int y = 0; y < height; y++) {
		const int band = firstLatitude + y;
		const MapLatitude &row = latitudes[ABS(band)];
		const int rowWidth = 2 * row.halfWidth;
		if (!rowWidth)
			continue;
		const int start = worldSize / 2 + (band < 0 ? -(int)row.offset : row.offset);
		const int count = MIN(width, rowWidth);
		int column = (((uint32)longitude * rowWidth >> 16) + rowWidth - count / 2) % rowWidth;
		byte *out = prepared + y * 200 + (width - count) / 2;
		for (int x = 0; x < count; x++) {
			const int index = start + column;
			out[x] = index >= 0 && (uint)index < worldSize ? world[index] : 0;
			column = (column + 1) % rowWidth;
		}
	}

	for (int y = height - 1; y >= 0; y--) {
		byte *out = screen + (view.top + y) * pitch + view.left;
		const byte *row = prepared + y * 200;
		const int polar = ABS(firstLatitude + y) - 70;
		const int edge = polar >= 0 ? edges[polar] : 0;
		if (2 * edge >= width) {
			memset(out, 0, width);
			continue;
		}
		if (edge) {
			memset(out, 0, edge);
			memset(out + width - edge, 0, edge);
			out[edge - 1] = 23;
			out[width - edge] = 23;
			if (edge >= 2) {
				out[edge - 2] = 24;
				out[width - edge + 1] = 24;
			}
			if (edge >= 4) {
				out[edge - 4] = 28;
				out[edge - 3] = 25;
				out[width - edge + 2] = 25;
				out[width - edge + 3] = 28;
			}
		}
		for (int x = edge; x < width - edge; x++)
			out[x] = 16 + (row[x] & 0x0f);
	}
}

uint16 drawPatternedMapLine(byte *screen, int pitch, Common::Point from, Common::Point to,
		const Common::Rect &clip, byte colour, uint16 pattern) {
	auto plot = [&](int x, int y) {
		// ROL occurs before the carry test and advances even for clipped dots.
		const bool draw = (pattern & 0x8000) != 0;
		pattern = (uint16)((pattern << 1) | (pattern >> 15));
		if (draw && x >= clip.left && x < clip.right && y >= clip.top && y < clip.bottom)
			screen[y * pitch + x] = colour;
	};
	const int dx = (int16)(to.x - from.x), dy = (int16)(to.y - from.y);
	if (!dy) {
		// The horizontal fast path sorts its endpoints and includes both.
		if (from.y >= clip.top && from.y < clip.bottom)
			for (int x = MIN(from.x, to.x); x <= MAX(from.x, to.x); x++)
				plot(x, from.y);
	} else if (!dx) {
		int y = dy < 0 ? to.y : from.y;
		uint count = ABS(dy);
		if (y >= 200 || from.x < clip.left || from.x >= clip.right)
			return pattern;
		// 1A9E/1AA0 discards negative screen rows without advancing the
		// pattern. Preserve its word-sized count, including the DOS quirk
		// where a wholly negative vertical segment extends into the viewport.
		if (y < 0) {
			count = (uint16)(count + y);
			y = 0;
		}
		count = (uint16)(count + 1);
		if (!count)
			count = 65536; // LOOP entered with CX=0 runs a full word's cycle.
		for (uint i = 0; i < count; i++)
			plot(from.x, y + i);
	} else {
		const int xStep = dx < 0 ? -1 : 1, yStep = dy < 0 ? -1 : 1;
		const bool horizontal = ABS(dx) > ABS(dy);
		const int major = MAX(ABS(dx), ABS(dy)), minor = MIN(ABS(dx), ABS(dy));
		int error = major / 2;
		for (int i = 0; i < major; i++) {
			error += minor;
			const bool diagonal = error >= major;
			if (diagonal)
				error -= major;
			if (diagonal || horizontal)
				from.x = (int16)(from.x + xStep);
			if (diagonal || !horizontal)
				from.y = (int16)(from.y + yStep);
			// Unlike the two axis paths, 1B35 steps before plotting: the
			// starting endpoint is omitted and the final endpoint included.
			plot(from.x, from.y);
		}
	}
	return pattern;
}

void drawSurveyViewport(byte *screen, int pitch, const Common::Point &centre,
		const Common::Rect &view) {
	if (view.isEmpty())
		return;
	const int x1 = CLIP((int)centre.x - 40, (int)view.left, (int)view.right - 1);
	const int x2 = CLIP((int)centre.x + 39, (int)view.left, (int)view.right - 1);
	const int y1 = CLIP((int)centre.y - 20, (int)view.top, (int)view.bottom - 1);
	const int y2 = CLIP((int)centre.y + 19, (int)view.top, (int)view.bottom - 1);
	if (x1 == x2 || y1 == y2)
		return;
	// C53E reloads 2772 for each edge; the previous edge's phase is discarded.
	drawPatternedMapLine(screen, pitch, Common::Point(x1, y1), Common::Point(x2, y1), view, 251, 0x5555);
	drawPatternedMapLine(screen, pitch, Common::Point(x1, y2), Common::Point(x2, y2), view, 251, 0x5555);
	drawPatternedMapLine(screen, pitch, Common::Point(x1, y1), Common::Point(x1, y2), view, 251, 0x5555);
	drawPatternedMapLine(screen, pitch, Common::Point(x2, y1), Common::Point(x2, y2), view, 251, 0x5555);
}

Common::Point clampSurveyWindowPosition(const Common::Point &position) {
	return Common::Point(CLIP<int>(position.x, 5, 145), CLIP<int>(position.y, 5, 29));
}

void drawSurveyDragOutline(byte *screen, int pitch, const Common::Point &position) {
	// DNVGA 181/3724 XORs the outline with 0x0F, touching each corner once.
	// 5978 initially outlines 79EE's (92,30) handover frame unchanged;
	// only a subsequent 59D1 motion applies the top <=29 drag clamp.
	const Common::Point origin = position == Common::Point(92, 30)
		? position : clampSurveyWindowPosition(position);
	for (int x = origin.x; x < origin.x + 170; x++) {
		screen[origin.y * pitch + x] ^= 0x0f;
		screen[(origin.y + 107) * pitch + x] ^= 0x0f;
	}
	for (int y = origin.y + 1; y < origin.y + 107; y++) {
		screen[y * pitch + origin.x] ^= 0x0f;
		screen[y * pitch + origin.x + 169] ^= 0x0f;
	}
}

void MapPointerTarget::consider(Common::Point from, Common::Point direction, Common::Point candidate) {
	const int dx = candidate.x - from.x;
	const int dy = candidate.y - from.y;
	int distance;
	if (direction.x && direction.y) {
		const int alongX = dx * direction.x, alongY = dy * direction.y;
		if (alongX < 6 || alongY < 6)
			return;
		distance = ABS(alongX - alongY);
	} else {
		const int along = direction.x ? dx * direction.x : dy * direction.y;
		const int across = direction.x ? ABS(dy) : ABS(dx);
		if (along < 6 || across >= 50)
			return;
		distance = along + 2 * across;
	}
	if (distance < score) {
		point = candidate;
		score = distance;
	}
}

void MapPointerTarget::consider(Common::Point from, Common::Point direction, const Common::Rect &bounds) {
	// D6FE uses unsigned strict bounds. A rectangle that begins offscreen
	// remains eligible even when the cursor is over its visible portion.
	const uint16 x = from.x, y = from.y;
	if (x > uint16(bounds.left) && x < uint16(bounds.right) &&
			y > uint16(bounds.top) && y < uint16(bounds.bottom))
		return;
	consider(from, direction, Common::Point(uint16(bounds.left + bounds.right) / 2,
			bounds.bottom - uint16(bounds.height()) / 4));
}

void MapPointerGlide::start(Common::Point from, Common::Point to, uint16 ticks) {
	position = from;
	target = to;
	lastTick = ticks;
	steps = 100;
}

bool MapPointerGlide::advance(uint16 ticks) {
	const byte elapsed = ticks - lastTick;
	if (!steps || elapsed < 6)
		return false;
	lastTick = ticks;
	const int divisor = elapsed < 12 ? 4 : elapsed < 24 ? 2 : 1;
	auto move = [divisor](int delta) {
		if (!delta)
			return 0;
		if (divisor == 1)
			return delta - (delta - (delta < 0 ? 3 : 0)) / 4;
		const int part = (delta - (delta < 0 ? divisor - 1 : 0)) / divisor;
		return part % 2 ? part : part + 1;
	};
	const Common::Point delta(move(target.x - position.x), move(target.y - position.y));
	steps = delta == Common::Point() ? 0 : steps - 1;
	position = Common::Point(CLIP<int>(position.x + delta.x, 0, 319),
			CLIP<int>(position.y + delta.y, 0, 199));
	return delta != Common::Point();
}

SurveyMapControl surveyMapControlAt(const Common::Rect &frame, const Common::Point &at) {
	if (at.y >= 155)
		return kSurveyPointer;
	// D6FE's interior is strict on all four sides.
	if (at.x > frame.left && at.x < frame.right && at.y > frame.top && at.y < frame.bottom)
		return kSurveyHand;
	if (at.y >= frame.top && at.y < frame.bottom) {
		if ((uint16)(frame.left - at.x) < 50)
			return kSurveyWest;
		if ((uint16)(at.x - frame.right) < 50)
			return kSurveyEast;
	} else if (at.x >= frame.left && at.x < frame.right) {
		if ((uint16)(frame.top - at.y) < 25)
			return kSurveyNorth;
		if ((uint16)(at.y - frame.bottom) < 25)
			return kSurveySouth;
	}
	return kSurveyPointer;
}

} // End of namespace Dune
