
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

#ifndef M4_GUI_GUI_MOUSE_H
#define M4_GUI_GUI_MOUSE_H

#include "m4/m4_types.h"
#include "m4/gui/gui_univ.h"
#include "m4/mem/reloc.h"

namespace M4 {

constexpr uint32 kArrowCursor = 0;

struct Mouse_Globals {
	ScreenContext *_mouseScreen = nullptr;
	transSprite *_mouseScreenSource = nullptr;

	Buffer _mouseBuffer;							// A scratch buffer used by RedrawMouse()
	Common::String _mouseSeriesResource;
	MemHandle _mouseSeriesHandle = nullptr;
	int32 _mouseSeriesOffset = 0;
	int32 _mouseSeriesPalOffset = 0;
	M4sprite *_mouseSprite = nullptr;			// The specific sprite which is drawn to indicate the current mouse position.
	int32 _mouseX1offset = 0;					// These two are the "x" and "Y" offset into the sprite which will center that 
	int32 _mouseY1offset = 0;					// sprite onto the current mouse position. ie. the mouse "hot spot".
	int32 _mouseX2offset = 0;					// These two are the horizontal and vertical distances from the mouse "hot spot"
	int32 _mouseY2offset = 0;					// to the edge of the sprite.
	int32 _currMouseNum = 0;						// The current index into the sprite series of the sprite which currently is the mouse.
	bool _mouseIsLocked = false;					// An bool which when TRUE will prevent any other mouse from being set.
	int32 _newMouseNum = 0;						// An int32 which stores the currMouseNum while the mouse is locked.

	int32 _hideMouseX1 = MIN_VIDEO_X - 1;	// These 4 represent the mouse "hide" rectangle.  The mouse is drawn if'f it is
	int32 _hideMouseY1 = MIN_VIDEO_Y - 1;	// located within the mouse "show" rectangle (below), or it is not within the
	int32 _hideMouseX2 = MIN_VIDEO_X - 1;	// mouse "hide" rectangle.  ie. If the "hide" rect covers the whole monitor, and
	int32 _hideMouseY2 = MIN_VIDEO_Y - 1;	// the "show" rectangle covers a specific window, then the mouse will be hidden
																	// everywhere except within the specific window.
	int32 _showMouseX1 = MIN_VIDEO_X;		   // These 4 represent the mouse "show" rectangle.
	int32 _showMouseY1 = MIN_VIDEO_Y;
	int32 _showMouseX2 = MAX_VIDEO_X;
	int32 _showMouseY2 = MAX_VIDEO_Y;

	int32 _mouseX = MAX_VIDEO_X >> 1;			// The current mouse "x" position (global coords).
	int32 _mouseY = MAX_VIDEO_Y >> 1;			// The current mouse "y" position (global coords).

	int32 _oldX = MAX_VIDEO_X >> 1;				// The previous mouse "x" and "y" position.  When the mouse has moved, the
	int32 _oldY = MAX_VIDEO_Y >> 1;				// position where the mouse sprite was last drawn is here.
};

bool gui_mouse_init();
void gui_mouse_shutdown();
bool mouse_set_sprite(int32 spriteNum);
void gui_mouse_refresh();
void mouse_hide();
void mouse_show();
void mouse_lock_sprite(int32 mouseNum);
void mouse_unlock_sprite();

} // End of namespace M4

#endif
