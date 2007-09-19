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
#include "agi/preagi_winnie.h"
#include "agi/graphics.h"

#include "graphics/cursorman.h"

#include "common/events.h"
#include "common/savefile.h"

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
	doWind = false;
	winnie_event = false;
}

void Winnie::readRoom(int iRoom, uint8 *buffer, int buflen) {
	char szFile[256] = {0};
	sprintf(szFile, IDS_WTP_PATH_ROOM, iRoom);
	Common::File file;
	if (!file.open(szFile))
		return;
	uint32 filelen = file.size();
	memset(buffer, 0, sizeof(buffer));
	file.read(buffer, filelen);
	file.close();
}

int Winnie::readObj(int iObj, uint8 *buffer, int buflen) {
	char szFile[256] = {0};
	sprintf(szFile, IDS_WTP_PATH_OBJ, iObj);
	Common::File file;
	if (!file.open(szFile))
		return 0;
	uint32 filelen = file.size();
	memset(buffer, 0, sizeof(buffer));
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
	drawPic(IDS_WTP_FILE_LOGO);
	_vm->printStr(IDS_WTP_INTRO_0);
	_vm->_system->delayMillis(0x640);
	drawPic(IDS_WTP_FILE_TITLE);
	_vm->printStr(IDS_WTP_INTRO_1);
	_vm->_system->delayMillis(0x640);
	//if (!Winnie_PlaySound(IDI_WTP_SND_POOH_0)) return;
	//if (!Winnie_PlaySound(IDI_WTP_SND_POOH_1)) return;
	//if (!Winnie_PlaySound(IDI_WTP_SND_POOH_2)) return;
}

int Winnie::getObjInRoom(int iRoom) {
	for (int iObj = 1; iObj < IDI_WTP_MAX_ROOM_OBJ; iObj++) 
		if (game.iObjRoom[iObj] == iRoom) 
			return iObj;
	return 0;
}

#define setTakeDrop() {\
	if (getObjInRoom(room))\
		fCanSel[IDI_WTP_SEL_TAKE] = true;\
	else\
		fCanSel[IDI_WTP_SEL_TAKE] = false;\
	if (game.iObjHave)\
		fCanSel[IDI_WTP_SEL_DROP] = true;\
	else\
		fCanSel[IDI_WTP_SEL_DROP] = false;\
}

void Winnie::setFlag(int iFlag) {
	game.fGame[iFlag] = 1;
}

