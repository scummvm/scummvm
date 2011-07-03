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
 */

#include "agi/preagi.h"
#include "agi/preagi_winnie.h"
#include "agi/graphics.h"

#include "graphics/cursorman.h"

#include "common/events.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/textconsole.h"

namespace Agi {

void Winnie::parseRoomHeader(WTP_ROOM_HDR *roomHdr, byte *buffer, int len) {
	int i;

	Common::MemoryReadStreamEndian readS(buffer, len, _isBigEndian);

	roomHdr->roomNumber = readS.readByte();
	roomHdr->objId = readS.readByte();
	roomHdr->ofsPic = readS.readUint16();
	roomHdr->fileLen = readS.readUint16();
	roomHdr->reserved0 = readS.readUint16();

	for (i = 0; i < IDI_WTP_MAX_DIR; i++)
		roomHdr->roomNew[i] = readS.readByte();

	roomHdr->objX = readS.readByte();
	roomHdr->objY = readS.readByte();

	roomHdr->reserved1 = readS.readUint16();

	for (i = 0; i < IDI_WTP_MAX_BLOCK; i++)
		roomHdr->ofsDesc[i] = readS.readUint16();

	for (i = 0; i < IDI_WTP_MAX_BLOCK; i++)
		roomHdr->ofsBlock[i] = readS.readUint16();

	for (i = 0; i < IDI_WTP_MAX_STR; i++)
		roomHdr->ofsStr[i] = readS.readUint16();

	roomHdr->reserved2 = readS.readUint32();

	for (i = 0; i < IDI_WTP_MAX_BLOCK; i++)
		for (byte j = 0; j < IDI_WTP_MAX_BLOCK; j++)
			roomHdr->opt[i].ofsOpt[j] = readS.readUint16();
}

void Winnie::parseObjHeader(WTP_OBJ_HDR *objHdr, byte *buffer, int len) {
	int i;

	Common::MemoryReadStreamEndian readS(buffer, len, _isBigEndian);

	objHdr->fileLen = readS.readUint16();
	objHdr->objId = readS.readUint16();

	for (i = 0; i < IDI_WTP_MAX_OBJ_STR_END; i++)
		objHdr->ofsEndStr[i] = readS.readUint16();

	for (i = 0; i < IDI_WTP_MAX_OBJ_STR; i++)
		objHdr->ofsStr[i] = readS.readUint16();

	objHdr->ofsPic = readS.readUint16();
}

uint32 Winnie::readRoom(int iRoom, uint8 *buffer, WTP_ROOM_HDR &roomHdr) {
	Common::String fileName;

	if (_vm->getPlatform() == Common::kPlatformPC)
		fileName = Common::String::format(IDS_WTP_ROOM_DOS, iRoom);
	else if (_vm->getPlatform() == Common::kPlatformAmiga)
		fileName = Common::String::format(IDS_WTP_ROOM_AMIGA, iRoom);
	else if (_vm->getPlatform() == Common::kPlatformC64)
		fileName = Common::String::format(IDS_WTP_ROOM_C64, iRoom);
	else if (_vm->getPlatform() == Common::kPlatformApple2GS)
		fileName = Common::String::format(IDS_WTP_ROOM_APPLE, iRoom);

	Common::File file;
	if (!file.open(fileName)) {
		warning("Could not open file \'%s\'", fileName.c_str());
		return 0;
	}

	uint32 filelen = file.size();
	if (_vm->getPlatform() == Common::kPlatformC64) { // Skip the loading address
		filelen -= 2;
		file.seek(2, SEEK_CUR);
	}

	memset(buffer, 0, 4096);
	file.read(buffer, filelen);
	file.close();

	parseRoomHeader(&roomHdr, buffer, filelen);

	return filelen;
}

uint32 Winnie::readObj(int iObj, uint8 *buffer) {
	Common::String fileName;

	if (_vm->getPlatform() == Common::kPlatformPC)
		fileName = Common::String::format(IDS_WTP_OBJ_DOS, iObj);
	else if (_vm->getPlatform() == Common::kPlatformAmiga)
		fileName = Common::String::format(IDS_WTP_OBJ_AMIGA, iObj);
	else if (_vm->getPlatform() == Common::kPlatformC64)
		fileName = Common::String::format(IDS_WTP_OBJ_C64, iObj);
	else if (_vm->getPlatform() == Common::kPlatformApple2GS)
		fileName = Common::String::format(IDS_WTP_OBJ_APPLE, iObj);

	Common::File file;
	if (!file.open(fileName)) {
		warning ("Could not open file \'%s\'", fileName.c_str());
		return 0;
	}

	uint32 filelen = file.size();
	if (_vm->getPlatform() == Common::kPlatformC64) { // Skip the loading address
		filelen -= 2;
		file.seek(2, SEEK_CUR);
	}

	memset(buffer, 0, 2048);
	file.read(buffer, filelen);
	file.close();
	return filelen;
}

void Winnie::randomize() {
	int iObj = 0;
	int iRoom = 0;
	bool done;

	for (int i = 0; i < IDI_WTP_MAX_OBJ_MISSING; i++) {
		done = false;

		while (!done) {
			iObj = _vm->rnd(IDI_WTP_MAX_OBJ - 1);
			done = true;

			for (int j = 0; j < IDI_WTP_MAX_OBJ_MISSING; j++) {
				if (_game.iUsedObj[j] == iObj) {
					done = false;
					break;
				}
			}
		}

		_game.iUsedObj[i] = iObj;

		done = false;
		while (!done) {
			iRoom = _vm->rnd(IDI_WTP_MAX_ROOM_NORMAL);
			done = true;

			for (int j = 0; j < IDI_WTP_MAX_ROOM_OBJ; j++) {
				if (_game.iObjRoom[j] == iRoom) {
					done = false;
					break;
				}
			}
		}

		_game.iObjRoom[iObj] = iRoom;
	}
}

void Winnie::intro() {
	drawPic(IDS_WTP_FILE_LOGO);
	_vm->printStr(IDS_WTP_INTRO_0);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();
	_vm->_system->delayMillis(0x640);

	if (_vm->getPlatform() == Common::kPlatformAmiga)
		_vm->_gfx->clearScreen(0);

	drawPic(IDS_WTP_FILE_TITLE);

	_vm->printStr(IDS_WTP_INTRO_1);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();
	_vm->_system->delayMillis(0x640);

	if (!playSound(IDI_WTP_SND_POOH_0))
		return;

	if (!playSound(IDI_WTP_SND_POOH_1))
		return;

	if (!playSound(IDI_WTP_SND_POOH_2))
		return;
}

int Winnie::getObjInRoom(int iRoom) {
	for (int iObj = 1; iObj < IDI_WTP_MAX_ROOM_OBJ; iObj++)
		if (_game.iObjRoom[iObj] == iRoom)
			return iObj;
	return 0;
}

void Winnie::setTakeDrop(int fCanSel[]) {
	fCanSel[IDI_WTP_SEL_TAKE] = getObjInRoom(_room);
	fCanSel[IDI_WTP_SEL_DROP] = _game.iObjHave;
}

void Winnie::setFlag(int iFlag) {
	_game.fGame[iFlag] = 1;
}

void Winnie::clearFlag(int iFlag) {
	_game.fGame[iFlag] = 0;
}

int Winnie::parser(int pc, int index, uint8 *buffer) {
	WTP_ROOM_HDR hdr;
	int startpc = pc;
	int8 opcode;
	int iNewRoom = 0;

	int iSel, iDir, iBlock;
	int fCanSel[IDI_WTP_SEL_LAST + 1];
	char szMenu[121] = {0};
	bool done;
	int fBlock;

	// extract header from buffer
	parseRoomHeader(&hdr, buffer, sizeof(WTP_ROOM_HDR));

	while (!_vm->shouldQuit()) {
		pc = startpc;

		// check if block is to be run

		iBlock = *(buffer + pc++);
		if (iBlock == 0)
			return IDI_WTP_PAR_OK;

		fBlock = *(buffer + pc++);
		if (_game.fGame[iBlock] != fBlock)
			return IDI_WTP_PAR_OK;

		// extract text from block

		opcode = *(buffer + pc);
		switch (opcode) {
		case 0:
		case IDO_WTP_OPTION_0:
		case IDO_WTP_OPTION_1:
		case IDO_WTP_OPTION_2:
			// clear fCanSel block
			memset(fCanSel, 0, sizeof(fCanSel));

			// check if NSEW directions should be displayed
			if (hdr.roomNew[0])
				fCanSel[IDI_WTP_SEL_NORTH] = fCanSel[IDI_WTP_SEL_SOUTH] =
				fCanSel[IDI_WTP_SEL_EAST] = fCanSel[IDI_WTP_SEL_WEST] = true;

			// check if object in room or player carrying one
			setTakeDrop(fCanSel);

			// check which rows have a menu option
			for (iSel = 0; iSel < IDI_WTP_MAX_OPTION; iSel++) {
				opcode = *(buffer + pc++);
				if (opcode) {
					fCanSel[opcode - IDO_WTP_OPTION_0] = true;
					fCanSel[iSel + IDI_WTP_SEL_REAL_OPT_1] = opcode - 0x14;
				}
			}

			// extract menu string
			strcpy(szMenu, (char *)(buffer + pc));
			_vm->XOR80(szMenu);
			break;
		default:
			// print description
			printStrWinnie((char *)(buffer + pc));
			if (_vm->getSelection(kSelBackspace) == 1)
				return IDI_WTP_PAR_OK;
			else
				return IDI_WTP_PAR_BACK;
		}

		// input handler

		done = false;
		while (!done) {
			// run wind if it's time
			if (_doWind)
				wind();

			// get menu selection
			getMenuSel(szMenu, &iSel, fCanSel);

			if (++_game.nMoves == IDI_WTP_MAX_MOVES_UNTIL_WIND)
				_doWind = true;

			if (_winnieEvent && (_room <= IDI_WTP_MAX_ROOM_TELEPORT)) {
				if (!_tiggerMist) {
					_tiggerMist = 1;
					tigger();
				} else {
					_tiggerMist = 0;
					mist();
				}
				_winnieEvent = false;
				return IDI_WTP_PAR_GOTO;
			}

			// process selection
			switch (iSel) {
			case IDI_WTP_SEL_HOME:
				switch (_room) {
				case IDI_WTP_ROOM_HOME:
				case IDI_WTP_ROOM_MIST:
				case IDI_WTP_ROOM_TIGGER:
					break;
				default:
					_room = IDI_WTP_ROOM_HOME;
					return IDI_WTP_PAR_GOTO;
				}
				break;
			case IDI_WTP_SEL_BACK:
				return IDI_WTP_PAR_BACK;
			case IDI_WTP_SEL_OPT_1:
			case IDI_WTP_SEL_OPT_2:
			case IDI_WTP_SEL_OPT_3:
				done = true;
				break;
			case IDI_WTP_SEL_NORTH:
			case IDI_WTP_SEL_SOUTH:
			case IDI_WTP_SEL_EAST:
			case IDI_WTP_SEL_WEST:
				iDir = iSel - IDI_WTP_SEL_NORTH;

				if (hdr.roomNew[iDir] == IDI_WTP_ROOM_NONE) {
					_vm->printStr(IDS_WTP_CANT_GO);
					_vm->getSelection(kSelAnyKey);
				} else {
					_room = hdr.roomNew[iDir];
					return IDI_WTP_PAR_GOTO;
				}
				break;
			case IDI_WTP_SEL_TAKE:
				takeObj(_room);
				setTakeDrop(fCanSel);
				break;
			case IDI_WTP_SEL_DROP:
				dropObj(_room);
				setTakeDrop(fCanSel);
				break;
			}
		}

		// jump to the script block of the selected option
		pc = hdr.opt[index].ofsOpt[iSel] - _roomOffset;

		opcode = *(buffer + pc);
		if (!opcode) pc++;

		// process script
		do {
			opcode = *(buffer + pc++);
			switch (opcode) {
			case IDO_WTP_GOTO_ROOM:
				opcode = *(buffer + pc++);
				iNewRoom = opcode;
				break;
			case IDO_WTP_PRINT_MSG:
				opcode = *(buffer + pc++);
				printRoomStr(_room, opcode);
				_vm->getSelection(kSelAnyKey);
				break;
			case IDO_WTP_PRINT_STR:
				opcode = *(buffer + pc++);
				printRoomStr(_room, opcode);
				break;
			case IDO_WTP_DROP_OBJ:
				opcode = *(buffer + pc++);
				opcode = -1;
				dropObjRnd();
				break;
			case IDO_WTP_FLAG_CLEAR:
				opcode = *(buffer + pc++);
				clearFlag(opcode);
				break;
			case IDO_WTP_FLAG_SET:
				opcode = *(buffer + pc++);
				setFlag(opcode);
				break;
			case IDO_WTP_GAME_OVER:
				gameOver();
				break;
			case IDO_WTP_WALK_MIST:
				_mist--;
				if (!_mist) {
					_room = _vm->rnd(IDI_WTP_MAX_ROOM_TELEPORT) + 1;
					return IDI_WTP_PAR_GOTO;
				}
				break;
			case IDO_WTP_PLAY_SOUND:
				opcode = *(buffer + pc++);
				playSound((ENUM_WTP_SOUND)opcode);
				break;
			case IDO_WTP_SAVE_GAME:
				saveGame();
				_room = IDI_WTP_ROOM_HOME;
				return IDI_WTP_PAR_GOTO;
			case IDO_WTP_LOAD_GAME:
				loadGame();
				_room = IDI_WTP_ROOM_HOME;
				return IDI_WTP_PAR_GOTO;
			case IDO_WTP_OWL_HELP:
				opcode = *(buffer + pc++);
				showOwlHelp();
				break;
			case IDO_WTP_GOTO_RND:
				_room = _vm->rnd(IDI_WTP_MAX_ROOM_TELEPORT) + 1;
				return IDI_WTP_PAR_GOTO;
			default:
				opcode = 0;
				break;
			}
		} while (opcode && !_vm->shouldQuit());

		if (iNewRoom) {
			_room = iNewRoom;
			return IDI_WTP_PAR_GOTO;
		}

		if (iBlock == 1)
			return IDI_WTP_PAR_OK;
		_vm->_gfx->doUpdate();
		_vm->_system->updateScreen();
	}

	return IDI_WTP_PAR_OK;
}

void Winnie::keyHelp() {
	playSound(IDI_WTP_SND_KEYHELP);
	_vm->printStr(IDS_WTP_HELP_0);
	_vm->getSelection(kSelAnyKey);
	_vm->printStr(IDS_WTP_HELP_1);
	_vm->getSelection(kSelAnyKey);
}

void Winnie::inventory() {
	if (_game.iObjHave)
		printObjStr(_game.iObjHave, IDI_WTP_OBJ_TAKE);
	else {
		_vm->clearTextArea();
		_vm->drawStr(IDI_WTP_ROW_MENU, IDI_WTP_COL_MENU, IDA_DEFAULT, IDS_WTP_INVENTORY_0);
	}

	Common::String missing = Common::String::format(IDS_WTP_INVENTORY_1, _game.nObjMiss);

	_vm->drawStr(IDI_WTP_ROW_OPTION_4, IDI_WTP_COL_MENU, IDA_DEFAULT, missing.c_str());
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen(); //TODO: Move to game's main loop
	_vm->getSelection(kSelAnyKey);
}

void Winnie::printObjStr(int iObj, int iStr) {
	WTP_OBJ_HDR hdr;
	uint8 *buffer = (uint8 *)malloc(2048);

	readObj(iObj, buffer);
	parseObjHeader(&hdr, buffer, sizeof(hdr));

	printStrWinnie((char *)(buffer + hdr.ofsStr[iStr] - _objOffset));
	free(buffer);
}

bool Winnie::isRightObj(int iRoom, int iObj, int *iCode) {
	WTP_ROOM_HDR roomhdr;
	WTP_OBJ_HDR	objhdr;
	uint8 *roomdata = (uint8 *)malloc(4096);
	uint8 *objdata = (uint8 *)malloc(2048);

	readRoom(iRoom, roomdata, roomhdr);
	readObj(iObj, objdata);
	parseObjHeader(&objhdr, objdata, sizeof(WTP_OBJ_HDR));

	free(roomdata);
	free(objdata);

	*iCode = objhdr.objId;

	if (objhdr.objId == 11) objhdr.objId = 34;

	if (roomhdr.objId == objhdr.objId)
		return true;
	else
		return false;
}

void Winnie::takeObj(int iRoom) {
	if (_game.iObjHave) {
		// player is already carrying an object, can't take
		_vm->printStr(IDS_WTP_CANT_TAKE);
		_vm->getSelection(kSelAnyKey);
	} else {
		// take object
		int iObj = getObjInRoom(iRoom);

		_game.iObjHave = iObj;
		_game.iObjRoom[iObj] = 0;

		_vm->printStr(IDS_WTP_OK);
		playSound(IDI_WTP_SND_TAKE);

		drawRoomPic();

		// print object "take" string
		printObjStr(_game.iObjHave, IDI_WTP_OBJ_TAKE);
		_vm->getSelection(kSelAnyKey);

		// HACK WARNING
		if (iObj == 18) {
			_game.fGame[0x0d] = 1;
		}
	}
}

void Winnie::dropObj(int iRoom) {
	int iCode;

	if (getObjInRoom(iRoom)) {
		// there already is an object in the room, can't drop
		_vm->printStr(IDS_WTP_CANT_DROP);
		_vm->getSelection(kSelAnyKey);
	} else {
		// HACK WARNING
		if (_game.iObjHave == 18) {
			_game.fGame[0x0d] = 0;
		}

		if (isRightObj(iRoom, _game.iObjHave, &iCode)) {
			// object has been dropped in the right place
			_vm->printStr(IDS_WTP_OK);
			_vm->getSelection(kSelAnyKey);
			playSound(IDI_WTP_SND_DROP_OK);
			printObjStr(_game.iObjHave, IDI_WTP_OBJ_DROP);
			_vm->getSelection(kSelAnyKey);

			// increase amount of objects returned, decrease amount of objects missing
			_game.nObjMiss--;
			_game.nObjRet++;

			// xor the dropped object with 0x80 to signify it has been dropped in the right place
			for (int i = 0; i < IDI_WTP_MAX_OBJ_MISSING; i++) {
				if (_game.iUsedObj[i] == _game.iObjHave) {
					_game.iUsedObj[i] ^= 0x80;
					break;
				}
			}

			// set flag according to dropped object's id
			_game.fGame[iCode] = 1;

			// player is carrying nothing
			_game.iObjHave = 0;

			if (!_game.nObjMiss) {
				// all objects returned, tell player to find party
				playSound(IDI_WTP_SND_FANFARE);
				_vm->printStr(IDS_WTP_GAME_OVER_0);
				_vm->getSelection(kSelAnyKey);
				_vm->printStr(IDS_WTP_GAME_OVER_1);
				_vm->getSelection(kSelAnyKey);
			}
		} else {
			// drop object in the given room
			_game.iObjRoom[_game.iObjHave] = iRoom;

			// object has been dropped in the wrong place
			_vm->printStr(IDS_WTP_WRONG_PLACE);
			_vm->getSelection(kSelAnyKey);

			playSound(IDI_WTP_SND_DROP);
			drawRoomPic();

			_vm->printStr(IDS_WTP_WRONG_PLACE);
			_vm->getSelection(kSelAnyKey);

			// print object description
			printObjStr(_game.iObjHave, IDI_WTP_OBJ_DESC);
			_vm->getSelection(kSelAnyKey);

			_game.iObjHave = 0;
		}
	}
}

void Winnie::dropObjRnd() {
	if (!_game.iObjHave)
		return;

	int iRoom = 0;
	bool done = false;

	while (!done) {
		iRoom = _vm->rnd(IDI_WTP_MAX_ROOM_NORMAL);
		done = true;
		if (iRoom == _room)
			done = false;
		for (int j = 0; j < IDI_WTP_MAX_ROOM_OBJ; j++) {
			if (_game.iObjRoom[j] == iRoom) {
				done = false;
			}
		}
	}

	_game.iObjRoom[_game.iObjHave] = iRoom;
	_game.iObjHave = 0;
}

void Winnie::wind() {
	int iRoom = 0;
	bool done;

	_doWind = 0;
	_game.nMoves = 0;
	if (!_game.nObjMiss)
		return;

	_vm->printStr(IDS_WTP_WIND_0);
	playSound(IDI_WTP_SND_WIND_0);
	_vm->getSelection(kSelAnyKey);

	_vm->printStr(IDS_WTP_WIND_1);
	playSound(IDI_WTP_SND_WIND_0);
	_vm->getSelection(kSelAnyKey);

	dropObjRnd();

	// randomize positions of objects at large
	for (int i = 0; i < IDI_WTP_MAX_OBJ_MISSING; i++) {
		if (!(_game.iUsedObj[i] & IDI_XOR_KEY)) {
			done = false;
			while (!done) {
				iRoom = _vm->rnd(IDI_WTP_MAX_ROOM_NORMAL);
				done = true;

				for (int j = 0; j < IDI_WTP_MAX_ROOM_OBJ; j++) {
					if (_game.iObjRoom[j] == iRoom) {
						done = false;
					}
				}
			}
			_game.iObjRoom[_game.iUsedObj[i]] = iRoom;
		}
	}
}

void Winnie::mist() {
	// mist length in turns is (2-5)
	_mist = _vm->rnd(4) + 2;

	_room = IDI_WTP_ROOM_MIST;
	drawRoomPic();

	_vm->printStr(IDS_WTP_MIST);
}

void Winnie::tigger() {
	_room = IDI_WTP_ROOM_TIGGER;

	drawRoomPic();
	_vm->printStr(IDS_WTP_TIGGER);

	dropObjRnd();
}

void Winnie::showOwlHelp() {
	if (_game.iObjHave) {
		_vm->printStr(IDS_WTP_OWL_0);
		_vm->getSelection(kSelAnyKey);
		printObjStr(_game.iObjHave, IDI_WTP_OBJ_HELP);
		_vm->getSelection(kSelAnyKey);
	}
	if (getObjInRoom(_room)) {
		_vm->printStr(IDS_WTP_OWL_0);
		_vm->getSelection(kSelAnyKey);
		printObjStr(getObjInRoom(_room), IDI_WTP_OBJ_HELP);
		_vm->getSelection(kSelAnyKey);
	}
}


void Winnie::drawMenu(char *szMenu, int iSel, int fCanSel[]) {
	int iRow = 0, iCol = 0;

	_vm->clearTextArea();
	_vm->drawStr(IDI_WTP_ROW_MENU, IDI_WTP_COL_MENU, IDA_DEFAULT, szMenu);

	if (fCanSel[IDI_WTP_SEL_NORTH])
		_vm->drawStr(IDI_WTP_ROW_OPTION_4, IDI_WTP_COL_NSEW, IDA_DEFAULT, IDS_WTP_NSEW);
	if (fCanSel[IDI_WTP_SEL_TAKE])
		_vm->drawStr(IDI_WTP_ROW_OPTION_4, IDI_WTP_COL_TAKE, IDA_DEFAULT, IDS_WTP_TAKE);
	if (fCanSel[IDI_WTP_SEL_DROP])
		_vm->drawStr(IDI_WTP_ROW_OPTION_4, IDI_WTP_COL_DROP, IDA_DEFAULT, IDS_WTP_DROP);

	switch (iSel) {
	case IDI_WTP_SEL_OPT_1:
	case IDI_WTP_SEL_OPT_2:
	case IDI_WTP_SEL_OPT_3:
		iRow = IDI_WTP_ROW_OPTION_1 + iSel;
		iCol = IDI_WTP_COL_OPTION;
		break;
	case IDI_WTP_SEL_NORTH:
		iRow = IDI_WTP_ROW_OPTION_4;
		iCol = IDI_WTP_COL_NORTH;
		break;
	case IDI_WTP_SEL_SOUTH:
		iRow = IDI_WTP_ROW_OPTION_4;
		iCol = IDI_WTP_COL_SOUTH;
		break;
	case IDI_WTP_SEL_EAST:
		iRow = IDI_WTP_ROW_OPTION_4;
		iCol = IDI_WTP_COL_EAST;
		break;
	case IDI_WTP_SEL_WEST:
		iRow = IDI_WTP_ROW_OPTION_4;
		iCol = IDI_WTP_COL_WEST;
		break;
	case IDI_WTP_SEL_TAKE:
		iRow = IDI_WTP_ROW_OPTION_4;
		iCol = IDI_WTP_COL_TAKE;
		break;
	case IDI_WTP_SEL_DROP:
		iRow = IDI_WTP_ROW_OPTION_4;
		iCol = IDI_WTP_COL_DROP;
		break;
	}
	_vm->drawStr(iRow, iCol - 1, IDA_DEFAULT, ">");
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen(); //TODO: Move to game's main loop
}

void Winnie::incMenuSel(int *iSel, int fCanSel[]) {
	do {
		*iSel += 1;
		if (*iSel > IDI_WTP_SEL_DROP) *iSel = IDI_WTP_SEL_OPT_1;
	} while (!fCanSel[*iSel]);
}

void Winnie::decMenuSel(int *iSel, int fCanSel[]) {
	do {
		*iSel -= 1;
		if (*iSel < IDI_WTP_SEL_OPT_1) *iSel = IDI_WTP_SEL_DROP;
	} while (!fCanSel[*iSel]);
}

void Winnie::getMenuMouseSel(int *iSel, int fCanSel[], int x, int y) {
	switch (y) {
	case IDI_WTP_ROW_OPTION_1:
	case IDI_WTP_ROW_OPTION_2:
	case IDI_WTP_ROW_OPTION_3:
		if (fCanSel[y - IDI_WTP_ROW_OPTION_1])	*iSel = y - IDI_WTP_ROW_OPTION_1;
		break;
	case IDI_WTP_ROW_OPTION_4:
		if (fCanSel[IDI_WTP_SEL_NORTH] && (x > IDI_WTP_COL_NORTH - 1) && (x < 6)) *iSel = IDI_WTP_SEL_NORTH;
		if (fCanSel[IDI_WTP_SEL_SOUTH] && (x > IDI_WTP_COL_SOUTH - 1) && (x < 13)) *iSel = IDI_WTP_SEL_SOUTH;
		if (fCanSel[IDI_WTP_SEL_EAST] && (x > IDI_WTP_COL_EAST - 1) && (x < 19)) *iSel = IDI_WTP_SEL_EAST;
		if (fCanSel[IDI_WTP_SEL_WEST] && (x > IDI_WTP_COL_WEST - 1) && (x < 25)) *iSel = IDI_WTP_SEL_WEST;
		if (fCanSel[IDI_WTP_SEL_TAKE] && (x > IDI_WTP_COL_TAKE - 1) && (x < 33)) *iSel = IDI_WTP_SEL_TAKE;
		if (fCanSel[IDI_WTP_SEL_DROP] && (x > IDI_WTP_COL_DROP - 1) && (x < 39)) *iSel = IDI_WTP_SEL_DROP;
		break;
	}
}

void Winnie::makeSel(int *iSel, int fCanSel[]) {
	if (fCanSel[*iSel])
		return;

	keyHelp();
	clrMenuSel(iSel, fCanSel);
}

void Winnie::getMenuSel(char *szMenu, int *iSel, int fCanSel[]) {
	Common::Event event;
	int x, y;

	clrMenuSel(iSel, fCanSel);
	drawMenu(szMenu, *iSel, fCanSel);

	// Show the mouse cursor for the menu
	CursorMan.showMouse(true);

	while (!_vm->shouldQuit()) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				return;
			case Common::EVENT_MOUSEMOVE:
				x = event.mouse.x / 8;
				y = event.mouse.y / 8;
				getMenuMouseSel(iSel, fCanSel, x, y);

				// Change cursor
				if (fCanSel[IDI_WTP_SEL_NORTH] && hotspotNorth.contains(event.mouse.x, event.mouse.y)) {
					_vm->_gfx->setCursorPalette(true);
				} else if (fCanSel[IDI_WTP_SEL_SOUTH] && hotspotSouth.contains(event.mouse.x, event.mouse.y)) {
					_vm->_gfx->setCursorPalette(true);
				} else if (fCanSel[IDI_WTP_SEL_WEST] && hotspotWest.contains(event.mouse.x, event.mouse.y)) {
					_vm->_gfx->setCursorPalette(true);
				} else if (fCanSel[IDI_WTP_SEL_EAST] && hotspotEast.contains(event.mouse.x, event.mouse.y)) {
					_vm->_gfx->setCursorPalette(true);
				} else {
					_vm->_gfx->setCursorPalette(false);
				}

				break;
			case Common::EVENT_LBUTTONUP:
				// Click to move
				if (fCanSel[IDI_WTP_SEL_NORTH] && hotspotNorth.contains(event.mouse.x, event.mouse.y)) {
					*iSel = IDI_WTP_SEL_NORTH;
					makeSel(iSel, fCanSel);
					_vm->_gfx->setCursorPalette(false);
					return;
				} else if (fCanSel[IDI_WTP_SEL_SOUTH] && hotspotSouth.contains(event.mouse.x, event.mouse.y)) {
					*iSel = IDI_WTP_SEL_SOUTH;
					makeSel(iSel, fCanSel);
					_vm->_gfx->setCursorPalette(false);
					return;
				} else if (fCanSel[IDI_WTP_SEL_WEST] && hotspotWest.contains(event.mouse.x, event.mouse.y)) {
					*iSel = IDI_WTP_SEL_WEST;
					makeSel(iSel, fCanSel);
					_vm->_gfx->setCursorPalette(false);
					return;
				} else if (fCanSel[IDI_WTP_SEL_EAST] && hotspotEast.contains(event.mouse.x, event.mouse.y)) {
					*iSel = IDI_WTP_SEL_EAST;
					makeSel(iSel, fCanSel);
					_vm->_gfx->setCursorPalette(false);
					return;
				} else {
					_vm->_gfx->setCursorPalette(false);
				}

				switch (*iSel) {
					case IDI_WTP_SEL_OPT_1:
					case IDI_WTP_SEL_OPT_2:
					case IDI_WTP_SEL_OPT_3:
						for (int iSel2 = 0; iSel2 < IDI_WTP_MAX_OPTION; iSel2++) {
							if (*iSel == (fCanSel[iSel2 + IDI_WTP_SEL_REAL_OPT_1] - 1)) {
								*iSel = iSel2;
								// Menu selection made, hide the mouse cursor
								CursorMan.showMouse(false);
								return;
							}
						}
						break;
					default:
						if (fCanSel[*iSel]) {
							// Menu selection made, hide the mouse cursor
							CursorMan.showMouse(false);
							return;
						}
						break;
				}
				break;
			case Common::EVENT_RBUTTONUP:
				*iSel = IDI_WTP_SEL_BACK;
				// Menu selection made, hide the mouse cursor
				CursorMan.showMouse(false);
				return;
			case Common::EVENT_WHEELUP:
				decMenuSel(iSel, fCanSel);
				break;
			case Common::EVENT_WHEELDOWN:
				incMenuSel(iSel, fCanSel);
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_d && (event.kbd.flags & Common::KBD_CTRL) && _vm->_console) {
					_vm->_console->attach();
					_vm->_console->onFrame();
					continue;
				}

				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					*iSel = IDI_WTP_SEL_HOME;
					// Menu selection made, hide the mouse cursor
					CursorMan.showMouse(false);
					return;
				case Common::KEYCODE_BACKSPACE:
					*iSel = IDI_WTP_SEL_BACK;
					// Menu selection made, hide the mouse cursor
					CursorMan.showMouse(false);
					return;
				case Common::KEYCODE_c:
					inventory();
					break;
				case Common::KEYCODE_SPACE:
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_DOWN:
					incMenuSel(iSel, fCanSel);
					break;
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_UP:
					decMenuSel(iSel, fCanSel);
					break;
				case Common::KEYCODE_1:
				case Common::KEYCODE_2:
				case Common::KEYCODE_3:
					*iSel = event.kbd.keycode - Common::KEYCODE_1;
					if (fCanSel[*iSel + IDI_WTP_SEL_REAL_OPT_1]) {
						// Menu selection made, hide the mouse cursor
						CursorMan.showMouse(false);
						return;
					} else {
						keyHelp();
						clrMenuSel(iSel, fCanSel);
					}
					break;
				case Common::KEYCODE_n:
					*iSel = IDI_WTP_SEL_NORTH;
					makeSel(iSel, fCanSel);
					break;
				case Common::KEYCODE_s:
					if (event.kbd.flags & Common::KBD_CTRL) {
						_vm->flipflag(fSoundOn);
					} else {
						*iSel = IDI_WTP_SEL_SOUTH;
						makeSel(iSel, fCanSel);
					}
					break;
				case Common::KEYCODE_e:
					*iSel = IDI_WTP_SEL_EAST;
					makeSel(iSel, fCanSel);
					break;
				case Common::KEYCODE_w:
					*iSel = IDI_WTP_SEL_WEST;
					makeSel(iSel, fCanSel);
					break;
				case Common::KEYCODE_t:
					*iSel = IDI_WTP_SEL_TAKE;
					makeSel(iSel, fCanSel);
					break;
				case Common::KEYCODE_d:
					*iSel = IDI_WTP_SEL_DROP;
					makeSel(iSel, fCanSel);
					break;
				case Common::KEYCODE_RETURN:
					switch (*iSel) {
					case IDI_WTP_SEL_OPT_1:
					case IDI_WTP_SEL_OPT_2:
					case IDI_WTP_SEL_OPT_3:
						for (int iSel2 = 0; iSel2 < IDI_WTP_MAX_OPTION; iSel2++) {
							if (*iSel == (fCanSel[iSel2 + IDI_WTP_SEL_REAL_OPT_1] - 1)) {
								*iSel = iSel2;
								// Menu selection made, hide the mouse cursor
								CursorMan.showMouse(false);
								return;
							}
						}
						break;
					default:
						if (fCanSel[*iSel]) {
							// Menu selection made, hide the mouse cursor
							CursorMan.showMouse(false);
							return;
						}
						break;
					}
				default:
					if (!event.kbd.flags) {	// if the control/alt/shift keys are not pressed
						keyHelp();
						clrMenuSel(iSel, fCanSel);
					}
					break;
				}
				break;
			default:
				break;
			}

			drawMenu(szMenu, *iSel, fCanSel);
		}
	}
}

