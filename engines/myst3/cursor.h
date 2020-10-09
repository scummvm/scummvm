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

#ifndef CURSOR_H_
#define CURSOR_H_

#include "common/hashmap.h"
#include "common/rect.h"

#include "engines/myst3/gfx.h"

namespace Myst3 {

class Myst3Engine;
class Texture;

class Cursor : public Drawable {
public:
	Cursor(Myst3Engine *vm);
	virtual ~Cursor();

	void changeCursor(uint32 index);
	bool isPositionLocked() { return _lockedAtCenter; }
	void lockPosition(bool lock);

	/**
	 * Get the mouse cursor position
	 *
	 * By default it is in 640x480 equivalent coordinates
	 *
	 * @param scaled When false the position is in actual game screen coordinates.
	 * @return
	 */
	Common::Point getPosition(bool scaled = true);
	void updatePosition(const Common::Point &mouse);

	void getDirection(float &pitch, float &heading);

	void draw() override;
	void setVisible(bool show);
	bool isVisible();
private:
	Myst3Engine *_vm;

	uint32 _currentCursorID;
	int32 _hideLevel;

	/** Position of the cursor */
	Common::Point _position;

	typedef Common::HashMap<uint32, Texture *> TextureMap;
	TextureMap _textures;

	bool _lockedAtCenter;

	void loadAvailableCursors();
	float getTransparencyForId(uint32 cursorId);
};

} // End of namespace Myst3

#endif // CURSOR_H_
