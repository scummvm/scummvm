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

#include "common/events.h"
#include "common/savefile.h"
#include "common/stream.h"

#include "graphics/cursorman.h"

#include "agi/preagi.h"
#include "agi/preagi_common.h"
#include "agi/preagi_mickey.h"
#include "agi/graphics.h"

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
	if (!infile.open("mickey.exe"))
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

void Mickey::readOfsData(int offset, int iItem, uint8 *buffer, long buflen) {
	uint16 ofs[256];

	readExe(offset, buffer, buflen);
	memcpy(ofs, buffer, sizeof(ofs));

	for (int i = 0; i < 256; i++)
		ofs[i] = buffer[i*2] + 256 * buffer[i*2+1];

	readExe(ofs[iItem] + IDI_MSA_OFS_EXE, buffer, buflen);
}

// User Interface

bool Mickey::chooseY_N(int ofsPrompt, bool fErrorMsg) {
	printExeStr(ofsPrompt);

	while (!_vm->shouldQuit()) {
		switch (_vm->getSelection(kSelYesNo)) {
		case 0: return false;
		case 1: return true;
		default:
			if (fErrorMsg) {
				printExeStr(IDO_MSA_PRESS_YES_OR_NO);
				waitAnyKey();
				printExeStr(ofsPrompt);
			}
			break;
		}
	}

	return false;
}

int Mickey::choose1to9(int ofsPrompt) {
	int answer = 0;
	printExeStr(ofsPrompt);

	while (!_vm->shouldQuit()) {
		answer = _vm->getSelection(kSelNumber);
		if (answer == 10) {
			printExeStr(IDO_MSA_PRESS_1_TO_9);
			if (_vm->getSelection(kSelAnyKey) == 0)
				return 0;
			printExeStr(ofsPrompt);
		} else return answer;
	}

	return 0;
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

	// Show the string on screen
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();
}

void Mickey::printLine(const char *buffer) {
	_vm->clearTextArea();

	_vm->drawStr(22, 18 - strlen(buffer) / 2, IDA_DEFAULT, buffer);

	// Show the string on screen
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();

	waitAnyKey(true);
}

void Mickey::printExeStr(int ofs) {
	uint8 buffer[256] = {0};

	if (!ofs)
		return;

	readExe(ofs, buffer, sizeof(buffer));
	printStr((char *)buffer);
}

void Mickey::printExeMsg(int ofs) {
	if (!ofs)
		return;

	printExeStr(ofs);
	waitAnyKey(true);
}

void Mickey::printDatString(int iStr) {
	char buffer[256];
	int iDat = getDat(_game.iRoom);

	MSA_DAT_HEADER hdr;
	char szFile[256] = {0};

	sprintf(szFile, IDS_MSA_PATH_DAT, IDS_MSA_NAME_DAT[iDat]);
	readDatHdr(szFile, &hdr);

	Common::File infile;

	if (!infile.open(szFile))
		return;

	infile.seek(hdr.ofsStr[iStr] + IDI_MSA_OFS_DAT, SEEK_SET);
	infile.read((uint8 *)buffer, 256);
	infile.close();

	printStr(buffer);
}

void Mickey::printDesc(int iRoom) {
	char *buffer = (char *)malloc(256);

	MSA_DAT_HEADER hdr;
	char szFile[256] = {0};

	getDatFileName(iRoom, szFile);
	readDatHdr(szFile, &hdr);

	Common::File infile;

	if (!infile.open(szFile))
		return;

	memset(buffer, 0, 256);

	infile.seek(hdr.ofsDesc[iRoom - 1] + IDI_MSA_OFS_DAT, SEEK_SET);
	infile.read(buffer, 256);
	infile.close();

	printStr(buffer);
	free(buffer);
}

bool Mickey::checkMenu() {
	char *buffer = new char[sizeof(MSA_MENU)];
	MSA_MENU menu;
	int iSel0, iSel1;
	MSA_DAT_HEADER hdr;
	char szFile[256] = {0};
	Common::File infile;

	getDatFileName(_game.iRoom, szFile);
	readDatHdr(szFile, &hdr);
	if (!infile.open(szFile))
		return false;
	infile.seek(hdr.ofsRoom[_game.iRoom - 1] + IDI_MSA_OFS_DAT, SEEK_SET);
	infile.read((uint8 *)buffer, sizeof(MSA_MENU));
	infile.close();

	memcpy(&menu, buffer, sizeof(MSA_MENU));
	patchMenu(&menu);
	memcpy(buffer, &menu, sizeof(MSA_MENU));

	getMenuSel(buffer, &iSel0, &iSel1);
	delete[] buffer;

	return parse(menu.cmd[iSel0].data[iSel1], menu.arg[iSel0].data[iSel1]);
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

	// Menu created, show it on screen
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();
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
	int goIndex = -1, northIndex = -1, southIndex = -1, eastIndex = -1, westIndex = -1;

	switch(iRow) {
	case 0:
		sel = sel0;
		break;
	case 1:
		sel = sel1;
		break;
	}
	nWords = menu.row[iRow].count;
	_clickToMove = false;

	for (int i = 0; i <= menu.row[0].count; i++)
		if (menu.row[0].entry[i].szText[0] == 71 && menu.row[0].entry[i].szText[1] == 79)	// GO
			goIndex = i;

	if (goIndex >= 0) {
		for (int j = 0; j <= menu.row[1].count; j++) {
			if (menu.row[1].entry[j].szText[0] == 78 && menu.row[1].entry[j].szText[1] == 79 &&
				menu.row[1].entry[j].szText[2] == 82 && menu.row[1].entry[j].szText[3] == 84 &&
				menu.row[1].entry[j].szText[4] == 72)
				northIndex = j;
			if (menu.row[1].entry[j].szText[0] == 83 && menu.row[1].entry[j].szText[1] == 79 &&
				menu.row[1].entry[j].szText[2] == 85 && menu.row[1].entry[j].szText[3] == 84 &&
				menu.row[1].entry[j].szText[4] == 72)
				southIndex = j;
			if (menu.row[1].entry[j].szText[0] == 69 && menu.row[1].entry[j].szText[1] == 65 &&
				menu.row[1].entry[j].szText[2] == 83 && menu.row[1].entry[j].szText[3] == 84)
				eastIndex = j;
			if (menu.row[1].entry[j].szText[0] == 87 && menu.row[1].entry[j].szText[1] == 69 &&
				menu.row[1].entry[j].szText[2] == 83 && menu.row[1].entry[j].szText[3] == 84)
				westIndex = j;
		}
	}

	drawMenu(menu, *sel0, *sel1);

	while (!_vm->shouldQuit()) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				return 0;
			case Common::EVENT_MOUSEMOVE:
				if (iRow < 2) {
					x = event.mouse.x / 8;
					y = event.mouse.y / 8;
					// If the mouse hovers over the menu, refresh the menu
					if ((iRow == 0 && y == IDI_MSA_ROW_MENU_0) || (iRow == 1 && y == IDI_MSA_ROW_MENU_1)) {
						getMouseMenuSelRow(menu, sel0, sel1, iRow, x, y);
						drawMenu(menu, *sel0, *sel1);
					}

					// Change cursor
					if (northIndex >= 0 && (event.mouse.x >= 20 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2) &&
							(event.mouse.y >= 0 && event.mouse.y <= 10)) {
						_vm->_gfx->setCursorPalette(true);
					} else if (southIndex >= 0 && (event.mouse.x >= 20 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2) &&
								(event.mouse.y >= IDI_MSA_PIC_HEIGHT - 10 && event.mouse.y <= IDI_MSA_PIC_HEIGHT)) {
						_vm->_gfx->setCursorPalette(true);
					} else if (westIndex >= 0 && (event.mouse.y >= 0  && event.mouse.y <= IDI_MSA_PIC_HEIGHT) &&
								(event.mouse.x >= 20 && event.mouse.x <= 30)) {
						_vm->_gfx->setCursorPalette(true);
					} else if (eastIndex >= 0 && (event.mouse.y >= 0  && event.mouse.y <= IDI_MSA_PIC_HEIGHT) &&
								(event.mouse.x >= IDI_MSA_PIC_WIDTH * 2 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2)) {
						_vm->_gfx->setCursorPalette(true);
					} else {
						_vm->_gfx->setCursorPalette(false);
					}
				}
				break;
			case Common::EVENT_LBUTTONUP:
				// Click to move
				if (northIndex >= 0 && (event.mouse.x >= 20 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2) &&
						(event.mouse.y >= 0 && event.mouse.y <= 10)) {
					*sel0 = goIndex;
					*sel1 = northIndex;

					drawMenu(menu, *sel0, *sel1);

					_vm->_gfx->setCursorPalette(false);
					_clickToMove = true;
				} else if (southIndex >= 0 && (event.mouse.x >= 20 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2) &&
							(event.mouse.y >= IDI_MSA_PIC_HEIGHT - 10 && event.mouse.y <= IDI_MSA_PIC_HEIGHT)) {
					*sel0 = goIndex;
					*sel1 = southIndex;

					drawMenu(menu, *sel0, *sel1);

					_vm->_gfx->setCursorPalette(false);
					_clickToMove = true;
				} else if (westIndex >= 0 && (event.mouse.y >= 0  && event.mouse.y <= IDI_MSA_PIC_HEIGHT) &&
							(event.mouse.x >= 20 && event.mouse.x <= 30)) {
					*sel0 = goIndex;
					*sel1 = westIndex;

					drawMenu(menu, *sel0, *sel1);

					_vm->_gfx->setCursorPalette(false);
					_clickToMove = true;
				} else if (eastIndex >= 0 && (event.mouse.y >= 0  && event.mouse.y <= IDI_MSA_PIC_HEIGHT) &&
							(event.mouse.x >= IDI_MSA_PIC_WIDTH * 2 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2)) {
					*sel0 = goIndex;
					*sel1 = eastIndex;

					drawMenu(menu, *sel0, *sel1);

					_vm->_gfx->setCursorPalette(false);
					_clickToMove = true;
				} else {
					_vm->_gfx->setCursorPalette(false);
				}
				return true;
			case Common::EVENT_RBUTTONUP:
				*sel0 = 0;
				*sel1 = -1;
				return false;
			case Common::EVENT_WHEELUP:
				if (iRow < 2) {
					*sel -= 1;

					if (*sel < 0)
						*sel = nWords - 1;

					drawMenu(menu, *sel0, *sel1);
				}
				break;
			case Common::EVENT_WHEELDOWN:
				if (iRow < 2) {
					*sel += 1;

					if (*sel == nWords)
						*sel = 0;

					drawMenu(menu, *sel0, *sel1);
				}
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_d && (event.kbd.flags & Common::KBD_CTRL) && _vm->_console) {
					_vm->_console->attach();
					_vm->_console->onFrame();
					continue;
				}

				switch (event.kbd.keycode) {
				case Common::KEYCODE_2:
					// Hidden message
					if (_game.iRoom == IDI_MSA_PIC_MERCURY_CAVE_0) {
						for (int i = 0; i < 5; i++) {
							printExeMsg(IDO_MSA_HIDDEN_MSG[i]);
						}
						_vm->clearTextArea();
						waitAnyKey();
					}
					break;
				case Common::KEYCODE_8:
					if (event.kbd.flags & Common::KBD_CTRL) {
						*sel0 = 0;
						*sel1 = -1;

						return false;
					}
					break;
				case Common::KEYCODE_ESCAPE:
					*sel0 = 0;
					*sel1 = -1;

					return false;
				case Common::KEYCODE_s:
					_vm->flipflag(fSoundOn);
					break;
				case Common::KEYCODE_c:
					inventory();
					drawRoom();

					*sel0 = 0;
					*sel1 = -1;

					return false;
				case Common::KEYCODE_b:
					printRoomDesc();
					drawMenu(menu, *sel0, *sel1);

					*sel0 = 0;
					*sel1 = -1;

					return false;
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_KP4:
				case Common::KEYCODE_4:
					if (iRow < 2) {
						*sel -= 1;

						if (*sel < 0)
							*sel = nWords - 1;

						drawMenu(menu, *sel0, *sel1);
					}
					break;
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_SPACE:
				case Common::KEYCODE_KP6:
				case Common::KEYCODE_6:
					if (iRow < 2) {
						*sel += 1;

						if (*sel == nWords)
							*sel = 0;

						drawMenu(menu, *sel0, *sel1);
					}
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
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

	return false;
}

