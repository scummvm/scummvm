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

int MainMenu::_selection;
int MainMenu::_personAni[3];

void MainMenu::execute() {
#ifdef TODO_REENABLE
	// TODO: Currently disabled so it doesn't keep playing on startup
	mem->file->select_pool_item(music_handle, EndOfPool - 17);
	mem->file->load_tmf(music_handle, (tmf_header *)Ci.MusicSlot);
	if (!modul)
		ailsnd->playMod((tmf_header *)Ci.MusicSlot);

	flic_cut(200, 0);
	ailsnd->stopMod();
#endif

	show_intro();

	cur->move(152, 92);
	minfo.x = 152;
	minfo.y = 92;
	_G(spieler).inv_cur = false;
	menu_display = 0;
	_G(spieler).soundLoopMode = 1;

	bool done = false;
	while (!done && !SHOULD_QUIT) {
		ailsnd->stopMod();
		ailsnd->endSound();
		SetUpScreenFunc = screenFunc;

		cursor_wahl(CUR_ZEIGE);
		_selection = -1;
		_G(spieler).scrollx = _G(spieler).scrolly = 0;
		_G(spieler).PersonRoomNr[P_CHEWY] = 98;
		room->load_room(&room_blk, 98, &_G(spieler));

		CurrentSong = -1;
		load_room_music(98);
		fx->border(workpage, 100, 0, 0);

		out->set_palette(pal);
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
			g_events->clearEvents();
			startGame();
			playGame();
			break;

		case MM_VIEW_INTRO:
			fx->border(workpage, 100, 0, 0);
			out->setze_zeiger(workptr);
			flags.NoPalAfterFlc = true;
			flic_cut(135, CFO_MODE);
			break;

		case MM_LOAD_GAME:
			if (loadGame())
				playGame();
			break;

		case MM_CINEMA:
			cursor_wahl(CUR_SAVE);
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
			flic_cut(159, CFO_MODE);
			flc->remove_custom_user_function();
			fx->border(workpage, 100, 0, 0);
			gbook();
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
	_G(maus_links_click) = false;
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
			if (in->get_switch_code() == ESC)
				return -1;
			g_events->update();
		}
		return 0;

	} else {
		return in->get_switch_code() == ESC ? -1 : 0;
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
	room->load_room(&room_blk, 0, &_G(spieler));

	spieler_vector[P_CHEWY].Phase = 6;
	spieler_vector[P_CHEWY].PhAnz = chewy_ph_anz[6];
	set_person_pos(160, 80, P_CHEWY, P_RIGHT);
	fx_blend = BLEND3;
	_G(spieler).PersonHide[P_CHEWY] = false;
	menu_item = CUR_WALK;
	cursor_wahl(CUR_WALK);
	enter_room(-1);
	_G(auto_obj) = 0;
}

