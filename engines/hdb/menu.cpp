/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "hdb/hdb.h"
#include "hdb/gfx.h"
#include "hdb/input.h"
#include "hdb/lua-script.h"
#include "hdb/menu.h"
#include "hdb/mpc.h"

namespace HDB {

Menu::Menu() {
	_starWarp = 0;
	_rocketEx = 0;
	_titleActive = false;
	_menuActive = false;
	_optionsActive = false;
	_gamefilesActive = false;
	_newgameActive = false;
	_warpActive = false;
	_optionsScrolling = false;
	_optionsScrollX = kMenuX;
	_rocketX = kMRocketX;
	_sayHDB = false;
	_menuKey = 0;

	_nextScreen = 0;

	_nebulaY = 0;		// Used as a flag
	_fStars[0].y = 0;	// Used as a flag

	_keyAssignUp = Common::KEYCODE_UP;
	_keyAssignDown = Common::KEYCODE_DOWN;
	_keyAssignLeft = Common::KEYCODE_LEFT;
	_keyAssignRight = Common::KEYCODE_RIGHT;
	_keyAssignUse = Common::KEYCODE_RETURN;

	_gCheckEmpty = NULL;
	_gCheckOff = NULL;
	_gCheckOn = NULL;
	_gCheckLeft = NULL;
	_gCheckRight = NULL;

	_contArrowUp = NULL;
	_contArrowDown = NULL;
	_contArrowLeft = NULL;
	_contArrowRight = NULL;
	_contAssign = NULL;

	_waitingForKey = false;
	_warpPlaque = NULL;
	_hdbLogoScreen = NULL;

	_titleScreen = NULL;
	_oohOohGfx = NULL;
	_newGfx = NULL;
	_loadGfx = NULL;
	_optionsGfx = NULL;
	_quitGfx = NULL;
	_resumeGfx = NULL;
	_slotGfx = NULL;
	_rocketMain = NULL;
	_rocketSecond = NULL;
	_rocketEx1 = NULL;
	_rocketEx2 = NULL;
	_titleLogo = NULL;
	_hdbLogoScreen = NULL;
	for (int i = 0; i < kNebulaCount; i++)
		_nebulaGfx[i] = NULL;

	_sliderLeft = NULL;
	_sliderMid = NULL;
	_sliderRight = NULL;
	_sliderKnob = NULL;
	_modePuzzleGfx = NULL;
	_modeActionGfx = NULL;
	_modeLoadGfx = NULL;
	_modeSaveGfx = NULL;
	_menuBackoutGfx = NULL;
	_menuBackspaceGfx = NULL;

	_controlButtonGfx = NULL;

	_controlsGfx = NULL;

	_vortexian[0] = _vortexian[1] = _vortexian[2] = NULL;

	_star[0] = _star[1] = _star[2] = NULL;

	// secret stars
	_starRedGfx[0] = _starRedGfx[1] = NULL;
	_starGreenGfx[0] = _starGreenGfx[1] = NULL;
	_starBlueGfx[0] = _starBlueGfx[1] = NULL;

	_versionGfx = NULL;
	_warpGfx = NULL;
}

Menu::~Menu() {
	freeMenu();

	delete _gCheckEmpty;
	delete _gCheckOff;
	delete _gCheckOn;
	delete _gCheckLeft;
	delete _gCheckRight;

	delete _contArrowUp;
	delete _contArrowDown;
	delete _contArrowLeft;
	delete _contArrowRight;
	delete _contAssign;

	delete _warpPlaque;
	delete _hdbLogoScreen;
}

bool Menu::init() {
	_gCheckEmpty = g_hdb->_gfx->loadPic(G_CHECK_EMPTY);
	_gCheckOff = g_hdb->_gfx->loadPic(G_CHECK_OFF);
	_gCheckOn = g_hdb->_gfx->loadPic(G_CHECK_ON);
	_gCheckLeft = g_hdb->_gfx->loadPic(G_CHECK_LEFT);
	_gCheckRight = g_hdb->_gfx->loadPic(G_CHECK_RIGHT);

	_contArrowUp = g_hdb->_gfx->loadPic(CTRL_ARROWUP);
	_contArrowDown = g_hdb->_gfx->loadPic(CTRL_ARROWDN);
	_contArrowLeft = g_hdb->_gfx->loadPic(CTRL_ARROWLEFT);
	_contArrowRight = g_hdb->_gfx->loadPic(CTRL_ARROWRIGHT);
	_contAssign = g_hdb->_gfx->loadPic(CTRL_ASSIGN);

	_warpPlaque = g_hdb->_gfx->loadPic(WARP_PLAQUE);
	_hdbLogoScreen = g_hdb->_gfx->loadPic(TITLESCREEN);

	return true;
}

void Menu::readConfig() {
	warning("STUB: readConfig: Music Config not implemented");

	if (ConfMan.hasKey(CONFIG_MSTONE7)) {
		g_hdb->setStarsMonkeystone7(ConfMan.getInt(CONFIG_MSTONE7));
	} else {
		ConfMan.setInt(CONFIG_MSTONE7, STARS_MONKEYSTONE_7_FAKE);
	}

	if (ConfMan.hasKey(CONFIG_MSTONE14)) {
		g_hdb->setStarsMonkeystone14(ConfMan.getInt(CONFIG_MSTONE14));
	} else {
		ConfMan.setInt(CONFIG_MSTONE14, STARS_MONKEYSTONE_14_FAKE);
	}

	if (ConfMan.hasKey(CONFIG_MSTONE21)) {
		g_hdb->setStarsMonkeystone21(ConfMan.getInt(CONFIG_MSTONE21));
	} else {
		ConfMan.setInt(CONFIG_MSTONE21, STARS_MONKEYSTONE_21_FAKE);
	}

	if (ConfMan.hasKey(CONFIG_KEY_UP)) {
		_keyAssignUp = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_UP);
		g_hdb->_input->assignKey(0, _keyAssignUp);
	} else {
		ConfMan.setInt(CONFIG_KEY_UP, _keyAssignUp);
	}

	if (ConfMan.hasKey(CONFIG_KEY_DOWN)) {
		_keyAssignDown = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_DOWN);
		g_hdb->_input->assignKey(1, _keyAssignDown);
	} else {
		ConfMan.setInt(CONFIG_KEY_DOWN, _keyAssignDown);
	}

	if (ConfMan.hasKey(CONFIG_KEY_LEFT)) {
		_keyAssignLeft = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_LEFT);
		g_hdb->_input->assignKey(2, _keyAssignLeft);
	} else {
		ConfMan.setInt(CONFIG_KEY_LEFT, _keyAssignLeft);
	}

	if (ConfMan.hasKey(CONFIG_KEY_RIGHT)) {
		_keyAssignRight = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_RIGHT);
		g_hdb->_input->assignKey(3, _keyAssignRight);
	} else {
		ConfMan.setInt(CONFIG_KEY_RIGHT, _keyAssignRight);
	}

	if (ConfMan.hasKey(CONFIG_KEY_USE)) {
		_keyAssignUse = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_USE);
		g_hdb->_input->assignKey(4, _keyAssignUse);
	} else {
		ConfMan.setInt(CONFIG_KEY_USE, _keyAssignUse);
	}

	if (ConfMan.hasKey(CONFIG_CHEAT)) {
		g_hdb->setCheatingOn();
		debug("Cheating enabled");
	}

	ConfMan.flushToDisk();
}

