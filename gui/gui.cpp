/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "scumm/scumm.h"
#include "sound/mididrv.h"
#include "scumm/sound.h"
#include "scumm/imuse.h"
#include "gui.h"
#include "guimaps.h"
#include "config-file.h"

#include <ctype.h>

#define hline(x, y, x2, color) line(x, y, x2, y, color);
#define vline(x, y, y2, color) line(x, y, x, y2, color);

#ifdef _WIN32_WCE
// Additional variables for WinCE specific GUI
#include "gapi_keys.h"
extern bool toolbar_drawn;
extern bool hide_toolbar;
extern bool draw_keyboard;
extern bool get_key_mapping;
extern struct keyops keyMapping;
extern void save_key_mapping(void);
extern void do_hide(bool);
extern void do_quit();
uint16 _key_mapping_required;
uint16 _current_page;
bool keyboard_override;
bool save_hide_toolbar;
#else
#define save_key_mapping() ;
bool get_key_mapping;
uint16 _key_mapping_required;
#endif

enum {
	GUI_NONE = 0,
	GUI_RESTEXT = 1,
	GUI_IMAGE = 2,
	GUI_STAT = 3,
	GUI_CUSTOMTEXT = 4,
	GUI_VARTEXT = 5,
	GUI_ACTIONTEXT = 6,
	GUI_KEYTEXT = 7,
	GUI_SCROLLTEXT = 8,
	GUI_NEXTTEXT = 9,
	GUI_UPDOWNARROW = 10,
	GUI_CHECKBOX = 11
};

enum {
	GWF_BORDER = 1,
	GWF_CLEARBG = 2,
	GWF_PARENT = 4,
	GWF_DELAY = 8,
	GWF_DEFAULT = GWF_BORDER | GWF_CLEARBG,
	GWF_BUTTON = GWF_BORDER | GWF_CLEARBG | GWF_DELAY
};

struct GuiWidget {
	byte _type;
	byte _page;
	byte _flags;
	int16 _x, _y;
	uint16 _w, _h;
	uint16 _id;
	byte _string_number;
	uint8 _hotkey;
};

enum {
	SAVELOAD_DIALOG,
	PAUSE_DIALOG,
	SOUND_DIALOG,
	KEYS_DIALOG,
	OPTIONS_DIALOG,
	ABOUT_DIALOG,
	LAUNCHER_DIALOG,
	MISC_DIALOG
};


#define IMG_SIZE	8

// Triangles pointing up-/downwards, used for save/load dialog
static uint32 up_arrow[IMG_SIZE] = {
	0x00011000,
	0x00011000,
	0x00100100,
	0x00100100,
	0x01000010,
	0x01000010,
	0x10000001,
	0x10000001,
};

static uint32 down_arrow[IMG_SIZE] = {
	0x10000001,
	0x10000001,
	0x01000010,
	0x01000010,
	0x00100100,
	0x00100100,
	0x00011000,
	0x00011000,
};

static uint32 checked_img[IMG_SIZE] = {
	0x00000000,
	0x01000010,
	0x00100100,
	0x00011000,
	0x00011000,
	0x00100100,
	0x01000010,
	0x00000000,
};

