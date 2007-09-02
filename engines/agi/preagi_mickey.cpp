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

#include "common/stdafx.h"
#include "common/events.h"

#include "agi/preagi_mickey.h"
#include "agi/graphics.h"

// default attributes
#define IDA_DEFAULT		0x0F
#define IDA_DEFAULT_REV	0xF0

namespace Agi {

int Mickey::getDat(int iRoom) {
	if (((iRoom > 0) && (iRoom < 24)) || iRoom == 154 || iRoom == 155) return IDI_MSA_PLANET_EARTH;
	if ((iRoom >= 30) && (iRoom <= 39)) return IDI_MSA_PLANET_VENUS;
	if ((iRoom >= 40) && (iRoom <= 69)) return IDI_MSA_PLANET_NEPTUNE;
	if ((iRoom >= 70) && (iRoom <= 82)) return IDI_MSA_PLANET_MERCURY;
	if ((iRoom >= 83) && (iRoom <= 92)) return IDI_MSA_PLANET_SATURN;
	if ((iRoom >= 93) && (iRoom <= 103)) return IDI_MSA_PLANET_PLUTO;
	if ((iRoom >= 106) && (iRoom <= 120)) return IDI_MSA_PLANET_JUPITER;
	if ((iRoom >= 121) && (iRoom <= 132)) return IDI_MSA_PLANET_MARS;
	if ((iRoom >= 133) && (iRoom <= 145)) return IDI_MSA_PLANET_URANUS;
	return IDI_MSA_PLANET_SPACESHIP;
}

void Mickey::readExe(int ofs, uint8 *buffer, long buflen) {
	Common::File infile;
	if (!infile.open(IDS_MSA_PATH_EXE))
		return;
	infile.seek(ofs, SEEK_SET);
	infile.read(buffer, buflen);
	infile.close();
}

void Mickey::getDatFileName(int iRoom, char *szFile) {
	sprintf(szFile, IDS_MSA_PATH_DAT, IDS_MSA_NAME_DAT[getDat(iRoom)]);
}

void Mickey::readDatHdr(char *szFile, MSA_DAT_HEADER *hdr) {
	Common::File infile;

	if (!infile.open(szFile))
		return;

	hdr->filelen = infile.readByte();
	hdr->filelen += infile.readByte() * 0x100;
	for (int i = 0; i < IDI_MSA_MAX_ROOM; i++) {
		hdr->ofsRoom[i] = infile.readByte();
		hdr->ofsRoom[i] += infile.readByte() * 0x100;
	}
	for (int i = 0; i < IDI_MSA_MAX_ROOM; i++) {
		hdr->ofsDesc[i] = infile.readByte();
		hdr->ofsDesc[i] += infile.readByte() * 0x100;
	}
	for (int i = 0; i < IDI_MSA_MAX_ROOM; i++) {
		hdr->ofsStr[i] = infile.readByte();
		hdr->ofsStr[i] += infile.readByte() * 0x100;
	}

	infile.close();
}

void Mickey::readDesc(int iRoom, char *buffer, long buflen) {
	MSA_DAT_HEADER hdr;
	char szFile[256] = {0};
	
	getDatFileName(iRoom, szFile);
	readDatHdr(szFile, &hdr);
	
	Common::File infile;

	if (!infile.open(szFile))
		return;

	memset(buffer, 0, buflen);

	infile.seek(hdr.ofsDesc[iRoom - 1] + IDI_MSA_OFS_DAT, SEEK_SET);
	infile.read(buffer, buflen);
	infile.close();
}

void Mickey::readMenu(int iRoom, char *buffer) {
	MSA_DAT_HEADER hdr;
	char szFile[256] = {0};

	getDatFileName(iRoom, szFile);
	readDatHdr(szFile, &hdr);
	
	Common::File infile;

	if (!infile.open(szFile))
		return;

	infile.seek(hdr.ofsRoom[iRoom - 1] + IDI_MSA_OFS_DAT, SEEK_SET);
	infile.read((uint8 *)buffer, sizeof(MSA_MENU));
	infile.close();
}

void Mickey::readDatStr(int iDat, int iStr, char *buffer, long buflen) {
	MSA_DAT_HEADER hdr;
	char szFile[256] = {0};
	
	sprintf(szFile, IDS_MSA_PATH_DAT, IDS_MSA_NAME_DAT[iDat]);
	readDatHdr(szFile, &hdr);

	Common::File infile;

	if (!infile.open(szFile))
		return;

	infile.seek(hdr.ofsStr[iStr] + IDI_MSA_OFS_DAT, SEEK_SET);
	infile.read((uint8 *)buffer, buflen);
	infile.close();
}

void Mickey::readOfsData(int offset, int iItem, uint8 *buffer, long buflen) {
	uint16 ofs[256];

	readExe(offset, buffer, buflen);
//	memcpy(ofs, buffer, sizeof(ofs));
	for (int i = 0; i < 256; i++)
		ofs[i] = buffer[i*2] + 256 * buffer[i*2+1];
	readExe(ofs[iItem] + IDI_MSA_OFS_EXE, buffer, buflen);
}

// User Interface

bool Mickey::chooseY_N(int ofsPrompt, bool fErrorMsg) {
	printExeStr(ofsPrompt);

	int a = _vm->getSelection(0);
	for (;;) {
		switch (a) {
			case 0: return false;
			case 1: return true;
			default: if (fErrorMsg) {
						printExeStr(IDO_MSA_PRESS_YES_OR_NO);
						_vm->waitAnyKey();
						printExeStr(ofsPrompt);
					}
					break;
		}
		a = _vm->getSelection(0);
	}
}

int Mickey::choose1to9(int ofsPrompt) {
	printExeStr(ofsPrompt);

	int a = _vm->getSelection(1);
	for (;;) {
		if (a == 10) {
			printExeStr(IDO_MSA_PRESS_1_TO_9);
			if (!_vm->waitAnyKeyChoice())
				return 0;
			printExeStr(ofsPrompt);
		} else return a;
		a = _vm->getSelection(1);
	}

}

void Mickey::printStr(char *buffer) {
	int pc = 1;
	int nRows, iCol, iRow;
	
	nRows = *buffer + IDI_MSA_ROW_MENU_0;

	_vm->clearTextArea();

	for (iRow = IDI_MSA_ROW_MENU_0; iRow < nRows; iRow++) {
		iCol = *(buffer + pc++);
		_vm->drawStr(iRow, iCol, IDA_DEFAULT, buffer + pc);
		pc += strlen(buffer + pc) + 1;
	}
}

void Mickey::printExeStr(int ofs) {
	uint8 buffer[256] = {0};

	if (!ofs)
		return;

	readExe(ofs, buffer, sizeof(buffer));
	printStr((char *)buffer);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop

}

void Mickey::printExeMsg(int ofs) {
	if (!ofs)
		return;
	printExeStr(ofs);
	_vm->waitAnyKeyAnim();
}

void Mickey::printDatStr(int iDat, int iStr) {
	char *buffer = (char *)malloc(256);
	readDatStr(iDat, iStr, buffer, 256);
	printStr(buffer);
	free(buffer);
}

void Mickey::printDesc(int iRoom) {
	char *buffer = (char *)malloc(256);
	readDesc(iRoom, buffer, 256);
	printStr(buffer);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
	free(buffer);
}

void Mickey::drawMenu(MSA_MENU menu, int sel0, int sel1) {
	int iWord;
	int iRow;
	int sel;
	uint8 attr;

	// draw menu

	_vm->clearTextArea();

	for (iRow = 0; iRow < 2; iRow++) {
		for (iWord = 0; iWord < menu.row[iRow].count; iWord++) {
			if (iRow)
				sel = sel1; 
			else 
				sel = sel0;

			if (iWord == sel) 
				attr = IDA_DEFAULT_REV; 
			else 
				attr = IDA_DEFAULT;

			_vm->drawStr(IDI_MSA_ROW_MENU_0 + iRow, menu.row[iRow].entry[iWord].x0, 
							 attr, (char *)menu.row[iRow].entry[iWord].szText);
		}
	}

	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
}

void Mickey::getMouseMenuSelRow(MSA_MENU menu, int *sel0, int *sel1, int iRow, int x, int y) {
	int iWord;
	int *sel = 0;

	switch(iRow) {
	case 0:
		if (y != IDI_MSA_ROW_MENU_0) return;
		sel = sel0;
		break;
	case 1:
		if (y != IDI_MSA_ROW_MENU_1) return;
		sel = sel1;
		break;
	}

	for (iWord = 0; iWord < menu.row[iRow].count; iWord++) {
		if ((x >= menu.row[iRow].entry[iWord].x0) && 
			(x < (int)(menu.row[iRow].entry[iWord].x0 + 
			strlen((char *)menu.row[iRow].entry[iWord].szText)))) {
				*sel = iWord;
				break;
		}
	}
}

bool Mickey::getMenuSelRow(MSA_MENU menu, int *sel0, int *sel1, int iRow) {
	Common::Event event;
	int *sel = 0;
	int nWords;
	int x, y;

	switch(iRow) {
	case 0:
		sel = sel0;
		break;
	case 1:
		sel = sel1;
		break;
	}
	nWords = menu.row[iRow].count;

	drawMenu(menu, *sel0, *sel1);

	for (;;) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				exit(0);
			case Common::EVENT_MOUSEMOVE:
				if (iRow < 2) {
					x = event.mouse.x / 8;
					y = event.mouse.y / 8;
					getMouseMenuSelRow(menu, sel0, sel1, iRow, x, y);
					drawMenu(menu, *sel0, *sel1);
				}
				break;
			case Common::EVENT_LBUTTONUP:
				return true;
			//FIXME: ScummVM does not support middle button
			/*case Common::EVENT_MBUTTONUP:
				inventory();
				drawRoom();
				*sel0 = 0; *sel1 = -1;
				return false;*/
			case Common::EVENT_RBUTTONUP:
				*sel0 = 0; *sel1 = -1; 
				return false;
			case Common::EVENT_WHEELUP:
				if (iRow < 2) {
					*sel -= 1;
					if (*sel < 0) *sel = nWords - 1;
					drawMenu(menu, *sel0, *sel1);
				}
				break;
			case Common::EVENT_WHEELDOWN:
				if (iRow < 2) {
					*sel += 1;
					if (*sel == nWords) *sel = 0;
					drawMenu(menu, *sel0, *sel1);
				}
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_2:
					hidden();
					break;
				case Common::KEYCODE_8:
					if (event.kbd.flags & Common::KBD_CTRL) {
						*sel0 = 0; *sel1 = -1; return false;
					}
					break;
				case Common::KEYCODE_ESCAPE:
					*sel0 = 0; *sel1 = -1; return false;
				case Common::KEYCODE_s:
					//flipSound();	// TODO
					break;
				case Common::KEYCODE_c:
					inventory();
					drawRoom();
					*sel0 = 0; *sel1 = -1; return false;
				case Common::KEYCODE_b:
					printRoomDesc();
					drawMenu(menu, *sel0, *sel1);
					*sel0 = 0; *sel1 = -1; return false;
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_KP4:
				case Common::KEYCODE_4:
					if (iRow < 2) {
						*sel -= 1;
						if (*sel < 0) *sel = nWords - 1;
						drawMenu(menu, *sel0, *sel1);
					}
					break;
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_SPACE:
				case Common::KEYCODE_KP6:
				case Common::KEYCODE_6:
					if (iRow < 2) {
						*sel += 1;
						if (*sel == nWords) *sel = 0;
						drawMenu(menu, *sel0, *sel1);
					}
					break;
				case Common::KEYCODE_RETURN:
				//FIXME:
				//case SDLK_KP_ENTER:
					return true;
				default:
					break;
				}
				break;
			default:
				break;
			}
			animate();
			drawMenu(menu, *sel0, *sel1);
		}
		animate();
		drawMenu(menu, *sel0, *sel1);
	}
}