void Winnie::gameLoop() {
	WTP_ROOM_HDR hdr;
	uint8 *roomdata = (uint8 *)malloc(4096);
	int iBlock;

phase0:
	if (!_game.nObjMiss && (_room == IDI_WTP_ROOM_PICNIC))
		_room = IDI_WTP_ROOM_PARTY;

	readRoom(_room, roomdata, hdr);
	drawRoomPic();
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();

phase1:
	if (getObjInRoom(_room)) {
		printObjStr(getObjInRoom(_room), IDI_WTP_OBJ_DESC);
		_vm->getSelection(kSelAnyKey);
	}

phase2:
	for (iBlock = 0; iBlock < IDI_WTP_MAX_BLOCK; iBlock++) {
		if (parser(hdr.ofsDesc[iBlock] - _roomOffset, iBlock, roomdata) == IDI_WTP_PAR_BACK)
			goto phase1;
	}

	while (!_vm->shouldQuit()) {
		for (iBlock = 0; iBlock < IDI_WTP_MAX_BLOCK; iBlock++) {
			switch (parser(hdr.ofsBlock[iBlock] - _roomOffset, iBlock, roomdata)) {
			case IDI_WTP_PAR_GOTO:
				goto phase0;
				break;
			case IDI_WTP_PAR_BACK:
				goto phase2;
				break;
			}
		}
	}

	free(roomdata);
}

