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

#ifndef ULTIMA8_GUMPS_RESIZABLEGUMP_H
#define ULTIMA8_GUMPS_RESIZABLEGUMP_H

#include "ultima/ultima8/gumps/gump.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Base class for gumps that can doesn't have a static size.
 * An example of such would be the Console and the GameMap gumps
 */
class ResizableGump : public Gump {
protected:
	Gump::Position _dragPosition, _mousePosition;
	int32 _minWidth;
	int32 _minHeight;

public:
	ResizableGump();
	ResizableGump(int x, int y, int width, int height);
	~ResizableGump() override;

	void setMinSize(int minWidth, int minHeight) {
		_minWidth = minWidth;
		_minHeight = minHeight;
	}

	Gump *onMouseMotion(int32 mx, int32 my) override;
	void onMouseLeft() override;

	bool onDragStart(int32 mx, int32 my) override;
	void onDragStop(int32 mx, int32 my) override;
	void onDrag(int32 mx, int32 my) override;

private:
	Gump::Position getPosition(int32 mx, int32 my);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