const GuiWidget launcher_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 0, 0, 320, 200, 0, 0},
	{GUI_CUSTOMTEXT, 0x01, GWF_CLEARBG, 5, 180, 45, 15, 20, 12},
	{GUI_CUSTOMTEXT, 0x01, GWF_CLEARBG, 130, 180, 65, 15, 21, 17},
	{GUI_CUSTOMTEXT, 0x01, GWF_CLEARBG, 265, 180, 50, 15, 22, 7},
	{0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const GuiWidget keys_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 30, 10, 260, 130, 0, 0},

	// First action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10, 15, 15, 10, 3},	// CUSTOMTEXT_PLUS
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10, 15, 15, 11, 4},	// CUSTOMTEXT_MINUS
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 11 + 33 + 10, 10 + 10, 100, 15, 100, 1},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 3, 100, 15, 1, 1},

	//Second action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10 + 15 + 5, 15, 15, 20, 3},	// CUSTOMTEXT_PLUS
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10 + 15 + 5, 15, 15, 21, 4},	// CUSTOMTEXT_MINUS
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 10 + 33 + 10, 10 + 10 + 15 + 5, 100, 15, 101, 2},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 15 + 5 + 3, 100, 15, 2, 2},

	//Third action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10 + 15 + 5 + 15 + 5, 15, 15, 30, 3},	// CUSTOMTEXT_PLUS
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10 + 15 + 5 + 15 + 5, 15, 15, 31, 4},	// CUSTOMTEXT_MINUS
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 10 + 33 + 10, 10 + 10 + 15 + 5 + 15 + 5, 100, 15, 102, 3},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 15 + 5 + 15 + 5 + 3, 100, 15, 3, 3},

	//Fourth action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5, 15, 15, 40, 3},
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5, 15, 15, 41, 4},
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 10 + 33 + 10, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5, 100, 15,
	 103, 4},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 3, 100, 15, 4, 4},

	//Fifth action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 15 + 5, 15, 15,
	 50, 3},
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 15 + 5, 15, 15,
	 51, 4},
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 10 + 33 + 10, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 15 + 5,
	 100, 15, 104, 5},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 15 + 5 + 3, 100,
	 15, 5, 5},

	//OK
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 30 + 60, 10 + 106, 54, 16, 60, 9},
	//Previous-Next
	{GUI_NEXTTEXT, 0x01, GWF_BUTTON, 30 + 120, 10 + 106, 54, 16, 61, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const GuiWidget about_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 30, 20, 260, 124, 0, 0},
	{GUI_CUSTOMTEXT, 0x01, 0, 30 + 68, 20 + 10, 160, 15, 0, 9},	// Build
	{GUI_CUSTOMTEXT, 0x01, 0, 30 + 10, 20 + 30, 240, 15, 0, 10},	// ScummVM Url
	{GUI_CUSTOMTEXT, 0x01, 0, 30 + 75, 20 + 50, 150, 15, 0, 11},	// Lucasarts
	{GUI_CUSTOMTEXT, 0x01, 0, 30 + 110, 20 + 64, 40, 15, 0, 21},	// Except:
	{GUI_CUSTOMTEXT, 0x01, 0, 30 + 25, 20 + 78, 210, 15, 0, 22},	// Adventuresoft
	{GUI_SCROLLTEXT, 0x01, 0, 30 + 95, 20 + 10, 100, 15, 0},
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 30 + 100, 20 + 100, 54, 16, 40, 9},
	{0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const GuiWidget options_dialog[] = {
	// Only display the "Keys" options for WinCE
#ifdef _WIN32_WCE
	{GUI_STAT, 0xFF, GWF_DEFAULT, 50, 80, 210, 60, 0, 0},
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10, 80 + 10, 40, 15, 1, 5, 'S'},	// Sound
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10 + 40 + 30, 80 + 10, 40, 15, 2, 6, 'K'},	// Keys
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10 + 40 + 30 + 40 + 30, 80 + 10, 40, 15, 3, 7, 'A'},	// About
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10, 80 + 10 + 15 + 10, 40, 15, 4, 18, 'M'},	// Misc
	{0, 0, 0, 0, 0, 0, 0, 0, 0}
#else
	{GUI_STAT, 0xFF, GWF_DEFAULT, 50, 80, 210, 40, 0, 0},
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10, 80 + 10, 40, 15, 1, 5, 'S'},	// Sound
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10 + 40 + 30, 80 + 10, 40, 15, 3, 7, 'A'},	// About
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10 + 40 + 30 + 40 + 30, 80 + 10, 40, 15, 4, 18, 'M'},	// Misc
	{0, 0, 0, 0, 0, 0, 0, 0, 0}
#endif
};

