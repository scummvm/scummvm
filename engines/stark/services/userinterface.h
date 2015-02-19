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

#ifndef STARK_SERVICES_USER_INTERFACE_H
#define STARK_SERVICES_USER_INTERFACE_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Stark {

namespace Gfx {
class Driver;
class Texture;
}

/**
 * Facade object for interacting with the game world
 */
class UserInterface {
public:
	UserInterface(Gfx::Driver *driver);
	~UserInterface();

	/** Skip currently playing speeches */
	void skipCurrentSpeeches();

	/** Scroll the current location by an offset */
	void scrollLocation(int32 dX, int32 dY);

	/** Update the mouse position */
	void setMousePosition(Common::Point pos);

	/** Draw the mouse pointer, and any additional currently active UI */
	void render();
private:
	Common::Point _mousePos;
	Gfx::Texture *_cursorTexture;
	Gfx::Driver *_gfx;
};

} // End of namespace Stark

#endif // STARK_SERVICES_USER_INTERFACE_H
