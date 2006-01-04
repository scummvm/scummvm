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
	int16 tmp = (int16)READ_LE_UINT16(_vm->_global->inter_execPtr);
	_vm->_global->inter_execPtr += 2;
	return tmp;
}

void Inter::setMousePos(void) {
	_vm->_global->inter_mouseX = _vm->_parse->parseValExpr();
	_vm->_global->inter_mouseY = _vm->_parse->parseValExpr();
	if (_vm->_global->useMouse != 0)
		_vm->_util->setMousePos(_vm->_global->inter_mouseX, _vm->_global->inter_mouseY);
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
		*pRes = _vm->_global->inter_resVal;
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
	if (token == 24 || (token == 20 && _vm->_global->inter_resVal != 0))
		return 1;
	else
		return 0;
}

void Inter::evaluateStore(void) {
	char *savedPos;
	int16 token;
	int16 result;
	int16 varOff;

	savedPos = _vm->_global->inter_execPtr;
	varOff = _vm->_parse->parseVarIndex();
	token = evalExpr(&result);
	switch (savedPos[0]) {
	case 23:
	case 26:
		WRITE_VAR_OFFSET(varOff, _vm->_global->inter_resVal);
		break;

	case 25:
	case 28:
		if (token == 20)
			*(_vm->_global->inter_variables + varOff) = result;
		else
			strcpy(_vm->_global->inter_variables + varOff, _vm->_global->inter_resStr);
		break;

	}
	return;
}

void Inter::capturePush(void) {
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
}

void Inter::capturePop(void) {
	if (*_vm->_scenery->pCaptureCounter != 0) {
		(*_vm->_scenery->pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
}

void Inter::printText(void) {
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
		for (i = 0; *_vm->_global->inter_execPtr != '.' && (byte)*_vm->_global->inter_execPtr != 200;
			 i++, _vm->_global->inter_execPtr++) {
			buf[i] = *_vm->_global->inter_execPtr;
		}

		if ((byte)*_vm->_global->inter_execPtr != 200) {
			_vm->_global->inter_execPtr++;
			switch (*_vm->_global->inter_execPtr) {
			case 23:
			case 26:
				sprintf(buf + i, "%d", VAR_OFFSET(_vm->_parse->parseVarIndex()));
				break;

			case 25:
			case 28:
				sprintf(buf + i, "%s", _vm->_global->inter_variables + _vm->_parse->parseVarIndex());
				break;
			}
			_vm->_global->inter_execPtr++;
		} else {
			buf[i] = 0;
		}
		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
	} while ((byte)*_vm->_global->inter_execPtr != 200);
	_vm->_global->inter_execPtr++;
}

void Inter::animPalette(void) {
	int16 i;
	Video::Color col;

	if (_animPalDir == 0)
		return;

	_vm->_video->waitRetrace(_vm->_global->videoMode);

	if (_animPalDir == -1) {
		col = _vm->_draw->vgaSmallPalette[_animPalLowIndex];

		for (i = _animPalLowIndex; i < _animPalHighIndex; i++)
			_vm->_draw->vgaSmallPalette[i] = _vm->_draw->vgaSmallPalette[i + 1];

		_vm->_draw->vgaSmallPalette[_animPalHighIndex] = col;
	} else {
		col = _vm->_draw->vgaSmallPalette[_animPalHighIndex];
		for (i = _animPalHighIndex; i > _animPalLowIndex; i--)
			_vm->_draw->vgaSmallPalette[i] = _vm->_draw->vgaSmallPalette[i - 1];

		_vm->_draw->vgaSmallPalette[_animPalLowIndex] = col;
	}

	_vm->_global->pPaletteDesc->vgaPal = _vm->_draw->vgaSmallPalette;
	_vm->_video->setFullPalette(_vm->_global->pPaletteDesc);
}

void Inter::animPalInit(void) {
	_animPalDir = load16();
	_animPalLowIndex = _vm->_parse->parseValExpr();
	_animPalHighIndex = _vm->_parse->parseValExpr();
}

void Inter::loadMult(void) {
	int16 resId;

	resId = load16();
	_vm->_mult->loadMult(resId);
}

void Inter::playMult(void) {
	int16 checkEscape;

	checkEscape = load16();
	_vm->_mult->playMult(VAR(57), -1, checkEscape, 0);
}

void Inter::freeMult(void) {
	load16();		// unused
	_vm->_mult->freeMultKeys();
}

void Inter::initCursor(void) {
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
		    _vm->_video->initSurfDesc(_vm->_global->videoMode, _vm->_draw->cursorWidth * count,
		    _vm->_draw->cursorHeight, 2);
		_vm->_draw->spritesArray[23] = _vm->_draw->cursorSprites;

		_vm->_draw->cursorBack =
		    _vm->_video->initSurfDesc(_vm->_global->videoMode, _vm->_draw->cursorWidth,
		    _vm->_draw->cursorHeight, 0);
		for (i = 0; i < 40; i++) {
			_vm->_draw->cursorAnimLow[i] = -1;
			_vm->_draw->cursorAnimDelays[i] = 0;
			_vm->_draw->cursorAnimHigh[i] = 0;
		}
		_vm->_draw->cursorAnimLow[1] = 0;
	}
}

