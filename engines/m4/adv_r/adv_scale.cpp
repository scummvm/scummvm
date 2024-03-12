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

#include "m4/adv_r/adv_scale.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_pal.h"
#include "m4/graphics/gr_line.h"
#include "m4/gui/gui_vmng.h"
#include "m4/vars.h"

namespace M4 {

#define LABEL_OFFSET 8
#define _GS(X) _G(scale)._##X

static void scale_editor_clear_rects() {
	_GS(old_front) = _GS(old_back) = -1;
	_GS(myback) = _GS(myfront) = _GS(mybs) = _GS(myfs) = -1;
}

static void scale_editor_init() {
	scale_editor_clear_rects();
}

static void scale_editor_undraw() {
	Buffer *scr_orig = _G(game_bgBuff)->get_buffer();
	Buffer *game_buff = _G(gameDrawBuff)->get_buffer();
	int32 status;
	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);

	if (_GS(old_back) != -1) {
		gr_buffer_rect_copy_2(scr_orig, game_buff, 0, _GS(old_back) - LABEL_OFFSET, 0,
			_GS(old_back) - LABEL_OFFSET, scr_orig->w, LABEL_OFFSET + 1);
		RestoreScreensInContext(0, _GS(old_back) - LABEL_OFFSET, scr_orig->w, _GS(old_back) + 1, game_buff_ptr);
		_GS(old_back) = -1;
	}
	if (_GS(old_front) != -1) {
		gr_buffer_rect_copy_2(scr_orig, game_buff, 0, _GS(old_front) - LABEL_OFFSET, 0,
			_GS(old_front) - LABEL_OFFSET, scr_orig->w, LABEL_OFFSET + 1);
		RestoreScreensInContext(0, _GS(old_front) - LABEL_OFFSET, scr_orig->w, _GS(old_front) + 1, game_buff_ptr);
		_GS(old_front) = -1;
	}

	_G(game_bgBuff)->release();
	_G(gameDrawBuff)->release();
	scale_editor_clear_rects();
}

void scale_editor_draw() {
	bool bail = true;

	// Have we drawn before?
	if (_GS(myback) == -1 && _GS(myfront) == -1)
		bail = false;

	// Has back or front y changed?
	if (bail && _GS(myback) != _G(currentSceneDef).back_y && _GS(myfront) != _G(currentSceneDef).front_y)
		bail = false;

	// Has back or front scale changed?
	if (bail && _GS(mybs) != _G(currentSceneDef).back_scale && _GS(myfs) != _G(currentSceneDef).front_scale)
		bail = false;

	// No changes, don't draw.
	if (bail)
		return;

	scale_editor_undraw();

	Buffer *game_buff = _G(gameDrawBuff)->get_buffer();
	int32 status;
	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);

	Buffer *scr_orig = _G(game_bgBuff)->get_buffer();

	gr_color_set(__YELLOW);
	_GS(myback) = _GS(old_back) = _G(currentSceneDef).back_y;
	_GS(myfront) = _GS(old_front) = _G(currentSceneDef).front_y;
	_GS(mybs) = _G(currentSceneDef).back_scale;
	_GS(myfs) = _G(currentSceneDef).front_scale;
	gr_hline(game_buff, 0, scr_orig->w, _GS(old_back));
	gr_hline(game_buff, 0, scr_orig->w, _GS(old_front));

	char string[20];
	gr_font_set_color(__WHITE);
	gr_font_set(_G(font_tiny));

	Common::sprintf_s(string, "Front: %d, %d", _GS(old_front), _G(currentSceneDef).front_scale);
	int x;
	for (x = 10; x < scr_orig->w - 220; x += 400)
		gr_font_write(game_buff, string, x, _GS(old_front) - LABEL_OFFSET, 0, 0);

	Common::sprintf_s(string, "Back: %d, %d", _GS(old_back), _G(currentSceneDef).back_scale);
	for (x = 110; x < scr_orig->w - 320; x += 400)
		gr_font_write(game_buff, string, x, _GS(old_back) - LABEL_OFFSET, 0, 0);

	RestoreScreensInContext(0, _GS(old_back) - LABEL_OFFSET, scr_orig->w, _GS(old_back) + 1, game_buff_ptr);
	RestoreScreensInContext(0, _GS(old_front) - LABEL_OFFSET, scr_orig->w, _GS(old_front) + 1, game_buff_ptr);

	_G(game_bgBuff)->release();
	_G(gameDrawBuff)->release();
}

void scale_editor_cancel() {
	scale_editor_undraw();
	_G(editors_in_use) &= ~kScaleEditor;
}

void scale_editor_toggle() {
	if (_G(editors_in_use) & kScaleEditor)
		scale_editor_cancel();
	else {
		_G(editors_in_use) |= kScaleEditor;
		scale_editor_init();
	}
}

} // End of namespace M4
