/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/events.h"
#include "common/savefile.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"

#include "agi/preagi/preagi.h"
#include "agi/preagi/mickey.h"
#include "agi/graphics.h"

namespace Agi {

int MickeyEngine::getDat(int iRoom) {
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

void MickeyEngine::readExe(int ofs, uint8 *buffer, long buflen) {
	Common::File infile;
	if (!infile.open("mickey.exe"))
		return;
	infile.seek(ofs, SEEK_SET);
	infile.read(buffer, buflen);
	infile.close();
}

void MickeyEngine::getDatFileName(int iRoom, char *szFile) {
	Common::sprintf_s(szFile, 256, IDS_MSA_PATH_DAT, IDS_MSA_NAME_DAT[getDat(iRoom)]);
}

void MickeyEngine::readDatHdr(char *szFile, MSA_DAT_HEADER *hdr) {
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

void MickeyEngine::readOfsData(int offset, int iItem, uint8 *buffer, long buflen) {
	uint16 ofs[256];

	readExe(offset, buffer, buflen);
	memcpy(ofs, buffer, sizeof(ofs));

	for (int i = 0; i < 256; i++)
		ofs[i] = buffer[i * 2] + 256 * buffer[i * 2 + 1];

	readExe(ofs[iItem] + IDI_MSA_OFS_EXE, buffer, buflen);
}

// User Interface

bool MickeyEngine::chooseY_N(int ofsPrompt, bool fErrorMsg) {
	printExeStr(ofsPrompt);

	while (!shouldQuit()) {
		switch (getSelection(kSelYesNo)) {
		case 0:
			return false;
		case 1:
			return true;
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

int MickeyEngine::choose1to9(int ofsPrompt) {
	int answer = 0;
	printExeStr(ofsPrompt);

	while (!shouldQuit()) {
		answer = getSelection(kSelNumber);
		if (answer == 10) {
			printExeStr(IDO_MSA_PRESS_1_TO_9);
			if (getSelection(kSelAnyKey) == 0)
				return 0;
			printExeStr(ofsPrompt);
		} else return answer;
	}

	return 0;
}

void MickeyEngine::printStr(char *buffer) {
	clearTextArea();

	int pc = 1;
	const int nRows = *buffer + IDI_MSA_ROW_MENU_0;

	for (int iRow = IDI_MSA_ROW_MENU_0; iRow < nRows; iRow++) {
		int iCol = *(buffer + pc++);
		drawStr(iRow, iCol, IDA_DEFAULT, buffer + pc);
		pc += strlen(buffer + pc) + 1;
	}

	// Show the string on screen
	_gfx->updateScreen();
}

void MickeyEngine::printLine(const char *buffer) {
	clearTextArea();

	drawStr(22, 18 - strlen(buffer) / 2, IDA_DEFAULT, buffer);

	// Show the string on screen
	_gfx->updateScreen();

	waitAnyKey(true);
}

void MickeyEngine::printExeStr(int ofs) {
	uint8 buffer[256] = {0};

	if (!ofs)
		return;

	readExe(ofs, buffer, sizeof(buffer));
	printStr((char *)buffer);
}

void MickeyEngine::printExeMsg(int ofs) {
	if (!ofs)
		return;

	printExeStr(ofs);
	waitAnyKey(true);
}

void MickeyEngine::printDatString(int iStr) {
	char buffer[256];
	int iDat = getDat(_gameStateMickey.iRoom);

	MSA_DAT_HEADER hdr;
	char szFile[256] = {0};

	Common::sprintf_s(szFile, IDS_MSA_PATH_DAT, IDS_MSA_NAME_DAT[iDat]);
	readDatHdr(szFile, &hdr);

	Common::File infile;

	if (!infile.open(szFile))
		return;

	infile.seek(hdr.ofsStr[iStr] + IDI_MSA_OFS_DAT, SEEK_SET);
	infile.read((uint8 *)buffer, 256);
	infile.close();

	printStr(buffer);
}

void MickeyEngine::printDesc(int iRoom) {
	MSA_DAT_HEADER hdr;
	char szFile[256] = {0};

	getDatFileName(iRoom, szFile);
	readDatHdr(szFile, &hdr);

	Common::File infile;

	if (!infile.open(szFile))
		return;

	char *buffer = (char *)malloc(256);
	memset(buffer, 0, 256);

	infile.seek(hdr.ofsDesc[iRoom - 1] + IDI_MSA_OFS_DAT, SEEK_SET);
	infile.read(buffer, 256);
	infile.close();

	printStr(buffer);
	free(buffer);
}

bool MickeyEngine::checkMenu() {
	MSA_MENU menu;
	int iSel0, iSel1;
	MSA_DAT_HEADER hdr;
	char szFile[256] = {0};
	Common::File infile;

	getDatFileName(_gameStateMickey.iRoom, szFile);
	readDatHdr(szFile, &hdr);
	if (!infile.open(szFile))
		return false;

	char *buffer = new char[sizeof(MSA_MENU)];
	infile.seek(hdr.ofsRoom[_gameStateMickey.iRoom - 1] + IDI_MSA_OFS_DAT, SEEK_SET);
	infile.read((uint8 *)buffer, sizeof(MSA_MENU));
	infile.close();

	memcpy(&menu, buffer, sizeof(MSA_MENU));
	patchMenu(&menu);
	memcpy(buffer, &menu, sizeof(MSA_MENU));

	getMenuSel(buffer, &iSel0, &iSel1);
	delete[] buffer;

	return parse(menu.cmd[iSel0].data[iSel1], menu.arg[iSel0].data[iSel1]);
}

void MickeyEngine::drawMenu(MSA_MENU &menu, int sel0, int sel1) {
	clearTextArea();

	for (int iRow = 0; iRow < 2; iRow++) {
		for (int iWord = 0; iWord < menu.row[iRow].count; iWord++) {
			int sel = (iRow == 0) ? sel0 : sel1;
			uint8 attr = (iWord == sel) ? IDA_DEFAULT_REV : IDA_DEFAULT;

			drawStr(IDI_MSA_ROW_MENU_0 + iRow, menu.row[iRow].entry[iWord].x0,
			        attr, (char *)menu.row[iRow].entry[iWord].szText);
		}
	}

	// Menu created, show it on screen
	_gfx->updateScreen();
}

void MickeyEngine::getMouseMenuSelRow(MSA_MENU &menu, int *sel0, int *sel1, int iRow, int x, int y) {
	int *sel = nullptr;

	switch (iRow) {
	case 0:
		if (y != IDI_MSA_ROW_MENU_0) return;
		sel = sel0;
		break;
	case 1:
		if (y != IDI_MSA_ROW_MENU_1) return;
		sel = sel1;
		break;
	default:
		return;
	}

	for (int iWord = 0; iWord < menu.row[iRow].count; iWord++) {
		if ((x >= menu.row[iRow].entry[iWord].x0) &&
		        (x < (int)(menu.row[iRow].entry[iWord].x0 +
		                   strlen((char *)menu.row[iRow].entry[iWord].szText)))) {
			*sel = iWord;
			break;
		}
	}
}

bool MickeyEngine::getMenuSelRow(MSA_MENU &menu, int *sel0, int *sel1, int iRow) {
	Common::Event event;
	int *sel = nullptr;
	int x, y;
	int goIndex = -1, northIndex = -1, southIndex = -1, eastIndex = -1, westIndex = -1;

	switch (iRow) {
	case 0:
		sel = sel0;
		break;
	case 1:
		sel = sel1;
		break;
	default:
		break;
	}
	int nWords = menu.row[iRow].count;
	_clickToMove = false;

	for (int i = 0; i <= menu.row[0].count; i++)
		if (menu.row[0].entry[i].szText[0] == 71 && menu.row[0].entry[i].szText[1] == 79)   // GO
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

	while (!shouldQuit()) {
		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RETURN_TO_LAUNCHER:
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
						//_gfx->setCursorPalette(true);
						// TODO:?????
					} else if (southIndex >= 0 && (event.mouse.x >= 20 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2) &&
					           (event.mouse.y >= IDI_MSA_PIC_HEIGHT - 10 && event.mouse.y <= IDI_MSA_PIC_HEIGHT)) {
						//_gfx->setCursorPalette(true);
					} else if (westIndex >= 0 && (event.mouse.y >= 0  && event.mouse.y <= IDI_MSA_PIC_HEIGHT) &&
					           (event.mouse.x >= 20 && event.mouse.x <= 30)) {
						//_gfx->setCursorPalette(true);
					} else if (eastIndex >= 0 && (event.mouse.y >= 0  && event.mouse.y <= IDI_MSA_PIC_HEIGHT) &&
					           (event.mouse.x >= IDI_MSA_PIC_WIDTH * 2 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2)) {
						//_gfx->setCursorPalette(true);
					} else {
						//_gfx->setCursorPalette(false);
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

					//_gfx->setCursorPalette(false);
					// TODO???
					_clickToMove = true;
				} else if (southIndex >= 0 && (event.mouse.x >= 20 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2) &&
				           (event.mouse.y >= IDI_MSA_PIC_HEIGHT - 10 && event.mouse.y <= IDI_MSA_PIC_HEIGHT)) {
					*sel0 = goIndex;
					*sel1 = southIndex;

					drawMenu(menu, *sel0, *sel1);

					//_gfx->setCursorPalette(false);
					// TODO???
					_clickToMove = true;
				} else if (westIndex >= 0 && (event.mouse.y >= 0  && event.mouse.y <= IDI_MSA_PIC_HEIGHT) &&
				           (event.mouse.x >= 20 && event.mouse.x <= 30)) {
					*sel0 = goIndex;
					*sel1 = westIndex;

					drawMenu(menu, *sel0, *sel1);

					//_gfx->setCursorPalette(false);
					// TODO???
					_clickToMove = true;
				} else if (eastIndex >= 0 && (event.mouse.y >= 0  && event.mouse.y <= IDI_MSA_PIC_HEIGHT) &&
				           (event.mouse.x >= IDI_MSA_PIC_WIDTH * 2 && event.mouse.x <= (IDI_MSA_PIC_WIDTH + 10) * 2)) {
					*sel0 = goIndex;
					*sel1 = eastIndex;

					drawMenu(menu, *sel0, *sel1);

					//_gfx->setCursorPalette(false);
					// TODO???
					_clickToMove = true;
				} else {
					//_gfx->setCursorPalette(false);
					// TODO???
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
				switch (event.kbd.keycode) {
				case Common::KEYCODE_2:
					// Hidden message
					if (_gameStateMickey.iRoom == IDI_MSA_PIC_MERCURY_CAVE_0) {
						for (int i = 0; i < 5; i++) {
							printExeMsg(IDO_MSA_HIDDEN_MSG[i]);
						}
						clearTextArea();
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
					flipFlag(VM_FLAG_SOUND_ON);
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

void MickeyEngine::getMenuSel(char *buffer, int *sel0, int *sel1) {
	MSA_MENU menu;

	memcpy(&menu, buffer, sizeof(MSA_MENU));

	*sel0 = 0;
	*sel1 = -1;

	// Show the mouse cursor for the menu
	CursorMan.showMouse(true);

	while (!shouldQuit()) {
		while (!shouldQuit()) {
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

void MickeyEngine::centerMenu(MSA_MENU *menu) {
	for (int iRow = 0; iRow < 2; iRow++) {
		int w = 0;
		for (int iWord = 0; iWord < menu->row[iRow].count; iWord++) {
			w += strlen((char *)menu->row[iRow].entry[iWord].szText);
		}
		w += menu->row[iRow].count - 1;
		int x = (40 - w) / 2;   // FIX

		for (int iWord = 0; iWord < menu->row[iRow].count; iWord++) {
			menu->row[iRow].entry[iWord].x0 = x;
			x += strlen((char *)menu->row[iRow].entry[iWord].szText) + 1;
		}
	}
}

void MickeyEngine::patchMenu(MSA_MENU *menu) {
	// change planet name in ship airlock menu
	if (_gameStateMickey.iRoom == IDI_MSA_PIC_SHIP_AIRLOCK) {
		Common::strcpy_s(menu->row[1].entry[2].szText, IDS_MSA_NAME_PLANET[_gameStateMickey.iPlanet]);
	}

	// exit if fix unnecessary
	if (!_gameStateMickey.iRmMenu[_gameStateMickey.iRoom]) {
		centerMenu(menu);
		return;
	}

	// copy menu to menubuf
	uint8 menubuf[sizeof(MSA_MENU)];
	memcpy(menubuf, menu, sizeof(menubuf));

	// read patches
	uint8 buffer[512];
	readOfsData(
	    IDOFS_MSA_MENU_PATCHES,
	    _gameStateMickey.nRmMenu[_gameStateMickey.iRoom] + _gameStateMickey.iRmMenu[_gameStateMickey.iRoom] - 1,
	    buffer, sizeof(buffer)
	);

	// get number of patches
	int pBuf = 0;
	int nPatches = buffer[pBuf++];

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

void MickeyEngine::printDatMessage(int iStr) {
	printDatString(iStr);
	waitAnyKey(true);
}

// Sound

bool MickeyEngine::playNote(MSA_SND_NOTE note, WaitOptions options) {
	int16 frequency = 0;
	if (note.counter != 0) {
		frequency = IDI_SND_OSCILLATOR_FREQUENCY / note.counter;
	}
	int32 lengthMs = (int32)(note.length / IDI_SND_TIMER_RESOLUTION);
	return playSpeakerNote(frequency, lengthMs, options);
}

bool MickeyEngine::playSound(ENUM_MSA_SOUND iSound, WaitOptions options) {
	bool completed = true;
	if (!getFlag(VM_FLAG_SOUND_ON))
		return completed;

	Common::Event event;
	MSA_SND_NOTE note;
	uint8 *buffer = new uint8[1024];
	int pBuf = 1;

	switch (iSound) {
	case IDI_MSA_SND_XL30:
		for (int iNote = 0; iNote < 6; iNote++) {
			note.counter = rnd(59600) + 59;
			note.length = 4;
			if (!playNote(note, options)) {
				completed = false;
				break;
			}
		}
		break;
	default:
		readOfsData(IDOFS_MSA_SOUND_DATA, iSound, buffer, 1024);

		for (;;) {
			memcpy(&note, buffer + pBuf, sizeof(note));
			if (!note.counter && !note.length)
				break;

			if (!playNote(note, options)) {
				completed = false;
				break;
			}

			pBuf += 3;
		}

		break;
	}

	delete[] buffer;
	return completed;
}

// Graphics

void MickeyEngine::drawObj(ENUM_MSA_OBJECT iObj, int x0, int y0) {
	char szFile[255] = {0};
	Common::sprintf_s(szFile, IDS_MSA_PATH_OBJ, IDS_MSA_NAME_OBJ[iObj]);

	Common::File file;
	if (!file.open(szFile))
		return;

	uint8 *buffer = new uint8[4096];
	uint32 size = file.size();
	file.read(buffer, size);
	file.close();

	int maxStep = 0; // default: draw all opcodes
	if (iObj == IDI_MSA_OBJECT_CRYSTAL) {
		// Handle crystal animation. Each "frame" is the picture
		// drawn with an additional opcode until it wraps around.
		// The crystal has 14 opcodes followed by the terminator.
		maxStep = _picture->getMaxStep() + 1;
		if (maxStep == 15) {
			maxStep = 1;
		}
	}

	_picture->setMaxStep(maxStep);
	_picture->setOffset(IDI_MSA_PIC_X0 + x0, IDI_MSA_PIC_Y0 + y0);
	_picture->decodePictureFromBuffer(buffer, size, false, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
	_picture->showPicture(IDI_MSA_PIC_X0, IDI_MSA_PIC_Y0, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
}

void MickeyEngine::drawPic(int iPic) {
	char szFile[255] = {0};
	Common::sprintf_s(szFile, IDS_MSA_PATH_PIC, iPic);

	Common::File file;
	if (!file.open(szFile))
		return;

	uint8 *buffer = new uint8[4096];
	uint32 size = file.size();
	file.read(buffer, size);
	file.close();

	// Note that decodePicture clears the screen
	_picture->setMaxStep(0);
	_picture->setOffset(IDI_MSA_PIC_X0, IDI_MSA_PIC_Y0);
	_picture->decodePictureFromBuffer(buffer, size, true, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
	_picture->showPicture(IDI_MSA_PIC_X0, IDI_MSA_PIC_Y0, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
}

void MickeyEngine::drawRoomAnimation() {
	switch (_gameStateMickey.iRoom) {
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
	case IDI_MSA_PIC_SHIP_URANUS: {
		// draw blinking ship lights
		uint8 lightPicture[] = {
			0xF0, 1,          // Set Color: 1
			0xF9, 2, 44, 45,  // Set Pattern: 2, plot at 44,45
			0xFF              // End
		};

		for (int i = 0; i < 12; i++) {
			uint8 iColor = _gameStateMickey.nFrame + i;
			if (iColor > 15)
				iColor -= 15;

			// FIXME: this is not the correct animation pattern.
			// the lights do not simply advance in a sequence from
			// left to right in the original, they do something else.
			lightPicture[1] = iColor; // change light color
			lightPicture[4] += 7;     // increase x coordinate

			_picture->setMaxStep(0);
			_picture->setOffset(IDI_MSA_PIC_X0, IDI_MSA_PIC_Y0);
			_picture->decodePictureFromBuffer(lightPicture, sizeof(lightPicture), false, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);
		}
		_picture->showPicture(IDI_MSA_PIC_X0, IDI_MSA_PIC_Y0, IDI_MSA_PIC_WIDTH, IDI_MSA_PIC_HEIGHT);

		_gameStateMickey.nFrame--;
		if (_gameStateMickey.nFrame < 0)
			_gameStateMickey.nFrame = 15;

		// play the spaceship beep but don't process events during playback.
		// this sound plays during menu usage, so events must not be consumed
		// while waiting or else inputs will be dropped. playing this sound
		// does create an input lag, but that is what happened in the original.
		playSound(IDI_MSA_SND_PRESS_BLUE, kWaitBlock);
	}
	break;

	case IDI_MSA_PIC_SHIP_CONTROLS:

		// draw XL30 screen
		if (_gameStateMickey.fAnimXL30) {
			if (_gameStateMickey.nFrame > 5)
				_gameStateMickey.nFrame = 0;

			drawObj((ENUM_MSA_OBJECT)(IDI_MSA_OBJECT_XL31 + _gameStateMickey.nFrame), 0, 4);
			_gameStateMickey.nFrame++;
		};

		break;

	default:

		// draw crystal
		if (_gameStateMickey.iRoom == IDI_MSA_XTAL_ROOM_XY[_gameStateMickey.iPlanet][0]) {
			if (isCrystalOnCurrentPlanet()) {
				switch (_gameStateMickey.iPlanet) {
				case IDI_MSA_PLANET_VENUS:
					if (_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] != 2)
						break;
					// fall through
				default:
					drawObj(
					    IDI_MSA_OBJECT_CRYSTAL,
					    IDI_MSA_XTAL_ROOM_XY[_gameStateMickey.iPlanet][1],
					    IDI_MSA_XTAL_ROOM_XY[_gameStateMickey.iPlanet][2]
					);
					break;
				}
			}
		}

		break;
	}
}

void MickeyEngine::drawRoom() {
	// Draw room picture
	if (_gameStateMickey.iRoom == IDI_MSA_PIC_TITLE) {
		drawPic(IDI_MSA_PIC_TITLE);
	} else {
		drawPic(_gameStateMickey.iRmPic[_gameStateMickey.iRoom]);

		if (_gameStateMickey.iRoom == IDI_MSA_PIC_SHIP_CONTROLS) {
			// Draw ship control room window
			if (_gameStateMickey.fFlying) {
				drawObj(IDI_MSA_OBJECT_W_SPACE, 0, 0);
			} else {
				drawObj((ENUM_MSA_OBJECT)(IDI_MSA_OBJECT_W_EARTH + _gameStateMickey.iPlanet), 0, 1);
			}
		}
	}

	// Draw room objects
	if (_gameStateMickey.iRoom < IDI_MSA_MAX_ROOM &&
		_gameStateMickey.iRmObj[_gameStateMickey.iRoom] != IDI_MSA_OBJECT_NONE) {
		uint8 buffer[512];
		readOfsData(IDO_MSA_ROOM_OBJECT_XY_OFFSETS,
		            _gameStateMickey.iRmObj[_gameStateMickey.iRoom], buffer, sizeof(buffer));

		int pBuf = 0;
		int nObjs = buffer[pBuf++];

		for (int iObj = 0; iObj < nObjs; iObj++) {
			drawObj((ENUM_MSA_OBJECT)buffer[pBuf], buffer[pBuf + 1], buffer[pBuf + 2]);
			pBuf += 3;
		}
	}

	// Draw room animation
	drawRoomAnimation();
}

// Straight mapping, CGA colors to CGA
static const byte BCGColorMappingCGAToCGA[4] = {
	0, 1, 2, 3
};

// Mapping table to map CGA colors to EGA
static const byte BCGColorMappingCGAToEGA[4] = {
	0, 11, 13, 15
};

bool MickeyEngine::drawLogo() {
	const int width = 80;
	const int height = 85 * 2;
	const byte *BCGColorMapping = BCGColorMappingCGAToEGA;

	// disable color mapping in case we are in CGA mode
	if (_renderMode == Common::kRenderCGA)
		BCGColorMapping = BCGColorMappingCGAToCGA;

	// read logos.bcg
	Common::File infile;
	if (!infile.open(IDS_MSA_PATH_LOGO)) {
		warning("%s: file not found", IDS_MSA_PATH_LOGO);
		return false;
	}

	uint32 fileBufferSize = infile.size();
	if (fileBufferSize < (width * height / 4)) {
		warning("%s: truncated file: %d", IDS_MSA_PATH_LOGO, fileBufferSize);
		return false;
	}
	byte *fileBuffer = new byte[fileBufferSize];
	infile.read(fileBuffer, fileBufferSize);

	// Show BCG picture
	// It's basically uncompressed CGA 4-color data (4 pixels per byte)
	byte *dataBuffer = fileBuffer;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			byte curByte = *dataBuffer++;

			byte color1 = BCGColorMapping[(curByte >> 6) & 0x03];
			byte color2 = BCGColorMapping[(curByte >> 4) & 0x03];
			byte color3 = BCGColorMapping[(curByte >> 2) & 0x03];
			byte color4 = BCGColorMapping[(curByte >> 0) & 0x03];

			_gfx->putPixelOnDisplay(x * 4 + 0, y, color1);
			_gfx->putPixelOnDisplay(x * 4 + 1, y, color2);
			_gfx->putPixelOnDisplay(x * 4 + 2, y, color3);
			_gfx->putPixelOnDisplay(x * 4 + 3, y, color4);
		}
	}

	_gfx->copyDisplayToScreen();

	delete[] fileBuffer;
	return true;
}

void MickeyEngine::animate() {
	_system->delayMillis(IDI_MSA_ANIM_DELAY);
	drawRoomAnimation();
}

void MickeyEngine::printRoomDesc() {
	// print room description
	printDesc(_gameStateMickey.iRoom);
	waitAnyKey(true);

	// print extended room description
	if (_gameStateMickey.oRmTxt[_gameStateMickey.iRoom]) {
		printExeMsg(_gameStateMickey.oRmTxt[_gameStateMickey.iRoom] + IDI_MSA_OFS_EXE);
	}
}

bool MickeyEngine::loadGame() {
	Common::InSaveFile *infile;
	char szFile[256] = {0};
	bool diskerror = true;
	int i = 0;

	while (diskerror) {
		int sel = choose1to9(IDO_MSA_LOAD_GAME[1]);
		if (!sel)
			return false;

		// load game
		Common::sprintf_s(szFile, "%s.s%02d", getTargetName().c_str(), sel);
		if (!(infile = getSaveFileMan()->openForLoading(szFile))) {
			printLine("PLEASE CHECK THE DISK DRIVE");

			if (getSelection(kSelAnyKey) == 0)
				return false;
		} else {
			if (infile->readUint32BE() != MKTAG('M', 'I', 'C', 'K')) {
				warning("MickeyEngine::loadGame wrong save game format");
				delete infile;
				return false;
			}

			byte saveVersion = infile->readByte();
			if (saveVersion != MSA_SAVEGAME_VERSION) { // currently only one valid version
				warning("MickeyEngine::loadGame unknown save version: %d", saveVersion);
				delete infile;
				return false;
			}

			_gameStateMickey.iRoom = infile->readByte();
			_gameStateMickey.iPlanet = infile->readByte();
			_gameStateMickey.iDisk = infile->readByte();

			_gameStateMickey.nAir = infile->readByte();
			_gameStateMickey.nButtons = infile->readByte();
			_gameStateMickey.nRocks = infile->readByte();

			_gameStateMickey.nXtals = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_DAT; i++)
				_gameStateMickey.iPlanetXtal[i] = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_PLANET; i++)
				_gameStateMickey.iClue[i] = infile->readUint16LE();

			infile->read(_gameStateMickey.szAddr, IDI_MSA_MAX_BUTTON + 1);

			_gameStateMickey.fHasXtal = infile->readByte() == 1;
			_gameStateMickey.fIntro = infile->readByte() == 1;
			_gameStateMickey.fSuit = infile->readByte() == 1;
			_gameStateMickey.fShipDoorOpen = infile->readByte() == 1;
			_gameStateMickey.fFlying = infile->readByte() == 1;
			_gameStateMickey.fStoryShown = infile->readByte() == 1;
			_gameStateMickey.fPlanetsInitialized = infile->readByte() == 1;
			_gameStateMickey.fTempleDoorOpen = infile->readByte() == 1;
			_gameStateMickey.fAnimXL30 = infile->readByte() == 1;

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				_gameStateMickey.fItem[i] = infile->readByte() == 1;

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				_gameStateMickey.fItemUsed[i] = infile->readByte() == 1;

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				_gameStateMickey.iItem[i] = infile->readSByte();

			_gameStateMickey.nItems = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_gameStateMickey.iRmObj[i] = infile->readSByte();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_gameStateMickey.iRmPic[i] = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_gameStateMickey.oRmTxt[i] = infile->readUint16LE();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_gameStateMickey.iRmMenu[i] = infile->readByte();

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				_gameStateMickey.nRmMenu[i] = infile->readByte();

			_gameStateMickey.nFrame = infile->readSByte();

			diskerror = false;
			delete infile;
		}
	}

	printExeMsg(IDO_MSA_LOAD_GAME[2]);
	return true;
}

void MickeyEngine::saveGame() {
	Common::OutSaveFile *outfile;
	char szFile[256] = {0};
	bool diskerror = true;
	int i = 0;

	bool fOldDisk = chooseY_N(IDO_MSA_SAVE_GAME[0], false);

	if (fOldDisk)
		printExeStr(IDO_MSA_SAVE_GAME[1]);
	else
		printExeStr(IDO_MSA_SAVE_GAME[2]);

	if (getSelection(kSelAnyKey) == 0)
		return;

	while (diskerror) {
		int sel = choose1to9(IDO_MSA_SAVE_GAME[3]);
		if (!sel)
			return;

		if (fOldDisk)
			printExeStr(IDO_MSA_SAVE_GAME[5]);
		else
			printExeStr(IDO_MSA_SAVE_GAME[4]);

		if (getSelection(kSelAnyKey) == 0)
			return;

		// save game
		Common::sprintf_s(szFile, "%s.s%02d", getTargetName().c_str(), sel);
		if (!(outfile = getSaveFileMan()->openForSaving(szFile))) {
			printLine("PLEASE CHECK THE DISK DRIVE");

			if (getSelection(kSelAnyKey) == 0)
				return;
		} else {
			outfile->writeUint32BE(MKTAG('M', 'I', 'C', 'K')); // header
			outfile->writeByte(MSA_SAVEGAME_VERSION);

			outfile->writeByte(_gameStateMickey.iRoom);
			outfile->writeByte(_gameStateMickey.iPlanet);
			outfile->writeByte(_gameStateMickey.iDisk);

			outfile->writeByte(_gameStateMickey.nAir);
			outfile->writeByte(_gameStateMickey.nButtons);
			outfile->writeByte(_gameStateMickey.nRocks);

			outfile->writeByte(_gameStateMickey.nXtals);

			for (i = 0; i < IDI_MSA_MAX_DAT; i++)
				outfile->writeByte(_gameStateMickey.iPlanetXtal[i]);

			for (i = 0; i < IDI_MSA_MAX_PLANET; i++)
				outfile->writeUint16LE(_gameStateMickey.iClue[i]);

			outfile->write(_gameStateMickey.szAddr, IDI_MSA_MAX_BUTTON + 1);

			outfile->writeByte(_gameStateMickey.fHasXtal ? 1 : 0);
			outfile->writeByte(_gameStateMickey.fIntro ? 1 : 0);
			outfile->writeByte(_gameStateMickey.fSuit ? 1 : 0);
			outfile->writeByte(_gameStateMickey.fShipDoorOpen ? 1 : 0);
			outfile->writeByte(_gameStateMickey.fFlying ? 1 : 0);
			outfile->writeByte(_gameStateMickey.fStoryShown ? 1 : 0);
			outfile->writeByte(_gameStateMickey.fPlanetsInitialized ? 1 : 0);
			outfile->writeByte(_gameStateMickey.fTempleDoorOpen ? 1 : 0);
			outfile->writeByte(_gameStateMickey.fAnimXL30 ? 1 : 0);

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				outfile->writeByte(_gameStateMickey.fItem[i] ? 1 : 0);

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				outfile->writeByte(_gameStateMickey.fItemUsed[i] ? 1 : 0);

			for (i = 0; i < IDI_MSA_MAX_ITEM; i++)
				outfile->writeSByte(_gameStateMickey.iItem[i]);

			outfile->writeByte(_gameStateMickey.nItems);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeSByte(_gameStateMickey.iRmObj[i]);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeByte(_gameStateMickey.iRmPic[i]);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeUint16LE(_gameStateMickey.oRmTxt[i]);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeByte(_gameStateMickey.iRmMenu[i]);

			for (i = 0; i < IDI_MSA_MAX_ROOM; i++)
				outfile->writeByte(_gameStateMickey.nRmMenu[i]);

			outfile->writeSByte(_gameStateMickey.nFrame);

			outfile->finalize();

			if (outfile->err())
				warning("Can't write file '%s'. (Disk full?)", szFile);

			diskerror = false;
			delete outfile;
		}
	}

	printExeMsg(IDO_MSA_SAVE_GAME[6]);
}

void MickeyEngine::showPlanetInfo() {
	for (int i = 0; i < 4; i++) {
		printExeMsg(IDO_MSA_PLANET_INFO[_gameStateMickey.iPlanet][i]);
	}
}

void MickeyEngine::printStory() {
	char buffer[IDI_MSA_LEN_STORY] = {0};
	char szLine[41] = {0};
	int pBuf = 0;

	readExe(IDO_MSA_GAME_STORY, (uint8 *)buffer, sizeof(buffer));

	clearScreen(IDA_DEFAULT);
	for (int iRow = 0; iRow < 25; iRow++) {
		Common::strlcpy(szLine, buffer + pBuf, 41);
		drawStr(iRow, 0, IDA_DEFAULT, szLine);
		pBuf += strlen(szLine) + 1;
	}
	waitAnyKey();

	clearScreen(IDA_DEFAULT);
	for (int iRow = 0; iRow < 21; iRow++) {
		Common::strlcpy(szLine, buffer + pBuf, 41);
		drawStr(iRow, 0, IDA_DEFAULT, szLine);
		pBuf += strlen(szLine) + 1;
	}
	waitAnyKey();

	//Set back to black
	_gfx->clearDisplay(0);
	_gfx->updateScreen();

	drawRoom();

	_gameStateMickey.fStoryShown = true;
}

int MickeyEngine::getPlanet() {
	if (!_gameStateMickey.nButtons)
		return -1;

	for (int iPlanet = 0; iPlanet < IDI_MSA_MAX_DAT - 1; iPlanet++) {
		if (!strcmp(IDS_MSA_ADDR_PLANET[iPlanet], _gameStateMickey.szAddr)) {
			return iPlanet;
		}
	}

	return -1;
}

void MickeyEngine::pressOB(int iButton) {
	char szButtons[12] = {0};

	// check if too many buttons pressed
	if (_gameStateMickey.nButtons == IDI_MSA_MAX_BUTTON) {
		_gameStateMickey.nButtons = 0;
		memset(_gameStateMickey.szAddr, 0, sizeof(_gameStateMickey.szAddr));
		printExeMsg(IDO_MSA_TOO_MANY_BUTTONS_PRESSED);
		return;
	}

	// add button press to address
	_gameStateMickey.nButtons++;
	_gameStateMickey.szAddr[_gameStateMickey.nButtons - 1] = (char)iButton;

	// format buttons string
	for (int i = 0; i < IDI_MSA_MAX_BUTTON; i++) {
		szButtons[i * 2] = _gameStateMickey.szAddr[i];
		if (_gameStateMickey.szAddr[i + 1]) szButtons[(i * 2) + 1] = ',';
	}

	// print pressed buttons
	printExeStr(IDO_MSA_MICKEY_HAS_PRESSED);
	drawStr(20, 22, IDA_DEFAULT, szButtons);
	waitAnyKey();
}

void MickeyEngine::insertDisk(int iDisk) {
	clearTextArea();
	drawStr(IDI_MSA_ROW_INSERT_DISK, IDI_MSA_COL_INSERT_DISK, IDA_DEFAULT, (const char *)IDS_MSA_INSERT_DISK[iDisk]);
	waitAnyKey();
}

void MickeyEngine::gameOver() {
	drawPic(IDI_MSA_PIC_EARTH_SHIP_LEAVING);
	printExeMsg(IDO_MSA_GAME_OVER[3]);
	playSound(IDI_MSA_SND_GAME_OVER);

	if (_gameStateMickey.fItemUsed[IDI_MSA_ITEM_LETTER]) {
		drawPic(IDI_MSA_PIC_EARTH_MINNIE);
		printExeMsg(IDO_MSA_GAME_OVER[4]);
		printExeMsg(IDO_MSA_GAME_OVER[5]);
	} else {
		printExeMsg(IDO_MSA_GAME_OVER[6]);
		printExeMsg(IDO_MSA_GAME_OVER[7]);
	}

	_isGameOver = true;
}

void MickeyEngine::flipSwitch() {
	if (_gameStateMickey.nXtals) {
		if (!_gameStateMickey.fStoryShown)
			printStory();

		// Initialize planet data
		if (!_gameStateMickey.fPlanetsInitialized) {
			int iHint = 0;
			int iPlanet = 0;

			memset(_gameStateMickey.iPlanetXtal, 0, sizeof(_gameStateMickey.iPlanetXtal));
			memset(_gameStateMickey.iClue, 0, sizeof(_gameStateMickey.iClue));

			_gameStateMickey.iPlanetXtal[0] = IDI_MSA_PLANET_EARTH;
			_gameStateMickey.iPlanetXtal[8] = IDI_MSA_PLANET_URANUS;

			for (int i = 1; i < IDI_MSA_MAX_PLANET; i++) {
				if (i < 8) {
					do {
						// Earth (planet 0) and Uranus (planet 8) are excluded
						iPlanet = rnd(IDI_MSA_MAX_PLANET - 2);
					} while (planetIsAlreadyAssigned(iPlanet));
				} else {
					iPlanet = IDI_MSA_PLANET_URANUS;    // Uranus is always last
				}

				_gameStateMickey.iPlanetXtal[i] = iPlanet;
				iHint = rnd(5) - 1; // clues are 0-4
				_gameStateMickey.iClue[i] = IDO_MSA_NEXT_PIECE[iPlanet][iHint];
			}

			_gameStateMickey.fPlanetsInitialized = true;
		}

		// activate screen animation
		_gameStateMickey.fAnimXL30 = true;

		clearTextArea();
		playSound(IDI_MSA_SND_XL30);
		printExeMsg(IDO_MSA_XL30_SPEAKING);

		if (_gameStateMickey.fHasXtal) {
			_gameStateMickey.fHasXtal = false;
			printExeMsg(IDO_MSA_CRYSTAL_PIECE_FOUND);
		}

		if (_gameStateMickey.nXtals == IDI_MSA_MAX_PLANET) {
			printExeMsg(IDO_MSA_GAME_OVER[0]);
			printExeMsg(IDO_MSA_GAME_OVER[1]);
			printExeMsg(IDO_MSA_GAME_OVER[2]);

#if 0
			// DEBUG
			strcpy(_gameStateMickey.szAddr, (char *)IDS_MSA_ADDR_PLANET[IDI_MSA_PLANET_EARTH]);
			_gameStateMickey.nButtons = strlen(_gameStateMickey.szAddr);
#endif

		} else {
			printExeStr(_gameStateMickey.iClue[_gameStateMickey.nXtals]);

#if 0
			// DEBUG
			drawStr(24, 12, IDA_DEFAULT, (char *)IDS_MSA_NAME_PLANET_2[_gameStateMickey.iPlanetXtal[_gameStateMickey.nXtals]]);
			drawStr(24, 22, IDA_DEFAULT, (char *)IDS_MSA_ADDR_PLANET[_gameStateMickey.iPlanetXtal[_gameStateMickey.nXtals]]);
			strcpy(_gameStateMickey.szAddr, (char *)IDS_MSA_ADDR_PLANET[_gameStateMickey.iPlanetXtal[_gameStateMickey.nXtals]]);
			_gameStateMickey.nButtons = strlen(_gameStateMickey.szAddr);
			_gfx->doUpdate();
#endif

			waitAnyKey(true);
		}
	} else {
		printStory();
	}
}

void MickeyEngine::inventory() {
	int iRow = IDI_MSA_ROW_INV_ITEMS;
	char szCrystals[12] = {0};

	Common::sprintf_s(szCrystals, IDS_MSA_CRYSTALS, IDS_MSA_CRYSTAL_NO[_gameStateMickey.nXtals]);

	CursorMan.showMouse(false);

	clearScreen(IDA_DEFAULT);
	drawStr(IDI_MSA_ROW_INV_TITLE, IDI_MSA_COL_INV_TITLE, IDA_DEFAULT, IDS_MSA_INVENTORY);
	drawStr(IDI_MSA_ROW_INV_CRYSTALS, IDI_MSA_COL_INV_ITEMS, IDA_DEFAULT, szCrystals);

	for (int iItem = 0; iItem < IDI_MSA_MAX_ITEM; iItem++) {
		if ((_gameStateMickey.iItem[iItem] != IDI_MSA_OBJECT_NONE) && _gameStateMickey.fItem[_gameStateMickey.iItem[iItem]]) {
			drawStr(iRow++, IDI_MSA_COL_INV_ITEMS, IDA_DEFAULT, (const char *)IDS_MSA_NAME_ITEM[_gameStateMickey.iItem[iItem]]);
		}
	}

	waitAnyKey();

	clearScreen(IDA_DEFAULT);

	CursorMan.showMouse(true);
}

void MickeyEngine::intro() {
	// Draw Sierra logo
	if (drawLogo()) {          // Original does not show the logo, we do if available
		waitAnyKey();          // Not in the original, but needed so that the logo is visible
		_gfx->clearDisplay(0); // Logo is larger than picture area, clear entire screen
	}

	// draw title picture
	_gameStateMickey.iRoom = IDI_MSA_PIC_TITLE;
	drawRoom();

	// show copyright
	printExeMsg(IDO_MSA_COPYRIGHT);

	// Quit if necessary
	if (shouldQuit())
		return;

	// play theme
	playSound(IDI_MSA_SND_THEME, kWaitAllowInterrupt);

	// load game
	_gameStateMickey.fIntro = true;
	if (chooseY_N(IDO_MSA_LOAD_GAME[0], true)) {
		if (loadGame()) {
			_gameStateMickey.fIntro = false;
			return;
		}
	}

	// Quit if necessary
	if (shouldQuit())
		return;

	// play spaceship landing scene
	_gameStateMickey.iPlanet = IDI_MSA_PLANET_EARTH;
	_gameStateMickey.iRoom = IDI_MSA_PIC_EARTH_ROAD_4;

	drawRoom();
	printRoomDesc();

	// Quit if necessary
	if (shouldQuit())
		return;

	playSound(IDI_MSA_SND_SHIP_LAND);

	// Flash screen 3 times
	for (byte i = 0; i < 3; i++) {
		playSound(IDI_MSA_SND_PRESS_BLUE);

		//Set screen to white
		_gfx->clearDisplay(getWhite());
		_gfx->updateScreen();

		_system->delayMillis(IDI_MSA_ANIM_DELAY);

		//Set back to black
		_gfx->clearDisplay(0);
		_gfx->updateScreen();

		drawRoom();
		printDesc(_gameStateMickey.iRoom);
	}

	printExeMsg(IDO_MSA_INTRO);
}

void MickeyEngine::getItem(ENUM_MSA_ITEM iItem) {
	_gameStateMickey.fItem[iItem] = true;
	_gameStateMickey.iItem[_gameStateMickey.nItems++] = iItem;
	_gameStateMickey.oRmTxt[_gameStateMickey.iRoom] = 0;
	playSound(IDI_MSA_SND_TAKE);
	drawRoom();
}

void MickeyEngine::getXtal(int iStr) {
	_gameStateMickey.oRmTxt[_gameStateMickey.iRoom] = 0;
	_gameStateMickey.fHasXtal = true;
	_gameStateMickey.nXtals++;
	playSound(IDI_MSA_SND_CRYSTAL);
	drawRoom();
	printDatMessage(iStr);
}

bool MickeyEngine::parse(int cmd, int arg) {
	switch (cmd) {

	// BASIC

	case IDI_MSA_ACTION_GOTO_ROOM:
		_gameStateMickey.iRoom = arg;
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
		if (_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] == 2) {
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = IDI_MSA_OBJECT_NONE;
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 3;
			getItem(IDI_MSA_ITEM_ROPE);
			printLine("MICKEY TAKES THE ROPE");
		} else {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			printDatMessage(11);
		}
		break;
	case IDI_MSA_ACTION_UNTIE_ROPE:
		_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_EARTH_TIRE_SWING_1;
		_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = 0;
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 2;
		drawRoom();
		printDatMessage(12);
		break;
	case IDI_MSA_ACTION_GET_BONE:
		_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = IDI_MSA_OBJECT_NONE;
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
		getItem(IDI_MSA_ITEM_BONE);
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_GET_XTAL_EARTH:
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
		getXtal(arg);
		break;
	case IDI_MSA_ACTION_LOOK_DESK:
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
		_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = 2;
		drawRoom();
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_WRITE_LETTER:
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 3;
		_gameStateMickey.iRmMenu[IDI_MSA_PIC_EARTH_MAILBOX] = 1;
		_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = IDI_MSA_OBJECT_NONE;
		getItem(IDI_MSA_ITEM_LETTER);
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_MAIL_LETTER:
		_gameStateMickey.fItemUsed[IDI_MSA_ITEM_LETTER] = true;
		_gameStateMickey.fItem[IDI_MSA_ITEM_LETTER] = false;
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 0;
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_OPEN_MAILBOX:
		if (_gameStateMickey.fItemUsed[IDI_MSA_ITEM_LETTER]) {
			printDatMessage(110);
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_CUPBOARD:
		if (_gameStateMickey.iRmMenu[_gameStateMickey.iRoom]) {
			if (_gameStateMickey.iRmObj[_gameStateMickey.iRoom] == IDI_MSA_OBJECT_NONE) {
				printDatMessage(78);
			} else {
				printDatMessage(arg);
			}
		} else {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_EARTH_KITCHEN_1;
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = 3;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_FLASHLIGHT:
		if (!mickeyHasItem(IDI_MSA_ITEM_FLASHLIGHT)) {
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = IDI_MSA_OBJECT_NONE;
			getItem(IDI_MSA_ITEM_FLASHLIGHT);
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_CABINET:
		if (_gameStateMickey.iRmMenu[_gameStateMickey.iRoom]) {
			printDatMessage(109);
		} else {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_EARTH_GARAGE_1;
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = 15;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_CROWBAR:
		if (!mickeyHasItem(IDI_MSA_ITEM_CROWBAR)) {
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom]--;
			getItem(IDI_MSA_ITEM_CROWBAR);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_WRENCH:
		if (!mickeyHasItem(IDI_MSA_ITEM_WRENCH)) {
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom] -= 2;
			getItem(IDI_MSA_ITEM_WRENCH);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_CLOSET:
		if (_gameStateMickey.iRmMenu[_gameStateMickey.iRoom]) {
			printDatMessage(99);
		} else {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_EARTH_BEDROOM_1;
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = 7;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_MATTRESS:
		if (!mickeyHasItem(IDI_MSA_ITEM_MATTRESS)) {
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom]--;
			getItem(IDI_MSA_ITEM_MATTRESS);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_SCARF:
		if (!mickeyHasItem(IDI_MSA_ITEM_SCARF)) {
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom] -= 2;
			getItem(IDI_MSA_ITEM_SCARF);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_SUNGLASSES:
		if (!mickeyHasItem(IDI_MSA_ITEM_SUNGLASSES)) {
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom]--;
			getItem(IDI_MSA_ITEM_SUNGLASSES);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_SCALE:
		if (!mickeyHasItem(IDI_MSA_ITEM_SCALE)) {
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_VENUS_WEIGH] = 1;
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_NEPTUNE_WEIGH] = 1;
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_MERCURY_WEIGH] = 1;
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_SATURN_WEIGH] = 1;
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_PLUTO_WEIGH] = 1;
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_JUPITER_WEIGH] = 1;
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_MARS_WEIGH] = 1;
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_URANUS_WEIGH] = 1;
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom] -= 2;
			getItem(IDI_MSA_ITEM_SCALE);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GOTO_SPACESHIP:
		_gameStateMickey.iRoom = IDI_MSA_PIC_SHIP_AIRLOCK;
		if (_gameStateMickey.iPlanet != IDI_MSA_PLANET_EARTH)
			insertDisk(0);
		return true;

	// VENUS

	case IDI_MSA_ACTION_DOWN_CHASM:
		if (_gameStateMickey.fItem[IDI_MSA_ITEM_ROPE]) {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
		}
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_DOWN_ROPE:
		if (_gameStateMickey.fItemUsed[IDI_MSA_ITEM_ROPE]) {
			_gameStateMickey.iRoom = IDI_MSA_PIC_VENUS_PROBE;
			return true;
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_ROPE:
		if (_gameStateMickey.fItemUsed[IDI_MSA_ITEM_ROPE]) {
			printDatMessage(22);
		} else {
			_gameStateMickey.fItemUsed[IDI_MSA_ITEM_ROPE] = true;
			_gameStateMickey.fItem[IDI_MSA_ITEM_ROPE] = false;
			_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_VENUS_CHASM_1;
			drawRoom();
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_HATCH:
		if (_gameStateMickey.fItemUsed[IDI_MSA_ITEM_WRENCH]) {
			if ((_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] == 3) || (_gameStateMickey.iRmPic[_gameStateMickey.iRoom] == IDI_MSA_PIC_VENUS_PROBE_1))
				printDatMessage(39);
			else {
				_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 2;
				_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_VENUS_PROBE_1;
				drawRoom();
				printDatMessage(24);
			}
		} else {
			if (_gameStateMickey.fItem[IDI_MSA_ITEM_WRENCH]) {
				_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			}

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_WRENCH:
		_gameStateMickey.fItemUsed[IDI_MSA_ITEM_WRENCH] = true;
		printDatString(arg);

		if (_gameStateMickey.iRmPic[_gameStateMickey.iRoom] == IDI_MSA_PIC_VENUS_PROBE_1) {
			clearRow(22);
		}

		waitAnyKey();
		break;
	case IDI_MSA_ACTION_GET_XTAL_VENUS:
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 3;
		getXtal(arg);
		break;

	// TRITON (NEPTUNE)

	case IDI_MSA_ACTION_LOOK_CASTLE:
		if (!_gameStateMickey.iRmMenu[_gameStateMickey.iRoom]) {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
		}
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_ENTER_OPENING:
		if (_gameStateMickey.fItemUsed[IDI_MSA_ITEM_CROWBAR]) {
			_gameStateMickey.iRoom = IDI_MSA_PIC_NEPTUNE_CASTLE_4;

			return true;
		} else {
			if (_gameStateMickey.fItem[IDI_MSA_ITEM_CROWBAR]) {
				_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 2;
			}

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_USE_CROWBAR:
		_gameStateMickey.fItemUsed[IDI_MSA_ITEM_CROWBAR] = true;
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
		_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_NEPTUNE_ENTRANCE_1;
		drawRoom();
		printDatMessage(arg);
		break;
	case IDI_MSA_ACTION_GET_XTAL_NEPTUNE:
		if (_gameStateMickey.fHasXtal) {
			printDatMessage(71);
		} else {
			if (_gameStateMickey.fItem[IDI_MSA_ITEM_SCARF]) {
				_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			}

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_TALK_LEADER:
		_gameStateMickey.iRoom = IDI_MSA_PIC_NEPTUNE_ENTRYWAY;

		printDatMessage(arg);
		return true;
	case IDI_MSA_ACTION_GIVE_SCARF:
		_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = 18;
		getXtal(arg);
		_gameStateMickey.fItem[IDI_MSA_ITEM_SCARF] = false;
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 0;
		_gameStateMickey.iRmMenu[IDI_MSA_PIC_EARTH_BEDROOM] = 2;
		_gameStateMickey.iRoom = IDI_MSA_PIC_NEPTUNE_ENTRYWAY;

		return true;

	// MERCURY

	case IDI_MSA_ACTION_GET_XTAL_MERCURY:
		if (isCrystalOnCurrentPlanet()) {
			if (_gameStateMickey.fItem[IDI_MSA_ITEM_SUNGLASSES]) {
				_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			}
			printDatMessage(arg);
		} else {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 2;
			printDatMessage(32);
		}
		break;
	case IDI_MSA_ACTION_GIVE_SUNGLASSES:
		_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = 17;
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 2;
		_gameStateMickey.fItem[IDI_MSA_ITEM_SUNGLASSES] = false;

		getXtal(arg);

		break;

	// TITAN (SATURN)

	case IDI_MSA_ACTION_CROSS_LAKE:
		if (_gameStateMickey.fItem[IDI_MSA_ITEM_MATTRESS]) {
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_SATURN_LAKE_0] = 1;
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_SATURN_LAKE_1] = 1;
			_gameStateMickey.iRmMenu[IDI_MSA_PIC_SATURN_LAKE_2] = 1;
		}

		printDatMessage(arg);

		break;
	case IDI_MSA_ACTION_USE_MATTRESS:
		printDatMessage(arg);
		
		// must set room after printDatMessage, or else the crystal from
		// the next room will appear and animate while still displaying
		// the picture for the current room
		_gameStateMickey.iRoom = IDI_MSA_PIC_SATURN_ISLAND;

		return true;
	case IDI_MSA_ACTION_GET_XTAL_SATURN:
		if (isCrystalOnCurrentPlanet()) {
			getXtal(arg);
		} else {
			printDatMessage(29);
		}
		break;
	case IDI_MSA_ACTION_LEAVE_ISLAND:
		_gameStateMickey.iRoom = IDI_MSA_PIC_SATURN_LAKE_1;

		printDatMessage(arg);

		return true;

	// PLUTO

	case IDI_MSA_ACTION_GET_XTAL_PLUTO:
		if (isCrystalOnCurrentPlanet()) {
			if (_gameStateMickey.fItem[IDI_MSA_ITEM_BONE]) {
				_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			}
			printDatMessage(arg);
		} else {
			printDatMessage(19);
		}
		break;
	case IDI_MSA_ACTION_GIVE_BONE:
		_gameStateMickey.fItem[IDI_MSA_ITEM_BONE] = false;
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 0;
		_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = 16;

		getXtal(arg);

		break;

	// IO (JUPITER)

	case IDI_MSA_ACTION_GET_ROCK_0:
	case IDI_MSA_ACTION_GET_ROCK_1:
		if (_gameStateMickey.fItem[IDI_MSA_ITEM_ROCK]) {
			printDatMessage(38);
		} else {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			_gameStateMickey.iRmObj[_gameStateMickey.iRoom] = IDI_MSA_OBJECT_NONE;
			getItem(IDI_MSA_ITEM_ROCK);
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_XTAL_JUPITER:
		if (isCrystalOnCurrentPlanet()) {
			switch (_gameStateMickey.nRocks) {
			case 0:
				if (_gameStateMickey.fItem[IDI_MSA_ITEM_ROCK]) {
					_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
				}
				printDatMessage(arg);
				break;
			case 1:
				if (_gameStateMickey.fItem[IDI_MSA_ITEM_ROCK]) {
					_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
				}
				printDatMessage(34);
				break;
			case 2:
				getXtal(35);
				break;
			default:
				break;
			}
		} else {
			printDatMessage(15);
		}
		break;
	case IDI_MSA_ACTION_THROW_ROCK:
		_gameStateMickey.fItem[IDI_MSA_ITEM_ROCK] = false;
		_gameStateMickey.nItems--;
		_gameStateMickey.iRmObj[_gameStateMickey.iRoom]++;
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 0;

		drawRoom();

		if (_gameStateMickey.nRocks) {
			printDatMessage(37);
		} else {
			printDatMessage(arg);
		}

		_gameStateMickey.nRocks++;
		break;

	// MARS

	case IDI_MSA_ACTION_GO_TUBE:
		if (_gameStateMickey.fItem[IDI_MSA_ITEM_FLASHLIGHT]) {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
		}

		printDatMessage(arg);

		break;
	case IDI_MSA_ACTION_USE_FLASHLIGHT:
		_gameStateMickey.iRoom = IDI_MSA_PIC_MARS_TUBE_1;

		printDatMessage(15);

		return true;
	case IDI_MSA_ACTION_PLUTO_DIG:
		if (isCrystalOnCurrentPlanet()) {
			getXtal(arg);
		} else {
			printDatMessage(21);
		}
		break;
	case IDI_MSA_ACTION_GET_XTAL_MARS:
		if (isCrystalOnCurrentPlanet()) {
			printDatMessage(arg);
		} else {
			printDatMessage(23);
		}
		break;

	// OBERON (URANUS)

	case IDI_MSA_ACTION_ENTER_TEMPLE:
		_gameStateMickey.iRoom = IDI_MSA_PIC_URANUS_TEMPLE;

		return true;
	case IDI_MSA_ACTION_USE_CRYSTAL:
		if (_gameStateMickey.iRmMenu[_gameStateMickey.iRoom]) {
			printDatMessage(25);
		} else {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_URANUS_TEMPLE_1;

			drawRoom();

			_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_URANUS_TEMPLE;

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_OPEN_DOOR:
		if (_gameStateMickey.fTempleDoorOpen) {
			printDatMessage(36);
		} else {
			_gameStateMickey.fTempleDoorOpen = 1;
			_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_URANUS_TEMPLE_2;

			drawRoom();

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_ENTER_DOOR:
		if (_gameStateMickey.fTempleDoorOpen) {
			_gameStateMickey.iRoom = IDI_MSA_PIC_URANUS_STEPS;

			return true;
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GET_XTAL_URANUS:
		if (isCrystalOnCurrentPlanet()) {
			if (_gameStateMickey.fItem[IDI_MSA_ITEM_CROWBAR]) {
				_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			}
			printDatMessage(arg);
		} else {
			printDatMessage(34);
		}
		break;
	case IDI_MSA_ACTION_USE_CROWBAR_1:
		_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 0;

		getXtal(arg);

		break;

	// SPACESHIP

	case IDI_MSA_ACTION_GO_NORTH:
		if (_gameStateMickey.fShipDoorOpen) {
			if (_gameStateMickey.fSuit) {
				printDatMessage(45);
			} else {
				_gameStateMickey.iRoom = IDI_MSA_PIC_SHIP_CORRIDOR;
				return true;
			}
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_GO_PLANET:
		if (!_gameStateMickey.fShipDoorOpen) {
			if ((_gameStateMickey.nXtals == IDI_MSA_MAX_PLANET) && (_gameStateMickey.iPlanet == IDI_MSA_PLANET_EARTH)) {
				gameOver();
				return true;
			}

			if ((_gameStateMickey.iPlanet == _gameStateMickey.iPlanetXtal[_gameStateMickey.nXtals]) || (_gameStateMickey.iPlanet == IDI_MSA_PLANET_EARTH)) {
				_gameStateMickey.iRoom = IDI_MSA_HOME_PLANET[_gameStateMickey.iPlanet];

				if (_gameStateMickey.iPlanet != IDI_MSA_PLANET_EARTH)
					insertDisk(1);

				return true;
			} else {
				_gameStateMickey.iRoom = IDI_MSA_SHIP_PLANET[_gameStateMickey.iPlanet];

				return true;
			}
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_PRESS_BUTTON:
		if (_gameStateMickey.fShipDoorOpen) {       // inner door open
			if (_gameStateMickey.iPlanet && !_gameStateMickey.fSuit) {
				printDatMessage(arg);
			} else {
				_gameStateMickey.fShipDoorOpen = false;
				_gameStateMickey.iRmPic[_gameStateMickey.iRoom]--;

				drawRoom();

				printDatMessage(2);
			}
		} else {
			_gameStateMickey.fShipDoorOpen = true;
			_gameStateMickey.iRmPic[_gameStateMickey.iRoom]++;

			drawRoom();

			printDatMessage(14);
		}
		break;
	case IDI_MSA_ACTION_WEAR_SPACESUIT:
		if (_gameStateMickey.fSuit) {
			if (_gameStateMickey.fShipDoorOpen) {
				_gameStateMickey.fSuit = false;
				_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 0;
				_gameStateMickey.iRmPic[_gameStateMickey.iRoom] -= 2;

				drawRoom();

				printDatMessage(13);
			} else {
				printDatMessage(3);
			}
		} else {
			if (_gameStateMickey.iPlanet) {
				_gameStateMickey.fSuit = true;
				_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
				_gameStateMickey.iRmPic[_gameStateMickey.iRoom] += 2;

				drawRoom();

				printDatMessage(arg);
			} else {
				printDatMessage(12);
			}
		}
		break;
	case IDI_MSA_ACTION_READ_GAUGE:
		printDatString(arg);
		drawStr(21, 15, IDA_DEFAULT, (const char *)IDS_MSA_TEMP_C[_gameStateMickey.iPlanet]);
		drawStr(21, 23, IDA_DEFAULT, (const char *)IDS_MSA_TEMP_F[_gameStateMickey.iPlanet]);

		waitAnyKey();

		break;
	case IDI_MSA_ACTION_PRESS_ORANGE:
		if (_gameStateMickey.fFlying) {
			printDatMessage(4);
		} else {
			playSound(IDI_MSA_SND_PRESS_ORANGE);
			printDatMessage(arg);
			pressOB(IDI_MSA_BUTTON_ORANGE);
		}
		break;
	case IDI_MSA_ACTION_PRESS_BLUE:
		if (_gameStateMickey.fFlying) {
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
		if (_gameStateMickey.fFlying) {
			_gameStateMickey.fFlying = false;
			_gameStateMickey.nButtons = 0;

			memset(_gameStateMickey.szAddr, 0, sizeof(_gameStateMickey.szAddr));

			drawRoom();

			printDatString(22);

			drawStr(IDI_MSA_ROW_PLANET, IDI_MSA_COL_PLANET, IDA_DEFAULT,
			        (const char *)IDS_MSA_PLANETS[_gameStateMickey.iPlanet]);

			waitAnyKey(true);

			showPlanetInfo();
		} else {
			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_PULL_THROTTLE:
		if (_gameStateMickey.fFlying) {
			printDatMessage(18);
		} else {
			if (getPlanet() != -1) {
				_gameStateMickey.fFlying = true;
				_gameStateMickey.iPlanet = getPlanet();

				drawRoom();

				printDatMessage(16);
			} else {
				_gameStateMickey.nButtons = 0;

				memset(_gameStateMickey.szAddr, 0, sizeof(_gameStateMickey.szAddr));

				printDatMessage(17);
			}
		}
		break;
	case IDI_MSA_ACTION_LEAVE_ROOM:
		if (_gameStateMickey.fFlying) {
			printDatMessage(24);
		} else {
			_gameStateMickey.iRoom = arg;
			return true;
		}
		break;
	case IDI_MSA_ACTION_OPEN_CABINET_1:
		if (_gameStateMickey.iRmMenu[_gameStateMickey.iRoom]) {
			printLine("THE CABINET IS ALREADY OPEN");
		} else {
			_gameStateMickey.iRmMenu[_gameStateMickey.iRoom] = 1;
			_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_SHIP_KITCHEN_1;

			drawRoom();

			printDatMessage(arg);
		}
		break;
	case IDI_MSA_ACTION_READ_MAP:
		_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_STAR_MAP;

		drawRoom();

		printDatMessage(46);
		printDatMessage(47);
		printDatMessage(48);

		_gameStateMickey.iRmPic[_gameStateMickey.iRoom] = IDI_MSA_PIC_SHIP_BEDROOM;

		drawRoom();
		break;
	case IDI_MSA_ACTION_GO_WEST:
		_gameStateMickey.nButtons = 0;

		memset(_gameStateMickey.szAddr, 0, sizeof(_gameStateMickey.szAddr));

		_gameStateMickey.iRoom = arg;

		return true;
		break;

	default:
		break;
	}

	return false;
}

// Keyboard

void MickeyEngine::waitAnyKey(bool anim) {
	Common::Event event;

	if (!anim)
		_gfx->updateScreen();

	while (!shouldQuit()) {
		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				// don't interrupt if a modifier is pressed
				if (event.kbd.flags & Common::KBD_NON_STICKY) {
					continue;
				}
				// fall through
			case Common::EVENT_RETURN_TO_LAUNCHER:
			case Common::EVENT_QUIT:
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_RBUTTONUP:
				return;
			default:
				break;
			}
		}

		if (anim) {
			animate();
		}

		_gfx->updateScreen();
		_system->delayMillis(10);
	}
}

// Console-related functions

void MickeyEngine::debugCurRoom() {
	getDebugger()->debugPrintf("Current Room = %d\n", _gameStateMickey.iRoom);

	if (_gameStateMickey.iRmObj[_gameStateMickey.iRoom] != IDI_MSA_OBJECT_NONE) {
		getDebugger()->debugPrintf("Object %d is in the room\n", _gameStateMickey.iRmObj[_gameStateMickey.iRoom]);
	}
}

void MickeyEngine::debugGotoRoom(int room) {
	_gameStateMickey.iRoom = room;
	drawRoom();
}

MickeyEngine::MickeyEngine(OSystem *syst, const AGIGameDescription *gameDesc) : PreAgiEngine(syst, gameDesc) {
	_isGameOver = false;
	setDebugger(new MickeyConsole(this));
}

MickeyEngine::~MickeyEngine() {
	//_console deleted by Engine
}

void MickeyEngine::init() {
	uint8 buffer[512];

	// clear game struct
	memset(&_gameStateMickey, 0, sizeof(_gameStateMickey));
	memset(&_gameStateMickey.iItem, IDI_MSA_OBJECT_NONE, sizeof(_gameStateMickey.iItem));
	// read room extended desc flags
	//readExe(IDO_MSA_ROOM_TEXT, buffer, sizeof(buffer));
	//memcpy(_gameStateMickey.fRmTxt, buffer, sizeof(_gameStateMickey.fRmTxt));

	// read room extended desc offsets
	readExe(IDO_MSA_ROOM_TEXT_OFFSETS, buffer, sizeof(buffer));
	memcpy(_gameStateMickey.oRmTxt, buffer, sizeof(_gameStateMickey.oRmTxt));
	for (int i = 0; i < IDI_MSA_MAX_ROOM; i++)
		_gameStateMickey.oRmTxt[i] = buffer[i * 2] + 256 * buffer[i * 2 + 1];

	// read room object indices
	//readExe(IDO_MSA_ROOM_OBJECT, buffer, sizeof(buffer));
	//memcpy(_gameStateMickey.iRmObj, buffer, sizeof(_gameStateMickey.iRmObj));

	// read room picture indices
	//readExe(IDO_MSA_ROOM_PICTURE, buffer, sizeof(buffer));
	//memcpy(_gameStateMickey.iRmPic, buffer, sizeof(_gameStateMickey.iRmPic));

	// read room menu patch indices
	readExe(IDO_MSA_ROOM_MENU_FIX, buffer, sizeof(buffer));
	memcpy(_gameStateMickey.nRmMenu, buffer, sizeof(_gameStateMickey.nRmMenu));

	// set room picture and room object indices
	for (int i = 0; i < IDI_MSA_MAX_ROOM; i++) {
		_gameStateMickey.iRmPic[i] = i;
		_gameStateMickey.iRmObj[i] = -1;
	}
	_gameStateMickey.iRmPic[IDI_MSA_PIC_SHIP_AIRLOCK] = IDI_MSA_PIC_SHIP_AIRLOCK_0;
	_gameStateMickey.iRmObj[IDI_MSA_PIC_EARTH_BATHROOM] = 11;
	_gameStateMickey.iRmObj[IDI_MSA_PIC_JUPITER_LAVA] = 21;
	_gameStateMickey.iRmObj[IDI_MSA_PIC_JUPITER_ROCK_0] = 20;
	_gameStateMickey.iRmObj[IDI_MSA_PIC_JUPITER_ROCK_1] = 19;
	_gameStateMickey.iRmObj[IDI_MSA_PIC_EARTH_IN_DOGHOUSE] = 1;

#if 0
	// DEBUG
	_gameStateMickey.iPlanet = IDI_MSA_PLANET_EARTH;
	_gameStateMickey.iRoom = IDI_MSA_PIC_SHIP_CONTROLS;
	_gameStateMickey.fHasXtal = true;
	_gameStateMickey.nXtals = 9;
	_gameStateMickey.fItemUsed[IDI_MSA_ITEM_LETTER] = true;

#endif

	setFlag(VM_FLAG_SOUND_ON, true); // enable sound

	_picture->setPictureVersion(AGIPIC_PREAGI);
}

Common::Error MickeyEngine::go() {
	init();

	// Game intro
	intro();

	// Game loop
	while (!shouldQuit() && !_isGameOver) {
		drawRoom();

		if (_gameStateMickey.fIntro) {
			_gameStateMickey.fIntro = false;
		} else {
			printRoomDesc();
		}

		bool done;
		if (_gameStateMickey.iRoom == IDI_MSA_PIC_NEPTUNE_GUARD) {
			_gameStateMickey.iRoom = IDI_MSA_PIC_NEPTUNE_LEADER;
			done = true;
		} else {
			done = false;
		}

		while (!done && !shouldQuit()) {
			// Check air supply
			if (_gameStateMickey.fSuit) {
				_gameStateMickey.nAir -= 1;
				for (int i = 0; i < 4; i++) {
					if (_gameStateMickey.nAir == IDI_MSA_AIR_SUPPLY[i]) {
						playSound(IDI_MSA_SND_XL30);
						printExeMsg(IDO_MSA_XL30_SPEAKING);
						printExeMsg(IDO_MSA_AIR_SUPPLY[i]);
						if (i == 3)
							return Common::kNoError;
					}
				}
			} else {
				_gameStateMickey.nAir = 50; // max air supply
			}

			done = checkMenu();
		}

		_gameStateMickey.nFrame = 0;
	}

	return Common::kNoError;
}

bool MickeyEngine::isCrystalOnCurrentPlanet() const {
	// Earth is a special case, because the planet list may not have been
	// initialized yet. Earth is always the first planet, so if no crystals
	// have been gotten yet, then earth's crystal must still be there.
	if (_gameStateMickey.iPlanet == IDI_MSA_PLANET_EARTH) {
		return (_gameStateMickey.nXtals == 0);
	}

	if (_gameStateMickey.fPlanetsInitialized) {
		for (uint8 i = 1; i < IDI_MSA_MAX_DAT; i++) {
			if (_gameStateMickey.iPlanetXtal[i] == _gameStateMickey.iPlanet) {
				return (_gameStateMickey.nXtals <= i);
			}
		}
	}
	return false;
}

} // End of namespace Agi