void Inter::initCursorAnim(void) {
	int16 ind;

	ind = _vm->_parse->parseValExpr();
	_vm->_draw->cursorAnimLow[ind] = load16();
	_vm->_draw->cursorAnimHigh[ind] = load16();
	_vm->_draw->cursorAnimDelays[ind] = load16();
}

void Inter::clearCursorAnim(void) {
	int16 ind;

	ind = _vm->_parse->parseValExpr();
	_vm->_draw->cursorAnimLow[ind] = -1;
	_vm->_draw->cursorAnimHigh[ind] = 0;
	_vm->_draw->cursorAnimDelays[ind] = 0;
}

void Inter::drawOperations(void) {
	byte cmd;
	int16 i;

	cmd = *_vm->_global->inter_execPtr++;

	debug(4, "drawOperations(%d)", cmd);

	switch (cmd) {
	case 0:
		loadMult();
		break;

	case 1:
		playMult();
		break;

	case 2:
		freeMult();
		break;

	case 7:
		initCursor();
		break;

	case 8:
		initCursorAnim();
		break;

	case 9:
		clearCursorAnim();
		break;

	case 10:
		_vm->_draw->renderFlags = _vm->_parse->parseValExpr();
		break;

	case 11:
		//word_23EC_DE = _vm->_parse->parseValExpr();
		break;

	case 16:
		_vm->_scenery->loadAnim(0);
		break;

	case 17:
		_vm->_scenery->freeAnim(-1);
		break;

	case 18:
		_vm->_scenery->interUpdateAnim();
		break;

	case 19:
		warning("Gob2 mult stub");
		break;

	case 20:
		_vm->_mult->interInitMult();
		break;

	case 21:
		_vm->_mult->freeMult();
		break;

	case 22:
		_vm->_mult->animate();
		break;

	case 23:
		_vm->_mult->interLoadMult();
		break;

	case 24:
		_vm->_scenery->interStoreParams();
		break;

	case 25:
		_vm->_mult->interGetObjAnimSize();
		break;

	case 26:
		_vm->_scenery->loadStatic(0);
		break;

	case 27:
		_vm->_scenery->freeStatic(-1);
		break;

	case 28:
		_vm->_scenery->interRenderStatic();
		break;

	case 29:
		_vm->_scenery->interLoadCurLayer();
		break;

	case 32:
		if (_vm->_features & GF_GOB1) {
			// Used in gob1 CD
			evalExpr(0);
			_vm->_cdrom->startTrack(_vm->_global->inter_resStr);
		} else {
		}
		break;

	case 33:
		if (_vm->_features & GF_GOB1) {
			// Used in gob1 CD

			// Some scripts busy-wait while calling this opcode.
			// This is a very nasty thing to do, so let's add a
			// short delay here. It's probably a safe thing to do.

			_vm->_util->longDelay(1);

			int pos = _vm->_cdrom->getTrackPos();
			if (pos == -1)
				pos = 32767;
			WRITE_VAR(5, pos);
		} else {
		}
		break;

	case 34:
		if (_vm->_features & GF_GOB1) {
			// Used in gob1 CD
			_vm->_cdrom->stopPlaying();
		} else {
		}
		break;

	case 35:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 36:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 37:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 48:
		i = load16();
		_vm->_draw->fontToSprite[i].sprite = load16();
		_vm->_draw->fontToSprite[i].base = load16();
		_vm->_draw->fontToSprite[i].width = load16();
		_vm->_draw->fontToSprite[i].height = load16();
		break;

	case 49:
		i = load16();
		_vm->_draw->fontToSprite[i].sprite = -1;
		_vm->_draw->fontToSprite[i].base = -1;
		_vm->_draw->fontToSprite[i].width = -1;
		_vm->_draw->fontToSprite[i].height = -1;
		break;

	case 64:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 65:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 66:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 67:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 80:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 81:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 82:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 83:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 84:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 85:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 86:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 128:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 129:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 130:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 131:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 132:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 133:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 134:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 135:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	case 136:
		if (_vm->_features & GF_GOB2) {
		}
		break;

	default:
		warning("unimplemented drawOperation: %d", cmd);
	}
}

