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

	_visible = false;
	_ani = nullptr;
	_curAniCountdown = 0;
	_aniCount = 0;
}

Cursor::~Cursor() {
}

void Cursor::plot_cur() {
	if (_visible) {
		if (_cursorMoveFl) {
			_cursorMoveFl = false;
			if (!_curblk->no_back) {

				_out->blockcopy(_curblk->cur_back, _cur_x_old, _cur_y_old, _scrWidth);

				_out->sprite_save(_curblk->cur_back, (_G(minfo).x + _curblk->page_off_x),
				                  (_G(minfo).y + _curblk->page_off_y), _curblk->xsize,
				                  _curblk->ysize, _scrWidth);
			}

			_cur_x_old = (_G(minfo).x + _curblk->page_off_x);
			_cur_y_old = (_G(minfo).y + _curblk->page_off_y);
		}

		--_curAniCountdown;
		if (_curAniCountdown <= 0 && _ani != nullptr) {
			_curAniCountdown = _ani->_delay;
			++_aniCount;
			if (_aniCount > _ani->_end)
				_aniCount = _ani->_start;
		}

		_out->spriteSet(_curblk->sprite[_aniCount], _cur_x_old, _cur_y_old, _scrWidth);
	}
}

void Cursor::show_cur() {
	if (!_visible) {
		_visible = true;

		_G(minfo).x = g_events->_mousePos.x;
		_G(minfo).y = g_events->_mousePos.y;

		if (!_curblk->no_back) {
			_out->sprite_save(_curblk->cur_back, (_G(minfo).x + _curblk->page_off_x),
			    (_G(minfo).y + _curblk->page_off_y), _curblk->xsize,
			    _curblk->ysize, _scrWidth);
		}

		_cur_x_old = (_G(minfo).x + _curblk->page_off_x);
		_cur_y_old = (_G(minfo).y + _curblk->page_off_y);
		_cursorMoveFl = true;
		plot_cur();
	}
}

void Cursor::hide_cur() {
	if (_visible) {
		if (!_curblk->no_back) {
			_out->blockcopy(_curblk->cur_back, _cur_x_old, _cur_y_old, _scrWidth);
		}
		_visible = false;
	}
}

void Cursor::set_cur_ani(CurAni *ani1) {
	_ani = ani1;
	_curAniCountdown = 0;
	_aniCount = _ani->_start;
}

void Cursor::move(int16 x, int16 y) {
	_G(minfo).x = x;
	_G(minfo).y = y;
	_cur_x_old = (_G(minfo).x + _curblk->page_off_x);
	_cur_y_old = (_G(minfo).y + _curblk->page_off_y);
	_in->setMousePos(x, y);
	if (_visible)
		_cursorMoveFl = true;
	else
		_cursorMoveFl = false;
}

} // namespace Chewy