void Winnie::drawPic(const char *szName) {
	Common::String fileName = szName;

	if (_vm->getPlatform() != Common::kPlatformAmiga)
		fileName += ".pic";

	Common::File file;

	if (!file.open(fileName)) {
		warning ("Could not open file \'%s\'", fileName.c_str());
		return;
	}

	uint8 *buffer = (uint8 *)malloc(4096);
	uint32 size = file.size();
	file.read(buffer, size);
	file.close();

	_vm->_picture->decodePicture(buffer, size, 1, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_picture->showPic(IDI_WTP_PIC_X0, IDI_WTP_PIC_Y0, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);

	free(buffer);
}

void Winnie::drawObjPic(int iObj, int x0, int y0) {
	if (!iObj)
		return;

	WTP_OBJ_HDR	objhdr;
	uint8 *buffer = (uint8 *)malloc(2048);
	uint32 objSize = readObj(iObj, buffer);
	parseObjHeader(&objhdr, buffer, sizeof(WTP_OBJ_HDR));

	_vm->_picture->setOffset(x0, y0);
	_vm->_picture->decodePicture(buffer + objhdr.ofsPic - _objOffset, objSize, 0, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_picture->setOffset(0, 0);
	_vm->_picture->showPic(10, 0, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);

	free(buffer);
}

void Winnie::drawRoomPic() {
	WTP_ROOM_HDR roomhdr;
	uint8 *buffer = (uint8 *)malloc(4096);
	int iObj = getObjInRoom(_room);

	// clear gfx screen
	_vm->_gfx->clearScreen(0);

	// read room picture
	readRoom(_room, buffer, roomhdr);

	// draw room picture
	_vm->_picture->decodePicture(buffer + roomhdr.ofsPic - _roomOffset, 4096, 1, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_picture->showPic(IDI_WTP_PIC_X0, IDI_WTP_PIC_Y0, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);

	// draw object picture
	drawObjPic(iObj, IDI_WTP_PIC_X0 + roomhdr.objX, IDI_WTP_PIC_Y0 + roomhdr.objY);

	free(buffer);
}

bool Winnie::playSound(ENUM_WTP_SOUND iSound) {
	//TODO
	warning ("STUB: playSound(%d)", iSound);
	return 1;
}

void Winnie::clrMenuSel(int *iSel, int fCanSel[]) {
	*iSel = IDI_WTP_SEL_OPT_1;
	while (!fCanSel[*iSel]) {
		*iSel += 1;
	}
	_vm->_gfx->setCursorPalette(false);
}

void Winnie::printRoomStr(int iRoom, int iStr) {
	WTP_ROOM_HDR hdr;
	uint8 *buffer = (uint8 *)malloc(4096);

	readRoom(iRoom, buffer, hdr);
	printStrWinnie((char *)(buffer + hdr.ofsStr[iStr - 1] - _roomOffset));

	free(buffer);
}

void Winnie::gameOver() {
	// sing the Pooh song forever
	while (!_vm->shouldQuit()) {
		_vm->printStr(IDS_WTP_SONG_0);
		playSound(IDI_WTP_SND_POOH_0);
		_vm->printStr(IDS_WTP_SONG_1);
		playSound(IDI_WTP_SND_POOH_1);
		_vm->printStr(IDS_WTP_SONG_2);
		playSound(IDI_WTP_SND_POOH_2);
		_vm->getSelection(kSelAnyKey);
	}
}

void Winnie::saveGame() {
	int i = 0;

	Common::OutSaveFile *outfile = _vm->getSaveFileMan()->openForSaving(IDS_WTP_FILE_SAVEGAME);

	if (!outfile)
		return;

	outfile->writeUint32BE(MKTAG('W','I','N','N'));	// header
	outfile->writeByte(WTP_SAVEGAME_VERSION);

	outfile->writeByte(_game.fSound);
	outfile->writeByte(_game.nMoves);
	outfile->writeByte(_game.nObjMiss);
	outfile->writeByte(_game.nObjRet);
	outfile->writeByte(_game.iObjHave);

	for (i = 0; i < IDI_WTP_MAX_FLAG; i++)
		outfile->writeByte(_game.fGame[i]);

	for (i = 0; i < IDI_WTP_MAX_OBJ_MISSING; i++)
		outfile->writeByte(_game.iUsedObj[i]);

	for (i = 0; i < IDI_WTP_MAX_ROOM_OBJ; i++)
		outfile->writeByte(_game.iObjRoom[i]);

	outfile->finalize();

	if (outfile->err())
		warning("Can't write file '%s'. (Disk full?)", IDS_WTP_FILE_SAVEGAME);

	delete outfile;
}

void Winnie::loadGame() {
	int saveVersion = 0;
	int i = 0;

	Common::InSaveFile *infile = _vm->getSaveFileMan()->openForLoading(IDS_WTP_FILE_SAVEGAME);

	if (!infile)
		return;

	if (infile->readUint32BE() == MKTAG('W','I','N','N')) {
		saveVersion = infile->readByte();
		if (saveVersion != WTP_SAVEGAME_VERSION)
			warning("Old save game version (%d, current version is %d). Will try and read anyway, but don't be surprised if bad things happen", saveVersion, WTP_SAVEGAME_VERSION);

		_game.fSound = infile->readByte();
		_game.nMoves = infile->readByte();
		_game.nObjMiss = infile->readByte();
		_game.nObjRet = infile->readByte();
		_game.iObjHave = infile->readByte();
	} else {
		// This is probably a save from the original interpreter, throw a warning and attempt
		// to read it as LE
		warning("No header found in save game, assuming it came from the original interpreter");
		// Note that the original saves variables as 16-bit integers, but only 8 bits are used.
		// Since we read the save file data as little-endian, we skip the first byte of each
		// variable

		infile->seek(0);					// Jump back to the beginning of the file

		infile->readUint16LE();				// skip unused field
		infile->readByte();					// first 8 bits of fSound
		_game.fSound = infile->readByte();
		infile->readByte();					// first 8 bits of nMoves
		_game.nMoves = infile->readByte();
		infile->readByte();					// first 8 bits of nObjMiss
		_game.nObjMiss = infile->readByte();
		infile->readByte();					// first 8 bits of nObjRet
		_game.nObjRet = infile->readByte();
		infile->readUint16LE();				// skip unused field
		infile->readUint16LE();				// skip unused field
		infile->readUint16LE();				// skip unused field
		infile->readByte();					// first 8 bits of iObjHave
		_game.iObjHave = infile->readByte();
		infile->readUint16LE();				// skip unused field
		infile->readUint16LE();				// skip unused field
		infile->readUint16LE();				// skip unused field
	}

	for (i = 0; i < IDI_WTP_MAX_FLAG; i++)
		_game.fGame[i] = infile->readByte();

	for (i = 0; i < IDI_WTP_MAX_OBJ_MISSING; i++)
		_game.iUsedObj[i] = infile->readByte();

	for (i = 0; i < IDI_WTP_MAX_ROOM_OBJ; i++)
		_game.iObjRoom[i] = infile->readByte();

	// Note that saved games from the original interpreter have 2 more 16-bit fields here
	// which are ignored

	delete infile;
}

void Winnie::printStrWinnie(char *szMsg) {
	if (_vm->getPlatform() != Common::kPlatformAmiga)
		_vm->printStrXOR(szMsg);
	else
		_vm->printStr(szMsg);
}

// Console-related functions

void Winnie::debugCurRoom() {
	_vm->_console->DebugPrintf("Current Room = %d\n", _room);
}

Winnie::Winnie(PreAgiEngine* vm) : _vm(vm) {
	_vm->_console = new Winnie_Console(_vm, this);
}

void Winnie::init() {
	memset(&_game, 0, sizeof(_game));
	_game.fSound = 1;
	_game.nObjMiss = IDI_WTP_MAX_OBJ_MISSING;
	_game.nObjRet = 0;
	_game.fGame[0] = 1;
	_game.fGame[1] = 1;
	_room = IDI_WTP_ROOM_HOME;

	_mist = -1;
	_doWind = false;
	_winnieEvent = false;

	if (_vm->getPlatform() != Common::kPlatformAmiga) {
		_isBigEndian = false;
		_roomOffset = IDI_WTP_OFS_ROOM;
		_objOffset = IDI_WTP_OFS_OBJ;
	} else {
		_isBigEndian = true;
		_roomOffset = 0;
		_objOffset = 0;
	}

	if (_vm->getPlatform() == Common::kPlatformC64 || _vm->getPlatform() == Common::kPlatformApple2GS)
		_vm->_picture->setPictureVersion(AGIPIC_C64);

	hotspotNorth = Common::Rect(20, 0, (IDI_WTP_PIC_WIDTH + 10) * 2, 10);
	hotspotSouth = Common::Rect(20, IDI_WTP_PIC_HEIGHT - 10, (IDI_WTP_PIC_WIDTH + 10) * 2, IDI_WTP_PIC_HEIGHT);
	hotspotEast  = Common::Rect(IDI_WTP_PIC_WIDTH * 2, 0, (IDI_WTP_PIC_WIDTH + 10) * 2, IDI_WTP_PIC_HEIGHT);
	hotspotWest  = Common::Rect(20, 0, 30, IDI_WTP_PIC_HEIGHT);
}

void Winnie::run() {
	randomize();
	if (_vm->getPlatform() != Common::kPlatformC64 && _vm->getPlatform() != Common::kPlatformApple2GS)
		intro();
	gameLoop();
}

}