void Inter::getFreeMem(void) {
	int16 freeVar;
	int16 maxFreeVar;

	freeVar = _vm->_parse->parseVarIndex();
	maxFreeVar = _vm->_parse->parseVarIndex();

	// HACK
	WRITE_VAR_OFFSET(freeVar, 1000000);
	WRITE_VAR_OFFSET(maxFreeVar, 1000000);
}

void Inter::manageDataFile(void) {
	evalExpr(0);

	if (_vm->_global->inter_resStr[0] != 0)
		_vm->_dataio->openDataFile(_vm->_global->inter_resStr);
	else
		_vm->_dataio->closeDataFile();
}

void Inter::writeData(void) {
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
	handle = _vm->_dataio->openData(_vm->_global->inter_resStr, Common::File::kFileWriteMode);

	if (handle < 0)
		return;

	if (offset < 0) {
		_vm->_dataio->seekData(handle, -offset - 1, 2);
	} else {
		_vm->_dataio->seekData(handle, offset, 0);
	}

	retSize = _vm->_dataio->file_getHandle(handle)->write(_vm->_global->inter_variables + dataVar, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	_vm->_dataio->closeData(handle);
}

void Inter::checkData(void) {
	int16 handle;
	int16 varOff;

	debug(4, "_vm->_dataio->cheackData");
	evalExpr(0);
	varOff = _vm->_parse->parseVarIndex();
	handle = _vm->_dataio->openData(_vm->_global->inter_resStr);

	WRITE_VAR_OFFSET(varOff, handle);
	if (handle >= 0)
		_vm->_dataio->closeData(handle);
}

void Inter::readData(void) {
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
	handle = _vm->_dataio->openData(_vm->_global->inter_resStr);
	if (handle >= 0) {
		_vm->_draw->animateCursor(4);
		if (offset < 0)
			_vm->_dataio->seekData(handle, -offset - 1, 2);
		else
			_vm->_dataio->seekData(handle, offset, 0);

		retSize = _vm->_dataio->readData(handle, _vm->_global->inter_variables + dataVar, size);
		_vm->_dataio->closeData(handle);

		if (retSize == size)
			WRITE_VAR(1, 0);
	}

	if (_vm->_game->extHandle >= 0)
		_vm->_game->extHandle = _vm->_dataio->openData(_vm->_game->curExtFile);
}

void Inter::loadFont(void) {
	int16 index;

	debug(4, "loadFont");
	evalExpr(0);
	index = load16();

	if (_vm->_draw->fonts[index] != 0)
		_vm->_util->freeFont(_vm->_draw->fonts[index]);

	_vm->_draw->animateCursor(4);
	if (_vm->_game->extHandle >= 0)
		_vm->_dataio->closeData(_vm->_game->extHandle);

	_vm->_draw->fonts[index] = _vm->_util->loadFont(_vm->_global->inter_resStr);

	if (_vm->_game->extHandle >= 0)
		_vm->_game->extHandle = _vm->_dataio->openData(_vm->_game->curExtFile);
}

void Inter::freeFont(void) {
	int16 index;

	index = load16();
	if (_vm->_draw->fonts[index] != 0)
		_vm->_util->freeFont(_vm->_draw->fonts[index]);

	_vm->_draw->fonts[index] = 0;
}

void Inter::prepareStr(void) {
	int16 var;

	var = _vm->_parse->parseVarIndex();
	_vm->_util->prepareStr(_vm->_global->inter_variables + var);
}

void Inter::insertStr(void) {
	int16 pos;
	int16 strVar;

	strVar = _vm->_parse->parseVarIndex();
	evalExpr(0);
	pos = _vm->_parse->parseValExpr();
	_vm->_util->insertStr(_vm->_global->inter_resStr, _vm->_global->inter_variables + strVar, pos);
}

void Inter::cutStr(void) {
	int16 var;
	int16 pos;
	int16 size;

	var = _vm->_parse->parseVarIndex();
	pos = _vm->_parse->parseValExpr();
	size = _vm->_parse->parseValExpr();
	_vm->_util->cutFromStr(_vm->_global->inter_variables + var, pos, size);
}

void Inter::strstr(void) {
	int16 strVar;
	int16 resVar;
	int16 pos;

	strVar = _vm->_parse->parseVarIndex();
	evalExpr(0);
	resVar = _vm->_parse->parseVarIndex();

	pos = _vm->_util->strstr(_vm->_global->inter_resStr, _vm->_global->inter_variables + strVar);
	WRITE_VAR_OFFSET(resVar, pos - 1);
}

void Inter::setFrameRate(void) {
	_vm->_util->setFrameRate(_vm->_parse->parseValExpr());
}

void Inter::istrlen(void) {
	int16 len;
	int16 var;

	var = _vm->_parse->parseVarIndex();
	len = strlen(_vm->_global->inter_variables + var);
	var = _vm->_parse->parseVarIndex();

	WRITE_VAR_OFFSET(var, len);
}

void Inter::strToLong(void) {
	char str[20];
	int16 strVar;
	int16 destVar;
	int32 res;

	strVar = _vm->_parse->parseVarIndex();
	strcpy(str, _vm->_global->inter_variables + strVar);
	res = atol(str);

	destVar = _vm->_parse->parseVarIndex();
	WRITE_VAR_OFFSET(destVar, res);
}

void Inter::invalidate(void) {
	warning("invalidate: 'bugged' function!");
	_vm->_draw->destSurface = load16();
	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_INVALIDATE);
}

