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

#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/globals.h"

namespace Chewy {

Cursor::Cursor(McgaGraphics *iout, InputMgr *iin, CurBlk *curblkp) {
	_out = iout;
	_in = iin;
	_scrWidth = _G(scr_w);
	_curblk = curblkp;
	_inzeig = _in->get_in_zeiger();

	if (!_inzeig->minfo) {
		_maus_da = false;
	} else {
		_maus_da = true;
		_minfo = _inzeig->minfo;
		_sichtbar = false;
		_ani = nullptr;
		_cur_aufruf = false;
		_ani_count = false;
	}
}

Cursor::~Cursor() {
}

void Cursor::plot_cur() {
	if (_maus_da && _sichtbar) {
		if (cur_move) {
			mouse_active = true;
			cur_move = false;
			if (!_curblk->no_back) {

				_out->blockcopy(_curblk->cur_back, _cur_x_old, _cur_y_old, _scrWidth);

				_out->sprite_save(_curblk->cur_back, (_minfo->x + _curblk->page_off_x),
				                  (_minfo->y + _curblk->page_off_y), _curblk->xsize,
				                  _curblk->ysize, _scrWidth);
			}

			_cur_x_old = (_minfo->x + _curblk->page_off_x);
			_cur_y_old = (_minfo->y + _curblk->page_off_y);
		}

		_cur_aufruf -= 1;
		if ((_cur_aufruf <= 0) && (_ani != nullptr)) {
			_cur_aufruf = _ani->delay;
			++_ani_count;
			if (_ani_count > _ani->ani_end)
				_ani_count = _ani->ani_anf;
		}

		_out->sprite_set(_curblk->sprite[_ani_count], _cur_x_old, _cur_y_old,
		                 _scrWidth);
		mouse_active = false;
	}
}

void Cursor::show_cur() {
	if ((_maus_da) && (!_sichtbar)) {
		_sichtbar = true;
		mouse_active = true;

		_minfo->x = g_events->_mousePos.x;
		_minfo->y = g_events->_mousePos.y;

		if (!_curblk->no_back) {
			_out->sprite_save(_curblk->cur_back, (_minfo->x + _curblk->page_off_x),
			    (_minfo->y + _curblk->page_off_y), _curblk->xsize,
			    _curblk->ysize, _scrWidth);
		}

		_cur_x_old = (_minfo->x + _curblk->page_off_x);
		_cur_y_old = (_minfo->y + _curblk->page_off_y);
		cur_move = true;
		plot_cur();
	}
}

void Cursor::hide_cur() {
	if ((_maus_da) && (_sichtbar)) {
		if (!_curblk->no_back) {
			_out->blockcopy(_curblk->cur_back, _cur_x_old, _cur_y_old, _scrWidth);
		}
		_sichtbar = false;
	}
}

void Cursor::set_cur_ani(CurAni *ani1) {
	_ani = ani1;
	_cur_aufruf = 0;
	_ani_count = _ani->ani_anf;
}

void Cursor::move(int16 x, int16 y) {
	if (_maus_da) {
		mouse_active = true;

		_minfo->x = x;
		_minfo->y = y;
		_cur_x_old = (_minfo->x + _curblk->page_off_x);
		_cur_y_old = (_minfo->y + _curblk->page_off_y);
		_in->move_mouse(x, y);
		if (_sichtbar)
			cur_move = true;
		else
			cur_move = false;
		mouse_active = false;
	}
}

void Cursor::wait_taste_los(bool maus_plot) {
	int16 is_mouse = 0;
	if (_maus_da) {
		g_events->update();
		is_mouse = _minfo->button;
	}

	if (!is_mouse)
		_in->_hotkey = 0;

	int16 stay = 1;
	int16 switch_code = 1;

	while ((switch_code != 0) && (stay)) {
		switch_code = _in->get_switch_code();
		if (is_mouse) {
			switch_code = 2;

			g_events->update();
			stay = _minfo->button;
		}

		if (maus_plot)
			plot_cur();
	}
}

} // namespace Chewy
