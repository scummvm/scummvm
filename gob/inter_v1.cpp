/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */
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

namespace Gob {

#define OPCODE(x) _OPCODE(Inter_v1, x)

Inter_v1::Inter_v1(GobEngine *vm) : Inter(vm) {
	setupOpcodes();
}

void Inter_v1::setupOpcodes(void) {
	static const OpcodeDrawEntryV1 opcodesDraw[256] = {
		/* 00 */
		OPCODE(o1_loadMult),
		OPCODE(o1_playMult),
		OPCODE(o1_freeMult),
		{NULL, ""},
		/* 04 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		OPCODE(o1_initCursor),
		/* 08 */
		OPCODE(o1_initCursorAnim),
		OPCODE(o1_clearCursorAnim),
		OPCODE(o1_setRenderFlags),
		{NULL, ""},
		/* 0C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 10 */
		OPCODE(o1_loadAnim),
		OPCODE(o1_freeAnim),
		OPCODE(o1_updateAnim),
		{NULL, ""},
		/* 14 */
		OPCODE(o1_initMult),
		OPCODE(o1_multFreeMult),
		OPCODE(o1_animate),
		OPCODE(o1_multLoadMult),
		/* 18 */
		OPCODE(o1_storeParams),
		OPCODE(o1_getObjAnimSize),
		OPCODE(o1_loadStatic),
		OPCODE(o1_freeStatic),
		/* 1C */
		OPCODE(o1_renderStatic),
		OPCODE(o1_loadCurLayer),
		{NULL, ""},
		{NULL, ""},
		/* 20 */
		OPCODE(o1_playCDTrack),
		OPCODE(o1_getCDTrackPos),
		OPCODE(o1_stopCD),
		{NULL, ""},
		/* 24 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 28 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 2C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 30 */
		OPCODE(o1_loadFontToSprite),
		OPCODE(o1_freeFontToSprite),
		{NULL, ""},
		{NULL, ""},
		/* 34 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 38 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 3C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 40 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 44 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 48 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 4C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 50 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 54 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 58 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 5C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 60 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 64 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 68 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 6C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 70 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 74 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 78 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 7C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 80 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 84 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 88 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 8C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 90 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 94 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 98 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 9C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* A0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* A4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* A8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* AC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* B0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* B4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* B8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* BC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* C0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* C4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* C8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* CC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* D0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* D4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* D8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* DC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* E0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* E4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* E8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* EC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* F0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* F4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* F8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* FC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""}
	};

	static const OpcodeFuncEntryV1 opcodesFunc[80] = {
		/* 00 */
		OPCODE(o1_callSub),
		OPCODE(o1_callSub),
		OPCODE(o1_drawPrintText),
		OPCODE(o1_loadCursor),
		/* 04 */
		{NULL, ""},
		OPCODE(o1_call),
		OPCODE(o1_repeatUntil),
		OPCODE(o1_whileDo),
		/* 08 */
		OPCODE(o1_callBool),
		OPCODE(o1_evaluateStore),
		OPCODE(o1_loadSpriteToPos),
		{NULL, ""},
		/* 0C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 10 */
		{NULL, ""},
		OPCODE(o1_printText),
		OPCODE(o1_loadTot),
		OPCODE(o1_palLoad),
		/* 14 */
		OPCODE(o1_keyFunc),
		OPCODE(o1_capturePush),
		OPCODE(o1_capturePop),
		OPCODE(o1_animPalInit),
		/* 18 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 1C */
		{NULL, ""},
		{NULL, ""},
		OPCODE(o1_drawOperations),
		OPCODE(o1_setcmdCount),
		/* 20 */
		OPCODE(o1_return),
		OPCODE(o1_renewTimeInVars),
		OPCODE(o1_speakerOn),
		OPCODE(o1_speakerOff),
		/* 24 */
		OPCODE(o1_putPixel),
		OPCODE(o1_func),
		OPCODE(o1_createSprite),
		OPCODE(o1_freeSprite),
		/* 28 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 2C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 30 */
		OPCODE(o1_returnTo),
		OPCODE(o1_loadSpriteContent),
		OPCODE(o1_copySprite),
		OPCODE(o1_fillRect),
		/* 34 */
		OPCODE(o1_drawLine),
		OPCODE(o1_strToLong),
		OPCODE(o1_invalidate),
		OPCODE(o1_setBackDelta),
		/* 38 */
		OPCODE(o1_playSound),
		OPCODE(o1_stopSound),
		OPCODE(o1_loadSound),
		OPCODE(o1_freeSoundSlot),
		/* 3C */
		OPCODE(o1_waitEndPlay),
		OPCODE(o1_playComposition),
		OPCODE(o1_getFreeMem),
		OPCODE(o1_checkData),
		/* 40 */
		{NULL, ""},
		OPCODE(o1_prepareStr),
		OPCODE(o1_insertStr),
		OPCODE(o1_cutStr),
		/* 44 */
		OPCODE(o1_strstr),
		OPCODE(o1_istrlen),
		OPCODE(o1_setMousePos),
		OPCODE(o1_setFrameRate),
		/* 48 */
		OPCODE(o1_animatePalette),
		OPCODE(o1_animateCursor),
		OPCODE(o1_blitCursor),
		OPCODE(o1_loadFont),
		/* 4C */
		OPCODE(o1_freeFont),
		OPCODE(o1_readData),
		OPCODE(o1_writeData),
		OPCODE(o1_manageDataFile),
	};

	_opcodesDrawV1 = opcodesDraw;
	_opcodesFuncV1 = opcodesFunc;
}

bool Inter_v1::o1_setMousePos(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_global->_inter_mouseX = _vm->_parse->parseValExpr();
	_vm->_global->_inter_mouseY = _vm->_parse->parseValExpr();
	if (_vm->_global->_useMouse != 0)
		_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
	return false;
}

bool Inter_v1::o1_evaluateStore(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *savedPos;
	int16 token;
	int16 result;
	int16 varOff;

	savedPos = _vm->_global->_inter_execPtr;
	varOff = _vm->_parse->parseVarIndex();
	token = evalExpr(&result);
	switch (savedPos[0]) {
	case 23:
	case 26:
		WRITE_VAR_OFFSET(varOff, _vm->_global->_inter_resVal);
		break;

	case 25:
	case 28:
		if (token == 20)
			*(_vm->_global->_inter_variables + varOff) = result;
		else
			strcpy(_vm->_global->_inter_variables + varOff, _vm->_global->_inter_resStr);
		break;

	}
	return false;
}

bool Inter_v1::o1_capturePush(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 left;
	int16 top;
	int16 width;
	int16 height;

	left = _vm->_parse->parseValExpr();
	top = _vm->_parse->parseValExpr();
	width = _vm->_parse->parseValExpr();
	height = _vm->_parse->parseValExpr();
	_vm->_game->capturePush(left, top, width, height);
	(*_vm->_scenery->pCaptureCounter)++;
	return false;
}

bool Inter_v1::o1_capturePop(char &cmdCount, int16 &counter, int16 &retFlag) {
	if (*_vm->_scenery->pCaptureCounter != 0) {
		(*_vm->_scenery->pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
	return false;
}

bool Inter_v1::o1_printText(char &cmdCount, int16 &counter, int16 &retFlag) {
	char buf[60];
	int16 i;

	debug(3, "printText");
	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->backColor = _vm->_parse->parseValExpr();
	_vm->_draw->frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->fontIndex = _vm->_parse->parseValExpr();
	_vm->_draw->destSurface = 21;
	_vm->_draw->textToPrint = buf;
	_vm->_draw->transparency = 0;

	if (_vm->_draw->backColor >= 16) {
		_vm->_draw->backColor = 0;
		_vm->_draw->transparency = 1;
	}

	do {
		for (i = 0; *_vm->_global->_inter_execPtr != '.' && (byte)*_vm->_global->_inter_execPtr != 200;
			 i++, _vm->_global->_inter_execPtr++) {
			buf[i] = *_vm->_global->_inter_execPtr;
		}

		if ((byte)*_vm->_global->_inter_execPtr != 200) {
			_vm->_global->_inter_execPtr++;
			switch (*_vm->_global->_inter_execPtr) {
			case 23:
			case 26:
				sprintf(buf + i, "%d", VAR_OFFSET(_vm->_parse->parseVarIndex()));
				break;

			case 25:
			case 28:
				sprintf(buf + i, "%s", _vm->_global->_inter_variables + _vm->_parse->parseVarIndex());
				break;
			}
			_vm->_global->_inter_execPtr++;
		} else {
			buf[i] = 0;
		}
		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
	} while ((byte)*_vm->_global->_inter_execPtr != 200);
	_vm->_global->_inter_execPtr++;

	return false;
}

bool Inter_v1::o1_animPalInit(char &cmdCount, int16 &counter, int16 &retFlag) {
	_animPalDir = load16();
	_animPalLowIndex = _vm->_parse->parseValExpr();
	_animPalHighIndex = _vm->_parse->parseValExpr();
	return false;
}

void Inter_v1::o1_loadMult(void) {
	int16 resId;

	resId = load16();
	_vm->_mult->loadMult(resId);
}

void Inter_v1::o1_playMult(void) {
	int16 checkEscape;

	checkEscape = load16();
	_vm->_mult->playMult(VAR(57), -1, checkEscape, 0);
}

void Inter_v1::o1_freeMult(void) {
	load16();		// unused
	_vm->_mult->freeMultKeys();
}

void Inter_v1::o1_initCursor(void) {
	int16 width;
	int16 height;
	int16 count;
	int16 i;

	_vm->_draw->cursorXDeltaVar = _vm->_parse->parseVarIndex();
	_vm->_draw->cursorYDeltaVar = _vm->_parse->parseVarIndex();

	width = load16();
	if (width < 16)
		width = 16;

	height = load16();
	if (height < 16)
		height = 16;

	count = load16();
	if (count < 2)
		count = 2;

	if (width != _vm->_draw->cursorWidth || height != _vm->_draw->cursorHeight ||
	    _vm->_draw->cursorSprites->width != width * count) {

		_vm->_video->freeSurfDesc(_vm->_draw->cursorSprites);
		_vm->_video->freeSurfDesc(_vm->_draw->cursorBack);

		_vm->_draw->cursorWidth = width;
		_vm->_draw->cursorHeight = height;

		if (count < 0x80)
			_vm->_draw->transparentCursor = 1;
		else
			_vm->_draw->transparentCursor = 0;

		if (count > 0x80)
			count -= 0x80;

		_vm->_draw->cursorSprites =
		    _vm->_video->initSurfDesc(_vm->_global->_videoMode, _vm->_draw->cursorWidth * count,
		    _vm->_draw->cursorHeight, 2);
		_vm->_draw->spritesArray[23] = _vm->_draw->cursorSprites;

		_vm->_draw->cursorBack =
		    _vm->_video->initSurfDesc(_vm->_global->_videoMode, _vm->_draw->cursorWidth,
		    _vm->_draw->cursorHeight, 0);
		for (i = 0; i < 40; i++) {
			_vm->_draw->cursorAnimLow[i] = -1;
			_vm->_draw->cursorAnimDelays[i] = 0;
			_vm->_draw->cursorAnimHigh[i] = 0;
		}
		_vm->_draw->cursorAnimLow[1] = 0;
	}
}

void Inter_v1::o1_initCursorAnim(void) {
	int16 ind;

	ind = _vm->_parse->parseValExpr();
	_vm->_draw->cursorAnimLow[ind] = load16();
	_vm->_draw->cursorAnimHigh[ind] = load16();
	_vm->_draw->cursorAnimDelays[ind] = load16();
}

void Inter_v1::o1_clearCursorAnim(void) {
	int16 ind;

	ind = _vm->_parse->parseValExpr();
	_vm->_draw->cursorAnimLow[ind] = -1;
	_vm->_draw->cursorAnimHigh[ind] = 0;
	_vm->_draw->cursorAnimDelays[ind] = 0;
}

bool Inter_v1::o1_drawOperations(char &cmdCount, int16 &counter, int16 &retFlag) {
	byte cmd;

	cmd = *_vm->_global->_inter_execPtr++;

	executeDrawOpcode(cmd);

	return false;
}

bool Inter_v1::o1_getFreeMem(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 freeVar;
	int16 maxFreeVar;

	freeVar = _vm->_parse->parseVarIndex();
	maxFreeVar = _vm->_parse->parseVarIndex();

	// HACK
	WRITE_VAR_OFFSET(freeVar, 1000000);
	WRITE_VAR_OFFSET(maxFreeVar, 1000000);
	return false;
}

bool Inter_v1::o1_manageDataFile(char &cmdCount, int16 &counter, int16 &retFlag) {
	evalExpr(0);

	if (_vm->_global->_inter_resStr[0] != 0)
		_vm->_dataio->openDataFile(_vm->_global->_inter_resStr);
	else
		_vm->_dataio->closeDataFile();
	return false;
}

bool Inter_v1::o1_writeData(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 offset;
	int16 handle;
	int16 size;
	int16 dataVar;
	int16 retSize;

	debug(4, "writeData");
	evalExpr(0);
	dataVar = _vm->_parse->parseVarIndex();
	size = _vm->_parse->parseValExpr();
	offset = _vm->_parse->parseValExpr();

	WRITE_VAR(1, 1);
	handle = _vm->_dataio->openData(_vm->_global->_inter_resStr, Common::File::kFileWriteMode);

	if (handle < 0)
		return false;

	if (offset < 0) {
		_vm->_dataio->seekData(handle, -offset - 1, 2);
	} else {
		_vm->_dataio->seekData(handle, offset, 0);
	}

	retSize = _vm->_dataio->file_getHandle(handle)->write(_vm->_global->_inter_variables + dataVar, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	_vm->_dataio->closeData(handle);
	return false;
}

bool Inter_v1::o1_checkData(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 handle;
	int16 varOff;

	debug(4, "_vm->_dataio->cheackData");
	evalExpr(0);
	varOff = _vm->_parse->parseVarIndex();
	handle = _vm->_dataio->openData(_vm->_global->_inter_resStr);

	WRITE_VAR_OFFSET(varOff, handle);
	if (handle >= 0)
		_vm->_dataio->closeData(handle);
	return false;
}

bool Inter_v1::o1_readData(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 retSize;
	int16 size;
	int16 dataVar;
	int16 offset;
	int16 handle;

	debug(4, "readData");
	evalExpr(0);
	dataVar = _vm->_parse->parseVarIndex();
	size = _vm->_parse->parseValExpr();
	offset = _vm->_parse->parseValExpr();

	if (_vm->_game->extHandle >= 0)
		_vm->_dataio->closeData(_vm->_game->extHandle);

	WRITE_VAR(1, 1);
	handle = _vm->_dataio->openData(_vm->_global->_inter_resStr);
	if (handle >= 0) {
		_vm->_draw->animateCursor(4);
		if (offset < 0)
			_vm->_dataio->seekData(handle, -offset - 1, 2);
		else
			_vm->_dataio->seekData(handle, offset, 0);

		retSize = _vm->_dataio->readData(handle, _vm->_global->_inter_variables + dataVar, size);
		_vm->_dataio->closeData(handle);

		if (retSize == size)
			WRITE_VAR(1, 0);
	}

	if (_vm->_game->extHandle >= 0)
		_vm->_game->extHandle = _vm->_dataio->openData(_vm->_game->curExtFile);
	return false;
}

bool Inter_v1::o1_loadFont(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;

	debug(4, "loadFont");
	evalExpr(0);
	index = load16();

	if (_vm->_draw->fonts[index] != 0)
		_vm->_util->freeFont(_vm->_draw->fonts[index]);

	_vm->_draw->animateCursor(4);
	if (_vm->_game->extHandle >= 0)
		_vm->_dataio->closeData(_vm->_game->extHandle);

	_vm->_draw->fonts[index] = _vm->_util->loadFont(_vm->_global->_inter_resStr);

	if (_vm->_game->extHandle >= 0)
		_vm->_game->extHandle = _vm->_dataio->openData(_vm->_game->curExtFile);
	return false;
}

bool Inter_v1::o1_freeFont(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;

	index = load16();
	if (_vm->_draw->fonts[index] != 0)
		_vm->_util->freeFont(_vm->_draw->fonts[index]);

	_vm->_draw->fonts[index] = 0;
	return false;
}

bool Inter_v1::o1_prepareStr(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 var;

	var = _vm->_parse->parseVarIndex();
	_vm->_util->prepareStr(_vm->_global->_inter_variables + var);
	return false;
}

bool Inter_v1::o1_insertStr(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 pos;
	int16 strVar;

	strVar = _vm->_parse->parseVarIndex();
	evalExpr(0);
	pos = _vm->_parse->parseValExpr();
	_vm->_util->insertStr(_vm->_global->_inter_resStr, _vm->_global->_inter_variables + strVar, pos);
	return false;
}

bool Inter_v1::o1_cutStr(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 var;
	int16 pos;
	int16 size;

	var = _vm->_parse->parseVarIndex();
	pos = _vm->_parse->parseValExpr();
	size = _vm->_parse->parseValExpr();
	_vm->_util->cutFromStr(_vm->_global->_inter_variables + var, pos, size);
	return false;
}

bool Inter_v1::o1_strstr(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 strVar;
	int16 resVar;
	int16 pos;

	strVar = _vm->_parse->parseVarIndex();
	evalExpr(0);
	resVar = _vm->_parse->parseVarIndex();

	pos = _vm->_util->strstr(_vm->_global->_inter_resStr, _vm->_global->_inter_variables + strVar);
	WRITE_VAR_OFFSET(resVar, pos - 1);
	return false;
}

bool Inter_v1::o1_setFrameRate(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_util->setFrameRate(_vm->_parse->parseValExpr());
	return false;
}

bool Inter_v1::o1_istrlen(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 len;
	int16 var;

	var = _vm->_parse->parseVarIndex();
	len = strlen(_vm->_global->_inter_variables + var);
	var = _vm->_parse->parseVarIndex();

	WRITE_VAR_OFFSET(var, len);
	return false;
}

bool Inter_v1::o1_strToLong(char &cmdCount, int16 &counter, int16 &retFlag) {
	char str[20];
	int16 strVar;
	int16 destVar;
	int32 res;

	strVar = _vm->_parse->parseVarIndex();
	strcpy(str, _vm->_global->_inter_variables + strVar);
	res = atol(str);

	destVar = _vm->_parse->parseVarIndex();
	WRITE_VAR_OFFSET(destVar, res);
	return false;
}

bool Inter_v1::o1_invalidate(char &cmdCount, int16 &counter, int16 &retFlag) {
	warning("invalidate: 'bugged' function!");
	_vm->_draw->destSurface = load16();
	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_INVALIDATE);
	return false;
}

bool Inter_v1::o1_loadSpriteContent(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->spriteLeft = load16();
	_vm->_draw->destSurface = load16();
	_vm->_draw->transparency = load16();
	_vm->_draw->destSpriteX = 0;
	_vm->_draw->destSpriteY = 0;
	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
	return false;
}

bool Inter_v1::o1_copySprite(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->sourceSurface = load16();
	_vm->_draw->destSurface = load16();

	_vm->_draw->spriteLeft = _vm->_parse->parseValExpr();
	_vm->_draw->spriteTop = _vm->_parse->parseValExpr();
	_vm->_draw->spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->transparency = load16();
	_vm->_draw->spriteOperation(DRAW_BLITSURF);
	return false;
}

bool Inter_v1::o1_putPixel(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->destSurface = load16();

	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_PUTPIXEL);
	return false;
}

bool Inter_v1::o1_fillRect(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->destSurface = load16();

	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->backColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_FILLRECT);
	return false;
}

bool Inter_v1::o1_drawLine(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->destSurface = load16();

	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_DRAWLINE);
	return false;
}

