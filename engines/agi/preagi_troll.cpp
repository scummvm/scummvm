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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "agi/preagi.h"
#include "agi/preagi_common.h"
#include "agi/preagi_troll.h"
#include "agi/graphics.h"

#include "common/events.h"

#include "graphics/cursorman.h"

namespace Agi {

Troll::Troll(PreAgiEngine* vm) : _vm(vm) {
}

// User Interface

void Troll::pressAnyKey(int col) {
	_vm->drawStr(24, col, kColorDefault, IDS_TRO_PRESSANYKEY);
	_vm->_gfx->doUpdate();
	_vm->getSelection(kSelAnyKey);
}

void Troll::drawMenu(const char *szMenu, int iSel) {
	_vm->clearTextArea();
	_vm->drawStr(21, 0, kColorDefault, szMenu);
	_vm->drawStr(22 + iSel, 0, kColorDefault, " *");
	_vm->_gfx->doUpdate();
}

bool Troll::getMenuSel(const char *szMenu, int *iSel, int nSel) {
	Common::Event event;
	int y;

	drawMenu(szMenu, *iSel);

	while (!_vm->shouldQuit()) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				return 0;
			case Common::EVENT_MOUSEMOVE:
				y = event.mouse.y / 8;

				if (y >= 22)
					if (nSel > y - 22)
						*iSel = y - 22;

				drawMenu(szMenu, *iSel);
				break;
			case Common::EVENT_LBUTTONUP:
				return true;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_t:
				case Common::KEYCODE_f:
					inventory();

					return false;

					break;
				case Common::KEYCODE_DOWN:
				case Common::KEYCODE_SPACE:
					*iSel += 1;

					if (*iSel == nSel)
						*iSel = IDI_TRO_SEL_OPTION_1;

					drawMenu(szMenu, *iSel);
					break;
				case Common::KEYCODE_UP:
					*iSel -= 1;

					if (*iSel == IDI_TRO_SEL_OPTION_1 - 1)
						*iSel = nSel - 1;

					drawMenu(szMenu, *iSel);
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
					return true;
				case Common::KEYCODE_s:
					if (event.kbd.flags == Common::KBD_CTRL) {
						if (_soundOn) {
							playTune(2, 1);
							_soundOn = !_soundOn;
						} else {
							_soundOn = !_soundOn;
							playTune(3, 1);
						}
					}
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(10);
	}

	return true;
}

// Graphics

void Troll::drawPic(int iPic, bool f3IsCont, bool clr, bool troll) {
	_vm->_picture->setDimensions(IDI_TRO_PIC_WIDTH, IDI_TRO_PIC_HEIGHT);

	if (clr) {
		_vm->clearScreen(0x0f, false);
		_vm->_picture->clear();
	}

	_vm->_picture->setPictureData(_gameData + IDO_TRO_FRAMEPIC);
	_vm->_picture->drawPicture();

	_vm->_picture->setPictureData(_gameData + _pictureOffsets[iPic]);

	int addFlag = 0;

	if (troll)
		addFlag = kPicFTrollMode;

	if (f3IsCont) {
		_vm->_picture->setPictureFlags(kPicFf3Cont | addFlag);
	} else {
		_vm->_picture->setPictureFlags(kPicFf3Stop | addFlag);
	}

	_vm->_picture->drawPicture();

	_vm->_picture->showPic();
	_vm->_gfx->doUpdate();
}

// Game Logic

void Troll::inventory() {
	char tmp[40];
	int n;

	_vm->clearScreen(0x07);
	_vm->drawStr(1, 12, kColorDefault, IDS_TRO_TREASURE_0);
	_vm->drawStr(2, 12, kColorDefault, IDS_TRO_TREASURE_1);


	for (int i = 0; i < IDI_TRO_MAX_TREASURE - _treasuresLeft; i++) {
		n = _inventory[i] - 1;

		sprintf(tmp, " %2d ", i + 1);

		_vm->drawStr(2 + i, 10, _items[n].bg << 4 | 0x0f,  tmp);
		_vm->drawStr(2 + i, 14, _items[n].bg << 4 | _items[n].fg,  _items[n].name);
	}


	switch (_treasuresLeft) {
	case 1:
		sprintf(tmp, IDS_TRO_TREASURE_5, _treasuresLeft);
		_vm->drawStr(20, 10, kColorDefault, tmp);
		break;
	case 0:
		_vm->drawStr(20, 1, kColorDefault, IDS_TRO_TREASURE_6);
		break;
	case IDI_TRO_MAX_TREASURE:
		_vm->drawStr(3, 17, kColorDefault, IDS_TRO_TREASURE_2);
	default:
		sprintf(tmp, IDS_TRO_TREASURE_4, _treasuresLeft);
		_vm->drawStr(20, 10, kColorDefault, tmp);
		break;
	}

	pressAnyKey(6);
}

