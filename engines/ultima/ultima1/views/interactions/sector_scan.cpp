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

#include "ultima/ultima1/views/interactions/sector_scan.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

// Same inset-dialog geometry the other space/overworld dialogs use
constexpr int DIALOG_X1 = 31, DIALOG_Y1 = 23, DIALOG_X2 = 287, DIALOG_Y2 = 127;
constexpr int BORDER_X1 = 34, BORDER_Y1 = 26, BORDER_X2 = 284, BORDER_Y2 = 124;

constexpr int TEXT_COLOR = 15;
constexpr int FRAME_COLOR = 1;
constexpr int MARKER_COLOR = 15;
constexpr int EMPTY_HILIGHT_COLOR = 14;
constexpr int EMPTY_DIM_COLOR = 1;

// The scan grid's own outline, and where sector (sx, sy)'s glyph sits
// within it - matching the original's drawSectorMapIcon
constexpr int GRID_X1 = 130, GRID_Y1 = 44, GRID_X2 = 200, GRID_Y2 = 115;
static int cellX(int sx) { return sx * 10 + 132; }
static int cellY(int sy) { return sy * 10 + 46; }

// tick() runs ~20/sec; the original waits 7 BIOS ticks (~0.38s) between
// blinks
constexpr int BLINK_TICKS = 8;

bool SectorScan::msgFocus(const FocusMessage &msg) {
	setBounds(Common::Rect(0, 0, 320, 200));
	_blinkCtr = 0;
	_blipVisible = true;
	return true;
}

void SectorScan::draw() {
	auto s = getSurface();

	// Inset dialog box
	s.fillRect(Rect(DIALOG_X1, DIALOG_Y1, DIALOG_X2, DIALOG_Y2), 0);
	s.frameRect(Rect(BORDER_X1, BORDER_Y1, BORDER_X2, BORDER_Y2), FRAME_COLOR);

	s.setColor(TEXT_COLOR);
	s.writeString(Point(5, 4), "Sector Scan:");

	// The scan grid outline
	s.drawLine(GRID_X1, GRID_Y1, GRID_X2, GRID_Y1, MARKER_COLOR);
	s.drawLine(GRID_X2, GRID_Y1, GRID_X2, GRID_Y2, MARKER_COLOR);
	s.drawLine(GRID_X2, GRID_Y2, GRID_X1, GRID_Y2, MARKER_COLOR);
	s.drawLine(GRID_X1, GRID_Y2, GRID_X1, GRID_Y1, MARKER_COLOR);

	// One glyph per sector
	for (int sx = 0; sx < Data::SPACE_SECTOR_COUNT; ++sx)
		for (int sy = 0; sy < Data::SPACE_SECTOR_COUNT; ++sy)
			drawSectorIcon(s, sx, sy);

	// Blink the player's current sector by blanking its glyph on the off
	// beat
	if (!_blipVisible) {
		int px = cellX(_G(sectorX)), py = cellY(_G(sectorY));
		s.fillRect(Rect(px, py, px + 8, py + 9), 0);
	}
}

void SectorScan::drawSectorIcon(Shared::Gfx::GfxSurface &s, int sx, int sy) {
	const Data::SpaceMapCell &cell = _G(spaceMap)._sectors[sx][sy];
	int px = cellX(sx), py = cellY(sy);

	if (cell._enemyCount != 0)
		drawEnemyMarker(s, px, py);
	else if (cell._anchorX != 0)
		drawStationMarker(s, px, py);
	else if (cell._hazardX != 0)
		drawStarMarker(s, px, py);
	else
		drawEmptyMarker(s, px, py);
}

void SectorScan::drawEnemyMarker(Shared::Gfx::GfxSurface &s, int x, int y) {
	// Two side bars each end, two rungs across the middle - a fighter shape
	for (int i = 0; i < 2; ++i) {
		s.drawLine(x + i, y, x + i, y + 7, MARKER_COLOR);
		s.drawLine(x + i + 5, y, x + i + 5, y + 7, MARKER_COLOR);
		s.drawLine(x, y + 3 + i, x + 6, y + 3 + i, MARKER_COLOR);
	}
}

void SectorScan::drawStationMarker(Shared::Gfx::GfxSurface &s, int x, int y) {
	// A cross
	for (int i = 0; i < 2; ++i) {
		s.drawLine(x + i + 3, y, x + i + 3, y + 7, MARKER_COLOR);
		s.drawLine(x, y + i + 3, x + 7, y + i + 3, MARKER_COLOR);
	}
}

void SectorScan::drawStarMarker(Shared::Gfx::GfxSurface &s, int x, int y) {
	// A small diamond
	for (int i = 0; i < 3; ++i) {
		s.drawLine(x + 3 - i, y + i + 1, x + 3 + i, y + i + 1, MARKER_COLOR);
		s.drawLine(x + 3 - i, y + 6 - i, x + 3 + i, y + 6 - i, MARKER_COLOR);
	}
}

void SectorScan::drawEmptyMarker(Shared::Gfx::GfxSurface &s, int x, int y) {
	// A sparse scattering of dots
	s.setPixel(x + 1, y + 1, MARKER_COLOR);
	s.setPixel(x + 3, y + 3, EMPTY_HILIGHT_COLOR);
	s.setPixel(x + 6, y + 2, EMPTY_DIM_COLOR);
	s.setPixel(x + 4, y + 5, MARKER_COLOR);
	s.setPixel(x + 2, y + 6, EMPTY_DIM_COLOR);
}

bool SectorScan::tick() {
	if (++_blinkCtr >= BLINK_TICKS) {
		_blinkCtr = 0;
		_blipVisible = !_blipVisible;
		redraw();
	}

	return Interaction::tick();
}

bool SectorScan::msgAction(const ActionMessage &msg) {
	close();
	return true;
}

bool SectorScan::msgKeypress(const KeypressMessage &msg) {
	close();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
