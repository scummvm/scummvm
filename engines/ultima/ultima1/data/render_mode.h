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

#ifndef ULTIMA1_DATA_RENDER_MODE_H
#define ULTIMA1_DATA_RENDER_MODE_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

/**
 * The graphics the game can be shown with. The original also had a mode for
 * the Tandy 1000, but it draws the very same pictures as EGA, only stored
 * differently, so there's nothing to choose between them
 */
enum RenderMode {
	RENDER_CGA = 0,		// 4 colors
	RENDER_EGA = 1,		// 16 colors
	RENDER_MODE_COUNT = 2
};

// Descriptions of the modes for the launcher's graphics mode drop-down, in
// the same order. Marked for translation, but not translated
extern const char *const RENDER_MODE_LABELS[RENDER_MODE_COUNT];

/**
 * The palette indices the game draws things with, which differ between the
 * modes since CGA only has four colors to share between everything
 */
struct RenderColors {
	byte _edge = 15;			// White lines and the most common highlight
	byte _border = 1;			// Frames around the screens
	byte _text = 11;			// Text and values
	byte _highlight = 12;		// Warnings, laser fire and other emphasis
	byte _shuttle = 12;			// The body of a ship's exhaust trail

	// Colors the original chooses a different way for CGA, rather than
	// by their role in the game
	byte _cockpitFrame = 7;
	byte _star1 = 12, _star2 = 14;
	byte _stationHighlight = 1, _stationBorder = 10, _stationEdge = 7;
	byte _flag1 = 11, _flag2 = 10;		// The two colors of the flags over the castle

	/**
	 * Sets the colors to use for a mode
	 */
	void set(RenderMode mode);
};

/**
 * Sets the screen palette for a mode
 */
void setPalette(RenderMode mode);

} // namespace Data
} // namespace Ultima1
} // namespace Ultima

#endif
