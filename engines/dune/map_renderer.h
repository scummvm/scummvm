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

#ifndef DUNE_MAP_RENDERER_H
#define DUNE_MAP_RENDERER_H

#include "common/rect.h"

namespace Dune {

struct MapLatitude {
	uint16 offset;
	uint16 halfWidth;
	uint16 step;

	// DOS 00E7's travel/contact-range scale, not the projection scale above.
	uint longitudeStep() const {
		return halfWidth ? (65536u + halfWidth) / (2 * halfWidth) : 256;
	}
};

// DNVGA's strategic view: 36 strips of four interpolated lines, inset four
// pixels from the 320x152 scene. The 120-byte polar edge table is asset data.
void drawStrategicMap(byte *screen, int pitch, const byte *world, uint worldSize,
		const MapLatitude *latitudes, const byte *edges, uint16 longitude, int latitude);

// DOS 0xB647 projects a signed longitude difference using this row's width.
Common::Point strategicMapPosition(const MapLatitude *latitudes,
		uint16 centreLongitude, int centreLatitude, uint16 longitude, int latitude);

typedef void (*MapDecorationCallback)(uint sprite, int x, int y, void *context);
// 633B/63C7: vegetation sprites over the interpolated terrain, before places.
void visitMapVegetation(const byte *world, uint worldSize, const MapLatitude *latitudes,
		uint16 longitude, int latitude, MapDecorationCallback draw, void *context);

// DOS 0x57E5 maps MAP2 deposit IDs to ONMAP palette indices, using the live
// place records' discovery, survey and remaining-reserve bytes.
void buildSpiceDensityColours(byte *colours, const byte *state, uint stateSize);

struct SurveyTroopCounts {
	byte occupation[3]; // Mining, military, ecology; counts of troops, not men.
};

// DOS 6639/5728: the place's standing chain followed by incoming troops.
SurveyTroopCounts countSurveyTroops(const byte *state, uint stateSize, uint16 place);
// DOS 586E packs four horizontal occupation bands into four two-bit fields.
byte surveyTroopPattern(const SurveyTroopCounts &counts);
void buildTroopDistributionColours(byte *colours, const byte *state, uint stateSize);

int clampSurveyLatitude(int latitude, int height);
Common::Point surveyMapPosition(const MapLatitude *latitudes,
		uint16 centreLongitude, int centreLatitude, uint16 longitude, int latitude,
		const Common::Rect &view);
// B6C3/B7D2 row preparation and DNVGA 0x17E's density/boundary/polar drawing.
// The 18-byte polar edge table is read from DNVGA CS:2384.
void drawSurveyMap(byte *screen, int pitch, const byte *world, uint worldSize,
		const MapLatitude *latitudes, const byte *edges, const byte *colours,
		uint16 longitude, int latitude, const Common::Rect &view, bool troopMode = false,
		byte *scratch = nullptr, uint scratchSize = 0);

// DOS B6C3's compact MAP.HSQ path and DNVGA 0x142: terrain is in the low nibble.
void drawTerrainInset(byte *screen, int pitch, const byte *world, uint worldSize,
		const MapLatitude *latitudes, const byte *edges,
		uint16 longitude, int latitude, const Common::Rect &view,
		byte *scratch = nullptr, uint scratchSize = 0);

// DNVGA 139/1A07. The clip rectangle is exclusive and inside the framebuffer.
// Returns the advanced 16-bit dash pattern (the original driver's BP).
uint16 drawPatternedMapLine(byte *screen, int pitch, Common::Point from, Common::Point to,
		const Common::Rect &clip, byte colour, uint16 pattern);
// DOS 54BE/C560 clamps and outlines the projected strategic viewport.
void drawSurveyViewport(byte *screen, int pitch, const Common::Point &centre,
		const Common::Rect &view);

// DOS 59D1 clamps the standalone survey's 170x108 frame to the scene.
Common::Point clampSurveyWindowPosition(const Common::Point &position);
void drawSurveyDragOutline(byte *screen, int pitch, const Common::Point &position);
enum SurveyMapControl {
	kSurveyPointer = -2, kSurveyHand = -1,
	kSurveyNorth = 0, kSurveyEast, kSurveySouth, kSurveyWest
};
// DC6A uses the whole frame, not the inset's terrain bounds.
SurveyMapControl surveyMapControlAt(const Common::Rect &frame, const Common::Point &at);

/** E159/E16F: keyboard targets are visited in panel, actor and place order. */
struct MapPointerTarget {
	Common::Point point;
	int score = 0x8000;

	void consider(Common::Point from, Common::Point direction, Common::Point candidate);
	void consider(Common::Point from, Common::Point direction, const Common::Rect &bounds);
};

/** D962: move towards the selected target on the 200 Hz input clock. */
struct MapPointerGlide {
	Common::Point position, target;
	uint16 lastTick = 0;
	int steps = 0;

	void start(Common::Point from, Common::Point to, uint16 ticks);
	bool advance(uint16 ticks);
};

/** D8DA/D935: repeat the originally pressed pan button every fifty ticks. */
struct MapPanRepeat {
	int direction = -1;
	uint16 lastTick = 0;

	void press(int way, uint16 ticks) {
		if (way < 0 || way > 3)
			return;
		direction = way;
		lastTick = ticks;
	}
	void release(uint16 ticks) {
		if (direction >= 0) {
			direction = -1;
			lastTick = ticks;
		}
	}
	bool poll(int way, uint16 ticks) {
		if (direction < 0 || direction != way || (uint16)(ticks - lastTick) < 50)
			return false;
		lastTick = ticks; // One callback, not a catch-up loop after a stall.
		return true;
	}
};

} // End of namespace Dune

#endif