void Mickey::getMenuSel(char *buffer, int *sel0, int *sel1) {
	MSA_MENU menu;

	memcpy(&menu, buffer, sizeof(MSA_MENU));

	*sel0 = 0;
	*sel1 = -1;

	for (;;) {
		for (;;) {
			if (getMenuSelRow(menu, sel0, sel1, 0)) {
				*sel1 = 0;
				if (getMenuSelRow(menu, sel0, sel1, 1)) {
					break;
				}
			}
		}
		if (getMenuSelRow(menu, sel0, sel1, 2)) {
			break;
		}
	}
}

void Mickey::centerMenu(MSA_MENU *menu) {
	int iWord;
	int iRow;
	int w, x;

	for (iRow = 0; iRow < 2; iRow++) {
		w = 0;
		for (iWord = 0; iWord < menu->row[iRow].count; iWord++) {
			w += strlen((char *)menu->row[iRow].entry[iWord].szText);
		}
		w += menu->row[iRow].count - 1;
		x = (40 - w) / 2;	// FIX
		for (iWord = 0; iWord < menu->row[iRow].count; iWord++) {
			menu->row[iRow].entry[iWord].x0 = x;
			x += strlen((char *)menu->row[iRow].entry[iWord].szText) + 1;
		}
	}
}

void Mickey::patchMenu(MSA_MENU *menu) {
	uint8 buffer[512];
	uint8 menubuf[sizeof(MSA_MENU)];
	int nPatches;
	int pBuf = 0;

	// change planet name in ship airlock menu
	if (game.iRoom == IDI_MSA_PIC_SHIP_AIRLOCK) {
		strcpy((char *)menu->row[1].entry[2].szText, IDS_MSA_NAME_PLANET[game.iPlanet]);
	}

	// exit if fix unnecessary
	if (!game.iRmMenu[game.iRoom]) {
		centerMenu(menu);
		return;
	}

	// copy menu to menubuf
	memcpy(menubuf, menu, sizeof(menubuf));

	// read patches
	readOfsData(
		IDOFS_MSA_MENU_PATCHES, 
		game.nRmMenu[game.iRoom] + game.iRmMenu[game.iRoom] - 1,
		buffer, sizeof(buffer)
	);

	// get number of patches
	nPatches = buffer[pBuf++];

	// patch menubuf
	for (int iPatch = 0; iPatch < nPatches; iPatch++) {
		if (buffer[pBuf] > sizeof(menubuf)) {
			// patch address out of bounds
		}
		menubuf[buffer[pBuf]] = buffer[pBuf + 1];
		pBuf += 2;
	}

	// copy menubuf back to menu
	memcpy(menu, menubuf, sizeof(MSA_MENU));

	// center menu
	centerMenu(menu);
}

void Mickey::printDatString(int iStr) {
	printDatStr(getDat(game.iRoom), iStr);
}

void Mickey::printDatMessage(int iStr) {
	printDatString(iStr);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
	_vm->waitAnyKeyAnim();
}

// Sound

// TODO
/*
void Mickey::_PlayNote(MSA_SND_NOTE note) {
	if (!note.counter)
		playNote(1, 0, note.length / IDI_SND_TIMER_RESOLUTION);
	else
		playNote(1, IDI_SND_OSCILLATOR_FREQUENCY / note.counter, 
			note.length / IDI_SND_TIMER_RESOLUTION / IDI_SND_PITCH);
}

void Mickey::PlaySound(ENUM_MSA_SOUND iSound) {
	if (!GetSound())
		return;

	SDL_Event event;
	MSA_SND_NOTE note;
	uint8 *buffer = new uint8[1024];
	int pBuf = 1;

	switch(iSound) {
	case IDI_MSA_SND_XL30:
		for (int iNote = 0; iNote < 6; iNote++) {
			note.counter = _vm->rnd(59600) + 59;
			note.length = 4;
			_PlayNote(note);
		}
		break;
	default:
		readOfsData(IDOFS_MSA_SOUND_DATA, iSound, buffer, 1024);

		for (;;) {
			memcpy(&note, buffer + pBuf, sizeof(note));
			if (!note.counter && !note.length)
				break;
			_PlayNote(note);
			pBuf += 3;

			if (iSound == IDI_MSA_SND_THEME) {
				while (SDL_PollEvent(&event)) {
					switch(event.type) {
					case SDL_QUIT:
						exit(0);
					case SDL_MOUSEBUTTONUP:
					case SDL_KEYDOWN:
						delete [] buffer;
						return;
					}
				}
			}
		}

		break;
	}

	delete [] buffer;
}
*/