void Troll::waitAnyKeyIntro() {
	Common::Event event;
	int iMsg = 0;

	while (!_vm->shouldQuit()) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_KEYDOWN:
				return;
			default:
				break;
			}
		}

		switch (iMsg) {
		case 200:
			iMsg = 0;
		case 0:
			_vm->drawStr(22, 3, kColorDefault, IDS_TRO_INTRO_2);
			_vm->_gfx->doUpdate();
			break;
		case 100:
			_vm->drawStr(22, 3, kColorDefault, IDS_TRO_INTRO_3);
			_vm->_gfx->doUpdate();
			break;
		}

		iMsg++;

		_vm->_system->updateScreen();
		_vm->_system->delayMillis(10);
	}
}

void Troll::credits() {
	_vm->clearScreen(0x07);

	_vm->drawStr(1, 2, kColorDefault, IDS_TRO_CREDITS_0);

	int color = 10;
	char str[2];

	str[1] = 0;

	for (uint i = 0; i < strlen(IDS_TRO_CREDITS_1); i++) {
		str[0] = IDS_TRO_CREDITS_1[i];
		_vm->drawStr(7, 19 + i, color++, str);
		if (color > 15)
			color = 9;
	}

	_vm->drawStr(8, 19, kColorDefault, IDS_TRO_CREDITS_2);

	_vm->drawStr(13, 11, 9, IDS_TRO_CREDITS_3);
	_vm->drawStr(15, 8, 10, IDS_TRO_CREDITS_4);
	_vm->drawStr(17, 7, 12, IDS_TRO_CREDITS_5);
	_vm->drawStr(19, 2, 14, IDS_TRO_CREDITS_6);

	_vm->_gfx->doUpdate();

	pressAnyKey();
}

void Troll::tutorial() {
	bool done = false;
	int iSel = 0;
	//char szTreasure[16] = {0};

	while (!_vm->shouldQuit()) {
		_vm->clearScreen(0xFF);

		_vm->printStr(IDS_TRO_TUTORIAL_0);
		_vm->getSelection(kSelSpace);

		_vm->clearScreen(0x55);
		_vm->setDefaultTextColor(0x0F);

		done = false;
		while (!done && !_vm->shouldQuit()) {
			getMenuSel(IDS_TRO_TUTORIAL_1, &iSel, IDI_TRO_MAX_OPTION);

			switch (iSel) {
			case IDI_TRO_SEL_OPTION_1:
				_vm->clearScreen(0x22, false);
				_vm->_gfx->doUpdate();
				break;
			case IDI_TRO_SEL_OPTION_2:
				_vm->clearScreen(0x00, false);
				_vm->_gfx->doUpdate();
				break;
			case IDI_TRO_SEL_OPTION_3:
				done = true;
				break;
			}
		}

		// do you need more practice ?
		_vm->clearScreen(0x4F);
		_vm->drawStr(7, 4, kColorDefault, IDS_TRO_TUTORIAL_5);
		_vm->drawStr(9, 4, kColorDefault, IDS_TRO_TUTORIAL_6);
		_vm->_gfx->doUpdate();

		if (!_vm->getSelection(kSelYesNo))
			break;
	}

	// show info texts
	_vm->clearScreen(0x5F);
	_vm->drawStr(4, 1, kColorDefault, IDS_TRO_TUTORIAL_7);
	_vm->drawStr(5, 1, kColorDefault, IDS_TRO_TUTORIAL_8);
	_vm->_gfx->doUpdate();
	pressAnyKey();

	_vm->clearScreen(0x2F);
	_vm->drawStr(6, 1, kColorDefault, IDS_TRO_TUTORIAL_9);
	_vm->_gfx->doUpdate();
	pressAnyKey();

	_vm->clearScreen(0x19);
	_vm->drawStr(7, 1, kColorDefault, IDS_TRO_TUTORIAL_10);
	_vm->drawStr(8, 1, kColorDefault, IDS_TRO_TUTORIAL_11);
	_vm->_gfx->doUpdate();
	pressAnyKey();

	_vm->clearScreen(0x6E);
	_vm->drawStr(9, 1, kColorDefault, IDS_TRO_TUTORIAL_12);
	_vm->drawStr(10, 1, kColorDefault, IDS_TRO_TUTORIAL_13);
	_vm->_gfx->doUpdate();
	pressAnyKey();

	_vm->clearScreen(0x4C);
	_vm->drawStr(11, 1, kColorDefault, IDS_TRO_TUTORIAL_14);
	_vm->drawStr(12, 1, kColorDefault, IDS_TRO_TUTORIAL_15);
	_vm->_gfx->doUpdate();
	pressAnyKey();

	_vm->clearScreen(0x5D);
	_vm->drawStr(13, 1, kColorDefault, IDS_TRO_TUTORIAL_16);
	_vm->drawStr(14, 1, kColorDefault, IDS_TRO_TUTORIAL_17);
	_vm->drawStr(15, 1, kColorDefault, IDS_TRO_TUTORIAL_18);
	_vm->_gfx->doUpdate();
	pressAnyKey();

	// show treasures
	_vm->clearScreen(0x2A);
	_vm->drawStr(2, 1, kColorDefault, IDS_TRO_TUTORIAL_19);
	for (int i = 0; i < IDI_TRO_MAX_TREASURE; i++)
		_vm->drawStr(19 - i, 11, kColorDefault, _items[i].name);

	_vm->_gfx->doUpdate();

	pressAnyKey();
}

