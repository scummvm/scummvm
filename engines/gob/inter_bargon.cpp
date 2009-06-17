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

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/palanim.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODE(x) _OPCODE(Inter_Bargon, x)
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(Inter_Bargon, x)

const int Inter_Bargon::_goblinFuncLookUp[][2] = {
	{1, 0},
	{2, 1},
	{3, 2},
	{4, 3},
	{5, 4},
	{6, 5},
	{7, 6},
	{8, 7},
	{9, 8},
	{10, 9},
	{11, 10},
	{13, 11},
	{14, 12},
	{15, 13},
	{16, 14},
	{21, 15},
	{22, 16},
	{23, 17},
	{24, 18},
	{25, 19},
	{26, 20},
	{27, 21},
	{28, 22},
	{29, 23},
	{30, 24},
	{32, 25},
	{33, 26},
	{34, 27},
	{35, 28},
	{36, 29},
	{37, 30},
	{40, 31},
	{41, 32},
	{42, 33},
	{43, 34},
	{44, 35},
	{50, 36},
	{52, 37},
	{53, 38},
	{100, 39},
	{152, 40},
	{200, 41},
	{201, 42},
	{202, 43},
	{203, 44},
	{204, 45},
	{250, 46},
	{251, 47},
	{252, 48},
	{500, 49},
	{502, 50},
	{503, 51},
	{600, 52},
	{601, 53},
	{602, 54},
	{603, 55},
	{604, 56},
	{605, 57},
	{1000, 58},
	{1001, 59},
	{1002, 60},
	{1003, 61},
	{1004, 62},
	{1005, 63},
	{1006, 64},
	{1008, 65},
	{1009, 66},
	{1010, 67},
	{1011, 68},
	{1015, 69},
	{2005, 70}
};

Inter_Bargon::Inter_Bargon(GobEngine *vm) : Inter_v2(vm) {
	setupOpcodes();
	NsetupOpcodes();
}

void Inter_Bargon::setupOpcodesDraw() {
	Inter_v2::setupOpcodesDraw();
}

void Inter_Bargon::setupOpcodes() {
	static const OpcodeFuncEntryBargon opcodesFunc[80] = {
		/* 00 */
		OPCODE(o1_callSub),
		OPCODE(o1_callSub),
		OPCODE(o1_printTotText),
		OPCODE(o1_loadCursor),
		/* 04 */
		{0, ""},
		OPCODE(o1_switch),
		OPCODE(o1_repeatUntil),
		OPCODE(o1_whileDo),
		/* 08 */
		OPCODE(o1_if),
		OPCODE(o2_assign),
		OPCODE(o1_loadSpriteToPos),
		{0, ""},
		/* 0C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 10 */
		{0, ""},
		OPCODE(o2_printText),
		OPCODE(o1_loadTot),
		OPCODE(o1_palLoad),
		/* 14 */
		OPCODE(o1_keyFunc),
		OPCODE(o1_capturePush),
		OPCODE(o1_capturePop),
		OPCODE(o2_animPalInit),
		/* 18 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 1C */
		{0, ""},
		{0, ""},
		OPCODE(o1_drawOperations),
		OPCODE(o1_setcmdCount),
		/* 20 */
		OPCODE(o1_return),
		OPCODE(o1_renewTimeInVars),
		OPCODE(o1_speakerOn),
		OPCODE(o1_speakerOff),
		/* 24 */
		OPCODE(o1_putPixel),
		OPCODE(o2_goblinFunc),
		OPCODE(o1_createSprite),
		OPCODE(o1_freeSprite),
		/* 28 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 2C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
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
		OPCODE(o2_stopSound),
		OPCODE(o2_loadSound),
		OPCODE(o1_freeSoundSlot),
		/* 3C */
		OPCODE(o1_waitEndPlay),
		OPCODE(o1_playComposition),
		OPCODE(o2_getFreeMem),
		OPCODE(o2_checkData),
		/* 40 */
		{0, ""},
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
		OPCODE(o2_readData),
		OPCODE(o2_writeData),
		OPCODE(o1_manageDataFile),
	};

	static const OpcodeGoblinEntryBargon opcodesGoblin[71] = {
		/* 00 */
		OPCODE(oBargon_intro0),
		OPCODE(oBargon_intro1),
		OPCODE(oBargon_intro2),
		OPCODE(oBargon_intro3),
		/* 04 */
		OPCODE(oBargon_intro4),
		OPCODE(oBargon_intro5),
		OPCODE(oBargon_intro6),
		OPCODE(oBargon_intro7),
		/* 08 */
		OPCODE(oBargon_intro8),
		OPCODE(oBargon_intro9),
		OPCODE(o_gobNOP),
		{0, ""},
		/* 0C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 10 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 14 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 18 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 1C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 20 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 24 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 28 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 2C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 30 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 34 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 38 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 3C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 40 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 44 */
		{0, ""},
		{0, ""},
		{0, ""},
	};

	_opcodesFuncBargon = opcodesFunc;
	_opcodesGoblinBargon = opcodesGoblin;
}

bool Inter_Bargon::executeFuncOpcode(byte i, byte j, OpFuncParams &params) {
	debugC(1, kDebugFuncOp, "opcodeFunc %d.%d [0x%X.0x%X] (%s)",
			i, j, i, j, getOpcodeFuncDesc(i, j));

	if ((i > 4) || (j > 15)) {
		warning("unimplemented opcodeFunc: %d.%d", i, j);
		return false;
	}

	OpcodeFuncProcBargon op = _opcodesFuncBargon[i*16 + j].proc;

	if (op == 0)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (params);

	return false;
}