void Mickey::debug() {
	char szLine[41] = {0};

	_vm->clearScreen(IDA_DEFAULT);

	sprintf(szLine, IDS_MSA_DEBUG_ROOM, game.iRoom);
	_vm->drawStr(5, 10, IDA_DEFAULT, szLine);

	if (game.iRoom < IDI_MSA_MAX_PIC_ROOM) {
		if (game.iRmObj[game.iRoom] != IDI_MSA_OBJECT_NONE) {
			sprintf(szLine, IDS_MSA_DEBUG_OBJ, game.iRmObj[game.iRoom]);
			_vm->drawStr(7, 10, IDA_DEFAULT, szLine);
		}
	} else {
		sprintf(szLine, IDS_MSA_DEBUG_OBJ, 32);
		_vm->drawStr(7, 10, IDA_DEFAULT, szLine);
	}
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
}

// Graphics

void Mickey::drawObj(ENUM_MSA_OBJECT iObj, int x0, int y0) {
	uint8 *buffer = new uint8[4096];
	char szFile[255] = {0};

	sprintf(szFile, IDS_MSA_PATH_OBJ, IDS_MSA_NAME_OBJ[iObj]);

	Common::File infile;

	if(!infile.open(szFile))
		return;
	
	
	infile.read(buffer, infile.size());

	// TODO
	/*
	if (iObj == IDI_MSA_OBJECT_CRYSTAL) {
		AGI_DrawPic(IDI_MSA_PIC_X0 + x0, IDI_MSA_PIC_Y0 + y0, IDF_AGI_PIC_V2 | IDF_AGI_STEP, buffer);
	} else {
		AGI_DrawPic(IDI_MSA_PIC_X0 + x0, IDI_MSA_PIC_Y0 + y0, IDF_AGI_PIC_V2, buffer);
	}
	*/

	infile.close();
	delete [] buffer;
}

