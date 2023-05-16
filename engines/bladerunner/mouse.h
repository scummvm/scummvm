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
	uint32 _lastFrameTime;
	int _animCounter;

	int _randomCountdownX;
	int _randomCountdownY;
	int _randomX;
	int _randomY;

	uint16 _drawModeBitFlags; // replaces the additive bool with a set of bit flags (including flags for additive mode)

public:
	Mouse(BladeRunnerEngine *vm);
	~Mouse();

	void setCursor(int cursor);

	void getXY(int *x, int *y) const;
	void setMouseJitterUp();
	void setMouseJitterDown();

	void disable();
	void enable(bool force = false);
	bool isDisabled() const;

	void draw(Graphics::Surface &surface, int x, int y);
	void updateCursorFrame();

	void tick(int x, int y);
	bool isRandomized() const;
	bool isInactive() const;

	Vector3 getXYZ(int x, int y) const;

	typedef enum mouseDrawFlags {
		REDCROSSHAIRS    = 0x0001,
		YELLOWCROSSHAIRS = 0x0002,
		BLUECROSSHAIRS   = 0x0004,
		SPECIAL          = 0x0008,
		ADDITIVE_MODE0   = 0x0010,
		ADDITIVE_MODE1   = 0x0020,
		CUSTOM           = 0x0040,
		EXIT_UP          = 0x0080,
		EXIT_DOWN        = 0x0100,
		EXIT_LEFT        = 0x0200,
		EXIT_RIGHT       = 0x0400,
		ESPER_UP         = 0x0800,
		ESPER_DOWN       = 0x1000,
		ESPER_LEFT       = 0x2000,
		ESPER_RIGHT      = 0x4000
	} MouseDrawFlags;
};

} // End of namespace BladeRunner

#endif
