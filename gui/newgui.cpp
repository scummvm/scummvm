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
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "newgui.h"
#include "guimaps.h"
#include "dialog.h"

/*
 * TODO list
 * - implement the missing / incomplete dialogs
 * - add more widgets
 * - allow multi line (l/c/r aligned) text via StaticTextWidget ?
 * - add "close" widget to all dialogs (with a flag to turn it off) ?
 * - make dialogs "moveable" ?
 * - come up with a new look&feel / theme for the GUI 
 * - ...
 */

NewGui::NewGui(Scumm *s) : _s(s), _use_alpha_blending(false),
	_need_redraw(false),_prepare_for_gui(true),
	_pauseDialog(0), _saveLoadDialog(0), _aboutDialog(0), _optionsDialog(0),
	_currentKeyDown(0)
{	
}

void NewGui::pauseDialog()
{
	if (!_pauseDialog)
		_pauseDialog = new PauseDialog(this);
	_pauseDialog->open();
}

void NewGui::saveloadDialog()
{
	if (!_saveLoadDialog)
		_saveLoadDialog = new SaveLoadDialog(this);
	_saveLoadDialog->open();
}

void NewGui::aboutDialog()
{
	if (!_aboutDialog)
		_aboutDialog = new AboutDialog(this);
	_aboutDialog->open();
}

void NewGui::optionsDialog()
{
	if (!_optionsDialog)
		_optionsDialog = new OptionsDialog(this);
	_optionsDialog->open();
}

void NewGui::soundDialog()
{
	if (!_soundDialog)
		_soundDialog = new SoundDialog(this);
	_soundDialog->open();
}

void NewGui::loop()
{
	Dialog *activeDialog = _dialogStack.top();
	int i;
	
	if (_prepare_for_gui) {
		ClearBlendCache(_s->_currentPalette, 128);
		saveState();
		if (_use_alpha_blending)
			activeDialog->setupScreenBuf();
#if 1
		// FIXME - hack to encode our own custom GUI colors. Since we have to live
		// with a given 8 bit palette, the result is not always as nice as one
		// would wish, but this is just an experiment after all.
		_bgcolor = RGBMatch(_s->_currentPalette, 0, 0, 0);
	
		_color = RGBMatch(_s->_currentPalette, 80, 80, 80);
		_shadowcolor = RGBMatch(_s->_currentPalette, 64, 64, 64);
	
		_textcolor = RGBMatch(_s->_currentPalette, 32, 192, 32);
		_textcolorhi = RGBMatch(_s->_currentPalette, 0, 256, 0);
#endif

		_eventList.clear();
		_currentKeyDown = 0;
		
		_lastClick.x = _lastClick.y = 0;
		_lastClick.time = 0;
		_lastClick.count = 0;

		_prepare_for_gui = false;
	}

	activeDialog->handleTickle();

	if (_need_redraw) {
		for (i = 0; i < _dialogStack.size(); i++)
			_dialogStack[i]->draw();
		_need_redraw = false;
	}
	
	_s->animateCursor();

	if (_eventList.size() > 0)
	{
		OSystem::Event t;
		
		for (i = 0; i < _eventList.size(); i++)
		{
			t = _eventList[i];
		
			switch(t.event_code) {
				case OSystem::EVENT_KEYDOWN:
					activeDialog->handleKeyDown(t.kbd.ascii, t.kbd.flags);

					// init continuous event stream
					_currentKeyDown = t.kbd.ascii;
					_currentKeyDownFlags = t.kbd.flags;
					_keyRepeatEvenCount = 1;
					_keyRepeatLoopCount = 0;
					break;
				case OSystem::EVENT_KEYUP:
					activeDialog->handleKeyUp(t.kbd.ascii, t.kbd.flags);
					if (t.kbd.ascii == _currentKeyDown)
						// only stop firing events if it's the current key
						_currentKeyDown = 0;
					break;
				case OSystem::EVENT_MOUSEMOVE:
					activeDialog->handleMouseMoved(t.mouse.x - activeDialog->_x, t.mouse.y - activeDialog->_y, 0);
					break;
				// We don't distinguish between mousebuttons (for now at least)
				case OSystem::EVENT_LBUTTONDOWN:
				case OSystem::EVENT_RBUTTONDOWN: {
					uint32 time = _s->_system->get_msecs();
					if (_lastClick.count && (time < _lastClick.time + kDoubleClickDelay)
					      && ABS(_lastClick.x - t.mouse.x) < 3
					      && ABS(_lastClick.y - t.mouse.y) < 3) {
						_lastClick.count++;
					} else {
						_lastClick.x = t.mouse.x;
						_lastClick.y = t.mouse.y;
						_lastClick.count = 1;
					}
					_lastClick.time = time;
					}
					activeDialog->handleMouseDown(t.mouse.x - activeDialog->_x, t.mouse.y - activeDialog->_y, 1, _lastClick.count);
					break;
				case OSystem::EVENT_LBUTTONUP:
				case OSystem::EVENT_RBUTTONUP:
					activeDialog->handleMouseUp(t.mouse.x - activeDialog->_x, t.mouse.y - activeDialog->_y, 1, _lastClick.count);
					break;
			}
		}

		_eventList.clear();
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

	_s->drawDirtyScreenParts();
}

#pragma mark -

void NewGui::saveState()
{
	_old_soundsPaused = _s->_sound->_soundsPaused;
	_s->_sound->pauseSounds(true);

	// Backup old cursor
	memcpy(_old_grabbedCursor, _s->_grabbedCursor, sizeof(_old_grabbedCursor));
	_old_cursorWidth = _s->_cursorWidth;
	_old_cursorHeight = _s->_cursorHeight;
	_old_cursorHotspotX = _s->_cursorHotspotX;
	_old_cursorHotspotY = _s->_cursorHotspotY;
	_old_cursor_mode = _s->_system->show_mouse(true);

	_s->_cursorAnimate++;
	_s->gdi._cursorActive = 1;
}

void NewGui::restoreState()
{
	_s->_fullRedraw = true;
	_s->_completeScreenRedraw = true;
	_s->_cursorAnimate--;

	// Restore old cursor
	memcpy(_s->_grabbedCursor, _old_grabbedCursor, sizeof(_old_grabbedCursor));
	_s->_cursorWidth = _old_cursorWidth;
	_s->_cursorHeight = _old_cursorHeight;
	_s->_cursorHotspotX = _old_cursorHotspotX;
	_s->_cursorHotspotY = _old_cursorHotspotY;
	_s->updateCursor();

	_s->_system->show_mouse(_old_cursor_mode);

	_s->_sound->pauseSounds(_old_soundsPaused);
}

void NewGui::openDialog(Dialog *dialog)
{
	if (_dialogStack.empty())
		_prepare_for_gui = true;
	else if (_use_alpha_blending)
		dialog->setupScreenBuf();

	_dialogStack.push(dialog);
	_need_redraw = true;
}

void NewGui::closeTopDialog()
{
	// Don't do anything if no dialog is open
	if (_dialogStack.empty())
		return;
	
	// Tear down its screenBuf
	if (_use_alpha_blending)
		_dialogStack.top()->teardownScreenBuf();
	
	// Remove the dialog from the stack
	_dialogStack.pop();
	if (_dialogStack.empty())
		restoreState();
	else
		_need_redraw = true;
}

#pragma mark -

const char *NewGui::queryResString(int stringno)
{
	char *result;
	int string;

	if (stringno == 0)
		return NULL;

	if (_s->_features & GF_AFTER_V7)
		string = _s->_vars[string_map_table_v7[stringno - 1].num];
	else if (_s->_features & GF_AFTER_V6)
		string = _s->_vars[string_map_table_v6[stringno - 1].num];
	else
		string = string_map_table_v5[stringno - 1].num;

	result = (char *)_s->getStringAddress(string);
	if (result && *result == '/') {
		_s->translateText((char*)result, (char*)&_s->transText);
		strcpy((char*)result, (char*)&_s->transText);
	}

	if (!result) {								// Gracelessly degrade to english :)
		if (_s->_features & GF_AFTER_V6)
			return string_map_table_v6[stringno - 1].string;
		else
			return string_map_table_v5[stringno - 1].string;
	}

	return result;
}

const char *NewGui::queryCustomString(int stringno)
{
	return string_map_table_custom[stringno];
}


#pragma mark -


byte *NewGui::getBasePtr(int x, int y)
{
	VirtScreen *vs = _s->findVirtScreen(y);

	if (vs == NULL)
		return NULL;

	return vs->screenPtr + x + (y - vs->topline) * _s->_realWidth +
		_s->_screenStartStrip * 8 + (_s->camera._cur.y - (_s->_realHeight / 2)) * _s->_realWidth;
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

void NewGui::line(int x, int y, int x2, int y2, byte color)
{
	byte *ptr;

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
			ptr += _s->_realWidth;
		}
	} else if (y == y2) {
		/* horizontal line */
		while (x++ <= x2) {
			*ptr++ = color;
		}
	}
}

void NewGui::blendRect(int x, int y, int w, int h, byte color)
{
	byte *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	while (h--) {
		for (int i = 0; i < w; i++) {
			ptr[i] = Blend(ptr[i], color, _s->_currentPalette);
		}
		ptr += _s->_realWidth;
	}
}

