/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "common/util.h"
#include "newgui.h"
#include "dialog.h"

#ifdef __PALM_OS__
#	include "palm.h"
#endif

/*
 * TODO list
 * - get a nicer font which contains diacrits (ŠšŸ§Žˆ etc.)
 * - add more widgets: edit field, popup, radio buttons, ...
 *
 * Other ideas:
 * - allow multi line (l/c/r aligned) text via StaticTextWidget ?
 * - add "close" widget to all dialogs (with a flag to turn it off) ?
 * - make dialogs "moveable" ?
 * - come up with a new look & feel / theme for the GUI 
 * - ...
 */

enum {
	kDoubleClickDelay = 500, // milliseconds
	kCursorAnimateDelay = 500,
	kKeyRepeatInitialDelay = 400,
	kKeyRepeatSustainDelay = 100
};

#ifdef __PALM_OS__
static byte *guifont;
#else
// Built-in font
static byte guifont[] = {
0,0,99,1,226,8,4,8,6,8,6,0,0,0,0,0,0,0,0,0,0,0,8,2,1,8,0,0,0,0,0,0,0,0,0,0,0,0,4,3,7,8,7,7,8,4,5,5,8,7,4,7,3,8,7,7,7,7,8,7,7,7,7,7,3,4,7,5,7,7,8,7,7,7,7,7,7,7,7,5,7,7,
7,8,7,7,7,7,7,7,7,7,7,8,7,7,7,5,8,5,8,8,7,7,7,6,7,7,7,7,7,5,6,7,5,8,7,7,7,7,7,7,7,7,7,8,7,7,7,5,3,5,7,8,7,7,7,7,7,7,0,6,7,7,7,5,5,5,7,0,6,8,8,7,7,7,7,7,0,7,7,0,0,
0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,1,3,6,12,
24,62,3,0,128,192,96,48,24,124,192,0,0,3,62,24,12,6,3,1,0,192,124,24,48,96,192,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,237,74,72,0,0,0,0,0,128,128,128,0,0,0,0,0,0,0,0,0,0,0,0,0,60,66,153,161,161,153,66,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,96,96,96,96,0,0,96,0,102,102,102,0,0,0,0,0,102,102,255,102,255,102,102,0,24,62,96,60,6,124,24,0,98,102,12,24,48,102,70,0,60,102,60,56,103,102,63,0,96,48,16,0,0,0,0,0,24,48,96,96,96,48,24,0,96,48,24,24,24,48,96,0,
0,102,60,255,60,102,0,0,0,24,24,126,24,24,0,0,0,0,0,0,0,48,48,96,0,0,0,126,0,0,0,0,0,0,0,0,0,96,96,0,0,3,6,12,24,48,96,0,60,102,102,102,102,102,60,0,24,24,56,24,24,24,126,0,60,102,6,12,48,96,126,0,60,102,6,28,6,102,60,0,6,
14,30,102,127,6,6,0,126,96,124,6,6,102,60,0,60,102,96,124,102,102,60,0,126,102,12,24,24,24,24,0,60,102,102,60,102,102,60,0,60,102,102,62,6,102,60,0,0,0,96,0,0,96,0,0,0,0,48,0,0,48,48,96,14,24,48,96,48,24,14,0,0,0,120,0,120,0,0,0,112,24,
12,6,12,24,112,0,60,102,6,12,24,0,24,0,0,0,0,255,255,0,0,0,24,60,102,126,102,102,102,0,124,102,102,124,102,102,124,0,60,102,96,96,96,102,60,0,120,108,102,102,102,108,120,0,126,96,96,120,96,96,126,0,126,96,96,120,96,96,96,0,60,102,96,110,102,102,60,0,102,102,102,
126,102,102,102,0,120,48,48,48,48,48,120,0,30,12,12,12,12,108,56,0,102,108,120,112,120,108,102,0,96,96,96,96,96,96,126,0,99,119,127,107,99,99,99,0,102,118,126,126,110,102,102,0,60,102,102,102,102,102,60,0,124,102,102,124,96,96,96,0,60,102,102,102,102,60,14,0,124,102,102,124,
120,108,102,0,60,102,96,60,6,102,60,0,126,24,24,24,24,24,24,0,102,102,102,102,102,102,60,0,102,102,102,102,102,60,24,0,99,99,99,107,127,119,99,0,102,102,60,24,60,102,102,0,102,102,102,60,24,24,24,0,126,6,12,24,48,96,126,0,120,96,96,96,96,96,120,0,3,6,12,24,48,
96,192,0,120,24,24,24,24,24,120,0,0,0,0,0,0,219,219,0,0,0,0,0,0,0,0,255,102,102,102,0,0,0,0,0,0,0,60,6,62,102,62,0,0,96,96,124,102,102,124,0,0,0,60,96,96,96,60,0,0,6,6,62,102,102,62,0,0,0,60,102,126,96,60,0,0,14,24,62,24,24,
24,0,0,0,62,102,102,62,6,124,0,96,96,124,102,102,102,0,0,48,0,112,48,48,120,0,0,12,0,12,12,12,12,120,0,96,96,108,120,108,102,0,0,112,48,48,48,48,120,0,0,0,102,127,127,107,99,0,0,0,124,102,102,102,102,0,0,0,60,102,102,102,60,0,0,0,124,102,102,124,96,
96,0,0,62,102,102,62,6,6,0,0,124,102,96,96,96,0,0,0,62,96,60,6,124,0,0,24,126,24,24,24,14,0,0,0,102,102,102,102,62,0,0,0,102,102,102,60,24,0,0,0,99,107,127,62,54,0,0,0,102,60,24,60,102,0,0,0,102,102,102,62,12,120,0,0,126,12,24,48,126,0,
24,48,48,96,48,48,24,0,96,96,96,0,96,96,96,0,96,48,48,24,48,48,96,0,0,0,97,153,134,0,0,0,8,12,14,255,255,14,12,8,60,102,96,96,102,60,24,56,102,0,102,102,102,102,62,0,12,24,60,102,126,96,60,0,24,36,60,6,62,102,62,0,102,0,60,6,62,102,62,0,48,
24,60,6,62,102,62,0,0,0,0,0,0,0,0,0,0,60,96,96,96,60,24,56,24,36,60,102,126,96,60,0,102,0,60,102,126,96,60,0,48,24,60,102,126,96,60,0,0,216,0,112,48,48,120,0,48,72,0,112,48,48,120,0,96,48,0,112,48,48,120,0,102,24,60,102,126,102,102,0,0,0,
0,0,0,0,0,0,24,48,124,96,120,96,124,0,0,0,108,26,126,216,110,0,30,40,40,126,72,136,142,0,24,36,60,102,102,102,60,0,102,0,60,102,102,102,60,0,48,24,60,102,102,102,60,0,24,36,0,102,102,102,62,0,48,24,102,102,102,102,62,0,0,0,0,0,0,0,0,0,102,60,102,
102,102,102,60,0,102,0,102,102,102,102,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,24,60,6,62,102,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,28,54,54,124,102,102,124,64,0,0,0
};
#endif

