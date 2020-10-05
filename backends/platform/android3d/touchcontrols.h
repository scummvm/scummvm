/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ANDROID_TOUCHCONTROLS_H_
#define ANDROID_TOUCHCONTROLS_H_

#if defined(__ANDROID__)

#include "common/events.h"

#include "backends/platform/android3d/events.h"
#include "backends/platform/android3d/texture.h"

class TouchControls {
public:
	TouchControls();
	~TouchControls();

	void init(int width, int height);
	void draw();
	void update(int ptr, int action, int x, int y);

private:
	int _screen_width, _screen_height;

	enum TouchArea{
		kTouchAreaJoystick = 0xffff,
		kTouchAreaCenter = 0xfffe,
		kTouchAreaRight = 0xfffd,
		kTouchAreaNone = 0xfffc,
	};

	uint16 getTouchArea(int x, int y);

	struct Pointer {
		uint16 startX, startY;
		uint16 currentX, currentY;
		TouchArea function;
		bool active;
	};

	enum { kNumPointers = 5 };
	Pointer _pointers[kNumPointers];
	int _activePointers[4];
	Common::KeyCode _joystickPressing, _centerPressing, _rightPressing;
	int &pointerFor(TouchArea ta);
	GLESTexture *_arrows_texture;
	void keyDown(Common::KeyCode kc);
	void keyUp(Common::KeyCode kc);
	void keyPress(Common::KeyCode kc);
};

#endif

#endif