const GuiWidget misc_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 50, 80, 210, 65, 0, 0},
	{GUI_CHECKBOX, 0x01, GWF_DEFAULT, 50 + 10, 80 + 6, 14, 14, 1, 0, 'S'},	// checkbox for subtitles
	{GUI_CUSTOMTEXT, 0x01, 0, 50 + 10 + 20, 80 + 10, 140, 15, 0, 19},	// "Show speech subtitles"
	{GUI_CHECKBOX, 0x01, GWF_DEFAULT, 50 + 10, 80 + 6 + 16, 14, 14, 5, 0, 'A'},	// checkbox for amiga pallete
	{GUI_CUSTOMTEXT, 0x01, 0, 50 + 10 + 20, 80 + 10 + 15, 140, 15, 0, 20},	// "Amiga pallete conversion"
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 50 + 10 + 20, 80 + 10 + 15 + 20, 54, 16, 3, 9, 13},	// ok
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 50 + 10 + 20 + 80, 80 + 10 + 15 + 20, 54, 16, 4, 7},	// cancel
	{0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const GuiWidget sound_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 30, 20, 260, 120, 0, 0},
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 20 + 11, 15, 15, 1, 3},	// Plus
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 20 + 11, 15, 15, 2, 4},	// Minus
	{GUI_VARTEXT, 0x01, GWF_DEFAULT, 30 + 73, 20 + 11, 128, 15, 3, 0},	// Master
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 20 + 25 + 11, 15, 15, 11, 3},	// Plus
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 20 + 25 + 11, 15, 15, 12, 4},	// Minus
	{GUI_VARTEXT, 0x01, GWF_BUTTON, 30 + 73, 20 + 25 + 11, 128, 15, 13, 1},	// Music
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 20 + 25 + 25 + 11, 15, 15, 21, 3},	// Plus
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 20 + 25 + 25 + 11, 15, 15, 22, 4},	// Minus
	{GUI_VARTEXT, 0x01, GWF_BUTTON, 30 + 73, 20 + 25 + 25 + 11, 128, 15, 23, 2},	// SFX
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 30 + (260 / 2) - 80, 20 + 25 + 25 + 11 + 25, 54, 16, 40, 9, 13},	/* OK */
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 30 + (260 / 2), 20 + 25 + 25 + 11 + 25, 54, 16, 50, 7},	/* Cancel */
	{0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const GuiWidget save_load_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT | GWF_PARENT, 30, 20, 260, 124, 0, 0},
	{GUI_RESTEXT, 0x01, 0, 10, 7, 240, 16, 0, 1},	/* How may I serve you? */
	{GUI_RESTEXT, 0x02, 0, 10, 7, 240, 16, 0, 2},	/* Select a game to LOAD */
	{GUI_RESTEXT, 0x04, 0, 10, 7, 240, 16, 0, 3},	/* Name your SAVE game */

	{GUI_STAT, 0xFF, GWF_DEFAULT, 6, 20, 170, 96, 0, 0},
	{GUI_UPDOWNARROW, 0x01, GWF_BUTTON, 180, 24, 16, 40, 0, 0},	/* Up (dummy) */
	{GUI_UPDOWNARROW, 0x01, GWF_BUTTON, 180, 72, 16, 40, 0, 1},	/* Down (dummy) */
	{GUI_UPDOWNARROW, 0xFE, GWF_BUTTON, 180, 24, 16, 40, 1, 0},	/* Up */
	{GUI_UPDOWNARROW, 0xFE, GWF_BUTTON, 180, 72, 16, 40, 2, 1},	/* Down */

	{GUI_RESTEXT, 0x06, GWF_CLEARBG, 10, 24, 160, 10, 20, 0},
	{GUI_RESTEXT, 0x06, GWF_CLEARBG, 10, 34, 160, 10, 21, 0},
	{GUI_RESTEXT, 0x06, GWF_CLEARBG, 10, 44, 160, 10, 22, 0},
	{GUI_RESTEXT, 0x06, GWF_CLEARBG, 10, 54, 160, 10, 23, 0},
	{GUI_RESTEXT, 0x06, GWF_CLEARBG, 10, 64, 160, 10, 24, 0},
	{GUI_RESTEXT, 0x06, GWF_CLEARBG, 10, 74, 160, 10, 25, 0},
	{GUI_RESTEXT, 0x06, GWF_CLEARBG, 10, 84, 160, 10, 26, 0},
	{GUI_RESTEXT, 0x06, GWF_CLEARBG, 10, 94, 160, 10, 27, 0},
	{GUI_RESTEXT, 0x06, GWF_CLEARBG, 10, 104, 160, 10, 28, 0},

	{GUI_RESTEXT, 0x01, GWF_BUTTON, 200, 20, 54, 16, 3, 4, 'S'},	/* Save */
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 200, 40, 54, 16, 4, 5, 'L'},	/* Load */
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 200, 60, 54, 16, 5, 6, 'P'},	/* Play */
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 200, 80, 54, 16, 9, 17, 'O'},	/* Options */
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 200, 100, 54, 16, 6, 8, 'Q'},	/* Quit */

	{GUI_RESTEXT, 0x02, GWF_BUTTON, 200, 60, 54, 16, 7, 7},	/* Cancel */

	{GUI_RESTEXT, 0x04, GWF_BUTTON, 200, 40, 54, 16, 8, 9},	/* Ok */
	{GUI_RESTEXT, 0x04, GWF_BUTTON, 200, 60, 54, 16, 7, 7},	/* Cancel */
	{0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const GuiWidget pause_dialog[] = {
	{GUI_RESTEXT, 0x01, GWF_DEFAULT, 50, 80, 220, 16, 0, 10},
	{0, 0, 0, 0, 0, 0, 0, 0, 0}
};

void Gui::draw(int start, int end)
{
	int i;

	if (end == -1)
		end = start;

	for (i = 0; i < (int)(sizeof(_widgets) / sizeof(_widgets[0])); i++) {
		const GuiWidget *w = _widgets[i];
		if (w) {
			_parentX = 0;
			_parentY = 0;
			while (w->_type != GUI_NONE) {
				if (w->_id >= start && w->_id <= end && (w->_page & (1 << _cur_page))) {
					drawWidget(w);
				}
				if (w->_flags & GWF_PARENT) {
					_parentX += w->_x;
					_parentY += w->_y;
				}
				w++;
			}
		}
	}
}

const GuiWidget *Gui::widgetFromPos(int x, int y)
{
	int i;

	for (i = sizeof(_widgets) / sizeof(_widgets[0]) - 1; i >= 0; i--) {
		const GuiWidget *w = _widgets[i];
		if (w) {
			while (w->_type != GUI_NONE) {
				if ((w->_page & (1 << _cur_page)) && w->_id &&
						(uint16)(x - w->_x) < w->_w && (uint16)(y - w->_y) < w->_h)
					return w;
				if (w->_flags & GWF_PARENT) {
					x -= w->_x;
					y -= w->_y;
				}
				w++;
			}
		}
	}
	return NULL;
}

void Gui::drawChar(const char str, int xx, int yy)
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
void Gui::drawString(const char *str, int x, int y, int w, byte color, bool center)
{
	StringTab *st = &_s->string[5];
	st->charset = 1;
	st->center = center;
	st->color = color;
	st->xpos = x;
	st->ypos = y;
	st->right = x + w;

	if (_s->_gameId) {						/* If a game is active.. */
		_s->_messagePtr = (byte *)str;
		_s->drawString(5);
	} else {
		for (uint letter = 0; letter < strlen(str); letter++)
			drawChar(str[letter], st->xpos + (letter * 8), st->ypos);
	}
}

void Gui::drawWidget(const GuiWidget *w)
{
	const char *s;
	int x, y;

	x = w->_x;
	y = w->_y;

	if (w->_flags & GWF_CLEARBG)
		widgetClear(w);

	if (w->_flags & GWF_BORDER) {
		widgetBorder(w);
		x += 4;
		y += 4;
	}

	switch (w->_type) {
	case GUI_CUSTOMTEXT:
	case GUI_VARTEXT:
	case GUI_KEYTEXT:
	case GUI_ACTIONTEXT:
	case GUI_RESTEXT:
	case GUI_NEXTTEXT:
		{
			char text[500];
			text[0] = '\0';

			switch (w->_type) {
			case GUI_CUSTOMTEXT:
				strcpy(text, string_map_table_custom[w->_string_number]);
				break;
			case GUI_RESTEXT:
				s = queryString(w->_string_number, w->_id);
				if (s) {
					int t = resStrLen(s);
					if (t >= 500) { // probably won't happen, but just in case...
						warning("Resource string is too long, truncating");
						t = 498;
						text[499] = '\0';
					}
					memcpy(text, s, t+1);					
				}
				break;
			case GUI_VARTEXT:
				sprintf(text, "%s %d", string_map_table_custom[w->_string_number],
								_gui_variables[w->_string_number]);
				break;
			case GUI_SCROLLTEXT:
				sprintf(text, "%s", _gui_scroller);
				break;
#ifdef _WIN32_WCE
			case GUI_KEYTEXT:
				strcpy(text, getGAPIKeyName(getAction((_current_page * 5) + w->_string_number - 1)->action_key));
				break;
			case GUI_ACTIONTEXT:
				strcpy(text, getActionName(getAction((_current_page * 5) + w->_string_number - 1)->action_type));
				break;
			case GUI_NEXTTEXT:
				if (_current_page == 0)
					strcpy(text, "Next");
				else
					strcpy(text, "Prev");
				break;

#endif
			}

			if (*text) {
				drawString(text, x + _parentX, y + _parentY, w->_w,
									 (_clickWidget && _clickWidget == w->_id) ? _textcolorhi : _textcolor, false);
			}
			break;
		}
	case GUI_IMAGE:
		break;
	case GUI_UPDOWNARROW:
	case GUI_CHECKBOX:
		{
			uint32 *data;
			byte color = (_clickWidget && _clickWidget == w->_id) ? _textcolorhi : _textcolor;

			if (w->_type == GUI_UPDOWNARROW) {
				if (w->_string_number == 0)
					data = up_arrow;
				else
					data = down_arrow;
				// if not an updownarrow, it must be a checkbox
			} else {
				data = checked_img;
			}

			// Center the image
			x += w->_w / 2 - IMG_SIZE / 2;
			y += w->_h / 2 - IMG_SIZE / 2;
			if (w->_flags & GWF_BORDER) {
				x -= 4;
				y -= 4;
			}

			byte *ptr = getBasePtr(x, y);
			if (ptr == NULL)
				return;

			// If the checkbox is checked, or this is not a checkbox, draw the image
			if ((getCheckboxChecked(w->_id) == true) || (w->_type != GUI_CHECKBOX)) {
				for (int y2 = 0; y2 < IMG_SIZE; y2++) {
					uint32 mask = 0xF0000000;
					for (int x2 = 0; x2 < IMG_SIZE; x2++) {
						if (data[y2] & mask)
							ptr[x2] = color;
						mask >>= 4;
					}
					ptr += _s->_realWidth;
				}
			}
		}
		break;
	}
}

void Gui::widgetClear(const GuiWidget *wid)
{
	int x = wid->_x;
	int y = wid->_y;
	int w = wid->_w;
	int h = wid->_h;
	int i;

	byte *ptr = getBasePtr(x, y);
	if (ptr == NULL)
		return;

	_s->setVirtscreenDirty(_vs, x + _parentX, y + _parentY, x + _parentX + w, y + _parentY + h);

	if (wid->_flags & GWF_BORDER) {
		// Inset by 1 pixel in all directions
		ptr += _s->_realWidth + 1;
		w -= 2;
		h -= 2;
	}

	while (h--) {
		for (i = 0; i < w; i++)
			ptr[i] = _bgcolor;
		ptr += _s->_realWidth;
	}
}

void Gui::widgetBorder(const GuiWidget *w)
{
	box(w->_x, w->_y, w->_w, w->_h);
}

void Gui::box(int x, int y, int width, int height)
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

byte *Gui::getBasePtr(int x, int y)
{
	x += _parentX;
	y += _parentY;
	_vs = _s->findVirtScreen(y);

	if (_vs == NULL)
		return NULL;

	return _vs->screenPtr + x + (y - _vs->topline) * _s->_realWidth +
		_s->_screenStartStrip * 8 + (_s->camera._cur.y - (_s->_realHeight / 2)) * _s->_realWidth;
}

void Gui::line(int x, int y, int x2, int y2, byte color)
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

void Gui::leftMouseClick(int x, int y)
{
	const GuiWidget *w = widgetFromPos(x, y);
	int old;

	_clickTimer = 0;

	old = _clickWidget;
	_clickWidget = w ? w->_id : 0;

	if (old)
		draw(old);
	if (_clickWidget) {
		draw(_clickWidget);
		if (w->_flags & GWF_DELAY)
			_clickTimer = 5;
		else
			handleCommand(_clickWidget);
	}

	if (_dialog == PAUSE_DIALOG)
		close();
}

void Gui::handleSoundDialogCommand(int cmd)
{
	if (cmd == 50) {
		close();
	} else if (cmd == 40) {
		_s->_sound->_sound_volume_master = _gui_variables[0];	// Master
		_s->_sound->_sound_volume_music = _gui_variables[1];	// Music
		_s->_sound->_sound_volume_sfx = _gui_variables[2];		// SFX

		_s->_imuse->set_music_volume(_s->_sound->_sound_volume_music);
		_s->_imuse->set_master_volume(_s->_sound->_sound_volume_master);
		_s->_mixer->setVolume(_s->_sound->_sound_volume_sfx);
		_s->_mixer->setMusicVolume(_s->_sound->_sound_volume_music);

		scummcfg->setInt("master_volume", _s->_sound->_sound_volume_master);
		scummcfg->setInt("music_volume", _s->_sound->_sound_volume_music);
		scummcfg->setInt("sfx_volume", _s->_sound->_sound_volume_sfx);
		scummcfg->flush();

		close();
	} else {
		if ((cmd % 10) == 1) {
			_gui_variables[cmd / 10] += 5;
			if (_gui_variables[cmd / 10] > 256)
				_gui_variables[cmd / 10] = 256;
		} else {
			_gui_variables[cmd / 10] -= 5;
			if (_gui_variables[cmd / 10] < 0)
				_gui_variables[cmd / 10] = 0;
		}

		draw((cmd / 10) * 10 + 3, (cmd / 10) * 10 + 3);
	}
}

void Gui::handleOptionsDialogCommand(int cmd)
{
	switch (cmd) {
	case 1:
		_widgets[0] = sound_dialog;
		_gui_variables[0] = _s->_sound->_sound_volume_master;
		_gui_variables[1] = _s->_sound->_sound_volume_music;
		_gui_variables[2] = _s->_sound->_sound_volume_sfx;
		_active = true;
		_cur_page = 0;
		_dialog = SOUND_DIALOG;
		draw(0, 100);
		return;
	case 2:
		_key_mapping_required = 0;
		get_key_mapping = true;
		_widgets[0] = keys_dialog;
		_active = true;
		_cur_page = 0;
		_dialog = KEYS_DIALOG;
		draw(0, 200);
		return;
	case 3:
		_widgets[0] = about_dialog;
		_active = true;
		_cur_page = 0;
		_return_to = 0;
		_dialog = ABOUT_DIALOG;
		draw(0, 100);
		return;
	case 4:
		_widgets[0] = misc_dialog;
		_active = true;
		_cur_page = 0;
		_return_to = 0;
		_dialog = MISC_DIALOG;
		clearCheckboxes();
		setCheckbox(!(_s->_noSubtitles), 1);
		if (_s->_features & GF_AMIGA)
			setCheckbox(true, 5);
		else
			setCheckbox(false, 5);
		draw(0, 100);
		return;
	}
}

void Gui::handleMiscDialogCommand(int cmd)
{
	switch (cmd) {
	case 1:
		if ((getCheckboxChecked(1)) == true)
			setCheckbox(false, 1);
		else
			setCheckbox(true, 1);
		draw(1, 1);
		return;
	case 5:
		if (getCheckboxChecked(5) == true)
			setCheckbox(false, 5);
		else
			setCheckbox(true, 5);
		draw(5, 5);
		return;
	case 3:
	case 4:
		// OK button - perform the actions of the checkboxes
		if (cmd == 3) {
			// The opposite of the checkbox(1) is set because the internal variable is 'no subtitles' but
			// a "Show subtitles" option makes more usability sense
			_s->_noSubtitles = (!getCheckboxChecked(1));

			// Amiga pallete conversion checkbox
			if (getCheckboxChecked(5))
				_s->_features = _s->_features | GF_AMIGA;
			else
				_s->_features = _s->_features & ~GF_AMIGA;
			_s->_fullRedraw = true;
		}
		close();
	}
}

void Gui::handleKeysDialogCommand(int cmd)
{
#ifdef _WIN32_WCE
	if (cmd < 100 && cmd != 60 && cmd != 61) {

		if ((cmd % 10) == 1)
			setNextType((_current_page * 5) + (cmd / 10) - 1);
		else
			setPreviousType((_current_page * 5) + (cmd / 10) - 1);

		draw(0, 200);

		return;
	}

	if (cmd >= 100)
		_key_mapping_required = cmd;

	if (cmd == 60) {
		get_key_mapping = false;
		save_key_mapping();
		close();
	}

	if (cmd == 61) {
		if (!_current_page)
			_current_page = 1;
		else
			_current_page = 0;
		draw(0, 200);
		return;
	}
#else
	close();
#endif
}

void Gui::handleLauncherDialogCommand(int cmd)
{
	debug(9, "handle launcher command\n");
	switch (cmd) {
	case 20:
		close();
		break;
	case 21:
		// Nothing yet
		break;
	case 22:
		_widgets[0] = about_dialog;
		_active = true;
		_cur_page = 0;
		_return_to = LAUNCHER_DIALOG;
		_dialog = ABOUT_DIALOG;
		draw(0, 100);
		debug(9, "about dialog\n");
		break;
	default:
		debug(9, "default\n");
		close();
	}
}

void Gui::handleCommand(int cmd)
{
	int lastEdit = _editString;
	showCaret(false);

	switch (_dialog) {
	case LAUNCHER_DIALOG:
		handleLauncherDialogCommand(cmd);
		return;
	case SOUND_DIALOG:
		handleSoundDialogCommand(cmd);
		return;
	case OPTIONS_DIALOG:
		handleOptionsDialogCommand(cmd);
		return;
	case MISC_DIALOG:
		handleMiscDialogCommand(cmd);
		return;
	case KEYS_DIALOG:
		handleKeysDialogCommand(cmd);
		return;
	case ABOUT_DIALOG:
		if (_return_to == LAUNCHER_DIALOG) {
			_widgets[0] = launcher_dialog;
			_active = true;
			_cur_page = 0;
			_dialog = LAUNCHER_DIALOG;
			draw(0, 100);
		} else
			close();
		return;
	}

	// If we get here, it's the SAVELOAD_DIALOG

	switch (cmd) {
	case 1:											/* up button */
		if (_slotIndex - 9 < 0)
			return;
		getSavegameNames(_slotIndex - 9);
		draw(20, 28);
		return;
	case 2:											/* down button */
		if (_slotIndex + 9 > 80)
			return;
		getSavegameNames(_slotIndex + 9);
		draw(20, 28);
		return;
	case 3:											/* save button */
		_cur_page = 2;
		getSavegameNames(1);				/* Start at 1, since slot 0 is reserved for autosave */
		draw(0, 100);
		return;
	case 4:											/* load button */
		_cur_page = 1;
		getSavegameNames(0);
		draw(0, 100);
		return;
	case 5:											/* play button */
		close();
		return;
	case 6:											/* quit button */
#ifdef _WIN32_WCE
		do_quit();
#endif
		exit(1);
		return;
	case 7:											/* cancel button */
		_cur_page = 0;
		draw(0, 100);
		return;
	case 8:											/* ok button (save game) */
		if (lastEdit == -1 || game_names[lastEdit][0] == 0)
			return;

		_s->_saveLoadSlot = lastEdit + _slotIndex;
		_s->_saveLoadCompatible = false;
		_s->_saveLoadFlag = 1;
		memcpy(_s->_saveLoadName, game_names[lastEdit], sizeof(_s->_saveLoadName));
		close();
		return;
	case 9:											/* options button */
		options();
		draw(0, 100);
		return;
	default:
		if (cmd >= 20 && cmd <= 28) {
			if (_cur_page == 1) {
				if (valid_games[cmd - 20]) {
					_s->_saveLoadSlot = cmd - 20 + _slotIndex;
					_s->_saveLoadCompatible = false;
					_s->_saveLoadFlag = 2;
					close();
				}
				return;
			} else if (_cur_page == 2) {
				_clickWidget = cmd;
				editString(cmd - 20);
			}
		}
	}

}

void Gui::getSavegameNames(int start)
{
	int i;
	_slotIndex = start;

	for (i = 0; i < 9; i++) {
		valid_games[i] = _s->getSavegameName(start + i, game_names[i]);
	}
}

const char *Gui::queryString(int stringno, int id)
{
	static char namebuf[64];
	char *result;
	int string;

	if (id >= 20 && id <= 28) {
		// Save game names
		sprintf(namebuf, "%2d. %s", id - 20 + _slotIndex, game_names[id - 20]);
		return namebuf;
	}

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

void Gui::showCaret(bool show)
{
	int i;
	char *s;

	if (_editString == -1)
		return;

	i = _editLen;
	s = game_names[_editString];

	if (show) {
		if (i < SAVEGAME_NAME_LEN - 1) {
			s[i] = '_';
			s[i + 1] = 0;
		}
	} else {
		s[i] = 0;
	}

	draw(_editString + 20);

	if (!show)
		_editString = -1;
}

void Gui::editString(int i)
{
	char *s = game_names[i];
	if (!valid_games[i]) {
		valid_games[i] = true;
		*s = 0;
	}
	_editString = i;
	_editLen = strlen(s);
	showCaret(true);
}

void Gui::addLetter(byte letter)
{
	switch (_dialog) {
	case SAVELOAD_DIALOG:
		if (_editString == -1)
			return;

/*
		FIXME - this code here has no effect at all, since Scumm::convertKeysToClicks()
		swallows all return key events.
		// Return pressed?
		if (letter == '\n' || letter == '\r') { 
			handleCommand(8);
			return;
		}
*/

		if (letter >= 32 && letter < 128 && _editLen < SAVEGAME_NAME_LEN - 1) {
			game_names[_editString][_editLen++] = letter;
		} else if (letter == 8 && _editLen > 0) {
			_editLen--;
		}
		showCaret(true);
		break;
	case PAUSE_DIALOG:
		if (letter == 32)
			close();
		break;

#ifdef _WIN32_WCE
	case KEYS_DIALOG:
		clearActionKey(letter);
		if (_key_mapping_required)
			getAction((_current_page * 5) + _key_mapping_required - 100)->action_key = letter;
		_key_mapping_required = 0;
		draw(0, 200);
		break;
#endif
	}
}

bool Gui::getCheckboxChecked(int id)
{
	return _cbox_checked[id];
}

void Gui::setCheckbox(bool state, int id)
{
	_cbox_checked[id] = state;
}

void Gui::clearCheckboxes()
{
	for (int id = 0; id <= 100; id++) {
		_cbox_checked[id] = false;
	}
}

void Gui::init(Scumm *s)
{
	/* Default GUI colors */
	_bgcolor = 0;
	_color = 0;
	_textcolor = 8;								// 15 is nice
	_textcolorhi = 15;
	_shadowcolor = 0;
	_s = s;

	strcpy(_gui_scroller, "Brought to you by:");
}

void Gui::loop()
{
	if (_active && !_inited) {
		_inited = true;
		draw(0, 200);								// was 100    
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
	_s->animateCursor();
	_s->getKeyInput(0);
	if (_s->_mouseButStat & MBS_LEFT_CLICK) {
		leftMouseClick(_s->mouse.x, _s->mouse.y);
	} else if (_s->_lastKeyHit) {
		if (_dialog != KEYS_DIALOG) {
			if (_s->_lastKeyHit == 27)
				close();
			else {
				addLetter((unsigned char)_s->_lastKeyHit);
				checkHotKey(_s->_lastKeyHit);
			}
#ifdef _WIN32_WCE
		} else if (_s->_lastKeyHit > 1000) {	// GAPI
			addLetter(_s->_lastKeyHit - 1000);
#endif
		}
	}

	if (_clickTimer && !--_clickTimer) {
		int old = _clickWidget;
		_clickWidget = 0;
		draw(old);
		handleCommand(old);
	}

	_s->drawDirtyScreenParts();
	_s->_mouseButStat = 0;
}

void Gui::close()
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
	_active = false;
	_inited = false;

#ifdef _WIN32_WCE

	// Option dialog can be accessed from the file dialog now, always check
	if (save_hide_toolbar) {
		do_hide(true);
		save_hide_toolbar = false;
	}
	if (keyboard_override) {
		keyboard_override = false;
		draw_keyboard = false;
		toolbar_drawn = false;
	}
#endif
}

void Gui::checkHotKey(int keycode)
{
	byte page;
	for (int i = 0; i < (int)(sizeof(_widgets) / sizeof(_widgets[0])); i++) {
		const GuiWidget *w = _widgets[i];
		if (w) {
			while (w->_type != GUI_NONE) {

				// This rubbish is needed because the current page is 0 when really it should be 1
				if (_cur_page == 0)
					page = 1;
				else
					page = _cur_page;

				// Only check for widgets that are on the current GUI page (otherwise save dialog problems occur)
				if (w->_page == page) {
					// Check the actual key pressed, and the uppercase version. For people who have caps lock on
					if (keycode == w->_hotkey || toupper(keycode) == w->_hotkey)
						handleCommand(w->_id);
				}
				w++;
			}
		}
	}
}

void Gui::saveLoadDialog()
{
	_widgets[0] = save_load_dialog;
	_editString = -1;
	_active = true;
	_cur_page = 0;
	_dialog = SAVELOAD_DIALOG;

#ifdef _WIN32_WCE
	save_hide_toolbar = hide_toolbar;
	if (save_hide_toolbar) {
		// Display the keyboard while the dialog is running
		do_hide(false);
	}
	if (!draw_keyboard) {
		keyboard_override = true;
		draw_keyboard = true;
		toolbar_drawn = false;
	}
#endif

}

void Gui::pause()
{
	_widgets[0] = pause_dialog;
	_active = true;
	_cur_page = 0;
	_dialog = PAUSE_DIALOG;
}

void Gui::options()
{
#ifdef _WIN32_WCE
	_current_page = 0;
#endif
	_widgets[0] = options_dialog;
	_active = true;
	_cur_page = 0;
	_dialog = OPTIONS_DIALOG;
}

void Gui::launcher()
{
	_widgets[0] = launcher_dialog;
	_active = true;
	_cur_page = 0;
	_dialog = LAUNCHER_DIALOG;
}


















/*
void Gui::loop()
{
	if (_active && !_inited) {
		_inited = true;
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
	_s->animateCursor();
	_s->getKeyInput(0);
	
	_s->drawDirtyScreenParts();
	_s->_mouseButStat = 0;
}
*/
