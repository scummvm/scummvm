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

#include "common/config-manager.h"
#include "common/random.h"
#include "graphics/thumbnail.h"

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/gfx.h"
#include "hdb/input.h"
#include "hdb/lua-script.h"
#include "hdb/sound.h"
#include "hdb/menu.h"
#include "hdb/map.h"
#include "hdb/mpc.h"

namespace HDB {

Menu::Menu() {

	if (g_hdb->isPPC()) {
		_menuX = 18 * 8;
		_menuY = 16;
		_menuItemWidth = 88;
		_menuItemHeight = 40;
		_mResumeY = 48 * 4;
		_mQuitY = 48 * 3;
		_mOptionsY = 48 * 2;
		_mLoadY = 48 * 1;
		_mControlsY = 160;
		_menuExitXLeft = 60;
		_menuExitY = (g_hdb->_screenHeight - 80);
		_menuExitYTop = 48;
		_menuVortSaveY = 220;
		_menuVortSaveX = 76;
		_mRocketX = 0;
		_mRocketY = 10;
		_mRocketYBottom = 21 * 8;
		_mRocketEXHX = 0;
		_mRocketEXHX2 = 0;
		_mTitleY = 27 * 8;
		_oohOhhX = 88;
		_oohOhhY = 128;
		_newGameX = 20;
		_newGameX2 = _newGameX + 50;
		_modePuzzleY = 70;
		_modeActionY = 160;
		_optionsX = 2 * 8;
		_optionsY = 6 * 8;
		_vortSaveX = 80;
		_vortSaveTextX = _vortSaveX + 40;
		_vortSaveY = 220;
		_saveSlotX = 8;
		_saveSlotY = 68;
		_quitX = 0;
		_quitY = 0;
		_quitNoX1 = (146 - 16);
		_quitNoX2 = (211 - 16);
		_quitNoY1 = (287);
		_quitNoY2 = (312);
		_quitYesX1 = (65 - 16);
		_quitYesX2 = (130 - 16);
		_quitYesY1 = (287);
		_quitYesY2 = (312);
		_controlX = 0;
		_controlY = 0;
		_controlUpX = 0;
		_controlUpY = 0;
		_controlDownX = 0;
		_controlDownY = 0;
		_controlLeftX = 0;
		_controlLeftY = 0;
		_controlRightX = 0;
		_controlRightY = 0;
		_controlUseX = 0;
		_controlUseY = 0;
		_controlWidth = 0;
		_controlHeight = 0;
		_assignX = 0;
		_assignY = 0;
		_backoutX = 0;
		_backoutY = 0;
		_warpX = 0;
		_warpY = 4;
	} else {
		_menuX = 48 * 8;
		_menuY = 80;
		_menuItemWidth = 88;
		_menuItemHeight = 40;
		_mResumeY = 48 * 4;
		_mQuitY = 48 * 3;
		_mOptionsY = 48 * 2;
		_mLoadY = 48 * 1;
		_mControlsY = 270;
		_menuExitXLeft = 60;
		_menuExitY = (g_hdb->_screenHeight - 80);
		_menuExitYTop = 48;
		_menuVortSaveY = 220;
		_menuVortSaveX = 76;
		_mRocketX = 18 * 8;
		_mRocketY = 48;
		_mRocketYBottom = 28 * 8;
		_mRocketEXHX = 6 * 8 - 4;
		_mRocketEXHX2 = 11 * 8 - 4;
		_mTitleY = 34 * 8;
		_oohOhhX = 256;
		_oohOhhY = 224;
		_newGameX = 220;
		_newGameX2 = _newGameX + 50;
		_modePuzzleY = 130;
		_modeActionY = 220;
		_optionsX = 26 * 8;
		_optionsY = 10 * 8;
		_vortSaveX = (200 - 44);
		_vortSaveTextX = _vortSaveX + 40;
		_vortSaveY = 72;
		_saveSlotX = 200;
		_saveSlotY = 80;
		_quitX = 0;
		_quitY = 0;
		_quitYesX1 = (30 * 8 + _quitX);
		_quitYesX2 = (38 * 8 + 4 + _quitX);
		_quitYesY1 = (50 * 8 + _quitY);
		_quitYesY2 = (53 * 8 + 4 + _quitY);
		_quitNoX1 = (41 * 8 + _quitX);
		_quitNoX2 = (49 * 8 + 4 + _quitX);
		_quitNoY1 = (50 * 8 + _quitY);
		_quitNoY2 = (53 * 8 + 4 + _quitY);
		_controlX = 60;
		_controlY = 40;
		_controlUpX = (276 + _controlX);
		_controlUpY = (114 + _controlY);
		_controlDownX = (276 + _controlX);
		_controlDownY = (129 + _controlY);
		_controlLeftX = (396 + _controlX);
		_controlLeftY = (114 + _controlY);
		_controlRightX = (396 + _controlX);
		_controlRightY = (129 + _controlY);
		_controlUseX = (330 + _controlX);
		_controlUseY = (209 + _controlY);
		_controlWidth = 32;
		_controlHeight = 16;
		_assignX = (_controlX + 20 * 8);
		_assignY = (_controlY + 150);
		_backoutX = 0;
		_backoutY = g_hdb->_screenHeight - 32;
		_warpBackoutX = _backoutX;
		_warpBackoutY = _backoutY;
		_warpX = 214;
		_warpY = 160;
	}

	_starWarp = 0;
	_rocketEx = 0;
	_titleActive = false;
	_menuActive = false;
	_optionsActive = false;
	_gamefilesActive = false;
	_newgameActive = false;
	_warpActive = false;
	_optionsScrolling = false;
	_optionsScrollX = _menuX;
	_rocketX = _mRocketX;
	_sayHDB = false;
	_menuKey = 0;

	_nextScreen = 0;

	_nebulaY = 0;		// Used as a flag
	_fStars[0].y = 0;	// Used as a flag

	_gCheckEmpty = nullptr;
	_gCheckOff = nullptr;
	_gCheckOn = nullptr;
	_gCheckLeft = nullptr;
	_gCheckRight = nullptr;

	_contArrowUp = nullptr;
	_contArrowDown = nullptr;
	_contArrowLeft = nullptr;
	_contArrowRight = nullptr;
	_contAssign = nullptr;

	_warpPlaque = nullptr;
	_hdbLogoScreen = nullptr;

	_titleScreen = nullptr;
	_oohOohGfx = nullptr;
	_newGfx = nullptr;
	_loadGfx = nullptr;
	_optionsGfx = nullptr;
	_quitGfx = nullptr;
	_resumeGfx = nullptr;
	_slotGfx = nullptr;
	_rocketMain = nullptr;
	_rocketSecond = nullptr;
	_rocketEx1 = nullptr;
	_rocketEx2 = nullptr;
	_titleLogo = nullptr;
	_hdbLogoScreen = nullptr;
	for (int i = 0; i < kNebulaCount; i++)
		_nebulaGfx[i] = nullptr;

	_sliderLeft = nullptr;
	_sliderMid = nullptr;
	_sliderRight = nullptr;
	_sliderKnob = nullptr;
	_modePuzzleGfx = nullptr;
	_modeActionGfx = nullptr;
	_modeLoadGfx = nullptr;
	_modeSaveGfx = nullptr;
	_menuBackoutGfx = nullptr;
	_menuBackspaceGfx = nullptr;

	_controlButtonGfx = nullptr;

	_controlsGfx = nullptr;
	_screenshots1gfx = nullptr;
	_screenshots1agfx = nullptr;
	_screenshots2gfx = nullptr;
	_demoPlaqueGfx = nullptr;

	_vortexian[0] = _vortexian[1] = _vortexian[2] = nullptr;

	_star[0] = _star[1] = _star[2] = nullptr;

	// secret stars
	_starRedGfx[0] = _starRedGfx[1] = nullptr;
	_starGreenGfx[0] = _starGreenGfx[1] = nullptr;
	_starBlueGfx[0] = _starBlueGfx[1] = nullptr;

	_versionGfx = nullptr;
	_warpGfx = nullptr;

	_warpBackoutX = 0;
	_warpBackoutY = 0;
	_titleCycle = 0;
	_titleDelay = 0;
	_resumeSong = SONG_NONE;
	_rocketY = 0;
	_rocketYVel = 0;
	_nebulaX = 0;
	_nebulaYVel = 0;
	_nebulaWhich = 0;
	_quitScreen = nullptr;
	_quitTimer = 0;
	_handangoGfx = nullptr;
	_clickDelay = 0;
	_saveSlot = 0;
	_quitActive = 0;
	_optionsXV = 0;
	_oBannerY = 0;
	_introSong = SONG_NONE;
	_titleSong = SONG_NONE;
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

	if (g_hdb->isPPC()) {
		if (g_hdb->isDemo()) {
			_hdbLogoScreen = g_hdb->_gfx->loadPic(DEMOTITLESCREEN);
			if (g_hdb->isHandango())
				_handangoGfx = g_hdb->_gfx->loadPic(PIC_HANDANGO);
		}
		else
			_hdbLogoScreen = g_hdb->_gfx->loadPic(TITLESCREEN);
	} else {
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
	}

	return true;
}

void Menu::readConfig() {
	bool needFlush = false;

	if (ConfMan.hasKey(CONFIG_MSTONE7)) {
		g_hdb->setStarsMonkeystone7(ConfMan.getInt(CONFIG_MSTONE7));
	} else {
		ConfMan.setInt(CONFIG_MSTONE7, STARS_MONKEYSTONE_7_FAKE);
		needFlush = true;
	}

	if (ConfMan.hasKey(CONFIG_MSTONE14)) {
		g_hdb->setStarsMonkeystone14(ConfMan.getInt(CONFIG_MSTONE14));
	} else {
		ConfMan.setInt(CONFIG_MSTONE14, STARS_MONKEYSTONE_14_FAKE);
		needFlush = true;
	}

	if (ConfMan.hasKey(CONFIG_MSTONE21)) {
		g_hdb->setStarsMonkeystone21(ConfMan.getInt(CONFIG_MSTONE21));
	} else {
		ConfMan.setInt(CONFIG_MSTONE21, STARS_MONKEYSTONE_21_FAKE);
		needFlush = true;
	}

	if (ConfMan.hasKey(CONFIG_CHEAT)) {
		g_hdb->setCheatingOn();
		debug("Cheating enabled");
	}

	if (needFlush)
		ConfMan.flushToDisk();
}

void Menu::writeConfig() {
	int value;

	value = g_hdb->getStarsMonkeystone7();
	ConfMan.setInt(CONFIG_MSTONE7, value);
	value = g_hdb->getStarsMonkeystone14();
	ConfMan.setInt(CONFIG_MSTONE14, value);
	value = g_hdb->getStarsMonkeystone21();
	ConfMan.setInt(CONFIG_MSTONE21, value);

	if (g_hdb->getCheatingOn())
		ConfMan.set(CONFIG_CHEAT, "1");

	ConfMan.flushToDisk();
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
	_menuBackspaceGfx = g_hdb->_gfx->loadPic(MENU_BACKSPACE);
	_menuBackoutGfx = g_hdb->_gfx->loadPic(MENU_BACK);
	if (g_hdb->isPPC()) {
		_warpBackoutX = (g_hdb->_screenWidth - _menuBackoutGfx->_width);
		_warpBackoutY = (g_hdb->_screenHeight - _menuBackoutGfx->_height);
	}
	_controlButtonGfx = g_hdb->_gfx->loadPic(MENU_CONTROLS);
	_controlsGfx = g_hdb->_gfx->loadPic(PIC_CONTROLSSCREEN);

	if (g_hdb->isDemo()) {

		if (!g_hdb->isPPC()) {
			_screenshots1gfx = g_hdb->_gfx->loadPic(PIC_DEMOSCREEN);
			_screenshots1agfx = g_hdb->_gfx->loadPic(PIC_DEMOSCREEN2);
			_screenshots2gfx = g_hdb->_gfx->loadPic(PIC_DEMO_BUY);
			_demoPlaqueGfx =  g_hdb->_gfx->loadPic(PIC_DEMO);
		} else {
			_screenshots1agfx = g_hdb->_gfx->loadPic("pic_demoscreenshots");
			_screenshots1gfx = g_hdb->_gfx->loadPic("pic_demoscreenshots2");
			_screenshots2gfx = nullptr;
			_demoPlaqueGfx =  nullptr;
		}
	}

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
			_fStars[i].x = g_hdb->_rnd->getRandomNumber(g_hdb->_screenWidth - 1);
			_fStars[i].speed = g_hdb->_rnd->getRandomNumber(4) + 1;
			_fStars[i].anim = g_hdb->_rnd->getRandomNumber(2);
			_fStars[i].delay = 5;
		}
	}

	_quitScreen = nullptr;

	// did we skip the intro?
	if (!_nebulaY) {
		g_hdb->_gfx->setup3DStars();	// setup the star info

		_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount - 1);
		_nebulaX = g_hdb->_rnd->getRandomNumber(g_hdb->_screenWidth - 1) + 10;
		_nebulaY = -20;
		_nebulaYVel = g_hdb->_rnd->getRandomNumber(9) + 2;
	}

	_optionsScrollX = _menuX;
	_oBannerY = -48;
	_rocketY = _mRocketY;
	_rocketX = _mRocketX;
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

		if (!g_hdb->isPPC()) {
			// draw version #
			_versionGfx->drawMasked(g_hdb->_screenWidth - 6 * 8, g_hdb->_screenHeight - 8);

			if (g_hdb->isDemo()) {
				_demoPlaqueGfx->drawMasked(g_hdb->_screenWidth / 2 - _demoPlaqueGfx->_width / 2, 2);
			}
		}

		//
		// see if the Options/GameFiles menu has scrolled off
		//
		if (_optionsScrolling) {
			_optionsScrollX += _optionsXV;
			_rocketX += -_optionsXV;
			_oBannerY += _optionsXV / 3;
			_optionsXV += 3;
			if (_optionsScrollX > g_hdb->_screenWidth + 10) {
				switch (_nextScreen) {
				case 0:
					_optionsActive = true;
					break;
				case 1:
					_gamefilesActive = 1;
					break;
				case 2:
					_newgameActive = true;
					break;
				default:
					break;
				}

				_oBannerY = 0;
				_optionsScrolling = false;
				_menuActive = false;
			}
		}

		// Draw the Secret Stars! (tm)
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
			_warpGfx->drawMasked(0, g_hdb->_screenHeight - _warpGfx->_height);
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
			if (_optionsScrollX < _menuX) {
				_optionsScrollX = _menuX;
				_rocketX = _mRocketX;
				_oBannerY = -48;
				_optionsScrolling = false;
				_newgameActive = false;
				_menuActive = true;
			}

			drawRocketAndSelections();
		} else {
			drawNebula();
			_newGfx->drawMasked(centerPic(_newGfx), _oBannerY);

			_modePuzzleGfx->drawMasked(_newGameX, _modePuzzleY);
			_modePuzzleGfx->drawMasked(_newGameX, _modeActionY);

			g_hdb->_gfx->setCursor(_newGameX2, _modePuzzleY - 10);
			g_hdb->_gfx->drawText("PUZZLE MODE");
			g_hdb->_gfx->setCursor(_newGameX2, _modePuzzleY + 10);
			g_hdb->_gfx->drawText("In this mode, the focus");
			g_hdb->_gfx->setCursor(_newGameX2, _modePuzzleY + 22);
			g_hdb->_gfx->drawText("is on solving puzzles and");
			g_hdb->_gfx->setCursor(_newGameX2, _modePuzzleY + 34);
			g_hdb->_gfx->drawText("avoiding enemies.");

			g_hdb->_gfx->setCursor(_newGameX2, _modeActionY - 10);
			g_hdb->_gfx->drawText("ACTION MODE");
			g_hdb->_gfx->setCursor(_newGameX2, _modeActionY + 10);
			g_hdb->_gfx->drawText("In this mode, the focus");
			g_hdb->_gfx->setCursor(_newGameX2, _modeActionY + 22);
			g_hdb->_gfx->drawText("is on solving puzzles and");
			g_hdb->_gfx->setCursor(_newGameX2, _modeActionY + 34);
			g_hdb->_gfx->drawText("attacking enemies!");

			// title logo
			_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + _mTitleY);
			_menuBackoutGfx->drawMasked(_backoutX, g_hdb->_menu->_backoutY);
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
			if (_optionsScrollX < _menuX) {
				_optionsScrollX = _menuX;
				_rocketX = _mRocketX;
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

			g_hdb->_gfx->setCursor(_optionsX + kOptionSPC, _optionsY);
			if (!g_hdb->_sound->getMusicVolume())
				g_hdb->_gfx->drawText("Music OFF");
			else
				g_hdb->_gfx->drawText("Music Volume");

			drawSlider(_optionsX, _optionsY + 20, g_hdb->_sound->getMusicVolume());

			g_hdb->_gfx->setCursor(_optionsX + kOptionSPC, _optionsY + kOptionLineSPC * 2);
			if (!g_hdb->_sound->getSFXVolume())
				g_hdb->_gfx->drawText("Sound Effects OFF");
			else
				g_hdb->_gfx->drawText("Sound Effects Volume");

			drawSlider(_optionsX, _optionsY + kOptionLineSPC * 2 + 20, g_hdb->_sound->getSFXVolume());

			if (!g_hdb->isPPC()) {
				// Voices ON or OFF
				drawToggle(_optionsX, _optionsY + kOptionLineSPC * 4 + 20, g_hdb->_sound->getVoiceStatus());

				if (!g_hdb->isVoiceless()) {
					g_hdb->_gfx->setCursor(_optionsX + kOptionSPC + 24, _optionsY + kOptionLineSPC * 4 + 24);
					if (!g_hdb->_sound->getVoiceStatus())
						g_hdb->_gfx->drawText("Voice Dialogue OFF");
					else
						g_hdb->_gfx->drawText("Voice Dialogue ON");
				}
			}

			// title logo
			_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + _mTitleY);
			_menuBackoutGfx->drawMasked(_backoutX, g_hdb->_menu->_backoutY);

			// Ignore Controls Screen Button
			//_controlButtonGfx->drawMasked(centerPic(_controlButtonGfx), _mControlsY);
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
			if (_optionsScrollX < _menuX) {
				_optionsScrollX = _menuX;
				_rocketX = _mRocketX;
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
			_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + _mTitleY);
			// CHOOSE SLOT screen
			_modeLoadGfx->drawMasked(centerPic(_modeLoadGfx), _oBannerY);
			_menuBackoutGfx->drawMasked(_backoutX, g_hdb->_menu->_backoutY);

			if (!g_hdb->isPPC()) {
				if (_saveGames[kAutoSaveSlot].seconds)
					_vortexian[anim]->drawMasked(_vortSaveX, _vortSaveY);
			}

			if (g_hdb->isPPC()) {
				g_hdb->_gfx->setCursor(_vortSaveTextX, _vortSaveY);
				g_hdb->_gfx->drawText("Last Vortexian");
				g_hdb->_gfx->setCursor(_vortSaveTextX, _vortSaveY + 12);
				g_hdb->_gfx->drawText("Saved Game");
			}

			if (anim_time < g_hdb->getTimeSlice()) {
				anim_time = g_hdb->getTimeSlice() + 50;
				anim++;
				if (anim > 2)
					anim = 0;
			}

			for (int i = 0; i < kNumSaveSlots; i++) {
				int seconds = _saveGames[i].seconds;

				_slotGfx->drawMasked(_saveSlotX - 8, i * 32 + (_saveSlotY - 4));
				if (seconds || _saveGames[i].mapName[0]) {

					g_hdb->_gfx->setTextEdges(0, g_hdb->_screenWidth + 60, 0, g_hdb->_screenHeight);
					g_hdb->_gfx->setCursor(_saveSlotX, i * 32 + _saveSlotY);
					g_hdb->_gfx->drawText(_saveGames[i].mapName);

					g_hdb->_gfx->setCursor(_saveSlotX + 180, i * 32 + _saveSlotY);
					Common::String buff = Common::String::format("%02d:%02d", seconds / 3600, (seconds / 60) % 60);
					g_hdb->_gfx->drawText(buff.c_str());
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
		_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + _mTitleY);
		_menuBackoutGfx->drawMasked(_warpBackoutX, g_hdb->_menu->_warpBackoutY);

		Common::String textString;
		for (int i = 0; i < 10; i++) {
			textString = Common::String::format("Map %2d", i);
			g_hdb->_gfx->setCursor(_warpX + 4, i * 16 + _warpY);
			g_hdb->_gfx->drawText(textString.c_str());
		}
		for (int i = 0; i < 10; i++) {
			textString = Common::String::format("Map %d", i + 10);
			g_hdb->_gfx->setCursor(_warpX + 80, i * 16 + _warpY);
			g_hdb->_gfx->drawText(textString.c_str());
		}
		for (int i = 0; i < 10; i++) {
			textString = Common::String::format("Map %d", i + 20);
			g_hdb->_gfx->setCursor(_warpX + 160, i * 16 + _warpY);
			g_hdb->_gfx->drawText(textString.c_str());
		}

		if (_warpActive > 1) {
			g_hdb->_gfx->setCursor(_warpX + 60, _warpY + 164);
			textString = Common::String::format("Warping to MAP%d", _warpActive - 2);
			g_hdb->_gfx->centerPrint(textString.c_str());
		}
	} else if (_quitActive) {
		//-------------------------------------------------------------------
		//	DRAW QUIT SCREEN
		//-------------------------------------------------------------------
		g_hdb->_gfx->draw3DStars();
		drawNebula();

		if (_quitActive == 3 || !g_hdb->isDemo()) {
			if (!_quitScreen)
				_quitScreen = g_hdb->_gfx->loadPic(PIC_QUITSCREEN);
			_quitScreen->drawMasked(_quitX, _quitY);
		} else if (_quitActive == 1) {
			_screenshots1agfx->drawMasked(_quitX, _quitY);
		} else if (_quitActive == 2) { // XXXX
			_screenshots1gfx->drawMasked(_quitX, _quitY);

			if (!g_hdb->isPPC())
				_screenshots2gfx->drawMasked(_quitX, g_hdb->_screenHeight - _screenshots2gfx->_height);
		}
	}
}