void Mickey::getMenuSel(char *buffer, int *sel0, int *sel1) {
	MSA_MENU menu;

	memcpy(&menu, buffer, sizeof(MSA_MENU));

	*sel0 = 0;
	*sel1 = -1;

	// Show the mouse cursor for the menu
	CursorMan.showMouse(true);

	while (!_vm->shouldQuit()) {
		while (!_vm->shouldQuit()) {
			if (getMenuSelRow(menu, sel0, sel1, 0)) {
				if (_clickToMove)
					break;

				*sel1 = 0;

				if (getMenuSelRow(menu, sel0, sel1, 1)) {
					break;
				}
			}
		}

		if (_clickToMove || getMenuSelRow(menu, sel0, sel1, 2)) {
			break;
		}
	}

	// Menu selection made, hide the mouse cursor
	CursorMan.showMouse(false);
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
	if (_game.iRoom == IDI_MSA_PIC_SHIP_AIRLOCK) {
		strcpy((char *)menu->row[1].entry[2].szText, IDS_MSA_NAME_PLANET[_game.iPlanet]);
	}

	// exit if fix unnecessary
	if (!_game.iRmMenu[_game.iRoom]) {
		centerMenu(menu);
		return;
	}

	// copy menu to menubuf
	memcpy(menubuf, menu, sizeof(menubuf));

	// read patches
	readOfsData(
		IDOFS_MSA_MENU_PATCHES,
		_game.nRmMenu[_game.iRoom] + _game.iRmMenu[_game.iRoom] - 1,
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

void Mickey::printDatMessage(int iStr) {
	printDatString(iStr);
	waitAnyKey(true);
}

// Sound

void Mickey::playNote(MSA_SND_NOTE note) {
	if (!note.counter) {
		// Pause
		_vm->_system->delayMillis((uint) (note.length / IDI_SND_TIMER_RESOLUTION));
	} else {
		_vm->playNote(IDI_SND_OSCILLATOR_FREQUENCY / note.counter, (int32) (note.length / IDI_SND_TIMER_RESOLUTION));
	}
}

void Mickey::playSound(ENUM_MSA_SOUND iSound) {
	if (!_vm->getflag(fSoundOn))
		return;

	Common::Event event;
	MSA_SND_NOTE note;
	uint8 *buffer = new uint8[1024];
	int pBuf = 1;

	switch(iSound) {
	case IDI_MSA_SND_XL30:
		for (int iNote = 0; iNote < 6; iNote++) {
			note.counter = _vm->rnd(59600) + 59;
			note.length = 4;
			playNote(note);
		}
		break;
	default:
		readOfsData(IDOFS_MSA_SOUND_DATA, iSound, buffer, 1024);

		for (;;) {
			memcpy(&note, buffer + pBuf, sizeof(note));
			if (!note.counter && !note.length)
				break;

			playNote(note);

			pBuf += 3;

			if (iSound == IDI_MSA_SND_THEME) {
				while (_vm->_system->getEventManager()->pollEvent(event)) {
					switch(event.type) {
					case Common::EVENT_RTL:
					case Common::EVENT_QUIT:
					case Common::EVENT_LBUTTONUP:
					case Common::EVENT_RBUTTONUP:
					case Common::EVENT_KEYDOWN:
						delete[] buffer;
						return;
					default:
						break;
					}
				}
			}
		}

		break;
	}

	delete[] buffer;
}

// Graphics

void Mickey::drawObj(ENUM_MSA_OBJECT iObj, int x0, int y0) {
	uint8* buffer = new uint8[4096];
	char szFile[255] = {0};
	sprintf(szFile, IDS_MSA_PATH_OBJ, IDS_MSA_NAME_OBJ[iObj]);

	Common::File file;
	if (!file.open(szFile))
		return;

	uint32 size = file.size();
	file.read(buffer, size);
	file.close();

	if (iObj == IDI_MSA_OBJECT_CRYSTAL)
		_vm->_picture->setPictureFlags(kPicFStep);

	_vm->_picture->setOffset(x0, y0);
	_vm->_picture->decodePicture(buffer, size, false, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
	_vm->_picture->setOffset(0, 0);
	_vm->_picture->showPic(10, 0, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
}

void Mickey::drawPic(int iPic) {
	uint8* buffer = new uint8[4096];
	char szFile[255] = {0};
	sprintf(szFile, IDS_MSA_PATH_PIC, iPic);

	Common::File file;
	if (!file.open(szFile))
		return;

	uint32 size = file.size();
	file.read(buffer, size);
	file.close();

	// Note that decodePicture clears the screen
	_vm->_picture->decodePicture(buffer, size, true, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
	_vm->_picture->showPic(10, 0, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
}

void Mickey::drawRoomAnimation() {
	uint8 objLight[] = {
		0xF0, 1, 0xF9, 2, 43, 45, 0xFF
	};

	switch(_game.iRoom) {
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
		{
			// draw blinking ship lights

			uint8 iColor = 0;

			_vm->_picture->setPattern(2, 0);

			for (int i = 0; i < 12; i++) {
				iColor = _game.nFrame + i;
				if (iColor > 15)
					iColor -= 15;

				objLight[1] = iColor;
				objLight[4] += 7;

				_vm->_picture->setPictureData(objLight);
				_vm->_picture->setPictureFlags(kPicFCircle);
				_vm->_picture->drawPicture();
			}
			_vm->_picture->showPic(10, 0, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);


			_game.nFrame--;
			if (_game.nFrame < 0)
				_game.nFrame = 15;

			playSound(IDI_MSA_SND_PRESS_BLUE);
		}
		break;

	case IDI_MSA_PIC_SHIP_CONTROLS:

		// draw XL30 screen
		if (_game.fAnimXL30) {
			if (_game.nFrame > 5)
				_game.nFrame = 0;

			drawObj((ENUM_MSA_OBJECT)(IDI_MSA_OBJECT_XL31 + _game.nFrame), 0, 4);
			_game.nFrame++;
		};

		break;

	default:

		// draw crystal
		if (_game.iRoom == IDI_MSA_XTAL_ROOM_XY[_game.iPlanet][0]) {
			if (!_game.fHasXtal) {
				switch(_game.iPlanet) {
				case IDI_MSA_PLANET_VENUS:
					if (_game.iRmMenu[_game.iRoom] != 2)
						break;
				default:
					drawObj(
						IDI_MSA_OBJECT_CRYSTAL,
						IDI_MSA_XTAL_ROOM_XY[_game.iPlanet][1],
						IDI_MSA_XTAL_ROOM_XY[_game.iPlanet][2]
					);
					break;
				}
			}
		}

		break;
	}
}

void Mickey::drawRoom() {
	uint8 buffer[256];
	int pBuf = 0;
	int nObjs;

	// Draw room picture
	if (_game.iRoom == IDI_MSA_PIC_TITLE) {
		drawPic(IDI_MSA_PIC_TITLE);
	} else {
		drawPic(_game.iRmPic[_game.iRoom]);

		if (_game.iRoom == IDI_MSA_PIC_SHIP_CONTROLS) {
			// Draw ship control room window
			if (_game.fFlying) {
				drawObj(IDI_MSA_OBJECT_W_SPACE, 0, 0);
			} else {
				drawObj((ENUM_MSA_OBJECT)(IDI_MSA_OBJECT_W_EARTH + _game.iPlanet), 0, 1);
			}
		}
	}

	// Draw room objects
	if (_game.iRoom >= IDI_MSA_MAX_PIC_ROOM) {
		drawRoomAnimation();
		return;
	}

	if (_game.iRmObj[_game.iRoom] != IDI_MSA_OBJECT_NONE) {
		readOfsData(IDO_MSA_ROOM_OBJECT_XY_OFFSETS,
			_game.iRmObj[_game.iRoom], buffer, sizeof(buffer));

		nObjs = buffer[pBuf++];

		for (int iObj = 0; iObj < nObjs; iObj++) {
			drawObj((ENUM_MSA_OBJECT)buffer[pBuf], buffer[pBuf + 1], buffer[pBuf + 2]);
			pBuf += 3;
		}
	}

	// Draw room animation
	drawRoomAnimation();
}

const uint8 colorBCG[16][2] = {
	{ 0x00,	0x00 },	// 0 (black, black)
	{ 0, 0 },
	{ 0x00,	0x0D },	// 2 (black, purple)
	{ 0x00,	0xFF },	// 3 (black, white)
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0x0D,	0x00 },	// 8 (purple, black)
	{ 0, 0 },
	{ 0x0D,	0x0D },	// A (purple, purple)
	{ 0, 0 },
	{ 0xFF,	0x00 },	// C (white, black)
	{ 0, 0 },
	{ 0, 0 },
	{ 0xFF,	0xFF }	// F (white, white)
};

void Mickey::drawLogo() {
	// TODO: clean this up and make it work properly, the logo is drawn way off to the right
	return;	// remove this once the code below is done

	char szFile[256] = {0};
	uint8 *buffer = new uint8[16384];
	const int w = 150;
	const int h = 80;
	const int xoffset = 30;	// FIXME: remove this
	uint8 bitmap[w][h];
	uint8 color, color2, color3, color4;

	// read in logos.bcg
	sprintf(szFile, IDS_MSA_PATH_LOGO);
	Common::File infile;
	if (!infile.open(szFile))
		return;

	infile.read(buffer, infile.size());
	infile.close();

	// draw logo bitmap
	memcpy(bitmap, buffer, sizeof(bitmap));

	_vm->_picture->setDimensions(w, h);

	// Show BCG picture
	for (int y = 0; y < h; y++) {
		for (int x = xoffset; x < w; x++) {
			color  = colorBCG[(bitmap[y][x] & 0xf0) / 0x10][0];	// background
			color2 = colorBCG[(bitmap[y][x] & 0xf0) / 0x10][1];	// background
			color3 = colorBCG[ bitmap[y][x] & 0x0f][0];			// foreground
			color4 = colorBCG[ bitmap[y][x] & 0x0f][1];			// foreground

			_vm->_picture->putPixel(x * 4 - xoffset,			y,		color);
			_vm->_picture->putPixel(x * 4 + 1 - xoffset,		y,		color2);
			_vm->_picture->putPixel(x * 4 + 2 - xoffset,		y,		color3);
			_vm->_picture->putPixel(x * 4 + 3 - xoffset,		y,		color4);
			_vm->_picture->putPixel(x * 4 - xoffset,			y + 1,	color);
			_vm->_picture->putPixel(x * 4 + 1 - xoffset,		y + 1,	color2);
			_vm->_picture->putPixel(x * 4 + 2 - xoffset,		y + 1,	color3);
			_vm->_picture->putPixel(x * 4 + 3 - xoffset,		y + 1,	color4);
		}
	}

	_vm->_picture->showPic(10, 10, w, h);

	delete[] buffer;
}

void Mickey::animate() {
	_vm->_system->delayMillis(IDI_MSA_ANIM_DELAY);
	drawRoomAnimation();
}

void Mickey::printRoomDesc() {
	// print room description
	printDesc(_game.iRoom);
	waitAnyKey(true);

	// print extended room description
	if (_game.oRmTxt[_game.iRoom]) {
		printExeMsg(_game.oRmTxt[_game.iRoom] + IDI_MSA_OFS_EXE);
	}
}

bool Mickey::loadGame() {
	Common::InSaveFile *infile;
	char szFile[256] = {0};
	bool diskerror = true;
	int sel;
	int saveVersion = 0;
	int i = 0;

	while (diskerror) {
		sel = choose1to9(IDO_MSA_LOAD_GAME[1]);
		if (!sel)
			return false;

		// load game
		sprintf(szFile, "%s.s%02d", _vm->getTargetName().c_str(), sel);
		if (!(infile = _vm->getSaveFileMan()->openForLoading(szFile))) {
			printLine("PLEASE CHECK THE DISK DRIVE");

			if (_vm->getSelection(kSelAnyKey) == 0)
				return false;
		} else {
			if (infile->readUint32BE() != MKID_BE('MICK')) {
				warning("Mickey::loadGame wrong save game format");
				return false;
			}

			saveVersion = infile->readByte();
			if (saveVersion < 2) {
				warning("The planet data in this save game is corrupted. Load aborted");
				return false;
			}

			if (saveVersion != MSA_SAVEGAME_VERSION)
				warning("Old save game version (%d, current version is %d). Will try and read anyway, but don't be surprised if bad things happen", saveVersion, MSA_SAVEGAME_VERSION);

			_game.iRoom = infile->readByte();
			_game.iPlanet = infile->readByte();
			_game.iDisk = infile->readByte();

			_game.nAir = infile->readByte();
			_game.nButtons = infile->readByte();
			_game.nRocks = infile->readByte();

			_game.nXtals = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_DAT; i++)
				_game.iPlanetXtal[i] = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_PLANET; i++)
				_game.iClue[i] = infile->readUint16LE();

			infile->read(_game.szAddr, IDI_MSA_MAX_BUTTON + 1);

			_game.fHasXtal = infile->readByte() == 1;
			_game.fIntro = infile->readByte() == 1;
			_game.fSuit = infile->readByte() == 1;
			_game.fShipDoorOpen = infile->readByte() == 1;
			_game.fFlying = infile->readByte() == 1;
			_game.fStoryShown = infile->readByte() == 1;
			_game.fPlanetsInitialized = infile->readByte() == 1;
			_game.fTempleDoorOpen = infile->readByte() == 1;
			_game.fAnimXL30 = infile->readByte() == 1;

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				_game.fItem[i] = infile->readByte() == 1;

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				_game.fItemUsed[i] = infile->readByte() == 1;

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				_game.iItem[i] = infile->readSByte();

			_game.nItems = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_game.iRmObj[i] = infile->readSByte();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_game.iRmPic[i] = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_game.oRmTxt[i] = infile->readUint16LE();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_game.iRmMenu[i] = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_game.nRmMenu[i] = infile->readByte();

			_game.nFrame = infile->readSByte();

			diskerror = false;
			delete infile;
		}
	}

	printExeMsg(IDO_MSA_LOAD_GAME[2]);
	return true;
}

void Mickey::saveGame() {
	Common::OutSaveFile* outfile;
	char szFile[256] = {0};
	bool diskerror = true;
	int sel;
	int i = 0;

	bool fOldDisk = chooseY_N(IDO_MSA_SAVE_GAME[0], false);

	if (fOldDisk)
		printExeStr(IDO_MSA_SAVE_GAME[1]);
	else
		printExeStr(IDO_MSA_SAVE_GAME[2]);

	if (_vm->getSelection(kSelAnyKey) == 0)
		return;

	while (diskerror) {
		sel = choose1to9(IDO_MSA_SAVE_GAME[3]);
		if (!sel)
			return;

		if (fOldDisk)
			printExeStr(IDO_MSA_SAVE_GAME[5]);
		else
			printExeStr(IDO_MSA_SAVE_GAME[4]);

		if (_vm->getSelection(kSelAnyKey) == 0)
			return;

		// save game
		sprintf(szFile, "%s.s%02d", _vm->getTargetName().c_str(), sel);
		if (!(outfile = _vm->getSaveFileMan()->openForSaving(szFile))) {
			printLine("PLEASE CHECK THE DISK DRIVE");

			if (_vm->getSelection(kSelAnyKey) == 0)
				return;
		} else {
			outfile->writeUint32BE(MKID_BE('MICK'));	// header
			outfile->writeByte(MSA_SAVEGAME_VERSION);

			outfile->writeByte(_game.iRoom);
			outfile->writeByte(_game.iPlanet);
			outfile->writeByte(_game.iDisk);

			outfile->writeByte(_game.nAir);
			outfile->writeByte(_game.nButtons);
			outfile->writeByte(_game.nRocks);

			outfile->writeByte(_game.nXtals);

			for (i = 0; i < IDI_MSA_MAX_DAT; i++)
				outfile->writeByte(_game.iPlanetXtal[i]);

			for (i = 0; i < IDI_MSA_MAX_PLANET; i++)
				outfile->writeUint16LE(_game.iClue[i]);

			outfile->write(_game.szAddr, IDI_MSA_MAX_BUTTON + 1);

			outfile->writeByte(_game.fHasXtal ? 1 : 0);
			outfile->writeByte(_game.fIntro ? 1 : 0);
			outfile->writeByte(_game.fSuit ? 1 : 0);
			outfile->writeByte(_game.fShipDoorOpen ? 1 : 0);
			outfile->writeByte(_game.fFlying ? 1 : 0);
			outfile->writeByte(_game.fStoryShown ? 1 : 0);
			outfile->writeByte(_game.fPlanetsInitialized ? 1 : 0);
			outfile->writeByte(_game.fTempleDoorOpen ? 1 : 0);
			outfile->writeByte(_game.fAnimXL30 ? 1 : 0);

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				outfile->writeByte(_game.fItem[i] ? 1 : 0);

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				outfile->writeByte(_game.fItemUsed[i] ? 1 : 0);

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				outfile->writeSByte(_game.iItem[i]);

			outfile->writeByte(_game.nItems);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeSByte(_game.iRmObj[i]);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeByte(_game.iRmPic[i]);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeUint16LE(_game.oRmTxt[i]);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeByte(_game.iRmMenu[i]);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeByte(_game.nRmMenu[i]);

			outfile->writeSByte(_game.nFrame);

			outfile->finalize();

			if (outfile->err())
				warning("Can't write file '%s'. (Disk full?)", szFile);

			diskerror = false;
			delete outfile;
		}
	}

	printExeMsg(IDO_MSA_SAVE_GAME[6]);
}

void Mickey::showPlanetInfo() {
	for (int i = 0; i < 4; i++) {
		printExeStr(IDO_MSA_PLANET_INFO[_game.iPlanet][i]);
		waitAnyKey();
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
	waitAnyKey();

	_vm->clearScreen(IDA_DEFAULT);
	for (iRow = 0; iRow < 21; iRow++) {
		strcpy(szLine, buffer + pBuf);
		_vm->drawStr(iRow, 0, IDA_DEFAULT, szLine);
		pBuf += strlen(szLine) + 1;
	}
	waitAnyKey();

	//Set back to black
	_vm->_gfx->clearScreen(0);
	_vm->_gfx->doUpdate();

	drawRoom();

	_game.fStoryShown = true;
}

int Mickey::getPlanet() {
	if (!_game.nButtons)
		return -1;

	for (int iPlanet = 0; iPlanet < IDI_MSA_MAX_DAT; iPlanet++) {
		if (!strcmp(IDS_MSA_ADDR_PLANET[iPlanet], _game.szAddr)) {
			return iPlanet;
		}
	}

	return -1;
}

void Mickey::pressOB(int iButton) {
	char szButtons[12] = {0};

	// check if too many buttons pressed
	if (_game.nButtons == IDI_MSA_MAX_BUTTON) {
		_game.nButtons = 0;
		memset(_game.szAddr, 0, sizeof(_game.szAddr));
		printExeMsg(IDO_MSA_TOO_MANY_BUTTONS_PRESSED);
		return;
	}

	// add button press to address
	_game.nButtons++;
	_game.szAddr[_game.nButtons - 1] = (char)iButton;

	// format buttons string
	for (int i = 0; i < IDI_MSA_MAX_BUTTON; i++) {
		szButtons[i * 2] = _game.szAddr[i];
		if (_game.szAddr[i + 1]) szButtons[(i * 2) + 1] = ',';
	}

	// print pressed buttons
	printLine("MICKEY HAS PRESSED:                   ");
	_vm->drawStr(20, 22, IDA_DEFAULT, szButtons);
	waitAnyKey();
}

void Mickey::insertDisk(int iDisk) {
	_vm->clearTextArea();
	_vm->drawStr(IDI_MSA_ROW_INSERT_DISK, IDI_MSA_COL_INSERT_DISK, IDA_DEFAULT, (const char *)IDS_MSA_INSERT_DISK[iDisk]);
	waitAnyKey();
}

void Mickey::gameOver() {
	// We shouldn't run the game over segment if we're quitting.
	if (_vm->shouldQuit())
		return;

	drawPic(IDI_MSA_PIC_EARTH_SHIP_LEAVING);
	printExeMsg(IDO_MSA_GAME_OVER[3]);
	playSound(IDI_MSA_SND_GAME_OVER);

	if (_game.fItemUsed[IDI_MSA_ITEM_LETTER]) {
		drawPic(IDI_MSA_PIC_EARTH_MINNIE);
		printExeMsg(IDO_MSA_GAME_OVER[4]);
		printExeMsg(IDO_MSA_GAME_OVER[5]);
	} else {
		printExeMsg(IDO_MSA_GAME_OVER[6]);
		printExeMsg(IDO_MSA_GAME_OVER[7]);
	}

	waitAnyKey();
}

void Mickey::flipSwitch() {
	if (_game.fHasXtal || _game.nXtals) {
		if (!_game.fStoryShown)
			printStory();

		// Initialize planet data
		if (!_game.fPlanetsInitialized) {
			int iHint = 0;
			int iPlanet = 0;

			memset(_game.iPlanetXtal, 0, sizeof(_game.iPlanetXtal));
			memset(_game.iClue, 0, sizeof(_game.iClue));

			_game.iPlanetXtal[0] = IDI_MSA_PLANET_EARTH;
			_game.iPlanetXtal[8] = IDI_MSA_PLANET_URANUS;

			for (int i = 1; i < 9; i++) {
				if (i < 8) {
					do {
						// Earth (planet 0) and Uranus (planet 8) are excluded
						iPlanet = _vm->rnd(IDI_MSA_MAX_PLANET - 2);
					} while (planetIsAlreadyAssigned(iPlanet));
				} else {
					iPlanet = IDI_MSA_PLANET_URANUS;	// Uranus is always last
				}

				_game.iPlanetXtal[i] = iPlanet;
				iHint = _vm->rnd(5) - 1;	// clues are 0-4
				_game.iClue[i] = IDO_MSA_NEXT_PIECE[iPlanet][iHint];
			}

			_game.fPlanetsInitialized = true;
		}

		// activate screen animation
		_game.fAnimXL30 = true;

		_vm->clearTextArea();
		playSound(IDI_MSA_SND_XL30);
		printExeMsg(IDO_MSA_XL30_SPEAKING);

		if (_game.fHasXtal) {
			_game.fHasXtal = false;
			printExeMsg(IDO_MSA_CRYSTAL_PIECE_FOUND);
		}

		if (_game.nXtals == IDI_MSA_MAX_PLANET) {
			printExeMsg(IDO_MSA_GAME_OVER[0]);
			printExeMsg(IDO_MSA_GAME_OVER[1]);
			printExeMsg(IDO_MSA_GAME_OVER[2]);

#if 0
			// DEBUG
			strcpy(_game.szAddr, (char *)IDS_MSA_ADDR_PLANET[IDI_MSA_PLANET_EARTH]);
			_game.nButtons = strlen(_game.szAddr);
#endif

		} else {
			printExeStr(_game.iClue[_game.nXtals]);

#if 0
			// DEBUG
			_vm->drawStr(24, 12, IDA_DEFAULT, (char *)IDS_MSA_NAME_PLANET_2[_game.iPlanetXtal[_game.nXtals]]);
			_vm->drawStr(24, 22, IDA_DEFAULT, (char *)IDS_MSA_ADDR_PLANET[_game.iPlanetXtal[_game.nXtals]]);
			strcpy(_game.szAddr, (char *)IDS_MSA_ADDR_PLANET[_game.iPlanetXtal[_game.nXtals]]);
			_game.nButtons = strlen(_game.szAddr);
			_vm->_gfx->doUpdate();
			_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
#endif

			waitAnyKey(true);
		}
	} else {
		printStory();
	}
}

void Mickey::inventory() {
	int iRow = IDI_MSA_ROW_INV_ITEMS;
	char szCrystals[12] = {0};

	sprintf(szCrystals, IDS_MSA_CRYSTALS, IDS_MSA_CRYSTAL_NO[_game.nXtals]);

	CursorMan.showMouse(false);

	_vm->clearScreen(IDA_DEFAULT);
	_vm->drawStr(IDI_MSA_ROW_INV_TITLE, IDI_MSA_COL_INV_TITLE, IDA_DEFAULT, IDS_MSA_INVENTORY);
	_vm->drawStr(IDI_MSA_ROW_INV_CRYSTALS, IDI_MSA_COL_INV_ITEMS, IDA_DEFAULT, szCrystals);

	for (int iItem = 0; iItem < IDI_MSA_MAX_ITEM; iItem++) {
		if (_game.fItem[_game.iItem[iItem]] && (_game.iItem[iItem] != IDI_MSA_OBJECT_NONE)) {
			_vm->drawStr(iRow++, IDI_MSA_COL_INV_ITEMS, IDA_DEFAULT, (const char *)IDS_MSA_NAME_ITEM[_game.iItem[iItem]]);
		}
	}

	waitAnyKey();

	_vm->clearScreen(IDA_DEFAULT);

	CursorMan.showMouse(true);
}

void Mickey::intro() {
	// Draw Sierra logo
	//drawLogo();		// Original does not even show this, so we skip it too
	//waitAnyKey();		// Not in the original, but needed so that the logo is visible

	// draw title picture
	_game.iRoom = IDI_MSA_PIC_TITLE;
	drawRoom();

	// show copyright and play theme
	printExeMsg(IDO_MSA_COPYRIGHT);

	// Quit if necessary
	if (_vm->shouldQuit())
		return;

	playSound(IDI_MSA_SND_THEME);

	// load game
	_game.fIntro = true;
	if (chooseY_N(IDO_MSA_LOAD_GAME[0], true)) {
		if (loadGame()) {
			_game.iPlanet = IDI_MSA_PLANET_EARTH;
			_game.fIntro = false;
			_game.iRoom = IDI_MSA_PIC_SHIP_CORRIDOR;
			return;
		}
	}

	// Quit if necessary
	if (_vm->shouldQuit())
		return;

	// play spaceship landing scene
	_game.iPlanet = IDI_MSA_PLANET_EARTH;
	_game.iRoom = IDI_MSA_PIC_EARTH_ROAD_4;

	drawRoom();
	printRoomDesc();

	// Quit if necessary
	if (_vm->shouldQuit())
		return;

	playSound(IDI_MSA_SND_SHIP_LAND);

	// Flash screen 3 times
	for (byte i = 0; i < 3; i++) {
		playSound(IDI_MSA_SND_PRESS_BLUE);

		//Set screen to white
		_vm->_gfx->clearScreen(15);
		_vm->_gfx->doUpdate();
		_vm->_system->updateScreen();

		_vm->_system->delayMillis(IDI_MSA_ANIM_DELAY);

		//Set back to black
		_vm->_gfx->clearScreen(0);
		_vm->_gfx->doUpdate();
		_vm->_system->updateScreen();

		drawRoom();
		printDesc(_game.iRoom);
	}

	printExeMsg(IDO_MSA_INTRO);
}

void Mickey::getItem(ENUM_MSA_ITEM iItem) {
	_game.fItem[iItem] = true;
	_game.iItem[_game.nItems++] = iItem;
	_game.oRmTxt[_game.iRoom] = 0;
	playSound(IDI_MSA_SND_TAKE);
	drawRoom();
}

void Mickey::getXtal(int iStr) {
	_game.oRmTxt[_game.iRoom] = 0;
	_game.fHasXtal = true;
	_game.nXtals++;
	playSound(IDI_MSA_SND_CRYSTAL);
	drawRoom();
	printDatMessage(iStr);
}

bool Mickey::parse(int cmd, int arg) {
	switch(cmd) {

	// BASIC

	case IDI_MSA_ACTION_GOTO_ROOM:
		_game.iRoom = arg;
		return true;
	case IDI_MSA_ACTION_SHOW_INT_STR:
		printLine(IDS_MSA_ERRORS[arg]);
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
		printLine("YOU CAN SEE MICKEY ALREADY");
		break;

	// EARTH

	case IDI_MSA_ACTION_GET_ROPE:
		if (_game.iRmMenu[_game.iRoom] == 2) {
			_game.iRmObj[_game.iRoom] = IDI_MSA_OBJECT_NONE;
			_game.iRmMenu[_game.iRoom] = 3;
			getItem(IDI_MSA_ITEM_ROPE);
			printLine("MICKEY TAKES THE ROPE");
		} else {
			_game.iRmMenu[_game.iRoom] = 1;
			printDatMessage(11);
		}
		break;
	case IDI_MSA_ACTION_UNTIE_ROPE:
		_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_EARTH_TIRE_SWING_1;
		_game.iRmObj[_game.iRoom] = 0;
		_game.iRmMenu[_game.iRoom] = 2;
		drawRoom();
		printDatMessage(12);
		break;
	case IDI_MSA_ACTION_GET_BONE:
		_game.iRmObj[_game.iRoom] = IDI_MSA_OBJECT_NONE;
		_game.iRmMenu[_game.iRoom] = 1;
		getItem(IDI_MSA_ITEM_BONE);
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_GET_XTAL_EARTH:
		_game.iRmMenu[_game.iRoom] = 1;
		getXtal(arg);
		break;
	case IDI_MSA_ACTION_LOOK_DESK:
		_game.iRmMenu[_game.iRoom] = 1;
		_game.iRmObj[_game.iRoom] = 2;
		drawRoom();
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_WRITE_LETTER:
		_game.iRmMenu[_game.iRoom] = 3;
		_game.iRmMenu[IDI_MSA_PIC_EARTH_MAILBOX] = 1;
		_game.iRmObj[_game.iRoom] = IDI_MSA_OBJECT_NONE;
		getItem(IDI_MSA_ITEM_LETTER);
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_MAIL_LETTER:
		_game.fItemUsed[IDI_MSA_ITEM_LETTER] = true;
		_game.fItem[IDI_MSA_ITEM_LETTER] = false;
		_game.iRmMenu[_game.iRoom] = 0;
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_OPEN_MAILBOX:
		if (_game.fItemUsed[IDI_MSA_ITEM_LETTER]) {
			printDatMessage(110);
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_CUPBOARD:
		if (_game.iRmMenu[_game.iRoom]) {
			if (_game.iRmObj[_game.iRoom] == IDI_MSA_OBJECT_NONE) {
				printDatMessage(78);
			} else {
				printDatMessage(arg);
			}
		} else {
			_game.iRmMenu[_game.iRoom] = 1;
			_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_EARTH_KITCHEN_1;
			_game.iRmObj[_game.iRoom] = 3;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_FLASHLIGHT:
		if (!mickeyHasItem(IDI_MSA_ITEM_FLASHLIGHT)) {
			_game.iRmObj[_game.iRoom] = IDI_MSA_OBJECT_NONE;
			getItem(IDI_MSA_ITEM_FLASHLIGHT);
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_CABINET:
		if (_game.iRmMenu[_game.iRoom]) {
			printDatMessage(109);
		} else {
			_game.iRmMenu[_game.iRoom] = 1;
			_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_EARTH_GARAGE_1;
			_game.iRmObj[_game.iRoom] = 15;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_CROWBAR:
		if (!mickeyHasItem(IDI_MSA_ITEM_CROWBAR)) {
			_game.iRmObj[_game.iRoom]--;
			getItem(IDI_MSA_ITEM_CROWBAR);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_WRENCH:
		if (!mickeyHasItem(IDI_MSA_ITEM_WRENCH)) {
			_game.iRmObj[_game.iRoom] -= 2;
			getItem(IDI_MSA_ITEM_WRENCH);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_CLOSET:
		if (_game.iRmMenu[_game.iRoom]) {
			printDatMessage(99);
		} else {
			_game.iRmMenu[_game.iRoom] = 1;
			_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_EARTH_BEDROOM_1;
			_game.iRmObj[_game.iRoom] = 7;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_MATTRESS:
		if (!mickeyHasItem(IDI_MSA_ITEM_MATTRESS)) {
			_game.iRmObj[_game.iRoom]--;
			getItem(IDI_MSA_ITEM_MATTRESS);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_SCARF:
		if (!mickeyHasItem(IDI_MSA_ITEM_SCARF)) {
			_game.iRmObj[_game.iRoom] -= 2;
			getItem(IDI_MSA_ITEM_SCARF);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_SUNGLASSES:
		if (!mickeyHasItem(IDI_MSA_ITEM_SUNGLASSES)) {
			_game.iRmObj[_game.iRoom]--;
			getItem(IDI_MSA_ITEM_SUNGLASSES);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_SCALE:
		if (!mickeyHasItem(IDI_MSA_ITEM_SCALE)) {
			_game.iRmMenu[IDI_MSA_PIC_VENUS_WEIGH] = 1;
			_game.iRmMenu[IDI_MSA_PIC_NEPTUNE_WEIGH] = 1;
			_game.iRmMenu[IDI_MSA_PIC_MERCURY_WEIGH] = 1;
			_game.iRmMenu[IDI_MSA_PIC_SATURN_WEIGH] = 1;
			_game.iRmMenu[IDI_MSA_PIC_PLUTO_WEIGH] = 1;
			_game.iRmMenu[IDI_MSA_PIC_JUPITER_WEIGH] = 1;
			_game.iRmMenu[IDI_MSA_PIC_MARS_WEIGH] = 1;
			_game.iRmMenu[IDI_MSA_PIC_URANUS_WEIGH] = 1;
			_game.iRmObj[_game.iRoom] -= 2;
			getItem(IDI_MSA_ITEM_SCALE);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GOTO_SPACESHIP:
		_game.iRoom = IDI_MSA_PIC_SHIP_AIRLOCK;
		if (_game.iPlanet != IDI_MSA_PLANET_EARTH)
			insertDisk(0);
		return true;

	// VENUS

	case IDI_MSA_ACTION_DOWN_CHASM:
		if (_game.fItem[IDI_MSA_ITEM_ROPE]) {
			_game.iRmMenu[_game.iRoom] = 1;
		}
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_DOWN_ROPE:
		if (_game.fItemUsed[IDI_MSA_ITEM_ROPE]) {
			_game.iRoom = IDI_MSA_PIC_VENUS_PROBE;
			return true;
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_ROPE:
		if (_game.fItemUsed[IDI_MSA_ITEM_ROPE]) {
			printDatMessage(22);
		} else {
			_game.fItemUsed[IDI_MSA_ITEM_ROPE] = true;
			_game.fItem[IDI_MSA_ITEM_ROPE] = false;
			_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_VENUS_CHASM_1;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_HATCH:
		if (_game.fItemUsed[IDI_MSA_ITEM_WRENCH]) {
			if ((_game.iRmMenu[_game.iRoom] == 3) || (_game.iRmPic[_game.iRoom] == IDI_MSA_PIC_VENUS_PROBE_1))
				printDatMessage(39);
			else {
				_game.iRmMenu[_game.iRoom] = 2;
				_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_VENUS_PROBE_1;
				drawRoom();
				printDatMessage(24);
			}
		} else {
			if (_game.fItem[IDI_MSA_ITEM_WRENCH]) {
				_game.iRmMenu[_game.iRoom] = 1;
			}

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_WRENCH:
		_game.fItemUsed[IDI_MSA_ITEM_WRENCH] = true;
		printDatString(arg);

		if (_game.iRmPic[_game.iRoom] == IDI_MSA_PIC_VENUS_PROBE_1) {
			_vm->clearRow(22);
		}

		waitAnyKey();
		break;
	case IDI_MSA_ACTION_GET_XTAL_VENUS:
		_game.iRmMenu[_game.iRoom] = 3;
		getXtal(arg);
		break;

	// TRITON (NEPTUNE)

	case IDI_MSA_ACTION_LOOK_CASTLE:
		if (!_game.iRmMenu[_game.iRoom]) {
			_game.iRmMenu[_game.iRoom] = 1;
		}
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_ENTER_OPENING:
		if (_game.fItemUsed[IDI_MSA_ITEM_CROWBAR]) {
			_game.iRoom = IDI_MSA_PIC_NEPTUNE_CASTLE_4;

			return true;
		} else {
			if (_game.fItem[IDI_MSA_ITEM_CROWBAR]) {
				_game.iRmMenu[_game.iRoom] = 2;
			}

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_CROWBAR:
		_game.fItemUsed[IDI_MSA_ITEM_CROWBAR] = true;
		_game.iRmMenu[_game.iRoom] = 1;
		_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_NEPTUNE_ENTRANCE_1;
		drawRoom();
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_GET_XTAL_NEPTUNE:
		if (_game.fHasXtal) {
			printDatMessage(71);
		} else {
			if (_game.fItem[IDI_MSA_ITEM_SCARF]) {
				_game.iRmMenu[_game.iRoom] = 1;
			}

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_TALK_LEADER:
		_game.iRoom = IDI_MSA_PIC_NEPTUNE_ENTRYWAY;

		printDatMessage(arg);
		return true;
	case IDI_MSA_ACTION_GIVE_SCARF:
		_game.iRmObj[_game.iRoom] = 18;
		getXtal(arg);
		_game.fItem[IDI_MSA_ITEM_SCARF] = false;
		_game.iRmMenu[_game.iRoom] = 0;
		_game.iRmMenu[IDI_MSA_PIC_EARTH_BEDROOM] = 2;
		_game.iRoom = IDI_MSA_PIC_NEPTUNE_ENTRYWAY;

		return true;

	// MERCURY

	case IDI_MSA_ACTION_GET_XTAL_MERCURY:
		if (_game.fHasXtal) {
			_game.iRmMenu[_game.iRoom] = 2;
			printDatMessage(32);
		} else {
			if (_game.fItem[IDI_MSA_ITEM_SUNGLASSES]) {
				_game.iRmMenu[_game.iRoom] = 1;
			}
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GIVE_SUNGLASSES:
		_game.iRmObj[_game.iRoom] = 17;
		_game.iRmMenu[_game.iRoom] = 2;
		_game.fItem[IDI_MSA_ITEM_SUNGLASSES] = false;

		getXtal(arg);

		break;

	// TITAN (SATURN)

	case IDI_MSA_ACTION_CROSS_LAKE:
		if (_game.fItem[IDI_MSA_ITEM_MATTRESS]) {
			_game.iRmMenu[IDI_MSA_PIC_SATURN_LAKE_0] = 1;
			_game.iRmMenu[IDI_MSA_PIC_SATURN_LAKE_1] = 1;
			_game.iRmMenu[IDI_MSA_PIC_SATURN_LAKE_2] = 1;
		}

		printDatMessage(arg);

		break;
	case IDI_MSA_ACTION_USE_MATTRESS:
		_game.iRoom = IDI_MSA_PIC_SATURN_ISLAND;

		printDatMessage(arg);

		return true;
	case IDI_MSA_ACTION_GET_XTAL_SATURN:
		if (_game.fHasXtal) {
			printDatMessage(29);
		} else {
			getXtal(arg);
		}
		break;
	case IDI_MSA_ACTION_LEAVE_ISLAND:
		_game.iRoom = IDI_MSA_PIC_SATURN_LAKE_1;

		printDatMessage(arg);

		return true;

	// PLUTO

	case IDI_MSA_ACTION_GET_XTAL_PLUTO:
		if (_game.fHasXtal) {
			printDatMessage(19);
		} else {
			if (_game.fItem[IDI_MSA_ITEM_BONE]) {
				_game.iRmMenu[_game.iRoom] = 1;
			}
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GIVE_BONE:
		_game.fItem[IDI_MSA_ITEM_BONE] = false;
		_game.iRmMenu[_game.iRoom] = 0;
		_game.iRmObj[_game.iRoom] = 16;

		getXtal(arg);

		break;

	// IO (JUPITER)

	case IDI_MSA_ACTION_GET_ROCK_0:
	case IDI_MSA_ACTION_GET_ROCK_1:
		if (_game.fItem[IDI_MSA_ITEM_ROCK]) {
			printDatMessage(38);
		} else {
			_game.iRmMenu[_game.iRoom] = 1;
			_game.iRmObj[_game.iRoom] = IDI_MSA_OBJECT_NONE;
			getItem(IDI_MSA_ITEM_ROCK);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_XTAL_JUPITER:
		if (_game.fHasXtal) {
			printDatMessage(15);
		} else {
			switch (_game.nRocks) {
			case 0:
				if (_game.fItem[IDI_MSA_ITEM_ROCK]) {
					_game.iRmMenu[_game.iRoom] = 1;
				}
				printDatMessage(arg);
				break;
			case 1:
				if (_game.fItem[IDI_MSA_ITEM_ROCK]) {
					_game.iRmMenu[_game.iRoom] = 1;
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
		_game.fItem[IDI_MSA_ITEM_ROCK] = false;
		_game.nItems--;
		_game.iRmObj[_game.iRoom]++;
		_game.iRmMenu[_game.iRoom] = 0;

		drawRoom();

		if (_game.nRocks) {
			printDatMessage(37);
		} else {
			printDatMessage(arg);
		}

		_game.nRocks++;
		break;

	// MARS

	case IDI_MSA_ACTION_GO_TUBE:
		if (_game.fItem[IDI_MSA_ITEM_FLASHLIGHT]) {
			_game.iRmMenu[_game.iRoom] = 1;
		}

		printDatMessage(arg);

		break;
	case IDI_MSA_ACTION_USE_FLASHLIGHT:
		_game.iRoom = IDI_MSA_PIC_MARS_TUBE_1;

		printDatMessage(15);

		return true;
	case IDI_MSA_ACTION_PLUTO_DIG:
		if (_game.fHasXtal) {
			printDatMessage(21);
		} else {
			getXtal(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_XTAL_MARS:
		if (_game.fHasXtal) {
			printDatMessage(23);
		} else {
			printDatMessage(arg);
		}
		break;

	// OBERON (URANUS)

	case IDI_MSA_ACTION_ENTER_TEMPLE:
		_game.iRoom = IDI_MSA_PIC_URANUS_TEMPLE;

		return true;
	case IDI_MSA_ACTION_USE_CRYSTAL:
		if (_game.iRmMenu[_game.iRoom]) {
			printDatMessage(25);
		} else {
			_game.iRmMenu[_game.iRoom] = 1;
			_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_URANUS_TEMPLE_1;

			drawRoom();

			_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_URANUS_TEMPLE;

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_DOOR:
		if (_game.fTempleDoorOpen) {
			printDatMessage(36);
		} else {
			_game.fTempleDoorOpen = 1;
			_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_URANUS_TEMPLE_2;

			drawRoom();

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_ENTER_DOOR:
		if (_game.fTempleDoorOpen) {
			_game.iRoom = IDI_MSA_PIC_URANUS_STEPS;

			return true;
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_XTAL_URANUS:
		if (_game.fHasXtal) {
			printDatMessage(34);
		} else {
			if (_game.fItem[IDI_MSA_ITEM_CROWBAR]) {
				_game.iRmMenu[_game.iRoom] = 1;
			}
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_CROWBAR_1:
		_game.iRmMenu[_game.iRoom] = 0;

		getXtal(arg);

		break;

	// SPACESHIP

	case IDI_MSA_ACTION_GO_NORTH:
		if (_game.fShipDoorOpen) {
			if (_game.fSuit) {
				printDatMessage(45);
			} else {
				_game.iRoom = IDI_MSA_PIC_SHIP_CORRIDOR;
				return true;
			}
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GO_PLANET:
		if (!_game.fShipDoorOpen) {
			if ((_game.nXtals == IDI_MSA_MAX_PLANET) && (_game.iPlanet == IDI_MSA_PLANET_EARTH))
				gameOver();
			if ((_game.iPlanet == _game.iPlanetXtal[_game.nXtals]) || (_game.iPlanet == IDI_MSA_PLANET_EARTH)) {
				_game.fHasXtal = false;
				_game.iRoom = IDI_MSA_HOME_PLANET[_game.iPlanet];

				if (_game.iPlanet != IDI_MSA_PLANET_EARTH)
					insertDisk(1);

				return true;
			} else {
				_game.iRoom = IDI_MSA_SHIP_PLANET[_game.iPlanet];

				return true;
			}
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_PRESS_BUTTON:
		if (_game.fShipDoorOpen) {		// inner door open
			if (_game.iPlanet && !_game.fSuit) {
				printDatMessage(arg);
			} else {
				_game.fShipDoorOpen = false;
				_game.iRmPic[_game.iRoom]--;

				drawRoom();

				printDatMessage(2);
			}
		} else {
			_game.fShipDoorOpen = true;
			_game.iRmPic[_game.iRoom]++;

			drawRoom();

			printDatMessage(14);
		}
		break;
	case IDI_MSA_ACTION_WEAR_SPACESUIT:
		if (_game.fSuit) {
			if (_game.fShipDoorOpen) {
				_game.fSuit = false;
				_game.iRmMenu[_game.iRoom] = 0;
				_game.iRmPic[_game.iRoom] -= 2;

				drawRoom();

				printDatMessage(13);
			} else {
				printDatMessage(3);
			}
		} else {
			if (_game.iPlanet) {
				_game.fSuit = true;
				_game.iRmMenu[_game.iRoom] = 1;
				_game.iRmPic[_game.iRoom] += 2;

				drawRoom();

				printDatMessage(arg);
			} else {
				printDatMessage(12);
			}
		}
		break;
	case IDI_MSA_ACTION_READ_GAUGE:
		printDatString(arg);
		_vm->drawStr(21, 15, IDA_DEFAULT, (const char *)IDS_MSA_TEMP_C[_game.iPlanet]);
		_vm->drawStr(21, 23, IDA_DEFAULT, (const char *)IDS_MSA_TEMP_F[_game.iPlanet]);

		waitAnyKey();

		break;
	case IDI_MSA_ACTION_PRESS_ORANGE:
		if (_game.fFlying) {
			printDatMessage(4);
		} else {
			playSound(IDI_MSA_SND_PRESS_ORANGE);
			printDatMessage(arg);
			pressOB(IDI_MSA_BUTTON_ORANGE);
		}
		break;
	case IDI_MSA_ACTION_PRESS_BLUE:
		if (_game.fFlying) {
			printDatMessage(4);
		} else {
			playSound(IDI_MSA_SND_PRESS_BLUE);
			printDatMessage(arg);
			pressOB(IDI_MSA_BUTTON_BLUE);
		}
		break;
	case IDI_MSA_ACTION_FLIP_SWITCH:
		flipSwitch();
		break;
	case IDI_MSA_ACTION_PUSH_THROTTLE:
		if (_game.fFlying) {
			_game.fFlying = false;
			_game.nButtons = 0;

			memset(_game.szAddr, 0, sizeof(_game.szAddr));

			drawRoom();

			printDatString(22);

			_vm->drawStr(IDI_MSA_ROW_PLANET, IDI_MSA_COL_PLANET, IDA_DEFAULT,
						(const char *)IDS_MSA_PLANETS[_game.iPlanet]);

			waitAnyKey(true);

			showPlanetInfo();
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_PULL_THROTTLE:
		if (_game.fFlying) {
			printDatMessage(18);
		} else {
			if (getPlanet() != -1) {
				_game.fFlying = true;
				_game.iPlanet = getPlanet();

				drawRoom();

				printDatMessage(16);
			} else {
				_game.nButtons = 0;

				memset(_game.szAddr, 0, sizeof(_game.szAddr));

				printDatMessage(17);
			}
		}
		break;
	case IDI_MSA_ACTION_LEAVE_ROOM:
		if (_game.fFlying) {
			printDatMessage(24);
		} else {
			_game.iRoom = arg;
			return true;
		}
		break;
	case IDI_MSA_ACTION_OPEN_CABINET_1:
		if (_game.iRmMenu[_game.iRoom]) {
			printLine("THE CABINET IS ALREADY OPEN");
		} else {
			_game.iRmMenu[_game.iRoom] = 1;
			_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_SHIP_KITCHEN_1;

			drawRoom();

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_READ_MAP:
		_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_STAR_MAP;

		drawRoom();

		printDatMessage(46);
		printDatMessage(47);
		printDatMessage(48);

		_game.iRmPic[_game.iRoom] = IDI_MSA_PIC_SHIP_BEDROOM;

		drawRoom();
		break;
	case IDI_MSA_ACTION_GO_WEST:
		_game.nButtons = 0;

		memset(_game.szAddr, 0, sizeof(_game.szAddr));

		_game.iRoom = arg;

		return true;
	}

	return false;
}

// Keyboard

void Mickey::waitAnyKey(bool anim) {
	Common::Event event;

	if (!anim)
		_vm->_gfx->doUpdate();

	while (!_vm->shouldQuit()) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
			case Common::EVENT_KEYDOWN:
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_RBUTTONUP:
				return;
			default:
				break;
			}
		}

		if (anim) {
			animate();
			_vm->_gfx->doUpdate();
		}

		_vm->_system->updateScreen();
		_vm->_system->delayMillis(10);
	}
}

// Console-related functions

void Mickey::debugCurRoom() {
	_vm->_console->DebugPrintf("Current Room = %d\n", _game.iRoom);

	if (_game.iRmObj[_game.iRoom] != IDI_MSA_OBJECT_NONE) {
		_vm->_console->DebugPrintf("Object %d is in the room\n", _game.iRmObj[_game.iRoom]);
	}
}

void Mickey::debugGotoRoom(int room) {
	_game.iRoom = room;
	drawRoom();
}

Mickey::Mickey(PreAgiEngine *vm) : _vm(vm) {
	_vm->_console = new Mickey_Console(_vm, this);
}

Mickey::~Mickey() {
}

void Mickey::init() {
	uint8 buffer[512];

	// clear game struct
	memset(&_game, 0, sizeof(_game));
	memset(&_game.iItem, IDI_MSA_OBJECT_NONE, sizeof(_game.iItem));
	// read room extended desc flags
	//readExe(IDO_MSA_ROOM_TEXT, buffer, sizeof(buffer));
	//memcpy(_game.fRmTxt, buffer, sizeof(_game.fRmTxt));

	// read room extended desc offsets
	readExe(IDO_MSA_ROOM_TEXT_OFFSETS, buffer, sizeof(buffer));
	memcpy(_game.oRmTxt, buffer, sizeof(_game.oRmTxt));
	for (int i = 0; i < IDI_MSA_MAX_ROOM; i++)
		_game.oRmTxt[i] = buffer[i*2] + 256 * buffer[i*2+1];

	// read room object indices
	//readExe(IDO_MSA_ROOM_OBJECT, buffer, sizeof(buffer));
	//memcpy(_game.iRmObj, buffer, sizeof(_game.iRmObj));

	// read room picture indices
	//readExe(IDO_MSA_ROOM_PICTURE, buffer, sizeof(buffer));
	//memcpy(_game.iRmPic, buffer, sizeof(_game.iRmPic));

	// read room menu patch indices
	readExe(IDO_MSA_ROOM_MENU_FIX, buffer, sizeof(buffer));
	memcpy(_game.nRmMenu, buffer, sizeof(_game.nRmMenu));

	// set room picture and room object indices
	for (int i = 0; i < IDI_MSA_MAX_ROOM; i++) {
		_game.iRmPic[i] = i;
		_game.iRmObj[i] = -1;
	}
	_game.iRmPic[IDI_MSA_PIC_SHIP_AIRLOCK] = IDI_MSA_PIC_SHIP_AIRLOCK_0;
	_game.iRmObj[IDI_MSA_PIC_EARTH_BATHROOM] = 11;
	_game.iRmObj[IDI_MSA_PIC_JUPITER_LAVA] = 21;
	_game.iRmObj[IDI_MSA_PIC_JUPITER_ROCK_0] = 20;
	_game.iRmObj[IDI_MSA_PIC_JUPITER_ROCK_1] = 19;
	_game.iRmObj[IDI_MSA_PIC_EARTH_IN_DOGHOUSE] = 1;

#if 0
	// DEBUG
	_game.iPlanet = IDI_MSA_PLANET_EARTH;
	_game.iRoom = IDI_MSA_PIC_SHIP_CONTROLS;
	_game.fHasXtal = true;
	_game.nXtals = 9;
	_game.fItemUsed[IDI_MSA_ITEM_LETTER] = true;

#endif
}

void Mickey::run() {
	bool done;

	// Game intro
	intro();

	// Game loop
	while (!_vm->shouldQuit()) {
		drawRoom();

		if (_game.fIntro) {
			_game.fIntro = false;
		} else {
			printRoomDesc();
		}

		if (_game.iRoom == IDI_MSA_PIC_NEPTUNE_GUARD) {
			_game.iRoom = IDI_MSA_PIC_NEPTUNE_LEADER;
			done = true;
		} else {
			done = false;
		}

		while (!done && !_vm->shouldQuit()) {
			// Check air supply
			if (_game.fSuit) {
				_game.nAir -= 1;
				for (int i = 0; i < 4; i++) {
					if (_game.nAir == IDI_MSA_AIR_SUPPLY[i]) {
						playSound(IDI_MSA_SND_XL30);
						printExeMsg(IDO_MSA_XL30_SPEAKING);
						printExeMsg(IDO_MSA_AIR_SUPPLY[i]);
						if (i == 3)
							return;
					}
				}
			} else {
				_game.nAir = 50;	// max air supply
			}

			done = checkMenu();
		}

		_game.nFrame = 0;
	}

	gameOver();
}

}