// Constructor
NewGui::NewGui(OSystem *system) : _system(system), _screen(0), _needRedraw(false),
	_stateIsSaved(false), _cursorAnimateCounter(0), _cursorAnimateTimer(0) {
	// Clear the cursor
	memset(_cursor, 0xFF, sizeof(_cursor));

	// Reset key repeat
	_currentKeyDown.keycode = 0;
}

void NewGui::runLoop() {
	Dialog *activeDialog = _dialogStack.top();
	bool didSaveState = false;

	if (activeDialog == 0)
		return;
	
	// Setup some default GUI colors. This has to be done here to ensure the
	// overlay has been created already. Even then, one can get wrong colors, namely
	// if the GUI is up and then the user toggles to full screen mode - on some system
	// different color modes (555 vs 565) might be used depending on the resolution
	// (e.g. that's the case on my system), so we still end up with wrong colors in those
	// sitauations. At least now the user can fix it by closing and reopening the GUI.
	_bgcolor = _system->RGBToColor(0, 0, 0);
	_color = _system->RGBToColor(96, 96, 96);
	_shadowcolor = _system->RGBToColor(64, 64, 64);
	_textcolor = _system->RGBToColor(32, 160, 32);
	_textcolorhi = _system->RGBToColor(0, 255, 0);

	if (!_stateIsSaved) {
		saveState();
		didSaveState = true;
	}

	while (!_dialogStack.empty() && activeDialog == _dialogStack.top()) {
#ifdef __PALM_OS__
		// There is no exit(.) function under PalmOS, to exit an app
		// we need to send an 'exit' event to the event handler
		// and then the system return to the launcher. But this event
		// is not handled by the following loop and so we need to force exit.
		// In other systems like Windows ScummVM exit immediatly and so this doesn't appear.
		if (((OSystem_PALMOS *)_system)->_quit)
			return;
#endif
		activeDialog->handleTickle();

		if (_needRedraw) {
			// Restore the overlay to its initial state, then draw all dialogs.
			// This is necessary to get the blending right.
			_system->clear_overlay();
			_system->grab_overlay(_screen, _screenPitch);
			for (int i = 0; i < _dialogStack.size(); i++)
				_dialogStack[i]->drawDialog();
			_needRedraw = false;
		}

		animateCursor();
		_system->update_screen();		

		OSystem::Event event;
		uint32 time = get_time();

		while (_system->poll_event(&event)) {
			switch(event.event_code) {
				case OSystem::EVENT_KEYDOWN:
#if !defined(_WIN32_WCE) && !defined(__PALM_OS__)
					// init continuous event stream
					// not done on WinCE because keyboard is emulated and
					// keyup is not generated
					_currentKeyDown.ascii = event.kbd.ascii;
					_currentKeyDown.keycode = event.kbd.keycode;
					_currentKeyDown.flags = event.kbd.flags;
					_keyRepeatTime = time + kKeyRepeatInitialDelay;
#endif
					activeDialog->handleKeyDown(event.kbd.ascii, event.kbd.keycode, event.kbd.flags);
					break;
				case OSystem::EVENT_KEYUP:
					activeDialog->handleKeyUp(event.kbd.ascii, event.kbd.keycode, event.kbd.flags);
					if (event.kbd.keycode == _currentKeyDown.keycode)
						// only stop firing events if it's the current key
						_currentKeyDown.keycode = 0;
					break;
				case OSystem::EVENT_MOUSEMOVE:
					activeDialog->handleMouseMoved(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, 0);
					break;
				// We don't distinguish between mousebuttons (for now at least)
				case OSystem::EVENT_LBUTTONDOWN:
				case OSystem::EVENT_RBUTTONDOWN: {
					if (_lastClick.count && (time < _lastClick.time + kDoubleClickDelay)
								&& ABS(_lastClick.x - event.mouse.x) < 3
								&& ABS(_lastClick.y - event.mouse.y) < 3) {
						_lastClick.count++;
					} else {
						_lastClick.x = event.mouse.x;
						_lastClick.y = event.mouse.y;
						_lastClick.count = 1;
					}
					_lastClick.time = time;
					}
					activeDialog->handleMouseDown(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, 1, _lastClick.count);
					break;
				case OSystem::EVENT_LBUTTONUP:
				case OSystem::EVENT_RBUTTONUP:
					activeDialog->handleMouseUp(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, 1, _lastClick.count);
					break;
				case OSystem::EVENT_WHEELUP:
					activeDialog->handleMouseWheel(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, -1);
					break;
				case OSystem::EVENT_WHEELDOWN:
					activeDialog->handleMouseWheel(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, 1);
					break;
				case OSystem::EVENT_QUIT:
					_system->quit();
					break;
			}
		}

		// check if event should be sent again (keydown)
		if (_currentKeyDown.keycode != 0) {
			if (_keyRepeatTime < time) {
				// fire event
				activeDialog->handleKeyDown(_currentKeyDown.ascii, _currentKeyDown.keycode, _currentKeyDown.flags);
				_keyRepeatTime = time + kKeyRepeatSustainDelay;
			}
		}

		// Delay for a moment
		_system->delay_msecs(10);
	}
	
	if (didSaveState)
		restoreState();
}

