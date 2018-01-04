/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef BLADERUNNER_MOUSE_H
#define BLADERUNNER_MOUSE_H

#include "bladerunner/vector.h"

namespace Graphics {
	struct Surface;
}

namespace BladeRunner {

class BladeRunnerEngine;

class Mouse {
	BladeRunnerEngine *_vm;

	int _cursor;
	int _frame;
	int _hotspotX;
	int _hotspotY;
	int _x;
	int _y;
	int _disabledCounter;
	int _lastFrameTime;
	int _animCounter;

public:
	Mouse(BladeRunnerEngine *vm);
	~Mouse();

	void setCursor(int cursor);

	void getXY(int *x, int *y);

	void disable();
	void enable();
	bool isDisabled();

	void draw(Graphics::Surface &surface, int x, int y);
	void updateCursorFrame();

	void tick(int x, int y);

// private:
	Vector3 getXYZ(int x, int y);
};

} // End of namespace BladeRunner

#endif