void Menu::writeConfig() {
	warning("STUB: writeConfig: Music Config not implemented");

	int value;

	value = g_hdb->getStarsMonkeystone7();
	ConfMan.setInt(CONFIG_MSTONE7, value);
	value = g_hdb->getStarsMonkeystone14();
	ConfMan.setInt(CONFIG_MSTONE14, value);
	value = g_hdb->getStarsMonkeystone21();
	ConfMan.setInt(CONFIG_MSTONE21, value);

	ConfMan.setInt(CONFIG_KEY_UP, _keyAssignUp);
	ConfMan.setInt(CONFIG_KEY_DOWN, _keyAssignDown);
	ConfMan.setInt(CONFIG_KEY_LEFT, _keyAssignLeft);
	ConfMan.setInt(CONFIG_KEY_RIGHT, _keyAssignRight);
	ConfMan.setInt(CONFIG_KEY_USE, _keyAssignUse);

	if (g_hdb->getCheatingOn())
		ConfMan.set(CONFIG_CHEAT, "1");
}

static const char nebulaNames[kNebulaCount][32] = {
	BACKSCROLL_PLANET1,
	BACKSCROLL_PLANET2,
	BACKSCROLL_PLANET3,
	BACKSCROLL_PLANET4,
	BACKSCROLL_PLANET5,
	BACKSCROLL_GALAXY1,
	BACKSCROLL_GALAXY2
};


void Menu::startMenu() {
	// stuff that gets loaded-in at Title Screen
	if (!_titleLogo) {
		_titleLogo = g_hdb->_gfx->loadPic(TITLELOGO);
		for (int i = 0; i < kNebulaCount; i++)
			_nebulaGfx[i] = g_hdb->_gfx->loadPic(nebulaNames[i]);

		_rocketMain = g_hdb->_gfx->loadPic(MENU_ROCKETSHIP1);
		_rocketSecond = g_hdb->_gfx->loadPic(MENU_ROCKETSHIP2);
		_rocketEx1 = g_hdb->_gfx->loadPic(MENU_EXHAUST1);
		_rocketEx2 = g_hdb->_gfx->loadPic(MENU_EXHAUST2);
	}
	//
	// menu-only stuff
	//
	_newGfx = g_hdb->_gfx->loadPic(MENU_NEWGAME);
	_loadGfx = g_hdb->_gfx->loadPic(MENU_LOADGAME);
	_optionsGfx = g_hdb->_gfx->loadPic(MENU_OPTIONS);
	_quitGfx = g_hdb->_gfx->loadPic(MENU_QUIT);
	_resumeGfx = g_hdb->_gfx->loadPic(MENU_RESUME);
	_slotGfx = g_hdb->_gfx->loadPic(GAMEFILE_SLOT);
	_menuBackoutGfx = g_hdb->_gfx->loadPic(MENU_BACK);
	_menuBackspaceGfx = g_hdb->_gfx->loadPic(MENU_BACKSPACE);
	_controlButtonGfx = g_hdb->_gfx->loadPic(MENU_CONTROLS);
	_controlsGfx = g_hdb->_gfx->loadPic(PIC_CONTROLSSCREEN);

	_vortexian[0] = g_hdb->_gfx->loadTile(GROUP_ENT_VORTEXIAN_STANDDOWN"01");
	_vortexian[1] = g_hdb->_gfx->loadTile(GROUP_ENT_VORTEXIAN_STANDDOWN"02");
	_vortexian[2] = g_hdb->_gfx->loadTile(GROUP_ENT_VORTEXIAN_STANDDOWN"03");

	_modeLoadGfx = g_hdb->_gfx->loadPic(MENU_LOAD);
	_modeSaveGfx = g_hdb->_gfx->loadPic(MENU_SAVE);

	_modePuzzleGfx = g_hdb->_gfx->loadPic(MODE_PUZZLE);
	_modeActionGfx = g_hdb->_gfx->loadPic(MODE_ACTION);

	_sliderLeft = g_hdb->_gfx->loadPic(MENU_SLIDER_LEFT);
	_sliderMid = g_hdb->_gfx->loadPic(MENU_SLIDER_MID);
	_sliderRight = g_hdb->_gfx->loadPic(MENU_SLIDER_RIGHT);
	_sliderKnob = g_hdb->_gfx->loadPic(MENU_SLIDER_KNOB);

	_starRedGfx[0] = g_hdb->_gfx->loadPic(SECRETSTAR_RED1);
	_starRedGfx[1] = g_hdb->_gfx->loadPic(SECRETSTAR_RED2);
	_starGreenGfx[0] = g_hdb->_gfx->loadPic(SECRETSTAR_GREEN1);
	_starGreenGfx[1] = g_hdb->_gfx->loadPic(SECRETSTAR_GREEN2);
	_starBlueGfx[0] = g_hdb->_gfx->loadPic(SECRETSTAR_BLUE1);
	_starBlueGfx[1] = g_hdb->_gfx->loadPic(SECRETSTAR_BLUE2);

	// setup menu falling stars
	_star[0] = g_hdb->_gfx->loadPic(STAR_1);
	_star[1] = g_hdb->_gfx->loadPic(STAR_2);
	_star[2] = g_hdb->_gfx->loadPic(STAR_3);

	_versionGfx = g_hdb->_gfx->loadPic(MENU_VERSION_NUMBER);

	_warpGfx = g_hdb->_gfx->loadPic(MENU_WARP);

	// if we're popping back into menu, don't init this
	if (!_fStars[0].y) {
		for (int i = 0; i < kMaxStars; i++) {
			_fStars[i].y = -30;
			_fStars[i].x = g_hdb->_rnd->getRandomNumber(kScreenWidth - 1);
			_fStars[i].speed = g_hdb->_rnd->getRandomNumber(4) + 1;
			_fStars[i].anim = g_hdb->_rnd->getRandomNumber(2);
			_fStars[i].delay = 5;
		}
	}

	_quitScreen = NULL;

	// did we skip the intro?
	if (!_nebulaY) {
		g_hdb->_gfx->setup3DStars();	// setup the star info

		_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount - 1);
		_nebulaX = g_hdb->_rnd->getRandomNumber(kScreenWidth - 1) + 10;
		_nebulaY = -20;
		_nebulaYVel = g_hdb->_rnd->getRandomNumber(9) + 2;
	}

	_optionsScrollX = kMenuX;
	_oBannerY = -48;
	_rocketY = kMRocketY;
	_rocketX = kMRocketX;
	_menuActive = true;
	_clickDelay = 30;

	fillSavegameSlots();

	// did we already say "HYPERSPACE DELIVERY BOY!" ??
	// if not, this is a great time to check for Copy Protection!
	if (_sayHDB == false) {
		g_hdb->_sound->playSound(SND_HDB);
		_sayHDB = true;
	}
}

