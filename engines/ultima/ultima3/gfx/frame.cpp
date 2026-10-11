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

#include "ultima/ultima3/gfx/charset.h"
#include "ultima/ultima3/gfx/frame.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

// The characters the frame is made of: the ends of each run have a corner
constexpr char GLYPH_HORIZONTAL = 0x0D;
constexpr char GLYPH_VERTICAL = 0x0E;
constexpr char GLYPH_CORNER = 0x0F;

constexpr int GAME_VIEW_SIZE = 24;
constexpr int PANEL_COL = 23;
constexpr int PANEL_END = 39;
constexpr int PANEL_ROWS = 17;
constexpr int PANEL_SLOT_ROWS = 4;

constexpr int WINDOW_TOP = 10;
constexpr int WINDOW_BOTTOM = 23;
constexpr int WINDOW_LAST_COL = 39;

// The ornaments on the top of the window sit half a cell in from the corners
constexpr int ORNAMENT_LEFT = 2 * 8 + 4;
constexpr int ORNAMENT_RIGHT = 36 * 8 + 4;

using Graphics::Views::GfxSurface;

static void horizontal(GfxSurface &s, int col, int row, int count) {
	for (int i = 0; i < count; ++i)
		s.writeChar(Common::Point(col + i, row), (i == 0 || i == count - 1) ? GLYPH_CORNER : GLYPH_HORIZONTAL);
}

static void vertical(GfxSurface &s, int col, int row, int count) {
	for (int i = 0; i < count; ++i)
		s.writeChar(Common::Point(col, row + i), (i == 0 || i == count - 1) ? GLYPH_CORNER : GLYPH_VERTICAL);
}

void drawGameFrame(GfxSurface &s) {
	vertical(s, 0, 0, GAME_VIEW_SIZE);
	horizontal(s, 0, 0, GAME_VIEW_SIZE);
	vertical(s, PANEL_COL, 0, GAME_VIEW_SIZE);
	horizontal(s, 0, WINDOW_BOTTOM, GAME_VIEW_SIZE);
	vertical(s, PANEL_END, 0, PANEL_ROWS);

	// The party members' panels are divided by lines
	for (int row = 0; row < PANEL_ROWS; row += PANEL_SLOT_ROWS)
		horizontal(s, PANEL_COL, row, PANEL_ROWS);
}

void drawWindowFrame(GfxSurface &s) {
	horizontal(s, 0, WINDOW_BOTTOM, WINDOW_LAST_COL + 1);
	horizontal(s, 0, WINDOW_TOP, WINDOW_LAST_COL + 1);

	// The ornaments are half way between character cells
	CharSet font;
	font.drawChar(s.surfacePtr(), GLYPH_CORNER, ORNAMENT_LEFT, WINDOW_TOP * 8, 0);
	font.drawChar(s.surfacePtr(), GLYPH_CORNER, ORNAMENT_RIGHT, WINDOW_TOP * 8, 0);
	s.addDirtyRect(Common::Rect(0, WINDOW_TOP * 8, 320, (WINDOW_TOP + 1) * 8));

	vertical(s, 0, WINDOW_TOP, WINDOW_BOTTOM - WINDOW_TOP + 1);
	vertical(s, WINDOW_LAST_COL, WINDOW_TOP, WINDOW_BOTTOM - WINDOW_TOP + 1);
}

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima
