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
#include "scumm.h"
#include "newgui.h"
#include "guimaps.h"
#include "gui/dialog.h"

#define hline(x, y, x2, color) line(x, y, x2, y, color);
#define vline(x, y, y2, color) line(x, y, x, y2, color);


NewGui::NewGui(Scumm *s) : _s(s), _active(false), _need_redraw(false), _activeDialog(0)
{
	_pauseDialog = new PauseDialog(this);
	_saveLoadDialog = new SaveLoadDialog(this);
}

void NewGui::pauseDialog()
{
	_active = true;
	_activeDialog = _pauseDialog;
	_need_redraw = true;
}

void NewGui::saveloadDialog()
{
	_active = true;
	_activeDialog = _saveLoadDialog;
	_need_redraw = true;
}

void NewGui::loop()
{
	if (_need_redraw) {
		_activeDialog->draw();
		saveState();
		_need_redraw = false;
	}
	_s->animateCursor();
	_s->getKeyInput(0);
	if (_s->_mouseButStat & MBS_LEFT_CLICK) {
		_activeDialog->handleClick(_s->mouse.x, _s->mouse.y, _s->_mouseButStat);
	} else if (_s->_lastKeyHit) {
		_activeDialog->handleKey(_s->_lastKeyHit, 0);
	} else if (_old_mouse.x != _s->mouse.x || _old_mouse.y != _s->mouse.y) {
		_activeDialog->handleMouseMoved(_s->mouse.x, _s->mouse.y, _s->_mouseButStat);
		_old_mouse.x = _s->mouse.x;
		_old_mouse.y = _s->mouse.y;
	}

	_s->drawDirtyScreenParts();
	_s->_mouseButStat = 0;
}

#pragma mark -

void NewGui::saveState()
{
	_old_soundsPaused = _s->_soundsPaused;
	_s->pauseSounds(true);

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

	_s->pauseSounds(_old_soundsPaused);
}

#pragma mark -

const char *NewGui::queryString(int stringno)
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

	if (!result) {									// Gracelessly degrade to english :)
		if (_s->_features & GF_AFTER_V6)
			return string_map_table_v6[stringno - 1].string;
		else
			return string_map_table_v5[stringno - 1].string;
	}
	
	return result;
}

#pragma mark -

byte *NewGui::getBasePtr(int x, int y)
{
	VirtScreen *vs = _s->findVirtScreen(y);

	if (vs == NULL)
		return NULL;

	return vs->screenPtr + x + (y - vs->topline) * 320 +
		_s->_screenStartStrip * 8 + (_s->camera._cur.y - 100)*320;
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
		x2 ^= x ^= x2 ^= x;	// Swap x2 and x

	if (y2 < y)
		y2 ^= y ^= y2 ^= y;	// Swap y2 and y

	ptr = getBasePtr(x, y);

	if (ptr == NULL)
		return;

	if (x == x2) {
		/* vertical line */
		while (y++ <= y2) {
			*ptr = color;
			ptr += 320;
		}
	} else if (y == y2) {
		/* horizontal line */
		while (x++ <= x2) {
			*ptr++ = color;
		}
	}
}

void NewGui::clearArea(int x, int y, int w, int h)
{
	VirtScreen *vs = _s->findVirtScreen(y);
	byte *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	_s->setVirtscreenDirty(vs, x, y, x + w, y + h);

	while (h--) {
		for (int i = 0; i < w; i++)
			ptr[i] = _bgcolor;
		ptr += 320;
	}
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
		ptr += 320;
	}
	_color = tempc;

}
void NewGui::drawString(const char *str, int x, int y, int w, byte color)
{
	StringTab *st = &_s->string[5];
	st->charset = 1;
	st->center = false;
	st->color = color;
	st->xpos = x;
	st->ypos = y;
	st->right = x + w;

	if (_s->_gameId) {						/* If a game is active.. */
		_s->_messagePtr = (byte *)str;
		_s->drawString(5);
	} else {
		uint len = strlen(str);
		for (uint letter = 0; letter < len; letter++)
			drawChar(str[letter], st->xpos + (letter * 8), st->ypos);
	}
}