void Menu::changeToMenu() {
	if (!g_hdb->_sound->songPlaying(_titleSong)) {
		g_hdb->_sound->stopMusic();
		g_hdb->_sound->startMusic(_titleSong);
	}
}

void Menu::drawMenu() {
	// DEC the counter...
	if (_clickDelay)
		_clickDelay--;

	//	sound.UpdateMusic();		// fading in/out
	g_hdb->_gfx->turnOffFade();			// heh

	//-------------------------------------------------------------------
	// Draw the MAIN MENU
	//-------------------------------------------------------------------
	if (_menuActive) {
		drawRocketAndSelections();

		// draw version #
		_versionGfx->drawMasked(kScreenWidth - 6 * 8, kScreenHeight - 8);

		//
		// see if the Options/GameFiles menu has scrolled off
		//
		if (_optionsScrolling) {
			_optionsScrollX += _optionsXV;
			_rocketX += -_optionsXV;
			_oBannerY += _optionsXV / 3;
			_optionsXV += 3;
			if (_optionsScrollX > kScreenWidth + 10) {
				switch (_nextScreen) {
				case 0: _optionsActive = true; break;
				case 1: _gamefilesActive = 1; break;
				case 2: _newgameActive = true; break;
				}

				_oBannerY = 0;
				_optionsScrolling = false;
				_menuActive = false;
			}
		}

		//
		// Draw the Secret Stars! (tm)
		//
		if (g_hdb->getStarsMonkeystone7() == STARS_MONKEYSTONE_7 ||
			g_hdb->getStarsMonkeystone14() == STARS_MONKEYSTONE_14 ||
			g_hdb->getStarsMonkeystone21() == STARS_MONKEYSTONE_21) {
			static int anim = 0, angler = 0, angleb = 90, angleg = 180;
			static uint32 anim_time;

			if (g_hdb->getStarsMonkeystone7() == STARS_MONKEYSTONE_7)
				_starRedGfx[anim]->drawMasked(
					kStarRedX + (int)(5 * g_hdb->_gfx->getCos(angler)),
					kStarRedY + (int)(5 * g_hdb->_gfx->getSin(angler))
				);
			if (g_hdb->getStarsMonkeystone14() == STARS_MONKEYSTONE_14)
				_starGreenGfx[anim]->drawMasked(
					kStarGreenX + (int)(5 * g_hdb->_gfx->getCos(angleg)),
					kStarGreenY + (int)(5 * g_hdb->_gfx->getSin(angleg))
				);
			if (g_hdb->getStarsMonkeystone21() == STARS_MONKEYSTONE_21)
				_starBlueGfx[anim]->drawMasked(
					kStarBlueX + (int)(5 * g_hdb->_gfx->getCos(angleb)),
					kStarBlueY + (int)(5 * g_hdb->_gfx->getSin(angleb))
				);

			angler += 10; if (angler > 359) angler = 0;
			angleg += 10; if (angleg > 359) angleg = 0;
			angleb += 10; if (angleb > 359) angleb = 0;

			if (anim_time < g_hdb->getTimeSlice()) {
				anim_time = g_hdb->getTimeSlice() + 500;
				anim = 1 - anim;
			}
		}

		//#ifndef HDB_DEMO
		//
		// Draw WARP
		//
		if (g_hdb->getCheatingOn() && _warpGfx)
			_warpGfx->drawMasked(0, kScreenHeight - _warpGfx->_height);
		//#endif
	} else if (_newgameActive) {
		//-------------------------------------------------------------------
		// Draw the NEWGAME menu
		//-------------------------------------------------------------------
		g_hdb->_gfx->draw3DStars();

		//
		// see if the Options menu has scrolled back on
		//
		if (_optionsScrolling) {
			_optionsScrollX += _optionsXV;
			_rocketX += -_optionsXV;
			_oBannerY += _optionsXV / 3;
			_optionsXV -= 3;
			if (_optionsScrollX < kMenuX) {
				_optionsScrollX = kMenuX;
				_rocketX = kMRocketX;
				_oBannerY = -48;
				_optionsScrolling = false;
				_newgameActive = false;
				_menuActive = true;
			}

			drawRocketAndSelections();
		} else {
			drawNebula();
			_newGfx->drawMasked(centerPic(_newGfx), _oBannerY);

			_modePuzzleGfx->drawMasked(kNewGameX, kModePuzzleY);
			_modePuzzleGfx->drawMasked(kNewGameX, kModeActionY);

			g_hdb->_gfx->setCursor(kNewGameX2, kModePuzzleY - 10);
			g_hdb->_gfx->drawText("PUZZLE MODE");
			g_hdb->_gfx->setCursor(kNewGameX2, kModePuzzleY + 10);
			g_hdb->_gfx->drawText("In this mode, the focus");
			g_hdb->_gfx->setCursor(kNewGameX2, kModePuzzleY + 22);
			g_hdb->_gfx->drawText("is on solving puzzles and");
			g_hdb->_gfx->setCursor(kNewGameX2, kModePuzzleY + 34);
			g_hdb->_gfx->drawText("avoiding enemies.");

			g_hdb->_gfx->setCursor(kNewGameX2, kModeActionY - 10);
			g_hdb->_gfx->drawText("ACTION MODE");
			g_hdb->_gfx->setCursor(kNewGameX2, kModeActionY + 10);
			g_hdb->_gfx->drawText("In this mode, the focus");
			g_hdb->_gfx->setCursor(kNewGameX2, kModeActionY + 22);
			g_hdb->_gfx->drawText("is on solving puzzles and");
			g_hdb->_gfx->setCursor(kNewGameX2, kModeActionY + 34);
			g_hdb->_gfx->drawText("attacking enemies!");

			// title logo
			_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + kMTitleY);
			_menuBackoutGfx->drawMasked(kBackoutX, kBackoutY);
		}
	} else if (_optionsActive) {
		//-------------------------------------------------------------------
		// Draw the OPTIONS menu
		//-------------------------------------------------------------------
		g_hdb->_gfx->draw3DStars();

		//
		// see if the Options menu has scrolled back on
		//
		if (_optionsScrolling) {
			_optionsScrollX += _optionsXV;
			_rocketX += -_optionsXV;
			_oBannerY += _optionsXV / 3;
			_optionsXV -= 3;
			if (_optionsScrollX < kMenuX) {
				_optionsScrollX = kMenuX;
				_rocketX = kMRocketX;
				_oBannerY = -48;
				_optionsScrolling = false;
				_optionsActive = false;
				_menuActive = true;
			}

			drawRocketAndSelections();
		} else if (_optionsActive == 1) {
			//
			// Options menu content
			//

			drawNebula();
			_optionsGfx->drawMasked(centerPic(_optionsGfx), _oBannerY);

			g_hdb->_gfx->setCursor(kOptionsX + kOptionSPC, kOptionsY);
			if (!g_hdb->_sound->getMusicVolume())
				g_hdb->_gfx->drawText("Music OFF");
			else
				g_hdb->_gfx->drawText("Music Volume");

			drawSlider(kOptionsX, kOptionsY + 20, g_hdb->_sound->getMusicVolume());

			g_hdb->_gfx->setCursor(kOptionsX + kOptionSPC, kOptionsY + kOptionLineSPC * 2);
			if (!g_hdb->_sound->getSFXVolume())
				g_hdb->_gfx->drawText("Sound Effects OFF");
			else
				g_hdb->_gfx->drawText("Sound Effects Volume");

			drawSlider(kOptionsX, kOptionsY + kOptionLineSPC * 2 + 20, g_hdb->_sound->getSFXVolume());

			// Voices ON or OFF
			drawToggle(kOptionsX, kOptionsY + kOptionLineSPC * 4 + 20, g_hdb->_sound->getVoiceStatus());

			if (!g_hdb->isVoiceless()) {
				g_hdb->_gfx->setCursor(kOptionsX + kOptionSPC + 24, kOptionsY + kOptionLineSPC * 4 + 24);
				if (!g_hdb->_sound->getVoiceStatus())
					g_hdb->_gfx->drawText("Voice Dialogue OFF");
				else
					g_hdb->_gfx->drawText("Voice Dialogue ON");
			}


			// title logo
			_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + kMTitleY);
			_menuBackoutGfx->drawMasked(kBackoutX, kBackoutY);

			// Ignore Controls Screen Button
			//_controlButtonGfx->drawMasked(centerPic(_controlButtonGfx), kMControlsY);
		} else if (_optionsActive == 2) {
			//
			// Draw CONTROLS screen
			//

			// Ignore Controls Assignment
			//controlsDraw();
			return;
		}
	} else if (_gamefilesActive) {
		//-------------------------------------------------------------------
		//	DRAW GAMEFILES MENU
		//-------------------------------------------------------------------

		g_hdb->_gfx->draw3DStars();
		//
		// see if the Options menu has scrolled back on
		//
		if (_optionsScrolling) {
			_optionsScrollX += _optionsXV;
			_rocketX += -_optionsXV;
			_oBannerY += _optionsXV / 3;
			_optionsXV -= 3;
			if (_optionsScrollX < kMenuX) {
				_optionsScrollX = kMenuX;
				_rocketX = kMRocketX;
				_oBannerY = -48;
				_optionsScrolling = false;
				_gamefilesActive = false;
				_menuActive = true;
			}

			drawRocketAndSelections();
		} else {
			static int anim = 0;
			static uint32 anim_time = 0;

			drawNebula();
			_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + kMTitleY);
			// CHOOSE SLOT screen
			_modeLoadGfx->drawMasked(centerPic(_modeLoadGfx), _oBannerY);
			_menuBackoutGfx->drawMasked(kBackoutX, kBackoutY);


			if (_saveGames[0].seconds) {
				_vortexian[anim]->drawMasked(kVortSaveX, kVortSaveY);
			}
			if (anim_time < g_hdb->getTimeSlice()) {
				anim_time = g_hdb->getTimeSlice() + 50;
				anim++;
				if (anim > 2)
					anim = 0;
			}

			for (int i = 0; i < kNumSaveSlots; i++) {
				int seconds = _saveGames[i].seconds;

				_slotGfx->drawMasked(kSaveSlotX - 8, i * 32 + (kSaveSlotY - 4));
				if (seconds || _saveGames[i].mapName[0]) {

					g_hdb->_gfx->setTextEdges(0, kScreenWidth + 60, 0, kScreenHeight);
					g_hdb->_gfx->setCursor(kSaveSlotX, i * 32 + kSaveSlotY);
					g_hdb->_gfx->drawText(_saveGames[i].mapName);

					g_hdb->_gfx->setCursor(kSaveSlotX + 180, i * 32 + kSaveSlotY);
					char buff[16];
					sprintf(buff, "%02d:%02d", seconds / 3600, (seconds / 60) % 60);
					g_hdb->_gfx->drawText(buff);
				}
			}
		}
	} else if (_warpActive) {
		//-------------------------------------------------------------------
		//	DRAW WARP MENU
		//-------------------------------------------------------------------
		g_hdb->_gfx->draw3DStars();
		drawNebula();
		drawWarpScreen();
		// title logo
		_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + kMTitleY);
		_menuBackoutGfx->drawMasked(kWarpBackoutX, kWarpBackoutY);

		char string[32];
		for (int i = 0; i < 10; i++) {
			sprintf(string, "Map %2d", i);
			g_hdb->_gfx->setCursor(kWarpX + 4, i * 16 + kWarpY);
			g_hdb->_gfx->drawText(string);
		}
		for (int i = 0; i < 10; i++) {
			sprintf(string, "Map %d", i + 10);
			g_hdb->_gfx->setCursor(kWarpX + 80, i * 16 + kWarpY);
			g_hdb->_gfx->drawText(string);
		}
		for (int i = 0; i < 10; i++) {
			sprintf(string, "Map %d", i + 20);
			g_hdb->_gfx->setCursor(kWarpX + 160, i * 16 + kWarpY);
			g_hdb->_gfx->drawText(string);
		}

		if (_warpActive > 1) {
			g_hdb->_gfx->setCursor(kWarpX + 60, kWarpY + 164);
			sprintf(string, "Warping to MAP%d", _warpActive - 2);
			g_hdb->_gfx->centerPrint(string);
		}
	} else if (_quitActive) {
		//-------------------------------------------------------------------
		//	DRAW QUIT SCREEN
		//-------------------------------------------------------------------
		g_hdb->_gfx->draw3DStars();
		drawNebula();
		{
			if (!_quitScreen)
				_quitScreen = g_hdb->_gfx->loadPic(PIC_QUITSCREEN);
			_quitScreen->drawMasked(kQuitX, kQuitY);
		}
	}
}