void Menu::freeMenu() {
	// title sequence stuff
	if (_titleScreen)
		delete _titleScreen;
	_titleScreen = nullptr;
	if (_oohOohGfx)
		delete _oohOohGfx;
	_oohOohGfx = nullptr;

	if (_newGfx)
		delete _newGfx;
	_newGfx = nullptr;
	if (_loadGfx)
		delete _loadGfx;
	_loadGfx = nullptr;
	if (_optionsGfx)
		delete _optionsGfx;
	_optionsGfx = nullptr;
	if (_quitGfx)
		delete _quitGfx;
	_quitGfx = nullptr;
	if (_resumeGfx)
		delete _resumeGfx;
	_resumeGfx = nullptr;
	if (_slotGfx)
		delete _slotGfx;
	_slotGfx = nullptr;
	if (_rocketMain)
		delete _rocketMain;
	_rocketMain = nullptr;
	if (_rocketSecond)
		delete _rocketSecond;
	_rocketSecond = nullptr;
	if (_rocketEx1)
		delete _rocketEx1;
	_rocketEx1 = nullptr;
	if (_rocketEx2)
		delete _rocketEx2;
	_rocketEx2 = nullptr;
	if (_titleLogo)
		delete _titleLogo;
	_titleLogo = nullptr;

	if (_hdbLogoScreen)
		delete _hdbLogoScreen;
	_hdbLogoScreen = nullptr;

	if (_screenshots1gfx)
		delete _screenshots1gfx;
	_screenshots1gfx = nullptr;
	if (_screenshots1agfx)
		delete _screenshots1agfx;
	_screenshots1agfx = nullptr;
	if (_screenshots2gfx)
		delete _screenshots2gfx;
	_screenshots2gfx = nullptr;
	if (_demoPlaqueGfx)
		delete _demoPlaqueGfx;
	_demoPlaqueGfx = nullptr;

	if (g_hdb->isPPC() && g_hdb->isHandango()) {
		if (_handangoGfx)
			delete _handangoGfx;
		_handangoGfx = nullptr;
	}

	if (_nebulaGfx[0]) {
		for (int i = 0; i < kNebulaCount; i++) {
			delete _nebulaGfx[i];
			_nebulaGfx[i] = nullptr;
		}
	}

	if (_sliderLeft)
		delete _sliderLeft;
	_sliderLeft = nullptr;
	if (_sliderMid)
		delete _sliderMid;
	_sliderMid = nullptr;
	if (_sliderRight)
		delete _sliderRight;
	_sliderRight = nullptr;
	if (_sliderKnob)
		delete _sliderKnob;
	_sliderKnob = nullptr;
	if (_modePuzzleGfx)
		delete _modePuzzleGfx;
	_modePuzzleGfx = nullptr;
	if (_modeActionGfx)
		delete _modeActionGfx;
	_modeActionGfx = nullptr;
	if (_modeLoadGfx)
		delete _modeLoadGfx;
	_modeLoadGfx = nullptr;
	if (_modeSaveGfx)
		delete _modeSaveGfx;
	_modeSaveGfx = nullptr;
	if (_menuBackoutGfx)
		delete _menuBackoutGfx;
	_menuBackoutGfx = nullptr;
	if (_menuBackspaceGfx)
		delete _menuBackspaceGfx;
	_menuBackspaceGfx = nullptr;

	if (_controlButtonGfx)
		delete _controlButtonGfx;
	_controlButtonGfx = nullptr;

	if (_controlsGfx)
		delete _controlsGfx;
	_controlsGfx = nullptr;

	if (_vortexian[0]) {
		delete _vortexian[0];
		delete _vortexian[1];
		delete _vortexian[2];
		_vortexian[0] = _vortexian[1] = _vortexian[2] = nullptr;
	}

	if (_star[0]) {
		delete _star[0];
		delete _star[1];
		delete _star[2];
		_star[0] = _star[1] = _star[2] = nullptr;
	}

	// secret stars
	if (_starRedGfx[0]) {
		delete _starRedGfx[0];
		delete _starRedGfx[1];
		delete _starGreenGfx[0];
		delete _starGreenGfx[1];
		delete _starBlueGfx[0];
		delete _starBlueGfx[1];
		_starRedGfx[0] = _starRedGfx[1] = nullptr;
		_starGreenGfx[0] = _starGreenGfx[1] = nullptr;
		_starBlueGfx[0] = _starBlueGfx[1] = nullptr;
	}

	if (_versionGfx)
		delete _versionGfx;
	_versionGfx = nullptr;

	if (_warpGfx)
		delete _warpGfx;
	_warpGfx = nullptr;
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
		_oohOohGfx->draw(_oohOhhX, _oohOhhY);
		break;

	case 4: // fadeout monkeystone logo
		_titleScreen->draw(0, 0);
		break;

	case 20: // fadein Handango title screen
	case 21: // wait
	case 22: // fadeout Handango title screen
	case 23:
		_handangoGfx->draw(0, 0);
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
		if (_nebulaY > g_hdb->_screenHeight + (g_hdb->_screenHeight / 2)) {
			_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount - 1);
			_nebulaX = g_hdb->_rnd->getRandomNumber(g_hdb->_screenWidth - 1) + 10;
			_nebulaY = -11 * 8;
			_nebulaYVel = g_hdb->_rnd->getRandomNumber(3) + 1;
			if (_nebulaWhich > 4)		// galaxy?
				_nebulaYVel = 1;
		}

		// draw rocket
		_rocketMain->drawMasked(_mRocketX, _rocketY);
		_rocketSecond->drawMasked(_mRocketX + 40, _rocketY + _mRocketYBottom);

		// exhaust
		if (_rocketEx < 5) {
			_rocketEx1->drawMasked(_mRocketX + _mRocketEXHX, _rocketY + _mRocketYBottom);
			if (!g_hdb->isPPC())
				_rocketEx2->drawMasked(_mRocketX + _mRocketEXHX2, _rocketY + _mRocketYBottom);
		} else if (_rocketEx >= 5 && _rocketEx < 10) {
			_rocketEx2->drawMasked(_mRocketX + _mRocketEXHX, _rocketY + _mRocketYBottom);
			if (!g_hdb->isPPC())
				_rocketEx1->drawMasked(_mRocketX + _mRocketEXHX2, _rocketY + _mRocketYBottom);
		} else {
			_rocketEx = 0;
			_rocketEx1->drawMasked(_mRocketX + _mRocketEXHX, _rocketY + _mRocketYBottom);
			if (!g_hdb->isPPC())
				_rocketEx2->drawMasked(_mRocketX + _mRocketEXHX2, _rocketY + _mRocketYBottom);
		}
		_rocketEx++;

		// title logo
		_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + _mTitleY);

		break;

	default:
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

		if (!g_hdb->isHandango())
			_titleCycle++;
		else
			_titleCycle = 20;
	break;

	//-------------------------------------------------------------------
	// HANDANGO LOGO
	//-------------------------------------------------------------------
	case 20:        // fadein handango screen
		if (g_hdb->_gfx->isFadeActive())
			break;
		_titleDelay = g_system->getMillis() + 750;
		_titleCycle++;
		break;

	case 21:        // wait
		g_hdb->_gfx->setFade(false, false, kScreenFade / time);                // FADEOUT
		_titleCycle++;
		break;

	case 22:        // fadeout handango screen
		if (g_hdb->_gfx->isFadeActive())
			break;
		_titleCycle++;
		break;

	case 23:        // fadein HDB title screen
		if (g_hdb->_gfx->isFadeActive())
			break;
		g_hdb->_gfx->setFade(true, false, kScreenFade / time);         // FADEIN
		_titleCycle = 5;
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
			_rocketY = g_hdb->_screenHeight;	// ycoord
			_rocketYVel = -1;		// yspeed
			_rocketEx = 0;			// exhaust toggle
			g_hdb->_gfx->setup3DStars();	// setup the star info

			_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount - 1);
			_nebulaX = g_hdb->_rnd->getRandomNumber(g_hdb->_screenWidth - 1) + 10;
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
		if (_rocketY < _mRocketY) {
			_rocketY = _mRocketY;
			_titleCycle = 12;
			_titleDelay = 1;
		}

		break;

		// shut down title....start up menu!
	case 12:
		_titleActive = false;
		g_hdb->changeGameState();
		break;

	default:
		break;
	}
}

