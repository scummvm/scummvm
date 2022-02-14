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

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "chewy/dialogs/main_menu.h"
#include "chewy/dialogs/cinema.h"
#include "chewy/dialogs/credits.h"
#include "chewy/dialogs/files.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/ngshext.h"

namespace Chewy {
namespace Dialogs {

int MainMenu::_selection;
int MainMenu::_personAni[3];

void MainMenu::execute() {
	// Convenience during testing to not keep showing title sequence
	if (!ConfMan.getBool("skip_title")) {
		_G(mem)->file->select_pool_item(_G(music_handle), _G(EndOfPool) - 17);
		_G(mem)->file->load_tmf(_G(music_handle), (TmfHeader *)_G(Ci).MusicSlot);
		if (!_G(modul))
			_G(sndPlayer)->playMod((TmfHeader *)_G(Ci).MusicSlot);

		flic_cut(200, 0);
		_G(sndPlayer)->stopMod();
	}

	show_intro();

	_G(cur)->move(152, 92);
	_G(minfo).x = 152;
	_G(minfo).y = 92;
	_G(spieler).inv_cur = false;
	_G(menu_display) = 0;
	_G(spieler).soundLoopMode = 1;

	bool done = false;
	while (!done && !SHOULD_QUIT) {
		_G(sndPlayer)->stopMod();
		_G(sndPlayer)->endSound();
		_G(SetUpScreenFunc) = screenFunc;

		cursor_wahl(CUR_ZEIGE);
		_selection = -1;
		_G(spieler).scrollx = _G(spieler).scrolly = 0;
		_G(spieler).PersonRoomNr[P_CHEWY] = 98;
		_G(room)->load_room(&_G(room_blk), 98, &_G(spieler));

		_G(currentSong) = -1;
		load_room_music(98);
		_G(fx)->border(_G(workpage), 100, 0, 0);

		_G(out)->set_palette(_G(pal));
		_G(spieler).PersonHide[P_CHEWY] = true;
		show_cur();

		// Wait for a selection to be made on the main menu
		do {
			animate();
			if (SHOULD_QUIT)
				return;
		} while (_selection == -1);

		switch (_selection) {
		case MM_START_GAME:
			EVENTS_CLEAR;
			startGame();
			playGame();
			break;

		case MM_VIEW_INTRO:
			_G(fx)->border(_G(workpage), 100, 0, 0);
			_G(out)->setze_zeiger(_G(workptr));
			_G(flags).NoPalAfterFlc = true;
			flic_cut(135, CFO_MODE);
			break;

		case MM_LOAD_GAME:
			if (loadGame())
				playGame();
			break;

		case MM_CINEMA:
			cursor_wahl(CUR_SAVE);
			_G(cur)->move(152, 92);
			_G(minfo).x = 152;
			_G(minfo).y = 92;
			Dialogs::Cinema::execute();
			break;

		case MM_QUIT:
			_G(out)->setze_zeiger(nullptr);
			_G(out)->cls();
			done = true;
			break;

		case MM_CREDITS:
			_G(fx)->border(_G(workpage), 100, 0, 0);
			_G(flags).NoPalAfterFlc = true;
			_G(flc)->set_custom_user_function(creditsFn);
			flic_cut(159, CFO_MODE);
			_G(flc)->remove_custom_user_function();
			_G(fx)->border(_G(workpage), 100, 0, 0);
			Dialogs::Credits::execute();
			break;

		default:
			break;
		}
	}
}

void MainMenu::screenFunc() {
	int vec = _G(det)->maus_vector(_G(minfo).x + _G(spieler).scrollx, _G(minfo).y + _G(spieler).scrolly);

	if (_G(in)->get_switch_code() == 28 || _G(minfo).button == 1) {
		_selection = vec;
	}
}

void MainMenu::animate() {
	if (_G(ani_timer)->TimeFlag) {
		_G(uhr)->reset_timer(0, 0);
		_G(spieler).DelaySpeed = _G(FrameSpeed) / _G(spieler).FramesPerSecond;
		_G(spieler_vector)->Delay = _G(spieler).DelaySpeed + _G(spz_delay)[0];
		_G(FrameSpeed) = 0;
		_G(det)->set_global_delay(_G(spieler).DelaySpeed);
	}

	++_G(FrameSpeed);
	_G(out)->setze_zeiger(_G(workptr));
	_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage],
		_G(spieler).scrollx, _G(spieler).scrolly);

	if (_G(SetUpScreenFunc) && !_G(menu_display) && !_G(flags).InventMenu) {
		_G(SetUpScreenFunc)();
		_G(out)->setze_zeiger(_G(workptr));
	}

