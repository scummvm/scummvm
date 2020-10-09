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

#include "common/array.h"
#include "common/rect.h"

namespace Graphics {
struct Surface;
}

namespace Stark {

class Cursor;

namespace Gfx {
class Driver;
}

namespace Resources {
	typedef Common::Array<uint32> ActionArray;
}

/**
 * A window is a portion of the game screen.
 *
 * A window can handle events happening in its screen portion,
 * as well as render graphics to that same screen portion.
 *
 * Coordinates inside a window are relative to its top left corner.
 * Rendering happens in a viewport matching the window's screen portion.
 *
 */
class Window {
public:
	Window(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~Window();

	/** Called by the user interface when the mouse moves inside the window */
	void handleMouseMove();

	/** Called by the user interface when the mouse is clicked inside the window */
	void handleClick();

	/** Called by the user interface when the mouse is right clicked inside the window */
	void handleRightClick();

	/** Called by the user interface when the mouse is double clicked inside the window */
	void handleDoubleClick();

	/** Called once per game loop when the screen is active. */
	void handleGameLoop();

	/** Called by the user interface in the render phase of the game loop */
	void render();

	/** Is the mouse inside the window? */
	bool isMouseInside() const;

	/** Is the window visible */
	bool isVisible() const;

	/** Grab a screenshot of the window if it is visible */
	Graphics::Surface *getScreenshot() const;

protected:
	virtual void onMouseMove(const Common::Point &pos) {}
	virtual void onClick(const Common::Point &pos) {}
	virtual void onRightClick(const Common::Point &pos) {}
	virtual void onDoubleClick(const Common::Point &pos) {}
	virtual void onGameLoop() {};
	virtual void onRender() = 0;

	Common::Point getRelativeMousePosition() const;

	Gfx::Driver *_gfx;
	Cursor *_cursor;

	Common::Rect _position;
	bool _visible;
};

} // End of namespace Stark

 #endif // STARK_UI_WINDOW_H
