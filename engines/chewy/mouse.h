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

#ifndef CHEWY_MOUSE_H
#define CHEWY_MOUSE_H

#include "chewy/ngstypes.h"

namespace Chewy {

void set_new_kb_handler(kb_info *key);
void set_old_kb_handler();

void set_mouse_handler(maus_info *mpos);

extern bool mouse_links_los;
extern bool mouse_active;
extern bool cur_move;

class InputMgr {
public:
	InputMgr();
	~InputMgr();

	int init();
	void speed(int16 x, int16 y);
	void move_mouse(int16 x, int16 y);
	void rectangle(int16 xmin, int16 ymin, int16 xmax, int16 ymax);

	void neuer_kb_handler(kb_info *key);
	void alter_kb_handler();
	void neuer_maushandler(maus_info *mpos);
	int16 maus_vector(int16 x, int16 y, const int16 *tbl, int16 anz);

	in_zeiger *get_in_zeiger();
	int16 get_switch_code();

	int16 _hotkey = 0;

private:
	maus_info *_mouseInfoBlk = nullptr;
	kb_info *_kbInfoBlk = nullptr;
	in_zeiger _inzeig;
};

} // namespace Chewy

#endif