void Inter::loadSpriteContent(void) {
	_vm->_draw->spriteLeft = load16();
	_vm->_draw->destSurface = load16();
	_vm->_draw->transparency = load16();
	_vm->_draw->destSpriteX = 0;
	_vm->_draw->destSpriteY = 0;
	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
}

void Inter::copySprite(void) {
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
}

void Inter::putPixel(void) {
	_vm->_draw->destSurface = load16();

	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_PUTPIXEL);
}

void Inter::fillRect(void) {
	_vm->_draw->destSurface = load16();

	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->backColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_FILLRECT);
}

void Inter::drawLine(void) {
	_vm->_draw->destSurface = load16();

	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_DRAWLINE);
}

void Inter::createSprite(void) {
	int16 index;
	int16 height;
	int16 width;
	int16 flag;

	index = load16();
	width = load16();
	height = load16();

	flag = load16();
	if (flag == 1)
		_vm->_draw->spritesArray[index] = _vm->_video->initSurfDesc(_vm->_global->videoMode, width, height, 2);
	else
		_vm->_draw->spritesArray[index] = _vm->_video->initSurfDesc(_vm->_global->videoMode, width, height, 0);

	_vm->_video->clearSurf(_vm->_draw->spritesArray[index]);
}

void Inter::freeSprite(void) {
	int16 index;

	index = load16();
	if (_vm->_draw->spritesArray[index] == 0)
		return;

	_vm->_video->freeSurfDesc(_vm->_draw->spritesArray[index]);
	_vm->_draw->spritesArray[index] = 0;
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

void Inter::playComposition(void) {
	static int16 composition[50];
	int16 i;
	int16 dataVar;
	int16 freqVal;

	dataVar = _vm->_parse->parseVarIndex();
	freqVal = _vm->_parse->parseValExpr();
	for (i = 0; i < 50; i++)
		composition[i] = (int16)VAR_OFFSET(dataVar + i * 4);

	_vm->_snd->playComposition(_vm->_game->soundSamples, composition, freqVal);
}

void Inter::stopSound(void) {
	_vm->_snd->stopSound(_vm->_parse->parseValExpr());
	_soundEndTimeKey = 0;
}

void Inter::playSound(void) {
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
		return;

	if (repCount < 0) {
		if (_vm->_global->soundFlags < 2)
			return;

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
}

void Inter::loadCursor(void) {
	Game::TotResItem *itemPtr;
	int16 width;
	int16 height;
	int32 offset;
	char *dataBuf;
	int16 id;
	int8 index;

	id = load16();
	index = *_vm->_global->inter_execPtr++;
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
}

void Inter::loadSpriteToPos(void) {
	debug(4, "loadSpriteToPos");
	_vm->_draw->spriteLeft = load16();

	_vm->_draw->destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->transparency = _vm->_global->inter_execPtr[0];
	_vm->_draw->destSurface = (_vm->_global->inter_execPtr[0] / 2) - 1;

	if (_vm->_draw->destSurface < 0)
		_vm->_draw->destSurface = 101;
	_vm->_draw->transparency &= 1;
	_vm->_global->inter_execPtr += 2;
	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
}

void Inter::loadTot(void) {
	char buf[20];
	int8 size;
	int16 i;

	debug(4, "loadTot");
	if ((*_vm->_global->inter_execPtr & 0x80) != 0) {
		_vm->_global->inter_execPtr++;
		evalExpr(0);
		strcpy(buf, _vm->_global->inter_resStr);
	} else {
		size = *_vm->_global->inter_execPtr++;
		for (i = 0; i < size; i++)
			buf[i] = *_vm->_global->inter_execPtr++;

		buf[size] = 0;
	}

	strcat(buf, ".tot");
	_terminate = true;
	strcpy(_vm->_game->totToLoad, buf);
}

void Inter::storeKey(int16 key) {
	WRITE_VAR(12, _vm->_util->getTimeKey() - _vm->_game->startTimeKey);

	WRITE_VAR(2, _vm->_global->inter_mouseX);
	WRITE_VAR(3, _vm->_global->inter_mouseY);
	WRITE_VAR(4, _vm->_game->mouseButtons);
	WRITE_VAR(1, _vm->_snd->playingSound);

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

void Inter::keyFunc(void) {
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
				return;
			}

			key = 0;

			if (_vm->_global->pressedKeys[0x48])
				key |= 1;

			if (_vm->_global->pressedKeys[0x50])
				key |= 2;

			if (_vm->_global->pressedKeys[0x4d])
				key |= 4;

			if (_vm->_global->pressedKeys[0x4b])
				key |= 8;

			if (_vm->_global->pressedKeys[0x1c])
				key |= 0x10;

			if (_vm->_global->pressedKeys[0x39])
				key |= 0x20;

			if (_vm->_global->pressedKeys[1])
				key |= 0x40;

			if (_vm->_global->pressedKeys[0x1d])
				key |= 0x80;

			if (_vm->_global->pressedKeys[0x2a])
				key |= 0x100;

			if (_vm->_global->pressedKeys[0x36])
				key |= 0x200;

			if (_vm->_global->pressedKeys[0x38])
				key |= 0x400;

			if (_vm->_global->pressedKeys[0x3b])
				key |= 0x800;

			if (_vm->_global->pressedKeys[0x3c])
				key |= 0x1000;

			if (_vm->_global->pressedKeys[0x3d])
				key |= 0x2000;

			if (_vm->_global->pressedKeys[0x3e])
				key |= 0x4000;

			WRITE_VAR(0, key);
			_vm->_util->waitKey();
			return;
		}
		key = _vm->_game->checkKeys(&_vm->_global->inter_mouseX, &_vm->_global->inter_mouseY, &_vm->_game->mouseButtons, 0);

		storeKey(key);
		return;
	} else {
		key = _vm->_game->checkCollisions(0, 0, 0, 0);
		storeKey(key);

		if (flag == 1)
			return;

		_vm->_util->waitKey();
	}
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
		len = *(int8*)_vm->_global->inter_execPtr++; // must be a signed char typ and char is not default signed on all platforms.

		if (len == -5)
			break;

		for (i = 0; i < len; i++) {
			evalExpr(0);

			if (_terminate)
				return;

			if (_vm->_global->inter_resVal == value) {
				found = 1;
				notFound = 0;
			}
		}

		if (found != 0)
			*ppExec = _vm->_global->inter_execPtr;

		_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr + 2) + 2;
		found = 0;
	} while (len != -5);

	if (len != -5)
		_vm->_global->inter_execPtr++;

	defFlag = *_vm->_global->inter_execPtr;
	defFlag >>= 4;
	if (defFlag != 4)
		return;
	_vm->_global->inter_execPtr++;

	if (notFound)
		*ppExec = _vm->_global->inter_execPtr;

	_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr + 2) + 2;
}

