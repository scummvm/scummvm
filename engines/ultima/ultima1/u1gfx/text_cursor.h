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

#ifndef ULTIMA_ULTIMA1_U1GFX_TEXT_CURSOR_H
#define ULTIMA_ULTIMA1_U1GFX_TEXT_CURSOR_H

#include "ultima/shared/gfx/text_cursor.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

class U1TextCursor : public Shared::Gfx::TextCursor {
private:
	const byte &_fgColor, &_bgColor;
	int _frameNum;
	uint32 _lastFrameFrame;
private:
	/**
	 * Get the current game milliseconds
	 */
	uint32 getTime();
public:
	/**
	 * Constructor
	 */
	U1TextCursor(const byte &fgColor, const byte &bgColor);

	/**
	 * Destructor
	 */
	~U1TextCursor() override {}

	/**
	 * Update the cursor
	 */
	void update() override;

	/**
	 * Draw the cursor
	 */
	void draw() override;
};

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
