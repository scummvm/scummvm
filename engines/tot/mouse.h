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
#ifndef TOT_MOUSE_H
#define TOT_MOUSE_H

#include "common/rect.h"
#include "common/scummsys.h"

#include "tot/chrono.h"

namespace Tot {
struct MouseMask {
	void *mask;
	uint width;
	uint height;
};

class MouseManager {
public:
	MouseManager();
	~MouseManager();
	void drawMask(int idx);
	void setMousePos(int mask, int x, int y);
	void setMask(int maskNum);
	void animateMouseIfNeeded();
	void hide();
	void show();
	void setMouseArea(Common::Rect rect);
	void setMousePos(Common::Point p);
	void printPos(int x, int y, int screenPosX, int screenPosY);

private:
	Common::Rect _mouseArea;
	int _currentMouseMask = 0;
	MouseMask _mouseMasks[8];
	void loadMasks();
	void setMouseMask(int numMask);
};

} // End of namespace Tot

#endif
