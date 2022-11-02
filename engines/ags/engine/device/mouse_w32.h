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

#ifndef AGS_ENGINE_DEVICE_MOUSEW32_H
#define AGS_ENGINE_DEVICE_MOUSEW32_H

#include "ags/shared/util/geometry.h"
#include "ags/shared/util/geometry.h"

namespace AGS3 {
namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

void mgetgraphpos();
// Sets the area of the game frame (zero-based coordinates) where the mouse cursor is allowed to move;
// this function was meant to be used to achieve gameplay effect
void msetcursorlimit(int x1, int y1, int x2, int y2);
void msetgraphpos(int xa, int ya);
void msethotspot(int xx, int yy);
int minstalled();

struct Mouse {
	// Tells whether mouse was locked to the game window
	bool LockedToWindow = false;

	// Screen rectangle, in which the mouse movement is controlled by engine
	Rect  ControlRect;
	// Mouse control enabled flag
	bool  ControlEnabled = false;
	// Flag that tells whether the mouse must be forced to stay inside control rect
	bool  ConfineInCtrlRect = false;
	// Mouse speed value provided by user
	float SpeedVal = 1.f;
	// Mouse speed unit
	float SpeedUnit = 1.f;
	// Actual speed factor (cached)
	float Speed = 1.f;


	void WindowToGame(int &x, int &y);

	// Get if mouse is locked to the game window
	bool IsLockedToWindow();
	// Try locking mouse to the game window
	bool TryLockToWindow();
	// Unlock mouse from the game window
	void UnlockFromWindow();

	// Tell if the mouse movement control is enabled
	bool IsControlEnabled() const;
	// Set base speed factor, which would serve as a mouse speed unit
	void SetSpeedUnit(float f);
	// Get base speed factor
	float GetSpeedUnit();
	// Set speed factors
	void SetSpeed(float speed);
	// Get speed factor
	float GetSpeed();

	// Limits the area where the game cursor can move on virtual screen;
	// parameter must be in native game coordinates
	void SetMoveLimit(const Rect &r);
	// Set actual OS cursor position on screen; parameter must be in native game coordinates
	void SetPosition(const Point p);

	void UpdateGraphicArea();
	void SetMovementControl(bool flag);
};

} // namespace AGS3

#endif