#pragma mark -

void NewGui::saveState() {
	int sys_height = _system->get_height();
	int sys_width = _system->get_width();

	// Backup old cursor
	_oldCursorMode = _system->show_mouse(true);

	_system->show_overlay();
	// TODO - add getHeight & getWidth methods to OSystem.
	_screen = (NewGuiColor*)calloc(sys_width*sys_height,sizeof(NewGuiColor));
	_screenPitch = sys_width;
	_system->grab_overlay(_screen, _screenPitch);

	_currentKeyDown.keycode = 0;
	_lastClick.x = _lastClick.y = 0;
	_lastClick.time = 0;
	_lastClick.count = 0;

	_stateIsSaved = true;
}

void NewGui::restoreState() {
	_system->show_mouse(_oldCursorMode);

	_system->hide_overlay();
	if (_screen) {
		free(_screen);
		_screen = 0;
	}

	_system->update_screen();
	
	_stateIsSaved = false;
}

void NewGui::openDialog(Dialog *dialog) {
	_dialogStack.push(dialog);
	_needRedraw = true;
}

void NewGui::closeTopDialog() {
	// Don't do anything if no dialog is open
	if (_dialogStack.empty())
		return;

	// Remove the dialog from the stack
	_dialogStack.pop();
	_needRedraw = true;
}

