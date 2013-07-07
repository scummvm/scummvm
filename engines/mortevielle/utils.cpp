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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "common/scummsys.h"
#include "graphics/cursorman.h"

#include "mortevielle/dialogs.h"
#include "mortevielle/menu.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/speech.h"

namespace Mortevielle {

/**
 * Check is a key was pressed
 * It also delays the engine and check if the screen has to be updated
 * @remarks	Originally called 'keypressed'
 */
bool MortevielleEngine::keyPressed() {
	// Check for any pending key presses
	handleEvents();

	// Check if it's time to draw the next frame
	if (g_system->getMillis() > (_lastGameFrame + GAME_FRAME_DELAY)) {
		_lastGameFrame = g_system->getMillis();

		_screenSurface.updateScreen();

		_debugger.onFrame();
	}

	// Delay briefly to keep CPU usage down
	g_system->delayMillis(5);

	// Return if there are any pending key presses
	return !_keypresses.empty();
}

/**
 * Wait for a keypress
 * @remarks	Originally called 'get_ch'
 */
int MortevielleEngine::getChar() {
	// If there isn't any pending keypress, wait until there is
	while (!shouldQuit() && _keypresses.empty()) {
		keyPressed();
	}

	// Return the top keypress
	return shouldQuit() ? 0 : _keypresses.pop();
}

/**
 * Handle pending events
 * @remarks		Since the ScummVM screen surface is double height to handle 640x200 using 640x400,
 * the mouse Y position is divided by 2 to keep the game thinking the Y goes from 0 - 199
 */
bool MortevielleEngine::handleEvents() {
	Common::Event event;
	if (!g_system->getEventManager()->pollEvent(event))
		return false;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_MOUSEMOVE:
		_mousePos = Common::Point(event.mouse.x, event.mouse.y / 2);
		_mouse._pos.x = event.mouse.x;
		_mouse._pos.y = event.mouse.y / 2;

		if (event.type == Common::EVENT_LBUTTONDOWN)
			_mouseClick = true;
		else if (event.type == Common::EVENT_LBUTTONUP)
			_mouseClick = false;

		break;
	case Common::EVENT_KEYDOWN:
		addKeypress(event);
		break;
	default:
		break;
	}

	return true;
}

/**
 * Add the specified key to the pending keypress stack
 */
void MortevielleEngine::addKeypress(Common::Event &evt) {
	// Character to add
	char ch = evt.kbd.ascii;

	// Check for debugger
	if ((evt.kbd.keycode == Common::KEYCODE_d) && (evt.kbd.flags & Common::KBD_CTRL)) {
		// Attach to the debugger
		_debugger.attach();
		_debugger.onFrame();
	} else if ((evt.kbd.keycode >= Common::KEYCODE_a) && (evt.kbd.keycode <= Common::KEYCODE_z)) {
		// Handle alphabetic keys
		if (evt.kbd.hasFlags(Common::KBD_CTRL))
			ch = evt.kbd.keycode - Common::KEYCODE_a + 1;
		else
			ch = evt.kbd.keycode - Common::KEYCODE_a + 'A';
	} else if ((evt.kbd.keycode >= Common::KEYCODE_F1) && (evt.kbd.keycode <= Common::KEYCODE_F12)) {
		// Handle function keys
		ch = 59 + evt.kbd.keycode - Common::KEYCODE_F1;
	} else {
		// Series of special cases
		switch (evt.kbd.keycode) {
		case Common::KEYCODE_KP4:
		case Common::KEYCODE_LEFT:
			ch = '4';
			break;
		case Common::KEYCODE_KP2:
		case Common::KEYCODE_DOWN:
			ch = '2';
			break;
		case Common::KEYCODE_KP6:
		case Common::KEYCODE_RIGHT:
			ch = '6';
			break;
		case Common::KEYCODE_KP8:
		case Common::KEYCODE_UP:
			ch = '8';
			break;
		case Common::KEYCODE_KP7:
			ch = '7';
			break;
		case Common::KEYCODE_KP1:
			ch = '1';
			break;
		case Common::KEYCODE_KP9:
			ch = '9';
			break;
		case Common::KEYCODE_KP3:
			ch = '3';
			break;
		case Common::KEYCODE_KP5:
			ch = '5';
			break;
		case Common::KEYCODE_RETURN:
			ch = '\13';
			break;
		case Common::KEYCODE_ESCAPE:
			ch = '\33';
			break;
		default:
			break;
		}
	}

	if (ch != 0)
		_keypresses.push(ch);
}


static byte CURSOR_ARROW_DATA[16 * 16] = {
	0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/**
 * Initialize the mouse
 */
void MortevielleEngine::initMouse() {
	CursorMan.replaceCursor(CURSOR_ARROW_DATA, 16, 16, 0, 0, 0xff);
	CursorMan.showMouse(true);

	_mouse.initMouse();
}

/**
 * Sets the mouse position
 * @remarks		Since the ScummVM screen surface is double height to handle 640x200 using 640x400,
 * the mouse Y position is doubled to convert from 0-199 to 0-399
 */
void MortevielleEngine::setMousePos(const Common::Point &pt) {
	// Adjust the passed position from simulated 640x200 to 640x400 co-ordinates
	Common::Point newPoint(pt.x, (pt.y == 199) ? 399 : pt.y * 2);

	if (newPoint != _mousePos)
		// Warp the mouse to the new position
		g_system->warpMouse(newPoint.x, newPoint.y);

	// Save the new position
	_mousePos = newPoint;
}

/**
 * Delay by a given amount
 */
void MortevielleEngine::delay(int amount) {
	uint32 endTime = g_system->getMillis() + amount;

	while (g_system->getMillis() < endTime) {
		if (g_system->getMillis() > (_lastGameFrame + GAME_FRAME_DELAY)) {
			_lastGameFrame = g_system->getMillis();
			_screenSurface.updateScreen();

			_debugger.onFrame();
		}

		g_system->delayMillis(10);
	}
}

/**
 * Waits for the user to select an action, and then handles it
 * @remarks	Originally called tecran
 */
void MortevielleEngine::handleAction() {
	const int lim = 20000;
	int temps = 0;
	char inkey = '\0';
	bool oo, funct = 0;

	clearVerbBar();
	oo = false;
	_controlMenu = 0;
	if (!_keyPressedEsc) {
		_menu.drawMenu();
		_menu._menuDisplayed = true;
		temps = 0;
		_key = 0;
		funct = false;
		inkey = '.';

		_inMainGameLoop = true;
		do {
			_menu.updateMenu();
			prepareRoom();
			_mouse.moveMouse(funct, inkey);
			if (shouldQuit())
				return;
			++temps;
		} while (!((_menu._menuSelected) || (temps > lim) || (funct) || (_anyone)));
		_inMainGameLoop = false;

		_menu.eraseMenu();
		_menu._menuDisplayed = false;
		if ((inkey == '\1') || (inkey == '\3') || (inkey == '\5') || (inkey == '\7') || (inkey == '\11')) {
			changeGraphicalDevice((uint)(ord(inkey) - 1) >> 1);
			return;
		}
		if (_menu._menuSelected && (_msg[3] == MENU_SAVE)) {
			Common::String saveName = Common::String::format("Savegame #%d", _msg[4] & 15);
			_savegameManager.saveGame(_msg[4] & 15, saveName);
		}
		if (_menu._menuSelected && (_msg[3] == MENU_LOAD))
			_savegameManager.loadGame((_msg[4] & 15) - 1);
		if (inkey == '\103') {       /* F9 */
			temps = _dialogManager.show(_hintPctMessage, 1);
			return;
		} else if (inkey == '\77') {
			if ((_menuOpcode != OPCODE_NONE) && ((_msg[3] == MENU_ACTION) || (_msg[3] == MENU_SELF))) {
				_msg[4] = _menuOpcode;
				displayTextInVerbBar(getEngineString(S_IDEM));
			} else
				return;
		} else if (inkey == '\104') {
			if ((_x != 0) && (_y != 0))
				_num = 9999;
			return;
		}
	}
	if (inkey == '\73') {
		_quitGame = true;
		hourToChar();
	} else {
		if ((funct) && (inkey != '\77'))
			return;
		if (temps > lim) {
			handleDescriptionText(2, 141);
			if (_num == 9999)
				_num = 0;
		} else {
			_menuOpcode = _msg[3];
			if ((_msg[3] == MENU_ACTION) || (_msg[3] == MENU_SELF))
				_menuOpcode = _msg[4];
			if (!_anyone) {
				if ((_heroSearching) || (_obpart)) {
					if (_mouse._pos.y < 12)
						return;

					if ((_msg[4] == OPCODE_SOUND) || (_msg[4] == OPCODE_LIFT)) {
						oo = true;
						if ((_msg[4] == OPCODE_LIFT) || (_obpart)) {
							endSearch();
							_caff = _coreVar._currPlace;
							_crep = 998;
						} else
							tsuiv();
						mennor();
					}
				}
			}
			do {
				if (! oo)
					handleOpcode();

				if ((_controlMenu == 0) && (! _loseGame) && (! _endGame)) {
					_text.taffich();
					if (_destinationOk) {
						_destinationOk = false;
						drawPicture();
					}
					if ((!_syn) || (_col))
						handleDescriptionText(2, _crep);
				}
			} while (_syn);
			if (_controlMenu != 0)
				displayControlMenu();
		}
	}
}

/**
 * Engine function - Init Places
 * @remarks	Originally called 'init_lieu'
 */
void MortevielleEngine::loadPlaces() {
	Common::File f;

	if (!f.open("MXX.mor"))
		if (!f.open("MFXX.mor"))
			error("Missing file - MXX.mor");

	for (int i = 0; i < 7; ++i) {
		for (int j = 0; j < 25; ++j)
			_destinationArray[i][j] = f.readByte();
	}

	f.close();
}

/**
 * Set Text Color
 * @remarks	Originally called 'text_color'
 */
void MortevielleEngine::setTextColor(int col) {
	_textColor = col;
}

/**
 * Prepare screen - Type 1!
 * @remarks	Originally called 'ecrf1'
 */
void MortevielleEngine::prepareScreenType1() {
	// Large drawing
	_screenSurface.drawBox(0, 11, 512, 163, 15);
}

/**
 * Prepare room - Type 2!
 * @remarks	Originally called 'ecrf2'
 */
void MortevielleEngine::prepareScreenType2() {
	setTextColor(5);
}

/**
 * Prepare room - Type 3!
 * @remarks	Originally called 'ecrf7'
 */
void MortevielleEngine::prepareScreenType3() {
	setTextColor(4);
}

/**
 * Engine function - Update hour
 * @remarks	Originally called 'calch'
 */
void MortevielleEngine::updateHour(int &day, int &hour, int &minute) {
	int newHour = readclock();
	int th = _currentHourCount + ((newHour - _currentDayHour) / _inGameHourDuration);
	minute = ((th % 2) + _currHalfHour) * 30;
	hour = ((uint)th >> 1) + _currHour;
	if (minute == 60) {
		minute = 0;
		++hour;
	}
	day = (hour / 24) + _currDay;
	hour = hour - ((day - _currDay) * 24);
}

/**
 * Engine function - Convert character index to bit index
 * @remarks	Originally called 'conv'
 */
int MortevielleEngine::convertCharacterIndexToBitIndex(int characterIndex) {
	return 128 >> (characterIndex - 1);
}

/**
 * Engine function - Convert bit index to character index
 * @remarks	Originally called 'tip'
 */
int MortevielleEngine::convertBitIndexToCharacterIndex(int bitIndex) {
	int retVal = 0;

	if (bitIndex == 128)
		retVal = 1;
	else if (bitIndex == 64)
		retVal = 2;
	else if (bitIndex == 32)
		retVal = 3;
	else if (bitIndex == 16)
		retVal = 4;
	else if (bitIndex == 8)
		retVal = 5;
	else if (bitIndex == 4)
		retVal = 6;
	else if (bitIndex == 2)
		retVal = 7;
	else if (bitIndex == 1)
		retVal = 8;

	return retVal;
}

/**
 * Engine function - Reset presence in other rooms
 * @remarks	Originally called 't5'
 */
void MortevielleEngine::resetPresenceInRooms(int roomId) {
	if (roomId == DINING_ROOM)
		_blo = false;

	if (roomId != GREEN_ROOM) {
		_roomPresenceLuc = false;
		_roomPresenceIda = false;
	}

	if (roomId != PURPLE_ROOM)
		_purpleRoomPresenceLeo = false;

	if (roomId != DARKBLUE_ROOM) {
		_roomPresenceGuy = false;
		_roomPresenceEva = false;
	}

	if (roomId != BLUE_ROOM)
		_roomPresenceMax = false;
	if (roomId != RED_ROOM)
		_roomPresenceBob = false;
	if (roomId != GREEN_ROOM2)
		_roomPresencePat = false;
	if (roomId != TOILETS)
		_toiletsPresenceBobMax = false;
	if (roomId != BATHROOM)
		_bathRoomPresenceBobMax = false;
	if (roomId != ROOM9)
		_room9PresenceLeo = false;
}

/**
 * Engine function - Show the people present in the given room
 * @remarks	Originally called 'affper'
 */
void MortevielleEngine::showPeoplePresent(int bitIndex) {
	int xp = 580 - (_screenSurface.getStringWidth("LEO") / 2);

	for (int i = 1; i <= 8; ++i)
		_menu.disableMenuItem(_menu._discussMenu[i]);

	clearUpperRightPart();
	if ((bitIndex & 128) == 128) {
		_screenSurface.putxy(xp, 24);
		_screenSurface.drawString("LEO", 4);
		_menu.enableMenuItem(_menu._discussMenu[1]);
	}
	if ((bitIndex & 64) == 64) {
		_screenSurface.putxy(xp, 32);
		_screenSurface.drawString("PAT", 4);
		_menu.enableMenuItem(_menu._discussMenu[2]);
	}
	if ((bitIndex & 32) == 32) {
		_screenSurface.putxy(xp, 40);
		_screenSurface.drawString("GUY", 4);
		_menu.enableMenuItem(_menu._discussMenu[3]);
	}
	if ((bitIndex & 16) == 16) {
		_screenSurface.putxy(xp, 48);
		_screenSurface.drawString("EVA", 4);
		_menu.enableMenuItem(_menu._discussMenu[4]);
	}
	if ((bitIndex & 8) == 8) {
		_screenSurface.putxy(xp, 56);
		_screenSurface.drawString("BOB", 4);
		_menu.enableMenuItem(_menu._discussMenu[5]);
	}
	if ((bitIndex & 4) == 4) {
		_screenSurface.putxy(xp, 64);
		_screenSurface.drawString("LUC", 4);
		_menu.enableMenuItem(_menu._discussMenu[6]);
	}
	if ((bitIndex & 2) == 2) {
		_screenSurface.putxy(xp, 72);
		_screenSurface.drawString("IDA", 4);
		_menu.enableMenuItem(_menu._discussMenu[7]);
	}
	if ((bitIndex & 1) == 1) {
		_screenSurface.putxy(xp, 80);
		_screenSurface.drawString("MAX", 4);
		_menu.enableMenuItem(_menu._discussMenu[8]);
	}
	_currBitIndex = bitIndex;
}

/**
 * Engine function - Select random characters
 * @remarks	Originally called 'choix'
 */
int MortevielleEngine::selectCharacters(int min, int max) {
	bool invertSelection = false;
	int rand = getRandomNumber(min, max);

	if (rand > 4) {
		rand = 8 - rand;
		invertSelection = true;
	}

	int i = 0;
	int retVal = 0;
	while (i < rand) {
		int charIndex = getRandomNumber(1, 8);
		int charBitIndex = convertCharacterIndexToBitIndex(charIndex);
		if ((retVal & charBitIndex) != charBitIndex) {
			++i;
			retVal |= charBitIndex;
		}
	}
	if (invertSelection)
		retVal = 255 - retVal;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Green Room
 * @remarks	Originally called 'cpl1'
 */
int MortevielleEngine::getPresenceStatsGreenRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	// The original uses an || instead of an &&, resulting
	// in an always true condition. Based on the other tests,
	// and on other scenes, we use an && instead.
	if ((hour > 7) && (hour < 11))
		retVal = 25;
	else if ((hour > 10) && (hour < 14))
		retVal = 35;
	else if ((hour > 13) && (hour < 16))
		retVal = 50;
	else if ((hour > 15) && (hour < 18))
		retVal = 5;
	else if ((hour > 17) && (hour < 22))
		retVal = 35;
	else if ((hour > 21) && (hour < 24))
		retVal = 50;
	else if ((hour >= 0) && (hour < 8))
		retVal = 70;

	_menu.updateMenu();

	return retVal;
}
/**
 * Engine function - Get Presence Statistics - Purple Room
 * @remarks	Originally called 'cpl2'
 */
int MortevielleEngine::getPresenceStatsPurpleRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if ((hour > 7) && (hour < 11))
		retVal = -2;
	else if (hour == 11)
		retVal = 100;
	else if ((hour > 11) && (hour < 23))
		retVal = 10;
	else if (hour == 23)
		retVal = 20;
	else if ((hour >= 0) && (hour < 8))
		retVal = 50;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Toilets
 * @remarks	Originally called 'cpl3'
 */
int MortevielleEngine::getPresenceStatsToilets() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 8) && (hour < 10)) || ((hour > 19) && (hour < 24)))
		retVal = 34;
	else if (((hour > 9) && (hour < 20)) || ((hour >= 0) && (hour < 9)))
		retVal = 0;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Blue Room
 * @remarks	Originally called 'cpl5'
 */
