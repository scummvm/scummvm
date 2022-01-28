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

#include "common/textconsole.h"
#include "chewy/mouse.h"
#include "chewy/events.h"

namespace Chewy {

// FIXME. Externals
uint8 mouse_links_los;
uint8 mouse_active;

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
	g_events->setKbdInfo(key);
}

void set_old_kb_handler() {
	g_events->setKbdInfo(nullptr);
}

void del_kb_puffer() {
	g_events->clearEvents();
}

void set_mouse_handler(maus_info *mpos) {
	// No implementation in ScummVM
}

maus::maus() {
	maus_info_blk = nullptr;
	kb_info_blk = nullptr;
}

maus::~maus() {
}

int maus::init() {
	// ScummVM supports three buttons
	return 3;
}

void maus::speed(int16 x, int16 y) {
	// Changing mouse speed isn't supported in ScummVM
}

void maus::move_mouse(int16 x, int16 y) {
	g_events->warpMouse(Common::Point(x, y));
}

void maus::rectangle(int16 xmin, int16 ymin, int16 xmax, int16 ymax) {
	// Mouse clip rectangle isn't supported in ScummVM
}

int16 maus::maus_vector(int16 x, int16 y, const int16 *tbl,
                            int16 anz) {
	int16 i, j;
	i = -1;
	for (j = 0; (j < anz * 4) && (i == -1); j += 4) {
		if ((x >= tbl[j]) && (x <= tbl[j + 2]) &&
		        (y >= tbl[j + 1]) && (y <= tbl[j + 3]))
			i = j / 4;
	}

	return i;
}

void maus::neuer_kb_handler(kb_info *key) {
	set_new_kb_handler(key);
	kb_info_blk = key;
	kb_info_blk->key_code = '\0';
}

void maus::alter_kb_handler() {
	set_old_kb_handler();
	kb_info_blk = nullptr;
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

	return &inzeig;
}

int16 maus::get_switch_code() {
	int16 switch_code;
	switch_code = 0;

	if (maus_info_blk) {
		if (maus_info_blk->button == 2) {
			switch_code = ESC;
		} else if (maus_info_blk->button == 1)
			switch_code = 255;
		else if (maus_info_blk->button == 4)
			switch_code = 254;
	}

	if (kb_info_blk)
		if (kb_info_blk->key_code != 0)
			switch_code = (int16)kb_info_blk->key_code;

	if (hot_key != 0) {
		switch_code = (int16)hot_key;
		hot_key = 0;
	}

	return switch_code;
}

} // namespace Chewy