void Winnie::clearFlag(int iFlag) {
	game.fGame[iFlag] = 0;
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
	memcpy(&hdr, buffer, sizeof(WTP_ROOM_HDR));

	for (;;) {
		pc = startpc;

		// check if block is to be run

		iBlock = *(buffer + pc++);
		if (iBlock == 0)
			return IDI_WTP_PAR_OK;

		fBlock = *(buffer + pc++);
		if (game.fGame[iBlock] != fBlock)
			return IDI_WTP_PAR_OK;

		// extract text from block

		opcode = *(buffer + pc);
		switch(opcode) {
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
			setTakeDrop();
			
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
			_vm->printStrXOR((char *)(buffer + pc));
			if (_vm->getSelection(kSelBackspace) == 1)
				return IDI_WTP_PAR_OK;
			else 
				return IDI_WTP_PAR_BACK;
		}

		// input handler

		done = false;
		while (!done) {
			// run wind if it's time
			if (doWind)
				wind();

			// get menu selection
			getMenuSel(szMenu, &iSel, fCanSel);

			if (++game.nMoves == IDI_WTP_MAX_MOVES_UNTIL_WIND)
				doWind = true;

			if (winnie_event && (room <= IDI_WTP_MAX_ROOM_TELEPORT)) {
				if (!tigger_mist) {
					tigger_mist = 1;
					//Winnie_Tigger();
				} else {
					tigger_mist = 0;
					//Winnie_Mist();
				}
				winnie_event = false;
				return IDI_WTP_PAR_GOTO;
			}

			// process selection
			switch(iSel) {
			case IDI_WTP_SEL_HOME:
				switch(room) {
				case IDI_WTP_ROOM_HOME:
				case IDI_WTP_ROOM_MIST:
				case IDI_WTP_ROOM_TIGGER:
					break;
				default:
					room = IDI_WTP_ROOM_HOME;
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
					room = hdr.roomNew[iDir];
					return IDI_WTP_PAR_GOTO;
				}
				break;
			case IDI_WTP_SEL_TAKE:
				takeObj(room);
				setTakeDrop();
				break;
			case IDI_WTP_SEL_DROP:
				dropObj(room);
				setTakeDrop();
				break;
			}
		}

		// jump to the script block of the selected option
		pc = hdr.opt[index].ofsOpt[iSel] - IDI_WTP_OFS_ROOM;
		opcode = *(buffer + pc);
		if (!opcode) pc++;

		// process script
		do {
			opcode = *(buffer + pc++);
			switch(opcode) {
			case IDO_WTP_GOTO_ROOM:
				opcode = *(buffer + pc++);
				iNewRoom = opcode;
				break;
			case IDO_WTP_PRINT_MSG:
				opcode = *(buffer + pc++);
				printRoomStr(room, opcode);
				_vm->getSelection(kSelAnyKey);
				break;
			case IDO_WTP_PRINT_STR:
				opcode = *(buffer + pc++);
				printRoomStr(room, opcode);
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
				mist--;
				if (!mist) {
					room = _vm->rnd(IDI_WTP_MAX_ROOM_TELEPORT) + 1;
					return IDI_WTP_PAR_GOTO;
				}
				break;
			case IDO_WTP_PLAY_SOUND:
				opcode = *(buffer + pc++);
				//Winnie_PlaySound((ENUM_WTP_SOUND)opcode);
				break;
			case IDO_WTP_SAVE_GAME:
				saveGame();
				room = IDI_WTP_ROOM_HOME;
				return IDI_WTP_PAR_GOTO;
			case IDO_WTP_LOAD_GAME:
				loadGame();
				room = IDI_WTP_ROOM_HOME;
				return IDI_WTP_PAR_GOTO;
			case IDO_WTP_OWL_HELP:
				opcode = *(buffer + pc++);
				showOwlHelp();
				break;
			case IDO_WTP_GOTO_RND:
				room = _vm->rnd(IDI_WTP_MAX_ROOM_TELEPORT) + 1;
				return IDI_WTP_PAR_GOTO;
			default:
				opcode = 0;
				break;
			}
		} while (opcode);

		if (iNewRoom) {
			room = iNewRoom;
			return IDI_WTP_PAR_GOTO;
		}

		if (iBlock == 1)
			return IDI_WTP_PAR_OK;
	}
}

void Winnie::keyHelp() {
	//Winnie_PlaySound(IDI_WTP_SND_KEYHELP);
	_vm->printStr(IDS_WTP_HELP_0);
	_vm->getSelection(kSelAnyKey);
	_vm->printStr(IDS_WTP_HELP_1);
	_vm->getSelection(kSelAnyKey);
}

void Winnie::inventory() {
	char szMissing[41] = {0};

	if (game.iObjHave)
		printObjStr(game.iObjHave, IDI_WTP_OBJ_TAKE);
	else {
		_vm->clearTextArea();
		_vm->drawStr(IDI_WTP_ROW_MENU, IDI_WTP_COL_MENU, IDA_DEFAULT, IDS_WTP_INVENTORY_0);
	}

	sprintf(szMissing, IDS_WTP_INVENTORY_1, game.nObjMiss);
	_vm->drawStr(IDI_WTP_ROW_OPTION_4, IDI_WTP_COL_MENU, IDA_DEFAULT, szMissing);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();
	_vm->getSelection(kSelAnyKey);
}

void Winnie::printObjStr(int iObj, int iStr) {
	WTP_OBJ_HDR hdr;
	uint8 *buffer = (uint8 *)malloc(2048);

	readObj(iObj, buffer, 2048);
	memcpy(&hdr, buffer, sizeof(hdr));
	_vm->printStrXOR((char *)(buffer + hdr.ofsStr[iStr] - IDI_WTP_OFS_OBJ));

	free(buffer);
}