void Menu::freeMenu() {
	// title sequence stuff
	if (_titleScreen)
		delete _titleScreen;
	_titleScreen = NULL;
	if (_oohOohGfx)
		delete _oohOohGfx;
	_oohOohGfx = NULL;

	if (_newGfx)
		delete _newGfx;
	_newGfx = NULL;
	if (_loadGfx)
		delete _loadGfx;
	_loadGfx = NULL;
	if (_optionsGfx)
		delete _optionsGfx;
	_optionsGfx = NULL;
	if (_quitGfx)
		delete _quitGfx;
	_quitGfx = NULL;
	if (_resumeGfx)
		delete _resumeGfx;
	_resumeGfx = NULL;
	if (_slotGfx)
		delete _slotGfx;
	_slotGfx = NULL;
	if (_rocketMain)
		delete _rocketMain;
	_rocketMain = NULL;
	if (_rocketSecond)
		delete _rocketSecond;
	_rocketSecond = NULL;
	if (_rocketEx1)
		delete _rocketEx1;
	_rocketEx1 = NULL;
	if (_rocketEx2)
		delete _rocketEx2;
	_rocketEx2 = NULL;
	if (_titleLogo)
		delete _titleLogo;
	_titleLogo = NULL;

	if (_hdbLogoScreen)
		delete _hdbLogoScreen;
	_hdbLogoScreen = NULL;

	if (_nebulaGfx[0]) {
		for (int i = 0; i < kNebulaCount; i++) {
			delete _nebulaGfx[i];
			_nebulaGfx[i] = NULL;
		}
	}

	if (_sliderLeft)
		delete _sliderLeft;
	_sliderLeft = NULL;
	if (_sliderMid)
		delete _sliderMid;
	_sliderMid = NULL;
	if (_sliderRight)
		delete _sliderRight;
	_sliderRight = NULL;
	if (_sliderKnob)
		delete _sliderKnob;
	_sliderKnob = NULL;
	if (_modePuzzleGfx)
		delete _modePuzzleGfx;
	_modePuzzleGfx = NULL;
	if (_modeActionGfx)
		delete _modeActionGfx;
	_modeActionGfx = NULL;
	if (_modeLoadGfx)
		delete _modeLoadGfx;
	_modeLoadGfx = NULL;
	if (_modeSaveGfx)
		delete _modeSaveGfx;
	_modeSaveGfx = NULL;
	if (_menuBackoutGfx)
		delete _menuBackoutGfx;
	_menuBackoutGfx = NULL;
	if (_menuBackspaceGfx)
		delete _menuBackspaceGfx;
	_menuBackspaceGfx = NULL;

	if (_controlButtonGfx)
		delete _controlButtonGfx;
	_controlButtonGfx = NULL;

	if (_controlsGfx)
		delete _controlsGfx;
	_controlsGfx = NULL;

	if (_vortexian[0]) {
		delete _vortexian[0];
		delete _vortexian[1];
		delete _vortexian[2];
		_vortexian[0] = _vortexian[1] = _vortexian[2] = NULL;
	}

	if (_star[0]) {
		delete _star[0];
		delete _star[1];
		delete _star[2];
		_star[0] = _star[1] = _star[2] = NULL;
	}

	// secret stars
	if (_starRedGfx[0]) {
		delete _starRedGfx[0];
		delete _starRedGfx[1];
		delete _starGreenGfx[0];
		delete _starGreenGfx[1];
		delete _starBlueGfx[0];
		delete _starBlueGfx[1];
		_starRedGfx[0] = _starRedGfx[1] = NULL;
		_starGreenGfx[0] = _starGreenGfx[1] = NULL;
		_starBlueGfx[0] = _starBlueGfx[1] = NULL;
	}

	if (_versionGfx)
		delete _versionGfx;
	_versionGfx = NULL;

	if (_warpGfx)
		delete _warpGfx;
	_warpGfx = NULL;
}