void Inter::repeatUntil(void) {
	char *blockPtr;
	int16 size;
	char flag;

	debug(4, "repeatUntil");
	_nestLevel[0]++;
	blockPtr = _vm->_global->inter_execPtr;

	do {
		_vm->_global->inter_execPtr = blockPtr;
		size = READ_LE_UINT16(_vm->_global->inter_execPtr + 2) + 2;

		funcBlock(1);
		_vm->_global->inter_execPtr = blockPtr + size + 1;
		flag = evalBoolResult();
	} while (flag == 0 && !_breakFlag && !_terminate);

	_nestLevel[0]--;

	if (*_breakFromLevel > -1) {
		_breakFlag = false;
		*_breakFromLevel = -1;
	}
}

void Inter::whileDo(void) {
	char *blockPtr;
	char *savedIP;
	char flag;
	int16 size;

	debug(4, "whileDo");
	_nestLevel[0]++;
	do {
		savedIP = _vm->_global->inter_execPtr;
		flag = evalBoolResult();

		if (_terminate)
			return;

		blockPtr = _vm->_global->inter_execPtr;

		size = READ_LE_UINT16(_vm->_global->inter_execPtr + 2) + 2;

		if (flag != 0) {
			funcBlock(1);
			_vm->_global->inter_execPtr = savedIP;
		} else {
			_vm->_global->inter_execPtr += size;
		}

		if (_breakFlag || _terminate) {
			_vm->_global->inter_execPtr = blockPtr;
			_vm->_global->inter_execPtr += size;
			break;
		}
	} while (flag != 0);

	_nestLevel[0]--;
	if (*_breakFromLevel > -1) {
		_breakFlag = false;
		*_breakFromLevel = -1;
	}
}

