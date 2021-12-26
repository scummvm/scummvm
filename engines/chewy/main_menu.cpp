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
#include "chewy/main_menu.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/main.h"
#include "chewy/ngshext.h"

namespace Chewy {

int MainMenu::_selection;
int MainMenu::_personAni[3];

void MainMenu::execute() {
#ifdef TODO_REENABLE
	// TODO: Currently disabled so it doesn't keep playing on startup
	mem->file->select_pool_item(music_handle, EndOfPool - 17);
	mem->file->load_tmf(music_handle, (tmf_header *)Ci.MusicSlot);
	if (!modul)
		ailsnd->play_mod((tmf_header *)Ci.MusicSlot);

	flic_cut(200, 0);
	ailsnd->stop_mod();
#endif

	show_intro();

	cur->move(152, 92);
	minfo.x = 152;
	minfo.y = 92;
	_G(spieler).inv_cur = 0;
	menu_display = 0;
	_G(spieler).soundLoopMode = 1;

	bool done = false;
	while (!done && !SHOULD_QUIT) {
		ailsnd->stop_mod();
		ailsnd->end_sound();
		SetUpScreenFunc = screenFunc;

		cursor_wahl(20);
		_selection = -1;
		_G(spieler).scrollx = _G(spieler).scrolly = 0;
		_G(spieler).PersonRoomNr[0] = 98;
		room->load_room(&room_blk, 98, &_G(spieler));
		ERROR

		CurrentSong = -1;
		load_room_music(98);
		fx->border(workpage, 100, 0, 0);

		out->set_palette(pal);
		_G(spieler).PersonHide[0] = 1;
		show_cur();

		// Wait for a selection to be made on the main menu
		do {
			animate();
			if (SHOULD_QUIT)
				return;
		} while (_selection == -1);

		switch (_selection) {
		case MM_START_GAME:
			startGame();
			playGame();
			break;

		case MM_VIEW_INTRO:
			fx->border(workpage, 100, 0, 0);
			out->setze_zeiger(workptr);
			flags.NoPalAfterFlc = true;
			flic_cut(135, 0);
			break;

		case MM_LOAD_GAME:
			if (loadGame())
				playGame();
			break;

		case MM_CINEMA:
			cursor_wahl(4);
			cur->move(152, 92);
			minfo.x = 152;
			minfo.y = 92;
			cinema();
			break;

		case MM_QUIT:
			out->setze_zeiger(nullptr);
			out->cls();
			done = true;
			break;

		case MM_CREDITS:
			fx->border(workpage, 100, 0, 0);
			flags.NoPalAfterFlc = true;
			flc->set_custom_user_function(creditsFn);
			flic_cut(159, 0);
			flc->remove_custom_user_function();
			fx->border(workpage, 100, 0, 0);
			break;

		default:
			break;
		}
	}
}

void MainMenu::screenFunc() {
	int vec = det->maus_vector(minfo.x + _G(spieler).scrollx, minfo.y + _G(spieler).scrolly);

	if (in->get_switch_code() == 28 || minfo.button == 1) {
		_selection = vec;
	}
}

void MainMenu::animate() {
	if (ani_timer->TimeFlag) {
		uhr->reset_timer(0, 0);
		_G(spieler).DelaySpeed = FrameSpeed / _G(spieler).FramesPerSecond;
		spieler_vector->Delay = _G(spieler).DelaySpeed + spz_delay[0];
		FrameSpeed = 0;
		det->set_global_delay(_G(spieler).DelaySpeed);
	}

	++FrameSpeed;
	out->setze_zeiger(workptr);
	out->map_spr2screen(ablage[room_blk.AkAblage],
		_G(spieler).scrollx, _G(spieler).scrolly);

	if (SetUpScreenFunc && !menu_display && !flags.InventMenu) {
		SetUpScreenFunc();
		out->setze_zeiger(workptr);
	}

	sprite_engine();
	kb_mov(1);
	calc_maus_txt(minfo.x, minfo.y, 1);
	cur->plot_cur();
	_G(maus_links_click) = 0;
	menu_flag = 0;
	out->setze_zeiger(nullptr);
	out->back2screen(workpage);

	g_screen->update();
	g_events->update();
}

int16 MainMenu::creditsFn(int16 key) {
	if (key == 32 || key == 72 || key == 92 ||
			key == 128 || key == 165 || key == 185 ||
			key == 211 || key == 248 || key == 266) {
		for (int idx = 0; idx < 2000; ++idx) {
			if (in->get_switch_code() == 1)
				return -1;
			g_events->update();
		}
		return 0;

	} else {
		return in->get_switch_code() == 1 ? -1 : 0;
	}
}

void MainMenu::startGame() {
	mem->file->fcopy(ADSH_TMP, "txt/diah.adh");
	atds->open_handle(ADSH_TMP, "rb", 3);
	ERROR

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

	_G(spieler).PersonRoomNr[0] = 0;
	room->load_room(&room_blk, 0, &_G(spieler));
	ERROR

	spieler_vector[P_CHEWY].Phase = 6;
	spieler_vector[P_CHEWY].PhAnz = chewy_ph_anz[6];
	set_person_pos(160, 80, 0, 1);
	fx_blend = BLEND3;
	_G(spieler).PersonHide[P_CHEWY] = 0;
	menu_item = 0;
	cursor_wahl(0);
	enter_room(-1);
	_G(auto_obj) = 0;
}

bool MainMenu::loadGame() {
	flags.SaveMenu = true;
	savePersonAni();
	out->setze_zeiger(screen0);
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	cursor_wahl(4);
	cur->move(152, 92);
	minfo.x = 152;
	minfo.y = 92;
	savegameFlag = true;
	int result = file_menue();

	cursor_wahl((_G(spieler).inv_cur && _G(spieler).AkInvent != -1 &&
		menu_item == 1) ? 8 : 0);
	_G(cur_display) = true;
	restorePersonAni();
	flags.SaveMenu = false;

	if (result == 0) {
		fx_blend = BLEND1;
		return true;
	} else {
		return false;
	}
}

void MainMenu::playGame() {
	// unused1 = 0;
	inv_disp_ok = false;
	_G(cur_display) = true;
	tmp_menu_item = 0;
	_G(maus_links_click) = 0;
	kbinfo.scan_code = Common::KEYCODE_INVALID;

	flags.main_maus_flag = false;
	flags.MainInput = true;
	flags.ShowAtsInvTxt = true;
	cur->show_cur();
	spieler_vector->Count = 0;
	uhr->reset_timer(0, 0);
	ailsnd->set_loopmode(_G(spieler).soundLoopMode);

	while (!SHOULD_QUIT && !main_loop(1)) {
	}

	_G(auto_obj) = 0;
}

void MainMenu::savePersonAni() {
	for (int i = 0; i < MAX_PERSON; ++i) {
		_personAni[i] = PersonAni[i];
		PersonAni[i] = -1;

		delete PersonTaf[i];
		PersonTaf[i] = nullptr;
	}
}

void MainMenu::restorePersonAni() {
	for (int i = 0; i < MAX_PERSON; ++i) {
		load_person_ani(_personAni[i], i);
	}
}

void MainMenu::cinema() {
	warning("TODO: cinema dialog");
}

} // namespace Chewy