bool Menu::startTitle() {
	readConfig();

	_titleScreen = g_hdb->_gfx->loadPic(MONKEYLOGOSCREEN);
	if (!_titleScreen)
		return false;

	_oohOohGfx = g_hdb->_gfx->loadPic(MONKEYLOGO_OOHOOH);
	_rocketMain = g_hdb->_gfx->loadPic(MENU_ROCKETSHIP1);
	_rocketSecond = g_hdb->_gfx->loadPic(MENU_ROCKETSHIP2);
	_rocketEx1 = g_hdb->_gfx->loadPic(MENU_EXHAUST1);
	_rocketEx2 = g_hdb->_gfx->loadPic(MENU_EXHAUST2);
	_titleLogo = g_hdb->_gfx->loadPic(TITLELOGO);

	for (int i = 0; i < kNebulaCount; i++)
		_nebulaGfx[i] = g_hdb->_gfx->loadPic(nebulaNames[i]);

	_titleCycle = 1;	// 1 = Waiting for OOH OOH

	_titleDelay = g_system->getMillis() + 1000 * TITLE_DELAY1;

	g_hdb->_sound->stopMusic();
	_introSong = SONG_TITLE;
	_titleSong = SONG_MENU;
	g_hdb->_sound->startMusic(_introSong);

	return _titleActive = true;
}

void Menu::drawTitle() {
	static uint32 time;

	//sound.UpdateMusic();

	if (!_titleActive)
		return;

	//-------------------------------------------------------------------
	// draw special gfx
	//-------------------------------------------------------------------
	switch (_titleCycle) {
	case 1:
	case 3:
		// draw entire screen
		_titleScreen->draw(0, 0);
		break;

	case 2:
		// draw entire screen & ooh ooh
		_titleScreen->draw(0, 0);
		_oohOohGfx->draw(kOohOhhX, kOohOhhY);
		break;

	case 4: // fadeout monkeystone logo
		_titleScreen->draw(0, 0);
		break;

	case 5: // fadein HDB title screen
	case 6: // wait
	case 7: // fadeout HDB title screen
		_hdbLogoScreen->draw(0, 0);
		break;

		// draw the rocket & exhaust until it stops
	case 8:
		g_hdb->_gfx->draw3DStars();

		// draw nebula
		_nebulaGfx[_nebulaWhich]->draw(_nebulaX, _nebulaY);
		_nebulaY += _nebulaYVel;
		if (_nebulaY > kScreenHeight + (kScreenHeight / 2)) {
			_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount - 1);
			_nebulaX = g_hdb->_rnd->getRandomNumber(kScreenWidth - 1) + 10;
			_nebulaY = -11 * 8;
			_nebulaYVel = g_hdb->_rnd->getRandomNumber(3) + 1;
			if (_nebulaWhich > 4)		// galaxy?
				_nebulaYVel = 1;
		}

		// draw rocket
		_rocketMain->drawMasked(kMRocketX, _rocketY);
		_rocketSecond->drawMasked(kMRocketX + 40, _rocketY + kMRocketYBottom);

		// exhaust
		if (_rocketEx < 5) {
			_rocketEx1->drawMasked(kMRocketX + kMRocketEXHX, _rocketY + kMRocketYBottom);
			_rocketEx2->drawMasked(kMRocketX + kMRocketEXHX2, _rocketY + kMRocketYBottom);
		} else if (_rocketEx >= 5 && _rocketEx < 10) {
			_rocketEx2->drawMasked(kMRocketX + kMRocketEXHX, _rocketY + kMRocketYBottom);
			_rocketEx1->drawMasked(kMRocketX + kMRocketEXHX2, _rocketY + kMRocketYBottom);
		} else {
			_rocketEx = 0;
			_rocketEx1->drawMasked(kMRocketX + kMRocketEXHX, _rocketY + kMRocketYBottom);
			_rocketEx2->drawMasked(kMRocketX + kMRocketEXHX2, _rocketY + kMRocketYBottom);
		}
		_rocketEx++;

		// title logo
		_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + kMTitleY);

		break;
	}

	// timer countdown...
	if (_titleDelay > g_system->getMillis())
		return;

	//-------------------------------------------------------------------
	// change title state...
	//-------------------------------------------------------------------
	switch (_titleCycle) {
		//-------------------------------------------------------------------
		// MONKEYSTONE LOGO
		//-------------------------------------------------------------------
		// time to OOH OOH
	case 1:
		_titleDelay = (uint32)(g_system->getMillis() + 1000 * TITLE_DELAY2);
		g_hdb->_sound->playSound(SND_MONKEY_OOHOOH);
		_titleCycle++;
		break;

		// delay after OOH OOH
	case 2:
		_titleDelay = g_system->getMillis() + 1000 * TITLE_DELAY3;
		_titleCycle++;
		break;

		// done with delay; set up the fadeout...
	case 3:
		time = g_hdb->getTimeSliceDelta();
		g_hdb->_gfx->setFade(false, false, kScreenFade / time); // FADEOUT
		_titleCycle++;
		g_hdb->_sound->stopMusic();
	break;

	// wait for fadeout, then start music
	case 4:
		if (g_hdb->_gfx->isFadeActive())
			break;

		g_hdb->_gfx->setFade(true, false, kScreenFade / time); // FADEIN
		g_hdb->_sound->startMusic(_titleSong);
		_titleCycle++;
	break;

		//-------------------------------------------------------------------
		// HDB TITLE SCREEN
		//-------------------------------------------------------------------
		// wait for fadein to stop
	case 5:
		if (g_hdb->_gfx->isFadeActive())
			break;
		_titleDelay = g_system->getMillis() + 5000;
		_titleCycle++;
		break;

		// set fadeout to stars
	case 6:
		g_hdb->_gfx->setFade(false, false, kScreenFade / time);		// FADEOUT
		_titleCycle++;
		break;

		// get rocket ready
	case 7:
		if (g_hdb->_gfx->isFadeActive())
			break;
		g_hdb->_gfx->turnOffFade();
		g_hdb->_gfx->fillScreen(0);
		{
			_titleCycle++;
			_rocketY = kScreenHeight;	// ycoord
			_rocketYVel = -1;		// yspeed
			_rocketEx = 0;			// exhaust toggle
			g_hdb->_gfx->setup3DStars();	// setup the star info

			_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount - 1);
			_nebulaX = g_hdb->_rnd->getRandomNumber(kScreenWidth - 1) + 10;
			_nebulaY = -11 * 8;
			_nebulaYVel = g_hdb->_rnd->getRandomNumber(9) + 2;
			if (_nebulaWhich > 4)		// galaxy?
				_nebulaYVel = 1;
		}
		break;

		// move rocket up the screen!
	case 8:
		_titleDelay = 1;

		_rocketY += _rocketYVel;
		if (_rocketY < kMRocketY) {
			_rocketY = kMRocketY;
			_titleCycle = 12;
			_titleDelay = 1;
		}

		break;

		// shut down title....start up menu!
	case 12:
		_titleActive = false;
		g_hdb->changeGameState();
		break;
	}
}