void Inter::funcBlock(int16 retFlag) {
	char cmdCount;
	int16 counter;
	byte cmd;
	byte cmd2;
	char *storedIP;
	char *callAddr;
	char boolRes;

	if (_vm->_global->inter_execPtr == 0)
		return;

	_breakFlag = false;
	_vm->_global->inter_execPtr++;
	cmdCount = *_vm->_global->inter_execPtr++;
	_vm->_global->inter_execPtr += 2;

	if (cmdCount == 0) {
		_vm->_global->inter_execPtr = 0;
		return;
	}

	counter = 0;
	do {
		if (_terminate)
			break;

		cmd = (byte)*_vm->_global->inter_execPtr;
		if ((cmd >> 4) >= 12) {
			cmd2 = 16 - (cmd >> 4);
			cmd &= 0xf;
		} else
			cmd2 = 0;

		_vm->_global->inter_execPtr++;
		counter++;

		debug(4, "funcBlock(%d, %d)", cmd2, cmd);

		switch (cmd2) {
		case 0:
			switch (cmd >> 4) {
			case 0:
			case 1:
				storedIP = _vm->_global->inter_execPtr;
				_vm->_global->inter_execPtr = (char *)_vm->_game->totFileData + READ_LE_UINT16(_vm->_global->inter_execPtr);

				if (counter == cmdCount && retFlag == 2)
					return;

				callSub(2);
				_vm->_global->inter_execPtr = storedIP + 2;
				break;

			case 2:
				_vm->_draw->printText();
				break;

			case 3:
				loadCursor();
				break;

			case 5:
				checkSwitchTable(&callAddr);
				storedIP = _vm->_global->inter_execPtr;
				_vm->_global->inter_execPtr = callAddr;

				if (counter == cmdCount && retFlag == 2)
					return;

				funcBlock(0);
				_vm->_global->inter_execPtr = storedIP;
				break;

			case 6:
				repeatUntil();
				break;

			case 7:
				whileDo();
				break;

			case 8:
				boolRes = evalBoolResult();
				if (boolRes != 0) {
					if (counter == cmdCount
					    && retFlag == 2)
						return;

					storedIP = _vm->_global->inter_execPtr;
					funcBlock(0);
					_vm->_global->inter_execPtr = storedIP;

					_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr + 2) + 2;

					debug(5, "cmd = %d", (int16)*_vm->_global->inter_execPtr);
					cmd = (byte)(*_vm->_global->inter_execPtr) >> 4;
					_vm->_global->inter_execPtr++;
					if (cmd != 12)
						break;

					_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr + 2) + 2;
				} else {
					_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr + 2) + 2;

					debug(5, "cmd = %d", (int16)*_vm->_global->inter_execPtr);
					cmd = (byte)(*_vm->_global->inter_execPtr) >> 4;
					_vm->_global->inter_execPtr++;
					if (cmd != 12)
						break;

					if (counter == cmdCount && retFlag == 2)
						return;

					storedIP = _vm->_global->inter_execPtr;
					funcBlock(0);
					_vm->_global->inter_execPtr = storedIP;
					_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr + 2) + 2;
				}
				break;

			case 9:
				evaluateStore();
				break;

			case 10:
				loadSpriteToPos();
				break;
			}
			break;

		case 1:
			switch (cmd) {
			case 1:
				printText();
				break;

			case 2:
				loadTot();
				break;

			case 3:
				_vm->_draw->interPalLoad();
				break;

			case 4:
				keyFunc();
				break;

			case 5:
				capturePush();
				break;

			case 6:
				capturePop();
				break;

			case 7:
				animPalInit();
				break;

			case 14:
				drawOperations();
				break;

			case 15:
				cmdCount = *_vm->_global->inter_execPtr++;
				counter = 0;
				break;
			}
			break;

		case 2:

			switch (cmd) {
			case 0:
				if (retFlag != 2)
					_breakFlag = true;

				_vm->_global->inter_execPtr = 0;
				return;

			case 1:
				renewTimeInVars();
				break;

			case 2:
				_vm->_snd->speakerOn(_vm->_parse->parseValExpr(), -1);
				break;

			case 3:
				_vm->_snd->speakerOff();
				break;

			case 4:
				putPixel();
				break;

			case 5:
				_vm->_goblin->interFunc();
				break;

			case 6:
				createSprite();
				break;

			case 7:
				freeSprite();
				break;
			}
			break;

		case 3:
			switch (cmd) {
			case 0:
				if (retFlag == 1) {
					_breakFlag = true;
					_vm->_global->inter_execPtr = 0;
					return;
				}

				if (*_nestLevel == 0)
					break;

				*_breakFromLevel = *_nestLevel;
				_breakFlag = true;
				_vm->_global->inter_execPtr = 0;
				return;

			case 1:
				loadSpriteContent();
				break;

			case 2:
				copySprite();
				break;

			case 3:
				fillRect();
				break;

			case 4:
				drawLine();
				break;

			case 5:
				strToLong();
				break;

			case 6:
				invalidate();
				break;

			case 7:
				_vm->_draw->backDeltaX = _vm->_parse->parseValExpr();
				_vm->_draw->backDeltaY = _vm->_parse->parseValExpr();
				break;

			case 8:
				playSound();
				break;

			case 9:
				stopSound();
				break;

			case 10:
				_vm->_game->interLoadSound(-1);
				break;

			case 11:
				_vm->_game->freeSoundSlot(-1);
				break;

			case 12:
				_vm->_snd->waitEndPlay();
				break;

			case 13:
				playComposition();
				break;

			case 14:
				getFreeMem();
				break;

			case 15:
				checkData();
				break;
			}
			break;

		case 4:

			switch (cmd) {
			case 1:
				prepareStr();
				break;

			case 2:
				insertStr();
				break;

			case 3:
				cutStr();
				break;

			case 4:
				strstr();
				break;

			case 5:
				istrlen();
				break;

			case 6:
				setMousePos();
				break;

			case 7:
				setFrameRate();
				break;

			case 8:
				_vm->_draw->blitInvalidated();
				_vm->_util->waitEndFrame();
				animPalette();
				storeKey(_vm->_game->checkKeys(&_vm->_global->inter_mouseX,
					&_vm->_global->inter_mouseY, &_vm->_game->mouseButtons, 0));
				break;

			case 9:
				_vm->_draw->animateCursor(1);
				break;

			case 10:
				_vm->_draw->blitCursor();
				break;

			case 11:
				loadFont();
				break;

			case 12:
				freeFont();
				break;

			case 13:
				readData();
				break;

			case 14:
				writeData();
				break;

			case 15:
				manageDataFile();
				break;
			}
			break;

		}

		if (_breakFlag) {
			if (retFlag != 2)
				break;

			if (*_breakFromLevel == -1)
				_breakFlag = false;
			break;
		}
	} while (counter != cmdCount);

	_vm->_global->inter_execPtr = 0;
	return;
}

void Inter::initControlVars(void) {
	*_nestLevel = 0;
	*_breakFromLevel = -1;

	*_vm->_scenery->pCaptureCounter = 0;

	_breakFlag = false;
	_terminate = false;
	_animPalDir = 0;
	_soundEndTimeKey = 0;
}

void Inter::callSub(int16 retFlag) {
	int16 block;
	while (_vm->_global->inter_execPtr != 0 && (char *)_vm->_global->inter_execPtr != _vm->_game->totFileData) {
		block = *_vm->_global->inter_execPtr;
		if (block == 1) {
			funcBlock(retFlag);
		} else if (block == 2) {
			_vm->_game->collisionsBlock();
		}
	}

	if ((char *)_vm->_global->inter_execPtr == _vm->_game->totFileData)
		_terminate = true;
}

} // End of namespace Gob