int MortevielleEngine::getPresenceStatsBlueRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if ((hour > 6) && (hour < 10))
		retVal = 0;
	else if (hour == 10)
		retVal = 100;
	else if ((hour > 10) && (hour < 24))
		retVal = 15;
	else if ((hour >= 0) && (hour < 7))
		retVal = 50;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Red Room
 * @remarks	Originally called 'cpl6'
 */
int MortevielleEngine::getPresenceStatsRedRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 7) && (hour < 13)) || ((hour > 17) && (hour < 20)))
		retVal = -2;
	else if (((hour > 12) && (hour < 17)) || ((hour > 19) && (hour < 24)))
		retVal = 35;
	else if (hour == 17)
		retVal = 100;
	else if ((hour >= 0) && (hour < 8))
		retVal = 60;

	return retVal;
}

/**
 * Shows the "you are alone" message in the status area
 * on the right hand side of the screen
 * @remarks	Originally called 'person'
 */
void MortevielleEngine::displayAloneText() {
	for (int cf = 1; cf <= 8; ++cf)
		_menu.disableMenuItem(_menu._discussMenu[cf]);

	Common::String sYou = getEngineString(S_YOU);
	Common::String sAre = getEngineString(S_ARE);
	Common::String sAlone = getEngineString(S_ALONE);

	clearUpperRightPart();
	_screenSurface.putxy(580 - (_screenSurface.getStringWidth(sYou) / 2), 30);
	_screenSurface.drawString(sYou, 4);
	_screenSurface.putxy(580 - (_screenSurface.getStringWidth(sAre) / 2), 50);
	_screenSurface.drawString(sAre, 4);
	_screenSurface.putxy(580 - (_screenSurface.getStringWidth(sAlone) / 2), 70);
	_screenSurface.drawString(sAlone, 4);

	_currBitIndex = 0;
}

/**
 * Engine function - Get Presence Statistics - Room Bureau
 * @remarks	Originally called 'cpl10'
 */
int MortevielleEngine::getPresenceStatsDiningRoom(int &hour) {
	int day, minute;

	int retVal = 0;
	updateHour(day, hour, minute);
	if (((hour > 7) && (hour < 11)) || ((hour > 11) && (hour < 14)) || ((hour > 18) && (hour < 21)))
		retVal = 100;
	else if ((hour == 11) || ((hour > 20) && (hour < 24)))
		retVal = 45;
	else if (((hour > 13) && (hour < 17)) || (hour == 18))
		retVal = 35;
	else if (hour == 17)
		retVal = 60;
	else if ((hour >= 0) && (hour < 8))
		retVal = 5;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Bureau
 * @remarks	Originally called 'cpl11'
 */
int MortevielleEngine::getPresenceStatsBureau(int &hour) {
	int day, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 8) && (hour < 12)) || ((hour > 20) && (hour < 24)))
		retVal = 25;
	else if (((hour > 11) && (hour < 14)) || ((hour > 18) && (hour < 21)))
		retVal = 5;
	else if ((hour > 13) && (hour < 17))
		retVal = 55;
	else if ((hour > 16) && (hour < 19))
		retVal = 45;
	else if ((hour >= 0) && (hour < 9))
		retVal = 0;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Kitchen
 * @remarks	Originally called 'cpl12'
 */
int MortevielleEngine::getPresenceStatsKitchen() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 8) && (hour < 15)) || ((hour > 16) && (hour < 22)))
		retVal = 55;
	else if (((hour > 14) && (hour < 17)) || ((hour > 21) && (hour < 24)))
		retVal = 25;
	else if ((hour >= 0) && (hour < 5))
		retVal = 0;
	else if ((hour > 4) && (hour < 9))
		retVal = 15;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Attic
 * @remarks	Originally called 'cpl13'
 */
int MortevielleEngine::getPresenceStatsAttic() {
	return 0;
}

/**
 * Engine function - Get Presence Statistics - Room Landing
 * @remarks	Originally called 'cpl15'
 */
int MortevielleEngine::getPresenceStatsLanding() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if ((hour > 7) && (hour < 12))
		retVal = 25;
	else if ((hour > 11) && (hour < 14))
		retVal = 0;
	else if ((hour > 13) && (hour < 18))
		retVal = 10;
	else if ((hour > 17) && (hour < 20))
		retVal = 55;
	else if ((hour > 19) && (hour < 22))
		retVal = 5;
	else if ((hour > 21) && (hour < 24))
		retVal = 15;
	else if ((hour >= 0) && (hour < 8))
		retVal = -15;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Chapel
 * @remarks	Originally called 'cpl20'
 */
int MortevielleEngine::getPresenceStatsChapel(int &hour) {
	int day, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (hour == 10)
		retVal = 65;
	else if ((hour > 10) && (hour < 21))
		retVal = 5;
	else if ((hour > 20) && (hour < 24))
		retVal = -15;
	else if ((hour >= 0) && (hour < 5))
		retVal = -300;
	else if ((hour > 4) && (hour < 10))
		retVal = -5;

	return retVal;
}

/**
 * Engine function - Check who is in the Green Room
 * @remarks	Originally called 'quelq1'
 */