void Menu::fillSavegameSlots() {
	Common::String saveGameFile;

	for (int i = 0; i < kNumSaveSlots; i++) {
		saveGameFile = g_hdb->genSaveFileName(i, false);
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(saveGameFile);

		if (!in) {
			memset(&_saveGames[i], 0, sizeof(Save));
		} else {
			Graphics::skipThumbnail(*in);

			strcpy(_saveGames[i].saveID, saveGameFile.c_str());
			_saveGames[i].seconds = in->readUint32LE();
			in->read(_saveGames[i].mapName, 32);
			delete in;
		}
		_saveGames[i].fileSlot = i + 1;
	}
}

void Menu::processInput(int x, int y) {
	// do not enter if clicking too fast
	if (_clickDelay)
		return;
	_clickDelay = 3;

	//-------------------------------------------------------------------
	//	MAIN MENU INPUT
	//-------------------------------------------------------------------
	if (_menuActive) {
		int	resume = getMenuKey();
		// quit game?
		if (x >= kMenuX && x < kMenuX + kMenuItemWidth &&
			y >= kMenuY + kMQuitY && y < kMenuY + kMQuitY + kMenuItemHeight) {
			g_hdb->_sound->playSound(SND_BYE);
			_quitTimer = g_hdb->getTimeSlice() + 1000;
			_quitActive = true;
			_menuActive = false;
			return;
		} else if (x >= kMenuX && x < kMenuX + kMenuItemWidth &&
			y >= kMenuY && y < kMenuY + kMenuItemHeight) {
			// new game?
			_optionsScrolling = true;
			_optionsXV = 5;
			g_hdb->_sound->playSound(SND_MENU_ACCEPT);
			g_hdb->_sound->freeSound(SND_HDB);
			_nextScreen = 2;
		} else if (x >= kMenuX && x < kMenuX + kMenuItemWidth &&
			y >= kMenuY + kMLoadY && y < kMenuY + kMLoadY + kMenuItemHeight) {
			// game files?
				_optionsScrolling = true;
				_optionsXV = 5;
				g_hdb->_sound->playSound(SND_MENU_ACCEPT);
				_nextScreen = 1;
		} else if (x >= kMenuX && x < kMenuX + kMenuItemWidth &&
			y >= kMenuY + kMOptionsY && y < kMenuY + kMOptionsY + kMenuItemHeight) {
			// options?
			SoundType temp;
			temp = g_hdb->_sound->whatSongIsPlaying();
			if (temp != SONG_NONE)
				_resumeSong = temp;
			_optionsScrolling = true;
			_optionsXV = 5;
			_nextScreen = 0;
			g_hdb->_sound->playSound(SND_MENU_ACCEPT);
		} else if (((x >= kMenuX && x < kMenuX + kMenuItemWidth &&
			y >= kMenuY + kMResumeY && y < kMenuY + kMResumeY + kMenuItemHeight) || resume) &&
			(true == g_hdb->_map->isLoaded() || _saveGames[5].seconds)) {
			// resume game? ( must be playing already or have an autosave )
			g_hdb->_sound->playSound(SND_POP);
			freeMenu();
			// if we're on the secret level, RESUME will load the autosave...
			if (g_hdb->_map->isLoaded() && scumm_strnicmp(g_hdb->currentMapName(), "map30", 5))
				g_hdb->setGameState(GAME_PLAY);
			else {
				if (g_hdb->loadGameState(kAutoSaveSlot).getCode() == Common::kNoError) {
					g_hdb->setGameState(GAME_PLAY);
				}
			}
		} else if (g_hdb->getStarsMonkeystone7() == STARS_MONKEYSTONE_7 ||
			g_hdb->getStarsMonkeystone14() == STARS_MONKEYSTONE_14 ||
			g_hdb->getStarsMonkeystone21() == STARS_MONKEYSTONE_21) {
			// Secret Stars! (tm)
			if (x >= kStarRedX && x <= kStarRedX + _starRedGfx[0]->_width &&
				y >= kStarRedY && y <= kStarRedY + _starRedGfx[0]->_height &&
				g_hdb->getStarsMonkeystone7() == STARS_MONKEYSTONE_7) {
				_optionsActive = false;
				g_hdb->setGameState(GAME_PLAY);
				if (scumm_strnicmp(g_hdb->currentMapName(), "map30", 5))	// don't save if we're already on 30!
					g_hdb->saveGameState(kAutoSaveSlot, "FIXME"); // Add here date/level name // TODO
				_starWarp = 0;
				g_hdb->_sound->playSound(SND_MONKEYSTONE_SECRET_STAR);
				g_hdb->startMap("MAP30");
			} else if (x >= kStarGreenX && x <= kStarGreenX + _starGreenGfx[0]->_width &&
				y >= kStarGreenY && y <= kStarGreenY + _starGreenGfx[0]->_height &&
				g_hdb->getStarsMonkeystone14() == STARS_MONKEYSTONE_14) {
				_optionsActive = false;
				g_hdb->setGameState(GAME_PLAY);
				if (scumm_strnicmp(g_hdb->currentMapName(), "map30", 5))	// don't save if we're already on 30!
					g_hdb->saveGameState(kAutoSaveSlot, "FIXME"); // Add here date/level name // TODO
				_starWarp = 1;
				g_hdb->_sound->playSound(SND_MONKEYSTONE_SECRET_STAR);
				g_hdb->startMap("MAP30");

			} else if (x >= kStarBlueX && x <= kStarBlueX + _starBlueGfx[0]->_width &&
				y >= kStarBlueY && y <= kStarBlueY + _starBlueGfx[0]->_height &&
				g_hdb->getStarsMonkeystone21() == STARS_MONKEYSTONE_21) {
				_optionsActive = false;
				g_hdb->setGameState(GAME_PLAY);
				if (scumm_strnicmp(g_hdb->currentMapName(), "map30", 5))	// don't save if we're already on 30!
					g_hdb->saveGameState(kAutoSaveSlot, "FIXME"); // Add here date/level name // TODO
				_starWarp = 2;
				g_hdb->_sound->playSound(SND_MONKEYSTONE_SECRET_STAR);
				g_hdb->startMap("MAP30");
			}
		}
		// secret warp menu? (click on nebula!)
		int		open;
		if (!g_hdb->getCheatingOn())
			open = (x >= _nebulaX && x < _nebulaX + 16 && y >= _nebulaY && y < _nebulaY + 16);
		else
			open = (y > kMenuExitY && x < kMenuExitXLeft);

		if (open) {

			g_hdb->_sound->playSound(SND_MONKEYSTONE_SECRET_STAR);

			_menuActive = false;
			_warpActive = true;
			_clickDelay = 30;
		}
	} else if (_newgameActive) {
		//-------------------------------------------------------------------
		//	NEWGAME INPUT
		//-------------------------------------------------------------------
		int	xit = getMenuKey();

		if (y >= kMenuExitY || y < kMenuExitYTop || xit) {
			_optionsScrolling = true;
			_optionsXV = -5;
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
		}

		// PUZZLE MODE area
		if (y >= kModePuzzleY - 10 && y <= kModeActionY - 10) {
			g_hdb->setActionMode(0);
			g_hdb->_sound->playSound(SND_MENU_ACCEPT);
			_newgameActive = false;
			g_hdb->changeGameState();
			// that's it!  the Game Loop takes over from here...
		} else if (y >= kModeActionY - 10 && y <= kMenuExitY) {
			// ACTION MODE area
			g_hdb->setActionMode(1);
			g_hdb->_sound->playSound(SND_MENU_ACCEPT);
			_newgameActive = false;
			g_hdb->changeGameState();
		}
	} else if (_optionsActive) {
		//-------------------------------------------------------------------
		//	OPTIONS INPUT
		//-------------------------------------------------------------------
		int	xit = getMenuKey();

		//
		// Controls screen
		//
		if (_optionsActive == 2) {
			controlsInput(x, y);
			return;
		}

		int	offset;
		// Slider 1
		if (x >= 0 && x <= kOptionsX + 200 &&
			y >= kOptionsY + 20 && y <= kOptionsY + 36) {
			int oldVol = g_hdb->_sound->getMusicVolume();
			if (x < kOptionsX) {
				if (oldVol) {
					g_hdb->_sound->stopMusic();
					g_hdb->_sound->setMusicVolume(0);
					g_hdb->_sound->playSound(SND_GUI_INPUT);
				}
			} else {
				offset = ((x - kOptionsX) * 256) / 200;
				g_hdb->_sound->setMusicVolume(offset);
				if (!oldVol)
					g_hdb->_sound->startMusic(_resumeSong);
			}
		} else if (x >= 0 && x <= kOptionsX + 200 &&
			y >= kOptionsY + kOptionLineSPC * 2 + 20 && y <= kOptionsY + kOptionLineSPC * 2 + 36) {
			// Slider 2
			if (x >= kOptionsX)
				offset = ((x - kOptionsX) * 256) / 200;
			else
				offset = 0;
			g_hdb->_sound->setSFXVolume(offset);
			g_hdb->_sound->playSound(SND_MENU_SLIDER);
		} else if (x >= kOptionsX && x <= kOptionsX + 200 &&
			y >= kOptionsY + kOptionLineSPC * 4 + 24 && y <= kOptionsY + kOptionLineSPC * 4 + 40) {
			// Voices ON/OFF
			if (!g_hdb->isVoiceless()) {
				int value = g_hdb->_sound->getVoiceStatus();
				value ^= 1;
				g_hdb->_sound->setVoiceStatus(value);
				g_hdb->_sound->playSound(SND_GUI_INPUT);
			}
		} else if (y >= kMenuExitY || y < kMenuExitYTop || xit) {
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
			_optionsScrolling = true;
			_optionsXV = -5;
		} else if (x >= (kScreenWidth / 2 - _controlButtonGfx->_width / 2) && x < (kScreenWidth / 2 + _controlButtonGfx->_width / 2) &&
			y >= kMControlsY && y < kMControlsY + _controlButtonGfx->_height) {
			// CONTROLS BUTTON!

			// Ignore Controls Button
			//_optionsActive = 2;
			//_clickDelay = 20;
			//g_hdb->_sound->playSound(SND_POP);
		}
	} else if (_gamefilesActive) {
		//-------------------------------------------------------------------
		//	GAMEFILES INPUT
		//-------------------------------------------------------------------
		int	xit = getMenuKey();

		if (y >= kMenuExitY + 15 || y < kMenuExitYTop || xit) {
			_optionsScrolling = true;
			_optionsXV = -5;
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
		}

		int i = 0;
		for (; i < kNumSaveSlots; i++)
			if (y >= (i * 32 + kSaveSlotY - 4) && y <= (i * 32 + kSaveSlotY + 24))
				break;
		if (i >= kNumSaveSlots)
			return;

		_saveSlot = i;

		// LOAD GAME!
		// clicked on empty slot?
		if (!_saveGames[i].seconds && !_saveGames[i].mapName[0]) {
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
			return;
		}

		g_hdb->_sound->playSound(SND_MENU_ACCEPT);
		if (g_hdb->loadGameState(_saveSlot).getCode() == Common::kNoError) {
			_gamefilesActive = false;

			freeMenu();
			g_hdb->setGameState(GAME_PLAY);
			// if we're at the very start of the level, re-init the level
			if (!_saveGames[i].seconds) {
				g_hdb->_lua->callFunction("level_loaded", 0); // call "level_loaded" Lua function, if it exists
				if (!g_hdb->_ai->cinematicsActive())
					g_hdb->_gfx->turnOffFade();
			}
		}
	} else if (_warpActive) {
		//-------------------------------------------------------------------
		//	WARP INPUT
		//-------------------------------------------------------------------
		int	xit = getMenuKey();

		if ((y >= kMenuExitY && x < kMenuExitXLeft) || xit) {
			_menuActive = true;
			_warpActive = false;
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
			_clickDelay = 10;
		} else if (y >= kWarpY && y < kWarpY + 160) {
			int map;

			if (x > kWarpX + 160)
				map = 20;
			else
				if (x > kWarpX + 80)
					map = 10;
				else
					map = 0;

			map += (y - kWarpY) / 16;

			_warpActive = map + 2;
			g_hdb->paint();
			if (g_hdb->getDebug())
				g_hdb->_gfx->updateVideo();
			_warpActive = 0;

			char string[16];
			if (map < 10)
				sprintf(string, "MAP0%d", map);
			else
				sprintf(string, "MAP%d", map);
			freeMenu();
			g_hdb->setGameState(GAME_PLAY);
			g_hdb->_sound->stopMusic();
			g_hdb->_ai->clearPersistent();
			g_hdb->resetTimer();
			g_hdb->_sound->playSound(SND_POP);
			g_hdb->startMap(string);
		}
	} else if (_quitActive) {
		//-------------------------------------------------------------------
		//	QUIT INPUT
		//-------------------------------------------------------------------
		int	xit = getMenuKey();

		if ((x >= kQuitNoX1 && x <= kQuitNoX2 && y > kQuitNoY1 && y < kQuitNoY2 && _quitTimer < g_hdb->getTimeSlice()) || xit) {
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
			delete _quitScreen;
			_quitScreen = NULL;

			_menuActive = true;
			_quitActive = 0;
		} else if (_quitTimer < g_hdb->getTimeSlice()) {
			if (x >= kQuitYesX1 && x <= kQuitYesX2 && y > kQuitYesY1 && y < kQuitYesY2) {
				writeConfig();
				g_hdb->quitGame();
			}
		}
	}
}

