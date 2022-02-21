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
bool mouse_links_los;
bool mouse_active;
// end of externals

bool cur_move;


void set_new_kb_handler(KbdInfo *key) {
	g_events->setKbdInfo(key);
}

void set_old_kb_handler() {
	g_events->setKbdInfo(nullptr);
}

InputMgr::InputMgr() {
	_mouseInfoBlk = nullptr;
	_kbInfoBlk = nullptr;
}

InputMgr::~InputMgr() {
}

int InputMgr::init() {
	// ScummVM supports three buttons
	return 3;
}

void InputMgr::move_mouse(int16 x, int16 y) {
	g_events->warpMouse(Common::Point(x, y));
}

void InputMgr::rectangle(int16 xmin, int16 ymin, int16 xmax, int16 ymax) {
	// Mouse clip rectangle isn't supported in ScummVM
}

int16 InputMgr::maus_vector(int16 x, int16 y, const int16 *tbl, int16 anz) {
	int16 i = -1;
	for (int16 j = 0; (j < anz * 4) && (i == -1); j += 4) {
		if ((x >= tbl[j]) && (x <= tbl[j + 2]) && (y >= tbl[j + 1]) && (y <= tbl[j + 3]))
			i = j / 4;
	}

	return i;
}

void InputMgr::neuer_kb_handler(KbdInfo *key) {
	set_new_kb_handler(key);
	_kbInfoBlk = key;
	_kbInfoBlk->key_code = '\0';
}

void InputMgr::alter_kb_handler() {
	set_old_kb_handler();
	_kbInfoBlk = nullptr;
	warning("STUB - InputMgr::alter_kb_handler");
#if 0
	while (kbhit())
		getch();
#endif
}

void InputMgr::neuer_maushandler(MouseInfo *mpos) {
	_mouseInfoBlk = mpos;
}

KbdMouseInfo *InputMgr::get_in_zeiger() {
	_inzeig.minfo = _mouseInfoBlk;
	_inzeig.kbinfo = _kbInfoBlk;

	return &_inzeig;
}

int16 InputMgr::get_switch_code() {
	int16 switch_code = 0;

	if (_mouseInfoBlk) {
		if (_mouseInfoBlk->button == 2) {
			switch_code = Common::KEYCODE_ESCAPE;
		} else if (_mouseInfoBlk->button == 1)
			switch_code = 255;
		else if (_mouseInfoBlk->button == 4)
			switch_code = 254;
	}

	if (_kbInfoBlk)
		if (_kbInfoBlk->key_code != 0)
			switch_code = (int16)_kbInfoBlk->key_code;

	if (_hotkey != 0) {
		switch_code = (int16)_hotkey;
		_hotkey = 0;
	}

	return switch_code;
}

} // namespace Chewy
