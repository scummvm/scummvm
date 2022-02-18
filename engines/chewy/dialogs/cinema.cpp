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

#include "chewy/dialogs/cinema.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/globals.h"

namespace Chewy {
namespace Dialogs {

static constexpr int CINEMA_LINES = 12;

static const int16 CINEMA_TBL[4 * 3] = {
	10,  80,  32, 105,
	10, 150,  32, 175,
	36,  64, 310, 188
};

static const uint8 CINEMA_FLICS[35] = {
	0, 2, 6, 9, 15, 12, 11, 18, 3, 48, 31, 44,
	55, 58, 45, 65, 67, 68, 69, 80, 74, 83, 84, 88,
	93, 87, 106, 108, 107, 113, 110, 121, 123, 122, 117
};

void Cinema::execute() {
	int timer_nr = 0;
	int16 txt_anz = 0;
	int topIndex = 0;
	int selected = -1;
	bool flag = false;
	int delay = 0;
	Common::Array<int> cutscenes;
	getCutscenes(cutscenes);

	_G(out)->set_fontadr(_G(font6x8));
	_G(out)->set_vorschub(_G(fvorx6x8), _G(fvory6x8));
	_G(atds)->load_atds(98, 1);

	_G(room)->open_handle(GBOOK, 0);
	_G(room)->load_tgp(4, &_G(room_blk), 1, 0, GBOOK);
	show_cur();
	EVENTS_CLEAR;
	_G(kbinfo).scan_code = 0;

	for (bool endLoop = false; !endLoop;) {
		timer_nr = 0;
		_G(out)->setze_zeiger(_G(workptr));
		_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], 0, 0);

		if (!cutscenes.empty()) {
			// Render cutscene list
			for (int i = 0; i < CINEMA_LINES; ++i) {
				char *csName = _G(atds)->ats_get_txt(546 + i + topIndex,
					0, &txt_anz, 1);
				int yp = i * 10 + 68;

				if (i == selected)
					_G(out)->box_fill(37, yp, 308, yp + 10, 42);
				_G(out)->printxy(40, yp, 14, 300, 0, "%s", csName);
			}
		} else {
			// No cutscenes seen yet
			char *none = _G(atds)->ats_get_txt(545, 0, &txt_anz, 1);
			_G(out)->printxy(40, 68, 14, 300, _G(scr_width), none);
		}

		if (_G(minfo).button == 1 && !flag) {
			flag = true;
			switch (_G(in)->maus_vector(_G(minfo).x, _G(minfo).y, CINEMA_TBL, 3)) {
			case 0:
				_G(kbinfo).scan_code = Common::KEYCODE_UP;
				if (!endLoop) {
					endLoop = true;
					timer_nr = 5;
				}
				break;

			case 1:
				_G(kbinfo).scan_code = Common::KEYCODE_DOWN;
				if (!endLoop) {
					endLoop = true;
					timer_nr = 5;
				}
				break;

			case 2:
			{
				int selIndex = (_G(minfo).y - 68) / 10 + topIndex;
				if (selIndex < (int)cutscenes.size())
					selected = selIndex;
				_G(kbinfo).scan_code = Common::KEYCODE_RETURN;
				break;
			}

			default:
				break;
			}
		} else if (_G(minfo).button == 2 && !flag) {
			_G(kbinfo).scan_code = Common::KEYCODE_ESCAPE;
			flag = true;
		} else if (_G(minfo).button != 1) {
			flag = false;
			timer_nr = 0;
			delay = 0;
		} else if (flag) {
			EVENTS_UPDATE;
			if (--delay <= 0)
				flag = false;
		}

		switch (_G(kbinfo).scan_code) {
		case Common::KEYCODE_ESCAPE:
			endLoop = true;
			_G(kbinfo).scan_code = 0;
			break;

		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			if (selected > 0) {
				--selected;
			} else if (topIndex > 0) {
				--topIndex;
			}
			_G(kbinfo).scan_code = 0;
			break;

		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2:
		{
			int newIndex = selected + 1;
			if (selected >= 11) {
				if ((topIndex + newIndex) < (int)cutscenes.size())
					++topIndex;
			} else {
				if ((topIndex + newIndex) < (int)cutscenes.size())
					++selected;
			}
			_G(kbinfo).scan_code = 0;
			break;
		}

		case Common::KEYCODE_RETURN:
			hide_cur();
			_G(out)->cls();
			_G(out)->setze_zeiger(_G(screen0));
			_G(fx)->blende1(_G(workptr), _G(screen0), _G(pal), 150, 0, 0);
			print_rows(546 + topIndex);

			_G(flc)->set_custom_user_function(cut_serv);
			flic_cut(CINEMA_FLICS[topIndex + selected], CFO_MODE);
			_G(flc)->remove_custom_user_function();
			_G(out)->set_fontadr(_G(font6x8));
			_G(out)->set_vorschub(_G(fvorx6x8), _G(fvory6x8));
			show_cur();
			delay = 0;
			flag = false;
			break;

		default:
			break;
		}

		// The below are hacks to get the dialog to work in ScummVM
		_G(kbinfo).scan_code = 0;
		_G(minfo).button = 0;
		txt_anz = 0;

		if (!txt_anz) {
			_G(cur)->plot_cur();

			if (flag) {
				flag = false;
				_G(out)->setze_zeiger(_G(screen0));
				_G(room)->set_ak_pal(&_G(room_blk));
				_G(fx)->blende1(_G(workptr), _G(screen0), _G(pal), 150, 0, 0);
			} else {
				_G(out)->back2screen(_G(workpage));
			}
		}

		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;
	}

	_G(room)->open_handle(EPISODE1, 0);
	_G(room)->set_ak_pal(&_G(room_blk));
	hide_cur();
	_G(uhr)->reset_timer(0, 5);
}

int16 Cinema::cut_serv(int16 frame) {
	if (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE) {
		_G(sndPlayer)->stopMod();
		_G(sndPlayer)->endSound();
		return -1;

	} else {
		return 0;
	}
}

} // namespace Dialogs
} // namespace Chewy
