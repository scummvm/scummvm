/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_CURSOR_H
#define STARK_CURSOR_H

#include "common/rect.h"
#include "common/scummsys.h"
#include "engines/stark/gfx/renderentry.h"

namespace Stark {

class VisualImageXMG;

namespace Gfx {
class Driver;
class Texture;
}

/** 
 * Manager for the current game Cursor
 */
class Cursor {
public:
	Cursor(Gfx::Driver *gfx);
	~Cursor();

	void init();

	/**
	 * Render the Cursor
	 */
	void render();

	/** Update the mouse position */
	void setMousePosition(Common::Point pos);

	Common::Point getMousePosition() const;

	enum CursorType {
		kNone = -1,
		kDefault = 0,
		kActive = 3,
		kPassive = 9
	};

	void setCursorType(CursorType type);
	/** Check whether the current mouse position intersects some renderEntry */
	void handleMouseOver(Gfx::RenderEntryArray renderEntries);
	void handleClick();
private:
	
	Gfx::Driver *_gfx;

	Common::Point _mousePos;
	VisualImageXMG *_cursorImage;
	Gfx::Texture *_mouseText;
	Gfx::RenderEntry *_mouseOverEntry;
	CursorType _currentCursorType;
};

} // End of namespace Stark

#endif // STARK_CURSOR_H
