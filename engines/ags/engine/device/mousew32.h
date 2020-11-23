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

//=============================================================================
//
// MOUSELIBW32.CPP
//
// Library of mouse functions for graphics and text mode
//
// (c) 1994 Chris Jones
// Win32 (allegro) update (c) 1999 Chris Jones
//
//=============================================================================

#include "ags/shared/util/geometry.h"

#define MAXCURSORS 20

#include "ags/shared/util/geometry.h"

namespace AGS3 {
namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

void msetgraphpos(int, int);
// Sets the area of the screen within which the mouse can move
void mgraphconfine(int x1, int y1, int x2, int y2);
void mgetgraphpos();
// Sets the area of the game frame (zero-based coordinates) where the mouse cursor is allowed to move;
// this function was meant to be used to achieve gameplay effect
void msetcursorlimit(int x1, int y1, int x2, int y2);
int ismouseinbox(int lf, int tp, int rt, int bt);
void mfreemem();
void mloadwcursor(char *namm);
void msetgraphpos(int xa, int ya);
void msethotspot(int xx, int yy);
int minstalled();

namespace Mouse {
// Get if mouse is locked to the game window
bool IsLockedToWindow();
// Try locking mouse to the game window
bool TryLockToWindow();
// Unlock mouse from the game window
void UnlockFromWindow();

// Enable mouse movement control
void EnableControl(bool confine);
// Disable mouse movement control
void DisableControl();
// Tell if the mouse movement control is enabled
bool IsControlEnabled();
// Set base speed factor, which would serve as a mouse speed unit
void SetSpeedUnit(float f);
// Get base speed factor
float GetSpeedUnit();
// Set speed factors
void SetSpeed(float speed);
// Get speed factor
float GetSpeed();
}

namespace Mouse {
// Updates limits of the area inside which the standard OS cursor is not shown;
// uses game's main viewport (in native coordinates) to calculate real area on screen
void SetGraphicArea();
// Limits the area where the game cursor can move on virtual screen;
// parameter must be in native game coordinates
void SetMoveLimit(const Rect &r);
// Set actual OS cursor position on screen; parameter must be in native game coordinates
void SetPosition(const Point p);
}


extern int mousex, mousey;
extern int hotx, hoty;
extern int disable_mgetgraphpos;
extern char currentcursor;

extern Shared::Bitmap *mousecurs[MAXCURSORS];

} // namespace AGS3