bool Inter_v1::o1_createSprite(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;
	int16 height;
	int16 width;
	int16 flag;

	index = load16();
	width = load16();
	height = load16();

	flag = load16();
	if (flag == 1)
		_vm->_draw->spritesArray[index] = _vm->_video->initSurfDesc(_vm->_global->_videoMode, width, height, 2);
	else
		_vm->_draw->spritesArray[index] = _vm->_video->initSurfDesc(_vm->_global->_videoMode, width, height, 0);

	_vm->_video->clearSurf(_vm->_draw->spritesArray[index]);
	return false;
}

bool Inter_v1::o1_freeSprite(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;

	index = load16();
	if (_vm->_draw->spritesArray[index] == 0)
		return false;

	_vm->_video->freeSurfDesc(_vm->_draw->spritesArray[index]);
	_vm->_draw->spritesArray[index] = 0;
	return false;
}

bool Inter_v1::o1_playComposition(char &cmdCount, int16 &counter, int16 &retFlag) {
	static int16 composition[50];
	int16 i;
	int16 dataVar;
	int16 freqVal;

	dataVar = _vm->_parse->parseVarIndex();
	freqVal = _vm->_parse->parseValExpr();
	for (i = 0; i < 50; i++)
		composition[i] = (int16)VAR_OFFSET(dataVar + i * 4);

	_vm->_snd->playComposition(_vm->_game->soundSamples, composition, freqVal);
	return false;
}

