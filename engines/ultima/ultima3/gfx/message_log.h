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

#ifndef ULTIMA3_GFX_MESSAGE_LOG_H
#define ULTIMA3_GFX_MESSAGE_LOG_H

#include "common/str.h"
#include "graphics/views/gfx_surface.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

constexpr int LOG_LINES = 7;
constexpr int LOG_WIDTH = 16;

/**
 * The scrolling message window at the bottom right of the game screen: seven
 * lines of sixteen characters, with text always added to the bottom line
 */
class MessageLog {
private:
	Common::String _lines[LOG_LINES];
	int _column = 0;

	void scroll();

public:
	/**
	 * Clears the window
	 */
	void clear();

	/**
	 * Adds text at the cursor. A newline scrolls the window up, as does
	 * running off the end of the bottom line
	 */
	void print(const char *text);

	/**
	 * Adds a single character (including the glyphs used for prompts) at
	 * the cursor without wrapping
	 */
	void putChar(char ch);

	/**
	 * Takes back the last characters added to the bottom line
	 */
	void backspace(int count);

	/**
	 * Draws the window, with the cursor arrow shown if waiting for input
	 */
	void draw(Graphics::Views::GfxSurface &s, bool showCursor) const;
};

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