void Troll::intro() {
	// sierra on-line presents
	_vm->clearScreen(0x2F);
	_vm->drawStr(9, 10, kColorDefault, IDS_TRO_INTRO_0);
	_vm->drawStr(14, 15, kColorDefault, IDS_TRO_INTRO_1);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();
	_vm->_system->delayMillis(3200);

	CursorMan.showMouse(true);

	// Draw logo
	_vm->setDefaultTextColor(0x0f);
	drawPic(45, false, true);
	_vm->_gfx->doUpdate();

	// wait for keypress and alternate message
	waitAnyKeyIntro();

	// have you played this game before?
	_vm->drawStr(22, 3, kColorDefault, IDS_TRO_INTRO_4);
	_vm->drawStr(23, 6, kColorDefault, IDS_TRO_INTRO_5);
	_vm->_gfx->doUpdate();

	if (!_vm->getSelection(kSelYesNo))
		tutorial();

	credits();
}

void Troll::gameOver() {
	// We do a check to see if the game should quit. Without this, the game show the picture, plays the
	// music, and then quits. So if the game is quitting, we shouldn't run the "game over" part.
	if (_vm->shouldQuit())
		return;

	char szMoves[40];

	_vm->clearTextArea();
	drawPic(42, true, true);

	playTune(4, 25);

	printUserMessage(16);

	printUserMessage(33);

	_vm->clearTextArea();

	drawPic(46, true, true);

	sprintf(szMoves, IDS_TRO_GAMEOVER_0, _moves);
	_vm->drawStr(21, 1, kColorDefault, szMoves);
	_vm->drawStr(22, 1, kColorDefault, IDS_TRO_GAMEOVER_1);
	_vm->_gfx->doUpdate();

	pressAnyKey();
}

void Troll::drawTroll() {
	for (int i = 0; i < IDI_TRO_NUM_NONTROLL; i++)
		if (_currentRoom == _nonTrollRooms[i]) {
			_isTrollAway = true;
			return;
		}

	drawPic(43, false, false, true);
}

int Troll::drawRoom(char *menu) {
	int n = 0;
	bool contFlag = false;

	if (_currentRoom == 1) {
		_vm->_picture->setDimensions(IDI_TRO_PIC_WIDTH, IDI_TRO_PIC_HEIGHT);
		_vm->clearScreen(0x00, false);
		_vm->_picture->clear();
	} else {

		if (_currentRoom != 42) {
			if (_roomPicDeltas[_currentRoom]) {
				contFlag = true;
			}
		}

		drawPic(_currentRoom, contFlag, true);
		_vm->_gfx->doUpdate();

		if (_currentRoom == 42) {
			drawPic(44, false, false); // don't clear
		} else {
			if (!_isTrollAway) {
				drawTroll();
			}
		}
	}

	_vm->_gfx->doUpdate();

	char tmp[10];
	strncat(menu, (char*)_gameData + _locMessagesIdx[_currentRoom], 39);

	for (int i = 0; i < 3; i++) {
		if (_roomDescs[_roomPicture - 1].options[i]) {
			sprintf(tmp, "\n  %d.", i);
			strcat(menu, tmp);

			strncat(menu, (char *)_gameData + _options[_roomDescs[_roomPicture - 1].options[i]- 1], 35);

			n = i + 1;
		}
	}

	return n;
}

