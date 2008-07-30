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
* $URL$
* $Id$
*
*/

#ifndef COMMON_VIRTUAL_KEYBOARD_GUI
#define COMMON_VIRTUAL_KEYBOARD_GUI

#include "common/rect.h"
#include "common/system.h"
#include "graphics/surface.h"

namespace Common {

class VirtualKeyboard;

class VirtualKeyboardGUI {

public:

	VirtualKeyboardGUI(VirtualKeyboard *kbd);
	void setKeyboardSurface(Graphics::Surface *sur, OverlayColor trans_color);
	void run();
	void hide();
	bool isDisplaying() { return _displaying; }
	void reset();
	void startDrag(int16 x, int16 y);
	void endDrag();

private:

	OSystem *_system;
	VirtualKeyboard *_kbd;
	Graphics::Surface *_kbdSurface;
	OverlayColor _kbdTransparentColor;

	static const int SNAP_WIDTH = 10;

	Graphics::Surface _overlayBackup;

	Rect _kbdBound;

	Point _dragPoint;
	bool _drag;

	bool _displaying;
	bool _firstRun;
	bool _needRedraw;
	int _lastScreenChanged;

	void setDefaultPosition();
	void move(int16 x, int16 y);
	void screenChanged();
	void mainLoop();
	void redraw();
	
	static const int kCursorAnimateDelay = 250;
	int _cursorAnimateCounter;
	int	_cursorAnimateTimer;
	byte _cursor[2048];
	void setupCursor();
	void removeCursor();
	void animateCursor();

};

} // end of namespace Common

#endif
