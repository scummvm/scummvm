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
#include "chewy/mouse.h"

namespace Chewy {

class Cursor {
public:
	Cursor();
	Cursor(McgaGraphics *out, InputMgr *in, CurBlk *curblk);
	~Cursor();

	void plot_cur();
	void show_cur();
	void hide_cur();
	void set_cur_ani(CurAni *ani);
	void move(int16 x, int16 y);

	McgaGraphics *_out = nullptr;
	InputMgr *_in = nullptr;
	KbdMouseInfo *_inzeig = nullptr;
	CurBlk *_curblk = nullptr;
	CurAni *_ani = nullptr;
	int _scrWidth = 0;
	int16 _cur_x_old = 0;
	int16 _cur_y_old = 0;
	int16 _cur_aufruf = 0;
	int16 _ani_count = 0;
	uint8 _maus_da = 0;
	bool _sichtbar = false;
};

} // namespace Chewy

#endif