bool Inter_v1::o1_stopSound(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_snd->stopSound(_vm->_parse->parseValExpr());
	_soundEndTimeKey = 0;
	return false;
}

bool Inter_v1::o1_playSound(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 frequency;
	int16 freq2;
	int16 repCount;
	int16 index;

	index = _vm->_parse->parseValExpr();
	repCount = _vm->_parse->parseValExpr();
	frequency = _vm->_parse->parseValExpr();

	_vm->_snd->stopSound(0);
	_soundEndTimeKey = 0;
	if (_vm->_game->soundSamples[index] == 0)
		return false;

	if (repCount < 0) {
		if (_vm->_global->_soundFlags < 2)
			return false;

		repCount = -repCount;
		_soundEndTimeKey = _vm->_util->getTimeKey();

		if (frequency == 0) {
			freq2 = _vm->_game->soundSamples[index]->frequency;
		} else {
			freq2 = frequency;
		}
		_soundStopVal =
		    (10 * (_vm->_game->soundSamples[index]->size / 2)) / freq2;
		_soundEndTimeKey +=
		    ((_vm->_game->soundSamples[index]->size * repCount -
			_vm->_game->soundSamples[index]->size / 2) * 1000) / freq2;
	}
	_vm->_snd->playSample(_vm->_game->soundSamples[index], repCount, frequency);
	return false;
}

