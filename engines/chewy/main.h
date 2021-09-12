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

#ifndef CHEWY_MAIN_H
#define CHEWY_MAIN_H

#include "graphics/surface.h"
#include "common/rect.h"

#include "chewy/defines.h"
#include "chewy/global.h"

void game_main();
void free_buffers();
void plot_main_menu();
void test_menu();
void shading(int16 new_shad_nr);
void palcopy(char *dipal, char *sipal, int16 distart,
             int16 sistart, int16 anz);
short calc_maus_txt(int16 x, int16 y, int16 mode);
void calc_ani_timer();
void mous_obj_action(int16 nr, int16 mode, int16 txt_mode, int16 txt_nr);
int16 get_ani_richtung(int16 zustand);
void menu_entry();
void menu_exit();


class ChewyFont {
public:
	ChewyFont(Common::String filename);
	virtual ~ChewyFont();

	Graphics::Surface *getLine(const Common::String &texts);
	void setDisplaySize(uint16 width, uint16 height);
	void setDeltaX(uint16 deltaX);

	uint16 getDataWidth() { return _dataWidth; }
	uint16 getDataHeight() { return _dataHeight; }

private:
	uint16 _count, _first, _last;
	uint16 _dataWidth, _dataHeight;
	uint16 _displayWidth, _displayHeight;
	uint16 _deltaX;

	Graphics::Surface _fontSurface;
};

class FontMgr {
private:
	ChewyFont *_font;

public:
	FontMgr() : _font(nullptr) {};
	virtual ~FontMgr() {};

	Graphics::Surface *getLine(const Common::String &texts);

	void setFont(ChewyFont *font) { _font = font; }
	ChewyFont *getFont() { return _font; }
};

extern ChewyFont *_font6;
extern ChewyFont *_font8;
extern FontMgr *_fontMgr;

#endif
