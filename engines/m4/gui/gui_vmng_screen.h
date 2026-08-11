
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

#ifndef M4_GUI_GUI_VMNG_SCREEN_H
#define M4_GUI_GUI_VMNG_SCREEN_H

#include "m4/m4_types.h"
#include "m4/gui/gui_univ.h"

namespace M4 {

/**
 * Change which procedure will be called when a portion of the window
 * needs to be redrawn.
 * @param scrnContent	The window identifier
 * @param  redraw		The new refresh function pointer
 */
bool vmng_SetScreenRefresh(void *scrnContent, RefreshFunc redraw);

/**
 * Obvious shell to MoveScreen
 */
bool MoveScreenAbs(ScreenContext *myScreen, int32 parmX, int32 parmY);

/**
 * Obvious shell to MoveScreen
 */
bool MoveScreenDelta(ScreenContext *myScreen, int32 parmX, int32 parmY);
bool MoveScreenDelta(int32 parmX, int32 parmY);

/**
 * Resize the windows width and/or height
 * @param newW		The new width of the window.  If <= 0, the old window width will remain
 * @param newH		The new height of the window.  If <= 0, the old window height will remain.
 * @returns		TRUE if the window was found, FALSE otherwise
 */
bool ResizeScreen(void *scrnContent, int32 newW, int32 newH);

/**
 * Restore the monitor image by redrawing the visible portions of each window
 * intersecting the given rectangle.
 * @param updateX1	Rectangle top-left X
 * @param updateY1	Rectangle top-left Y
 * @param updateX2	Rectangle bottom-right X
 * @param updateY2	Rectangle bottom-right Y
 */
void RestoreScreens(int32 updateX1, int32 updateY1, int32 updateX2, int32 updateY2);

/**
 * A shell to RestoreScreens
 */
void RestoreScreensInContext(int32 x1, int32 y1, int32 x2, int32 y2, ScreenContext *myScreen);

} // End of namespace M4

#endif
