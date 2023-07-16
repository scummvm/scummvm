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

#include "m4/adv_r/conv.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/gui/gui_univ.h"
#include "m4/gui/gui_vmng.h"
#include "m4/vars.h"

namespace M4 {

void set_dlg_rect() {
	int32 sizex = 0, sizey = 0;
	int32 screen_x_center = 0, screen_y_center = 0;
	int32 screen_x_size = 0, screen_y_size = 0;
	int32 status;

	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
	if (!game_buff_ptr)
		error_show(FL, 'BUF!');

	screen_x_center = VIDEO_W / 2;
	screen_y_center = (game_buff_ptr->y2 - game_buff_ptr->y1) / 2;
	screen_x_size = VIDEO_W;
	screen_y_size = (game_buff_ptr->y2 - game_buff_ptr->y1);

	_GC(height) = gr_font_get_height() + _GC(conv_font_spacing_v);
	_GC(width) += 2 * _GC(conv_font_spacing_h);

	sizex = _GC(width);
	sizey = _G(cdd).num_txt_ents * (_GC(height))+_GC(conv_font_spacing_v);

	switch (_GC(glob_x)) {
	case DLG_CENTER_H:
		_GC(r_x1) = screen_x_center - (sizex / 2);
		break;

	case DLG_FLUSH_LEFT:
		_GC(r_x1) = 0;
		break;

	case DLG_FLUSH_RIGHT:
		_GC(r_x1) = screen_x_size - sizex;
		break;

	default:
		_GC(r_x1) = _GC(glob_x);
		_GC(r_x1) += game_buff_ptr->x1;
		break;
	}

	switch (_GC(glob_y)) {
	case DLG_CENTER_V:
		_GC(r_y1) = screen_y_center - (sizey / 2);
		break;

	case DLG_FLUSH_TOP:
		_GC(r_y1) = 0;
		break;

	case DLG_FLUSH_BOTTOM:
		_GC(r_y1) = screen_y_size - sizey + game_buff_ptr->y1 - 10;
		break;

	default:
		_GC(r_y1) = _GC(glob_y);
		_GC(r_y1) += game_buff_ptr->y1;
		break;
	}

	if (_GC(r_x1) < 0)
		_GC(r_x1) = 0;

	if (_GC(r_y1) < 0)
		_GC(r_y1) = 0;

	//fprintf( conv_fp, "_GC(r_x1) %d sizex %d\n", _GC(r_x1), sizex );
	_GC(r_y2) = _GC(r_y1) + sizey - 1;
	_GC(r_x2) = _GC(r_x1) + sizex - 1;

	_GC(r_x2) = imath_min(VIDEO_W, _GC(r_x2));
	_GC(r_y2) = imath_min(VIDEO_H, _GC(r_y2));
}

void conv_go(Conv *c) {
	error("TODO: conv_go");
}

} // End of namespace M4
