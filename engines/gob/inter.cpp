/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/inter.h"
#include "gob/util.h"
#include "gob/scenery.h"
#include "gob/parse.h"
#include "gob/game.h"
#include "gob/draw.h"
#include "gob/mult.h"
#include "gob/goblin.h"
#include "gob/cdrom.h"
#include "gob/map.h"

namespace Gob {

Inter::Inter(GobEngine *vm) : _vm(vm) {
	_terminate = false;
	_breakFlag = false;
	_animPalLowIndex = 0;
	_animPalHighIndex = 0;
	_animPalDir = 0;
	_soundEndTimeKey = 0;
	_soundStopVal = 0;
	_breakFromLevel = 0;
	_nestLevel = 0;
}

int16 Inter::load16(void) {
	int16 tmp = (int16)READ_LE_UINT16(_vm->_global->_inter_execPtr);
	_vm->_global->_inter_execPtr += 2;
	return tmp;
}

char Inter::evalExpr(int16 *pRes) {
	byte token;

//
	_vm->_parse->printExpr(99);

	_vm->_parse->parseExpr(99, &token);
	if (pRes == 0)
		return token;

	switch (token) {
	case 20:
		*pRes = _vm->_global->_inter_resVal;
		break;

	case 22:
	case 23:
		*pRes = 0;
		break;

	case 24:
		*pRes = 1;
		break;
	}
	return token;
}

char Inter::evalBoolResult() {
	byte token;

	_vm->_parse->printExpr(99);

	_vm->_parse->parseExpr(99, &token);
	if (token == 24 || (token == 20 && _vm->_global->_inter_resVal != 0))
		return 1;
	else
		return 0;
}

void Inter::animPalette(void) {
	int16 i;
	Video::Color col;

	if (_animPalDir == 0)
		return;

	_vm->_video->waitRetrace(_vm->_global->_videoMode);

	if (_animPalDir == -1) {
		col = _vm->_draw->_vgaSmallPalette[_animPalLowIndex];

		for (i = _animPalLowIndex; i < _animPalHighIndex; i++)
			_vm->_draw->_vgaSmallPalette[i] = _vm->_draw->_vgaSmallPalette[i + 1];

		_vm->_draw->_vgaSmallPalette[_animPalHighIndex] = col;
	} else {
		col = _vm->_draw->_vgaSmallPalette[_animPalHighIndex];
		for (i = _animPalHighIndex; i > _animPalLowIndex; i--)
			_vm->_draw->_vgaSmallPalette[i] = _vm->_draw->_vgaSmallPalette[i - 1];

		_vm->_draw->_vgaSmallPalette[_animPalLowIndex] = col;
	}

	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaSmallPalette;
	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

void Inter::funcBlock(int16 retFlag) {
	char cmdCount;
	int16 counter;
	byte cmd;
	byte cmd2;

	if (_vm->_global->_inter_execPtr == 0)
		return;

	_breakFlag = false;
	_vm->_global->_inter_execPtr++;
	cmdCount = *_vm->_global->_inter_execPtr++;
	_vm->_global->_inter_execPtr += 2;

	if (cmdCount == 0) {
		_vm->_global->_inter_execPtr = 0;
		return;
	}

	int startaddr = _vm->_global->_inter_execPtr-_vm->_game->_totFileData;

	counter = 0;
	do {
		if (_terminate)
			break;

		// WORKAROUND:
		// The EGA version of gob1 doesn't add a delay after showing
		// images between levels. We manually add it here.
		if ((_vm->_features & GF_GOB1) && (_vm->_features & GF_EGA)) {
			int addr = _vm->_global->_inter_execPtr-_vm->_game->_totFileData;
			if ((startaddr == 0x18B4 && addr == 0x1A7F && // Zombie
				 !strncmp(_vm->_game->_curTotFile, "avt005.tot", 10)) ||
				(startaddr == 0x1299 && addr == 0x139A && // Dungeon
				 !strncmp(_vm->_game->_curTotFile, "avt006.tot", 10)) ||
				(startaddr == 0x11C0 && addr == 0x12C9 && // Cauldron
				 !strncmp(_vm->_game->_curTotFile, "avt012.tot", 10)) ||
				(startaddr == 0x09F2 && addr == 0x0AF3 && // Statue
				 !strncmp(_vm->_game->_curTotFile, "avt016.tot", 10)) ||
				(startaddr == 0x0B92 && addr == 0x0C93 && // Castle
				 !strncmp(_vm->_game->_curTotFile, "avt019.tot", 10)) ||
				(startaddr == 0x17D9 && addr == 0x18DA && // Finale
				 !strncmp(_vm->_game->_curTotFile, "avt022.tot", 10)))
			{
				_vm->_util->longDelay(5000);
			}
		}
		// (end workaround)

		cmd = (byte)*_vm->_global->_inter_execPtr;
		if ((cmd >> 4) >= 12) {
			cmd2 = 16 - (cmd >> 4);
			cmd &= 0xf;
		} else
			cmd2 = 0;

		_vm->_global->_inter_execPtr++;
		counter++;

		if (cmd2 == 0)
			cmd >>= 4;

		if (executeFuncOpcode(cmd2, cmd, cmdCount, counter, retFlag))
			return;

		if (_breakFlag) {
			if (retFlag != 2)
				break;

			if (*_breakFromLevel == -1)
				_breakFlag = false;
			break;
		}
	} while (counter != cmdCount);

	_vm->_global->_inter_execPtr = 0;
	return;
}

void Inter::storeKey(int16 key) {
	WRITE_VAR(12, _vm->_util->getTimeKey() - _vm->_game->_startTimeKey);

	WRITE_VAR(2, _vm->_global->_inter_mouseX);
	WRITE_VAR(3, _vm->_global->_inter_mouseY);
	WRITE_VAR(4, _vm->_game->_mouseButtons);
	WRITE_VAR(1, _vm->_snd->_playingSound);

	if (key == 0x4800)
		key = 0x0b;
	else if (key == 0x5000)
		key = 0x0a;
	else if (key == 0x4d00)
		key = 0x09;
	else if (key == 0x4b00)
		key = 0x08;
	else if (key == 0x011b)
		key = 0x1b;
	else if ((key & 0xff) != 0)
		key &= 0xff;

	WRITE_VAR(0, key);

	if (key != 0)
		_vm->_util->waitKey();
}

void Inter::checkSwitchTable(char **ppExec) {
	int16 i;
	int16 len;
	char found;
	int32 value;
	char notFound;
	char defFlag;

	found = 0;
	notFound = 1;
	*ppExec = 0;
	value = _vm->_parse->parseVarIndex();
	value = VAR_OFFSET(value);

	do {
		len = *(int8*)_vm->_global->_inter_execPtr++; // must be a signed char typ and char is not default signed on all platforms.

		if (len == -5)
			break;

		for (i = 0; i < len; i++) {
			evalExpr(0);

			if (_terminate)
				return;

			if (_vm->_global->_inter_resVal == value) {
				found = 1;
				notFound = 0;
			}
		}

		if (found != 0)
			*ppExec = _vm->_global->_inter_execPtr;

		_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
		found = 0;
	} while (len != -5);

	if (len != -5)
		_vm->_global->_inter_execPtr++;

	defFlag = *_vm->_global->_inter_execPtr;
	defFlag >>= 4;
	if (defFlag != 4)
		return;
	_vm->_global->_inter_execPtr++;

	if (notFound)
		*ppExec = _vm->_global->_inter_execPtr;

	_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
}

void Inter::callSub(int16 retFlag) {
	int16 block;
	while (_vm->_global->_inter_execPtr != 0 && (char *)_vm->_global->_inter_execPtr != _vm->_game->_totFileData) {
		block = *_vm->_global->_inter_execPtr;
		if (block == 1) {
			funcBlock(retFlag);
		} else if (block == 2) {
			_vm->_game->collisionsBlock();
		}
	}

	if ((char *)_vm->_global->_inter_execPtr == _vm->_game->_totFileData)
		_terminate = true;
}

void Inter::initControlVars(void) {
	*_nestLevel = 0;
	*_breakFromLevel = -1;

	*_vm->_scenery->_pCaptureCounter = 0;

	_breakFlag = false;
	_terminate = false;
	_animPalDir = 0;
	_soundEndTimeKey = 0;
}

void Inter::renewTimeInVars(void) {
	struct tm *t;
	time_t now = time(NULL);

	t = localtime(&now);

	WRITE_VAR(5, 1900 + t->tm_year);
	WRITE_VAR(6, t->tm_mon);
	WRITE_VAR(7, 0);
	WRITE_VAR(8, t->tm_mday);
	WRITE_VAR(9, t->tm_hour);
	WRITE_VAR(10, t->tm_min);
	WRITE_VAR(11, t->tm_sec);
}

void Inter::manipulateMap(int16 xPos, int16 yPos, int16 item) {
	for (int16 y = 0; y < Map::kMapHeight; y++) {
		for (int16 x = 0; x < Map::kMapWidth; x++) {
			if ((_vm->_map->_itemsMap[y][x] & 0xff) == item) {
				_vm->_map->_itemsMap[y][x] &= 0xff00;
			} else if (((_vm->_map->_itemsMap[y][x] & 0xff00) >> 8)
					== item) {
				_vm->_map->_itemsMap[y][x] &= 0xff;
			}
		}
	}

	if (xPos < Map::kMapWidth - 1) {
		if (yPos > 0) {
			if ((_vm->_map->_itemsMap[yPos][xPos] & 0xff00) != 0 ||
					(_vm->_map->_itemsMap[yPos - 1][xPos] & 0xff00) !=
					0
					|| (_vm->_map->_itemsMap[yPos][xPos +
						1] & 0xff00) != 0
					|| (_vm->_map->_itemsMap[yPos - 1][xPos +
						1] & 0xff00) != 0) {

				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xff00)
						+ item;

				_vm->_map->_itemsMap[yPos - 1][xPos] =
						(_vm->_map->_itemsMap[yPos -
					1][xPos] & 0xff00) + item;

				_vm->_map->_itemsMap[yPos][xPos + 1] =
						(_vm->_map->_itemsMap[yPos][xPos +
					1] & 0xff00) + item;

				_vm->_map->_itemsMap[yPos - 1][xPos + 1] =
						(_vm->_map->_itemsMap[yPos - 1][xPos +
					1] & 0xff00) + item;
			} else {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xff) +
						(item << 8);

				_vm->_map->_itemsMap[yPos - 1][xPos] =
						(_vm->_map->_itemsMap[yPos -
					1][xPos] & 0xff) + (item << 8);

				_vm->_map->_itemsMap[yPos][xPos + 1] =
						(_vm->_map->_itemsMap[yPos][xPos +
					1] & 0xff) + (item << 8);

				_vm->_map->_itemsMap[yPos - 1][xPos + 1] =
						(_vm->_map->_itemsMap[yPos - 1][xPos +
					1] & 0xff) + (item << 8);
			}
		} else {
			if ((_vm->_map->_itemsMap[yPos][xPos] & 0xff00) != 0 ||
					(_vm->_map->_itemsMap[yPos][xPos + 1] & 0xff00) !=
					0) {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xff00)
						+ item;

				_vm->_map->_itemsMap[yPos][xPos + 1] =
						(_vm->_map->_itemsMap[yPos][xPos +
					1] & 0xff00) + item;
			} else {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xff) +
						(item << 8);

				_vm->_map->_itemsMap[yPos][xPos + 1] =
						(_vm->_map->_itemsMap[yPos][xPos +
					1] & 0xff) + (item << 8);
			}
		}
	} else {
		if (yPos > 0) {
			if ((_vm->_map->_itemsMap[yPos][xPos] & 0xff00) != 0 ||
					(_vm->_map->_itemsMap[yPos - 1][xPos] & 0xff00) !=
					0) {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xff00)
						+ item;

				_vm->_map->_itemsMap[yPos - 1][xPos] =
						(_vm->_map->_itemsMap[yPos -
					1][xPos] & 0xff00) + item;
			} else {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xff) +
						(item << 8);

				_vm->_map->_itemsMap[yPos - 1][xPos] =
						(_vm->_map->_itemsMap[yPos -
					1][xPos] & 0xff) + (item << 8);
			}
		} else {
			if ((_vm->_map->_itemsMap[yPos][xPos] & 0xff00) != 0) {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xff00)
						+ item;
			} else {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xff) +
						(item << 8);
			}
		}
	}

	if (item < 0 || item >= 20)
		return;

	if (xPos > 1 && _vm->_map->_passMap[yPos][xPos - 2] == 1) {
		_vm->_map->_itemPoses[item].x = xPos - 2;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 4;
		return;
	}

	if (xPos < Map::kMapWidth - 2 && _vm->_map->_passMap[yPos][xPos + 2] == 1) {
		_vm->_map->_itemPoses[item].x = xPos + 2;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 0;
		return;
	}

	if (xPos < Map::kMapWidth - 1 && _vm->_map->_passMap[yPos][xPos + 1] == 1) {
		_vm->_map->_itemPoses[item].x = xPos + 1;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 0;
		return;
	}

	if (xPos > 0 && _vm->_map->_passMap[yPos][xPos - 1] == 1) {
		_vm->_map->_itemPoses[item].x = xPos - 1;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 4;
		return;
	}
}

} // End of namespace Gob