void Menu::fillSavegameSlots() {
	Common::String saveGameFile;

	for (int i = 0; i < kNumSaveSlots; i++) {
		saveGameFile = g_hdb->genSaveFileName(i, false);
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(saveGameFile);

		if (!in) {
			_saveGames[i].seconds = 0;
			_saveGames[i].saveID[0] = 0;
			_saveGames[i].mapName[0] = 0;
		} else {
			Graphics::skipThumbnail(*in);

			Common::strlcpy(_saveGames[i].saveID, saveGameFile.c_str(), sizeof(_saveGames[0].saveID));
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
		if (x >= _menuX && x < _menuX + _menuItemWidth &&
			y >= _menuY + _mQuitY && y < _menuY + _mQuitY + _menuItemHeight) {
			g_hdb->_sound->playSound(SND_BYE);
			_quitTimer = g_hdb->getTimeSlice() + 1000;
			_quitActive = 1;
			_menuActive = false;
			return;
		} else if (x >= _menuX && x < _menuX + _menuItemWidth &&
			y >= _menuY && y < _menuY + _menuItemHeight) {
			// new game?
			_optionsScrolling = true;
			_optionsXV = 5;
			g_hdb->_sound->playSound(SND_MENU_ACCEPT);
			g_hdb->_sound->freeSound(SND_HDB);
			_nextScreen = 2;
		} else if (x >= _menuX && x < _menuX + _menuItemWidth &&
			y >= _menuY + _mLoadY && y < _menuY + _mLoadY + _menuItemHeight) {
			// game files?
				_optionsScrolling = true;
				_optionsXV = 5;
				g_hdb->_sound->playSound(SND_MENU_ACCEPT);
				_nextScreen = 1;
		} else if (x >= _menuX && x < _menuX + _menuItemWidth &&
			y >= _menuY + _mOptionsY && y < _menuY + _mOptionsY + _menuItemHeight) {
			// options?
			SoundType temp;
			temp = g_hdb->_sound->whatSongIsPlaying();
			if (temp != SONG_NONE)
				_resumeSong = temp;
			_optionsScrolling = true;
			_optionsXV = 5;
			_nextScreen = 0;
			g_hdb->_sound->playSound(SND_MENU_ACCEPT);
		} else if (((x >= _menuX && x < _menuX + _menuItemWidth &&
			y >= _menuY + _mResumeY && y < _menuY + _mResumeY + _menuItemHeight) || resume) &&
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
			open = (y > g_hdb->_menu->_menuExitY && x < _menuExitXLeft);

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

		if (y >= g_hdb->_menu->_menuExitY || y < _menuExitYTop || xit) {
			_optionsScrolling = true;
			_optionsXV = -5;
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
		}

		// PUZZLE MODE area
		if (y >= _modePuzzleY - 10 && y <= _modeActionY - 10) {
			g_hdb->setActionMode(0);
			g_hdb->_sound->playSound(SND_MENU_ACCEPT);
			_newgameActive = false;
			g_hdb->changeGameState();
			// that's it!  the Game Loop takes over from here...
		} else if (y >= _modeActionY - 10 && y <= g_hdb->_menu->_menuExitY) {
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
		if (x >= 0 && x <= _optionsX + 200 &&
			y >= _optionsY + 20 && y <= _optionsY + 36) {
			int oldVol = g_hdb->_sound->getMusicVolume();
			if (x < _optionsX) {
				if (oldVol) {
					g_hdb->_sound->stopMusic();
					g_hdb->_sound->setMusicVolume(0);
					g_hdb->_sound->playSound(SND_GUI_INPUT);
				}
			} else {
				offset = ((x - _optionsX) * 256) / 200;
				g_hdb->_sound->setMusicVolume(offset);
				if (!oldVol)
					g_hdb->_sound->startMusic(_resumeSong);
			}
		} else if (x >= 0 && x <= _optionsX + 200 &&
			y >= _optionsY + kOptionLineSPC * 2 + 20 && y <= _optionsY + kOptionLineSPC * 2 + 36) {
			// Slider 2
			if (x >= _optionsX)
				offset = ((x - _optionsX) * 256) / 200;
			else
				offset = 0;
			g_hdb->_sound->setSFXVolume(offset);
			g_hdb->_sound->playSound(SND_MENU_SLIDER);
		} else if (x >= _optionsX && x <= _optionsX + 200 &&
			y >= _optionsY + kOptionLineSPC * 4 + 24 && y <= _optionsY + kOptionLineSPC * 4 + 40) {
			// Voices ON/OFF
			if (!g_hdb->isVoiceless()) {
				bool value = g_hdb->_sound->getVoiceStatus();
				value ^= true;
				g_hdb->_sound->setVoiceStatus(value);
				g_hdb->_sound->playSound(SND_GUI_INPUT);
			}
		} else if (y >= g_hdb->_menu->_menuExitY || y < _menuExitYTop || xit) {
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
			_optionsScrolling = true;
			_optionsXV = -5;
		} else if (x >= (g_hdb->_screenWidth / 2 - _controlButtonGfx->_width / 2) && x < (g_hdb->_screenWidth / 2 + _controlButtonGfx->_width / 2) &&
			y >= _mControlsY && y < _mControlsY + _controlButtonGfx->_height) {
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

		if (y >= g_hdb->_menu->_menuExitY + 15 || y < _menuExitYTop || xit) {
			_optionsScrolling = true;
			_optionsXV = -5;
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
		}

		// Vortexian Load only exists on PocketPC!
		if (g_hdb->isPPC()) {
			// 5 Slots screen
			// Vortexian autosave LOAD?
			if (y > _vortSaveY && (y < _vortSaveY + 32) && (x >= _vortSaveX) && (x < _vortSaveX + 96) && _saveGames[kAutoSaveSlot].seconds) {
				g_hdb->_sound->playSound(SND_VORTEX_SAVE);
				if (g_hdb->loadGameState(kAutoSaveSlot).getCode() == Common::kNoError) {
					_gamefilesActive = false;
					freeMenu();
					g_hdb->setGameState(GAME_PLAY);
					return;
				}
			}
		}

		int i = 0;
		for (; i < kNumSaveSlots; i++)
			if (y >= (i * 32 + _saveSlotY - 4) && y <= (i * 32 + _saveSlotY + 24))
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

		if ((y >= g_hdb->_menu->_menuExitY && x < _menuExitXLeft) || xit) {
			_menuActive = true;
			_warpActive = false;
			g_hdb->_sound->playSound(SND_MENU_BACKOUT);
			_clickDelay = 10;
		} else if (y >= _warpY && y < _warpY + 160) {
			int map;

			if (x > _warpX + 160)
				map = 20;
			else
				if (x > _warpX + 80)
					map = 10;
				else
					map = 0;

			map += (y - _warpY) / 16;

			_warpActive = map + 2;
			g_hdb->paint();
			if (g_hdb->getDebug())
				g_hdb->_gfx->updateVideo();
			_warpActive = 0;

			Common::String mapString = Common::String::format("MAP%02d", map);

			if (g_hdb->isDemo()) {
				mapString += "_DEMO";
			}

			freeMenu();
			g_hdb->setGameState(GAME_PLAY);
			g_hdb->_sound->stopMusic();
			g_hdb->_ai->clearPersistent();
			g_hdb->resetTimer();
			g_hdb->_sound->playSound(SND_POP);
			g_hdb->startMap(mapString.c_str());
		}
	} else if (_quitActive) {
		//-------------------------------------------------------------------
		//	QUIT INPUT
		//-------------------------------------------------------------------
		int	xit = getMenuKey();

		if (!g_hdb->isDemo()) {
			if ((x >= _quitNoX1 && x <= _quitNoX2 && y > _quitNoY1 && y < _quitNoY2 && _quitTimer < g_hdb->getTimeSlice()) || xit) {
				g_hdb->_sound->playSound(SND_MENU_BACKOUT);
				delete _quitScreen;
				_quitScreen = nullptr;

				_menuActive = true;
				_quitActive = 0;
			} else if (_quitTimer < g_hdb->getTimeSlice()) {
				if (x >= _quitYesX1 && x <= _quitYesX2 && y > _quitYesY1 && y < _quitYesY2) {
					writeConfig();
					g_hdb->quitGame();
				}
			}
		} else {
			if ((_quitActive == 1 || _quitActive == 2) && _quitTimer < g_hdb->getTimeSlice()) {
				_quitActive++;
				_quitTimer = g_hdb->getTimeSlice() + 1000;
			} else {
				if (_quitActive == 3 && (x >= _quitNoX1 && x <= _quitNoX2 && y > _quitNoY1 && y < _quitNoY2 && _quitTimer < g_hdb->getTimeSlice())) {
					g_hdb->_sound->playSound(SND_MENU_BACKOUT);
					delete _quitScreen;
					_quitScreen = nullptr;

					_menuActive = true;
					_quitActive = 0;
				} else if (_quitActive == 3 && _quitTimer < g_hdb->getTimeSlice()){
					writeConfig();
					g_hdb->quitGame();
				}
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

	if (_nebulaY > g_hdb->_screenHeight + (g_hdb->_screenHeight / 2)) {
		_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount - 1);
		_nebulaX = g_hdb->_rnd->getRandomNumber(g_hdb->_screenWidth - 1) + 10;
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
		if (_fStars[i].y > g_hdb->_screenHeight) {
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
	case 0:
		_optionsGfx->drawMasked(centerPic(_optionsGfx), _oBannerY);
		break;
	case 1:
		_modeLoadGfx->drawMasked(centerPic(_modeLoadGfx), _oBannerY);
		break;
	case 2:
		_newGfx->drawMasked(centerPic(_newGfx), _oBannerY);
		break;
	default:
		break;
	}

	// menu items
	_newGfx->drawMasked(_optionsScrollX, _menuY);
	_modeLoadGfx->drawMasked(_optionsScrollX, _menuY + _mLoadY);
	_optionsGfx->drawMasked(_optionsScrollX, _menuY + _mOptionsY);
	_quitGfx->drawMasked(_optionsScrollX, _menuY + _mQuitY);
	if (g_hdb->_map->isLoaded() || _saveGames[5].seconds)
		_resumeGfx->drawMasked(_optionsScrollX, _menuY + _mResumeY);

	// draw rocket
	_rocketMain->drawMasked(_rocketX, _rocketY);
	_rocketSecond->drawMasked(_rocketX + 40, _rocketY + _mRocketYBottom);

	// exhaust
	if (_rocketEx < 5) {
		_rocketEx1->drawMasked(_mRocketX + _mRocketEXHX, _rocketY + _mRocketYBottom);
		if (!g_hdb->isPPC()) {
			_rocketEx2->drawMasked(_mRocketX + _mRocketEXHX2, _rocketY + _mRocketYBottom);
		}
	} else if (_rocketEx >= 5 && _rocketEx < 10) {
		_rocketEx2->drawMasked(_mRocketX + _mRocketEXHX, _rocketY + _mRocketYBottom);
		if (!g_hdb->isPPC()) {
			_rocketEx1->drawMasked(_mRocketX + _mRocketEXHX2, _rocketY + _mRocketYBottom);
		}
	} else {
		_rocketEx = 0;
		_rocketEx1->drawMasked(_mRocketX + _mRocketEXHX, _rocketY + _mRocketYBottom);
		if (!g_hdb->isPPC()) {
			_rocketEx2->drawMasked(_mRocketX + _mRocketEXHX2, _rocketY + _mRocketYBottom);
		}
	}
	_rocketEx++;

	// title logo
	_titleLogo->drawMasked(centerPic(_titleLogo), _rocketY + _mTitleY);
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
	if (g_hdb->isPPC()) {
		g_hdb->_gfx->setCursor(0, _warpY + 176);
		g_hdb->_gfx->centerPrint("MONKEYSTONE WARP ZONE!");
	} else
		_warpPlaque->drawMasked(centerPic(_warpPlaque), 64);
}

} // End of Namespace