bool Inter_v1::o1_loadCursor(char &cmdCount, int16 &counter, int16 &retFlag) {
	Game::TotResItem *itemPtr;
	int16 width;
	int16 height;
	int32 offset;
	char *dataBuf;
	int16 id;
	int8 index;

	id = load16();
	index = *_vm->_global->_inter_execPtr++;
	itemPtr = &_vm->_game->totResourceTable->items[id];
	offset = itemPtr->offset;

	if (offset >= 0) {
		dataBuf =
		    ((char *)_vm->_game->totResourceTable) + szGame_TotResTable +
		    szGame_TotResItem * _vm->_game->totResourceTable->itemsCount + offset;
	} else {
		dataBuf = _vm->_game->imFileData + (int32)READ_LE_UINT32(&((int32 *)_vm->_game->imFileData)[-offset - 1]);
	}

	width = itemPtr->width;
	height = itemPtr->height;

	_vm->_video->fillRect(_vm->_draw->cursorSprites, index * _vm->_draw->cursorWidth, 0,
	    index * _vm->_draw->cursorWidth + _vm->_draw->cursorWidth - 1,
	    _vm->_draw->cursorHeight - 1, 0);

	_vm->_video->drawPackedSprite((byte*)dataBuf, width, height,
	    index * _vm->_draw->cursorWidth, 0, 0, _vm->_draw->cursorSprites);
	_vm->_draw->cursorAnimLow[index] = 0;

	return false;
}