	sprite_engine();
	kb_mov(1);
	calc_maus_txt(_G(minfo).x, _G(minfo).y, 1);
	_G(cur)->plot_cur();
	_G(maus_links_click) = false;
	_G(menu_flag) = 0;
	_G(out)->setze_zeiger(nullptr);
	_G(out)->back2screen(_G(workpage));

	g_screen->update();
	g_events->update();
}

int16 MainMenu::creditsFn(int16 key) {
	if (key == 32 || key == 72 || key == 92 ||
		key == 128 || key == 165 || key == 185 ||
		key == 211 || key == 248 || key == 266) {
		for (int idx = 0; idx < 2000; ++idx) {
			if (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE)
				return -1;
			g_events->update();
		}
		return 0;

	} else {
		return _G(in)->get_switch_code() == Common::KEYCODE_ESCAPE ? -1 : 0;
	}
}

void MainMenu::startGame() {
	hide_cur();
	animate();
	exit_room(-1);

	bool soundSwitch = _G(spieler).SoundSwitch;
	uint8 soundVol = _G(spieler).SoundVol;
	bool musicSwitch = _G(spieler).MusicSwitch;
	uint8 musicVol = _G(spieler).MusicVol;
	bool speechSwitch = _G(spieler).SpeechSwitch;
	uint8 framesPerSecond = _G(spieler).FramesPerSecond;
	bool displayText = _G(spieler).DisplayText;
	int sndLoopMode = _G(spieler).soundLoopMode;

	var_init();

	_G(spieler).SoundSwitch = soundSwitch;
	_G(spieler).SoundVol = soundVol;
	_G(spieler).MusicSwitch = musicSwitch;
	_G(spieler).MusicVol = musicVol;
	_G(spieler).SpeechSwitch = speechSwitch;
	_G(spieler).FramesPerSecond = framesPerSecond;
	_G(spieler).DisplayText = displayText;
	_G(spieler).soundLoopMode = sndLoopMode;

	_G(spieler).PersonRoomNr[P_CHEWY] = 0;
	_G(room)->load_room(&_G(room_blk), 0, &_G(spieler));

	_G(spieler_vector)[P_CHEWY].Phase = 6;
	_G(spieler_vector)[P_CHEWY].PhAnz = _G(chewy_ph_anz)[6];
	set_person_pos(160, 80, P_CHEWY, P_RIGHT);
	_G(fx_blend) = BLEND3;
	_G(spieler).PersonHide[P_CHEWY] = false;
	_G(menu_item) = CUR_WALK;
	cursor_wahl(CUR_WALK);
	enter_room(-1);
	_G(auto_obj) = 0;
}

bool MainMenu::loadGame() {
	_G(flags).SaveMenu = true;
	savePersonAni();
	_G(out)->setze_zeiger(_G(screen0));
	_G(out)->set_fontadr(_G(font6x8));
	_G(out)->set_vorschub(_G(fvorx6x8), _G(fvory6x8));
	cursor_wahl(CUR_SAVE);
	_G(cur)->move(152, 92);
	_G(minfo).x = 152;
	_G(minfo).y = 92;
	_G(savegameFlag) = true;
	int result = Dialogs::Files::execute(false);

	cursor_wahl((_G(spieler).inv_cur && _G(spieler).AkInvent != -1 &&
		_G(menu_item) == CUR_USE) ? 8 : 0);
	_G(cur_display) = true;
	restorePersonAni();
	_G(flags).SaveMenu = false;

	if (result == 0) {
		_G(fx_blend) = BLEND1;
		return true;
	} else {
		return false;
	}
}

void MainMenu::playGame() {
	// unused1 = 0;
	_G(inv_disp_ok) = false;
	_G(cur_display) = true;
	_G(tmp_menu_item) = 0;
	_G(maus_links_click) = false;
	_G(kbinfo).scan_code = Common::KEYCODE_INVALID;

	_G(flags).main_maus_flag = false;
	_G(flags).MainInput = true;
	_G(flags).ShowAtsInvTxt = true;
	_G(cur)->show_cur();
	_G(spieler_vector)[P_CHEWY].Count = 0;
	_G(uhr)->reset_timer(0, 0);
	_G(sndPlayer)->setLoopMode(_G(spieler).soundLoopMode);

	while (!SHOULD_QUIT && !main_loop(1)) {
	}

	_G(auto_obj) = 0;
}

void MainMenu::savePersonAni() {
	for (int i = 0; i < MAX_PERSON; ++i) {
		_personAni[i] = _G(PersonAni)[i];
		_G(PersonAni)[i] = -1;

		delete _G(PersonTaf)[i];
		_G(PersonTaf)[i] = nullptr;
	}
}

void MainMenu::restorePersonAni() {
	for (int i = 0; i < MAX_PERSON; ++i) {
		load_person_ani(_personAni[i], i);
	}
}

} // namespace Dialogs
} // namespace Chewy
