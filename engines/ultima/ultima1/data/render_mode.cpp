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

#include "common/system.h"
#include "common/translation.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "ultima/ultima1/data/render_mode.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

const char *const RENDER_MODE_LABELS[RENDER_MODE_COUNT] = {
	_s("CGA (4 colors)"), _s("EGA / Tandy (16 colors)")
};

// CGA's palette 1 at low intensity, as the original sets it up
static const byte CGA_PALETTE[4 * 3] = {
	0x00, 0x00, 0x00, // 0: black
	0x00, 0xaa, 0xaa, // 1: cyan
	0xaa, 0x00, 0xaa, // 2: magenta
	0xaa, 0xaa, 0xaa  // 3: white
};

void RenderColors::set(RenderMode mode) {
	if (mode == RENDER_CGA) {
		_edge = 3;
		_border = 2;
		_text = 3;
		_highlight = 1;
		_shuttle = _border;

		_cockpitFrame = _edge;
		_star1 = _border;
		_star2 = _edge;
		_stationHighlight = _highlight;
		_stationBorder = _border;
		_stationEdge = _edge;
		_flag1 = 1;
		_flag2 = 2;
	} else {
		_edge = 15;
		_border = 1;
		_text = 11;
		_highlight = 12;
		_shuttle = _highlight;

		_cockpitFrame = 7;
		_star1 = 12;
		_star2 = 14;
		_stationHighlight = 1;
		_stationBorder = 10;
		_stationEdge = 7;
		_flag1 = 11;
		_flag2 = 10;
	}
}

void setPalette(RenderMode mode) {
	if (mode == RENDER_EGA) {
		g_system->getPaletteManager()->setPalette(Graphics::Palette::createEGAPalette());
		return;
	}

	// Only the first four colors are ever used, so show any others as white
	Graphics::Palette palette(256);
	for (int i = 0; i < 256; ++i)
		palette.set(&CGA_PALETTE[3 * 3], i, 1);
	palette.set(CGA_PALETTE, 0, 4);

	g_system->getPaletteManager()->setPalette(palette);
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