bool Inter_v1::o1_loadSpriteToPos(char &cmdCount, int16 &counter, int16 &retFlag) {
	debug(4, "loadSpriteToPos");
	_vm->_draw->spriteLeft = load16();

	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->transparency = _vm->_global->_inter_execPtr[0];
	_vm->_draw->destSurface = (_vm->_global->_inter_execPtr[0] / 2) - 1;

	if (_vm->_draw->destSurface < 0)
		_vm->_draw->destSurface = 101;
	_vm->_draw->transparency &= 1;
	_vm->_global->_inter_execPtr += 2;
	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);

	return false;
}

bool Inter_v1::o1_loadTot(char &cmdCount, int16 &counter, int16 &retFlag) {
	char buf[20];
	int8 size;
	int16 i;

	debug(4, "loadTot");
	if ((*_vm->_global->_inter_execPtr & 0x80) != 0) {
		_vm->_global->_inter_execPtr++;
		evalExpr(0);
		strcpy(buf, _vm->_global->_inter_resStr);
	} else {
		size = *_vm->_global->_inter_execPtr++;
		for (i = 0; i < size; i++)
			buf[i] = *_vm->_global->_inter_execPtr++;

		buf[size] = 0;
	}

	strcat(buf, ".tot");
	_terminate = true;
	strcpy(_vm->_game->totToLoad, buf);

	return false;
}

