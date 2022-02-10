/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHEWY_CURSOR_H
#define CHEWY_CURSOR_H

#include "chewy/mcga_graphics.h"
#include "chewy/ngstypes.h"

namespace Chewy {

extern int16 scr_w;

class cursor {
public:
	cursor();
	cursor(McgaGraphics *out, InputMgr *in, cur_blk *curblk);
	~cursor();

	void plot_cur();

	void show_cur();

	void hide_cur();

	void set_cur_ani(cur_ani *ani);

	void move(int16 x, int16 y);
	void wait_taste_los(bool maus_plot);

	McgaGraphics *out = nullptr;
	InputMgr *in = nullptr;
	maus_info *minfo = nullptr;
	in_zeiger *inzeig = nullptr;
	cur_blk *curblk = nullptr;
	cur_ani *ani = nullptr;
	int scr_width = 0;
	int16 cur_x_old = 0;
	int16 cur_y_old = 0;
	int16 cur_aufruf = 0;
	int16 ani_count = 0;
	uint8 maus_da = 0;
	bool sichtbar = false;
};

} // namespace Chewy

#endif