void MortevielleEngine::setPresenceGreenRoom(int roomId) {
	int rand = getRandomNumber(1, 2);
	if (roomId == GREEN_ROOM) {
		if (rand == 1)
			_roomPresenceLuc = true;
		else
			_roomPresenceIda = true;
	} else if (roomId == DARKBLUE_ROOM) {
		if (rand == 1)
			_roomPresenceGuy = true;
		else
			_roomPresenceEva = true;
	}

	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Purple Room
 * @remarks	Originally called 'quelq2'
 */
void MortevielleEngine::setPresencePurpleRoom() {
	if (_place == PURPLE_ROOM)
		_purpleRoomPresenceLeo = true;
	else
		_room9PresenceLeo = true;

	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Blue Room
 * @remarks	Originally called 'quelq5'
 */
void MortevielleEngine::setPresenceBlueRoom() {
	_roomPresenceMax = true;
	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Red Room
 * @remarks	Originally called 'quelq6'
 */
void MortevielleEngine::setPresenceRedRoom(int roomId) {
	if (roomId == RED_ROOM)
		_roomPresenceBob = true;
	else if (roomId == GREEN_ROOM2)
		_roomPresencePat = true;

	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Dining Room
 * @remarks	Originally called 'quelq10'
 */
int MortevielleEngine::setPresenceDiningRoom(int hour) {
	int retVal = 0;

	if ((hour >= 0) && (hour < 8))
		retVal = checkLeoMaxRandomPresence();
	else {
		int min = 0, max = 0;
		if ((hour > 7) && (hour < 10)) {
			min = 5;
			max = 7;
		} else if ((hour > 9) && (hour < 12)) {
			min = 1;
			max = 4;
		} else if (((hour > 11) && (hour < 15)) || ((hour > 18) && (hour < 21))) {
			min = 6;
			max = 8;
		} else if (((hour > 14) && (hour < 19)) || ((hour > 20) && (hour < 24))) {
			min = 1;
			max = 5;
		}
		retVal = selectCharacters(min, max);
	}
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Bureau
 * @remarks	Originally called 'quelq11'
 */
int MortevielleEngine::setPresenceBureau(int hour) {
	int retVal = 0;

	if ((hour >= 0) && (hour < 8))
		retVal = checkLeoMaxRandomPresence();
	else {
		int min = 0, max = 0;
		if (((hour > 7) && (hour < 10)) || ((hour > 20) && (hour < 24))) {
			min = 1;
			max = 3;
		} else if (((hour > 9) && (hour < 12)) || ((hour > 13) && (hour < 19))) {
			min = 1;
			max = 4;
		} else if (((hour > 11) && (hour < 14)) || ((hour > 18) && (hour < 21))) {
			min = 1;
			max = 2;
		}
		retVal = selectCharacters(min, max);
	}
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Kitchen
 * @remarks	Originally called 'quelq12'
 */
int MortevielleEngine::setPresenceKitchen() {
	int retVal = checkLeoMaxRandomPresence();
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Landing
 * @remarks	Originally called 'quelq15'
 */
int MortevielleEngine::setPresenceLanding() {
	bool test = false;
	int rand = 0;
	do {
		rand = getRandomNumber(1, 8);
		test = (((rand == 1) && (_purpleRoomPresenceLeo || _room9PresenceLeo)) ||
		        ((rand == 2) && _roomPresencePat) ||
		        ((rand == 3) && _roomPresenceGuy) ||
		        ((rand == 4) && _roomPresenceEva) ||
		        ((rand == 5) && _roomPresenceBob) ||
		        ((rand == 6) && _roomPresenceLuc) ||
		        ((rand == 7) && _roomPresenceIda) ||
		        ((rand == 8) && _roomPresenceMax));
	} while (test);

	int retVal = convertCharacterIndexToBitIndex(rand);
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the chapel
 * @remarks	Originally called 'quelq20'
 */
int MortevielleEngine::setPresenceChapel(int hour) {
	int retVal = 0;

	if (((hour >= 0) && (hour < 10)) || ((hour > 18) && (hour < 24)))
		retVal = checkLeoMaxRandomPresence();
	else {
		int min = 0, max = 0;
		if ((hour > 9) && (hour < 12)) {
			min = 3;
			max = 7;
		} else if ((hour > 11) && (hour < 18)) {
			min = 1;
			max = 2;
		} else if (hour == 18) {
			min = 2;
			max = 4;
		}
		retVal = selectCharacters(min, max);
	}
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Get the answer after you known a door
 * @remarks	Originally called 'frap'
 */
void MortevielleEngine::getKnockAnswer() {
	int day, hour, minute;

	updateHour(day, hour, minute);
	if ((hour >= 0) && (hour < 8))
		_crep = 190;
	else {
		if (getRandomNumber(1, 100) > 70)
			_crep = 190;
		else
			_crep = 147;
	}
}

/**
 * Engine function - Get Room Presence Bit Index
 * @remarks	Originally called 'nouvp'
 */
int MortevielleEngine::getPresenceBitIndex(int roomId) {
	int bitIndex = 0;
	if (roomId == GREEN_ROOM) {
		if (_roomPresenceLuc)
			bitIndex = 4;  // LUC
		if (_roomPresenceIda)
			bitIndex = 2;  // IDA
	} else if ( ((roomId == PURPLE_ROOM) && (_purpleRoomPresenceLeo))
			 || ((roomId == ROOM9) && (_room9PresenceLeo)))
		bitIndex = 128;    // LEO
	else if (roomId == DARKBLUE_ROOM) {
		if (_roomPresenceGuy)
			bitIndex = 32; // GUY
		if (_roomPresenceEva)
			bitIndex = 16; // EVA
	} else if ((roomId == BLUE_ROOM) && (_roomPresenceMax))
		bitIndex = 1;      // MAX
	else if ((roomId == RED_ROOM) && (_roomPresenceBob))
		bitIndex = 8;      // BOB
	else if ((roomId == GREEN_ROOM2) && (_roomPresencePat))
		bitIndex = 64;     // PAT
	else if ( ((roomId == TOILETS) && (_toiletsPresenceBobMax))
		   || ((roomId == BATHROOM) && (_bathRoomPresenceBobMax)) )
		bitIndex = 9;      // BOB + MAX

	if (bitIndex != 9)
		showPeoplePresent(bitIndex);

	return bitIndex;
}

/**
 * Engine function - initGame
 * @remarks	Originally called 'dprog'
 */
void MortevielleEngine::initGame() {
	_place = MANOR_FRONT;
	_currentHourCount = 0;
	if (!_coreVar._alreadyEnteredManor)
		_blo = true;
	_inGameHourDuration = kTime1;
	_currentDayHour = readclock();
}

/**
 * Engine function - Set Random Presence - Green Room
 * @remarks	Originally called 'pl1'
 */
void MortevielleEngine::setRandomPresenceGreenRoom(int cf) {
	if ( ((_place == GREEN_ROOM) && (!_roomPresenceLuc) && (!_roomPresenceIda))
	  || ((_place == DARKBLUE_ROOM) && (!_roomPresenceGuy) && (!_roomPresenceEva)) ) {
		int p = getPresenceStatsGreenRoom();
		int rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceGreenRoom(_place);
	}
}

/**
 * Engine function - Set Random Presence - Purple Room
 * @remarks	Originally called 'pl2'
 */
void MortevielleEngine::setRandomPresencePurpleRoom(int cf) {
	if (!_purpleRoomPresenceLeo) {
		int p = getPresenceStatsPurpleRoom();
		int rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresencePurpleRoom();
	}
}

/**
 * Engine function - Set Random Presence - Blue Room
 * @remarks	Originally called 'pl5'
 */
void MortevielleEngine::setRandomPresenceBlueRoom(int cf) {
	if (!_roomPresenceMax) {
		int p = getPresenceStatsBlueRoom();
		int rand;

		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceBlueRoom();
	}
}

/**
 * Engine function - Set Random Presence - Red Room
 * @remarks	Originally called 'pl6'
 */
void MortevielleEngine::setRandomPresenceRedRoom(int cf) {
	if ( ((_place == RED_ROOM) && (!_roomPresenceBob))
	  || ((_place == GREEN_ROOM2) && (!_roomPresencePat)) ) {
		int p = getPresenceStatsRedRoom();
		int rand;

		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceRedRoom(_place);
	}
}

/**
 * Engine function - Set Random Presence - Room 9
 * @remarks	Originally called 'pl9'
 */
void MortevielleEngine::setRandomPresenceRoom9(int cf) {
	if (!_room9PresenceLeo) {
		cf = -10;
		int p, rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresencePurpleRoom();
	}
}

/**
 * Engine function - Set Random Presence - Dining Room
 * @remarks	Originally called 'pl10'
 */
void MortevielleEngine::setRandomPresenceDiningRoom(int cf) {
	int h, rand;
	int p = getPresenceStatsDiningRoom(h);
	phaz(rand, p, cf);

	if (rand > p)
		displayAloneText();
	else
		setPresenceDiningRoom(h);
}

/**
 * Engine function - Set Random Presence - Bureau
 * @remarks	Originally called 'pl11'
 */
void MortevielleEngine::setRandomPresenceBureau(int cf) {
	int h, rand;

	int p = getPresenceStatsBureau(h);
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceBureau(h);
}

/**
 * Engine function - Set Random Presence - Kitchen
 * @remarks	Originally called 'pl12'
 */
void MortevielleEngine::setRandomPresenceKitchen(int cf) {
	int p, rand;

	p = getPresenceStatsKitchen();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceKitchen();
}

/**
 * Engine function - Set Random Presence - Attic / Cellar
 * @remarks	Originally called 'pl13'
 */
void MortevielleEngine::setRandomPresenceAttic(int cf) {
	int p, rand;

	p = getPresenceStatsAttic();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceKitchen();
}

/**
 * Engine function - Set Random Presence - Landing
 * @remarks	Originally called 'pl15'
 */
void MortevielleEngine::setRandomPresenceLanding(int cf) {
	int p, rand;

	p = getPresenceStatsLanding();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceLanding();
}

/**
 * Engine function - Set Random Presence - Chapel
 * @remarks	Originally called 'pl20'
 */
void MortevielleEngine::setRandomPresenceChapel(int cf) {
	int h, rand;

	int p = getPresenceStatsChapel(h);
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceChapel(h);
}

/**
 * Start music or speech
 * @remarks	Originally called 'musique'
 */
void MortevielleEngine::startMusicOrSpeech(int so) {
	if (so == 0) {
		/* musik(0) */
		;
	} else if ((!_introSpeechPlayed) && (!_coreVar._alreadyEnteredManor)) {
		// Type 1: Speech
		_speechManager.startSpeech(10, 1, 1);
		_introSpeechPlayed = true;
	} else {
		if (((_coreVar._currPlace == MOUNTAIN) || (_coreVar._currPlace == MANOR_FRONT) || (_coreVar._currPlace == MANOR_BACK)) && (getRandomNumber(1, 3) == 2))
			// Type 1: Speech
			_speechManager.startSpeech(9, getRandomNumber(2, 4), 1);
		else if ((_coreVar._currPlace == CHAPEL) && (getRandomNumber(1, 2) == 1))
			// Type 1: Speech
			_speechManager.startSpeech(8, 1, 1);
		else if ((_coreVar._currPlace == WELL) && (getRandomNumber(1, 2) == 2))
			// Type 1: Speech
			_speechManager.startSpeech(12, 1, 1);
		else if (_coreVar._currPlace == INSIDE_WELL)
			// Type 1: Speech
			_speechManager.startSpeech(13, 1, 1);
		else
			// Type 2 : music
			_speechManager.startSpeech(getRandomNumber(1, 17), 1, 2);
	}
}

/**
 * Engine function - You lose!
 * @remarks	Originally called 'tperd'
 */
void MortevielleEngine::loseGame() {
	resetOpenObjects();
	_roomDoorId = OWN_ROOM;
	_mchai = 0;
	_menu.unsetSearchMenu();
	if (!_blo)
		getPresence(MANOR_FRONT);

	_loseGame = true;
	clearUpperLeftPart();
	_screenSurface.drawBox(60, 35, 400, 50, 15);
	handleDescriptionText(9, _crep);
	clearDescriptionBar();
	clearVerbBar();
	_col = false;
	_syn = false;
	_destinationOk = false;
}

/**
 * Engine function - Check inventory for a given object
 * @remarks	Originally called 'cherjer'
 */
bool MortevielleEngine::checkInventory(int objectId) {
	bool retVal = false;
	for (int i = 1; i <= 6; ++i)
		retVal = (retVal || (_coreVar._inventory[i] == objectId));

	if (_coreVar._selectedObjectId == objectId)
		retVal = true;

	return retVal;
}

/**
 * Engine function - Display Dining Room
 * @remarks	Originally called 'st1sama'
 */
void MortevielleEngine::displayDiningRoom() {
	_coreVar._currPlace = DINING_ROOM;
	prepareDisplayText();
}

/**
 * Engine function - Start non interactive Dialog
 * @remarks	Originally called 'sparl'
 */
void MortevielleEngine::startDialog(int16 rep) {
	const int haut[9] = { 0, 0, 1, -3, 6, -2, 2, 7, -1 };
	int key;

	assert(rep >= 0);

	_mouse.hideMouse();
	Common::String dialogStr = getString(rep + kDialogStringIndex);
	_text.displayStr(dialogStr, 230, 4, 65, 24, 5);
	_dialogManager.drawF3F8();

	key = 0;
	do {
		_speechManager.startSpeech(rep, haut[_caff - 69], 0);
		key = _dialogManager.waitForF3F8();
		if (shouldQuit())
			return;
	} while (key != 66);
	hirs();
	_mouse.showMouse();
}

/**
 * Engine function - End of Search: reset globals
 * @remarks	Originally called 'finfouill'
 */
void MortevielleEngine::endSearch() {
	_heroSearching = false;
	_obpart = false;
	_searchCount = 0;
	_menu.unsetSearchMenu();
}

/**
 * Engine function - Go to Dining room
 * @remarks	Originally called 't1sama'
 */
void MortevielleEngine::gotoDiningRoom() {
	int day, hour, minute;

	updateHour(day, hour, minute);
	if ((hour < 5) && (_coreVar._currPlace > ROOM18)) {
		if (!checkInventory(137)) {        //You don't have the keys, and it's late
			_crep = 1511;
			loseGame();
		} else
			displayDiningRoom();
	} else if (!_coreVar._alreadyEnteredManor) {     //Is it your first time?
		_currBitIndex = 255; // Everybody is present
		showPeoplePresent(_currBitIndex);
		_caff = 77;
		drawPictureWithText();
		_screenSurface.drawBox(223, 47, 155, 91, 15);
		handleDescriptionText(2, 33);
		testKey(false);
		mennor();
		_mouse.hideMouse();
		hirs();
		premtet();
		startDialog(140);
		drawRightFrame();
		drawClock();
		_mouse.showMouse();
		_coreVar._currPlace = OWN_ROOM;
		prepareDisplayText();
		resetPresenceInRooms(DINING_ROOM);
		if (!_blo)
			getPresence(OWN_ROOM);
		_currBitIndex = 0;
		_savedBitIndex = 0;
		_coreVar._alreadyEnteredManor = true;
	} else
		displayDiningRoom();
}

/**
 * Engine function - Check Manor distance (in the mountains)
 * @remarks	Originally called 't1neig'
 */
void MortevielleEngine::checkManorDistance() {
	++_manorDistance;
	if (_manorDistance > 2) {
		_crep = 1506;
		loseGame();
	} else {
		_destinationOk = true;
		_coreVar._currPlace = MOUNTAIN;
		prepareDisplayText();
	}
}

/**
 * Engine function - Go to Manor front
 * @remarks	Originally called 't1deva'
 */
void MortevielleEngine::gotoManorFront() {
	_manorDistance = 0;
	_coreVar._currPlace = MANOR_FRONT;
	prepareDisplayText();
}

/**
 * Engine function - Go to Manor back
 * @remarks	Originally called 't1derr'
 */
void MortevielleEngine::gotoManorBack() {
	_coreVar._currPlace = MANOR_BACK;
	prepareDisplayText();
}

/**
 * Engine function - Dead : Flooded in Well
 * @remarks	Originally called 't1deau'
 */
void MortevielleEngine::floodedInWell() {
	_crep = 1503;
	loseGame();
}

/**
 * Engine function - Change Graphical Device
 * @remarks	Originally called 'change_gd'
 */
void MortevielleEngine::changeGraphicalDevice(int newDevice) {
	_mouse.hideMouse();
	_currGraphicalDevice = newDevice;
	hirs();
	_mouse.initMouse();
	_mouse.showMouse();
	drawRightFrame();
	prepareRoom();
	drawClock();
	if (_currBitIndex != 0)
		showPeoplePresent(_currBitIndex);
	else
		displayAloneText();
	clearDescriptionBar();
	clearVerbBar();
	_maff = 68;
	drawPictureWithText();
	handleDescriptionText(2, _crep);
	_menu.displayMenu();
}

/**
 * Called when a savegame has been loaded.
 * @remarks	Originally called 'antegame'
 */
void MortevielleEngine::gameLoaded() {
	_mouse.hideMouse();
	_menu._menuDisplayed = false;
	_loseGame = true;
	_anyone = false;
	_destinationOk = true;
	_col = false;
	_hiddenHero = false;
	_uptodatePresence = false;
	_maff = 68;
	_menuOpcode = OPCODE_NONE;
	_introSpeechPlayed = false;
	_x = 0;
	_y = 0;
	_num = 0;
	_startHour = 0;
	_endHour = 0;
	_searchCount = 0;
	_roomDoorId = OWN_ROOM;
	_syn = true;
	_heroSearching = true;
	_mchai = 0;
	_manorDistance = 0;
	resetOpenObjects();
	_takeObjCount = 0;
	prepareDisplayText();
	_hintPctMessage = getString(580);

	_destinationOk = false;
	_endGame = true;
	_loseGame = false;
	_heroSearching = false;

	displayAloneText();
	prepareRoom();
	drawClock();
	drawPictureWithText();
	handleDescriptionText(2, _crep);
	clearVerbBar();
	_endGame = false;
	_menu.setDestinationText(_coreVar._currPlace);
	_menu.setInventoryText();
	if (_coreVar._selectedObjectId != 0)
		displayItemInHand(_coreVar._selectedObjectId + 400);
	_mouse.showMouse();
}

/**
 * Engine function - Handle OpCodes
 * @remarks	Originally called 'tsitu'
 */
void MortevielleEngine::handleOpcode() {
	if (!_col)
		clearDescriptionBar();
	_syn = false;
	_keyPressedEsc = false;
	if (!_anyone) {
		if (_uptodatePresence) {
			if ((_msg[3] == MENU_MOVE) || (_msg[4] == OPCODE_LEAVE) || (_msg[4] == OPCODE_SLEEP) || (_msg[4] == OPCODE_EAT)) {
				_controlMenu = 4;
				mennor();
				return;
			}
		}
		if (_msg[3] == MENU_MOVE)
			fctMove();
		if (_msg[3] == MENU_DISCUSS)
			fctDiscuss();
		if (_msg[3] == MENU_INVENTORY)
			fctInventoryTake();
		if (_msg[4] == OPCODE_ATTACH)
			fctAttach();
		if (_msg[4] == OPCODE_WAIT)
			fctWait();
		if (_msg[4] == OPCODE_FORCE)
			fctForce();
		if (_msg[4] == OPCODE_SLEEP)
			fctSleep();
		if (_msg[4] == OPCODE_LISTEN)
			fctListen();
		if (_msg[4] == OPCODE_ENTER)
			fctEnter();
		if (_msg[4] == OPCODE_CLOSE)
			fctClose();
		if (_msg[4] == OPCODE_SEARCH)
			fctSearch();
		if (_msg[4] == OPCODE_KNOCK)
			fctKnock();
		if (_msg[4] == OPCODE_SCRATCH)
			fctScratch();
		if (_msg[4] == OPCODE_READ)
			fctRead();
		if (_msg[4] == OPCODE_EAT)
			fctEat();
		if (_msg[4] == OPCODE_PLACE)
			fctPlace();
		if (_msg[4] == OPCODE_OPEN)
			fctOpen();
		if (_msg[4] == OPCODE_TAKE)
			fctTake();
		if (_msg[4] == OPCODE_LOOK)
			fctLook();
		if (_msg[4] == OPCODE_SMELL)
			fctSmell();
		if (_msg[4] == OPCODE_SOUND)
			fctSound();
		if (_msg[4] == OPCODE_LEAVE)
			fctLeave();
		if (_msg[4] == OPCODE_LIFT)
			fctLift();
		if (_msg[4] == OPCODE_TURN)
			fctTurn();
		if (_msg[4] == OPCODE_SSEARCH)
			fctSelfSearch();
		if (_msg[4] == OPCODE_SREAD)
			fctSelfRead();
		if (_msg[4] == OPCODE_SPUT)
			fctSelfPut();
		if (_msg[4] == OPCODE_SLOOK)
			fctSelftLook();
		_hiddenHero = false;

		if (_msg[4] == OPCODE_SHIDE)
			fctSelfHide();
	} else {
		if (_anyone) {
			interactNPC();
			_anyone = false;
			mennor();
			return;
		}
	}
	int hour, day, minute;
	updateHour(day, hour, minute);
	if ((((hour == 12) || (hour == 13) || (hour == 19)) && (_coreVar._currPlace != DINING_ROOM)) ||
	        ((hour > 0) && (hour < 6) && (_coreVar._currPlace != OWN_ROOM)))
		++_coreVar._faithScore;
	if (((_coreVar._currPlace < CRYPT) || (_coreVar._currPlace > MOUNTAIN)) && (_coreVar._currPlace != INSIDE_WELL)
	        && (_coreVar._currPlace != OWN_ROOM) && (_coreVar._selectedObjectId != 152) && (!_loseGame)) {
		if ((_coreVar._faithScore > 99) && (hour > 8) && (hour < 16)) {
			_crep = 1501;
			loseGame();
		}
		if ((_coreVar._faithScore > 99) && (hour > 0) && (hour < 9)) {
			_crep = 1508;
			loseGame();
		}
		if ((day > 1) && (hour > 8) && (!_loseGame)) {
			_crep = 1502;
			loseGame();
		}
	}
	mennor();
}

/**
 * Engine function - Transform time into a char
 * @remarks	Originally called 'tmaj3'
 */
void MortevielleEngine::hourToChar() {
	int day, hour, minute;

	updateHour(day, hour, minute);
	if (minute == 30)
		minute = 1;
	hour += day * 24;
	minute += hour * 2;
	_coreVar._fullHour = chr(minute);
}

/**
 * Engine function - extract time from a char
 * @remarks	Originally called 'theure'
 */
void MortevielleEngine::charToHour() {
	int fullHour = ord(_coreVar._fullHour);
	int tmpHour = fullHour % 48;
	_currDay = fullHour / 48;
	_currHalfHour = tmpHour % 2;
	_currHour = tmpHour / 2;
	_hour = _currHour;
	if (_currHalfHour == 1)
		_minute = 30;
	else
		_minute = 0;
}

/**
 * Engine function - Clear upper left part of Screen - Type 1
 * @remarks	Originally called 'clsf1'
 */
void MortevielleEngine::clearUpperLeftPart() {
	_mouse.hideMouse();
	_screenSurface.fillRect(0, Common::Rect(0, 11, 514, 175));
	_mouse.showMouse();
}

/**
 * Engine function - Clear low bar used by description
 * @remarks	Originally called 'clsf2'
 */
void MortevielleEngine::clearDescriptionBar() {
	_mouse.hideMouse();
	if (_largestClearScreen) {
		_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 199));
		_screenSurface.drawBox(0, 175, 634, 24, 15);
		_largestClearScreen = false;
	} else {
		_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 190));
		_screenSurface.drawBox(0, 175, 634, 15, 15);
	}
	_mouse.showMouse();
}

/**
 * Engine function - Clear lowest bar used by verbs
 * @remarks	Originally called 'clsf3'
 */
void MortevielleEngine::clearVerbBar() {
	_mouse.hideMouse();
	_screenSurface.fillRect(0, Common::Rect(1, 192, 633, 199));
	_screenSurface.drawBox(0, 191, 634, 8, 15);
	_mouse.showMouse();
}

/**
 * Engine function - Clear upper right part of the screen
 * @remarks	Originally called 'clsf10'
 */
void MortevielleEngine::clearUpperRightPart() {
	int x1, x2;
	Common::String st;

	_mouse.hideMouse();
	if (_resolutionScaler == 1) {
		x2 = 634;
		x1 = 534;
	} else {
		x2 = 600;
		x1 = 544;
	}
	// Clear ambiance description
	_screenSurface.fillRect(15, Common::Rect(x1, 93, x2, 98));
	if (_coreVar._faithScore < 33)
		st = getEngineString(S_COOL);
	else if (_coreVar._faithScore < 66)
		st = getEngineString(S_LOURDE);
	else if (_coreVar._faithScore > 65)
		st = getEngineString(S_MALSAINE);

	x1 = 580 - (_screenSurface.getStringWidth(st) / 2);
	_screenSurface.putxy(x1, 92);
	_screenSurface.drawString(st, 4);

	// Clear person list
	_screenSurface.fillRect(15, Common::Rect(560, 24, 610, 86));
	_mouse.showMouse();
}

/**
 * Engine function - Get a random number between two values
 * @remarks	Originally called 'get_random_number' and 'hazard'
 */
int MortevielleEngine::getRandomNumber(int minval, int maxval) {
	return _randomSource.getRandomNumber(maxval - minval) + minval;
}

/**
 * Engine function - Show alert "use move menu"
 * @remarks	Originally called 'aldepl'
 */
void MortevielleEngine::showMoveMenuAlert() {
	_dialogManager.show(getEngineString(S_USE_DEP_MENU), 1);
}

/**
 * The original engine used this method to display a starting text screen letting the player
 * select the graphics mode to use
 * @remarks	Originally called 'dialpre'
 */
void MortevielleEngine::showConfigScreen() {
	_crep = 998;
}

/**
 * Decodes a number of 64 byte blocks
 * @param pStart	Start of data
 * @param count		Number of 64 byte blocks
 * @remarks	Originally called 'zzuul'
 */
void MortevielleEngine::decodeNumber(byte *pStart, int count) {
	while (count-- > 0) {
		for (int idx = 0; idx < 64; ++pStart, ++idx) {
			uint16 v = ((*pStart - 0x80) << 1) + 0x80;

			if (v & 0x8000)
				*pStart = 0;
			else if (v & 0xff00)
				*pStart = 0xff;
			else
				*pStart = (byte)v;
		}
	}
}

const byte cryptoArrDefaultFr[32] = {
	32, 101, 115,  97, 114, 105, 110,
	117, 116, 111, 108,  13, 100,  99,
	112, 109,  46, 118, 130,  39, 102,
	98,  44, 113, 104, 103,  33,  76,
	85, 106,  30,  31
};

const byte cryptoArr30Fr[32] = {
	69,  67,  74, 138, 133, 120,  77, 122,
	121,  68,  65,  63,  73,  80,  83,  82,
	156,  45,  58,  79,  49,  86,  78,  84,
	71,  81,  64,  66, 135,  34, 136,  91
};

const byte cryptoArr31Fr[32]= {
	93,  47,  48,  53,  50,  70, 124,  75,
	72, 147, 140, 150, 151,  57,  56,  51,
	107, 139,  55,  89, 131,  37,  54,  88,
	119,   0,   0,   0,   0,   0,   0,   0
};

const byte cryptoArrDefaultDe[32] = {
	0x20, 0x65, 0x6E, 0x69, 0x73, 0x72, 0x74,
	0x68, 0x61, 0x75, 0x0D, 0x63, 0x6C, 0x64,
	0x6D, 0x6F, 0x67, 0x2E, 0x62, 0x66, 0x53,
	0x2C, 0x77, 0x45, 0x7A, 0x6B, 0x44, 0x76,
	0x9C, 0x47, 0x1E, 0x1F
};

const byte cryptoArr30De[32] = {
	0x49, 0x4D, 0x21, 0x42, 0x4C, 0x70, 0x41, 0x52,
	0x57, 0x4E, 0x48, 0x3F, 0x46, 0x50, 0x55, 0x4B,
	0x5A, 0x4A, 0x54, 0x31, 0x4F, 0x56, 0x79, 0x3A,
	0x6A, 0x5B, 0x5D, 0x40, 0x22, 0x2F, 0x30, 0x35
};

const byte cryptoArr31De[32]= {
	0x78, 0x2D, 0x32, 0x82, 0x43, 0x39, 0x33, 0x38,
	0x7C, 0x27, 0x37, 0x3B, 0x25, 0x28, 0x29, 0x36,
	0x51, 0x59, 0x71, 0x81, 0x87, 0x88, 0x93, 0,
	0,    0,    0,    0,    0,    0,    0,    0
};

const byte *cryptoArrDefault, *cryptoArr30, *cryptoArr31;
uint16 ctrlChar;

void MortevielleEngine::cinq_huit(char &c, int &idx, byte &pt, bool &the_end) {
	uint16 oct, ocd;

	/* 5-8 */
	oct = _dialogIndexArray[idx];
	oct = ((uint16)(oct << (16 - pt))) >> (16 - pt);
	if (pt < 6) {
		++idx;
		oct = oct << (5 - pt);
		pt += 11;
		oct = oct | ((uint)_dialogIndexArray[idx] >> pt);
	} else {
		pt -= 5;
		oct = (uint)oct >> pt;
	}

	if (oct == ctrlChar) {
		c = '$';
		the_end = true;
	} else if (oct == 30 || oct == 31) {
		ocd = _dialogIndexArray[idx];
		ocd = (uint16)(ocd << (16 - pt)) >> (16 - pt);
		if (pt < 6) {
			++idx;
			ocd = ocd << (5 - pt);
			pt += 11;
			ocd = ocd | ((uint)_dialogIndexArray[idx] >> pt);
		} else {
			pt -= 5;
			ocd = (uint)ocd >> pt;
		}

		if (oct == 30)
			c = chr(cryptoArr30[ocd]);
		else
			c = chr(cryptoArr31[ocd]);

		if (c == '\0') {
			the_end = true;
			c = '#';
		}
	} else {
		c = chr(cryptoArrDefault[oct]);
	}
}

/**
 * Decode and extract the line with the given Id
 * @remarks	Originally called 'deline'
 */
Common::String MortevielleEngine::getString(int num) {
	Common::String wrkStr = "";

	if (num < 0) {
		warning("getString(%d): num < 0! Skipping", num);
	} else if (!_txxFileFl) {
		wrkStr = getGameString(num);
	} else {
		int hint = _dialogHintArray[num]._hintId;
		byte point = _dialogHintArray[num]._point;
		int length = 0;
		bool endFl = false;
		char let;
		do {
			cinq_huit(let, hint, point, endFl);
			wrkStr += let;
			++length;
		} while (!endFl);
	}

	while (wrkStr.lastChar() == '$')
		// Remove trailing '$'s
		wrkStr.deleteLastChar();

	return wrkStr;
}

void MortevielleEngine::copcha() {
	for (int i = kAcha; i < kAcha + 390; i++)
		_tabdon[i] = _tabdon[i + 390];
}

/**
 * Engine function - When restarting the game, reset the main variables used by the engine
 * @remarks	Originally called 'inzon'
 */
void MortevielleEngine::resetVariables() {
	copcha();

	_coreVar._alreadyEnteredManor = false;
	_coreVar._selectedObjectId = 0;
	_coreVar._cellarObjectId = 0;
	_coreVar._atticBallHoleObjectId = 0;
	_coreVar._atticRodHoleObjectId = 0;
	_coreVar._wellObjectId = 0;
	_coreVar._secretPassageObjectId = 0;
	_coreVar._purpleRoomObjectId = 136;
	_coreVar._cryptObjectId = 141;
	_coreVar._faithScore = getRandomNumber(4, 10);
	_coreVar._currPlace = MANOR_FRONT;

	for (int i = 2; i <= 6; ++i)
		_coreVar._inventory[i] = 0;

	// Only object in inventory: a gun
	_coreVar._inventory[1] = 113;

	_coreVar._fullHour = chr(20);

	for (int i = 1; i <= 10; ++i)
		_coreVar._pctHintFound[i] = ' ';

	for (int i = 1; i <= 6; ++i)
		_coreVar._availableQuestion[i] = '*';

	for (int i = 7; i <= 9; ++i)
		_coreVar._availableQuestion[i] = ' ';

	for (int i = 10; i <= 28; ++i)
		_coreVar._availableQuestion[i] = '*';

	for (int i = 29; i <= 42; ++i)
		_coreVar._availableQuestion[i] = ' ';

	_coreVar._availableQuestion[33] = '*';

	for (int i = 1; i <= 8; ++i)
		_nbrep[i] = 0;

	init_nbrepm();
}

/**
 * Engine function - Set the palette
 * @remarks	Originally called 'writepal'
 */
void MortevielleEngine::setPal(int n) {
	switch (_currGraphicalDevice) {
	case MODE_TANDY:
	case MODE_EGA:
	case MODE_AMSTRAD1512:
		for (int i = 1; i <= 16; ++i) {
			_mem[(0x7000 * 16) + (2 * i)] = _stdPal[n][i].x;
			_mem[(0x7000 * 16) + (2 * i) + 1] = _stdPal[n][i].y;
		}
		break;
	case MODE_CGA: {
		nhom pal[16];
		for (int i = 0; i < 16; ++i) {
			pal[i] = _cgaPal[n]._a[i];
		}

		if (n < 89)
			palette(_cgaPal[n]._p);

		for (int i = 0; i <= 15; ++i)
			displayCGAPattern(i, _patternArr[pal[i]._id], pal);
		}
		break;
	default:
		break;
	}
}

/**
 * Engine function - Display a CGA pattern, using a specified palette
 * @remarks	Originally called 'outbloc'
 */
void MortevielleEngine::displayCGAPattern(int n, Pattern p, nhom *pal) {
	int addr = n * 404 + 0xd700;

	WRITE_LE_UINT16(&_mem[0x6000 * 16 + addr], p._tax);
	WRITE_LE_UINT16(&_mem[0x6000 * 16 + addr + 2], p._tay);
	addr += 4;
	for (int i = 0; i < p._tax; ++i) {
		for (int j = 0; j < p._tay; ++j)
			_mem[(0x6000 * 16) + addr + j * p._tax + i] = pal[n]._hom[p._des[i + 1][j + 1]];
	}
}

/**
 * Engine function - Load Palette from File
 * @remarks	Originally called 'charpal'
 */
void MortevielleEngine::loadPalette() {
	Common::File f;
	byte b;

	if (!f.open("fxx.mor")) {
		if (f.open("mfxx.mor"))
			f.seek(7 * 25);
		else
			error("Missing file - fxx.mor");
	}

	for (int i = 0; i < 108; ++i)
		_fxxBuffer[i] = f.readSint16LE();
	f.close();

	if (!f.open("plxx.mor"))
		error("Missing file - plxx.mor");
	for (int i = 0; i <= 90; ++i) {
		for (int j = 1; j <= 16; ++j) {
			_stdPal[i][j].x = f.readByte();
			_stdPal[i][j].y = f.readByte();
		}
	}
	f.close();

	if (!f.open("cxx.mor"))
		error("Missing file - cxx.mor");

	for (int j = 0; j <= 90; ++j) {
		_cgaPal[j]._p = f.readByte();
		for (int i = 0; i <= 15; ++i) {
			nhom &with = _cgaPal[j]._a[i];

			b = f.readByte();
			with._id = (uint)b >> 4;
			with._hom[0] = ((uint)b >> 2) & 3;
			with._hom[1] = b & 3;
		}
	}

	_cgaPal[10]._a[9] = _cgaPal[10]._a[5];
	for (int j = 0; j <= 14; ++j) {
		_patternArr[j]._tax = f.readByte();
		_patternArr[j]._tay = f.readByte();
		for (int i = 1; i <= 20; ++i) {
			for (int k = 1; k <= 20; ++k)
				_patternArr[j]._des[i][k] = f.readByte();
		}
	}
	f.close();
}

/**
 * Engine function - Load Texts from File
 * @remarks	Originally called 'chartex'
 */
void MortevielleEngine::loadTexts() {
	Common::File inpFile;
	Common::File ntpFile;

	_txxFileFl = false;
	if (getLanguage() == Common::EN_ANY) {
		warning("English version expected - Switching to DAT file");
		return;
	}

	if (!inpFile.open("TXX.INP")) {
		if (!inpFile.open("TXX.MOR")) {
			warning("Missing file - TXX.INP or .MOR - Switching to DAT file");
			return;
		}
	}
	if (ntpFile.open("TXX.NTP")) {
		cryptoArr30 = cryptoArr30Fr;
		cryptoArr31 = cryptoArr31Fr;
		cryptoArrDefault = cryptoArrDefaultFr;
		ctrlChar = 11;
	} else if (ntpFile.open("TXX.IND")) {
		cryptoArr30 = cryptoArr30De;
		cryptoArr31 = cryptoArr31De;
		cryptoArrDefault = cryptoArrDefaultDe;
		ctrlChar = 10;
	} else {
		warning("Missing file - TXX.NTP or .IND - Switching to DAT file");
		return;
	}

	if ((inpFile.size() > (kMaxDialogIndex * 2)) || (ntpFile.size() > (kMaxDialogHint * 3))) {
		warning("TXX file - Unexpected format - Switching to DAT file");
		return;
	}

	for (int i = 0; i < inpFile.size() / 2; ++i)
		_dialogIndexArray[i] = inpFile.readUint16LE();

	inpFile.close();
	_txxFileFl = true;

	for (int i = 0; i < (ntpFile.size() / 3); ++i) {
		_dialogHintArray[i]._hintId = ntpFile.readSint16LE();
		_dialogHintArray[i]._point = ntpFile.readByte();
	}

	ntpFile.close();

}

void MortevielleEngine::loadBRUIT5() {
	Common::File f;

	if (!f.open("bruit5"))
		error("Missing file - bruit5");

	f.read(&_mem[kAdrNoise5 * 16 + 0], 149 * 128);
	f.close();
}

void MortevielleEngine::loadCFIEC() {
	Common::File f;

	if (!f.open("cfiec.mor")) {
		if (!f.open("alcfiec.mor"))
			error("Missing file - *cfiec.mor");
	}

	_cfiecBufferSize = ((f.size() / 128) + 1) * 128;
	int32 fileSize = f.size();

	if (!_reloadCFIEC)
		_cfiecBuffer = (byte *)malloc(sizeof(byte) * _cfiecBufferSize);

	for (int32 i = 0; i < fileSize; ++i)
		_cfiecBuffer[i] = f.readByte();

	for (int i = fileSize; i < _cfiecBufferSize; i++)
		_cfiecBuffer[i] = 0;

	f.close();

	_reloadCFIEC = false;
}


void MortevielleEngine::loadCFIPH() {
	Common::File f;

	if (!f.open("cfiph.mor")) {
		if (!f.open("alcfiph.mor"))
			error("Missing file - *cfiph.mor");
	}

	_speechManager._cfiphBuffer = (int16 *)malloc(sizeof(int16) * (f.size() / 2));

	for (int i = 0; i < (f.size() / 2); ++i)
		_speechManager._cfiphBuffer[i] = f.readSint16LE();

	f.close();
}

/**
 * Engine function - Play Music
 * @remarks	Originally called 'music'
 */
void MortevielleEngine::music() {
	if (_soundOff)
		return;

	_reloadCFIEC = true;

	Common::File fic;
	if (!fic.open("mort.img"))
		error("Missing file - mort.img");

	fic.read(&_mem[0x3800 * 16 + 0], 500);
	fic.read(&_mem[0x47a0 * 16 + 0], 123);
	fic.close();

	_soundManager.decodeMusic(&_mem[0x3800 * 16], &_mem[0x5000 * 16], 623);
	_addFix = (float)((kTempoMusic - 8)) / 256;
	_speechManager.cctable(_speechManager._tbi);

	bool fin = false;
	int k = 0;
	do {
		fin = keyPressed();
		_soundManager.musyc(_speechManager._tbi, 9958, kTempoMusic);
		++k;
		fin = fin | keyPressed() | (k >= 5);
	} while (!fin);
	while (keyPressed())
		getChar();
}

/**
 * Engine function - Show title screen
 * @remarks	Originally called 'suite'
 */
void MortevielleEngine::showTitleScreen() {
	hirs();
	handleDescriptionText(7, 2035);
	_caff = 51;
	_text.taffich();
	testKeyboard();
	if (_newGraphicalDevice != _currGraphicalDevice)
		_currGraphicalDevice = _newGraphicalDevice;
	hirs();
	draw(kAdrDes, 0, 0);

	Common::String cpr = "COPYRIGHT 1989 : LANKHOR";
	_screenSurface.putxy(104 + 72 * _resolutionScaler, 185);
	_screenSurface.drawString(cpr, 0);
}

/**
 * Draw picture
 * @remarks	Originally called 'dessine'
 */
void MortevielleEngine::draw(int ad, int x, int y) {
	_mouse.hideMouse();
	setPal(_numpal);
	pictout(ad, 0, x, y);
	_mouse.showMouse();
}

/**
 * Draw right frame
 * @remarks	Originally called 'dessine_rouleau'
 */
void MortevielleEngine::drawRightFrame() {
	setPal(89);
	if (_currGraphicalDevice == MODE_HERCULES) {
		_mem[0x7000 * 16 + 14] = 15;
	}
	_mouse.hideMouse();
	pictout(0x73a2, 0, 0, 0);
	_mouse.showMouse();
}

/**
 * Read the current system time
 */
int MortevielleEngine::readclock() {
	TimeDate dateTime;
	g_system->getTimeAndDate(dateTime);

	int m = dateTime.tm_min * 60;
	int h = dateTime.tm_hour * 3600;
	return h + m + dateTime.tm_sec;
}

/**
 * Engine function - Prepare room and hint string
 * @remarks	Originally called 'tinke'
 */
void MortevielleEngine::prepareRoom() {
	int day, hour, minute;

	_anyone = false;
	updateHour(day, hour, minute);
	if (day != _day) {
		_day = day;
		int i = 0;
		do {
			++i;
			if (_nbrepm[i] != 0)
				--_nbrepm[i];
			_nbrep[i] = 0;
		} while (i != 8);
	}
	if ((hour > _hour) || ((hour == 0) && (_hour == 23))) {
		_hour = hour;
		_minute = 0;
		drawClock();
		int hintCount = 0;
		for (int i = 1; i <= 10; ++i) {
			if (_coreVar._pctHintFound[i] == '*')
				++hintCount;
		}

		Common::String pctStr;
		if (hintCount == 10)
			pctStr = "10";
		else
			pctStr = chr(hintCount + 48);

		_hintPctMessage = "[1][";
		_hintPctMessage += getEngineString(S_SHOULD_HAVE_NOTICED);
		_hintPctMessage += pctStr;
		_hintPctMessage += '0';
		_hintPctMessage += getEngineString(S_NUMBER_OF_HINTS);
		_hintPctMessage += "][";
		_hintPctMessage += getEngineString(S_OK);
		_hintPctMessage += ']';
	}
	if (minute > _minute) {
		_minute = 30;
		drawClock();
	}
	if (_mouse._pos.y < 12)
		return;

	if (!_blo) {
		if ((hour == 12) || ((hour > 18) && (hour < 21)) || ((hour >= 0) && (hour < 7)))
			_inGameHourDuration = kTime2;
		else
			_inGameHourDuration = kTime1;
		if ((_coreVar._faithScore > 33) && (_coreVar._faithScore < 66))
			_inGameHourDuration -= (_inGameHourDuration / 3);

		if (_coreVar._faithScore > 65)
			_inGameHourDuration -= ((_inGameHourDuration / 3) * 2);

		int newHour = readclock();
		if ((newHour - _currentDayHour) > _inGameHourDuration) {
			bool activeMenu = _menu._menuActive;
			_menu.eraseMenu();
			_currentHourCount += ((newHour - _currentDayHour) / _inGameHourDuration);
			_currentDayHour = newHour;
			switch (_place) {
			case GREEN_ROOM:
			case DARKBLUE_ROOM:
				setRandomPresenceGreenRoom(_coreVar._faithScore);
				break;
			case PURPLE_ROOM:
				setRandomPresencePurpleRoom(_coreVar._faithScore);
				break;
			case BLUE_ROOM:
				setRandomPresenceBlueRoom(_coreVar._faithScore);
				break;
			case RED_ROOM:
			case GREEN_ROOM2:
				setRandomPresenceRedRoom(_coreVar._faithScore);
				break;
			case ROOM9:
				setRandomPresenceRoom9(_coreVar._faithScore);
				break;
			case DINING_ROOM:
				setRandomPresenceDiningRoom(_coreVar._faithScore);
				break;
			case BUREAU:
				setRandomPresenceBureau(_coreVar._faithScore);
				break;
			case KITCHEN:
				setRandomPresenceKitchen(_coreVar._faithScore);
				break;
			case ATTIC:
			case CELLAR:
				setRandomPresenceAttic(_coreVar._faithScore);
				break;
			case LANDING:
			case ROOM26:
				setRandomPresenceLanding(_coreVar._faithScore);
				break;
			case CHAPEL:
				setRandomPresenceChapel(_coreVar._faithScore);
				break;
			}
			if ((_savedBitIndex != 0) && (_currBitIndex != 10))
				_savedBitIndex = _currBitIndex;

			if ((_savedBitIndex == 0) && (_currBitIndex > 0)) {
				if ((_coreVar._currPlace == ATTIC) || (_coreVar._currPlace == CELLAR)) {
					initCaveOrCellar();
				} else if (_currBitIndex == 10) {
					_currBitIndex = 0;
					if (!_uptodatePresence) {
						_uptodatePresence = true;
						_startHour = readclock();
						if (getRandomNumber(1, 5) < 5) {
							clearVerbBar();
							prepareScreenType2();
							displayTextInVerbBar(getEngineString(S_HEAR_NOISE));
							int rand = (getRandomNumber(0, 4)) - 2;
							_speechManager.startSpeech(1, rand, 1);
							clearVerbBar();
						}
					}
				}
			}

			if (activeMenu)
				_menu.drawMenu();
		}
	}
	_endHour = readclock();
	if ((_uptodatePresence) && ((_endHour - _startHour) > 17)) {
		getPresenceBitIndex(_place);
		_uptodatePresence = false;
		_startHour = 0;
		if ((_coreVar._currPlace > OWN_ROOM) && (_coreVar._currPlace < DINING_ROOM))
			_anyone = true;
	}
}

/**
 * Engine function - Draw Clock
 * @remarks	Originally called 'pendule'
 */
void MortevielleEngine::drawClock() {
	const int cv[2][12] = {
		{  5,  8, 10,  8,  5,  0, -5, -8, -10, -8, -5,  0 },
		{ -5, -3,  0,  3,  5,  6,  5,  3,   0, -3, -5, -6 }
	};
	const int x = 580;
	const int y = 123;
	const int rg = 9;
	int hourColor;

	_mouse.hideMouse();

	_screenSurface.drawRectangle(570, 118, 20, 10);
	_screenSurface.drawRectangle(578, 114, 6, 18);
	if ((_currGraphicalDevice == MODE_CGA) || (_currGraphicalDevice == MODE_HERCULES))
		hourColor = 0;
	else
		hourColor = 1;

	if (_minute == 0)
		_screenSurface.drawLine(((uint)x >> 1) * _resolutionScaler, y, ((uint)x >> 1) * _resolutionScaler, (y - rg), hourColor);
	else
		_screenSurface.drawLine(((uint)x >> 1) * _resolutionScaler, y, ((uint)x >> 1) * _resolutionScaler, (y + rg), hourColor);

	int hour12 = _hour;
	if (hour12 > 12)
		hour12 -= 12;
	if (hour12 == 0)
		hour12 = 12;

	_screenSurface.drawLine(((uint)x >> 1) * _resolutionScaler, y, ((uint)(x + cv[0][hour12 - 1]) >> 1) * _resolutionScaler, y + cv[1][hour12 - 1], hourColor);
	_mouse.showMouse();
	_screenSurface.putxy(568, 154);

	if (_hour > 11)
		_screenSurface.drawString("PM ", 1);
	else
		_screenSurface.drawString("AM ", 1);

	_screenSurface.putxy(550, 160);
	if ((_day >= 0) && (_day <= 8)) {
		Common::String tmp = getEngineString(S_DAY);
		tmp.insertChar((char)(_day + 49), 0);
		_screenSurface.drawString(tmp, 1);
	}
}

void MortevielleEngine::palette(int v1) {
	warning("TODO: palette");
}

/**
 * Returns a substring of the given string
 * @param s		Source string
 * @param idx	Starting index (1 based)
 * @param size	Number of characters to return
 */

Common::String MortevielleEngine::copy(const Common::String &s, int idx, size_t size) {
	// Copy the substring into a temporary buffer
	char *tmp = new char[size + 1];
	strncpy(tmp, s.c_str() + idx - 1, size);
	tmp[size] = '\0';

	Common::String result(tmp);
	delete[] tmp;
	return result;
}

void MortevielleEngine::hirs() {
	// Note: The original used this to set the graphics mode and clear the screen, both at
	// the start of the game, and whenever the screen need to be cleared. As such, this
	// method is deprecated in favour of clearing the screen
	debugC(1, kMortevielleCore, "TODO: hirs is deprecated in favour of ScreenSurface::clearScreen");

	if (_currGraphicalDevice == MODE_TANDY) {
		_screenSurface.fillRect(0, Common::Rect(0, 0, 639, 200));
		_resolutionScaler = 1;
	} else if (_currGraphicalDevice == MODE_CGA) {
		palette(1);
		_resolutionScaler = 1;
	} else
		_resolutionScaler = 2;

	_screenSurface.clearScreen();
}

/**
 * Init room : Cave or Cellar
 * @remarks	Originally called 'cavegre'
 */
void MortevielleEngine::initCaveOrCellar() {
	_coreVar._faithScore += 2;
	if (_coreVar._faithScore > 69)
		_coreVar._faithScore += (_coreVar._faithScore / 10);
	clearVerbBar();
	prepareScreenType2();
	displayTextInVerbBar(getEngineString(S_SOMEONE_ENTERS));
	int rand = (getRandomNumber(0, 4)) - 2;
	_speechManager.startSpeech(2, rand, 1);

	// The original was doing here a useless loop.
	// It has been removed

	clearVerbBar();
	displayAloneText();
}

/**
 * Display control menu string
 * @remarks	Originally called 'tctrm'
 */
void MortevielleEngine::displayControlMenu() {
	handleDescriptionText(2, (3000 + _controlMenu));
	_controlMenu = 0;
}

void MortevielleEngine::pictout(int seg, int dep, int x, int y) {
	GfxSurface surface;
	surface.decode(&_mem[seg * 16 + dep]);

	if (_currGraphicalDevice == MODE_HERCULES) {
		_mem[0x7000 * 16 + 2] = 0;
		_mem[0x7000 * 16 + 32] = 15;
	}

	if ((_caff != 51) && (READ_LE_UINT16(&_mem[0x7000 * 16 + 0x4138]) > 0x100))
		WRITE_LE_UINT16(&_mem[0x7000 * 16 + 0x4138], 0x100);

	_screenSurface.drawPicture(surface, x, y);
}

void MortevielleEngine::adzon() {
	Common::File f;

	if (!f.open("don.mor"))
		error("Missing file - don.mor");

	f.read(_tabdon, 7 * 256);
	f.close();

	if (!f.open("bmor.mor"))
		error("Missing file - bmor.mor");

	f.read(&_tabdon[kFleche], 1 * 1916);
	f.close();

	if (!f.open("dec.mor"))
		error("Missing file - dec.mor");

	f.read(&_mem[0x73a2 * 16 + 0], 1 * 1664);
	f.close();
}

/**
 * Returns the offset within the compressed image data resource of the desired image
 */
int MortevielleEngine::animof(int ouf, int num) {
	int nani = _mem[kAdrAni * 16 + 1];
	int aux = num;
	if (ouf != 1)
		aux += nani;

	int animof_result = (nani << 2) + 2 + READ_BE_UINT16(&_mem[kAdrAni * 16 + (aux << 1)]);

	return animof_result;
}

/**
 * Display text in description bar
 * @remarks	Originally called 'text1'
 */
void MortevielleEngine::displayTextInDescriptionBar(int x, int y, int nb, int mesgId) {
	int co;

	if (_resolutionScaler == 1)
		co = 10;
	else
		co = 6;
	Common::String tmpStr = getString(mesgId);
	if ((y == 182) && ((int) tmpStr.size() * co > nb * 6))
		y = 176;
	_text.displayStr(tmpStr, x, y, nb, 20, _textColor);
}

/**
 * Display description text
 * @remarks	Originally called 'repon'
 */
void MortevielleEngine::handleDescriptionText(int f, int mesgId) {
	if ((mesgId > 499) && (mesgId < 563)) {
		Common::String tmpStr = getString(mesgId - 501 + kInventoryStringIndex);

		if ((int) tmpStr.size() > ((58 + (_resolutionScaler - 1) * 37) << 1))
			_largestClearScreen = true;
		else
			_largestClearScreen = false;

		clearDescriptionBar();
		_text.displayStr(tmpStr, 8, 176, 85, 3, 5);
	} else {
		mapMessageId(mesgId);
		switch (f) {
		case 2:
		case 8:
			clearDescriptionBar();
			prepareScreenType2();
			displayTextInDescriptionBar(8, 182, 103, mesgId);
			if ((mesgId == 68) || (mesgId == 69))
				_coreVar._availableQuestion[40] = '*';
			if ((mesgId == 104) && (_caff == CELLAR)) {
				_coreVar._availableQuestion[36] = '*';
				if (_coreVar._availableQuestion[39] == '*') {
					_coreVar._pctHintFound[3] = '*';
					_coreVar._availableQuestion[38] = '*';
				}
			}
			break;
		case 1:
		case 6:
		case 9: {
			int i;
			if ((f == 1) || (f == 6))
				i = 4;
			else
				i = 5;

			Common::String tmpStr = getString(mesgId);
			_text.displayStr(tmpStr, 80, 40, 60, 25, i);

			if (mesgId == 180)
				_coreVar._pctHintFound[6] = '*';
			else if (mesgId == 179)
				_coreVar._pctHintFound[10] = '*';
			}
			break;
		default:
			break;
		}
	}
}

/**
 * Recompute message Id
 * @remarks	Originally called 'modif'
 */
void MortevielleEngine::mapMessageId(int &mesgId) {
	if (mesgId == 26)
		mesgId = 25;
	else if ((mesgId > 29) && (mesgId < 36))
		mesgId -= 4;
	else if ((mesgId > 69) && (mesgId < 78))
		mesgId -= 37;
	else if ((mesgId > 99) && (mesgId < 194))
		mesgId -= 59;
	else if ((mesgId > 996) && (mesgId < 1000))
		mesgId -= 862;
	else if ((mesgId > 1500) && (mesgId < 1507))
		mesgId -= 1363;
	else if ((mesgId > 1507) && (mesgId < 1513))
		mesgId -= 1364;
	else if ((mesgId > 1999) && (mesgId < 2002))
		mesgId -= 1851;
	else if (mesgId == 2010)
		mesgId = 151;
	else if ((mesgId > 2011) && (mesgId < 2025))
		mesgId -= 1860;
	else if (mesgId == 2026)
		mesgId = 165;
	else if ((mesgId > 2029) && (mesgId < 2037))
		mesgId -= 1864;
	else if ((mesgId > 3000) && (mesgId < 3005))
		mesgId -= 2828;
	else if (mesgId == 4100)
		mesgId = 177;
	else if (mesgId == 4150)
		mesgId = 178;
	else if ((mesgId > 4151) && (mesgId < 4156))
		mesgId -= 3973;
	else if (mesgId == 4157)
		mesgId = 183;
	else if ((mesgId == 4160) || (mesgId == 4161))
		mesgId -= 3976;
}

/**
 * Initialize open objects array
 * @remarks	Originally called 'initouv'
 */
void MortevielleEngine::resetOpenObjects() {
	for (int cx = 1; cx <= 7; ++cx)
		_openObjects[cx] = 0;
	_openObjCount = 0;
}

void MortevielleEngine::ecr2(Common::String text) {
	// Some dead code was present in the original: removed
	_screenSurface.putxy(8, 177);
	int tlig = 59 + (_resolutionScaler - 1) * 36;

	if ((int)text.size() < tlig)
		_screenSurface.drawString(text, 5);
	else if ((int)text.size() < (tlig << 1)) {
		_screenSurface.putxy(8, 176);
		_screenSurface.drawString(copy(text, 1, (tlig - 1)), 5);
		_screenSurface.putxy(8, 182);
		_screenSurface.drawString(copy(text, tlig, tlig << 1), 5);
	} else {
		_largestClearScreen = true;
		clearDescriptionBar();
		_screenSurface.putxy(8, 176);
		_screenSurface.drawString(copy(text, 1, (tlig - 1)), 5);
		_screenSurface.putxy(8, 182);
		_screenSurface.drawString(copy(text, tlig, ((tlig << 1) - 1)), 5);
		_screenSurface.putxy(8, 190);
		_screenSurface.drawString(copy(text, tlig << 1, tlig * 3), 5);
	}
}

void MortevielleEngine::displayTextInVerbBar(Common::String text) {
	clearVerbBar();
	_screenSurface.putxy(8, 192);
	_screenSurface.drawString(text, 5);
}

/**
 * Display item in hand
 * @remarks	Originally called 'modobj'
 */
void MortevielleEngine::displayItemInHand(int objId) {
	Common::String strp = Common::String(' ');

	if (objId != 500)
		strp = getString(objId - 501 + kInventoryStringIndex);

	_menu.setText(_menu._inventoryMenu[8], strp);
	_menu.disableMenuItem(_menu._inventoryMenu[8]);
}

/**
 * Display empty hand
 * @remarks	Originally called 'maivid'
 */
void MortevielleEngine::displayEmptyHand() {
	_coreVar._selectedObjectId = 0;
	displayItemInHand(500);
}

/**
 * Set a random presence: Leo or Max
 * @remarks	Originally called 'chlm'
 */
int MortevielleEngine::checkLeoMaxRandomPresence() {
	int retval = getRandomNumber(1, 2);
	if (retval == 2)
		retval = 128;

	return retval;
}

/**
 * Reset room variables
 * @remarks	Originally called 'debloc'
 */
void MortevielleEngine::resetRoomVariables(int roomId) {
	_num = 0;
	_x = 0;
	_y = 0;
	if ((roomId != ROOM26) && (roomId != LANDING))
		resetPresenceInRooms(roomId);
	_savedBitIndex = _currBitIndex;
}

/**
 * Compute presence stats
 * @remarks	Originally called 'ecfren'
 */
int MortevielleEngine::getPresenceStats(int &rand, int cf, int roomId) {
	if (roomId == OWN_ROOM)
		displayAloneText();
	int retVal = -500;
	rand = 0;
	if ( ((roomId == GREEN_ROOM) && (!_roomPresenceLuc) && (!_roomPresenceIda))
	  || ((roomId == DARKBLUE_ROOM) && (!_roomPresenceGuy) && (!_roomPresenceEva)) )
		retVal = getPresenceStatsGreenRoom();
	if ((roomId == PURPLE_ROOM) && (!_purpleRoomPresenceLeo) && (!_room9PresenceLeo))
		retVal = getPresenceStatsPurpleRoom();
	if ( ((roomId == TOILETS) && (!_toiletsPresenceBobMax))
	  || ((roomId == BATHROOM) && (!_bathRoomPresenceBobMax)) )
		retVal = getPresenceStatsToilets();
	if ((roomId == BLUE_ROOM) && (!_roomPresenceMax))
		retVal = getPresenceStatsBlueRoom();
	if ( ((roomId == RED_ROOM) && (!_roomPresenceBob))
	  || ((roomId == GREEN_ROOM2) && (!_roomPresencePat)))
		retVal = getPresenceStatsRedRoom();
	if ((roomId == ROOM9) && (!_room9PresenceLeo) && (!_purpleRoomPresenceLeo))
		retVal = 10;
	if ( ((roomId == PURPLE_ROOM) && (_room9PresenceLeo))
	  || ((roomId == ROOM9) && (_purpleRoomPresenceLeo)))
		retVal = -400;
	if (retVal != -500) {
		retVal += cf;
		rand = getRandomNumber(1, 100);
	}

	return retVal;
}

/**
 * Set presence flags
 * @remarks	Originally called 'becfren'
 */
void MortevielleEngine::setPresenceFlags(int roomId) {
	if ((roomId == GREEN_ROOM) || (roomId == DARKBLUE_ROOM)) {
		int rand = getRandomNumber(1, 2);
		if (roomId == GREEN_ROOM) {
			if (rand == 1)
				_roomPresenceLuc = true;
			else
				_roomPresenceIda = true;
		} else { // roomId == DARKBLUE_ROOM
			if (rand == 1)
				_roomPresenceGuy = true;
			else
				_roomPresenceEva = true;
		}
	} else if (roomId == PURPLE_ROOM)
		_purpleRoomPresenceLeo = true;
	else if (roomId == TOILETS)
		_toiletsPresenceBobMax = true;
	else if (roomId == BLUE_ROOM)
		_roomPresenceMax = true;
	else if (roomId == RED_ROOM)
		_roomPresenceBob = true;
	else if (roomId == BATHROOM)
		_bathRoomPresenceBobMax = true;
	else if (roomId == GREEN_ROOM2)
		_roomPresencePat = true;
	else if (roomId == ROOM9)
		_room9PresenceLeo = true;
}

void MortevielleEngine::init_nbrepm() {
	static const byte ipm[9] = { 0, 4, 5, 6, 7, 5, 6, 5, 8 };

	for (int idx = 0; idx < 9; ++idx)
		_nbrepm[idx] = ipm[idx];
}

void MortevielleEngine::phaz(int &rand, int &p, int cf) {
	p += cf;
	rand = getRandomNumber(1, 100);
}

/**
 * Get Presence
 * @remarks	Originally called 't11'
 */
int MortevielleEngine::getPresence(int roomId) {
	int retVal = 0;
	int rand;

	int p = getPresenceStats(rand, _coreVar._faithScore, roomId);
	_place = roomId;
	if ((roomId > OWN_ROOM) && (roomId < DINING_ROOM)) {
		if (p != -500) {
			if (rand > p) {
				displayAloneText();
				retVal = 0;
			} else {
				setPresenceFlags(_place);
				retVal = getPresenceBitIndex(_place);
			}
		} else
			retVal = getPresenceBitIndex(_place);
	}

	if (roomId > ROOM9) {
		if ((roomId > LANDING) && (roomId != CHAPEL) && (roomId != ROOM26))
			displayAloneText();
		else {
			int h = 0;
			if (roomId == DINING_ROOM)
				p = getPresenceStatsDiningRoom(h);
			else if (roomId == BUREAU)
				p = getPresenceStatsBureau(h);
			else if (roomId == KITCHEN)
				p = getPresenceStatsKitchen();
			else if ((roomId == ATTIC) || (roomId == CELLAR))
				p = getPresenceStatsAttic();
			else if ((roomId == LANDING) || (roomId == ROOM26))
				p = getPresenceStatsLanding();
			else if (roomId == CHAPEL)
				p = getPresenceStatsChapel(h);
			p += _coreVar._faithScore;
			rand = getRandomNumber(1, 100);
			if (rand > p) {
				displayAloneText();
				retVal = 0;
			} else {
				if (roomId == DINING_ROOM)
					p = setPresenceDiningRoom(h);
				else if (roomId == BUREAU)
					p = setPresenceBureau(h);
				else if ((roomId == KITCHEN) || (roomId == ATTIC) || (roomId == CELLAR))
					p = setPresenceKitchen();
				else if ((roomId == LANDING) || (roomId == ROOM26))
					p = setPresenceLanding();
				else if (roomId == CHAPEL)
					p = setPresenceChapel(h);
				retVal = p;
			}
		}
	}

	return retVal;
}

void MortevielleEngine::writetp(Common::String s, int t) {
	if (_resolutionScaler == 2)
		_screenSurface.drawString(s, t);
	else
		_screenSurface.drawString(copy(s, 1, 25), t);
}

void MortevielleEngine::aniof(int ouf, int num) {
	if ((_caff == BATHROOM) && ((num == 4) || (num == 5)))
		return;

	if ((_caff == DINING_ROOM) && (num == 7))
		num = 6;
	else if (_caff == KITCHEN) {
		if (num == 3)
			num = 4;
		else if (num == 4)
			num = 3;
	}

	int ad = kAdrAni;
	int offset = animof(ouf, num);

	GfxSurface surface;
	surface.decode(&_mem[ad * 16 + offset]);
	_screenSurface.drawPicture(surface, 0, 12);

	prepareScreenType1();
}

/**
 * Draw Picture
 * @remarks	Originally called 'dessin'
 */
void MortevielleEngine::drawPicture() {
	clearUpperLeftPart();
	if (_caff > 99) {
		draw(kAdrDes, 60, 33);
		_screenSurface.drawBox(118, 32, 291, 121, 15);         // Medium box
	} else if (_caff > 69) {
		draw(kAdrDes, 112, 48);           // Heads
		_screenSurface.drawBox(222, 47, 155, 91, 15);
	} else {
		draw(kAdrDes, 0, 12);
		prepareScreenType1();
		if ((_caff < 30) || (_caff > 32)) {
			for (int cx = 1; cx <= 6; ++cx) {
				if (_openObjects[cx] != 0)
					aniof(1, _openObjects[cx]);
			}

			if (_caff == ATTIC) {
				if (_coreVar._atticBallHoleObjectId == 141)
					aniof(1, 7);

				if (_coreVar._atticRodHoleObjectId == 159)
					aniof(1, 6);
			} else if ((_caff == CELLAR) && (_coreVar._cellarObjectId == 151))
				aniof(1, 2);
			else if ((_caff == SECRET_PASSAGE) && (_coreVar._secretPassageObjectId == 143))
				aniof(1, 1);
			else if ((_caff == WELL) && (_coreVar._wellObjectId != 0))
				aniof(1, 1);
		}

		if (_caff < ROOM26)
			startMusicOrSpeech(1);
	}
}

void MortevielleEngine::drawPictureWithText() {
	_text.taffich();
	drawPicture();
	_destinationOk = false;
}

/**
 * Engine function - Place
 * @remarks	Originally called 'tkey1'
 */
void MortevielleEngine::testKey(bool d) {
	bool quest = false;
	int x, y;
	bool click;

	_mouse.hideMouse();
	displayStatusInDescriptionBar('K');

	// Wait for release from any key or mouse button
	while (keyPressed())
		_key = gettKeyPressed();

	do {
		_mouse.getMousePosition(x, y, click);
		keyPressed();
	} while (click);

	// Event loop
	do {
		if (d)
			prepareRoom();
		quest = keyPressed();
		_mouse.getMousePosition(x, y, click);
		if (shouldQuit())
			return;
	} while (!(quest || (click) || (d && _anyone)));
	if (quest)
		gettKeyPressed();
	setMouseClick(false);
	_mouse.showMouse();
}

void MortevielleEngine::tlu(int af, int ob) {
	_caff = 32;
	drawPictureWithText();
	handleDescriptionText(6, ob + 4000);
	handleDescriptionText(2, 999);
	testKey(true);
	_caff = af;
	_msg[3] = OPCODE_NONE;
	_crep = 998;
}

/**
 * Prepare Display Text
 * @remarks	Originally called 'affrep'
 */
void MortevielleEngine::prepareDisplayText() {
	_caff = _coreVar._currPlace;
	_crep = _coreVar._currPlace;
}

/**
 * Exit room
 * @remarks	Originally called 'tsort'
 */
void MortevielleEngine::exitRoom() {
	if ((_openObjCount > 0) && (_coreVar._currPlace != OWN_ROOM)) {
		if (_coreVar._faithScore < 50)
			_coreVar._faithScore += 2;
		else
			_coreVar._faithScore += (_coreVar._faithScore / 10);
	}

	resetOpenObjects();

	_roomDoorId = OWN_ROOM;
	_mchai = 0;
	resetRoomVariables(_coreVar._currPlace);
}

/**
 * get 'read' description
 * @remarks	Originally called 'st4'
 */
void MortevielleEngine::getReadDescription(int objId) {
	_crep = 997;

	switch (objId) {
	case 114 :
		_crep = 109;
		break;
	case 110 :
		_crep = 107;
		break;
	case 158 :
		_crep = 113;
		break;
	case 152:
	case 153:
	case 154:
	case 155:
	case 156:
	case 150:
	case 100:
	case 157:
	case 160:
	case 161 :
		tlu(_caff, objId);
		break;
	default:
		break;
	}
}

/**
 * get 'search' description
 * @remarks	Originally called 'st7'
 */
void MortevielleEngine::getSearchDescription(int objId) {
	switch (objId) {
	case 116:
	case 144:
		_crep = 104;
		break;
	case 126:
	case 111:
		_crep = 108;
		break;
	case 132:
		_crep = 111;
		break;
	case 142:
		_crep = 112;
		break;
	default:
		_crep = 183;
		getReadDescription(objId);
	}
}

void MortevielleEngine::mennor() {
	_menu.menuUp(_msg[3]);
}

void MortevielleEngine::premtet() {
	draw(kAdrDes, 10, 80);
	_screenSurface.drawBox(18, 79, 155, 91, 15);
}

void MortevielleEngine::ajchai() {
	int cy = kAcha + ((_mchai - 1) * 10) - 1;
	int cx = 0;
	do {
		++cx;
	} while ((cx <= 9) && (_tabdon[cy + cx] != 0));

	if (_tabdon[cy + cx] == 0)
		_tabdon[cy + cx] = _coreVar._selectedObjectId;
	else
		_crep = 192;
}

/**
 * Check if inventory is full and, if not, add object in it.
 * @remarks	Originally called 'ajjer'
 */
void MortevielleEngine::addObjectToInventory(int objectId) {
	int i = 0;
	do {
		++i;
	} while ((i <= 5) && (_coreVar._inventory[i] != 0));

	if (_coreVar._inventory[i] == 0) {
		_coreVar._inventory[i] = objectId;
		_menu.setInventoryText();
	} else
		// Inventory is full
		_crep = 139;
}

/**
 * Interact with NPC
 * @remarks	Originally called 'quelquun'
 */
void MortevielleEngine::interactNPC() {
	if (_menu._menuDisplayed)
		_menu.eraseMenu();

	endSearch();
	_crep = 997;
L1:
	if (!_hiddenHero) {
		if (_crep == 997)
			_crep = 138;
		handleDescriptionText(2, _crep);
		if (_crep == 138)
			_speechManager.startSpeech(5, 2, 1);
		else
			_speechManager.startSpeech(4, 4, 1);

		if (_openObjCount == 0)
			_coreVar._faithScore += 2;
		else if (_coreVar._faithScore < 50)
			_coreVar._faithScore += 4;
		else
			_coreVar._faithScore += 3 * (_coreVar._faithScore / 10);
		exitRoom();
		_menu.setDestinationText(LANDING);
		int cx = convertBitIndexToCharacterIndex(_currBitIndex);
		_caff = 69 + cx;
		_crep = _caff;
		_msg[3] = MENU_DISCUSS;
		_msg[4] = _menu._discussMenu[cx];
		_syn = true;
		_col = true;
	} else {
		if (getRandomNumber(1, 3) == 2) {
			_hiddenHero = false;
			_crep = 137;
			goto L1;
		} else {
			handleDescriptionText(2, 136);
			int rand = (getRandomNumber(0, 4)) - 2;
			_speechManager.startSpeech(3, rand, 1);
			clearDescriptionBar();
			displayAloneText();
			resetRoomVariables(MANOR_FRONT);
			prepareDisplayText();
		}
	}
	if (_menu._menuDisplayed)
		_menu.drawMenu();
}

void MortevielleEngine::tsuiv() {
	int tbcl;
	int cy = kAcha + ((_mchai - 1) * 10) - 1;
	int cx = 0;
	do {
		++cx;
		++_searchCount;
		int cl = cy + _searchCount;
		tbcl = _tabdon[cl];
	} while ((tbcl == 0) && (_searchCount <= 9));

	if ((tbcl != 0) && (_searchCount < 11)) {
		_caff = tbcl;
		_crep = _caff + 400;
		if (_currBitIndex != 0)
			_coreVar._faithScore += 2;
	} else {
		prepareDisplayText();
		endSearch();
		if (cx > 9)
			_crep = 131;
	}
}

/**
 * Display Arrow status
 * @remarks	Originally called 'tfleche'
 */
void MortevielleEngine::displayStatusArrow() {
	bool qust;
	char touch;

	if (_num == 9999)
		return;

	displayStatusInDescriptionBar(chr(152));
	bool inRect = false;
	do {
		touch = '\0';

		do {
			_mouse.moveMouse(qust, touch);
			if (shouldQuit())
				return;

			if (getMouseClick())
				inRect = (_mouse._pos.x < 256 * _resolutionScaler) && (_mouse._pos.y < 176) && (_mouse._pos.y > 12);
			prepareRoom();
		} while (!(qust || inRect || _anyone));

		if (qust && (touch == '\103'))
			_dialogManager.show(_hintPctMessage, 1);
	} while (!((touch == '\73') || ((touch == '\104') && (_x != 0) && (_y != 0)) || (_anyone) || (inRect)));

	if (touch == '\73')
		_keyPressedEsc = true;

	if (inRect) {
		_x = _mouse._pos.x;
		_y = _mouse._pos.y;
	}
}

/**
 * Set coordinates
 * @remarks	Originally called 'tcoord'
 */
void MortevielleEngine::setCoordinates(int sx) {
	int sy, ix, iy;
	int ib;


	_num = 0;
	_crep = 999;
	int a = 0;
	int atdon = kAmzon + 3;
	int cy = 0;
	while (cy < _caff) {
		a += _tabdon[atdon];
		atdon += 4;
		++cy;
	}

	if (_tabdon[atdon] == 0) {
		_crep = 997;
		return;
	}

	a += kFleche;
	int cb = 0;
	for (cy = 0; cy <= (sx - 2); ++cy) {
		ib = (_tabdon[a + cb] << 8) + _tabdon[(a + cb + 1)];
		cb += (ib * 4) + 2;
	}
	ib = (_tabdon[a + cb] << 8) + _tabdon[(a + cb + 1)];
	if (ib == 0) {
		_crep = 997;
		return;
	}

	cy = 1;
	do {
		cb += 2;
		sx = _tabdon[a + cb] * _resolutionScaler;
		sy = _tabdon[(a + cb + 1)];
		cb += 2;
		ix = _tabdon[a + cb] * _resolutionScaler;
		iy = _tabdon[(a + cb + 1)];
		++cy;
	} while (!(((_x >= sx) && (_x <= ix) && (_y >= sy) && (_y <= iy)) || (cy > ib)));

	if ((_x >= sx) && (_x <= ix) && (_y >= sy) && (_y <= iy)) {
		_num = cy - 1;
		return;
	}

	_crep = 997;
}

void MortevielleEngine::treg(int objId) {
	int mdes = _caff;
	_caff = objId;

	if (((_caff > 29) && (_caff < 33)) || (_caff == 144) || (_caff == 147) || (_caff == 149) || (_msg[4] == OPCODE_SLOOK)) {
		drawPictureWithText();
		if ((_caff > 29) && (_caff < 33))
			handleDescriptionText(2, _caff);
		else
			handleDescriptionText(2, _caff + 400);
		testKey(true);
		_caff = mdes;
		_msg[3] = MENU_NONE;
		_crep = 998;
	} else {
		_obpart = true;
		_crep = _caff + 400;
		_menu.setSearchMenu();
	}
}

/**
 * Engine function - Put in hand
 * @remarks	Originally called 'avpoing'
 */
void MortevielleEngine::putInHand(int &objId) {
	_crep = 999;
	if (_coreVar._selectedObjectId != 0)
		addObjectToInventory(_coreVar._selectedObjectId);

	// If inventory wasn't full
	if (_crep != 139) {
		displayItemInHand(objId + 400);
		_coreVar._selectedObjectId = objId;
		objId = 0;
	}
}

int MortevielleEngine::rechai() {
	int tmpPlace = _coreVar._currPlace;

	if (_coreVar._currPlace == CRYPT)
		tmpPlace = CELLAR;

	return _tabdon[kAchai + (tmpPlace * 7) + _num - 1];
}

/**
 * Check before leaving the secret passage
 * @remarks	Originally called 't23coul'
 */
int MortevielleEngine::checkLeaveSecretPassage() {
	if (!checkInventory(143)) {
		_crep = 1512;
		loseGame();
	}

	return CELLAR;
}

/**
 * Display status character in description bar
 * @remarks	Originally called 'fenat'
 */
void MortevielleEngine::displayStatusInDescriptionBar(char stat) {
	int color;

	_mouse.hideMouse();
	if (_currGraphicalDevice == MODE_CGA)
		color = 2;
	else if (_currGraphicalDevice == MODE_HERCULES)
		color = 1;
	else
		color = 12;

	_screenSurface.writeCharacter(Common::Point(306, 193), ord(stat), color);
	_screenSurface.drawBox(300, 191, 16, 8, 15);
	_mouse.showMouse();
}

/**
 * Test Keyboard
 * @remarks	Originally called 'teskbd'
 */
void MortevielleEngine::testKeyboard() {
	if (keyPressed())
		gettKeyPressed();
}

/**
 * Test Key Pressed
 * @remarks	Originally called 'testou'
 */
int MortevielleEngine::gettKeyPressed() {
	char ch = getChar();

	switch (ch)  {
	case '\23' :
		_soundOff = !_soundOff;
		break;
	case '\26' :
		if ((_x26KeyCount == 1) || (_x26KeyCount == 2)) {
			decodeNumber(&_cfiecBuffer[161 * 16], (_cfiecBufferSize - (161 * 16)) / 64);
			++_x26KeyCount;

			return 61;
		}
		break;
	case '\33' :
		if (keyPressed())
			ch = getChar();
		break;
	default:
		break;
	}

	return ord(ch);
}

} // End of namespace Mortevielle