void Menu::controlsInput(int x, int y) {
}

void Menu::controlsDraw() {
}

void Menu::drawNebula() {
	// draw nebula
	_nebulaGfx[_nebulaWhich]->draw(_nebulaX, _nebulaY);
	_nebulaY += _nebulaYVel;

	if (_nebulaY > kScreenHeight + (kScreenHeight / 2)) {
		_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount - 1);
		_nebulaX = g_hdb->_rnd->getRandomNumber(kScreenWidth - 1) + 10;
		_nebulaY = -11 * 8;
		_nebulaYVel = g_hdb->_rnd->getRandomNumber(3) + 1;
		if (_nebulaWhich > 4)		// galaxy?
			_nebulaYVel = 1;
	}

	//
	// draw the falling stars
	//
	for (int i = 0; i < kMaxStars; i++) {
		_fStars[i].y += _fStars[i].speed;
		if (_fStars[i].y > kScreenHeight) {
			_fStars[i].y = (g_hdb->_rnd->getRandomNumber(29) + 30) * -1;
			_fStars[i].speed = g_hdb->_rnd->getRandomNumber(4) + 1;
		}
		if (_fStars[i].delay-- < 1) {
			_fStars[i].delay = 5;
			_fStars[i].anim = (_fStars[i].anim + 1) % 3;
		}
		_star[_fStars[i].anim]->drawMasked(_fStars[i].x, _fStars[i].y);
	}
}