bool MainMenu::loadGame() {
	flags.SaveMenu = true;
	savePersonAni();
	out->setze_zeiger(screen0);
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	cursor_wahl(CUR_SAVE);
	cur->move(152, 92);
	minfo.x = 152;
	minfo.y = 92;
	savegameFlag = true;
	int result = file_menue();

	cursor_wahl((_G(spieler).inv_cur && _G(spieler).AkInvent != -1 &&
		menu_item == CUR_USE) ? 8 : 0);
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
	_G(maus_links_click) = false;
	kbinfo.scan_code = Common::KEYCODE_INVALID;

	flags.main_maus_flag = false;
	flags.MainInput = true;
	flags.ShowAtsInvTxt = true;
	cur->show_cur();
	spieler_vector[P_CHEWY].Count = 0;
	uhr->reset_timer(0, 0);
	ailsnd->setLoopMode(_G(spieler).soundLoopMode);

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
	int timer_nr = 0;
	int16 txt_anz = 0;
	int topIndex = 0;
	int selected = -1;
	bool flag = false;
	int delay = 0;
	Common::Array<int> cutscenes;
	getCutscenes(cutscenes);

	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	atds->load_atds(98, 1);
	
	room->open_handle("BACK/GBOOK.TGP", "rb", 0);
	room->load_tgp(4, &room_blk, 1, 0, "BACK/GBOOK.TGP");
	show_cur();
	g_events->clearEvents();
	kbinfo.scan_code = 0;

	for (bool endLoop = false; !endLoop;) {
		timer_nr = 0;
		out->setze_zeiger(workptr);
		out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);

		if (!cutscenes.empty()) {
			// Render cutscene list
			for (int i = 0; i < CINEMA_LINES; ++i) {
				char *csName = atds->ats_get_txt(546 + i + topIndex,
					0, &txt_anz, 1);
				int yp = i * 10 + 68;

				if (i == selected)
					out->box_fill(37, yp, 308, yp + 10, 42);
				out->printxy(40, yp, 14, 300, 0, "%s", csName);
			}
		} else {
			// No cutscenes seen yet
			char *none = atds->ats_get_txt(545, 0, &txt_anz, 1);
			out->printxy(40, 68, 14, 300, scr_width, none);
		}

		if (minfo.button == 1 && !flag) {
			flag = true;
			switch (in->maus_vector(minfo.x, minfo.y, CINEMA_TBL, 3)) {
			case 0:
				kbinfo.scan_code = Common::KEYCODE_UP;
				if (!endLoop) {
					endLoop = true;
					timer_nr = 5;
				}
				break;

			case 1:
				kbinfo.scan_code = Common::KEYCODE_DOWN;
				if (!endLoop) {
					endLoop = true;
					timer_nr = 5;
				}
				break;

			case 2: {
				int selIndex = (minfo.y - 68) / 10 + topIndex;
				if (selIndex < (int)cutscenes.size())
					selected = selIndex;
				kbinfo.scan_code = Common::KEYCODE_RETURN;
				break;
			}

			default:
				break;
			}
		} else if (minfo.button == 2 && !flag) {
			kbinfo.scan_code = ESC;
			flag = true;
		} else if (minfo.button != 1) {
			flag = false;
			timer_nr = 0;
			delay = 0;
		} else if (flag) {
			EVENTS_UPDATE;
			if (--delay <= 0)
				flag = false;
		}

		switch (kbinfo.scan_code) {
		case ESC:
			endLoop = true;
			kbinfo.scan_code = 0;
			break;

		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			if (selected > 0) {
				--selected;
			} else if (topIndex > 0) {
				--topIndex;
			}
			kbinfo.scan_code = 0;
			break;

		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2: {
			int newIndex = selected + 1;
			if (selected >= 11) {
				if ((topIndex + newIndex) < (int)cutscenes.size())
					++topIndex;
			} else {
				if ((topIndex + newIndex) < (int)cutscenes.size())
					++selected;
			}
			kbinfo.scan_code = 0;
			break;
		}

		case Common::KEYCODE_RETURN:
			hide_cur();
			out->cls();
			out->setze_zeiger(screen0);
			fx->blende1(workptr, screen0, pal, 150, 0, 0);
			print_rows(546 + topIndex);

			flc->set_custom_user_function(cinema_cut_serv);
			flic_cut(CINEMA_FLICS[topIndex + selected], CFO_MODE);
			flc->remove_custom_user_function();
			out->set_fontadr(font6x8);
			out->set_vorschub(fvorx6x8, fvory6x8);
			show_cur();
			delay = 0;
			flag = false;
			break;

		default:
			break;
		}

		// The below are hacks to get the dialog to work in ScummVM
		kbinfo.scan_code = 0;
		minfo.button = 0;
		txt_anz = 0;

		if (!txt_anz) {
			cur->plot_cur();

			if (flag) {
				flag = false;
				out->setze_zeiger(screen0);
				room->set_ak_pal(&room_blk);
				fx->blende1(workptr, screen0, pal, 150, 0, 0);
			} else {
				out->back2screen(workpage);
			}
		}

		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;
	}

	room->open_handle(EPISODE1, "rb", 0);
	room->set_ak_pal(&room_blk);
	hide_cur();
	uhr->reset_timer(0, 5);
}

int16 MainMenu::cinema_cut_serv(int16 frame) {
	if (in->get_switch_code() == ESC) {
		ailsnd->stopMod();
		ailsnd->endSound();
		return -1;

	} else {
		return 0;
	}
}

} // namespace Chewy
