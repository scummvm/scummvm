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

#include "igor/igor.h"
#include "common/system.h"
#include "graphics/paletteman.h"

namespace Igor {

// Debug overlays for the scene mask.
//
// The decoded MSK (kept in _screenLayer2, see SCENE_LAYERS_MASK_AREAS.md) holds
// a region id per pixel; that id indexes _roomObjectAreasTable (the BOX table).
// These overlays recolour the regions so the data is visible on screen:
//
//   debugPaintWalkAreas()  -> regions whose BOX record has area  != 0
//   debugPaintHotspots()   -> regions whose BOX record has object != 0
//   debugPaintY1Lum()      -> regions whose BOX record has y1Lum  != 0
//   debugPaintY2Lum()      -> regions whose BOX record has y2Lum  != 0
//   debugPaintDeltaLum()   -> regions whose BOX record has deltaLum != 0
//
// y1Lum / y2Lum are the depth thresholds of the enableLight == 1 shading
// (`wd->y > y1Lum && wd->y <= y2Lum`, see ROOM_LIGHTING.md) and are compared
// against Igor's scanline, so they are quantised into 16 buckets of 16 lines
// (value >> 4). deltaLum is the amount subtracted from the pixel colour, whose
// observed range is 0..15 (e.g. part_10.cpp:62 sets 3), so its low nibble is
// painted directly. Regions with a zero value are left untouched, which also
// means "no threshold" and "no darkening" show up as background.
// Each region/object id gets a distinct colour from the 16-entry ramp written
// to palette entries 240..255, so different walk areas (and different objects)
// are told apart. debugClearOverlay() restores those palette entries from
// _currentPalette.
//
// The overlay is (re)applied by debugApplyOverlay(), called from waitForTimer()
// right before the frame is presented, so it survives scene redraws and Igor
// movement. It composites onto _screenVGA's top 144 rows into a scratch buffer;
// _screenVGA itself is left untouched.

static const uint8 kDebugOverlayPalette[16 * 3] = {
	63,  0,  0,   63, 32,  0,   63, 63,  0,   32, 63,  0,
	 0, 63,  0,    0, 63, 32,    0, 63, 63,    0, 32, 63,
	 0,  0, 63,   32,  0, 63,   63,  0, 63,   63,  0, 32,
	63, 63, 63,   32, 32, 32,   63, 32, 32,   32, 63, 32
};

void IgorEngine::debugPaintWalkAreas() {
	_debugOverlayMode = kOverlayWalkAreas;
	debugApplyOverlay();
}

void IgorEngine::debugPaintHotspots() {
	_debugOverlayMode = kOverlayHotspots;
	debugApplyOverlay();
}

void IgorEngine::debugPaintY1Lum() {
	_debugOverlayMode = kOverlayY1Lum;
	debugApplyOverlay();
}

void IgorEngine::debugPaintY2Lum() {
	_debugOverlayMode = kOverlayY2Lum;
	debugApplyOverlay();
}

void IgorEngine::debugPaintDeltaLum() {
	_debugOverlayMode = kOverlayDeltaLum;
	debugApplyOverlay();
}

void IgorEngine::debugClearOverlay() {
	_debugOverlayMode = kOverlayOff;
	// Undo the palette ramp written by debugApplyOverlay().
	setPaletteRange(240, 255);
}

void IgorEngine::debugApplyOverlay() {
	if (_debugOverlayMode == kOverlayOff) {
		return;
	}

	// Recolour palette entries 240..255 (6-bit -> 8-bit, same expansion as
	// updatePalette()) so each id is a distinct colour.
	uint8 pal[16 * 3];
	for (int i = 0; i < 16 * 3; ++i) {
		uint8 v = kDebugOverlayPalette[i];
		pal[i] = (v << 2) | (v >> 4);
	}
	_system->getPaletteManager()->setPalette(pal, 240, 16);

	memcpy(_debugOverlayBuffer, _screenVGA, 320 * 144);
	for (int y = 0; y < 144; ++y) {
		const uint8 *mask = _screenLayer2 + y * 320;
		uint8 *dst = _debugOverlayBuffer + y * 320;
		for (int x = 0; x < 320; ++x) {
			const RoomObjectArea &roa = _roomObjectAreasTable[mask[x]];
			uint8 id;
			switch (_debugOverlayMode) {
			case kOverlayWalkAreas:
				id = roa.area;
				break;
			case kOverlayHotspots:
				id = roa.object;
				break;
			case kOverlayY1Lum:
				id = roa.y1Lum >> 4;
				break;
			case kOverlayY2Lum:
				id = roa.y2Lum >> 4;
				break;
			default:
				id = roa.deltaLum & 0x0F;
				break;
			}
			if (id != 0) {
				dst[x] = 240 + (id & 0x0F);
			}
		}
	}
	_system->copyRectToScreen(_debugOverlayBuffer, 320, 0, _screenVGAVOffset, 320, 144);
}

} // End of namespace Igor
