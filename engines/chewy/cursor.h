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

#ifndef CHEWY_CURSOR_H
#define CHEWY_CURSOR_H

#include "engines/chewy/mcga.h"
#include "engines/chewy/ngstypes.h"

extern int16 scr_w;

class cursor {

public:
	cursor();
	cursor(mcga_grafik *out, maus *in, cur_blk *curblk);
	~cursor();

	void plot_cur();

	void show_cur();

	void hide_cur();

	void set_cur_ani(cur_ani *ani);

	void move(int16 x, int16 y);
	void wait_taste_los(int16 maus_plot);
	void wait_taste(int16 maus_plot);

	mcga_grafik *out;
	maus *in;
	maus_info *minfo;
	in_zeiger *inzeig;
	cur_blk *curblk;
	cur_ani *ani;
	int scr_width;
	int16 cur_x_old;
	int16 cur_y_old;
	int16 cur_aufruf;
	int16 ani_count;
	uint8 maus_da;
	bool sichtbar;
};
#endif