void Menu::drawRocketAndSelections() {
	g_hdb->_gfx->draw3DStars();
	drawNebula();

	// top-down/up scrolling stuff
	switch (_nextScreen) {
	case 0: _optionsGfx->drawMasked(centerPic(_optionsGfx), _oBannerY); break;
	case 1: _modeLoadGfx->drawMasked(centerPic(_modeLoadGfx), _oBannerY); break;
	case 2: _newGfx->drawMasked(centerPic(_newGfx), _oBannerY); break;
	}

	// menu items
	_newGfx->drawMasked(_optionsScrollX, kMenuY);
	_modeLoadGfx->drawMasked(_optionsScrollX, kMenuY + kMLoadY);
	_optionsGfx->drawMasked(_optionsScrollX, kMenuY + kMOptionsY);
	_quitGfx->drawMasked(_optionsScrollX, kMenuY + kMQuitY);
	if (g_hdb->_map->isLoaded() || _saveGames[5].seconds)
		_resumeGfx->drawMasked(_optionsScrollX, kMenuY + kMResumeY);

	// draw rocket
	_rocketMain->drawMasked(_rocketX, _rocketY);
	_rocketSecond->drawMasked(_rocketX + 40, _rocketY + kMRocketYBottom);

	// exhaust
	if (_rocketEx < 5) {
		_rocketEx1->drawMasked(kMRocketX + kMRocketEXHX, _rocketY + kMRocketYBottom);
		_rocketEx2->drawMasked(kMRocketX + kMRocketEXHX2, _rocketY + kMRocketYBottom);
	} else if (_rocketEx >= 5 && _rocketEx < 10) {
		_rocketEx2->drawMasked(kMRocketX + kMRocketEXHX, _rocketY + kMRocketYBottom);
		_rocketEx1->drawMasked(kMRocketX + kMRocketEXHX2, _rocketY + kMRocketYBottom);
	} else {
		_rocketEx = 0;
		_rocketEx1->drawMasked(kMRocketX + kMRocketEXHX, _rocketY + kMRocketYBottom);
		_rocketEx2->drawMasked(kMRocketX + kMRocketEXHX2, _rocketY + kMRocketYBottom);
	}
	_rocketEx++;

	// title logo
	_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + kMTitleY);
}

void Menu::drawSlider(int x, int y, int offset) {
	int x1 = x;

	_sliderLeft->drawMasked(x, y);
	x += _sliderLeft->_width;

	for (int i = 0; i < 12; i++) {
		_sliderMid->draw(x, y);
		x += _sliderMid->_width;
	}

	_sliderRight->drawMasked(x, y);
	_sliderKnob->drawMasked(x1 + (offset * 200) / 256, y + 2);
}

void Menu::drawToggle(int x, int y, bool flag) {
	int	x1 = x;

	_gCheckLeft->drawMasked(x, y);
	x += _gCheckLeft->_width;

	for (int i = 0; i < 12; i++) {
		_gCheckEmpty->draw(x, y);
		x += _gCheckEmpty->_width;
	}

	_gCheckRight->drawMasked(x, y);

	if (!flag)
		_gCheckOff->drawMasked(x1 + _sliderLeft->_width, y);
	else
		_gCheckOn->drawMasked(x1 + _sliderLeft->_width, y);
}

void Menu::drawWarpScreen() {
	_warpPlaque->drawMasked(centerPic(_warpPlaque), 64);
}

} // End of Namespace
