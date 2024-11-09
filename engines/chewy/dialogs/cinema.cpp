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
#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/font.h"
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/mcga_graphics.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Dialogs {

static constexpr int CINEMA_LINES = 12;

static const uint8 CINEMA_FLICS[35] = {
	FCUT_000, FCUT_002, FCUT_006, FCUT_009, FCUT_015,
	FCUT_012, FCUT_011, FCUT_SPACECHASE_18, FCUT_003, FCUT_048,
	FCUT_031, FCUT_044, FCUT_055, FCUT_058, FCUT_045,
	FCUT_065, FCUT_067, FCUT_068, FCUT_069, FCUT_080,
	FCUT_074, FCUT_083, FCUT_084, FCUT_088, FCUT_093,
	FCUT_087, FCUT_106, FCUT_108, FCUT_107, FCUT_113,
	FCUT_110, FCUT_121, FCUT_123, FCUT_122, FCUT_117
};

void Cinema::execute() {
	int topIndex = 0;
	int selected = 0;
	bool flag = true;
	int delay = 0;
	Common::Array<int> cutscenes;
	Common::String cutsceneName;

	getCutscenes(cutscenes);

	_G(fontMgr)->setFont(_G(font6));

	_G(room)->load_tgp(4, &_G(room_blk), 1, false, GBOOK);
	showCur();
	EVENTS_CLEAR;
	g_events->_kbInfo._scanCode = 0;

	for (bool endLoop = false; !endLoop;) {
		_G(out)->setPointer(_G(workptr));
		_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], 0, 0);

		if (!cutscenes.empty()) {
			// Render cut-scene list

			for (int i = 0; i < CINEMA_LINES; ++i) {
				if ((topIndex + i) >= (int)cutscenes.size())
					continue;

				cutsceneName = _G(atds)->getTextEntry(98,
					546 + cutscenes[topIndex + i] - 1, ATS_DATA);
				int yp = i * 10 + 68;

				if (i == selected)
					_G(out)->boxFill(37, yp, 308, yp + 10, 42);
				_G(out)->printxy(40, yp, 14, 300, 0, cutsceneName.c_str());
			}
		} else {
			// No cut-scene seen yet
			cutsceneName = _G(atds)->getTextEntry(98, 545, ATS_DATA);
			_G(out)->printxy(40, 68, 14, 300, _G(scr_width), cutsceneName.c_str());
		}

		if (_G(minfo).button == 1 && !flag) {
			flag = true;
			switch (_G(out)->findHotspot(_G(cinematicsHotspots))) {
			case 0:
				g_events->_kbInfo._scanCode = Common::KEYCODE_UP;
				break;

			case 1:
				g_events->_kbInfo._scanCode = Common::KEYCODE_DOWN;
				break;

			case 2: {
				int selLine = (g_events->_mousePos.y - 68) / 10;
				int selIndex = topIndex + selLine;
				if (selIndex < (int)cutscenes.size()) {
					selected = selLine;
					g_events->_kbInfo._scanCode = Common::KEYCODE_RETURN;
				}
				break;
			}

			default:
				break;
			}
		} else if (_G(minfo).button == 2 && !flag) {
			g_events->_kbInfo._scanCode = Common::KEYCODE_ESCAPE;
			flag = true;
		} else if (_G(minfo).button != 1) {
			delay = 0;
		} else if (flag) {
			g_events->update();
			if (--delay <= 0)
				flag = false;
		}

		switch (g_events->_kbInfo._scanCode) {
		case Common::KEYCODE_ESCAPE:
			endLoop = true;
			g_events->_kbInfo._scanCode = 0;
			break;

		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			if (selected > 0) {
				--selected;
			} else if (topIndex > 0) {
				--topIndex;
			}
			g_events->_kbInfo._scanCode = 0;
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
			g_events->_kbInfo._scanCode = 0;
			break;
		}

		case Common::KEYCODE_RETURN:
			hideCur();
			_G(out)->cls();
			_G(out)->setPointer((byte *)g_screen->getPixels());
			_G(fx)->blende1(_G(workptr), _G(pal), 0, 0);

			flic_cut(CINEMA_FLICS[cutscenes[topIndex + selected] - 1]);
			_G(fontMgr)->setFont(_G(font6));
			showCur();
			delay = 0;
			flag = false;
			break;

		default:
			break;
		}

		// The below are hacks to get the dialog to work in ScummVM
		g_events->_kbInfo._scanCode = 0;
		_G(minfo).button = 0;

		_G(cur)->updateCursor();

		if (flag) {
			flag = false;
			_G(out)->setPointer((byte *)g_screen->getPixels());
			_G(room)->set_ak_pal(&_G(room_blk));
			_G(fx)->blende1(_G(workptr), _G(pal), 0, 0);
		} else {
			_G(out)->copyToScreen();
		}

		g_events->update();
		SHOULD_QUIT_RETURN;
	}

	_G(room)->set_ak_pal(&_G(room_blk));
	hideCur();
	_G(uhr)->resetTimer(0, 5);
}

} // namespace Dialogs
} // namespace Chewy