void Mickey::drawPic(int iPic) {
	_vm->preAgiLoadResource(rPICTURE, iPic);
	// Note that decodePicture clears the screen
	_vm->_picture->decodePicture(iPic, true, false, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
	_vm->_picture->showPic(10, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
/*

	// Original code used in TrollVM

	int w = IDI_MSA_PIC_WIDTH, h = IDI_MSA_PIC_HEIGHT;
	int flags = IDF_AGI_PIC_V2;

	if (iPic == IDI_MSA_PIC_STAR_MAP) {
		w++; 
		h++;
		flags |= IDF_AGI_CIRCLE;
	}

	ClearScreenAGI(w, h, IDI_MSA_PIC_X0, IDI_MSA_PIC_Y0, flags);

	if (iPic) {
		uint8 *buffer = new uint8[4096];
		char szFile[255] = {0};

		sprintf(szFile, IDS_MSA_PATH_PIC, iPic);
		Common::File infile;
		if (!infile.open(szFile))
			return;
		infile.read(buffer, infile.size());
		infile.close();
		AGI_DrawPic(IDI_MSA_PIC_X0, IDI_MSA_PIC_Y0, flags, buffer);
		delete [] buffer;
	}
*/
}

void Mickey::drawRoomPicture() {
	if (false) {	// (getDebug()) {	// TODO
		drawPic(0);
		debug();
	} else {
		if (game.iRoom == IDI_MSA_PIC_TITLE) {
			drawPic(IDI_MSA_PIC_TITLE);
		} else {
			drawPic(game.iRmPic[game.iRoom]);
		}
	}
}

void Mickey::drawRoomObjects() {
	if (game.iRoom >= IDI_MSA_MAX_PIC_ROOM)
		return;

	uint8 buffer[256];
	int pBuf = 0;
	int nObjs;

	// draw ship control room window

	if (game.iRoom == IDI_MSA_PIC_SHIP_CONTROLS) {
		if (game.fFlying) {
			drawObj(IDI_MSA_OBJECT_W_SPACE, 0, 0);
		} else {
			drawObj((ENUM_MSA_OBJECT)(IDI_MSA_OBJECT_W_EARTH + game.iPlanet), 0, 1);
		}
	}

	// draw objects

	if (game.iRmObj[game.iRoom] != IDI_MSA_OBJECT_NONE) {
		readOfsData(IDO_MSA_ROOM_OBJECT_XY_OFFSETS, 
			game.iRmObj[game.iRoom], buffer, sizeof(buffer));

		nObjs = buffer[pBuf++];

		for (int iObj = 0; iObj < nObjs; iObj++) {
			drawObj((ENUM_MSA_OBJECT)buffer[pBuf], buffer[pBuf + 1], buffer[pBuf + 2]);
			pBuf += 3;
		}
	}
}

void Mickey::drawRoomAnimation() {
	uint8 objLight[] = {
		0xF0, 1, 0xF9, 2, 53, 45, 0xFF
	};

	switch(game.iRoom) {
	case IDI_MSA_PIC_EARTH_SHIP:
	case IDI_MSA_PIC_VENUS_SHIP:
	case IDI_MSA_PIC_NEPTUNE_SHIP:
	case IDI_MSA_PIC_MERCURY_SHIP:
	case IDI_MSA_PIC_SATURN_SHIP:
	case IDI_MSA_PIC_PLUTO_SHIP:
	case IDI_MSA_PIC_JUPITER_SHIP:
	case IDI_MSA_PIC_MARS_SHIP:
	case IDI_MSA_PIC_URANUS_SHIP:
	case IDI_MSA_PIC_SHIP_VENUS:
	case IDI_MSA_PIC_SHIP_NEPTUNE:
	case IDI_MSA_PIC_SHIP_MERCURY:
	case IDI_MSA_PIC_SHIP_SATURN:
	case IDI_MSA_PIC_SHIP_PLUTO:
	case IDI_MSA_PIC_SHIP_JUPITER:
	case IDI_MSA_PIC_SHIP_MARS:
	case IDI_MSA_PIC_SHIP_URANUS:

		// draw blinking ship lights

		int iColor;

		for (int i = 0; i < 12; i++) {
			iColor = game.nFrame + i;
			if (iColor > 15) iColor -= 15;

			objLight[1] = iColor;
			objLight[4] += 7;
			
			// TODO
			//AGI_DrawPic(0, 0, IDF_AGI_PIC_V2 | IDF_AGI_CIRCLE, (uint8 *)objLight);
		}

		game.nFrame--;
		if (game.nFrame < 0) game.nFrame = 15;

		// TODO
		//PlaySound(IDI_MSA_SND_PRESS_BLUE);

		break;

	case IDI_MSA_PIC_SHIP_CONTROLS:

		// draw XL30 screen

		if (game.fAnimXL30) {
			if (game.nFrame > 5) game.nFrame = 0;
			drawObj((ENUM_MSA_OBJECT)(IDI_MSA_OBJECT_XL31 + game.nFrame), 0, 4);
			game.nFrame++;
		};

		break;

	default:

		// draw crystal

		if (game.iRoom == IDI_MSA_XTAL_ROOM_XY[game.iPlanet][0]) {
			if (!game.fHasXtal) {
				switch(game.iPlanet) {
				case IDI_MSA_PLANET_VENUS:
					if (game.iRmMenu[game.iRoom] != 2) break;
				default:
					drawObj(
						IDI_MSA_OBJECT_CRYSTAL, 
						IDI_MSA_XTAL_ROOM_XY[game.iPlanet][1], 
						IDI_MSA_XTAL_ROOM_XY[game.iPlanet][2]
					);
					break;
				}
			}
		}

		break;
	}
}

void Mickey::drawRoom() {
	drawRoomPicture();
	drawRoomObjects();
	drawRoomAnimation();
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
	//drawPictureBCG(buffer);	// TODO
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop

	delete [] buffer;
}

void Mickey::animate() {
	// TODO
/*	if ((int)SDL_GetTicks() > (game.nTicks + IDI_MSA_ANIM_DELAY)) {
		game.nTicks = SDL_GetTicks();
		drawRoomAnimation();
	}
*/
}

void Mickey::printRoomDesc() {
	// print room description

	printDesc(game.iRoom);
	_vm->waitAnyKeyAnim();

	// print extended room description

	if (game.fRmTxt[game.iRoom]) {
		printExeMsg(game.oRmTxt[game.iRoom] + IDI_MSA_OFS_EXE);
	}
}

bool Mickey::loadGame() {
	Common::File infile;
	char szFile[256] = {0};
	bool diskerror = true;
	int sel;

	while (diskerror) {
		sel = choose1to9(IDO_MSA_LOAD_GAME[1]);
		if (!sel)
			return false;

		// load game
		//sprintf(szFile, "%s.s%2d", target, sel);	// TODO
		if (!infile.open(szFile)) {
			printExeStr(IDO_MSA_CHECK_DISK_DRIVE);
			if (!_vm->waitAnyKeyChoice())
				return false;
		} else {
			//infile->read(game, sizeof(MSA_GAME));
			diskerror = false;
			infile.close();			
		}
	}

	printExeMsg(IDO_MSA_LOAD_GAME[2]);
	return true;
}

void Mickey::saveGame() {
	Common::File outfile;
	//char szFile[256] = {0};
	bool diskerror = true;
	int sel;

	bool fOldDisk = chooseY_N(IDO_MSA_SAVE_GAME[0], false);

	if (fOldDisk)
		printExeStr(IDO_MSA_SAVE_GAME[1]);
	else
		printExeStr(IDO_MSA_SAVE_GAME[2]);

	if (!_vm->waitAnyKeyChoice())
		return;
	
	while (diskerror) {
		sel = choose1to9(IDO_MSA_SAVE_GAME[3]);
		if (!sel)
			return;

		if (fOldDisk)
			printExeStr(IDO_MSA_SAVE_GAME[5]);
		else
			printExeStr(IDO_MSA_SAVE_GAME[4]);

		if (!_vm->waitAnyKeyChoice())
			return;

		// save game
		// TODO
		/*
		sprintf(szFile, "%s.s%2d", target, sel);
		if (!outfile.open(szFile)) {
			printExeStr(IDO_MSA_CHECK_DISK_DRIVE);
			if (!_vm->waitAnyKeyChoice())
				return;
		} else {
			//outfile->write(game, sizeof(MSA_GAME))
			diskerror = false;
			outfile.close();						
		}
		*/
	}

	printExeMsg(IDO_MSA_SAVE_GAME[6]);
}

void Mickey::showPlanetInfo() {
	for (int i = 0; i < IDI_MSA_MAX_PLANET_INFO; i++) {
		printExeStr(IDO_MSA_PLANET_INFO[game.iPlanet][i]);
		_vm->waitAnyKey();
	}
}

void Mickey::printStory() {
	char buffer[IDI_MSA_LEN_STORY] = {0};
	char szLine[41] = {0};
	int iRow;
	int pBuf = 0;
	
	readExe(IDO_MSA_GAME_STORY, (uint8 *)buffer, sizeof(buffer));
	
	_vm->clearScreen(IDA_DEFAULT);
	for (iRow = 0; iRow < 25; iRow++) {
		strcpy(szLine, buffer + pBuf);
		_vm->drawStr(iRow, 0, IDA_DEFAULT, szLine);
		pBuf += strlen(szLine) + 1;
	}
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
	_vm->waitAnyKey();

	_vm->clearScreen(IDA_DEFAULT);
	for (iRow = 0; iRow < 21; iRow++) {
		strcpy(szLine, buffer + pBuf);
		_vm->drawStr(iRow, 0, IDA_DEFAULT, szLine);	// TODO
		pBuf += strlen(szLine) + 1;
	}
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
	_vm->waitAnyKey();

	drawRoom();
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop

	game.fStoryShown = true;
}

void Mickey::hidden() {
	if (game.iRoom == IDI_MSA_PIC_MERCURY_CAVE_0) {
		for (int i = 0; i < 5; i++) {
			printExeMsg(IDO_MSA_HIDDEN_MSG[i]);
		}
		_vm->clearTextArea();
		_vm->_gfx->doUpdate();
		_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
		_vm->waitAnyKey();
	}
}

int Mickey::getPlanet() {
	if (!game.nButtons)
		return -1;

	for (int iPlanet = 0; iPlanet < IDI_MSA_MAX_DAT; iPlanet++) {
		if (!strcmp(IDS_MSA_ADDR_PLANET[iPlanet], game.szAddr)) {
			return iPlanet;
		}
	}

	return -1;
}

void Mickey::pressOB(int iButton) {
	char szButtons[12] = {0};

	// check if too many buttons pressed
	if (game.nButtons == IDI_MSA_MAX_BUTTON) {
		game.nButtons = 0;
		memset(game.szAddr, 0, sizeof(game.szAddr));
		printExeMsg(IDO_MSA_TOO_MANY_BUTTONS_PRESSED);
		return;
	}
	
	// add button press to address
	game.nButtons++;
	game.szAddr[game.nButtons - 1] = (char)iButton;

	// format buttons string
	for (int i = 0; i < IDI_MSA_MAX_BUTTON; i++) {
		szButtons[i * 2] = game.szAddr[i];
		if (game.szAddr[i + 1]) szButtons[(i * 2) + 1] = ',';
	}

	// print pressed buttons
	printExeStr(IDO_MSA_MICKEY_HAS_PRESSED);
	_vm->drawStr(IDI_MSA_ROW_BUTTONS, IDI_MSA_COL_BUTTONS, IDA_DEFAULT, szButtons);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
	_vm->waitAnyKey();
}

void Mickey::checkAirSupply(bool fSuit, int *iSupply) {
	if (fSuit) {
		*iSupply -= 1;
		for (int i = 0; i < 4; i++) {
			if (*iSupply == IDI_MSA_AIR_SUPPLY[i]) {
				//playSound(IDI_MSA_SND_XL30);	// TODO
				printExeMsg(IDO_MSA_XL30_SPEAKING);
				printExeMsg(IDO_MSA_AIR_SUPPLY[i]);
				if (i == 3) {
					exit(0);
				}
			}
		}
	} else {
		*iSupply = IDI_MSA_MAX_AIR_SUPPLY;
	}
}

void Mickey::insertDisk(int iDisk) {
	_vm->clearTextArea();
	_vm->drawStr(IDI_MSA_ROW_INSERT_DISK, IDI_MSA_COL_INSERT_DISK, IDA_DEFAULT, (const char *)IDS_MSA_INSERT_DISK[iDisk]);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
	_vm->waitAnyKey();
}

void Mickey::gameOver() {
	drawPic(IDI_MSA_PIC_EARTH_SHIP_LEAVING);
	printExeMsg(IDO_MSA_GAME_OVER[3]);
	//playSound(IDI_MSA_SND_GAME_OVER);	// TODO

	if (game.fItemUsed[IDI_MSA_ITEM_LETTER]) {
		drawPic(IDI_MSA_PIC_EARTH_MINNIE);
		printExeMsg(IDO_MSA_GAME_OVER[4]);
		printExeMsg(IDO_MSA_GAME_OVER[5]);
	} else {
		printExeMsg(IDO_MSA_GAME_OVER[6]);
		printExeMsg(IDO_MSA_GAME_OVER[7]);
	}

	_vm->waitAnyKey();
	exit(0);
}

void Mickey::flipSwitch() {
	if (game.fHasXtal || game.nXtals) {
		if (!game.fStoryShown) {
			printStory();
			randomize();
		}

		// activate screen animation
		game.fAnimXL30 = true;

		_vm->clearTextArea();
		_vm->_gfx->doUpdate();
		_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
		//playSound(IDI_MSA_SND_XL30);	// TODO
		printExeMsg(IDO_MSA_XL30_SPEAKING);

		if (game.fHasXtal) {
			game.fHasXtal = false;
			printExeMsg(IDO_MSA_CRYSTAL_PIECE_FOUND);
		}
		
		if (game.nXtals == IDI_MSA_MAX_PLANET) {
			printExeMsg(IDO_MSA_GAME_OVER[0]);
			printExeMsg(IDO_MSA_GAME_OVER[1]);
			printExeMsg(IDO_MSA_GAME_OVER[2]);

#ifdef _DEBUG
			strcpy(game.szAddr, (char *)IDS_MSA_ADDR_PLANET[IDI_MSA_PLANET_EARTH]);
			game.nButtons = strlen(game.szAddr);
#endif

		} else {
			printExeStr(game.iClue[game.nXtals]);

#ifdef _DEBUG
			_vm->drawStr(24, 12, IDA_DEFAULT, (char *)IDS_MSA_NAME_PLANET_2[game.iPlanetXtal[game.nXtals]]);
			_vm->drawStr(24, 22, IDA_DEFAULT, (char *)IDS_MSA_ADDR_PLANET[game.iPlanetXtal[game.nXtals]]);
			strcpy(game.szAddr, (char *)IDS_MSA_ADDR_PLANET[game.iPlanetXtal[game.nXtals]]);
			game.nButtons = strlen(game.szAddr);
			_vm->_gfx->doUpdate();
			_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
#endif

			_vm->waitAnyKeyAnim();
		}
	} else {
		printStory();
	}
}

void Mickey::inventory() {
	int iRow = IDI_MSA_ROW_INV_ITEMS;
	char szCrystals[12] = {0};

	sprintf(szCrystals, IDS_MSA_CRYSTALS, IDS_MSA_CRYSTAL_NO[game.nXtals]);

	_vm->clearScreen(IDA_DEFAULT);
	_vm->drawStr(IDI_MSA_ROW_INV_TITLE, IDI_MSA_COL_INV_TITLE, IDA_DEFAULT, IDS_MSA_INVENTORY);
	_vm->drawStr(IDI_MSA_ROW_INV_CRYSTALS, IDI_MSA_COL_INV_ITEMS, IDA_DEFAULT, szCrystals);

	for (int iItem = 0; iItem < IDI_MSA_MAX_ITEM; iItem++) {
		if (game.fItem[game.iItem[iItem]] && (game.iItem[iItem] != IDI_MSA_OBJECT_NONE)) {
			_vm->drawStr(iRow++, IDI_MSA_COL_INV_ITEMS, IDA_DEFAULT, (const char *)IDS_MSA_NAME_ITEM[game.iItem[iItem]]);
		}
	}

	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
	_vm->waitAnyKey();

	_vm->clearScreen(IDA_DEFAULT);
}

void Mickey::randomize() {
	int iPlanet = 0;
	int iHint = 0;
	bool done;

	memset(game.iPlanetXtal, 0, sizeof(game.iPlanetXtal));
	memset(game.iClue, 0, sizeof(game.iClue));

	game.iPlanetXtal[0] = IDI_MSA_PLANET_EARTH;
	game.iPlanetXtal[8] = IDI_MSA_PLANET_URANUS;

	for (int i = 1; i < 9; i++) {
		if (i == 8) {
			iPlanet = IDI_MSA_PLANET_URANUS;
		} else {
			done = false;
			while (!done) {
				iPlanet = _vm->rnd(IDI_MSA_MAX_PLANET);
				done = true;
				for (int j = 0; j < IDI_MSA_MAX_PLANET; j++) {
					if (game.iPlanetXtal[j] == iPlanet) {
						done = false;
						break;
					}
				}
			}
		}

		game.iPlanetXtal[i] = iPlanet;
		
		done = false;
		while (!done) {
			iHint = _vm->rnd(5);
			done = true;
		}

		game.iClue[i] = IDO_MSA_NEXT_PIECE[iPlanet][iHint];
	}
}

void Mickey::flashScreen() {
	//playSound(IDI_MSA_SND_PRESS_BLUE);	// TODO

	//clearGfxScreen(15);	// TODO
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop

	drawRoom();
	printDesc(game.iRoom);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
}

void Mickey::intro() {
	// draw sierra logo
	drawLogo();

	// draw title picture
	game.iRoom = IDI_MSA_PIC_TITLE;
	drawRoom();
	
	// show copyright and play theme
	printExeMsg(IDO_MSA_COPYRIGHT);
	//playSound(IDI_MSA_SND_THEME);	// TODO
	
	// load game
	game.fIntro = true;
	if (chooseY_N(IDO_MSA_LOAD_GAME[0], true)) {
		if (loadGame()) {
			game.iPlanet = IDI_MSA_PLANET_EARTH;
			game.fIntro = false;
			game.iRoom = IDI_MSA_PIC_SHIP_CORRIDOR;
			return;
		}
	}

	// play spaceship landing scene
	game.iPlanet = IDI_MSA_PLANET_EARTH;
	game.iRoom = IDI_MSA_PIC_EARTH_ROAD_4;

	drawRoom();
	printRoomDesc();

	//playSound(IDI_MSA_SND_SHIP_LAND);	// TODO

	flashScreen();
	flashScreen();
	flashScreen();

	printExeMsg(IDO_MSA_INTRO);
}

void Mickey::getItem(ENUM_MSA_ITEM iItem) {
	game.fItem[iItem] = true;
	game.iItem[game.nItems++] = iItem;
	game.fRmTxt[game.iRoom] = 0;
	//playSound(IDI_MSA_SND_TAKE);	// TODO
	drawRoom();
}

void Mickey::getXtal(int iStr) {
	game.fRmTxt[game.iRoom] = 0;
	game.fHasXtal = true;
	game.nXtals++;
	//playSound(IDI_MSA_SND_CRYSTAL);	// TODO
	drawRoom();
	printDatMessage(iStr);
}

bool Mickey::parse(int cmd, int arg) {
	switch(cmd) {

	// BASIC

	case IDI_MSA_ACTION_GOTO_ROOM:
		game.iRoom = arg;
		return true;
	case IDI_MSA_ACTION_SHOW_INT_STR:
		printExeMsg(IDO_MSA_ERROR[arg]);
		break;
	case IDI_MSA_ACTION_SHOW_DAT_STR:
		printDatMessage(arg);
		break;

	// GENERAL

	case IDI_MSA_ACTION_PLANET_INFO:
		showPlanetInfo();
		break;
	case IDI_MSA_ACTION_SAVE_GAME:
		saveGame();
		break;
	case IDI_MSA_ACTION_LOOK_MICKEY:
		printExeMsg(IDO_MSA_YOU_CAN_SEE_MICKEY_ALREADY);
		break;

	// EARTH

	case IDI_MSA_ACTION_GET_ROPE:
		if (game.iRmMenu[game.iRoom] == 2) {
			game.iRmObj[game.iRoom] = IDI_MSA_OBJECT_NONE;
			game.iRmMenu[game.iRoom] = 3;
			getItem(IDI_MSA_ITEM_ROPE);
			printExeMsg(IDO_MSA_ERROR[7]);
		} else {
			game.iRmMenu[game.iRoom] = 1;
			printDatMessage(11);
		}
		break;
	case IDI_MSA_ACTION_UNTIE_ROPE:
		game.iRmPic[game.iRoom] = IDI_MSA_PIC_EARTH_TIRE_SWING_1;
		game.iRmObj[game.iRoom] = 0;
		game.iRmMenu[game.iRoom] = 2;
		drawRoom();
		printDatMessage(12);
		break;
	case IDI_MSA_ACTION_GET_BONE:
		game.iRmObj[game.iRoom] = IDI_MSA_OBJECT_NONE;
		game.iRmMenu[game.iRoom] = 1;
		getItem(IDI_MSA_ITEM_BONE);
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_GET_XTAL_EARTH:
		game.iRmMenu[game.iRoom] = 1;
		getXtal(arg);
		break;
	case IDI_MSA_ACTION_LOOK_DESK:
		game.iRmMenu[game.iRoom] = 1;
		game.iRmObj[game.iRoom] = 2;
		drawRoom();
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_WRITE_LETTER:
		game.iRmMenu[game.iRoom] = 3;
		game.iRmMenu[IDI_MSA_PIC_EARTH_MAILBOX] = 1;
		game.iRmObj[game.iRoom] = IDI_MSA_OBJECT_NONE;
		getItem(IDI_MSA_ITEM_LETTER);
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_MAIL_LETTER:
		game.fItemUsed[IDI_MSA_ITEM_LETTER] = true;
		game.fItem[IDI_MSA_ITEM_LETTER] = false;
		game.iRmMenu[game.iRoom] = 0;
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_OPEN_MAILBOX:
		if (game.fItemUsed[IDI_MSA_ITEM_LETTER]) {
			printDatMessage(110);
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_CUPBOARD:
		if (game.iRmMenu[game.iRoom]) {
			if (game.iRmObj[game.iRoom] == IDI_MSA_OBJECT_NONE) {
				printDatMessage(78);
			} else {
				printDatMessage(arg);
			}
		} else {
			game.iRmMenu[game.iRoom] = 1;
			game.iRmPic[game.iRoom] = IDI_MSA_PIC_EARTH_KITCHEN_1;
			game.iRmObj[game.iRoom] = 3;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_FLASHLIGHT:
		if (game.fItem[IDI_MSA_ITEM_FLASHLIGHT]) {
			printDatMessage(90);
		} else {
			game.iRmObj[game.iRoom] = IDI_MSA_OBJECT_NONE;
			getItem(IDI_MSA_ITEM_FLASHLIGHT);
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_CABINET:
		if (game.iRmMenu[game.iRoom]) {
			printDatMessage(109);
		} else {
			game.iRmMenu[game.iRoom] = 1;
			game.iRmPic[game.iRoom] = IDI_MSA_PIC_EARTH_GARAGE_1;
			game.iRmObj[game.iRoom] = 15;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_CROWBAR:
		if (game.fItem[IDI_MSA_ITEM_CROWBAR]) {
			printDatMessage(90);
		} else {
			game.iRmObj[game.iRoom]--;
			getItem(IDI_MSA_ITEM_CROWBAR);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_WRENCH:
		if (game.fItem[IDI_MSA_ITEM_WRENCH]) {
			printDatMessage(90);
		} else {
			game.iRmObj[game.iRoom] -= 2;
			getItem(IDI_MSA_ITEM_WRENCH);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_CLOSET:
		if (game.iRmMenu[game.iRoom]) {
			printDatMessage(99);
		} else {
			game.iRmMenu[game.iRoom] = 1;
			game.iRmPic[game.iRoom] = IDI_MSA_PIC_EARTH_BEDROOM_1;
			game.iRmObj[game.iRoom] = 7;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_MATTRESS:
		if (game.fItem[IDI_MSA_ITEM_MATTRESS]) {
			printDatMessage(90);
		} else {
			game.iRmObj[game.iRoom]--;
			getItem(IDI_MSA_ITEM_MATTRESS);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_SCARF:
		if (game.fItem[IDI_MSA_ITEM_SCARF]) {
			printDatMessage(90);
		} else {
			game.iRmObj[game.iRoom] -= 2;
			getItem(IDI_MSA_ITEM_SCARF);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_SUNGLASSES:
		if (game.fItem[IDI_MSA_ITEM_SUNGLASSES]) {
			printDatMessage(90);
		} else {
			game.iRmObj[game.iRoom]--;
			getItem(IDI_MSA_ITEM_SUNGLASSES);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_SCALE:
		if (game.fItem[IDI_MSA_ITEM_SCALE]) {
			printDatMessage(90);
		} else {
			game.iRmMenu[IDI_MSA_PIC_VENUS_WEIGH] = 1;
			game.iRmMenu[IDI_MSA_PIC_NEPTUNE_WEIGH] = 1;
			game.iRmMenu[IDI_MSA_PIC_MERCURY_WEIGH] = 1;
			game.iRmMenu[IDI_MSA_PIC_SATURN_WEIGH] = 1;
			game.iRmMenu[IDI_MSA_PIC_PLUTO_WEIGH] = 1;
			game.iRmMenu[IDI_MSA_PIC_JUPITER_WEIGH] = 1;
			game.iRmMenu[IDI_MSA_PIC_MARS_WEIGH] = 1;
			game.iRmMenu[IDI_MSA_PIC_URANUS_WEIGH] = 1;
			game.iRmObj[game.iRoom] -= 2;
			getItem(IDI_MSA_ITEM_SCALE);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GOTO_SPACESHIP:
		game.iRoom = IDI_MSA_PIC_SHIP_AIRLOCK;
		if (game.iPlanet != IDI_MSA_PLANET_EARTH)
			insertDisk(0);
		return true;

	// VENUS

	case IDI_MSA_ACTION_DOWN_CHASM:
		if (game.fItem[IDI_MSA_ITEM_ROPE]) {
			game.iRmMenu[game.iRoom] = 1;
		}
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_DOWN_ROPE:
		if (game.fItemUsed[IDI_MSA_ITEM_ROPE]) {
			game.iRoom = IDI_MSA_PIC_VENUS_PROBE;
			return true;
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_ROPE:
		if (game.fItemUsed[IDI_MSA_ITEM_ROPE]) {
			printDatMessage(22);
		} else {
			game.fItemUsed[IDI_MSA_ITEM_ROPE] = true;
			game.fItem[IDI_MSA_ITEM_ROPE] = false;
			game.iRmPic[game.iRoom] = IDI_MSA_PIC_VENUS_CHASM_1;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_HATCH:
		if (game.fItemUsed[IDI_MSA_ITEM_WRENCH]) {
			if ((game.iRmMenu[game.iRoom] == 3) || (game.iRmPic[game.iRoom] == IDI_MSA_PIC_VENUS_PROBE_1))
				printDatMessage(39);
			else {
				game.iRmMenu[game.iRoom] = 2;
				game.iRmPic[game.iRoom] = IDI_MSA_PIC_VENUS_PROBE_1;
				drawRoom();
				printDatMessage(24);
			}
		} else {
			if (game.fItem[IDI_MSA_ITEM_WRENCH]) {
				game.iRmMenu[game.iRoom] = 1;
			}
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_WRENCH:
		game.fItemUsed[IDI_MSA_ITEM_WRENCH] = true;
		printDatString(arg);
		if (game.iRmPic[game.iRoom] == IDI_MSA_PIC_VENUS_PROBE_1) {
			//ClearRow(22);	// TODO
		}
		_vm->_gfx->doUpdate();
		_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
		_vm->waitAnyKey();
		break;
	case IDI_MSA_ACTION_GET_XTAL_VENUS:
		game.iRmMenu[game.iRoom] = 3;
		getXtal(arg);
		break;

	// TRITON (NEPTUNE)

	case IDI_MSA_ACTION_LOOK_CASTLE:
		if (!game.iRmMenu[game.iRoom]) {
			game.iRmMenu[game.iRoom] = 1;
		}
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_ENTER_OPENING:
		if (game.fItemUsed[IDI_MSA_ITEM_CROWBAR]) {
			game.iRoom = IDI_MSA_PIC_NEPTUNE_CASTLE_4;
			return true;
		} else {
			if (game.fItem[IDI_MSA_ITEM_CROWBAR]) {
				game.iRmMenu[game.iRoom] = 2;
			}
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_CROWBAR:
		game.fItemUsed[IDI_MSA_ITEM_CROWBAR] = true;
		game.iRmMenu[game.iRoom] = 1;
		game.iRmPic[game.iRoom] = IDI_MSA_PIC_NEPTUNE_ENTRANCE_1;
		drawRoom();
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_GET_XTAL_NEPTUNE:
		if (game.fHasXtal) {
			printDatMessage(71);
		} else {
			if (game.fItem[IDI_MSA_ITEM_SCARF]) {
				game.iRmMenu[game.iRoom] = 1;
			}
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_TALK_LEADER:
		game.iRoom = IDI_MSA_PIC_NEPTUNE_ENTRYWAY;
		printDatMessage(arg);
		return true;
	case IDI_MSA_ACTION_GIVE_SCARF:
		game.iRmObj[game.iRoom] = 18;
		getXtal(arg);
		game.fItem[IDI_MSA_ITEM_SCARF] = false;
		game.iRmMenu[game.iRoom] = 0;
		game.iRmMenu[IDI_MSA_PIC_EARTH_BEDROOM] = 2;
		game.iRoom = IDI_MSA_PIC_NEPTUNE_ENTRYWAY;
		return true;

	// MERCURY

	case IDI_MSA_ACTION_GET_XTAL_MERCURY:
		if (game.fHasXtal) {
			game.iRmMenu[game.iRoom] = 2;
			printDatMessage(32);
		} else {
			if (game.fItem[IDI_MSA_ITEM_SUNGLASSES]) {
				game.iRmMenu[game.iRoom] = 1;
			}
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GIVE_SUNGLASSES:
		game.iRmObj[game.iRoom] = 17;
		game.iRmMenu[game.iRoom] = 2;
		game.fItem[IDI_MSA_ITEM_SUNGLASSES] = false;
		getXtal(arg);
		break;

	// TITAN (SATURN)

	case IDI_MSA_ACTION_CROSS_LAKE:
		if (game.fItem[IDI_MSA_ITEM_MATTRESS]) {
			game.iRmMenu[IDI_MSA_PIC_SATURN_LAKE_0] = 1;
			game.iRmMenu[IDI_MSA_PIC_SATURN_LAKE_1] = 1;
			game.iRmMenu[IDI_MSA_PIC_SATURN_LAKE_2] = 1;
		}
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_USE_MATTRESS:
		game.iRoom = IDI_MSA_PIC_SATURN_ISLAND;
		printDatMessage(arg);
		return true;
	case IDI_MSA_ACTION_GET_XTAL_SATURN:
		if (game.fHasXtal) {
			printDatMessage(29);
		} else {
			getXtal(arg);
		}
		break;
	case IDI_MSA_ACTION_LEAVE_ISLAND:
		game.iRoom = IDI_MSA_PIC_SATURN_LAKE_1;
		printDatMessage(arg);
		return true;

	// PLUTO

	case IDI_MSA_ACTION_GET_XTAL_PLUTO:
		if (game.fHasXtal) {
			printDatMessage(19);
		} else {
			if (game.fItem[IDI_MSA_ITEM_BONE]) {
				game.iRmMenu[game.iRoom] = 1;
			}
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GIVE_BONE:
		game.fItem[IDI_MSA_ITEM_BONE] = false;
		game.iRmMenu[game.iRoom] = 0;
		game.iRmObj[game.iRoom] = 16;
		getXtal(arg);
		break;

	// IO (JUPITER)

	case IDI_MSA_ACTION_GET_ROCK_0:
		if (game.fItem[IDI_MSA_ITEM_ROCK]) {
			printDatMessage(38);
		} else {
			game.iRmMenu[game.iRoom] = 1;
			game.iRmObj[game.iRoom] = IDI_MSA_OBJECT_NONE;
			getItem(IDI_MSA_ITEM_ROCK);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_ROCK_1:
		if (game.fItem[IDI_MSA_ITEM_ROCK]) {
			printDatMessage(38);
		} else {
			game.iRmMenu[game.iRoom] = 1;
			game.iRmObj[game.iRoom] = IDI_MSA_OBJECT_NONE;
			getItem(IDI_MSA_ITEM_ROCK);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_XTAL_JUPITER:
		if (game.fHasXtal) {
			printDatMessage(15);
		} else {
			switch (game.nRocks) {
			case 0:
				if (game.fItem[IDI_MSA_ITEM_ROCK]) {
					game.iRmMenu[game.iRoom] = 1;
				}
				printDatMessage(arg);
				break;
			case 1:
				if (game.fItem[IDI_MSA_ITEM_ROCK]) {
					game.iRmMenu[game.iRoom] = 1;
				}
				printDatMessage(34);
				break;
			case 2:
				getXtal(35);
				break;
			}
		}
		break;
	case IDI_MSA_ACTION_THROW_ROCK:
		game.fItem[IDI_MSA_ITEM_ROCK] = false;
		game.nItems--;
		game.iRmObj[game.iRoom]++;
		game.iRmMenu[game.iRoom] = 0;
		drawRoom();
		if (game.nRocks) {
			printDatMessage(37);
		} else {
			printDatMessage(arg);
		}
		game.nRocks++;
		break;

	// MARS

	case IDI_MSA_ACTION_GO_TUBE:
		if (game.fItem[IDI_MSA_ITEM_FLASHLIGHT]) {
			game.iRmMenu[game.iRoom] = 1;
		}
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_USE_FLASHLIGHT:
		game.iRoom = IDI_MSA_PIC_MARS_TUBE_1;
		printDatMessage(15);
		return true;
	case IDI_MSA_ACTION_PLUTO_DIG:
		if (game.fHasXtal) {
			printDatMessage(21);
		} else {
			getXtal(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_XTAL_MARS:
		if (game.fHasXtal) {
			printDatMessage(23);
		} else {
			printDatMessage(arg);
		}
		break;

	// OBERON (URANUS)

	case IDI_MSA_ACTION_ENTER_TEMPLE:
		game.iRoom = IDI_MSA_PIC_URANUS_TEMPLE;
		return true;
	case IDI_MSA_ACTION_USE_CRYSTAL:
		if (game.iRmMenu[game.iRoom]) {
			printDatMessage(25);
		} else {
			game.iRmMenu[game.iRoom] = 1;
			game.iRmPic[game.iRoom] = IDI_MSA_PIC_URANUS_TEMPLE_1;
			drawRoom();
			game.iRmPic[game.iRoom] = IDI_MSA_PIC_URANUS_TEMPLE;
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_DOOR:
		if (game.fTempleDoorOpen) {
			printDatMessage(36);
		} else {
			game.fTempleDoorOpen = 1;
			game.iRmPic[game.iRoom] = IDI_MSA_PIC_URANUS_TEMPLE_2;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_ENTER_DOOR:
		if (game.fTempleDoorOpen) {
			game.iRoom = IDI_MSA_PIC_URANUS_STEPS;
			return true;
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_XTAL_URANUS:
		if (game.fHasXtal) {
			printDatMessage(34);
		} else {
			if (game.fItem[IDI_MSA_ITEM_CROWBAR]) {
				game.iRmMenu[game.iRoom] = 1;
			}
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_CROWBAR_1:
		game.iRmMenu[game.iRoom] = 0;
		getXtal(arg);
		break;

	// SPACESHIP

	case IDI_MSA_ACTION_GO_NORTH:
		if (game.fShipDoorOpen) {
			if (game.fSuit) {
				printDatMessage(45);
			} else {
				game.iRoom = IDI_MSA_PIC_SHIP_CORRIDOR;
				return true;
			}
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GO_PLANET:
		if (!game.fShipDoorOpen) {
			if ((game.nXtals == IDI_MSA_MAX_PLANET) && (game.iPlanet == IDI_MSA_PLANET_EARTH))
				gameOver();
			if ((game.iPlanet == game.iPlanetXtal[game.nXtals]) || (game.iPlanet == IDI_MSA_PLANET_EARTH)) {
				game.fHasXtal = false;
				game.iRoom = IDI_MSA_HOME_PLANET[game.iPlanet];
				if (game.iPlanet != IDI_MSA_PLANET_EARTH)
					insertDisk(1);
				return true;
			} else {
				game.iRoom = IDI_MSA_SHIP_PLANET[game.iPlanet];
				return true;
			}
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_PRESS_BUTTON:
		if (game.fShipDoorOpen) {		// inner door open
			if (game.iPlanet && !game.fSuit) {
				printDatMessage(arg);
			} else {
				game.fShipDoorOpen = false;
				game.iRmPic[game.iRoom]--;
				drawRoom();
				printDatMessage(2);
			}
		} else {
			game.fShipDoorOpen = true;
			game.iRmPic[game.iRoom]++;
			drawRoom();
			printDatMessage(14);
		}
		break;
	case IDI_MSA_ACTION_WEAR_SPACESUIT:
		if (game.fSuit) {
			if (game.fShipDoorOpen) {
				game.fSuit = false;
				game.iRmMenu[game.iRoom] = 0;
				game.iRmPic[game.iRoom] -= 2;
				drawRoom();
				printDatMessage(13);
			} else {
				printDatMessage(3);
			}
		} else {
			if (game.iPlanet) {
				game.fSuit = true;
				game.iRmMenu[game.iRoom] = 1;
				game.iRmPic[game.iRoom] += 2;
				drawRoom();
				printDatMessage(arg);
			} else {
				printDatMessage(12);
			}
		}
		break;
	case IDI_MSA_ACTION_READ_GAUGE:
		printDatString(arg);
		_vm->drawStr(IDI_MSA_ROW_TEMPERATURE, IDI_MSA_COL_TEMPERATURE_C, IDA_DEFAULT,
			(const char *)IDS_MSA_TEMP_C[game.iPlanet]);
		_vm->drawStr(IDI_MSA_ROW_TEMPERATURE, IDI_MSA_COL_TEMPERATURE_F, IDA_DEFAULT,
			(const char *)IDS_MSA_TEMP_F[game.iPlanet]);
		_vm->_gfx->doUpdate();
		_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
		_vm->waitAnyKey();
		break;
	case IDI_MSA_ACTION_PRESS_ORANGE:
		if (game.fFlying) {
			printDatMessage(4);
		} else {
			//playSound(IDI_MSA_SND_PRESS_ORANGE);	// TODO
			printDatMessage(arg);
			pressOB(IDI_MSA_BUTTON_ORANGE);
		}
		break;
	case IDI_MSA_ACTION_PRESS_BLUE:
		if (game.fFlying) {
			printDatMessage(4);
		} else {
			//playSound(IDI_MSA_SND_PRESS_BLUE);	// TODO
			printDatMessage(arg);
			pressOB(IDI_MSA_BUTTON_BLUE);
		}
		break;
	case IDI_MSA_ACTION_FLIP_SWITCH:
		flipSwitch();
		break;
	case IDI_MSA_ACTION_PUSH_THROTTLE:
		if (game.fFlying) {
			game.fFlying = false;
			game.nButtons = 0;
			memset(game.szAddr, 0, sizeof(game.szAddr));
			drawRoom();
			printDatString(22);
			_vm->drawStr(IDI_MSA_ROW_PLANET, IDI_MSA_COL_PLANET, IDA_DEFAULT,
						(const char *)IDS_MSA_PLANETS[game.iPlanet]);
			_vm->_gfx->doUpdate();
			_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
			_vm->waitAnyKeyAnim();
			showPlanetInfo();
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_PULL_THROTTLE:
		if (game.fFlying) {
			printDatMessage(18);
		} else {
			if (getPlanet() != -1) {
				game.fFlying = true;
				game.iPlanet = getPlanet();
				drawRoom();
				printDatMessage(16);
			} else {
				game.nButtons = 0;
				memset(game.szAddr, 0, sizeof(game.szAddr));
				printDatMessage(17);
			}
		}
		break;
	case IDI_MSA_ACTION_LEAVE_ROOM:
		if (game.fFlying) {
			printDatMessage(24);
		} else {
			game.iRoom = arg;
			return true;
		}
		break;
	case IDI_MSA_ACTION_OPEN_CABINET_1:
		if (game.iRmMenu[game.iRoom]) {
			printExeMsg(IDO_MSA_THE_CABINET_IS_ALREADY_OPEN);
		} else {
			game.iRmMenu[game.iRoom] = 1;
			game.iRmPic[game.iRoom] = IDI_MSA_PIC_SHIP_KITCHEN_1;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_READ_MAP:
		game.iRmPic[game.iRoom] = IDI_MSA_PIC_STAR_MAP;
		drawRoom();
		printDatMessage(46);
		printDatMessage(47);
		printDatMessage(48);
		game.iRmPic[game.iRoom] = IDI_MSA_PIC_SHIP_BEDROOM;
		drawRoom();
		break;
	case IDI_MSA_ACTION_GO_WEST:
		game.nButtons = 0;
		memset(game.szAddr, 0, sizeof(game.szAddr));
		game.iRoom = arg;
		return true;
	}

	return false;
}

void Mickey::gameLoop() {
	char *buffer = new char[sizeof(MSA_MENU)];
	MSA_MENU menu;
	int iSel0, iSel1;
	bool done;

	for (;;) {
		drawRoom();

		if (game.fIntro) {
			game.fIntro = false;
		} else {
			printRoomDesc();
		}
		
		if (game.iRoom == IDI_MSA_PIC_NEPTUNE_GUARD) {
			game.iRoom = IDI_MSA_PIC_NEPTUNE_LEADER;
			done = true;
		} else {
			done = false;
		}

		while (!done) {
			checkAirSupply(game.fSuit, &game.nAir);
			readMenu(game.iRoom, buffer);
			memcpy(&menu, buffer, sizeof(MSA_MENU));
			patchMenu(&menu);
			memcpy(buffer, &menu, sizeof(MSA_MENU));
			getMenuSel(buffer, &iSel0, &iSel1);
			done = parse(menu.cmd[iSel0].data[iSel1], menu.arg[iSel0].data[iSel1]);
		}

		game.nFrame = 0;
	}

	delete [] buffer;
}

// Debug

void Mickey::debug_DrawObjs() {
	char szTitle[14] = {0};

	for (int iObj = 0; iObj < IDI_MSA_MAX_OBJ; iObj++) {
		drawPic(0);
		drawObj((ENUM_MSA_OBJECT)iObj, 0, 0);

		_vm->clearTextArea();
		sprintf(szTitle, "Object %d", iObj);
		_vm->drawStrMiddle(22, IDA_DEFAULT, szTitle);
		_vm->drawStrMiddle(23, IDA_DEFAULT, (const char *)IDS_MSA_NAME_OBJ[iObj]);
		_vm->_gfx->doUpdate();
		_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
		_vm->waitAnyKey();
	}
}

void Mickey::debug_DrawPics(){
	char szTitle[14] = {0};

	for (int iPic = 1; iPic <= IDI_MSA_MAX_PIC; iPic++) {
		drawPic(iPic);

		_vm->clearTextArea();
		sprintf(szTitle, "Picture %d", iPic);
		_vm->drawStrMiddle(22, IDA_DEFAULT, szTitle);
		_vm->_gfx->doUpdate();
		_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
		_vm->waitAnyKey();
	}
}

// Init

void Mickey::initVars() {
	uint8 buffer[512];

	// clear game struct
	memset(&game, 0, sizeof(game));
	memset(&game.iItem, IDI_MSA_OBJECT_NONE, sizeof(game.iItem));
	// read room extended desc flags
	readExe(IDO_MSA_ROOM_TEXT, buffer, sizeof(buffer));
	memcpy(game.fRmTxt, buffer, sizeof(game.fRmTxt));

	// read room extended desc offsets
	readExe(IDO_MSA_ROOM_TEXT_OFFSETS, buffer, sizeof(buffer));
//	memcpy(game.oRmTxt, buffer, sizeof(game.oRmTxt));
	for (int i = 0; i < IDI_MSA_MAX_ROOM; i++)
		game.oRmTxt[i] = buffer[i*2] + 256 * buffer[i*2+1];

	// read room object indices
	readExe(IDO_MSA_ROOM_OBJECT, buffer, sizeof(buffer));
	memcpy(game.iRmObj, buffer, sizeof(game.iRmObj));
	
	// read room picture indices
	//Mickey_readExe(IDO_MSA_ROOM_PICTURE, buffer, sizeof(buffer));
	//memcpy(game.iRmPic, buffer, sizeof(game.iRmPic));

	// read room menu patch indices
	readExe(IDO_MSA_ROOM_MENU_FIX, buffer, sizeof(buffer));
	memcpy(game.nRmMenu, buffer, sizeof(game.nRmMenu));

	// set room picture indices
	for (int i = 0; i < IDI_MSA_MAX_ROOM; i++) {
		game.iRmPic[i] = i;
	}
	game.iRmPic[IDI_MSA_PIC_SHIP_AIRLOCK] = IDI_MSA_PIC_SHIP_AIRLOCK_0;

#ifdef _DEBUG

	game.iPlanet = IDI_MSA_PLANET_EARTH;
	game.iRoom = IDI_MSA_PIC_SHIP_CONTROLS;
	game.fHasXtal = true;
	game.nXtals = 9;
	game.fItemUsed[IDI_MSA_ITEM_LETTER] = true;

#endif

}

void Mickey::initEngine() {
	// PreAGI sets the screen params here, but we've already done that in the preagi class
}

Mickey::Mickey(PreAgiEngine *vm) : _vm(vm) {
}

Mickey::~Mickey() {
}

void Mickey::init() {
	initEngine();
	initVars();
}

void Mickey::run() {
	intro();
	gameLoop();
	gameOver();
}

}