bool Inter_v1::o1_keyFunc(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 flag;
	int16 key;

	debug(4, "keyFunc");
	flag = load16();
	animPalette();
	_vm->_draw->blitInvalidated();

	if (flag != 0) {

		if (flag != 1) {
			if (flag != 2) {
				_vm->_util->longDelay(flag);
				return false;
			}

			key = 0;

			if (_vm->_global->_pressedKeys[0x48])
				key |= 1;

			if (_vm->_global->_pressedKeys[0x50])
				key |= 2;

			if (_vm->_global->_pressedKeys[0x4d])
				key |= 4;

			if (_vm->_global->_pressedKeys[0x4b])
				key |= 8;

			if (_vm->_global->_pressedKeys[0x1c])
				key |= 0x10;

			if (_vm->_global->_pressedKeys[0x39])
				key |= 0x20;

			if (_vm->_global->_pressedKeys[1])
				key |= 0x40;

			if (_vm->_global->_pressedKeys[0x1d])
				key |= 0x80;

			if (_vm->_global->_pressedKeys[0x2a])
				key |= 0x100;

			if (_vm->_global->_pressedKeys[0x36])
				key |= 0x200;

			if (_vm->_global->_pressedKeys[0x38])
				key |= 0x400;

			if (_vm->_global->_pressedKeys[0x3b])
				key |= 0x800;

			if (_vm->_global->_pressedKeys[0x3c])
				key |= 0x1000;

			if (_vm->_global->_pressedKeys[0x3d])
				key |= 0x2000;

			if (_vm->_global->_pressedKeys[0x3e])
				key |= 0x4000;

			WRITE_VAR(0, key);
			_vm->_util->waitKey();
			return false;
		}
		key = _vm->_game->checkKeys(&_vm->_global->_inter_mouseX, &_vm->_global->_inter_mouseY, &_vm->_game->mouseButtons, 0);

		storeKey(key);
		return false;
	} else {
		key = _vm->_game->checkCollisions(0, 0, 0, 0);
		storeKey(key);

		if (flag == 1)
			return false;

		_vm->_util->waitKey();
	}
	return false;
}

bool Inter_v1::o1_repeatUntil(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *blockPtr;
	int16 size;
	char flag;

	debug(4, "repeatUntil");
	_nestLevel[0]++;
	blockPtr = _vm->_global->_inter_execPtr;

	do {
		_vm->_global->_inter_execPtr = blockPtr;
		size = READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		funcBlock(1);
		_vm->_global->_inter_execPtr = blockPtr + size + 1;
		flag = evalBoolResult();
	} while (flag == 0 && !_breakFlag && !_terminate);

	_nestLevel[0]--;

	if (*_breakFromLevel > -1) {
		_breakFlag = false;
		*_breakFromLevel = -1;
	}
	return false;
}

