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

int16 inter_animPalLowIndex;
int16 inter_animPalHighIndex;
int16 inter_animPalDir;
uint32 inter_soundEndTimeKey;
int16 inter_soundStopVal;
char inter_terminate = 0;
char inter_breakFlag = 0;
int16 *inter_breakFromLevel;
int16 *inter_nestLevel;

int16 inter_load16(void) {
	int16 tmp = (int16)READ_LE_UINT16(inter_execPtr);
	inter_execPtr += 2;
	return tmp;
}

void inter_setMousePos(void) {
	inter_mouseX = parse_parseValExpr();
	inter_mouseY = parse_parseValExpr();
	if (useMouse != 0)
		util_setMousePos(inter_mouseX, inter_mouseY);
}

char inter_evalExpr(int16 *pRes) {
	byte token;

//
	parse_printExpr(99);

	parse_parseExpr(99, &token);
	if (pRes == 0)
		return token;

	switch (token) {
	case 20:
		*pRes = inter_resVal;
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

char inter_evalBoolResult() {
	byte token;

	parse_printExpr(99);

	parse_parseExpr(99, &token);
	if (token == 24 || (token == 20 && inter_resVal != 0))
		return 1;
	else
		return 0;
}

void inter_evaluateStore(void) {
	char *savedPos;
	int16 token;
	int16 result;
	int16 varOff;

	savedPos = inter_execPtr;
	varOff = parse_parseVarIndex();
	token = inter_evalExpr(&result);
	switch (savedPos[0]) {
	case 23:
	case 26:
		WRITE_VAR_OFFSET(varOff, inter_resVal);
		break;

	case 25:
	case 28:
		if (token == 20)
			*(inter_variables + varOff) = result;
		else
			strcpy(inter_variables + varOff, inter_resStr);
		break;

	}
	return;
}

void inter_capturePush(void) {
	int16 left;
	int16 top;
	int16 width;
	int16 height;

	left = parse_parseValExpr();
	top = parse_parseValExpr();
	width = parse_parseValExpr();
	height = parse_parseValExpr();
	game_capturePush(left, top, width, height);
	(*scen_pCaptureCounter)++;
}

void inter_capturePop(void) {
	if (*scen_pCaptureCounter != 0) {
		(*scen_pCaptureCounter)--;
		game_capturePop(1);
	}
}

void inter_printText(void) {
	char buf[60];
	int16 i;

	debug(3, "inter_printText");
	draw_destSpriteX = parse_parseValExpr();
	draw_destSpriteY = parse_parseValExpr();

	draw_backColor = parse_parseValExpr();
	draw_frontColor = parse_parseValExpr();
	draw_fontIndex = parse_parseValExpr();
	draw_destSurface = 21;
	draw_textToPrint = buf;
	draw_transparency = 0;

	if (draw_backColor >= 16) {
		draw_backColor = 0;
		draw_transparency = 1;
	}

	do {
		for (i = 0; *inter_execPtr != '.' && (byte)*inter_execPtr != 200;
			 i++, inter_execPtr++) {
			buf[i] = *inter_execPtr;
		}

		if ((byte)*inter_execPtr != 200) {
			inter_execPtr++;
			switch (*inter_execPtr) {
			case 23:
			case 26:
				sprintf(buf + i, "%d", VAR_OFFSET(parse_parseVarIndex()));
				break;

			case 25:
			case 28:
				sprintf(buf + i, "%s", inter_variables + parse_parseVarIndex());
				break;
			}
			inter_execPtr++;
		} else {
			buf[i] = 0;
		}
		draw_spriteOperation(DRAW_PRINTTEXT);
	} while ((byte)*inter_execPtr != 200);
	inter_execPtr++;
}

void inter_animPalette(void) {
	int16 i;
	Color col;

	if (inter_animPalDir == 0)
		return;

	vid_waitRetrace(videoMode);

	if (inter_animPalDir == -1) {
		col = draw_vgaSmallPalette[inter_animPalLowIndex];

		for (i = inter_animPalLowIndex; i < inter_animPalHighIndex; i++)
			draw_vgaSmallPalette[i] = draw_vgaSmallPalette[i + 1];

		draw_vgaSmallPalette[inter_animPalHighIndex] = col;
	} else {
		col = draw_vgaSmallPalette[inter_animPalHighIndex];
		for (i = inter_animPalHighIndex; i > inter_animPalLowIndex; i--)
			draw_vgaSmallPalette[i] = draw_vgaSmallPalette[i - 1];

		draw_vgaSmallPalette[inter_animPalLowIndex] = col;
	}

	pPaletteDesc->vgaPal = draw_vgaSmallPalette;
	vid_setFullPalette(pPaletteDesc);
}

void inter_animPalInit(void) {
	inter_animPalDir = inter_load16();
	inter_animPalLowIndex = parse_parseValExpr();
	inter_animPalHighIndex = parse_parseValExpr();
}

void inter_loadMult(void) {
	int16 resId;

	resId = inter_load16();
	mult_loadMult(resId);
}

void inter_playMult(void) {
	int16 checkEscape;

	checkEscape = inter_load16();
	mult_playMult(VAR(57), -1, checkEscape, 0);
}

void inter_freeMult(void) {
	inter_load16();		// unused
	mult_freeMultKeys();
}

void inter_initCursor(void) {
	int16 width;
	int16 height;
	int16 count;
	int16 i;

	draw_cursorXDeltaVar = parse_parseVarIndex();
	draw_cursorYDeltaVar = parse_parseVarIndex();

	width = inter_load16();
	if (width < 16)
		width = 16;

	height = inter_load16();
	if (height < 16)
		height = 16;

	count = inter_load16();
	if (count < 2)
		count = 2;

	if (width != draw_cursorWidth || height != draw_cursorHeight ||
	    draw_cursorSprites->width != width * count) {

		vid_freeSurfDesc(draw_cursorSprites);
		vid_freeSurfDesc(draw_cursorBack);

		draw_cursorWidth = width;
		draw_cursorHeight = height;

		if (count < 0x80)
			draw_transparentCursor = 1;
		else
			draw_transparentCursor = 0;

		if (count > 0x80)
			count -= 0x80;

		draw_cursorSprites =
		    vid_initSurfDesc(videoMode, draw_cursorWidth * count,
		    draw_cursorHeight, 2);
		draw_spritesArray[23] = draw_cursorSprites;

		draw_cursorBack =
		    vid_initSurfDesc(videoMode, draw_cursorWidth,
		    draw_cursorHeight, 0);
		for (i = 0; i < 40; i++) {
			draw_cursorAnimLow[i] = -1;
			draw_cursorAnimDelays[i] = 0;
			draw_cursorAnimHigh[i] = 0;
		}
		draw_cursorAnimLow[1] = 0;
	}
}

void inter_initCursorAnim(void) {
	int16 ind;

	ind = parse_parseValExpr();
	draw_cursorAnimLow[ind] = inter_load16();
	draw_cursorAnimHigh[ind] = inter_load16();
	draw_cursorAnimDelays[ind] = inter_load16();
}

void inter_clearCursorAnim(void) {
	int16 ind;

	ind = parse_parseValExpr();
	draw_cursorAnimLow[ind] = -1;
	draw_cursorAnimHigh[ind] = 0;
	draw_cursorAnimDelays[ind] = 0;
}

void inter_drawOperations(void) {
	byte cmd;
	int16 i;

	cmd = *inter_execPtr++;

	debug(4, "inter_drawOperations(%d)", cmd);

	switch (cmd) {
	case 0:
		inter_loadMult();
		break;

	case 1:
		inter_playMult();
		break;

	case 2:
		inter_freeMult();
		break;

	case 7:
		inter_initCursor();
		break;

	case 8:
		inter_initCursorAnim();
		break;

	case 9:
		inter_clearCursorAnim();
		break;

	case 10:
		draw_renderFlags = parse_parseValExpr();
		break;

	case 11:
		//word_23EC_DE = parse_parseValExpr();
		break;

	case 16:
		scen_loadAnim(0);
		break;

	case 17:
		scen_freeAnim(-1);
		break;

	case 18:
		scen_interUpdateAnim();
		break;

	case 19:
		warning("Gob2 mult stub");
		break;

	case 20:
		mult_interInitMult();
		break;

	case 21:
		mult_freeMult();
		break;

	case 22:
		mult_animate();
		break;

	case 23:
		mult_interLoadMult();
		break;

	case 24:
		scen_interStoreParams();
		break;

	case 25:
		mult_interGetObjAnimSize();
		break;

	case 26:
		scen_loadStatic(0);
		break;

	case 27:
		scen_freeStatic(-1);
		break;

	case 28:
		scen_interRenderStatic();
		break;

	case 29:
		scen_interLoadCurLayer();
		break;

	case 32:
		if (_vm->_features & GF_GOB1) {
			// Used in gob1 CD
			inter_evalExpr(0);
			cd_startTrack(inter_resStr);
		} else {
		}
		break;

	case 33:
		if (_vm->_features & GF_GOB1) {
			// Used in gob1 CD

			// Some scripts busy-wait while calling this opcode.
			// This is a very nasty thing to do, so let's add a
			// short delay here. It's probably a safe thing to do.

			util_longDelay(1);

			int pos = cd_getTrackPos();
			if (pos == -1)
				pos = 32767;
			WRITE_VAR(5, pos);
		} else {
		}
		break;

	case 34:
		if (_vm->_features & GF_GOB1) {
			// Used in gob1 CD
			cd_stopPlaying();
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
		i = inter_load16();
		draw_fontToSprite[i].sprite = inter_load16();
		draw_fontToSprite[i].base = inter_load16();
		draw_fontToSprite[i].width = inter_load16();
		draw_fontToSprite[i].height = inter_load16();
		break;

	case 49:
		i = inter_load16();
		draw_fontToSprite[i].sprite = -1;
		draw_fontToSprite[i].base = -1;
		draw_fontToSprite[i].width = -1;
		draw_fontToSprite[i].height = -1;
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

void inter_getFreeMem(void) {
	int16 freeVar;
	int16 maxFreeVar;

	freeVar = parse_parseVarIndex();
	maxFreeVar = parse_parseVarIndex();

	// HACK
	WRITE_VAR_OFFSET(freeVar, 1000000);
	WRITE_VAR_OFFSET(maxFreeVar, 1000000);
}

void inter_manageDataFile(void) {
	inter_evalExpr(0);

	if (inter_resStr[0] != 0)
		data_openDataFile(inter_resStr);
	else
		data_closeDataFile();
}

void inter_writeData(void) {
	int16 offset;
	int16 handle;
	int16 size;
	int16 dataVar;
	int16 retSize;

	debug(4, "inter_writeData");
	inter_evalExpr(0);
	dataVar = parse_parseVarIndex();
	size = parse_parseValExpr();
	offset = parse_parseValExpr();

	WRITE_VAR(1, 1);
	handle = data_openData(inter_resStr, Common::File::kFileWriteMode);

	if (handle < 0)
		return;

	if (offset < 0) {
		data_seekData(handle, -offset - 1, 2);
	} else {
		data_seekData(handle, offset, 0);
	}

	retSize = file_getHandle(handle)->write(inter_variables + dataVar, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	data_closeData(handle);
}

void inter_checkData(void) {
	int16 handle;
	int16 varOff;

	debug(4, "data_cheackData");
	inter_evalExpr(0);
	varOff = parse_parseVarIndex();
	handle = data_openData(inter_resStr);

	WRITE_VAR_OFFSET(varOff, handle);
	if (handle >= 0)
		data_closeData(handle);
}

void inter_readData(void) {
	int16 retSize;
	int16 size;
	int16 dataVar;
	int16 offset;
	int16 handle;

	debug(4, "inter_readData");
	inter_evalExpr(0);
	dataVar = parse_parseVarIndex();
	size = parse_parseValExpr();
	offset = parse_parseValExpr();

	if (game_extHandle >= 0)
		data_closeData(game_extHandle);

	WRITE_VAR(1, 1);
	handle = data_openData(inter_resStr);
	if (handle >= 0) {
		draw_animateCursor(4);
		if (offset < 0)
			data_seekData(handle, -offset - 1, 2);
		else
			data_seekData(handle, offset, 0);

		retSize = data_readData(handle, inter_variables + dataVar, size);
		data_closeData(handle);

		if (retSize == size)
			WRITE_VAR(1, 0);
	}

	if (game_extHandle >= 0)
		game_extHandle = data_openData(game_curExtFile);
}

void inter_loadFont(void) {
	int16 index;

	debug(4, "inter_loadFont");
	inter_evalExpr(0);
	index = inter_load16();

	if (draw_fonts[index] != 0)
		util_freeFont(draw_fonts[index]);

	draw_animateCursor(4);
	if (game_extHandle >= 0)
		data_closeData(game_extHandle);

	draw_fonts[index] = util_loadFont(inter_resStr);

	if (game_extHandle >= 0)
		game_extHandle = data_openData(game_curExtFile);
}

void inter_freeFont(void) {
	int16 index;

	index = inter_load16();
	if (draw_fonts[index] != 0)
		util_freeFont(draw_fonts[index]);

	draw_fonts[index] = 0;
}

void inter_prepareStr(void) {
	int16 var;

	var = parse_parseVarIndex();
	util_prepareStr(inter_variables + var);
}

void inter_insertStr(void) {
	int16 pos;
	int16 strVar;

	strVar = parse_parseVarIndex();
	inter_evalExpr(0);
	pos = parse_parseValExpr();
	util_insertStr(inter_resStr, inter_variables + strVar, pos);
}

void inter_cutStr(void) {
	int16 var;
	int16 pos;
	int16 size;

	var = parse_parseVarIndex();
	pos = parse_parseValExpr();
	size = parse_parseValExpr();
	util_cutFromStr(inter_variables + var, pos, size);
}

void inter_strstr(void) {
	int16 strVar;
	int16 resVar;
	int16 pos;

	strVar = parse_parseVarIndex();
	inter_evalExpr(0);
	resVar = parse_parseVarIndex();

	pos = util_strstr(inter_resStr, inter_variables + strVar);
	WRITE_VAR_OFFSET(resVar, pos - 1);
}

void inter_setFrameRate(void) {
	util_setFrameRate(parse_parseValExpr());
}

void inter_strlen(void) {
	int16 len;
	int16 var;

	var = parse_parseVarIndex();
	len = strlen(inter_variables + var);
	var = parse_parseVarIndex();

	WRITE_VAR_OFFSET(var, len);
}

void inter_strToLong(void) {
	char str[20];
	int16 strVar;
	int16 destVar;
	int32 res;

	strVar = parse_parseVarIndex();
	strcpy(str, inter_variables + strVar);
	res = atol(str);

	destVar = parse_parseVarIndex();
	WRITE_VAR_OFFSET(destVar, res);
}

void inter_invalidate(void) {
	warning("inter_invalidate: 'bugged' function!");
	draw_destSurface = inter_load16();
	draw_destSpriteX = parse_parseValExpr();
	draw_destSpriteY = parse_parseValExpr();
	draw_spriteRight = parse_parseValExpr();
	draw_frontColor = parse_parseValExpr();
	draw_spriteOperation(DRAW_INVALIDATE);
}

void inter_loadSpriteContent(void) {
	draw_spriteLeft = inter_load16();
	draw_destSurface = inter_load16();
	draw_transparency = inter_load16();
	draw_destSpriteX = 0;
	draw_destSpriteY = 0;
	draw_spriteOperation(DRAW_LOADSPRITE);
}

void inter_copySprite(void) {
	draw_sourceSurface = inter_load16();
	draw_destSurface = inter_load16();

	draw_spriteLeft = parse_parseValExpr();
	draw_spriteTop = parse_parseValExpr();
	draw_spriteRight = parse_parseValExpr();
	draw_spriteBottom = parse_parseValExpr();

	draw_destSpriteX = parse_parseValExpr();
	draw_destSpriteY = parse_parseValExpr();

	draw_transparency = inter_load16();
	draw_spriteOperation(DRAW_BLITSURF);
}

void inter_putPixel(void) {
	draw_destSurface = inter_load16();

	draw_destSpriteX = parse_parseValExpr();
	draw_destSpriteY = parse_parseValExpr();
	draw_frontColor = parse_parseValExpr();
	draw_spriteOperation(DRAW_PUTPIXEL);
}

void inter_fillRect(void) {
	draw_destSurface = inter_load16();

	draw_destSpriteX = parse_parseValExpr();
	draw_destSpriteY = parse_parseValExpr();
	draw_spriteRight = parse_parseValExpr();
	draw_spriteBottom = parse_parseValExpr();

	draw_backColor = parse_parseValExpr();
	draw_spriteOperation(DRAW_FILLRECT);
}

void inter_drawLine(void) {
	draw_destSurface = inter_load16();

	draw_destSpriteX = parse_parseValExpr();
	draw_destSpriteY = parse_parseValExpr();
	draw_spriteRight = parse_parseValExpr();
	draw_spriteBottom = parse_parseValExpr();

	draw_frontColor = parse_parseValExpr();
	draw_spriteOperation(DRAW_DRAWLINE);
}

void inter_createSprite(void) {
	int16 index;
	int16 height;
	int16 width;
	int16 flag;

	index = inter_load16();
	width = inter_load16();
	height = inter_load16();

	flag = inter_load16();
	if (flag == 1)
		draw_spritesArray[index] = vid_initSurfDesc(videoMode, width, height, 2);
	else
		draw_spritesArray[index] = vid_initSurfDesc(videoMode, width, height, 0);

	vid_clearSurf(draw_spritesArray[index]);
}

void inter_freeSprite(void) {
	int16 index;

	index = inter_load16();
	if (draw_spritesArray[index] == 0)
		return;

	vid_freeSurfDesc(draw_spritesArray[index]);
	draw_spritesArray[index] = 0;
}

void inter_renewTimeInVars(void) {
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

void inter_playComposition(void) {
	static int16 inter_composition[50];
	int16 i;
	int16 dataVar;
	int16 freqVal;

	dataVar = parse_parseVarIndex();
	freqVal = parse_parseValExpr();
	for (i = 0; i < 50; i++)
		inter_composition[i] = (int16)VAR_OFFSET(dataVar + i * 4);

	snd_playComposition(game_soundSamples, inter_composition, freqVal);
}

void inter_stopSound(void) {
	snd_stopSound(parse_parseValExpr());
	inter_soundEndTimeKey = 0;
}

void inter_playSound(void) {
	int16 frequency;
	int16 freq2;
	int16 repCount;
	int16 index;

	index = parse_parseValExpr();
	repCount = parse_parseValExpr();
	frequency = parse_parseValExpr();

	snd_stopSound(0);
	inter_soundEndTimeKey = 0;
	if (game_soundSamples[index] == 0)
		return;

	if (repCount < 0) {
		if (soundFlags < 2)
			return;

		repCount = -repCount;
		inter_soundEndTimeKey = util_getTimeKey();

		if (frequency == 0) {
			freq2 = game_soundSamples[index]->frequency;
		} else {
			freq2 = frequency;
		}
		inter_soundStopVal =
		    (10 * (game_soundSamples[index]->size / 2)) / freq2;
		inter_soundEndTimeKey +=
		    ((game_soundSamples[index]->size * repCount -
			game_soundSamples[index]->size / 2) * 1000) / freq2;
	}
	snd_playSample(game_soundSamples[index], repCount, frequency);
}

void inter_loadCursor(void) {
	Game_TotResItem *itemPtr;
	int16 width;
	int16 height;
	int32 offset;
	char *dataBuf;
	int16 id;
	int8 index;

	id = inter_load16();
	index = *inter_execPtr++;
	itemPtr = &game_totResourceTable->items[id];
	offset = itemPtr->offset;

	if (offset >= 0) {
		dataBuf =
		    ((char *)game_totResourceTable) + szGame_TotResTable +
		    szGame_TotResItem * game_totResourceTable->itemsCount + offset;
	} else {
		dataBuf = game_imFileData + (int32)READ_LE_UINT32(&((int32 *)game_imFileData)[-offset - 1]);
	}

	width = itemPtr->width;
	height = itemPtr->height;

	vid_fillRect(draw_cursorSprites, index * draw_cursorWidth, 0,
	    index * draw_cursorWidth + draw_cursorWidth - 1,
	    draw_cursorHeight - 1, 0);

	vid_drawPackedSprite((byte*)dataBuf, width, height,
	    index * draw_cursorWidth, 0, 0, draw_cursorSprites);
	draw_cursorAnimLow[index] = 0;
}

void inter_loadSpriteToPos(void) {
	debug(4, "inter_loadSpriteToPos");
	draw_spriteLeft = inter_load16();

	draw_destSpriteX = parse_parseValExpr();
	draw_destSpriteY = parse_parseValExpr();

	draw_transparency = inter_execPtr[0];
	draw_destSurface = (inter_execPtr[0] / 2) - 1;

	if (draw_destSurface < 0)
		draw_destSurface = 101;
	draw_transparency &= 1;
	inter_execPtr += 2;
	draw_spriteOperation(DRAW_LOADSPRITE);
}

void inter_loadTot(void) {
	char buf[20];
	int8 size;
	int16 i;

	debug(4, "inter_loadTot");
	if ((*inter_execPtr & 0x80) != 0) {
		inter_execPtr++;
		inter_evalExpr(0);
		strcpy(buf, inter_resStr);
	} else {
		size = *inter_execPtr++;
		for (i = 0; i < size; i++)
			buf[i] = *inter_execPtr++;

		buf[size] = 0;
	}

	strcat(buf, ".tot");
	inter_terminate = 1;
	strcpy(game_totToLoad, buf);
}

void inter_storeKey(int16 key) {
	WRITE_VAR(12, util_getTimeKey() - game_startTimeKey);

	WRITE_VAR(2, inter_mouseX);
	WRITE_VAR(3, inter_mouseY);
	WRITE_VAR(4, game_mouseButtons);
	WRITE_VAR(1, snd_playingSound);

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
		util_waitKey();
}

void inter_keyFunc(void) {
	int16 flag;
	int16 key;

	debug(4, "inter_keyFunc");
	flag = inter_load16();
	inter_animPalette();
	draw_blitInvalidated();

	if (flag != 0) {

		if (flag != 1) {
			if (flag != 2) {
				util_longDelay(flag);
				return;
			}

			key = 0;

			if (pressedKeys[0x48])
				key |= 1;

			if (pressedKeys[0x50])
				key |= 2;

			if (pressedKeys[0x4d])
				key |= 4;

			if (pressedKeys[0x4b])
				key |= 8;

			if (pressedKeys[0x1c])
				key |= 0x10;

			if (pressedKeys[0x39])
				key |= 0x20;

			if (pressedKeys[1])
				key |= 0x40;

			if (pressedKeys[0x1d])
				key |= 0x80;

			if (pressedKeys[0x2a])
				key |= 0x100;

			if (pressedKeys[0x36])
				key |= 0x200;

			if (pressedKeys[0x38])
				key |= 0x400;

			if (pressedKeys[0x3b])
				key |= 0x800;

			if (pressedKeys[0x3c])
				key |= 0x1000;

			if (pressedKeys[0x3d])
				key |= 0x2000;

			if (pressedKeys[0x3e])
				key |= 0x4000;

			WRITE_VAR(0, key);
			util_waitKey();
			return;
		}
		key = game_checkKeys(&inter_mouseX, &inter_mouseY, &game_mouseButtons, 0);

		inter_storeKey(key);
		return;
	} else {
		key = game_checkCollisions(0, 0, 0, 0);
		inter_storeKey(key);

		if (flag == 1)
			return;

		util_waitKey();
	}
}

void inter_checkSwitchTable(char **ppExec) {
	int16 i;
	int16 len;
	char found;
	int32 value;
	char notFound;
	char defFlag;

	found = 0;
	notFound = 1;
	*ppExec = 0;
	value = parse_parseVarIndex();
	value = VAR_OFFSET(value);

	do {
		len = *(int8*)inter_execPtr++; // must be a signed char typ and char is not default signed on all platforms.

		if (len == -5)
			break;

		for (i = 0; i < len; i++) {
			inter_evalExpr(0);

			if (inter_terminate != 0)
				return;

			if (inter_resVal == value) {
				found = 1;
				notFound = 0;
			}
		}

		if (found != 0)
			*ppExec = inter_execPtr;

		inter_execPtr += READ_LE_UINT16(inter_execPtr + 2) + 2;
		found = 0;
	} while (len != -5);

	if (len != -5)
		inter_execPtr++;

	defFlag = *inter_execPtr;
	defFlag >>= 4;
	if (defFlag != 4)
		return;
	inter_execPtr++;

	if (notFound)
		*ppExec = inter_execPtr;

	inter_execPtr += READ_LE_UINT16(inter_execPtr + 2) + 2;
}

void inter_repeatUntil(void) {
	char *blockPtr;
	int16 size;
	char flag;

	debug(4, "inter_repeatUntil");
	inter_nestLevel[0]++;
	blockPtr = inter_execPtr;

	do {
		inter_execPtr = blockPtr;
		size = READ_LE_UINT16(inter_execPtr + 2) + 2;

		inter_funcBlock(1);
		inter_execPtr = blockPtr + size + 1;
		flag = inter_evalBoolResult();
	} while (flag == 0 && inter_breakFlag == 0 && inter_terminate == 0);

	inter_nestLevel[0]--;

	if (*inter_breakFromLevel > -1) {
		inter_breakFlag = 0;
		*inter_breakFromLevel = -1;
	}
}

void inter_whileDo(void) {
	char *blockPtr;
	char *savedIP;
	char flag;
	int16 size;

	debug(4, "inter_whileDo");
	inter_nestLevel[0]++;
	do {
		savedIP = inter_execPtr;
		flag = inter_evalBoolResult();

		if (inter_terminate != 0)
			return;

		blockPtr = inter_execPtr;

		size = READ_LE_UINT16(inter_execPtr + 2) + 2;

		if (flag != 0) {
			inter_funcBlock(1);
			inter_execPtr = savedIP;
		} else {
			inter_execPtr += size;
		}

		if (inter_breakFlag != 0 || inter_terminate != 0) {
			inter_execPtr = blockPtr;
			inter_execPtr += size;
			break;
		}
	} while (flag != 0);

	inter_nestLevel[0]--;
	if (*inter_breakFromLevel > -1) {
		inter_breakFlag = 0;
		*inter_breakFromLevel = -1;
	}
}

void inter_funcBlock(int16 retFlag) {
	char cmdCount;
	int16 counter;
	byte cmd;
	byte cmd2;
	char *storedIP;
	char *callAddr;
	char boolRes;

	if (inter_execPtr == 0)
		return;

	inter_breakFlag = 0;
	inter_execPtr++;
	cmdCount = *inter_execPtr++;
	inter_execPtr += 2;

	if (cmdCount == 0) {
		inter_execPtr = 0;
		return;
	}

	counter = 0;
	do {
		if (inter_terminate != 0)
			break;

		cmd = (byte)*inter_execPtr;
		if ((cmd >> 4) >= 12) {
			cmd2 = 16 - (cmd >> 4);
			cmd &= 0xf;
		} else
			cmd2 = 0;

		inter_execPtr++;
		counter++;

		debug(4, "inter_funcBlock(%d, %d)", cmd2, cmd);

		switch (cmd2) {
		case 0:
			switch (cmd >> 4) {
			case 0:
			case 1:
				storedIP = inter_execPtr;
				inter_execPtr = (char *)game_totFileData + READ_LE_UINT16(inter_execPtr);

				if (counter == cmdCount && retFlag == 2)
					return;

				inter_callSub(2);
				inter_execPtr = storedIP + 2;
				break;

			case 2:
				draw_printText();
				break;

			case 3:
				inter_loadCursor();
				break;

			case 5:
				inter_checkSwitchTable(&callAddr);
				storedIP = inter_execPtr;
				inter_execPtr = callAddr;

				if (counter == cmdCount && retFlag == 2)
					return;

				inter_funcBlock(0);
				inter_execPtr = storedIP;
				break;

			case 6:
				inter_repeatUntil();
				break;

			case 7:
				inter_whileDo();
				break;

			case 8:
				boolRes = inter_evalBoolResult();
				if (boolRes != 0) {
					if (counter == cmdCount
					    && retFlag == 2)
						return;

					storedIP = inter_execPtr;
					inter_funcBlock(0);
					inter_execPtr = storedIP;

					inter_execPtr += READ_LE_UINT16(inter_execPtr + 2) + 2;

					debug(5, "cmd = %d", (int16)*inter_execPtr);
					cmd = (byte)(*inter_execPtr) >> 4;
					inter_execPtr++;
					if (cmd != 12)
						break;

					inter_execPtr += READ_LE_UINT16(inter_execPtr + 2) + 2;
				} else {
					inter_execPtr += READ_LE_UINT16(inter_execPtr + 2) + 2;

					debug(5, "cmd = %d", (int16)*inter_execPtr);
					cmd = (byte)(*inter_execPtr) >> 4;
					inter_execPtr++;
					if (cmd != 12)
						break;

					if (counter == cmdCount && retFlag == 2)
						return;

					storedIP = inter_execPtr;
					inter_funcBlock(0);
					inter_execPtr = storedIP;
					inter_execPtr += READ_LE_UINT16(inter_execPtr + 2) + 2;
				}
				break;

			case 9:
				inter_evaluateStore();
				break;

			case 10:
				inter_loadSpriteToPos();
				break;
			}
			break;

		case 1:
			switch (cmd) {
			case 1:
				inter_printText();
				break;

			case 2:
				inter_loadTot();
				break;

			case 3:
				draw_interPalLoad();
				break;

			case 4:
				inter_keyFunc();
				break;

			case 5:
				inter_capturePush();
				break;

			case 6:
				inter_capturePop();
				break;

			case 7:
				inter_animPalInit();
				break;

			case 14:
				inter_drawOperations();
				break;

			case 15:
				cmdCount = *inter_execPtr++;
				counter = 0;
				break;
			}
			break;

		case 2:

			switch (cmd) {
			case 0:
				if (retFlag != 2)
					inter_breakFlag = 1;

				inter_execPtr = 0;
				return;

			case 1:
				inter_renewTimeInVars();
				break;

			case 2:
				snd_speakerOn(parse_parseValExpr(), -1);
				break;

			case 3:
				snd_speakerOff();
				break;

			case 4:
				inter_putPixel();
				break;

			case 5:
				gob_interFunc();
				break;

			case 6:
				inter_createSprite();
				break;

			case 7:
				inter_freeSprite();
				break;
			}
			break;

		case 3:
			switch (cmd) {
			case 0:
				if (retFlag == 1) {
					inter_breakFlag = 1;
					inter_execPtr = 0;
					return;
				}

				if (*inter_nestLevel == 0)
					break;

				*inter_breakFromLevel = *inter_nestLevel;
				inter_breakFlag = 1;
				inter_execPtr = 0;
				return;

			case 1:
				inter_loadSpriteContent();
				break;

			case 2:
				inter_copySprite();
				break;

			case 3:
				inter_fillRect();
				break;

			case 4:
				inter_drawLine();
				break;

			case 5:
				inter_strToLong();
				break;

			case 6:
				inter_invalidate();
				break;

			case 7:
				draw_backDeltaX = parse_parseValExpr();
				draw_backDeltaY = parse_parseValExpr();
				break;

			case 8:
				inter_playSound();
				break;

			case 9:
				inter_stopSound();
				break;

			case 10:
				game_interLoadSound(-1);
				break;

			case 11:
				game_freeSoundSlot(-1);
				break;

			case 12:
				snd_waitEndPlay();
				break;

			case 13:
				inter_playComposition();
				break;

			case 14:
				inter_getFreeMem();
				break;

			case 15:
				inter_checkData();
				break;
			}
			break;

		case 4:

			switch (cmd) {
			case 1:
				inter_prepareStr();
				break;

			case 2:
				inter_insertStr();
				break;

			case 3:
				inter_cutStr();
				break;

			case 4:
				inter_strstr();
				break;

			case 5:
				inter_strlen();
				break;

			case 6:
				inter_setMousePos();
				break;

			case 7:
				inter_setFrameRate();
				break;

			case 8:
				draw_blitInvalidated();
				util_waitEndFrame();
				inter_animPalette();
				inter_storeKey(game_checkKeys(&inter_mouseX,
					&inter_mouseY, &game_mouseButtons, 0));
				break;

			case 9:
				draw_animateCursor(1);
				break;

			case 10:
				draw_blitCursor();
				break;

			case 11:
				inter_loadFont();
				break;

			case 12:
				inter_freeFont();
				break;

			case 13:
				inter_readData();
				break;

			case 14:
				inter_writeData();
				break;

			case 15:
				inter_manageDataFile();
				break;
			}
			break;

		}

		if (inter_breakFlag != 0) {
			if (retFlag != 2)
				break;

			if (*inter_breakFromLevel == -1)
				inter_breakFlag = 0;
			break;
		}
	} while (counter != cmdCount);

	inter_execPtr = 0;
	return;
}

void inter_initControlVars(void) {
	*inter_nestLevel = 0;
	*inter_breakFromLevel = -1;

	*scen_pCaptureCounter = 0;

	inter_breakFlag = 0;
	inter_terminate = 0;
	inter_animPalDir = 0;
	inter_soundEndTimeKey = 0;
}

void inter_callSub(int16 retFlag) {
	int16 block;
	while (inter_execPtr != 0 && (char *)inter_execPtr != game_totFileData) {
		block = *inter_execPtr;
		if (block == 1) {
			inter_funcBlock(retFlag);
		} else if (block == 2) {
			game_collisionsBlock();
		}
	}

	if ((char *)inter_execPtr == game_totFileData)
		inter_terminate = 1;
}

} // End of namespace Gob
