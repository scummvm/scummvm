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

#ifndef STARK_UI_WINDOW_H
#define STARK_UI_WINDOW_H

#include "common/rect.h"

#include "engines/stark/cursor.h"

namespace Stark {

class Cursor;

namespace Gfx {
class Driver;
}

namespace Resources {
	typedef Common::Array<uint32> ActionArray;
}

class Window {
public:
	Window(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~Window();

	void handleMouseMove();
	void handleClick();

	void render();

	bool isMouseInside() const;
	bool isVisible() const;

protected:
	virtual void onMouseMove(const Common::Point &pos) = 0;
	virtual void onClick(const Common::Point &pos) = 0;
	virtual void onRender() = 0;

	void setCursor(Cursor::CursorType type);
	Common::Point getMousePosition() const;
	Common::Point getScreenMousePosition() const;

	Gfx::Driver *_gfx;

	Common::Rect _position;
	bool _unscaled;
	bool _visible;


	// Item handling
	void updateItems();
	void setCursorDependingOnActionsAvailable(Resources::ActionArray actionsAvailable);

	Gfx::RenderEntryArray _renderEntries;
	Resources::Item *_objectUnderCursor;
	Common::Point _objectRelativePosition;

private:
	Cursor *_cursor;
};

} // End of namespace Stark

 #endif // STARK_UI_WINDOW_H
