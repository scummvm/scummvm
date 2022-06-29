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
#include "chewy/dialogs/main_menu.h"
#include "chewy/dialogs/cinema.h"
#include "chewy/dialogs/credits.h"
#include "chewy/dialogs/files.h"
#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/mcga_graphics.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Dialogs {

int MainMenu::_selection;
int MainMenu::_personAni[3];

void MainMenu::execute() {
	// Convenience during testing to not keep showing title sequence
	if (!ConfMan.getBool("skip_title")) {
		g_engine->_sound->playMusic(17);

		// NOTE: Originally, this was set to play video 200, but this actually
		// jumped to the very last video in the file, so we play it explicitly
		flic_cut(g_engine->getLanguage() == Common::Language::DE_DEU ? FCUT_155 : FCUT_160);
		g_engine->_sound->stopMusic();
	}

	show_intro();

	_G(cur)->move(152, 92);
	g_events->_mousePos.x = 152;
	g_events->_mousePos.y = 92;
	_G(cur)->setInventoryCursor(-1);
	_G(menu_display) = 0;
	_G(gameState).soundLoopMode = 1;

	bool done = false;
	while (!done && !SHOULD_QUIT) {
		g_engine->_sound->stopAllSounds();
		_G(SetUpScreenFunc) = screenFunc;

		cursorChoice(CUR_POINT);
		_selection = -1;
		_G(gameState).scrollx = _G(gameState).scrolly = 0;
		_G(gameState)._personRoomNr[P_CHEWY] = 98;
		_G(room)->loadRoom(&_G(room_blk), 98, &_G(gameState));

		g_engine->_sound->playRoomMusic(98);
		_G(fx)->border(_G(workpage), 0, 0);

		_G(out)->setPalette(_G(pal));
		_G(gameState)._personHide[P_CHEWY] = true;
		showCur();

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
			_G(fx)->border(_G(workpage), 0, 0);
			_G(out)->setPointer(_G(workptr));
			_G(flags).NoPalAfterFlc = true;
			flic_cut(FCUT_135);
			break;

		case MM_LOAD_GAME:
			if (loadGame())
				playGame();
			break;

		case MM_CINEMA:
			cursorChoice(CUR_SAVE);
			_G(cur)->move(152, 92);
			g_events->_mousePos.x = 152;
			g_events->_mousePos.y = 92;
			Dialogs::Cinema::execute();
			break;

		case MM_QUIT:
			_G(out)->setPointer(nullptr);
			_G(out)->cls();
			done = true;
			break;

		case MM_CREDITS:
			_G(fx)->border(_G(workpage), 0, 0);
			_G(flags).NoPalAfterFlc = true;
			flic_cut(FCUT_159);
			_G(fx)->border(_G(workpage), 0, 0);
			Dialogs::Credits::execute();
			break;

		default:
			break;
		}
	}
}

void MainMenu::screenFunc() {
	int vec = _G(det)->maus_vector(g_events->_mousePos.x + _G(gameState).scrollx, g_events->_mousePos.y + _G(gameState).scrolly);

	if (_G(in)->getSwitchCode() == 28 || _G(minfo).button == 1) {
		_selection = vec;
	}
}

void MainMenu::animate() {
	if (_G(ani_timer)->_timeFlag) {
		_G(uhr)->resetTimer(0, 0);
		_G(gameState).DelaySpeed = _G(FrameSpeed) / _G(gameState).FramesPerSecond;
		_G(moveState)->Delay = _G(gameState).DelaySpeed + _G(spz_delay)[0];
		_G(FrameSpeed) = 0;
		_G(det)->set_global_delay(_G(gameState).DelaySpeed);
	}

	++_G(FrameSpeed);
	_G(out)->setPointer(_G(workptr));
	_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage],
		_G(gameState).scrollx, _G(gameState).scrolly);

	if (_G(SetUpScreenFunc) && !_G(menu_display) && !_G(flags).InventMenu) {
		_G(SetUpScreenFunc)();
		_G(out)->setPointer(_G(workptr));
	}

	spriteEngine();
	kb_mov(1);
	calcMouseText(g_events->_mousePos.x, g_events->_mousePos.y, 1);
	_G(cur)->updateCursor();
	_G(mouseLeftClick) = false;
	_G(out)->setPointer(nullptr);
	_G(out)->copyToScreen();

	g_screen->update();
	g_events->update();
}

void MainMenu::startGame() {
	hideCur();
	animate();
	exit_room(-1);

	uint8 framesPerSecond = _G(gameState).FramesPerSecond;
	int sndLoopMode = _G(gameState).soundLoopMode;

	var_init();

	_G(gameState).FramesPerSecond = framesPerSecond;
	_G(gameState).soundLoopMode = sndLoopMode;

	_G(gameState)._personRoomNr[P_CHEWY] = 0;
	_G(room)->loadRoom(&_G(room_blk), 0, &_G(gameState));

	_G(moveState)[P_CHEWY].Phase = 6;
	_G(moveState)[P_CHEWY].PhAnz = _G(chewy_ph_nr)[6];
	setPersonPos(160, 80, P_CHEWY, P_RIGHT);
	_G(fx_blend) = BLEND3;
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(menu_item) = CUR_WALK;
	cursorChoice(CUR_WALK);
	enter_room(-1);
	_G(auto_obj) = 0;
}

bool MainMenu::loadGame() {
	_G(flags).SaveMenu = true;
	savePersonAni();
	_G(out)->setPointer((byte *)g_screen->getPixels());
	_G(fontMgr)->setFont(_G(font6));
	cursorChoice(CUR_SAVE);
	_G(cur)->move(152, 92);
	g_events->_mousePos.x = 152;
	g_events->_mousePos.y = 92;
	_G(savegameFlag) = true;
	int result = Files::execute(false);

	cursorChoice((_G(cur)->usingInventoryCursor() && _G(menu_item) == CUR_USE) ? 8 : 0);
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
	_G(mouseLeftClick) = false;
	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;

	_G(flags).mainMouseFlag = false;
	_G(flags).MainInput = true;
	_G(flags).ShowAtsInvTxt = true;
	_G(cur)->showCursor();
	_G(moveState)[P_CHEWY].Count = 0;
	_G(uhr)->resetTimer(0, 0);

	while (!SHOULD_QUIT && !mainLoop(1)) {
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
