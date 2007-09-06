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

#include "agi/preagi_winnie.h"
#include "agi/graphics.h"

namespace Agi {

// default attributes
#define IDA_DEFAULT		0x0F
#define IDA_DEFAULT_REV	0xF0

void Winnie::initEngine() {
	//SetScreenPar(320, 200, (char*)ibm_fontdata);
	//SetMenuPars(21, 21, IDS_WTP_SELECTION);
}

void Winnie::initVars() {
	memset(&game, 0, sizeof(game));
	game.fSound = 1;
	game.nObjMiss = IDI_WTP_MAX_OBJ_MISSING;
	game.nObjRet = 0;
	game.fGame[0] = 1;
	game.fGame[1] = 1;
	room = IDI_WTP_ROOM_HOME;

	mist = -1;
	wind = false;
	event = false;
}

void Winnie::randomize() {
	int iObj = 0;
	int iRoom = 0;
	bool done;

	for (int i = 0; i < IDI_WTP_MAX_OBJ_MISSING; i++) {
		done = false;
		while (!done) {
			iObj = _vm->rnd(IDI_WTP_MAX_OBJ - 1) + 2;
			done = true;
			for (int j = 0; j < IDI_WTP_MAX_OBJ_MISSING; j++) {
				if (game.iUsedObj[j] == iObj) {
					done = false;
					break;
				}
			}
		}

		game.iUsedObj[i] = iObj;
		
		done = false;
		while (!done) {
			iRoom = _vm->rnd(IDI_WTP_MAX_ROOM_NORMAL) + 1;
			done = true;
			for (int j = 0; j < IDI_WTP_MAX_ROOM_OBJ; j++) {
				if (game.iObjRoom[j] == iRoom) {
					done = false;
					break;
				}
			}
		}

		game.iObjRoom[iObj] = iRoom;
	}
}

void Winnie::intro() {
	intro_DrawLogo();
	intro_DrawTitle();
	intro_PlayTheme();
}

void Winnie::intro_DrawLogo() {
	drawPic(IDS_WTP_FILE_LOGO);
	printStr(IDS_WTP_INTRO_0);
	_vm->waitAnyKeyChoice();
}

void Winnie::intro_DrawTitle() {
	drawPic(IDS_WTP_FILE_TITLE);
	printStr(IDS_WTP_INTRO_1);
	_vm->waitAnyKeyChoice();
}

void Winnie::intro_PlayTheme() {
	//if (!Winnie_PlaySound(IDI_WTP_SND_POOH_0)) return;
	//if (!Winnie_PlaySound(IDI_WTP_SND_POOH_1)) return;
	//if (!Winnie_PlaySound(IDI_WTP_SND_POOH_2)) return;
}

void Winnie::drawPic(const char *szName) {
	char szFile[256] = {0};
	uint8 *buffer = new uint8[4096];

	// construct filename
	sprintf(szFile, IDS_WTP_PATH, szName);

	_vm->preAgiLoadResource(rPICTURE, szName);
	_vm->_picture->decodePicture(0, true, false, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_picture->showPic(IDI_WTP_PIC_X0, IDI_WTP_PIC_Y0, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
	_vm->preAgiUnloadResource(rPICTURE, 0);

	delete [] buffer;
}

void Winnie::printStr(const char* szMsg) {
	_vm->clearTextArea();
	_vm->drawStr(21, 0, IDA_DEFAULT, szMsg);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();
}

Winnie::Winnie(PreAgiEngine* vm) : _vm(vm) {

}

void Winnie::init() {
	initEngine();
	initVars();
}

void Winnie::run() {
	randomize();
	intro();
	//gameLoop();
}

}
