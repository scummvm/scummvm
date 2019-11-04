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

#include "engines/chewy/cursor.h"
#include "common/textconsole.h"

#define false false
#define true true

cursor::cursor(mcga_grafik *iout, maus *iin, cur_blk *curblkp) {
	out = iout;
	in = iin;
	scr_width = scr_w;
	curblk = curblkp;
	inzeig = in->get_in_zeiger();
	if (!inzeig->minfo)
		maus_da = false;
	else {
		maus_da = true;
		minfo = inzeig->minfo;
		sichtbar = false;
		ani = false;
		cur_aufruf = false;
		ani_count = false;
	}
}

cursor::~cursor() {
}

void cursor::plot_cur() {
	if (maus_da && sichtbar) {
		if (cur_move == 1) {
			mouse_aktiv = 1;
			cur_move = 0;
			if (!curblk->no_back) {

				out->blockcopy(curblk->cur_back, cur_x_old, cur_y_old, scr_width);

				out->sprite_save(curblk->cur_back, (minfo->x + curblk->page_off_x),
				                  (minfo->y + curblk->page_off_y), curblk->xsize,
				                  curblk->ysize, scr_width);
			}

			cur_x_old = (minfo->x + curblk->page_off_x);
			cur_y_old = (minfo->y + curblk->page_off_y);
		}

		cur_aufruf -= 1;
		if ((cur_aufruf <= 0) && (ani != false)) {
			cur_aufruf = ani->delay;
			++ani_count;
			if (ani_count > ani->ani_end)
				ani_count = ani->ani_anf;
		}

		out->sprite_set(curblk->sprite[ani_count], cur_x_old, cur_y_old,
		                 scr_width);
		mouse_aktiv = 0;
	}
}

void cursor::show_cur() {
	warning("STUB - show_cur");
#if 0
	int16 x, y;
	if ((maus_da) && (!sichtbar)) {
		sichtbar = TRUE;
		mouse_aktiv = 1;
		x = minfo->x;
		y = minfo->y;
#pragma aux asm_pos = \
"push eax"\
"push edx"\
"push ecx"\
"mov ax,3"\
"int 033h"\
"mov x,cx"\
"mov y,dx"\
"pop ecx"\
"pop edx"\
"pop eax"
		asm_pos();
		minfo->x = x;
		minfo->y = y;
		if (curblk->no_back != TRUE) {

			out->sprite_save(curblk->cur_back, (minfo->x + curblk->page_off_x),
			                  (minfo->y + curblk->page_off_y), curblk->xsize,
			                  curblk->ysize, scr_width);
		}

		cur_x_old = (minfo->x + curblk->page_off_x);
		cur_y_old = (minfo->y + curblk->page_off_y);
		cur_move = 1;
		plot_cur();
	}
#endif
}

void cursor::hide_cur() {
	if ((maus_da) && (sichtbar)) {
		if (!curblk->no_back) {
			out->blockcopy(curblk->cur_back, cur_x_old, cur_y_old, scr_width);
		}
		sichtbar = false;
	}
}

void cursor::set_cur_ani(cur_ani *ani1) {
	ani = ani1;
	cur_aufruf = 0;
	ani_count = ani->ani_anf;
}

void cursor::move(int16 x, int16 y) {
	if (maus_da) {
		mouse_aktiv = 1;

		minfo->x = x;
		minfo->y = y;
		cur_x_old = (minfo->x + curblk->page_off_x);
		cur_y_old = (minfo->y + curblk->page_off_y);
		in->move_mouse(x, y);
		if (sichtbar == true)
			cur_move = 1;
		else
			cur_move = 0;
		mouse_aktiv = 0;
	}
}

void cursor::wait_taste_los(int16 maus_plot) {
	warning("STUB - wait_taste_los");
#if 0
	int16 is_mouse, stay;
	int16 switch_code;
	is_mouse = 0;
	if (maus_da) {

#pragma aux check_mknopf = \
"push ax"\
"push bx"\
"push cx"\
"push dx"\
"mov ax,3"\
"int 033h"\
"mov is_mouse,bx"\
"pop dx"\
"pop cx"\
"pop bx"\
"pop ax"
		check_mknopf();
	}
	if (!is_mouse)
		in->hot_key = 0;
	stay = 1;
	switch_code = 1;
	while ((switch_code != 0) && (stay)) {
		switch_code = in->get_switch_code();
		if (is_mouse) {
			switch_code = 2;
#pragma aux check_mknopf2 = \
"push ax"\
"push bx"\
"push cx"\
"push dx"\
"mov ax,3"\
"int 033h"\
"mov stay,bx"\
"pop dx"\
"pop cx"\
"pop bx"\
"pop ax"\
check_mknopf2();
		}
		if (maus_plot != FALSE)
			plot_cur();
	}
#endif
}

void cursor::wait_taste(int16 maus_plot) {
	while (in->get_switch_code() == 0)
		if (maus_plot != false)
			plot_cur();
}