#pragma mark -

NewGuiColor *NewGui::getBasePtr(int x, int y) {
	return _screen + x + y * _screenPitch;
}

void NewGui::box(int x, int y, int width, int height, bool inverted) {
	NewGuiColor colorA = inverted ? _shadowcolor : _color;
	NewGuiColor colorB = inverted ? _color : _shadowcolor;

	hline(x + 1, y, x + width - 2, colorA);
	hline(x, y + 1, x + width - 1, colorA);
	vline(x, y + 1, y + height - 2, colorA);
	vline(x + 1, y, y + height - 1, colorA);

	hline(x + 1, y + height - 2, x + width - 1, colorB);
	hline(x + 1, y + height - 1, x + width - 2, colorB);
	vline(x + width - 1, y + 1, y + height - 2, colorB);
	vline(x + width - 2, y + 1, y + height - 1, colorB);
}

void NewGui::line(int x, int y, int x2, int y2, NewGuiColor color) {
	NewGuiColor *ptr;

	if (x2 < x)
		SWAP(x2, x);

	if (y2 < y)
		SWAP(y2, y);

	ptr = getBasePtr(x, y);

	if (x == x2) {
		/* vertical line */
		while (y++ <= y2) {
			*ptr = color;
			ptr += _screenPitch;
		}
	} else if (y == y2) {
		/* horizontal line */
		while (x++ <= x2) {
			*ptr++ = color;
		}
	}
}

void NewGui::blendRect(int x, int y, int w, int h, NewGuiColor color, int level) {
#ifdef NEWGUI_256
	fillRect(x,y,w,h,color);
#else
	int r, g, b;
	uint8 ar, ag, ab;
	_system->colorToRGB(color, ar, ag, ab);
	r = ar * level;
	g = ag * level;
	b = ab * level;

	NewGuiColor *ptr = getBasePtr(x, y);

	while (h--) {
		for (int i = 0; i < w; i++) {
			_system->colorToRGB(ptr[i], ar, ag, ab);
			ptr[i] = _system->RGBToColor((ar+r)/(level+1),
										 (ag+g)/(level+1),
										 (ab+b)/(level+1));
		}
		ptr += _screenPitch;
	}
#endif
}

void NewGui::fillRect(int x, int y, int w, int h, NewGuiColor color) {
	int i;
	NewGuiColor *ptr = getBasePtr(x, y);

	while (h--) {
		for (i = 0; i < w; i++) {
			ptr[i] = color;
		}
		ptr += _screenPitch;
	}
}

void NewGui::checkerRect(int x, int y, int w, int h, NewGuiColor color) {
	int i;
	NewGuiColor *ptr = getBasePtr(x, y);

	while (h--) {
		for (i = 0; i < w; i++) {
			if ((h ^ i) & 1)
				ptr[i] = color;
		}
		ptr += _screenPitch;
	}
}

void NewGui::frameRect(int x, int y, int w, int h, NewGuiColor color) {
	int i;
	NewGuiColor *ptr, *basePtr = getBasePtr(x, y);
	if (basePtr == NULL)
		return;

	ptr = basePtr;
	for (i = 0; i < w; i++, ptr++)
		*ptr = color;
	ptr--;
	for (i = 0; i < h; i++, ptr += _screenPitch)
		*ptr = color;
	ptr = basePtr;
	for (i = 0; i < h; i++, ptr += _screenPitch)
		*ptr = color;
	ptr -= _screenPitch;
	for (i = 0; i < w; i++, ptr++)
		*ptr = color;
}