void Troll::playTune(int tune, int len) {
	if (!_soundOn)
		return;

	int freq, duration;
	int ptr = _tunes[tune - 1];

	for (int i = 0; i < len; i++) {
		freq = READ_LE_UINT16(_gameData + ptr);
		ptr += 2;
		duration = READ_LE_UINT16(_gameData + ptr);
		ptr += 2;

		_vm->playNote(freq, duration);
	}
}

void Troll::pickupTreasure(int treasureId) {
	char tmp[40];

	_inventory[IDI_TRO_MAX_TREASURE - _treasuresLeft] = treasureId;

	if (_currentRoom != 24) {
		_vm->clearTextArea();
		drawPic(_currentRoom, false, true);
		_vm->_gfx->doUpdate();
	}

	printUserMessage(treasureId + 16);

	_vm->clearTextArea();

	_treasuresLeft--;

	switch (_treasuresLeft) {
	case 1:
		_vm->drawStr(22, 1, kColorDefault, IDS_TRO_TREASURE_7);
		break;
	case 0:
		_vm->drawStr(22, 1, kColorDefault, IDS_TRO_TREASURE_8);
		_vm->drawStr(23, 4, kColorDefault, IDS_TRO_TREASURE_9);

		_roomStates[6] = 1;

		_locMessagesIdx[6] = IDO_TRO_ALLTREASURES;
		break;
	default:
		sprintf(tmp, IDS_TRO_TREASURE_3, _treasuresLeft);
		_vm->drawStr(22, 1, kColorDefault, tmp);
		break;
	}

	pressAnyKey();
}

void Troll::printUserMessage(int msgId) {
	int i;

	_vm->clearTextArea();

	for (i = 0; i < _userMessages[msgId - 1].num; i++) {
		_vm->drawStr(21 + i, 1, kColorDefault, _userMessages[msgId - 1].msg[i]);
	}

	if (msgId == 34) {
		for (i = 0; i < 2; i++)
			playTune(5, 11);
	}
	pressAnyKey();
}

void Troll::gameLoop() {
	bool done = false;
	char menu[160+5];
	int currentOption, numberOfOptions;
	int roomParam;
	int haveFlashlight;

	_moves = 0;
	_roomPicture = 1;
	_treasuresLeft = IDI_TRO_MAX_TREASURE;
	haveFlashlight = false;
	_currentRoom = 0;
	_isTrollAway = true;
	_soundOn = true;

	memset(_roomStates, 0, sizeof(_roomStates));

	memset(_inventory, 0, sizeof(_inventory));

	while (!done && !_vm->shouldQuit()) {
		*menu = 0;

		currentOption = 0;

		numberOfOptions = drawRoom(menu);

		if (getMenuSel(menu, &currentOption, numberOfOptions)) {
			_moves++;
		} else {
			continue;
		}

		roomParam = _roomDescs[_roomPicture - 1].roomDescIndex[currentOption];

		switch (_roomDescs[_roomPicture - 1].optionTypes[currentOption]) {
		case OT_FLASHLIGHT:
			if (!haveFlashlight) {
				printUserMessage(13);
				break;
			}
			// fall down
		case OT_GO:
			_currentRoom = roomParam;
			_roomPicture = _roomPicStartIdx[_currentRoom];
			_roomPicture += _roomStates[_currentRoom];

			if (_currentRoom < 6 || _treasuresLeft == 0) {
				_isTrollAway = true;
			} else { // make odd 1:3
				_isTrollAway = (_vm->rnd(3) != 2);
			}
			break;
		case OT_GET:
			if (!_isTrollAway) {
				printUserMessage(34);
			} else {
				for (int i = 0; i < 4; i++) {
					playTune(1, 3);
					// delayMillis()
				}

				_roomStates[_currentRoom] = 1;
				_roomPicDeltas[_currentRoom] = 0;

				_roomPicture++;

				if (_roomConnects[roomParam - 1] != 0xff) {
					_roomStates[_roomConnects[roomParam - 1]] = 1;
				}

				if (roomParam == 1)
					haveFlashlight = true;

				_locMessagesIdx[_currentRoom] = IDO_TRO_LOCMESSAGES +
					(roomParam + 42) * 39;

				pickupTreasure(roomParam);
			}
			break;
		case OT_DO:
			if (roomParam != 16) {
				printUserMessage(roomParam);
				break;
			}

			done = true;
			break;
		default:
			break;
		}
	}

}

