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

#ifndef CHEWY_MAUS_H
#define CHEWY_MAUS_H

#include "engines/chewy/ngstypes.h"

void set_new_kb_handler(kb_info *key);
void set_old_kb_handler();
void del_kb_puffer();

void set_mouse_handler(maus_info *mpos);

extern uint8 new_kb;

extern uint8 mouse_on;
extern uint8 mouse_show;
extern int16 mouse_button;
extern uint8 mouse_links;
extern uint8 mouse_links_los;
extern uint8 mouse_aktiv;
extern uint8 cur_move;
extern uint8 mouse_hor;
extern uint8 mouse_ver;
extern uint8 mouse_hot_x;
extern uint8 mouse_hot_y;

extern uint8 jflag;
extern uint8 joy_code;
extern uint8 button;
extern int16 xdiff;
extern int16 ydiff;
extern int16 joyx;
extern int16 joyy;
extern int16 eck1x;
extern int16 eck1y;
extern int16 eck2x;
extern int16 eck2y;

extern int16 modul;
extern int16 fcode;

class maus {
public:
	maus();
	~maus();
	int init();
	void speed(int16 x, int16 y);
	void move_mouse(int16 x, int16 y);
	void rectangle(int16 xmin, int16 ymin, int16 xmax, int16 ymax);

	void neuer_kb_handler(kb_info *key);
	void alter_kb_handler();
	void flush_kb_puffer();

	void neuer_maushandler(maus_info *mpos);
	maus_info *maus_info_adr();

	int16 maus_vector(int16 x, int16 y, int16 *tbl, int16 anz);

	in_zeiger *get_in_zeiger();
	int16 get_switch_code();
	int16 hot_key;
private:
	maus_info *maus_info_blk;
	kb_info *kb_info_blk;
	in_zeiger inzeig;
};
#endif