bool Inter_v1::o1_whileDo(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *blockPtr;
	char *savedIP;
	char flag;
	int16 size;

	debug(4, "whileDo");
	_nestLevel[0]++;
	do {
		savedIP = _vm->_global->_inter_execPtr;
		flag = evalBoolResult();

		if (_terminate)
			return false;

		blockPtr = _vm->_global->_inter_execPtr;

		size = READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		if (flag != 0) {
			funcBlock(1);
			_vm->_global->_inter_execPtr = savedIP;
		} else {
			_vm->_global->_inter_execPtr += size;
		}

		if (_breakFlag || _terminate) {
			_vm->_global->_inter_execPtr = blockPtr;
			_vm->_global->_inter_execPtr += size;
			break;
		}
	} while (flag != 0);

	_nestLevel[0]--;
	if (*_breakFromLevel > -1) {
		_breakFlag = false;
		*_breakFromLevel = -1;
	}
	return false;
}

void Inter_v1::o1_setRenderFlags(void) {
	_vm->_draw->renderFlags = _vm->_parse->parseValExpr();
}

void Inter_v1::o1_loadAnim(void) {
	_vm->_scenery->loadAnim(0);
}

void Inter_v1::o1_freeAnim(void) {
	_vm->_scenery->freeAnim(-1);
}

void Inter_v1::o1_updateAnim(void) {
	_vm->_scenery->interUpdateAnim();
}

void Inter_v1::o1_initMult(void) {
	_vm->_mult->interInitMult();
}

void Inter_v1::o1_multFreeMult(void) {
	_vm->_mult->freeMult();
}

void Inter_v1::o1_animate(void) {
	_vm->_mult->animate();
}

void Inter_v1::o1_multLoadMult(void) {
	_vm->_mult->interLoadMult();
}

void Inter_v1::o1_storeParams(void) {
	_vm->_scenery->interStoreParams();
}

void Inter_v1::o1_getObjAnimSize(void) {
	_vm->_mult->interGetObjAnimSize();
}

void Inter_v1::o1_loadStatic(void) {
	_vm->_scenery->loadStatic(0);
}

void Inter_v1::o1_freeStatic(void) {
	_vm->_scenery->freeStatic(-1);
}

void Inter_v1::o1_renderStatic(void) {
	_vm->_scenery->interRenderStatic();
}

void Inter_v1::o1_loadCurLayer(void) {
	_vm->_scenery->interLoadCurLayer();
}

void Inter_v1::o1_playCDTrack(void) {
	// Used in gob1 CD
	evalExpr(0);
	_vm->_cdrom->startTrack(_vm->_global->_inter_resStr);
}

void Inter_v1::o1_getCDTrackPos(void) {
	// Used in gob1 CD

	// Some scripts busy-wait while calling this opcode.
	// This is a very nasty thing to do, so let's add a
	// short delay here. It's probably a safe thing to do.

	_vm->_util->longDelay(1);

	int pos = _vm->_cdrom->getTrackPos();
	if (pos == -1)
		pos = 32767;
	WRITE_VAR(5, pos);
}

void Inter_v1::o1_stopCD(void) {
	// Used in gob1 CD
	_vm->_cdrom->stopPlaying();
}

void Inter_v1::o1_loadFontToSprite(void) {
	int16 i = load16();
	_vm->_draw->fontToSprite[i].sprite = load16();
	_vm->_draw->fontToSprite[i].base = load16();
	_vm->_draw->fontToSprite[i].width = load16();
	_vm->_draw->fontToSprite[i].height = load16();
}

void Inter_v1::o1_freeFontToSprite(void) {
	int16 i = load16();
	_vm->_draw->fontToSprite[i].sprite = -1;
	_vm->_draw->fontToSprite[i].base = -1;
	_vm->_draw->fontToSprite[i].width = -1;
	_vm->_draw->fontToSprite[i].height = -1;
}

void Inter_v1::executeDrawOpcode(byte i) {
	debug(4, "opcodeDraw %d (%s)", i, getOpcodeDrawDesc(i));

	OpcodeDrawProcV1 op = _opcodesDrawV1[i].proc;

	if (op == NULL)
		warning("unimplemented opcodeDraw: %d", i);
	else
		(this->*op) ();
}

bool Inter_v1::executeFuncOpcode(byte i, byte j, char &cmdCount, int16 &counter, int16 &retFlag) {
	debug(4, "opcodeFunc %d (%s)", i, getOpcodeFuncDesc(i, j));

	if ((i > 4) || (j > 15)) {
		warning("unimplemented opcodeFunc: %d.%d", i, j);
		return false;
	}

	OpcodeFuncProcV1 op = _opcodesFuncV1[i*16 + j].proc;

	if (op == NULL)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (cmdCount, counter, retFlag);
	return false;
}

