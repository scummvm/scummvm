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
#include "hdb/menu.h"

namespace HDB {

bool Menu::init() {
//	_starWarp = 0;
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

	_nebulaY = 0;		// Used as a flag
	_fStars[0].y = 0;	// Used as a flag

	_keyAssignUp = Common::KEYCODE_UP;
	_keyAssignDown = Common::KEYCODE_DOWN;
	_keyAssignLeft = Common::KEYCODE_LEFT;
	_keyAssignRight = Common::KEYCODE_RIGHT;
	_keyAssignUse = Common::KEYCODE_RETURN;

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

	_waitingForKey = false;
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

void Menu::startMenu() {
	debug(9, "STUB: Start Menu");
}

void Menu::changeToMenu() {
	warning("STUB: Change To Menu");
}

void Menu::drawMenu() {
	warning("STUB: Draw Menu");
}

void Menu::freeMenu() {
	debug(9, "STUB: Free Menu");
}

char nebulaNames[kNebulaCount][32] = {
	BACKSCROLL_PLANET1,
	BACKSCROLL_PLANET2,
	BACKSCROLL_PLANET3,
	BACKSCROLL_PLANET4,
	BACKSCROLL_PLANET5,
	BACKSCROLL_GALAXY1,
	BACKSCROLL_GALAXY2
};

bool Menu::startTitle() {
	// Defaults the game into Action Mode
	warning("REMOVE: Putting this here since Menu hasn't been implemented yet");
	g_hdb->setActionMode(1);
	g_hdb->setGameState(GAME_PLAY);

	int i;

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

	for (i = 0; i < kNebulaCount; i++)
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
	static int	line;
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
			_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount);
			_nebulaX = g_hdb->_rnd->getRandomNumber(kScreenWidth) + 10;
			_nebulaY = -11 * 8;
			_nebulaYVel = g_hdb->_rnd->getRandomNumber(4) + 1;
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

			_nebulaWhich = g_hdb->_rnd->getRandomNumber(kNebulaCount);
			_nebulaX = g_hdb->_rnd->getRandomNumber(kScreenWidth) + 10;
			_nebulaY = -11 * 8;
			_nebulaYVel = g_hdb->_rnd->getRandomNumber(10) + 2;
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
	int i;
	int max = kNumSaveSlots;

	Common::InSaveFile *in;
	Common::String saveGameFile;

	for (i = 0; i < max; i++) {
		saveGameFile = Common::String::format("%s.%03d", g_hdb->getTargetName()->c_str(), i);
		in = g_system->getSavefileManager()->openForLoading(saveGameFile);

		if (!in) {
			memset(&_saveGames[i], 0, sizeof(Save));
		} else {
			strcpy(_saveGames[i].saveID, saveGameFile.c_str());
			_saveGames[i].seconds = in->readUint32LE();
			in->read(_saveGames[i].mapName, 32);
			delete in;
		}
		_saveGames[i].fileSlot = i + 1;
	}
}

} // End of Namespace