void NewGui::fillRect(int x, int y, int w, int h, byte color)
{
	byte *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	while (h--) {
		for (int i = 0; i < w; i++) {
			ptr[i] = color;
		}
		ptr += _s->_realWidth;
	}
}

void NewGui::checkerRect(int x, int y, int w, int h, byte color)
{
	byte *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	while (h--) {
		for (int i = 0; i < w; i++) {
			if ((h ^ i) & 1)
				ptr[i] = color;
		}
		ptr += _s->_realWidth;
	}
}

void NewGui::frameRect(int x, int y, int w, int h, byte color)
{
	int i;
	byte *ptr, *basePtr = getBasePtr(x, y);
	if (basePtr == NULL)
		return;

	ptr = basePtr;
	for (i = 0; i < w; i++, ptr++)
		*ptr = color;
	ptr--;
	for (i = 0; i < h; i++, ptr += _s->_realWidth)
		*ptr = color;
	ptr = basePtr;
	for (i = 0; i < h; i++, ptr += _s->_realWidth)
		*ptr = color;
	ptr -= _s->_realWidth;
	for (i = 0; i < w; i++, ptr++)
		*ptr = color;
}

void NewGui::addDirtyRect(int x, int y, int w, int h)
{
	VirtScreen *vs = _s->findVirtScreen(y);

	if (vs != NULL)
		_s->setVirtscreenDirty(vs, x, y, x + w, y + h);
}

void NewGui::drawChar(const char str, int xx, int yy)
{
	unsigned int buffer = 0, mask = 0, x, y;
	byte *tmp;
	int tempc = _color;
	_color = _textcolor;

	tmp = &guifont[0];
	tmp += 224 + (str + 1) * 8;

	byte *ptr = getBasePtr(xx, yy);
	if (ptr == NULL)
		return;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			unsigned char color;
			if ((mask >>= 1) == 0) {
				buffer = *tmp++;
				mask = 0x80;
			}
			color = ((buffer & mask) != 0);
			if (color)
				ptr[x] = _color;
		}
		ptr += _s->_realWidth;
	}
	_color = tempc;

}

void NewGui::drawString(const char *str, int x, int y, int w, byte color, int align)
{
	if (_s->_gameId) {						/* If a game is active.. */
		StringTab *st = &_s->string[5];
		st->charset = 1;
		st->center = (align == kTextAlignCenter);
		st->color = color;

		if (align == kTextAlignLeft)
			st->xpos = x;
		else if (align == kTextAlignCenter)
			st->xpos = x + w/2;
		else if (align == kTextAlignRight)
			st->xpos = x + w - _s->charset.getStringWidth(0, (byte *)str, 0);

		st->ypos = y;
		st->right = x + w;
	
		_s->_messagePtr = (byte *)str;
		_s->drawString(5);
	} else {
		// FIXME - support center/right align, use nicer custom font.
		// Ultimately, we might want to *always* draw our messages this way,
		// but only if we have a nice font.
		uint len = strlen(str);
		for (uint letter = 0; letter < len; letter++)
			drawChar(str[letter], x + (letter * 8), y);
	}
}

/*
 * Draw an 8x8 bitmap at location (x,y)
 */
void NewGui::drawBitmap(uint32 bitmap[8], int x, int y, byte color)
{
	byte *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	for (int y2 = 0; y2 < 8; y2++) {
		uint32 mask = 0xF0000000;
		for (int x2 = 0; x2 < 8; x2++) {
			if (bitmap[y2] & mask)
				ptr[x2] = color;
			mask >>= 4;
		}
		ptr += _s->_realWidth;
	}
}

void NewGui::blitTo(byte buffer[320*200], int x, int y, int w, int h)
{
	byte *dstPtr = buffer + x + y * _s->_realWidth;
	byte *srcPtr = getBasePtr(x, y);
	if (srcPtr == NULL)
		return;

	while (h--) {
		for (int i = 0; i < w; i++) {
			*dstPtr++ = *srcPtr++;
		}
		dstPtr += _s->_realWidth - w;
		srcPtr += _s->_realWidth - w;
	}
}

void NewGui::blitFrom(byte buffer[320*200], int x, int y, int w, int h)
{
	byte *srcPtr = buffer + x + y * _s->_realWidth;
	byte *dstPtr = getBasePtr(x, y);
	if (dstPtr == NULL)
		return;

	while (h--) {
		for (int i = 0; i < w; i++) {
			*dstPtr++ = *srcPtr++;
		}
		dstPtr += _s->_realWidth - w;
		srcPtr += _s->_realWidth - w;
	}
}

