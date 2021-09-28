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

#include "common/textconsole.h"
#include "chewy/maus.h"
#include "chewy/events.h"

namespace Chewy {

// FIXME. Externals
uint8 mouse_show;
uint8 mouse_links_los;
uint8 mouse_aktiv;

uint8 jflag;
int16 xdiff;
int16 ydiff;
int16 eck1x;
int16 eck1y;
int16 eck2x;
int16 eck2y;
// end of externals

uint8 cur_move;
uint8 mouse_hot_x;
uint8 mouse_hot_y;


void set_new_kb_handler(kb_info *key) {
	warning("STUB: set_new_kb_handler()");
}

void set_old_kb_handler() {
	warning("STUB: set_old_kb_handler()");
}

void del_kb_puffer() {
	warning("STUB: del_kb_puffer()");
}

void set_mouse_handler(maus_info *mpos) {
	warning("STUB: set_mouse_handler()");
}

maus::maus() {
	maus_info_blk = nullptr;
	kb_info_blk = nullptr;
}

maus::~maus() {
}

int maus::init() {
	warning("maus::init - STUB");
#if 0
	int16 tasten;
	int16 err;
#pragma aux asm_init = "push ax"\
"mov ax,0"\
"int 033h"\
"mov err,ax"\
"mov tasten,bx"\
"pop ax"
	asm_init();
	if (err != -1) {
		tasten = 0;
		modul = MAUS;
		fcode = 0;
	}
	maus_info_blk = false;
	return (tasten);
#endif
	return 0;
}

void maus::speed(int16 x, int16 y) {
	warning("STUB - maus::speed");
#if 0
	if (x <= 0)
		x = 1;
	if (y <= 0)
		y = 1;
#pragma aux asm_speed = "push ax"\
"push cx"\
"push dx"\
"mov ax,0x0f"\
"mov cx,x"\
"mov dx,y"\
"int 0x33"\
"pop dx"\
"pop cx"\
"pop ax"
	asm_speed();
#endif
}

void maus::move_mouse(int16 x, int16 y) {
	g_events->warpMouse(Common::Point(x, y));
}

void maus::rectangle(int16 xmin, int16 ymin, int16 xmax, int16 ymax) {
	int16 txmin, tymin, txmax, tymax;
	txmin = xmin;
	tymin = ymin;
	txmax = xmax;
	tymax = ymax;
#pragma aux asm_rectangle = "push ax"\
"push cx"\
"push dx"\
"mov ax,7"\
"mov cx,txmin"\
"mov dx,txmax"\
"int 033h"\
"mov ax,8"\
"mov cx,tymin"\
"mov dx,tymax"\
"int 033h"\
"pop dx"\
"pop cx"\
"pop ax"\
asm_rectangle();
}

int16 maus::maus_vector(int16 x, int16 y, int16 *tbl,
                            int16 anz) {
	int16 i, j;
	i = -1;
	for (j = 0; (j < anz * 4) && (i == -1); j += 4) {
		if ((x >= tbl[j]) && (x <= tbl[j + 2]) &&
		        (y >= tbl[j + 1]) && (y <= tbl[j + 3]))
			i = j / 4;
	}
	return (i);
}

void maus::neuer_kb_handler(kb_info *key) {
	set_new_kb_handler(key);
	kb_info_blk = key;
	kb_info_blk->key_code = 0;
}

void maus::alter_kb_handler() {
	set_old_kb_handler();
	kb_info_blk = false;
	warning("STUB - maus::alter_kb_handler");
#if 0
	while (kbhit())
		getch();
#endif
}

void maus::flush_kb_puffer() {
	del_kb_puffer();
}

void maus::neuer_maushandler(maus_info *mpos) {
	set_mouse_handler(mpos);
	maus_info_blk = mpos;
}

in_zeiger *maus::get_in_zeiger() {
	inzeig.minfo = maus_info_blk;
	inzeig.kbinfo = kb_info_blk;
	return (&inzeig);
}

int16 maus::get_switch_code() {
	int16 switch_code;
	switch_code = 0;

	if (maus_info_blk != false) {
		if (maus_info_blk->button == 2) {
			switch_code = 1;
		} else if (maus_info_blk->button == 1)
			switch_code = 255;
		else if (maus_info_blk->button == 4)
			switch_code = 254;
	}

	if (kb_info_blk != false)
		if (kb_info_blk->key_code != 0)
			switch_code = (int16)kb_info_blk->key_code;

	if (hot_key != 0) {
		switch_code = (int16)hot_key;
		hot_key = 0;
	}
	return (switch_code);
}

} // namespace Chewy