void Inter_Bargon::executeGoblinOpcode(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
			i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcBargon op = 0;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinBargon[_goblinFuncLookUp[j][1]].proc;
			break;
		}

	if (op == 0) {
		int16 val;

		_vm->_global->_inter_execPtr -= 2;
		val = load16();
		_vm->_global->_inter_execPtr += val << 1;
		warning("unimplemented opcodeGob: %d", i);
	} else
		(this->*op) (params);
}

const char *Inter_Bargon::getOpcodeFuncDesc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	return _opcodesFuncBargon[i*16 + j].desc;
}

const char *Inter_Bargon::getOpcodeGoblinDesc(int i) {
	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i)
			return _opcodesGoblinBargon[_goblinFuncLookUp[j][1]].desc;
	return "";
}

void Inter_Bargon::oBargon_intro0(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scaa", 0, 160)) {
		_vm->_vidPlayer->primaryPlay(0, 92, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro1(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scaa", 0, 160)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0, 0, 0, true, 23);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro2(OpGobParams &params) {
	int i;
	int16 mouseX;
	int16 mouseY;
	int16 buttons;
	SurfaceDescPtr surface;
	SoundDesc samples[4];
	int16 comp[5] = { 0, 1, 2, 3, -1 };
	static const char *sndFiles[] = {"1INTROII.snd", "2INTROII.snd", "1INTRO3.snd", "2INTRO3.snd"};

	surface = _vm->_video->initSurfDesc(_vm->_global->_videoMode, 320, 200, 0);
	_vm->_video->drawPackedSprite("2ille.ims", *surface);
	_vm->_video->drawSprite(*surface, *_vm->_draw->_frontSurface, 0, 0, 319, 199, 0, 0, 0);
	_vm->_video->drawPackedSprite("2ille4.ims", *surface);
	_vm->_video->drawSprite(*surface, *_vm->_draw->_frontSurface, 0, 0, 319, 199, 320, 0, 0);
	_vm->_util->setScrollOffset(320, 0);
	_vm->_video->dirtyRectsAll();
	_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
	_vm->_util->longDelay(1000);
	for (i = 320; i >= 0; i--) {
		_vm->_util->setScrollOffset(i, 0);
		_vm->_video->dirtyRectsAll();
		if ((_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B) ||
				_vm->shouldQuit()) {
			_vm->_palAnim->fade(0, -2, 0);
			_vm->_video->clearSurf(*_vm->_draw->_frontSurface);
			memset((char *) _vm->_draw->_vgaPalette, 0, 768);
			WRITE_VAR(4, buttons);
			WRITE_VAR(0, 0x11B);
			WRITE_VAR(57, (uint32) -1);
			break;
		}
	}
	if (!_vm->shouldQuit()) {
		_vm->_util->setScrollOffset(0, 0);
		_vm->_video->dirtyRectsAll();
	}
	surface.reset();
	if (VAR(57) == ((uint32) -1))
		return;

	for (i = 0; i < 4; i++)
		_vm->_sound->sampleLoad(&samples[i], SOUND_SND, sndFiles[i]);
	_vm->_sound->blasterPlayComposition(comp, 0, samples, 4);
	_vm->_sound->blasterWaitEndPlay(true, false);
	_vm->_palAnim->fade(0, 0, 0);
	_vm->_video->clearSurf(*_vm->_draw->_frontSurface);
}

void Inter_Bargon::oBargon_intro3(OpGobParams &params) {
	int16 mouseX;
	int16 mouseY;
	int16 buttons;
	Video::Color *palBak;
	SoundDesc samples[2];
	int16 comp[3] = { 0, 1, -1 };
	byte *palettes[4];
	static const char *sndFiles[] = {"1INTROIV.snd", "2INTROIV.snd"};
	static const char *palFiles[] = {"2ou2.clt", "2ou3.clt", "2ou4.clt", "2ou5.clt"};

	for (int i = 0; i < 2; i++)
		_vm->_sound->sampleLoad(&samples[i], SOUND_SND, sndFiles[i]);
	for (int i = 0; i < 4; i++)
		palettes[i] = _vm->_dataIO->getData(palFiles[i]);
	palBak = _vm->_global->_pPaletteDesc->vgaPal;

	_vm->_sound->blasterPlayComposition(comp, 0, samples, 2);
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 4; j++) {
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *) palettes[j];
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			_vm->_util->longDelay(_vm->_util->getRandom(200));
		}
		if ((_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B) ||
				_vm->shouldQuit()) {
			_vm->_sound->blasterStop(10);
			_vm->_palAnim->fade(0, -2, 0);
			_vm->_video->clearSurf(*_vm->_draw->_frontSurface);
			memset(_vm->_draw->_vgaPalette, 0, 768);
			WRITE_VAR(4, buttons);
			WRITE_VAR(0, 0x11B);
			WRITE_VAR(57, (uint32) -1);
			break;
		}
	}
	_vm->_sound->blasterWaitEndPlay(false, false);

	_vm->_global->_pPaletteDesc->vgaPal = palBak;
	for (int i = 0; i < 4; i++)
		delete[] palettes[i];
}

void Inter_Bargon::oBargon_intro4(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scba", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0, 0, 0, true);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro5(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbb", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro6(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbc", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro7(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbf", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro8(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbc", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro9(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbd", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

} // End of namespace Gob