const char *Inter_v1::getOpcodeDrawDesc(byte i) {
	return _opcodesDrawV1[i].desc;
}

const char *Inter_v1::getOpcodeFuncDesc(byte i, byte j)
{
	if ((i > 4) || (j > 15))
		return "";

	return _opcodesFuncV1[i*16 + j].desc;
}

bool Inter_v1::o1_callSub(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *storedIP = _vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr = (char *)_vm->_game->totFileData + READ_LE_UINT16(_vm->_global->_inter_execPtr);

	if (counter == cmdCount && retFlag == 2)
		return true;

	callSub(2);
	_vm->_global->_inter_execPtr = storedIP + 2;

	return false;
}

bool Inter_v1::o1_drawPrintText(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->printText();
	return false;
}

bool Inter_v1::o1_call(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *callAddr;

	checkSwitchTable(&callAddr);
	char *storedIP = _vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr = callAddr;

	if (counter == cmdCount && retFlag == 2)
		return true;

	funcBlock(0);
	_vm->_global->_inter_execPtr = storedIP;

	return false;
}

bool Inter_v1::o1_callBool(char &cmdCount, int16 &counter, int16 &retFlag) {
	byte cmd;
	bool boolRes = evalBoolResult() != 0;
	if (boolRes != 0) {
		if (counter == cmdCount
				&& retFlag == 2)
			return true;

		char *storedIP = _vm->_global->_inter_execPtr;
		funcBlock(0);
		_vm->_global->_inter_execPtr = storedIP;

		_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		debug(5, "cmd = %d", (int16)*_vm->_global->_inter_execPtr);
		cmd = (byte)(*_vm->_global->_inter_execPtr) >> 4;
		_vm->_global->_inter_execPtr++;
		if (cmd != 12)
			return false;

		_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
	} else {
		_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		debug(5, "cmd = %d", (int16)*_vm->_global->_inter_execPtr);
		cmd = (byte)(*_vm->_global->_inter_execPtr) >> 4;
		_vm->_global->_inter_execPtr++;
		if (cmd != 12)
			return false;

		if (counter == cmdCount && retFlag == 2)
			return true;

		char *storedIP = _vm->_global->_inter_execPtr;
		funcBlock(0);
		_vm->_global->_inter_execPtr = storedIP;
		_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
	}
	return false;
}

bool Inter_v1::o1_palLoad(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->interPalLoad();
	return false;
}

bool Inter_v1::o1_setcmdCount(char &cmdCount, int16 &counter, int16 &retFlag) {
	cmdCount = *_vm->_global->_inter_execPtr++;
	counter = 0;
	return false;
}

bool Inter_v1::o1_return(char &cmdCount, int16 &counter, int16 &retFlag) {
	if (retFlag != 2)
		_breakFlag = true;

	_vm->_global->_inter_execPtr = 0;
	return false;
}

bool Inter_v1::o1_renewTimeInVars(char &cmdCount, int16 &counter, int16 &retFlag) {
	renewTimeInVars();
	return false;
}

bool Inter_v1::o1_speakerOn(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_snd->speakerOn(_vm->_parse->parseValExpr(), -1);
	return false;
}

bool Inter_v1::o1_speakerOff(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_snd->speakerOff();
	return false;
}

bool Inter_v1::o1_func(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_goblin->interFunc();
	return false;
}

bool Inter_v1::o1_returnTo(char &cmdCount, int16 &counter, int16 &retFlag) {
	if (retFlag == 1) {
		_breakFlag = true;
		_vm->_global->_inter_execPtr = 0;
		return true;
	}

	if (*_nestLevel == 0)
		return false;

	*_breakFromLevel = *_nestLevel;
	_breakFlag = true;
	_vm->_global->_inter_execPtr = 0;
	return true;
}

bool Inter_v1::o1_setBackDelta(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->backDeltaX = _vm->_parse->parseValExpr();
	_vm->_draw->backDeltaY = _vm->_parse->parseValExpr();
	return false;
}

bool Inter_v1::o1_loadSound(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_game->interLoadSound(-1);
	return false;
}

bool Inter_v1::o1_freeSoundSlot(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_game->freeSoundSlot(-1);
	return false;
}

bool Inter_v1::o1_waitEndPlay(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_snd->waitEndPlay();
	return false;
}

bool Inter_v1::o1_animatePalette(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->blitInvalidated();
	_vm->_util->waitEndFrame();
	animPalette();
	storeKey(_vm->_game->checkKeys(&_vm->_global->_inter_mouseX,
		&_vm->_global->_inter_mouseY, &_vm->_game->mouseButtons, 0));
	return false;
}

bool Inter_v1::o1_animateCursor(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->animateCursor(1);
	return false;
}

bool Inter_v1::o1_blitCursor(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->blitCursor();
	return false;
}


} // End of namespace Gob