bool Winnie::isRightObj(int iRoom, int iObj, int *iCode) {
	WTP_ROOM_HDR roomhdr;
	WTP_OBJ_HDR	objhdr;
	uint8 *roomdata = (uint8 *)malloc(4096);
	uint8 *objdata = (uint8 *)malloc(2048);

	readRoom(iRoom, roomdata, 4096);
	memcpy(&roomhdr, roomdata, sizeof(WTP_ROOM_HDR));
	readObj(iObj, objdata, 2048);
	memcpy(&objhdr, objdata, sizeof(WTP_OBJ_HDR));

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
	if (game.iObjHave) {
		// player is already carrying an object, can't take
		_vm->printStr(IDS_WTP_CANT_TAKE);
		_vm->getSelection(kSelAnyKey);
	} else {
		// take object
		int iObj = getObjInRoom(iRoom);
		game.iObjHave = iObj;
		game.iObjRoom[iObj] = 0;

		_vm->printStr(IDS_WTP_OK);
		//Winnie_PlaySound(IDI_WTP_SND_TAKE);

		drawRoomPic();

		// print object "take" string
		printObjStr(game.iObjHave, IDI_WTP_OBJ_TAKE);
		_vm->getSelection(kSelAnyKey);

		// HACK WARNING
		if (iObj == 18) {
			game.fGame[0x0d] = 1;
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
		if (game.iObjHave == 18) {
			game.fGame[0x0d] = 0;
		}

		if (isRightObj(iRoom, game.iObjHave, &iCode)) {
			// object has been dropped in the right place
			_vm->printStr(IDS_WTP_OK);
			_vm->getSelection(kSelAnyKey);
			//Winnie_PlaySound(IDI_WTP_SND_DROP_OK);
			printObjStr(game.iObjHave, IDI_WTP_OBJ_DROP);
			_vm->getSelection(kSelAnyKey);

			// increase amount of objects returned, decrease amount of objects missing
			game.nObjMiss--;
			game.nObjRet++;
			
			// xor the dropped object with 0x80 to signify it has been dropped in the right place
			for (int i = 0; i < IDI_WTP_MAX_OBJ_MISSING; i++) {
				if (game.iUsedObj[i] == game.iObjHave) {
					game.iUsedObj[i] ^= 0x80;
					break;
				}
			}

			// set flag according to dropped object's id
			game.fGame[iCode] = 1;
			
			// player is carrying nothing
			game.iObjHave = 0;
			
			if (!game.nObjMiss) {
				// all objects returned, tell player to find party
				//Winnie_PlaySound(IDI_WTP_SND_FANFARE);
				_vm->printStr(IDS_WTP_GAME_OVER_0);
				_vm->getSelection(kSelAnyKey);
				_vm->printStr(IDS_WTP_GAME_OVER_1);
				_vm->getSelection(kSelAnyKey);
			}
		} else {
			// drop object in the given room
			game.iObjRoom[game.iObjHave] = iRoom;

			// object has been dropped in the wrong place
			_vm->printStr(IDS_WTP_WRONG_PLACE);
			_vm->getSelection(kSelAnyKey);
			//Winnie_PlaySound(IDI_WTP_SND_DROP);
			drawRoomPic();
			_vm->printStr(IDS_WTP_WRONG_PLACE);
			_vm->getSelection(kSelAnyKey);

			// print object description
			printObjStr(game.iObjHave, IDI_WTP_OBJ_DESC);
			_vm->getSelection(kSelAnyKey);

			game.iObjHave = 0;
		}
	}
}

void Winnie::dropObjRnd() {
	if (!game.iObjHave)
		return;
	
	int iRoom = 0;
	bool done = false;

	while (!done) {
		iRoom = _vm->rnd(IDI_WTP_MAX_ROOM_NORMAL);
		done = true;
		if (iRoom == room)
			done = false;
		for (int j = 0; j < IDI_WTP_MAX_ROOM_OBJ; j++) {
			if (game.iObjRoom[j] == iRoom) {
				done = false;
			}
		}
	}

	game.iObjRoom[game.iObjHave] = iRoom;
	game.iObjHave = 0;
}

void Winnie::wind() {
	int iRoom = 0;
	bool done;

	doWind = 0;
	game.nMoves = 0;
	if (!game.nObjMiss)
		return;

	_vm->printStr(IDS_WTP_WIND_0);
	//Winnie_PlaySound(IDI_WTP_SND_WIND_0);
	_vm->getSelection(kSelAnyKey);
	_vm->printStr(IDS_WTP_WIND_1);
	//Winnie_PlaySound(IDI_WTP_SND_WIND_0);
	_vm->getSelection(kSelAnyKey);

	dropObjRnd();

	// randomize positions of objects at large
	for (int i = 0; i < IDI_WTP_MAX_OBJ_MISSING; i++) {
		if (!(game.iUsedObj[i] & IDI_XOR_KEY)) {
			done = false;
			while (!done) {
				iRoom = _vm->rnd(IDI_WTP_MAX_ROOM_NORMAL);
				done = true;
				for (int j = 0; j < IDI_WTP_MAX_ROOM_OBJ; j++) {
					if (game.iObjRoom[j] == iRoom) {
						done = false;
					}
				}
			}
			game.iObjRoom[game.iUsedObj[i]] = iRoom;
		}
	}
}

void Winnie::showOwlHelp() {
	if (game.iObjHave) {
		_vm->printStr(IDS_WTP_OWL_0);
		_vm->getSelection(kSelAnyKey);
		printObjStr(game.iObjHave, IDI_WTP_OBJ_HELP);
		_vm->getSelection(kSelAnyKey);
	}
	if (getObjInRoom(room)) {
		_vm->printStr(IDS_WTP_OWL_0);
		_vm->getSelection(kSelAnyKey);
		printObjStr(getObjInRoom(room), IDI_WTP_OBJ_HELP);
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
	
	switch(iSel) {
	case IDI_WTP_SEL_OPT_1:
		iRow = IDI_WTP_ROW_OPTION_1;
		iCol = IDI_WTP_COL_OPTION;
		break;
	case IDI_WTP_SEL_OPT_2:
		iRow = IDI_WTP_ROW_OPTION_2;
		iCol = IDI_WTP_COL_OPTION;
		break;
	case IDI_WTP_SEL_OPT_3:
		iRow = IDI_WTP_ROW_OPTION_3;
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
	_vm->drawStr(iRow, iCol - 1, IDA_DEFAULT, IDS_WTP_SELECTION);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop
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
	switch(y) {
	case IDI_WTP_ROW_OPTION_1:
		if (fCanSel[IDI_WTP_SEL_OPT_1])	*iSel = IDI_WTP_SEL_OPT_1;
		break;
	case IDI_WTP_ROW_OPTION_2:
		if (fCanSel[IDI_WTP_SEL_OPT_2])	*iSel = IDI_WTP_SEL_OPT_2;
		break;
	case IDI_WTP_ROW_OPTION_3:
		if (fCanSel[IDI_WTP_SEL_OPT_3])	*iSel = IDI_WTP_SEL_OPT_3;
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

#define makeSel() {\
	if (fCanSel[*iSel]) {\
		return;\
	} else {\
		keyHelp();\
		clrMenuSel(iSel, fCanSel);\
	}\
}

void Winnie::getMenuSel(char *szMenu, int *iSel, int fCanSel[]) {
	Common::Event event;
	int x, y;

	clrMenuSel(iSel, fCanSel);
	drawMenu(szMenu, *iSel, fCanSel);

	// Show the mouse cursor for the menu
	CursorMan.showMouse(true);

	for (;;) {
		// check if tigger/mist is to be triggered
//			if (something)
//				event = true;

		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				_vm->_system->quit();
				break;
			case Common::EVENT_MOUSEMOVE:
				x = event.mouse.x / 8;
				y = event.mouse.y / 8;
				getMenuMouseSel(iSel, fCanSel, x, y);

				// Change cursor
				if (fCanSel[IDI_WTP_SEL_NORTH] && (event.mouse.x >= 20 && event.mouse.x <= (IDI_WTP_PIC_WIDTH + 10) * 2) &&
					(event.mouse.y >= 0 && event.mouse.y <= 10)) {
					_vm->_gfx->setCursorPalette(true);
				} else if (fCanSel[IDI_WTP_SEL_SOUTH] && (event.mouse.x >= 20 && event.mouse.x <= (IDI_WTP_PIC_WIDTH + 10) * 2) &&
					(event.mouse.y >= IDI_WTP_PIC_HEIGHT - 10 && event.mouse.y <= IDI_WTP_PIC_HEIGHT)) {
					_vm->_gfx->setCursorPalette(true);			
				} else if (fCanSel[IDI_WTP_SEL_WEST] && (event.mouse.y >= 0  && event.mouse.y <= IDI_WTP_PIC_HEIGHT) &&
					(event.mouse.x >= 20 && event.mouse.x <= 30)) {
					_vm->_gfx->setCursorPalette(true);
				} else if (fCanSel[IDI_WTP_SEL_EAST] && (event.mouse.y >= 0  && event.mouse.y <= IDI_WTP_PIC_HEIGHT) &&
					(event.mouse.x >= IDI_WTP_PIC_WIDTH * 2 && event.mouse.x <= (IDI_WTP_PIC_WIDTH + 10) * 2)) {
					_vm->_gfx->setCursorPalette(true);
				} else {
					_vm->_gfx->setCursorPalette(false);
				}

				break;
			case Common::EVENT_LBUTTONUP:
				// Click to move
				if (fCanSel[IDI_WTP_SEL_NORTH] && (event.mouse.x >= 20 && event.mouse.x <= (IDI_WTP_PIC_WIDTH + 10) * 2) &&
					(event.mouse.y >= 0 && event.mouse.y <= 10)) {
					*iSel = IDI_WTP_SEL_NORTH;
					makeSel();
					_vm->_gfx->setCursorPalette(false);
					return;
				} else if (fCanSel[IDI_WTP_SEL_SOUTH] && (event.mouse.x >= 20 && event.mouse.x <= (IDI_WTP_PIC_WIDTH + 10) * 2) &&
					(event.mouse.y >= IDI_WTP_PIC_HEIGHT - 10 && event.mouse.y <= IDI_WTP_PIC_HEIGHT)) {
					*iSel = IDI_WTP_SEL_SOUTH;
					makeSel();
					_vm->_gfx->setCursorPalette(false);
					return;
				} else if (fCanSel[IDI_WTP_SEL_WEST] && (event.mouse.y >= 0  && event.mouse.y <= IDI_WTP_PIC_HEIGHT) &&
					(event.mouse.x >= 20 && event.mouse.x <= 30)) {
					*iSel = IDI_WTP_SEL_WEST;
					makeSel();
					_vm->_gfx->setCursorPalette(false);
					return;
				} else if (fCanSel[IDI_WTP_SEL_EAST] && (event.mouse.y >= 0  && event.mouse.y <= IDI_WTP_PIC_HEIGHT) &&
					(event.mouse.x >= IDI_WTP_PIC_WIDTH * 2 && event.mouse.x <= (IDI_WTP_PIC_WIDTH + 10) * 2)) {
					*iSel = IDI_WTP_SEL_EAST;
					makeSel();
					_vm->_gfx->setCursorPalette(false);
					return;
				} else {
					_vm->_gfx->setCursorPalette(false);
				}

				switch(*iSel) {
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
					makeSel();
					break;
				case Common::KEYCODE_s:
					if (event.kbd.flags & Common::KBD_CTRL) {
						_vm->flipflag(fSoundOn);
					} else {
						*iSel = IDI_WTP_SEL_SOUTH;
						makeSel();
					}
					break;
				case Common::KEYCODE_e:
					*iSel = IDI_WTP_SEL_EAST;
					makeSel();
					break;
				case Common::KEYCODE_w:
					*iSel = IDI_WTP_SEL_WEST;
					makeSel();
					break;
				case Common::KEYCODE_t:
					*iSel = IDI_WTP_SEL_TAKE;
					makeSel();
					break;
				case Common::KEYCODE_d:
					*iSel = IDI_WTP_SEL_DROP;
					makeSel();
					break;
				case Common::KEYCODE_RETURN:
					switch(*iSel) {
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
					keyHelp();
					clrMenuSel(iSel, fCanSel);
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
	if (!game.nObjMiss && (room == IDI_WTP_ROOM_PICNIC))
		room = IDI_WTP_ROOM_PARTY;
	readRoom(room, roomdata, 4096);
	memcpy(&hdr, roomdata, sizeof(WTP_ROOM_HDR));
	drawRoomPic();
phase1:
	if (getObjInRoom(room)) {
		printObjStr(getObjInRoom(room), IDI_WTP_OBJ_DESC);
		_vm->getSelection(kSelAnyKey);
	}
phase2:
	for (iBlock = 0; iBlock < IDI_WTP_MAX_BLOCK; iBlock++) {
		if (parser(hdr.ofsDesc[iBlock] - IDI_WTP_OFS_ROOM, iBlock, roomdata) == IDI_WTP_PAR_BACK) {
			goto phase1;
		}
	}
	for (;;) {
		for (iBlock = 0; iBlock < IDI_WTP_MAX_BLOCK; iBlock++) {
			switch(parser(hdr.ofsBlock[iBlock] - IDI_WTP_OFS_ROOM, iBlock, roomdata)) {
			case IDI_WTP_PAR_GOTO:
				goto phase0;
				break;
			case IDI_WTP_PAR_BACK:
				goto phase2;
				break;
			}
		}
	}

	delete [] roomdata;
}

void Winnie::drawPic(const char *szName) {
	char szFile[256] = {0};
	uint8 *buffer = (uint8 *)malloc(4096);

	// construct filename
	sprintf(szFile, IDS_WTP_PATH, szName);
	Common::File file;
	if (!file.open(szName))
		return;
	uint32 size = file.size();
	file.read(buffer, size);
	file.close();

	_vm->_picture->decodePicture(buffer, size, 1, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_picture->showPic(IDI_WTP_PIC_X0, IDI_WTP_PIC_Y0, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop

	delete [] buffer;
}

void Winnie::drawObjPic(int iObj, int x0, int y0) {
	WTP_OBJ_HDR	objhdr;
	uint8 *buffer = (uint8 *)malloc(2048);

	if (!iObj)
		return;

	int objSize = readObj(iObj, buffer, 2048);
	memcpy(&objhdr, buffer, sizeof(WTP_OBJ_HDR));

	_vm->_picture->setOffset(x0, y0);
	_vm->_picture->decodePicture(buffer + objhdr.ofsPic - IDI_WTP_OFS_OBJ, objSize, 0, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_picture->setOffset(0, 0);
	_vm->_picture->showPic(10, 0, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();

	delete [] buffer;
}

void Winnie::drawRoomPic() {
	WTP_ROOM_HDR roomhdr;
	uint8 *buffer = (uint8 *)malloc(4096);
	int iObj = getObjInRoom(room);

	// clear gfx screen
	_vm->_gfx->clearScreen(0);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop

	// read room picture
	readRoom(room, buffer, 4096);
	memcpy(&roomhdr, buffer, sizeof(WTP_ROOM_HDR));

	// draw room picture
	_vm->_picture->decodePicture(buffer + roomhdr.ofsPic - IDI_WTP_OFS_ROOM, 4096, 1, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_picture->showPic(IDI_WTP_PIC_X0, IDI_WTP_PIC_Y0, IDI_WTP_PIC_WIDTH, IDI_WTP_PIC_HEIGHT);
	_vm->_gfx->doUpdate();
	_vm->_system->updateScreen();	// TODO: this should go in the game's main loop

	// draw object picture
	drawObjPic(iObj, IDI_WTP_PIC_X0 + roomhdr.objX, IDI_WTP_PIC_Y0 + roomhdr.objY);

	delete [] buffer;
}

void Winnie::clrMenuSel(int *iSel, int fCanSel[]) {
	*iSel = IDI_WTP_SEL_OPT_1;
	while (!fCanSel[*iSel]) {
		*iSel += 1;
	}
}

void Winnie::printRoomStr(int iRoom, int iStr) {
	WTP_ROOM_HDR hdr;
	uint8 *buffer = (uint8 *)malloc(4096);

	readRoom(iRoom, buffer, 4096);
	memcpy(&hdr, buffer, sizeof(hdr));
	_vm->printStrXOR((char *)(buffer + hdr.ofsStr[iStr - 1] - IDI_WTP_OFS_ROOM));

	free(buffer);
}

void Winnie::gameOver() {
	// sing the Pooh song forever
	for (;;) {
		_vm->printStr(IDS_WTP_SONG_0);
		//Winnie_PlaySound(IDI_WTP_SND_POOH_0);
		_vm->printStr(IDS_WTP_SONG_1);
		//Winnie_PlaySound(IDI_WTP_SND_POOH_1);
		_vm->printStr(IDS_WTP_SONG_2);
		//Winnie_PlaySound(IDI_WTP_SND_POOH_2);
		_vm->getSelection(kSelAnyKey);
	}
}

void Winnie::saveGame() {
	uint8 *buffer = (uint8 *)malloc(sizeof(WTP_SAVE_GAME));
	memcpy(buffer, &game, sizeof(WTP_SAVE_GAME));
	writeSaveGame(buffer);
	delete [] buffer;
}

void Winnie::loadGame() {
	uint8 *buffer = (uint8 *)malloc(sizeof(WTP_SAVE_GAME));
	readSaveGame(buffer);
	memcpy(&game, buffer, sizeof(WTP_SAVE_GAME));
	delete [] buffer;
}

void Winnie::readSaveGame(uint8 *buffer) {
	Common::InSaveFile* infile;
	char szFile[256] = {0};
	sprintf(szFile, IDS_WTP_FILE_SAVEGAME);
	if (!(infile = _vm->getSaveFileMan()->openForLoading(szFile)))
		return;
	infile->read(buffer, sizeof(WTP_SAVE_GAME));
	delete infile;
}

void Winnie::writeSaveGame(uint8 *buffer) {
	Common::OutSaveFile* outfile;
	char szFile[256] = {0};
	sprintf(szFile, IDS_WTP_FILE_SAVEGAME);
	if (!(outfile = _vm->getSaveFileMan()->openForSaving(szFile)))
		return;
	outfile->write(buffer, sizeof(WTP_SAVE_GAME));
	delete outfile;
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
	gameLoop();
}

}