void NewGui::addDirtyRect(int x, int y, int w, int h) {
	// For now we don't keep yet another list of dirty rects but simply
	// blit the affected area directly to the overlay. At least for our current
	// GUI/widget/dialog code that is just fine.
	NewGuiColor *buf = getBasePtr(x, y);
	_system->copy_rect_overlay(buf, _screenPitch, x, y, w, h);
}

void NewGui::drawChar(const byte chr, int xx, int yy, NewGuiColor color) {
	unsigned int buffer = 0, mask = 0, x, y;
	byte *tmp;

	tmp = guifont + 224 + (chr + 1) * 8;

	NewGuiColor *ptr = getBasePtr(xx, yy);

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			unsigned char c;
			if ((mask >>= 1) == 0) {
				buffer = *tmp++;
				mask = 0x80;
			}
			c = ((buffer & mask) != 0);
			if (c)
				ptr[x] = color;
		}
		ptr += _screenPitch;
	}
}

int NewGui::getStringWidth(const String &str) {
	int space = 0;

	for (int i = 0; i < str.size(); ++i)
		space += getCharWidth(str[i]);
	return space;
}

int NewGui::getCharWidth(byte c) {
	return guifont[c+6];
}

void NewGui::drawString(const String &str, int x, int y, int w, NewGuiColor color, int align, int deltax) {
	const int leftX = x, rightX = x + w;
	int width = getStringWidth(str);
	if (align == kTextAlignCenter)
		x = x + (w - width - 1)/2;
	else if (align == kTextAlignRight)
		x = x + w - width;
	x += deltax;

	for (int i = 0; i < str.size(); ++i) {
		w = getCharWidth(str[i]);
		if (x+w > rightX)
			break;
		if (x >= leftX)
			drawChar(str[i], x, y, color);
		x += w;
	}
}

//
// Blit from a buffer to the display
//
void NewGui::blitFromBuffer(int x, int y, int w, int h, const byte *buf, int pitch) {
	NewGuiColor *ptr = getBasePtr(x, y);

	assert(buf);
	while (h--) {
		memcpy(ptr, buf, w*2);
		ptr += _screenPitch;
		buf += pitch;
	}
}

//
// Blit from the display to a buffer
//
void NewGui::blitToBuffer(int x, int y, int w, int h, byte *buf, int pitch) {
	NewGuiColor *ptr = getBasePtr(x, y);

	assert(buf);
	while (h--) {
		memcpy(buf, ptr, w*2);
		ptr += _screenPitch;
		buf += pitch;
	}
}

//
// Draw an 8x8 bitmap at location (x,y)
//
void NewGui::drawBitmap(uint32 *bitmap, int x, int y, NewGuiColor color, int h) {
	NewGuiColor *ptr = getBasePtr(x, y);

	for (y = 0; y < h; y++) {
		uint32 mask = 0xF0000000;
		for (x = 0; x < 8; x++) {
			if (bitmap[y] & mask)
				ptr[x] = color;
			mask >>= 4;
		}
		ptr += _screenPitch;
	}
}

//
// Draw the mouse cursor (animated). This is mostly ripped from the cursor code in gfx.cpp
// We could plug in a different cursor here if we like to.
//
void NewGui::animateCursor() {
	int time = get_time(); 
	if (time > _cursorAnimateTimer + kCursorAnimateDelay) {
		const byte colors[4] = { 15, 15, 7, 8 };
		const byte color = colors[_cursorAnimateCounter];
		int i;
		
		for (i = 0; i < 15; i++) {
			if ((i < 6) || (i > 8)) {
				_cursor[16 * 7 + i] = color;
				_cursor[16 * i + 7] = color;
			}
		}
	
		_system->set_mouse_cursor(_cursor, 16, 16, 7, 7);

		_cursorAnimateTimer = time;
		_cursorAnimateCounter = (_cursorAnimateCounter + 1) % 4;
	}
}

#ifdef __PALM_OS__
#include "scumm_globals.h"	// init globals
void NewGui_initGlobals()	{
	GSETPTR(guifont, GBVARS_GUIFONT_INDEX, byte, GBVARS_SCUMM)
}
void NewGui_releaseGlobals(){	GRELEASEPTR(GBVARS_GUIFONT_INDEX, GBVARS_SCUMM)				}
#endif
