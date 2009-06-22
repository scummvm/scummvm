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

#include "common/endian.h"
#include "common/file.h"
#include "graphics/dither.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/parse.h"
#include "gob/draw.h"
#include "gob/sound/sound.h"
#include "gob/videoplayer.h"

namespace Gob {

#define OPCODEVER Inter_v6
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v6::Inter_v6(GobEngine *vm) : Inter_v5(vm) {
	_gotFirstPalette = false;
}

void Inter_v6::setupOpcodesDraw() {
	Inter_v5::setupOpcodesDraw();

	OPCODEDRAW(0x40, o6_totSub);
	OPCODEDRAW(0x83, o6_playVmdOrMusic);
	OPCODEDRAW(0x85, o6_openItk);
}

void Inter_v6::setupOpcodesFunc() {
	Inter_v5::setupOpcodesFunc();

	OPCODEFUNC(0x03, o6_loadCursor);
	OPCODEFUNC(0x09, o6_assign);
	OPCODEFUNC(0x13, o6_palLoad);
	OPCODEFUNC(0x19, o6_freeCollision);
	OPCODEFUNC(0x33, o6_fillRect);
}

void Inter_v6::setupOpcodesGob() {
}

void Inter_v6::o6_totSub() {
	char totFile[14];
	byte length;
	int flags;
	int i;

	length = *_vm->_global->_inter_execPtr++;
	if ((length & 0x7F) > 13)
		error("Length in o2_totSub is greater than 13 (%d)", length);

	if (length & 0x80) {
		evalExpr(0);
		strcpy(totFile, _vm->_parse->_resultStr);
	} else {
		for (i = 0; i < length; i++)
			totFile[i] = (char) *_vm->_global->_inter_execPtr++;
		totFile[i] = 0;
	}

	flags = *_vm->_global->_inter_execPtr++;

	if (flags & 0x40)
		warning("Urban Stub: o6_totSub(), flags & 0x40");

	_vm->_game->totSub(flags, totFile);
}

void Inter_v6::o6_playVmdOrMusic() {
	char fileName[128];
	int16 x, y;
	int16 startFrame;
	int16 lastFrame;
	int16 breakKey;
	int16 flags;
	int16 palStart;
	int16 palEnd;
	uint16 palCmd;
	bool close;

	evalExpr(0);
	strncpy0(fileName, _vm->_parse->_resultStr, 127);

	x = _vm->_parse->parseValExpr();
	y = _vm->_parse->parseValExpr();
	startFrame = _vm->_parse->parseValExpr();
	lastFrame = _vm->_parse->parseValExpr();
	breakKey = _vm->_parse->parseValExpr();
	flags = _vm->_parse->parseValExpr();
	palStart = _vm->_parse->parseValExpr();
	palEnd = _vm->_parse->parseValExpr();
	palCmd = 1 << (flags & 0x3F);

	debugC(1, kDebugVideo, "Playing video \"%s\" @ %d+%d, frames %d - %d, "
			"paletteCmd %d (%d - %d), flags %X", fileName, x, y, startFrame, lastFrame,
			palCmd, palStart, palEnd, flags);

	close = false;
	if (lastFrame == -1) {
		close = true;
	} else if (lastFrame == -5) {
		_vm->_sound->bgStop();
		return;
	} else if (lastFrame == -9) {
		if (!strchr(fileName, '.'))
			strcat(fileName, ".WA8");

		probe16bitMusic(fileName);

		_vm->_sound->bgStop();
		_vm->_sound->bgPlay(fileName, SOUND_WAV);
		return;
	} else if (lastFrame == -10) {
		_vm->_vidPlayer->primaryClose();
		warning("Urban Stub: Video/Music command -10 (close video?)");
		return;
	} else if (lastFrame < 0) {
		warning("Unknown Video/Music command: %d, %s", lastFrame, fileName);
		return;
	}

	if (startFrame == -2) {
		startFrame = 0;
		lastFrame = -1;
		close = false;
	}

	if ((fileName[0] != 0) && !_vm->_vidPlayer->primaryOpen(fileName, x, y, flags)) {
		WRITE_VAR(11, (uint32) -1);
		return;
	}

	if (startFrame >= 0) {
		_vm->_game->_preventScroll = true;
		_vm->_vidPlayer->primaryPlay(startFrame, lastFrame, breakKey,
				palCmd, palStart, palEnd, 0, -1, false, -1, true);
		_vm->_game->_preventScroll = false;
	}

	if (close)
		_vm->_vidPlayer->primaryClose();
}

void Inter_v6::o6_openItk() {
	char fileName[32];

	evalExpr(0);
	strncpy0(fileName, _vm->_parse->_resultStr, 27);
	if (!strchr(fileName, '.'))
		strcat(fileName, ".ITK");

	_vm->_dataIO->openDataFile(fileName, true);

	// WORKAROUND: The CD number detection in Urban Runner is quite daft
	// (it checks CD1.ITK - CD4.ITK and the first that's found determines
	// the CD number), while its NO_CD modus wants everything in CD1.ITK.
	// So we just open the other ITKs, too.
	if (_vm->_game->_noCd && !scumm_stricmp(fileName, "CD1.ITK")) {
		_vm->_dataIO->openDataFile("CD2.ITK", true);
		_vm->_dataIO->openDataFile("CD3.ITK", true);
		_vm->_dataIO->openDataFile("CD4.ITK", true);
	}
}

bool Inter_v6::o6_loadCursor(OpFuncParams &params) {
	int16 id = load16();

	if ((id == -1) || (id == -2)) {
		char file[10];

		if (id == -1) {
			for (int i = 0; i < 9; i++)
				file[i] = *_vm->_global->_inter_execPtr++;
		} else
			strncpy(file, GET_VAR_STR(load16()), 10);

		file[9] = '\0';

		uint16 start = load16();
		int8 index = (int8) *_vm->_global->_inter_execPtr++;

		int vmdSlot = _vm->_vidPlayer->slotOpen(file);

		if (vmdSlot == -1) {
			warning("Can't open video \"%s\" as cursor", file);
			return false;
		}

		int16 framesCount = _vm->_vidPlayer->getFramesCount(vmdSlot);

		for (int i = 0; i < framesCount; i++) {
			_vm->_vidPlayer->slotPlay(vmdSlot);
			_vm->_vidPlayer->slotCopyFrame(vmdSlot, _vm->_draw->_cursorSprites->getVidMem(),
					0, 0, _vm->_draw->_cursorWidth, _vm->_draw->_cursorWidth,
					(start + i) * _vm->_draw->_cursorWidth, 0,
					_vm->_draw->_cursorSprites->getWidth());
		}

		_vm->_vidPlayer->slotClose(vmdSlot);

		_vm->_draw->_cursorAnimLow[index] = start;
		_vm->_draw->_cursorAnimHigh[index] = framesCount + start - 1;
		_vm->_draw->_cursorAnimDelays[index] = 10;

		return false;
	}

	int8 index = (int8) *_vm->_global->_inter_execPtr++;

	if ((index * _vm->_draw->_cursorWidth) >= _vm->_draw->_cursorSprites->getWidth())
		return false;

	int16 width, height;
	byte *dataBuf = _vm->_game->loadTotResource(id, 0, &width, &height);

	_vm->_video->fillRect(*_vm->_draw->_cursorSprites,
			index * _vm->_draw->_cursorWidth, 0,
			index * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
			_vm->_draw->_cursorHeight - 1, 0);

	_vm->_video->drawPackedSprite(dataBuf, width, height,
			index * _vm->_draw->_cursorWidth, 0, 0, *_vm->_draw->_cursorSprites);
	_vm->_draw->_cursorAnimLow[index] = 0;

	return false;
}

bool Inter_v6::o6_assign(OpFuncParams &params) {
	uint16 size, destType;
	int16 dest = _vm->_parse->parseVarIndex(&size, &destType);

	if (size != 0) {
		int16 src;

		byte *savedPos = _vm->_global->_inter_execPtr;

		src = _vm->_parse->parseVarIndex(&size, 0);

		memcpy(_vm->_inter->_variables->getAddressOff8(dest),
				_vm->_inter->_variables->getAddressOff8(src), size * 4);

		_vm->_global->_inter_execPtr = savedPos;

		evalExpr(&src);

		return false;
	}

	byte loopCount;
	if (*_vm->_global->_inter_execPtr == 98) {
		_vm->_global->_inter_execPtr++;
		loopCount = *_vm->_global->_inter_execPtr++;

		for (int i = 0; i < loopCount; i++) {
			uint8 c = *_vm->_global->_inter_execPtr++;
			uint16 n = load16();

			memset(_vm->_inter->_variables->getAddressOff8(dest), c, n);

			dest += n;
		}

		return false;

	} else if (*_vm->_global->_inter_execPtr == 99) {
		_vm->_global->_inter_execPtr++;
		loopCount = *_vm->_global->_inter_execPtr++;
	} else
		loopCount = 1;

	for (int i = 0; i < loopCount; i++) {
		int16 result;
		int16 srcType = evalExpr(&result);

		switch (destType) {
		case TYPE_VAR_INT8:
		case TYPE_ARRAY_INT8:
			WRITE_VARO_UINT8(dest + i, _vm->_parse->getResultInt());
			break;

		case TYPE_VAR_INT16:
		case TYPE_ARRAY_INT16:
			WRITE_VARO_UINT16(dest + i * 2, _vm->_parse->getResultInt());
			break;

		case TYPE_VAR_INT32:
		case TYPE_ARRAY_INT32:
			WRITE_VAR_OFFSET(dest + i * 4, _vm->_parse->getResultInt());
			break;

		case TYPE_VAR_INT32_AS_INT16:
			WRITE_VARO_UINT16(dest + i * 4, _vm->_parse->getResultInt());
			break;

		case TYPE_VAR_STR:
		case TYPE_ARRAY_STR:
			if (srcType == TYPE_IMM_INT16)
				WRITE_VARO_UINT8(dest, result);
			else
				WRITE_VARO_STR(dest, _vm->_parse->_resultStr);
			break;
		}
	}

	return false;
}

bool Inter_v6::o6_palLoad(OpFuncParams &params) {
	o1_palLoad(params);

	if (_gotFirstPalette)
		_vm->_video->_palLUT->setPalette((const byte *) _vm->_global->_pPaletteDesc->vgaPal,
				Graphics::PaletteLUT::kPaletteRGB, 6, 0);

	_gotFirstPalette = true;
	return false;
}

bool Inter_v6::o6_freeCollision(OpFuncParams &params) {
	int16 id;

	id = _vm->_parse->parseValExpr();

	switch (id + 5) {
	case 0:
		_vm->_game->pushCollisions(1);
		break;
	case 1:
		_vm->_game->popCollisions();
		break;
	case 2:
		_vm->_game->pushCollisions(2);
		break;
	case 3:
		for (int i = 0; i < 150; i++) {
			if (((_vm->_game->_collisionAreas[i].id & 0xF000) == 0xD000) ||
					((_vm->_game->_collisionAreas[i].id & 0xF000) == 0x4000))
				_vm->_game->_collisionAreas[i].left = 0xFFFF;
		}
		break;
	case 4:
		for (int i = 0; i < 150; i++) {
			if ((_vm->_game->_collisionAreas[i].id & 0xF000) == 0xE000)
				_vm->_game->_collisionAreas[i].left = 0xFFFF;
		}
		break;
	default:
		_vm->_game->freeCollision(0xE000 + id);
		break;
	}

	return false;
}

bool Inter_v6::o6_fillRect(OpFuncParams &params) {
	int16 destSurf;

	_vm->_draw->_destSurface = destSurf = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteBottom = _vm->_parse->parseValExpr();

	evalExpr(0);

	_vm->_draw->_backColor = _vm->_parse->getResultInt() & 0xFFFF;
	uint16 extraVar = _vm->_parse->getResultInt() >> 16;

	if (extraVar != 0)
		warning("Urban Stub: o6_fillRect(), extraVar = %d", extraVar);

	if (_vm->_draw->_spriteRight < 0) {
		_vm->_draw->_destSpriteX += _vm->_draw->_spriteRight - 1;
		_vm->_draw->_spriteRight = -_vm->_draw->_spriteRight + 2;
	}
	if (_vm->_draw->_spriteBottom < 0) {
		_vm->_draw->_destSpriteY += _vm->_draw->_spriteBottom - 1;
		_vm->_draw->_spriteBottom = -_vm->_draw->_spriteBottom + 2;
	}

	if (destSurf & 0x80) {
		warning("Urban Stub: o6_fillRect(), destSurf & 0x80");
		return false;
	}

	if (!_vm->_draw->_spritesArray[(destSurf > 100) ? (destSurf - 80) : destSurf])
		return false;

	_vm->_draw->spriteOperation(DRAW_FILLRECT);
	return false;
}

void Inter_v6::probe16bitMusic(char *fileName) {
	int len = strlen(fileName);

	if (len < 4)
		return;

	if (scumm_stricmp(fileName + len - 4, ".WA8"))
		return;

	fileName[len - 1] = 'V';

	int16 handle;
	if ((handle = _vm->_dataIO->openData(fileName)) >= 0) {
		_vm->_dataIO->closeData(handle);
		return;
	}

	fileName[len - 1] = '8';
}

} // End of namespace Gob
