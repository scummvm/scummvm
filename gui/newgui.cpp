/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
#include "util.h"
#include "newgui.h"
#include "dialog.h"


#ifdef _MSC_VER

#	pragma warning( disable : 4068 ) // unknown pragma

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

// Built-in font
static byte guifont[] = {
0,0,99,1,226,8,4,8,6,8,6,0,0,0,0,0,0,0,0,0,0,0,8,2,1,8,0,0,0,0,0,0,0,0,0,0,0,0,4,3,7,8,7,7,8,4,5,5,8,7,4,7,3,8,7,7,7,7,8,7,7,7,7,7,3,4,7,5,7,7,8,7,7,7,7,7,7,7,7,5,7,7,
7,8,7,7,7,7,7,7,7,7,7,8,7,7,7,5,8,5,8,8,7,7,7,6,7,7,7,7,7,5,6,7,5,8,7,7,7,7,7,7,7,7,7,8,7,7,7,5,3,5,0,8,7,7,7,7,7,7,0,6,7,7,7,5,5,5,7,0,6,8,8,7,7,7,7,7,0,7,7,0,0,
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
24,48,48,96,48,48,24,0,96,96,96,0,96,96,96,0,96,48,48,24,48,48,96,0,0,0,0,0,0,0,0,0,8,12,14,255,255,14,12,8,60,102,96,96,102,60,24,56,102,0,102,102,102,102,62,0,12,24,60,102,126,96,60,0,24,36,60,6,62,102,62,0,102,0,60,6,62,102,62,0,48,
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

// Constructor
NewGui::NewGui(OSystem *system) : _system(system), _screen(0),
	_use_alpha_blending(true), _need_redraw(false),
	_currentKeyDown(0), _cursorAnimateCounter(0), _cursorAnimateTimer(0)
{
	// Setup some default GUI colors.
	// TODO - either use nicer values, or maybe make this configurable?
	_bgcolor = RGB_TO_16(0, 0, 0);
	_color = RGB_TO_16(80, 80, 80);
	_shadowcolor = RGB_TO_16(64, 64, 64);
	_textcolor = RGB_TO_16(32, 160, 32);
	_textcolorhi = RGB_TO_16(0, 255, 0);
	
	// Clear the cursor
	memset(_cursor, 0xFF, sizeof(_cursor));
}

void NewGui::runLoop()
{
	if (!isActive())
		return;
	
	Dialog *activeDialog;
	int i;
	OSystem::Event event;

	saveState();

	_currentKeyDown = 0;
	
	_lastClick.x = _lastClick.y = 0;
	_lastClick.time = 0;
	_lastClick.count = 0;

	while (isActive()) {
		activeDialog = _dialogStack.top();

		activeDialog->handleTickle();
	
		if (_need_redraw) {
			// Restore the overlay to its initial state, then draw all dialogs.
			// This is necessary to get the blending right.
			_system->clear_overlay();
			_system->grab_overlay(_screen, _screen_pitch);
			for (i = 0; i < _dialogStack.size(); i++)
				_dialogStack[i]->draw();
			_need_redraw = false;
		}
		
		animateCursor();
	
		_system->update_screen();		

		while (_system->poll_event(&event)) {
			switch(event.event_code) {
				case OSystem::EVENT_KEYDOWN:
					activeDialog->handleKeyDown((byte)event.kbd.ascii, event.kbd.flags);

					// init continuous event stream
					_currentKeyDown = event.kbd.ascii;
					_currentKeyDownFlags = event.kbd.flags;
					_keyRepeatEvenCount = 1;
					_keyRepeatLoopCount = 0;
					break;
				case OSystem::EVENT_KEYUP:
					activeDialog->handleKeyUp((byte)event.kbd.ascii, event.kbd.flags);
					if (event.kbd.ascii == _currentKeyDown)
						// only stop firing events if it's the current key
						_currentKeyDown = 0;
					break;
				case OSystem::EVENT_MOUSEMOVE:
					_system->set_mouse_pos(event.mouse.x, event.mouse.y);
					activeDialog->handleMouseMoved(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, 0);
					break;
				// We don'event distinguish between mousebuttons (for now at least)
				case OSystem::EVENT_LBUTTONDOWN:
				case OSystem::EVENT_RBUTTONDOWN: {
					uint32 time = _system->get_msecs();
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
			}
		}

		// check if event should be sent again (keydown)
		if (_currentKeyDown != 0)
		{
			// if only fired once, wait longer
			if ( _keyRepeatLoopCount >= ((_keyRepeatEvenCount > 1) ? 2 : 4) )
			//                                                           ^  loops to wait first event
			//                                                       ^      loops to wait after first event
			{
				// fire event
				activeDialog->handleKeyDown(_currentKeyDown, _currentKeyDownFlags);
				_keyRepeatEvenCount++;
				_keyRepeatLoopCount = 0;
			}
			_keyRepeatLoopCount++;
		}

		// Delay for a moment
		_system->delay_msecs(10);
	}
}

#pragma mark -

void NewGui::saveState()
{
	// Backup old cursor
	_oldCursorMode = _system->show_mouse(true);
	
	_system->show_overlay();
	// TODO - add getHeight & getWidth methods to OSystem.
	// Note that this alone is not a sufficient solution, as in the future the screen size
	// might change. E.g. we start up in 320x200 mode but then go on playing Zak256
	// which makes us switch to 320x240, or even CMI which uses 640x480...
	// FIXME - for now just use a dirty HACK
	_screen = new int16[320 * 240];
	_screen_pitch = 320;
//	_screen = new int16[_s->_realWidth * _s->_realHeight];
//	_screen_pitch = _s->_realWidth;
	_system->grab_overlay(_screen, _screen_pitch);
}

void NewGui::restoreState()
{
	_system->show_mouse(_oldCursorMode);

	_system->hide_overlay();
	if (_screen) {
		delete _screen;
		_screen = 0;
	}
}

void NewGui::openDialog(Dialog *dialog)
{
	_dialogStack.push(dialog);
	_need_redraw = true;
}

void NewGui::closeTopDialog()
{
	// Don't do anything if no dialog is open
	if (_dialogStack.empty())
		return;
	
	// Remove the dialog from the stack
	_dialogStack.pop();
	if (_dialogStack.empty())
		restoreState();
	else
		_need_redraw = true;
}


#pragma mark -


int16 *NewGui::getBasePtr(int x, int y)
{
	return _screen + x + y * _screen_pitch;
}

void NewGui::box(int x, int y, int width, int height)
{
	hline(x + 1, y, x + width - 2, _color);
	hline(x, y + 1, x + width - 1, _color);
	vline(x, y + 1, y + height - 2, _color);
	vline(x + 1, y, y + height - 1, _color);

	hline(x + 1, y + height - 2, x + width - 1, _shadowcolor);
	hline(x + 1, y + height - 1, x + width - 2, _shadowcolor);
	vline(x + width - 1, y + 1, y + height - 2, _shadowcolor);
	vline(x + width - 2, y + 1, y + height - 1, _shadowcolor);
}

void NewGui::line(int x, int y, int x2, int y2, int16 color)
{
	int16 *ptr;

	if (x2 < x)
		x2 ^= x ^= x2 ^= x;					// Swap x2 and x

	if (y2 < y)
		y2 ^= y ^= y2 ^= y;					// Swap y2 and y

	ptr = getBasePtr(x, y);

	if (ptr == NULL)
		return;

	if (x == x2) {
		/* vertical line */
		while (y++ <= y2) {
			*ptr = color;
			ptr += _screen_pitch;
		}
	} else if (y == y2) {
		/* horizontal line */
		while (x++ <= x2) {
			*ptr++ = color;
		}
	}
}

void NewGui::blendRect(int x, int y, int w, int h, int16 color)
{
	int r = RED_FROM_16(color) * 3;
	int g = GREEN_FROM_16(color) * 3;
	int b = BLUE_FROM_16(color) * 3;
	int16 *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	while (h--) {
		for (int i = 0; i < w; i++) {
			ptr[i] = RGB_TO_16((RED_FROM_16(ptr[i])+r)/4,
			                   (GREEN_FROM_16(ptr[i])+g)/4,
			                   (BLUE_FROM_16(ptr[i])+b)/4);
		}
		ptr += _screen_pitch;
	}
}

void NewGui::fillRect(int x, int y, int w, int h, int16 color)
{
	int i;
	int16 *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	while (h--) {
		for (i = 0; i < w; i++) {
			ptr[i] = color;
		}
		ptr += _screen_pitch;
	}
}

void NewGui::checkerRect(int x, int y, int w, int h, int16 color)
{
	int i;
	int16 *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	while (h--) {
		for (i = 0; i < w; i++) {
			if ((h ^ i) & 1)
				ptr[i] = color;
		}
		ptr += _screen_pitch;
	}
}

void NewGui::frameRect(int x, int y, int w, int h, int16 color)
{
	int i;
	int16 *ptr, *basePtr = getBasePtr(x, y);
	if (basePtr == NULL)
		return;

	ptr = basePtr;
	for (i = 0; i < w; i++, ptr++)
		*ptr = color;
	ptr--;
	for (i = 0; i < h; i++, ptr += _screen_pitch)
		*ptr = color;
	ptr = basePtr;
	for (i = 0; i < h; i++, ptr += _screen_pitch)
		*ptr = color;
	ptr -= _screen_pitch;
	for (i = 0; i < w; i++, ptr++)
		*ptr = color;
}

void NewGui::addDirtyRect(int x, int y, int w, int h)
{
	// For now we don't keep yet another list of dirty rects but simply
	// blit the affected area directly to the overlay. At least for our current
	// GUI/widget/dialog code that is just fine.
	int16 *buf = getBasePtr(x, y);
	_system->copy_rect_overlay(buf, _screen_pitch, x, y, w, h);
}

void NewGui::drawChar(const char chr, int xx, int yy, int16 color)
{
	unsigned int buffer = 0, mask = 0, x, y;
	byte *tmp;

	tmp = guifont + 224 + (chr + 1) * 8;

	int16 *ptr = getBasePtr(xx, yy);
	if (ptr == NULL)
		return;

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
		ptr += _screen_pitch;
	}
}

int NewGui::getStringWidth(const String &str)
{
	int space = 0;

	for (int i = 0; i < str.size(); ++i)
		space += getCharWidth(str[i]);
	return space;
}

int NewGui::getCharWidth(char c)
{
	return guifont[c+6];
}

void NewGui::drawString(const String &str, int x, int y, int w, int16 color, int align)
{
	int width = getStringWidth(str);
	if (align == kTextAlignCenter)
		x = x + (w - width - 1)/2;
	else if (align == kTextAlignRight)
		x = x + w - width;

	for (int i = 0; i < str.size(); ++i) {
		drawChar(str[i], x, y, color);
		x += getCharWidth(str[i]);
	}
}

//
// Draw an 8x8 bitmap at location (x,y)
//
void NewGui::drawBitmap(uint32 bitmap[8], int x, int y, int16 color)
{
	int16 *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	for (int y2 = 0; y2 < 8; y2++) {
		uint32 mask = 0xF0000000;
		for (int x2 = 0; x2 < 8; x2++) {
			if (bitmap[y2] & mask)
				ptr[x2] = color;
			mask >>= 4;
		}
		ptr += _screen_pitch;
	}
}

//
// Draw the mouse cursor (animated). This is mostly ripped from the cursor code in gfx.cpp
// We could plug in a different cursor here if we like to.
//
void NewGui::animateCursor()
{
	int time = _system->get_msecs(); 
	if (time > _cursorAnimateTimer + kCursorAnimateDelay) {
		const byte colors[4] = { 15, 15, 7, 8 };
		const byte color = colors[_cursorAnimateCounter];
		int i;
		
		for (i = 0; i < 16; i++) {
			if ((i < 7) || (i > 9)) {
				_cursor[16 * 8 + i] = color;
				_cursor[16 * i + 8] = color;
			}
		}
	
		_system->set_mouse_cursor(_cursor, 16, 16, 8, 8);

		_cursorAnimateTimer = time;
		_cursorAnimateCounter = (_cursorAnimateCounter + 1) % 4;
	}
}
