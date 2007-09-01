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

#include "agi/preagi_mickey.h"
#include "agi/graphics.h"

namespace Agi {

Mickey::Mickey(PreAgiEngine *vm) : _vm(vm) {
}

Mickey::~Mickey() {
}

void Mickey::init() {
}

void Mickey::run() {
	intro();
	while(true) {
		_vm->getSelection(0);
		_vm->_system->delayMillis(10);
	}
	//gameLoop();
	//gameOver();
}

void Mickey::intro() {
	// draw sierra logo
	drawLogo();
	
	// draw title picture
	game.iRoom = IDI_MSA_PIC_TITLE;
	drawRoom();

#if 0
	// show copyright and play theme
	PrintExeMsg(IDO_MSA_COPYRIGHT);
	PlaySound(IDI_MSA_SND_THEME);
		
	// load game
	game.fIntro = true;
	if (ChooseY_N(IDO_MSA_LOAD_GAME[0], true)) {
		if (LoadGame()) {
			game.iPlanet = IDI_MSA_PLANET_EARTH;
			game.fIntro = false;
			game.iRoom = IDI_MSA_PIC_SHIP_CORRIDOR;
			return;
		}
	}

	// play spaceship landing scene
	game.iPlanet = IDI_MSA_PLANET_EARTH;
	game.iRoom = IDI_MSA_PIC_EARTH_ROAD_4;

	DrawRoom();
	PrintRoomDesc();

	#ifndef _DEBUG
	PlaySound(IDI_MSA_SND_SHIP_LAND);
	#endif

	FlashScreen();
	FlashScreen();
	FlashScreen();

	PrintExeMsg(IDO_MSA_INTRO);
#endif
}

void Mickey::drawLogo() {
	char szFile[256] = {0};
	uint8 *buffer = new uint8[16384];

	// read in logos.bcg
	sprintf(szFile, IDS_MSA_PATH_LOGO);
	Common::File infile;
	if (!infile.open(szFile))
		return;
	infile.read(buffer, infile.size());
	infile.close();
		
	// draw logo bitmap
	// TODO
	//drawPictureBCG(buffer);
	//updateScreen();

	delete [] buffer;
}

void Mickey::drawRoomPicture() {
	if (false) { //(getDebug()) {
		drawPic(0);
		//debug();	// TODO
	} else {
		if (game.iRoom == IDI_MSA_PIC_TITLE) {
			drawPic(IDI_MSA_PIC_TITLE);
		} else {
			drawPic(game.iRmPic[game.iRoom]);
		}
	}
}

void Mickey::drawPic(int iPic) {
	_vm->preAgiLoadResource(rPICTURE, iPic);
	_vm->_picture->decodePicture(iPic, true);
	_vm->_picture->showPic();
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
}

void Mickey::drawRoom() {
	drawRoomPicture();
	//drawRoomObjects();
	//drawRoomAnimation();
}

}
