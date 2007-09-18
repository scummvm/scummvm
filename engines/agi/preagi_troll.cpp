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

#include "graphics/cursorman.h"

#include "common/events.h"

namespace Agi {

Troll::Troll(PreAgiEngine* vm) : _vm(vm) {
}

// User Interface

void Troll::pressAnyKey() {
	_vm->drawStr(24, 4, kColorDefault, IDS_TRO_PRESSANYKEY);
	_vm->_gfx->doUpdate();
	_vm->waitAnyKeyChoice();
}

void Troll::drawMenu(const char *szMenu, int iSel) {
	_vm->clearTextArea();
	_vm->drawStr(21, 0, kColorDefault, szMenu);
	_vm->drawStr(22 + iSel, 0, kColorDefault, " *");
	_vm->_gfx->doUpdate();
}

void Troll::getMenuSel(const char *szMenu, int *iSel, int nSel) {
	Common::Event event;
	int y;
	
	drawMenu(szMenu, *iSel);

	for (;;) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				_vm->_system->quit();
			case Common::EVENT_MOUSEMOVE:
				y = event.mouse.y / 8;

				if (y >= 22)
					if (nSel > y - 22)
						*iSel = y - 22;

				drawMenu(szMenu, *iSel);
				break;
			case Common::EVENT_LBUTTONUP:
				return;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_t:
				case Common::KEYCODE_f:
					inventory();
					*iSel = 0;
					drawMenu(szMenu, *iSel);
					break;
				case Common::KEYCODE_SPACE:
					*iSel += 1;

					if (*iSel == nSel)
						*iSel = IDI_TRO_SEL_OPTION_1;

					drawMenu(szMenu, *iSel);
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
					return;
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
}

// Graphics

void Troll::drawPic(int iPic, bool f3IsCont, bool clear) {
	uint8 frame[] = {
		0xf1, 0x3, 0xf9, 0x0, 0x0, 0x9f, 0x0, 0x9f, 0xa7, 0x0, 0xa7, 0x0, 0x0, 0xff
	};

	if (clear)
		_vm->clearScreen(0x0f);

	_vm->_picture->setDimensions(IDI_TRO_PIC_WIDTH, IDI_TRO_PIC_HEIGHT);

	_vm->_picture->setPictureData(frame, ARRAYSIZE(frame));
	_vm->_picture->drawPicture();

	_vm->_picture->setPictureData(_gameData + _pictureOffsets[iPic], 4096);

	if (f3IsCont)
		_vm->_picture->setPictureFlags(kPicFf3Cont);
	else
		_vm->_picture->setPictureFlags(kPicFf3Stop);

	_vm->_picture->drawPicture();

	_vm->_picture->showPic();
	_vm->_gfx->doUpdate();
}

// Game Logic

void Troll::inventory() {
	char szMissing[40];

	_vm->clearScreen(0x07);
	_vm->drawStr(1, 12, kColorDefault, IDS_TRO_TREASURE_0);
	_vm->drawStr(2, 12, kColorDefault, IDS_TRO_TREASURE_1);

	switch (_treasuresLeft) {
	case 1:
		sprintf(szMissing, IDS_TRO_TREASURE_5, _treasuresLeft);
		_vm->drawStr(20, 10,kColorDefault,  szMissing);
		break;
	case 0:
		_vm->drawStr(20, 1, kColorDefault, IDS_TRO_TREASURE_6);
		break;
	case IDI_TRO_MAX_TREASURE:
		_vm->drawStr(3, 17, kColorDefault, IDS_TRO_TREASURE_2);
	default:
		sprintf(szMissing, IDS_TRO_TREASURE_4, _treasuresLeft);
		_vm->drawStr(20, 10,kColorDefault,  szMissing);
		break;
	}	

	_vm->drawStr(24, 6, kColorDefault, IDS_TRO_PRESSANYKEY);
	_vm->_gfx->doUpdate();
	_vm->waitAnyKeyChoice();
}

void Troll::waitAnyKeyIntro() {
	Common::Event event;
	int iMsg = 0;

	for (;;) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				_vm->_system->quit();
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

	_vm->drawStr(7, 19, 10, "T");
	_vm->drawStr(7, 20, 11, "R");
	_vm->drawStr(7, 21, 12, "O");
	_vm->drawStr(7, 22, 13, "L");
	_vm->drawStr(7, 23, 14, "L");
	_vm->drawStr(7, 24, 15, "'");
	_vm->drawStr(7, 25, 9, "S");
	_vm->drawStr(7, 27, 11, "T");
	_vm->drawStr(7, 28, 12, "A");
	_vm->drawStr(7, 29, 13, "L");
	_vm->drawStr(7, 30, 14, "E");
	_vm->drawStr(7, 32, 9, "(");
	_vm->drawStr(7, 33, 10, "t");
	_vm->drawStr(7, 34, 11, "m");
	_vm->drawStr(7, 35, 12, ")");

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

	for (;;) {
		_vm->clearScreen(0xFF);

		_vm->printStr(IDS_TRO_TUTORIAL_0);
		_vm->getSelection(kSelSpace);
		
		_vm->clearScreen(0x55);
		_vm->setDefaultTextColor(0x0F);

		done = false;
		while (!done) {
			getMenuSel(IDS_TRO_TUTORIAL_1, &iSel, IDI_TRO_MAX_OPTION);
			switch(iSel) {
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
		_vm->drawStr(19 - i, 11, kColorDefault, (const char *)IDS_TRO_NAME_TREASURE[i]);

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

	// draw troll picture
	_vm->_system->delayMillis(320);

	// Draw logo
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
	char szMoves[40];

	_vm->clearScreen(0x0f); // hack

	_vm->clearTextArea();
	//DrawPic(0);

	_vm->clearTextArea();
	//DrawPic(0);

	sprintf(szMoves, IDS_TRO_GAMEOVER_0, _moves);
	_vm->drawStr(21, 1, kColorDefault, szMoves);
	_vm->drawStr(22, 1, kColorDefault, IDS_TRO_GAMEOVER_1);
	_vm->_gfx->doUpdate();
	pressAnyKey();
}

void Troll::gameLoop() {
	int iSel;
	//int nSel;
	bool done = false;
	//char szMsg[161];
	//char szMenu[161];
	
	while (!done) {
//		Troll_DrawRoomPic(_currentRoom);
//		Troll_ReadRoomText(_currentRoom, szMsg, szMenu, &nSel);

		iSel = 0;
		//getMenuSel(szMsg, szMenu, &iSel, nSel);
		_moves++;

		switch(iSel) {
		case IDI_TRO_SEL_OPTION_1:
			break;
		case IDI_TRO_SEL_OPTION_2:
			break;
		case IDI_TRO_SEL_OPTION_3:
			break;
		}
	}

}

void Troll::fillPicOffsets() {
	for (int i = 0; i < IDI_TRO_PICNUM; i++)
		_pictureOffsets[i] = READ_LE_UINT16(_gameData + IDO_TRO_PIC_START + i * 2) 
			+ IDO_TRO_DATA_START;
}

// Init

void Troll::init() {
	_moves = 0;
	_currentRoom = 1;
	_treasuresLeft = IDI_TRO_MAX_TREASURE;
	
	_vm->_picture->setPictureVersion(AGIPIC_V15);
	//SetScreenPar(320, 200, (char*)ibm_fontdata);

	Common::File infile;
	if (!infile.open(IDA_TRO_BINNAME))
		return;

	_gameData = (byte *)malloc(infile.size());
	infile.read(_gameData, infile.size());
	infile.close();

	fillPicOffsets();
}

void Troll::run() {
	intro();
	//gameLoop();
	gameOver();
}

} // end of namespace Agi
