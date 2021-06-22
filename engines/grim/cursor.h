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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRIM_CURSOR_H_
#define GRIM_CURSOR_H_

#include "common/rect.h"

namespace Grim {

class GrimEngine;
class Bitmap;

class Cursor {
public:
	Cursor(GrimEngine *vm);
	~Cursor();

	Common::Point getPosition() { return _position; }
	void updatePosition(Common::Point &mouse) { _position = mouse; }
	void setCursor(int id) { _curCursor = id; }
	void reload();
	void setPersistent(int id, int x=-1, int y=-1);

	void draw();
private:

	int _hotspotx, _hotspoty;
	int _curCursor, _persistentCursor;

	Common::Point _position;
	Common::Point _persistentPosition;

	void loadAvailableCursors();
	Bitmap** _bitmaps;
};

} // namespace Grim

#endif