void Troll::fillOffsets() {
	int i;

	for (i = 0; i < IDI_TRO_PICNUM; i++)
		_pictureOffsets[i] = READ_LE_UINT16(_gameData + IDO_TRO_PIC_START + i * 2);

	for (i = 0; i < IDI_TRO_NUM_OPTIONS; i++)
		_options[i] = READ_LE_UINT16(_gameData + IDO_TRO_OPTIONS + i * 2);

	for (i = 0; i < IDI_TRO_NUM_NUMROOMS; i++) {
		_roomPicStartIdx[i] = _gameData[IDO_TRO_PICSTARTIDX + i];
		_roomPicDeltas[i] = _gameData[IDO_TRO_ROOMPICDELTAS + i];
		_roomConnects[i] = _gameData[IDO_TRO_ROOMCONNECTS + i];
	}

	for (i = 0; i < IDI_TRO_NUM_LOCDESCS; i++)
		_locMessagesIdx[i] = IDO_TRO_LOCMESSAGES + i * 39;

	int start = READ_LE_UINT16(_gameData + IDO_TRO_ROOMDESCS);
	int ptr;
	int j;

	for (i = 0; i < IDI_TRO_NUM_ROOMDESCS; i++, start += 2) {
		ptr = READ_LE_UINT16(_gameData + start);

		for (j = 0; j < 3; j++)
			_roomDescs[i].options[j] = _gameData[ptr++];

		for (j = 0; j < 3; j++) {
			switch (_gameData[ptr++]) {
			case 0:
				_roomDescs[i].optionTypes[j] = OT_GO;
				break;
			case 1:
				_roomDescs[i].optionTypes[j] = OT_GET;
				break;
			case 2:
				_roomDescs[i].optionTypes[j] = OT_DO;
				break;
			case 3:
				_roomDescs[i].optionTypes[j] = OT_FLASHLIGHT;
				break;
			default:
				error("Bad data @ (%x) %d", ptr - 1, i);
			}
		}

		for (j = 0; j < 3; j++)
			_roomDescs[i].roomDescIndex[j] = _gameData[ptr++];
	}

	start = IDO_TRO_USERMESSAGES;

	for (i = 0; i < IDI_TRO_NUM_USERMSGS; i++, start += 2) {
		ptr = READ_LE_UINT16(_gameData + start);

		_userMessages[i].num = _gameData[ptr++];

		for (j = 0; j < _userMessages[i].num; j++, ptr += 39) {
			memcpy(_userMessages[i].msg[j], _gameData + ptr, 39);
			_userMessages[i].msg[j][39] = 0;
		}
	}

	start = IDO_TRO_ITEMS;

	for (i = 0; i < IDI_TRO_MAX_TREASURE; i++, start += 2) {
		ptr = READ_LE_UINT16(_gameData + start);
		_items[i].bg = _gameData[ptr++];
		_items[i].fg = _gameData[ptr++];
		memcpy(_items[i].name, _gameData + ptr, 15);
		_items[i].name[15] = 0;
	}

	for (i = 0; i < IDO_TRO_NONTROLLROOMS; i++)
		_nonTrollRooms[i] = _gameData[IDO_TRO_NONTROLLROOMS + i];

	_tunes[0] = 0x3BFD;
	_tunes[1] = 0x3C09;
	_tunes[2] = 0x3C0D;
	_tunes[3] = 0x3C11;
	_tunes[4] = 0x3C79;
	_tunes[5] = 0x3CA5;
}

// Init

void Troll::init() {
	_vm->_picture->setPictureVersion(AGIPIC_V15);
	//SetScreenPar(320, 200, (char*)ibm_fontdata);

	const int gaps[] = { 0x3A40,  0x4600,  0x4800,  0x5800,  0x5a00,  0x6a00,
						 0x6c00,  0x7400,  0x7600,  0x7c00,  0x7e00,  0x8e00,
						 0x9000,  0xa000,  0xa200,  0xb200,  0xb400,  0xc400,
						 0xc600,  0xd600,  0xd800,  0xe800,  0xea00,  0xfa00,
						 0xfc00,  0x10c00, 0x10e00, 0x11e00, 0x12000, 0x13000 };

	Common::File infile;
	if (!infile.open(IDA_TRO_BINNAME))
		return;

	_gameData = (byte *)malloc(0xD9C0);

	bool flip = true;
	byte *ptr = _gameData;
	int diff;

	for (int i = 0; i < ARRAYSIZE(gaps) - 1; i++) {
		diff = gaps[i + 1] - gaps[i];

		if (flip) {
			infile.seek(gaps[i]);
			infile.read(ptr, diff);
			ptr += diff;
		} else {
		}
		flip = !flip;
	}

	// One sector is off
	infile.seek(0x18470);
	infile.read(_gameData + 15632, 592);

	infile.close();

	fillOffsets();
}

void Troll::run() {
	while (!_vm->shouldQuit()) {
		intro();
		gameLoop();
		gameOver();
	}
}

} // end of namespace Agi
