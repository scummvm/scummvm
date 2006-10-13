/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

// Video script opcodes for Simon1/Simon2
#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"
#include "agos/vga.h"

#include "common/system.h"

namespace AGOS {

// Opcode tables
void AGOSEngine::setupCommonVideoOpcodes(VgaOpcodeProc *op) {
	op[1] = &AGOSEngine::vc1_fadeOut;
	op[2] = &AGOSEngine::vc2_call;
	op[3] = &AGOSEngine::vc3_loadSprite;
	op[4] = &AGOSEngine::vc4_fadeIn;
	op[5] = &AGOSEngine::vc5_skip_if_neq;
	op[6] = &AGOSEngine::vc6_skip_ifn_sib_with_a;
	op[7] = &AGOSEngine::vc7_skip_if_sib_with_a;
	op[8] = &AGOSEngine::vc8_skip_if_parent_is;
	op[9] = &AGOSEngine::vc9_skip_if_unk3_is;
	op[10] = &AGOSEngine::vc10_draw;
	op[12] = &AGOSEngine::vc12_delay;
	op[13] = &AGOSEngine::vc13_addToSpriteX;
	op[14] = &AGOSEngine::vc14_addToSpriteY;
	op[15] = &AGOSEngine::vc15_sync;
	op[16] = &AGOSEngine::vc16_waitSync;
	op[18] = &AGOSEngine::vc18_jump;
	op[20] = &AGOSEngine::vc20_setRepeat;
	op[21] = &AGOSEngine::vc21_endRepeat;
	op[23] = &AGOSEngine::vc23_setSpritePriority;
	op[24] = &AGOSEngine::vc24_setSpriteXY;
	op[25] = &AGOSEngine::vc25_halt_sprite;
	op[26] = &AGOSEngine::vc26_setSubWindow;
	op[27] = &AGOSEngine::vc27_resetSprite;
	op[29] = &AGOSEngine::vc29_stopAllSounds;
	op[30] = &AGOSEngine::vc30_setFrameRate;
	op[31] = &AGOSEngine::vc31_setWindow;
	op[33] = &AGOSEngine::vc33_setMouseOn;
	op[34] = &AGOSEngine::vc34_setMouseOff;
	op[35] = &AGOSEngine::vc35_clearWindow;
	op[36] = &AGOSEngine::vc36_setWindowImage;
	op[38] = &AGOSEngine::vc38_skipIfVarZero;
	op[39] = &AGOSEngine::vc39_setVar;
	op[40] = &AGOSEngine::vc40;
	op[41] = &AGOSEngine::vc41;
	op[42] = &AGOSEngine::vc42_delayIfNotEQ;
	op[43] = &AGOSEngine::vc43_skipIfBitClear;
	op[44] = &AGOSEngine::vc44_skipIfBitSet;
	op[45] = &AGOSEngine::vc45_setSpriteX;
	op[46] = &AGOSEngine::vc46_setSpriteY;
	op[47] = &AGOSEngine::vc47_addToVar;
	op[49] = &AGOSEngine::vc49_setBit;
	op[50] = &AGOSEngine::vc50_clearBit;
	op[51] = &AGOSEngine::vc51_enableBox;
	op[55] = &AGOSEngine::vc55_moveBox;
	op[59] = &AGOSEngine::vc59;
}

void AGOSEngine::setupElvira1VideoOpcodes(VgaOpcodeProc *op) {
	op[1] = &AGOSEngine::vc1_fadeOut;
	op[2] = &AGOSEngine::vc2_call;
	op[3] = &AGOSEngine::vc3_loadSprite;
	op[4] = &AGOSEngine::vc4_fadeIn;
	op[5] = &AGOSEngine::vc5_skip_if_neq;
	op[6] = &AGOSEngine::vc6_skip_ifn_sib_with_a;
	op[7] = &AGOSEngine::vc7_skip_if_sib_with_a;
	op[8] = &AGOSEngine::vc8_skip_if_parent_is;
	op[9] = &AGOSEngine::vc9_skip_if_unk3_is;
	op[10] = &AGOSEngine::vc10_draw;

	op[13] = &AGOSEngine::vc12_delay;
	op[14] = &AGOSEngine::vc13_addToSpriteX;
	op[15] = &AGOSEngine::vc14_addToSpriteY;
	op[16] = &AGOSEngine::vc15_sync;
	op[17] = &AGOSEngine::vc16_waitSync;
	op[18] = &AGOSEngine::vc17_waitEnd;
	op[19] = &AGOSEngine::vc18_jump;
	op[20] = &AGOSEngine::vc19_loop;
	op[21] = &AGOSEngine::vc20_setRepeat;
	op[22] = &AGOSEngine::vc21_endRepeat;
	op[23] = &AGOSEngine::vc22_setSpritePaletteOld;
	op[24] = &AGOSEngine::vc23_setSpritePriority;
	op[25] = &AGOSEngine::vc24_setSpriteXY;
	op[26] = &AGOSEngine::vc25_halt_sprite;
	op[27] = &AGOSEngine::vc26_setSubWindow;
	op[28] = &AGOSEngine::vc27_resetSprite;
	op[29] = &AGOSEngine::vc28_playSFX;
	op[30] = &AGOSEngine::vc29_stopAllSounds;
	op[31] = &AGOSEngine::vc30_setFrameRate;
	op[32] = &AGOSEngine::vc31_setWindow;
	op[33] = &AGOSEngine::vc32_saveScreen;
	op[34] = &AGOSEngine::vc33_setMouseOn;
	op[35] = &AGOSEngine::vc34_setMouseOff;

	op[38] = &AGOSEngine::vc35_clearWindow;

	op[40] = &AGOSEngine::vc36_setWindowImage;
	op[41] = &AGOSEngine::vc37_pokePalette;

	op[51] = &AGOSEngine::vc38_skipIfVarZero;
	op[52] = &AGOSEngine::vc39_setVar;
	op[53] = &AGOSEngine::vc40;
	op[54] = &AGOSEngine::vc41;
}

void AGOSEngine::setupElvira2VideoOpcodes(VgaOpcodeProc *op) {
	setupCommonVideoOpcodes(op);

	op[11] = &AGOSEngine::vc11;
	op[17] = &AGOSEngine::vc17_waitEnd;
	op[19] = &AGOSEngine::vc19_loop;
	op[22] = &AGOSEngine::vc22_setSpritePaletteOld;
	op[28] = &AGOSEngine::vc28_playSFX;
	op[32] = &AGOSEngine::vc32_saveScreen;
	op[37] = &AGOSEngine::vc37_pokePalette;
	op[45] = &AGOSEngine::vc45;
	op[46] = &AGOSEngine::vc46;
	op[47] = &AGOSEngine::vc47;
	op[48] = &AGOSEngine::vc48;
	op[53] = &AGOSEngine::vc53_dissolveIn;
	op[54] = &AGOSEngine::vc54_dissolveOut;
	op[57] = &AGOSEngine::vc57_blackPalette;
	op[56] = &AGOSEngine::vc56_fullScreen;
}

void AGOSEngine::setupWaxworksVideoOpcodes(VgaOpcodeProc *op) {
	setupElvira2VideoOpcodes(op);

	op[60] = &AGOSEngine::vc60_killSprite;
	op[61] = &AGOSEngine::vc61;
	op[62] = &AGOSEngine::vc62_fastFadeOut;
	op[63] = &AGOSEngine::vc63_fastFadeIn;
}

void AGOSEngine::setupSimon1VideoOpcodes(VgaOpcodeProc *op) {
	setupCommonVideoOpcodes(op);

	op[11] = &AGOSEngine::vc11_clearPathFinder;
	op[17] = &AGOSEngine::vc17_setPathfinderItem;
	op[22] = &AGOSEngine::vc22_setSpritePaletteNew;
	op[32] = &AGOSEngine::vc32_copyVar;
	op[37] = &AGOSEngine::vc37_addToSpriteY;
	op[48] = &AGOSEngine::vc48_setPathFinder;
	op[52] = &AGOSEngine::vc52_playSound;
	op[60] = &AGOSEngine::vc60_killSprite;
	op[61] = &AGOSEngine::vc61_setMaskImage;
	op[62] = &AGOSEngine::vc62_fastFadeOut;
	op[63] = &AGOSEngine::vc63_fastFadeIn;
}

void AGOSEngine::setupSimon2VideoOpcodes(VgaOpcodeProc *op) {
	setupSimon1VideoOpcodes(op);

	op[56] = &AGOSEngine::vc56_delayLong;
	op[58] = &AGOSEngine::vc58;
	op[60] = &AGOSEngine::vc60_killSprite;
	op[64] = &AGOSEngine::vc64_skipIfSpeechEnded;
	op[65] = &AGOSEngine::vc65_slowFadeIn;
	op[66] = &AGOSEngine::vc66_skipIfNotEqual;
	op[67] = &AGOSEngine::vc67_skipIfGE;
	op[68] = &AGOSEngine::vc68_skipIfLE;
	op[69] = &AGOSEngine::vc69_playTrack;
	op[70] = &AGOSEngine::vc70_queueMusic;
	op[71] = &AGOSEngine::vc71_checkMusicQueue;
	op[72] = &AGOSEngine::vc72_play_track_2;
	op[73] = &AGOSEngine::vc73_setMark;
	op[74] = &AGOSEngine::vc74_clearMark;
}

void AGOSEngine::setupFeebleVideoOpcodes(VgaOpcodeProc *op) {
	setupSimon2VideoOpcodes(op);

	op[53] = &AGOSEngine::vc53_panSFX;
	op[75] = &AGOSEngine::vc75_setScale;
	op[76] = &AGOSEngine::vc76_setScaleXOffs;
	op[77] = &AGOSEngine::vc77_setScaleYOffs;
	op[78] = &AGOSEngine::vc78_computeXY;
	op[79] = &AGOSEngine::vc79_computePosNum;
	op[80] = &AGOSEngine::vc80_setOverlayImage;
	op[81] = &AGOSEngine::vc81_setRandom;
	op[82] = &AGOSEngine::vc82_getPathValue;
	op[83] = &AGOSEngine::vc83_playSoundLoop;
	op[84] = &AGOSEngine::vc84_stopSoundLoop;
}

void AGOSEngine::setupVgaOpcodes() {
	memset(_vga_opcode_table, 0, sizeof(_vga_opcode_table));

	switch (getGameType()) {
	case GType_ELVIRA1:
		setupElvira1VideoOpcodes(_vga_opcode_table);
		break;
	case GType_ELVIRA2:
		setupElvira2VideoOpcodes(_vga_opcode_table);
		break;
	case GType_WW:
		setupWaxworksVideoOpcodes(_vga_opcode_table);
		break;
	case GType_SIMON1:
		setupSimon1VideoOpcodes(_vga_opcode_table);
		break;
	case GType_SIMON2:
		setupSimon2VideoOpcodes(_vga_opcode_table);
		break;
	case GType_FF:
	case GType_PP:
		setupFeebleVideoOpcodes(_vga_opcode_table);
		break;
	default:
		error("setupVgaOpcodes: Unknown game");
	}
}

// Script parser
void AGOSEngine::runVgaScript() {
	for (;;) {
		uint opcode;

		if (_continousVgaScript) {
			if (_vcPtr != (const byte *)&_vc_get_out_of_code) {
				printf("%.5d %.5X: %5d %4d ", _vgaTickCounter, (unsigned int)(_vcPtr - _curVgaFile1), _vgaCurSpriteId, _vgaCurZoneNum);
				dumpVideoScript(_vcPtr, true);
			}
		}

		if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP) {
			opcode = *_vcPtr++;
		} else {
			opcode = READ_BE_UINT16(_vcPtr);
			_vcPtr += 2;
		}

		if (opcode == 0)
			return;

		debug(1, "runVgaScript: Video opcode %d", opcode);
		if (opcode >= _numVideoOpcodes || !_vga_opcode_table[opcode])
			error("Invalid VGA opcode '%d' encountered", opcode);

		(this->*_vga_opcode_table[opcode]) ();
	}
}

bool AGOSEngine::itemIsSiblingOf(uint16 a) {
	Item *item;

	CHECK_BOUNDS(a, _objectArray);

	item = _objectArray[a];
	if (item == NULL)
		return true;

	return me()->parent == item->parent;
}

bool AGOSEngine::itemIsParentOf(uint16 a, uint16 b) {
	Item *item_a, *item_b;

	CHECK_BOUNDS(a, _objectArray);
	CHECK_BOUNDS(b, _objectArray);

	item_a = _objectArray[a];
	item_b = _objectArray[b];

	if (item_a == NULL || item_b == NULL)
		return true;

	return derefItem(item_a->parent) == item_b;
}

bool AGOSEngine::vc_maybe_skip_proc_1(uint16 a, int16 b) {
	Item *item;

	CHECK_BOUNDS(a, _objectArray);

	item = _objectArray[a];
	if (item == NULL)
		return true;
	return item->state == b;
}

VgaSprite *AGOSEngine::findCurSprite() {
	VgaSprite *vsp = _vgaSprites;
	while (vsp->id) {
		if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP) {
			if (vsp->id == _vgaCurSpriteId && vsp->zoneNum == _vgaCurZoneNum)
				break;
		} else {
			if (vsp->id == _vgaCurSpriteId)
				break;
		}
		vsp++;
	}
	return vsp;
}

int AGOSEngine::vcReadVarOrWord() {
	if (getGameType() == GType_ELVIRA1) {
		return vcReadNextWord();
	} else {
		int16 var = vcReadNextWord();
		if (var < 0)
			var = vcReadVar(-var);
		return var;
	}
}

uint AGOSEngine::vcReadNextWord() {
	uint a;
	a = readUint16Wrapper(_vcPtr);
	_vcPtr += 2;
	return a;
}

uint AGOSEngine::vcReadNextByte() {
	return *_vcPtr++;
}

uint AGOSEngine::vcReadVar(uint var) {
	assert(var < _numVars);
	return (uint16)_variableArrayPtr[var];
}

void AGOSEngine::vcWriteVar(uint var, int16 value) {
	assert(var < _numVars);
	_variableArrayPtr[var] = value;
}

void AGOSEngine::vcSkipNextInstruction() {

	static const byte opcodeParamLenElvira1[] = {
		0, 6,  2, 10, 6, 4, 2, 2,
		4, 4,  8,  2, 0, 2, 2, 2,
		2, 2,  2,  2, 0, 4, 2, 2,
		2, 8,  0, 10, 0, 8, 0, 2,
		2, 0,  0,  0, 0, 2, 4, 2,
		4, 4,  0,  0, 2, 2, 2, 4,
		4, 0, 18,  2, 4, 4, 4, 0,
		4
	};

	static const byte opcodeParamLenWW[] = {
		0, 6, 2, 10, 6, 4, 2, 2,
		4, 4, 8, 2, 2, 2, 2, 2,
		2, 2, 2, 0, 4, 2, 2, 2,
		8, 0, 10, 0, 8, 0, 2, 2,
		0, 0, 0, 4, 4, 4, 2, 4,
		4, 4, 4, 2, 2, 4, 2, 2,
		2, 2, 2, 2, 2, 4, 6, 6,
		0, 0, 0, 0, 2, 2, 0, 0,
	};

	static const byte opcodeParamLenSimon1[] = {
		0, 6, 2, 10, 6, 4, 2, 2,
		4, 4, 10, 0, 2, 2, 2, 2,
		2, 0, 2, 0, 4, 2, 4, 2,
		8, 0, 10, 0, 8, 0, 2, 2,
		4, 0, 0, 4, 4, 2, 2, 4,
		4, 4, 4, 2, 2, 2, 2, 4,
		0, 2, 2, 2, 2, 4, 6, 6,
		0, 0, 0, 0, 2, 6, 0, 0,
	};

	static const byte opcodeParamLenSimon2[] = {
		0, 6, 2, 12, 6, 4, 2, 2,
		4, 4, 9, 0, 1, 2, 2, 2,
		2, 0, 2, 0, 4, 2, 4, 2,
		7, 0, 10, 0, 8, 0, 2, 2,
		4, 0, 0, 4, 4, 2, 2, 4,
		4, 4, 4, 2, 2, 2, 2, 4,
		0, 2, 2, 2, 2, 4, 6, 6,
		2, 0, 6, 6, 4, 6, 0, 0,
		0, 0, 4, 4, 4, 4, 4, 0,
		4, 2, 2
	};

	static const byte opcodeParamLenFeebleFiles[] = {
		0, 6, 2, 12, 6, 4, 2, 2,
		4, 4, 9, 0, 1, 2, 2, 2,
		2, 0, 2, 0, 4, 2, 4, 2,
		7, 0, 10, 0, 8, 0, 2, 2,
		4, 0, 0, 4, 4, 2, 2, 4,
		4, 4, 4, 2, 2, 2, 2, 4,
		0, 2, 2, 2, 6, 6, 6, 6,
		2, 0, 6, 6, 4, 6, 0, 0,
		0, 0, 4, 4, 4, 4, 4, 0,
		4, 2, 2, 4, 6, 6, 0, 0,
		6, 4, 2, 6, 0
	};

	uint16 opcode;
	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		opcode = vcReadNextByte();
		_vcPtr += opcodeParamLenFeebleFiles[opcode];
	} else if (getGameType() == GType_SIMON2) {
		opcode = vcReadNextByte();
		_vcPtr += opcodeParamLenSimon2[opcode];
	} else if (getGameType() == GType_SIMON1) {
		opcode = vcReadNextWord();
		_vcPtr += opcodeParamLenSimon1[opcode];
	} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
		opcode = vcReadNextWord();
		_vcPtr += opcodeParamLenWW[opcode];
	} else {
		opcode = vcReadNextWord();
		_vcPtr += opcodeParamLenElvira1[opcode];
	}

	if (_continousVgaScript)
		printf("; skipped\n");
}

// VGA Script commands
void AGOSEngine::vc1_fadeOut() {
	/* dummy opcode */
	_vcPtr += 6;
}

void AGOSEngine::vc2_call() {
	VgaPointersEntry *vpe;
	uint16 count, num, res;
	byte *old_file_1, *old_file_2;
	byte *b, *bb;
	const byte *vcPtrOrg;

	if (getGameType() == GType_ELVIRA2) {
		num = vcReadNextWord();
	} else {
		num = vcReadVarOrWord();
	}

	old_file_1 = _curVgaFile1;
	old_file_2 = _curVgaFile2;

	for (;;) {
		res = num / 100;
		vpe = &_vgaBufferPointers[res];

		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		if (vpe->vgaFile1 != NULL)
			break;
		if (_zoneNumber != res)
			_noOverWrite = _zoneNumber;

		loadZone(res);
		_noOverWrite = 0xFFFF;
	}


	bb = _curVgaFile1;
	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		b = bb + READ_LE_UINT16(bb + 2);
		count = READ_LE_UINT16(&((VgaFileHeader2_Feeble *) b)->imageCount);
		b = bb + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) b)->imageTable);

		while (count--) {
			if (READ_LE_UINT16(&((ImageHeader_Feeble *) b)->id) == num)
				break;
			b += sizeof(ImageHeader_Feeble);
		}
		assert(READ_LE_UINT16(&((ImageHeader_Feeble *) b)->id) == num);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		b = bb + READ_BE_UINT16(bb + 4);
		count = READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->imageCount);
		b = bb + READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->imageTable);

		while (count--) {
			if (READ_BE_UINT16(&((ImageHeader_Simon *) b)->id) == num)
				break;
			b += sizeof(ImageHeader_Simon);
		}
		assert(READ_BE_UINT16(&((ImageHeader_Simon *) b)->id) == num);
	} else {
		b = bb + READ_BE_UINT16(bb + 10);
		b += 20;

		count = READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->imageCount);
		b = bb + READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->imageTable);

		while (count--) {
			if (READ_BE_UINT16(&((ImageHeader_WW *) b)->id) == num)
				break;
			b += sizeof(ImageHeader_WW);
		}
		assert(READ_BE_UINT16(&((ImageHeader_WW *) b)->id) == num);
	}

	if (_startVgaScript) {
		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			dump_vga_script(_curVgaFile1 + READ_LE_UINT16(&((ImageHeader_Feeble*)b)->scriptOffs), res, num);
		} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
			dump_vga_script(_curVgaFile1 + READ_BE_UINT16(&((ImageHeader_Simon*)b)->scriptOffs), res, num);
		} else {
			dump_vga_script(_curVgaFile1 + READ_BE_UINT16(&((ImageHeader_WW*)b)->scriptOffs), res, num);
		}
	}

	vcPtrOrg = _vcPtr;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		_vcPtr = _curVgaFile1 + READ_LE_UINT16(&((ImageHeader_Feeble *) b)->scriptOffs);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		_vcPtr = _curVgaFile1 + READ_BE_UINT16(&((ImageHeader_Simon *) b)->scriptOffs);
	} else {
		_vcPtr = _curVgaFile1 + READ_BE_UINT16(&((ImageHeader_WW *) b)->scriptOffs);
	}

	runVgaScript();

	_curVgaFile1 = old_file_1;
	_curVgaFile2 = old_file_2;

	_vcPtr = vcPtrOrg;
}

void AGOSEngine::vc3_loadSprite() {
	uint16 windowNum, zoneNum, palette, x, y, vgaSpriteId;
	uint16 count, res;
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p, *pp;
	byte *old_file_1;

	if (getGameType() == GType_PP && getBitFlag(100)) {
		startAnOverlayAnim();
		return;
	}

	windowNum = vcReadNextWord();		/* 0 */

	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP) {
		zoneNum = vcReadNextWord();	/* 0 */
		vgaSpriteId = vcReadNextWord();	/* 2 */
	} else {
		vgaSpriteId = vcReadNextWord();	/* 2 */
		zoneNum = vgaSpriteId / 100;
	}

	x = vcReadNextWord();			/* 4 */
	y = vcReadNextWord();			/* 6 */
	palette = vcReadNextWord();		/* 8 */

	if (isSpriteLoaded(vgaSpriteId, zoneNum))
		return;

	vsp = _vgaSprites;
	while (vsp->id)
		vsp++;

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW)
		vsp->palette = 0;
	else
		vsp->palette = palette;
	vsp->windowNum = windowNum;
	vsp->priority = 0;
	vsp->flags = 0;
	vsp->image = 0;
	vsp->x = x;
	vsp->y = y;
	vsp->id = vgaSpriteId;
	vsp->zoneNum = res = zoneNum;

	old_file_1 = _curVgaFile1;
	for (;;) {
		vpe = &_vgaBufferPointers[res];
		_curVgaFile1 = vpe->vgaFile1;

		if (vpe->vgaFile1 != NULL)
			break;
		if (_zoneNumber != res)
			_noOverWrite = _zoneNumber;

		loadZone(res);
		_noOverWrite = 0xFFFF;
	}

	pp = _curVgaFile1;
	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		p = pp + READ_LE_UINT16(pp + 2);
		count = READ_LE_UINT16(&((VgaFileHeader2_Feeble *) p)->animationCount);
		p = pp + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) p)->animationTable);

		while (count--) {
			if (READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->id) == vgaSpriteId)
				break;
			p += sizeof(AnimationHeader_Feeble);
		}
		assert(READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->id) == vgaSpriteId);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		p = pp + READ_BE_UINT16(pp + 4);
		count = READ_BE_UINT16(&((VgaFileHeader2_Common *) p)->animationCount);
		p = pp + READ_BE_UINT16(&((VgaFileHeader2_Common *) p)->animationTable);

		while (count--) {
			if (READ_BE_UINT16(&((AnimationHeader_Simon *) p)->id) == vgaSpriteId)
				break;
			p += sizeof(AnimationHeader_Simon);
		}
		assert(READ_BE_UINT16(&((AnimationHeader_Simon *) p)->id) == vgaSpriteId);
	} else {
		p = pp + READ_BE_UINT16(pp + 10);
		p += 20;

		count = READ_BE_UINT16(&((VgaFileHeader2_Common *) p)->animationCount);
		p = pp + READ_BE_UINT16(&((VgaFileHeader2_Common *) p)->animationTable);

		while (count--) {
			if (READ_BE_UINT16(&((AnimationHeader_WW *) p)->id) == vgaSpriteId)
				break;
			p += sizeof(AnimationHeader_WW);
		}
		assert(READ_BE_UINT16(&((AnimationHeader_WW *) p)->id) == vgaSpriteId);
	}

#ifdef DUMP_FILE_NR
	{
		static bool dumped = false;
		if (res == DUMP_FILE_NR && !dumped) {
			dumped = true;
			dump_vga_file(_curVgaFile1);
		}
	}
#endif

#ifdef DUMP_BITMAPS_FILE_NR
	{
		static bool dumped = false;
		if (res == DUMP_BITMAPS_FILE_NR && !dumped) {
			dumped = true;
			dump_vga_bitmaps(_curVgaFile2, _curVgaFile1, res);
		}
	}
#endif

	if (_startVgaScript) {
		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			dump_vga_script(_curVgaFile1 + READ_LE_UINT16(&((AnimationHeader_Feeble*)p)->scriptOffs), res, vgaSpriteId);
		} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
			dump_vga_script(_curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_Simon*)p)->scriptOffs), res, vgaSpriteId);
		} else {
			dump_vga_script(_curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_WW*)p)->scriptOffs), res, vgaSpriteId);
		}
	}

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		addVgaEvent(_vgaBaseDelay, _curVgaFile1 + READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->scriptOffs), vgaSpriteId, res);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		addVgaEvent(_vgaBaseDelay, _curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_Simon *) p)->scriptOffs), vgaSpriteId, res);
	} else {
		addVgaEvent(_vgaBaseDelay, _curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_WW *) p)->scriptOffs), vgaSpriteId, res);
	}

	_curVgaFile1 = old_file_1;
}

void AGOSEngine::vc4_fadeIn() {
	/* dummy opcode */
	_vcPtr += 6;
}

void AGOSEngine::vc5_skip_if_neq() {
	uint16 var;

	if (getGameType() == GType_PP)
		var = vcReadVarOrWord();
	else
		var = vcReadNextWord();

	uint16 value = vcReadNextWord();
	if (vcReadVar(var) != value)
		vcSkipNextInstruction();
}

void AGOSEngine::vc6_skip_ifn_sib_with_a() {
	if (!itemIsSiblingOf(vcReadNextWord()))
		vcSkipNextInstruction();
}

void AGOSEngine::vc7_skip_if_sib_with_a() {
	if (itemIsSiblingOf(vcReadNextWord()))
		vcSkipNextInstruction();
}

void AGOSEngine::vc8_skip_if_parent_is() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();
	if (!itemIsParentOf(a, b))
		vcSkipNextInstruction();
}

void AGOSEngine::vc9_skip_if_unk3_is() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();
	if (!vc_maybe_skip_proc_1(a, b))
		vcSkipNextInstruction();
}

byte *vc10_depackColumn(VC10_state * vs) {
	int8 a = vs->depack_cont;
	const byte *src = vs->depack_src;
	byte *dst = vs->depack_dest;
	uint16 dh = vs->dh;
	byte color;

	if (a == -0x80)
		a = *src++;

	for (;;) {
		if (a >= 0) {
			color = *src++;
			do {
				*dst++ = color;
				if (!--dh) {
					if (--a < 0)
						a = -0x80;
					else
						src--;
					goto get_out;
				}
			} while (--a >= 0);
		} else {
			do {
				*dst++ = *src++;
				if (!--dh) {
					if (++a == 0)
						a = -0x80;
					goto get_out;
				}
			} while (++a != 0);
		}
		a = *src++;
	}

get_out:;
	vs->depack_src = src;
	vs->depack_cont = a;
	return vs->depack_dest + vs->y_skip;
}

void vc10_skip_cols(VC10_state *vs) {
	while (vs->x_skip) {
		vc10_depackColumn(vs);
		vs->x_skip--;
	}
}

byte *AGOSEngine::vc10_uncompressFlip(const byte *src, uint w, uint h) {
	w *= 8;

	byte *dst, *dstPtr, *srcPtr;
	byte color;
	int8 cur = -0x80;
	uint i, w_cur = w;

	dstPtr = _videoBuf1 + w;

	do {
		dst = dstPtr;
		uint h_cur = h;

		if (cur == -0x80)
			cur = *src++;

		for (;;) {
			if (cur >= 0) {
				/* rle_same */
				color = *src++;
				do {
					*dst = color;
					dst += w;
					if (!--h_cur) {
						if (--cur < 0)
							cur = -0x80;
						else
							src--;
						goto next_line;
					}
				} while (--cur >= 0);
			} else {
				/* rle_diff */
				do {
					*dst = *src++;
					dst += w;
					if (!--h_cur) {
						if (++cur == 0)
							cur = -0x80;
						goto next_line;
					}
				} while (++cur != 0);
			}
			cur = *src++;
		}
	next_line:
		dstPtr++;
	} while (--w_cur);

	srcPtr = dstPtr = _videoBuf1 + w;

	do {
		dst = dstPtr;
		for (i = 0; i != w; ++i) {
			byte b = srcPtr[i];
			b = (b >> 4) | (b << 4);
			*--dst = b;
		}

		srcPtr += w;
		dstPtr += w;
	} while (--h);

	return _videoBuf1;
}

byte *AGOSEngine::vc10_flip(const byte *src, uint w, uint h) {
	w *= 8;

	byte *dstPtr;
	uint i;

	dstPtr = _videoBuf1 + w;

	do {
		byte *dst = dstPtr;
		for (i = 0; i != w; ++i) {
			byte b = src[i];
			b = (b >> 4) | (b << 4);
			*--dst = b;
		}

		src += w;
		dstPtr += w;
	} while (--h);

	return _videoBuf1;
}

void AGOSEngine::decodeColumn(byte *dst, const byte *src, int height) {
	const uint pitch = _dxSurfacePitch;
	int8 reps = (int8)0x80;
	byte color;
	byte *dstPtr = dst;
	uint h = height, w = 8;

	for (;;) {
		reps = *src++;
		if (reps >= 0) {
			color = *src++;

			do {
				*dst = color;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */
					if (--w == 0)
						return;
					dst = ++dstPtr;
					h = height;
				}
			} while (--reps >= 0);
		} else {

			do {
				*dst = *src++;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */
					if (--w == 0)
						return;
					dst = ++dstPtr;
					h = height;
				}
			} while (++reps != 0);
		}
	}
}

void AGOSEngine::decodeRow(byte *dst, const byte *src, int width) {
	const uint pitch = _dxSurfacePitch;
	int8 reps = (int8)0x80;
	byte color;
	byte *dstPtr = dst;
	uint w = width, h = 8;

	for (;;) {
		reps = *src++;
		if (reps >= 0) {
			color = *src++;

			do {
				*dst++ = color;

				/* reached right edge? */
				if (--w == 0) {
					/* reached bottom? */
					if (--h == 0)
						return;
					dstPtr += pitch;
					dst = dstPtr;
					w = width;
				}
			} while (--reps >= 0);
		} else {

			do {
				*dst++ = *src++;

				/* reached right edge? */
				if (--w == 0) {
					/* reached bottom? */
					if (--h == 0)
						return;
					dstPtr += pitch;
					dst = dstPtr;
					w = width;
				}
			} while (++reps != 0);
		}
	}
}

void AGOSEngine::vc10_draw() {
	byte *p2;
	uint width, height;
	byte flags;
	VC10_state state;

	state.image = (int16)vcReadNextWord();
	if (state.image == 0)
		return;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		state.palette = (_vcPtr[0] * 16);
		_vcPtr += 2;
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		state.palette = (_vcPtr[1] * 16);
		_vcPtr += 2;
	} else {
		state.palette = 0;
	}

	if (getFeatures() & GF_32COLOR) {
		state.palette = 0;
	}

	state.x = (int16)vcReadNextWord();
	state.x -= _scrollX;

	state.y = (int16)vcReadNextWord();
	state.y -= _scrollY;

	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP) {
		state.flags = vcReadNextByte();
	} else {
		state.flags = vcReadNextWord();
	}

	if (state.image < 0)
		state.image = vcReadVar(-state.image);

	p2 = _curVgaFile2 + state.image * 8;
	state.depack_src = _curVgaFile2 + readUint32Wrapper(p2);
	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		width = READ_LE_UINT16(p2 + 6);
		height = READ_LE_UINT16(p2 + 4) & 0x7FFF;
		flags = p2[5];
	} else {
		width = READ_BE_UINT16(p2 + 6) / 16;
		height = p2[5];
		flags = p2[4];
	}

	if (height == 0 || width == 0)
		return;

	if (_dumpImages)
		dump_single_bitmap(_vgaCurZoneNum, state.image, state.depack_src, width, height,
											 state.palette);
	// Check if image is compressed
	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		if (flags & 0x80) {
			state.flags |= kDFCompressed;
		}
	} else {
		if (flags & 0x80 && !(state.flags & kDFCompressedFlip)) {
			if (state.flags & kDFFlip) {
				state.flags &= ~kDFFlip;
				state.flags |= kDFCompressedFlip;
			} else {
				state.flags |= kDFCompressed;
			}
		}
	}

	state.width = state.draw_width = width;		/* cl */
	state.height = state.draw_height = height;	/* ch */

	state.depack_cont = -0x80;

	state.x_skip = 0;				/* colums to skip = bh */
	state.y_skip = 0;				/* rows to skip   = bl */

	uint maxWidth = (getGameType() == GType_FF || getGameType() == GType_PP) ? 640 : 20;
	if ((getGameType() == GType_SIMON2 || getGameType() == GType_FF) && width > maxWidth) {
		horizontalScroll(&state);
		return;
	}
	if (getGameType() == GType_FF && height > 480) {
		verticalScroll(&state);
		return;
	}

	if (getGameType() != GType_FF && getGameType() != GType_PP) {
		if (state.flags & kDFCompressedFlip) {
			state.depack_src = vc10_uncompressFlip(state.depack_src, width, height);
		} else if (state.flags & kDFFlip) {
			state.depack_src = vc10_flip(state.depack_src, width, height);
		}
	}

	state.surf2_addr = getFrontBuf();
	state.surf2_pitch = _dxSurfacePitch;

	state.surf_addr = getBackBuf();
	state.surf_pitch = _dxSurfacePitch;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		drawImages_Feeble(&state);
	} else {
		drawImages(&state);
	}
}

bool AGOSEngine::drawImages_clip(VC10_state *state) {
	const uint16 *vlut;
	uint maxWidth, maxHeight;
	int cur;

	vlut = &_videoWindows[_windowNum * 4];

	if (getGameType() != GType_FF && getGameType() != GType_PP) {
		state->draw_width = state->width * 2;
	} 

	cur = state->x;
	if (cur < 0) {
		do {
			if (!--state->draw_width)
				return 0;
			state->x_skip++;
		} while (++cur);
	}
	state->x = cur;

	maxWidth = (getGameType() == GType_FF || getGameType() == GType_PP) ? _screenWidth : (vlut[2] * 2);
	cur += state->draw_width - maxWidth;
	if (cur > 0) {
		do {
			if (!--state->draw_width)
				return 0;
		} while (--cur);
	}

	cur = state->y;
	if (cur < 0) {
		do {
			if (!--state->draw_height)
				return 0;
			state->y_skip++;
		} while (++cur);
	}
	state->y = cur;

	maxHeight = (getGameType() == GType_FF || getGameType() == GType_PP) ? _screenHeight : vlut[3];
	cur += state->draw_height - maxHeight;
	if (cur > 0) {
		do {
			if (!--state->draw_height)
				return 0;
		} while (--cur);
	}

	assert(state->draw_width != 0 && state->draw_height != 0);

	if (getGameType() != GType_FF && getGameType() != GType_PP) {
		state->draw_width *= 4;
	}

	return 1;
}

void AGOSEngine::drawImages_Feeble(VC10_state *state) {
	if (state->flags & kDFCompressed) {
		if (state->flags & kDFScaled) {
			state->surf_addr = getScaleBuf();
			state->surf_pitch = _dxSurfacePitch;

			uint w, h;
			byte *src, *dst, *dstPtr;

			state->dl = state->width;
			state->dh = state->height;

			dstPtr = state->surf_addr;
			w = 0;
			do {
				src = vc10_depackColumn(state);
				dst = dstPtr;

				h = 0;
				do {
					*dst = *src;
					dst += _screenWidth;
					src++;
				} while (++h != state->draw_height);
				dstPtr++;
			} while (++w != state->draw_width);

			if (_vgaCurSpritePriority % 10 != 9) {
				_scaleX = state->x;
				_scaleY = state->y;
				_scaleWidth = state->width;
				_scaleHeight = state->height;
			} else {
				scaleClip(state->height, state->width, state->y, state->x, state->y + _scrollY);
			}
		} else if (state->flags & kDFOverlayed) {
			state->surf_addr = getScaleBuf();
			state->surf_pitch = _dxSurfacePitch;
			state->surf_addr += (state->x + _scrollX) + (state->y + _scrollY) * state->surf_pitch;

			uint w, h;
			byte *src, *dst, *dstPtr;

			state->dl = state->width;
			state->dh = state->height;

			dstPtr = state->surf_addr;
			w = 0;
			do {
				byte color;

				src = vc10_depackColumn(state);
				dst = dstPtr;

				h = 0;
				do {
					color = *src;
					if (color != 0)
						*dst = color;
					dst += _screenWidth;
					src++;
				} while (++h != state->draw_height);
				dstPtr++;
			} while (++w != state->draw_width);

			if (_vgaCurSpritePriority % 10 == 9) {
				scaleClip(_scaleHeight, _scaleWidth, _scaleY, _scaleX, _scaleY + _scrollY);
			}
		} else {
			if (drawImages_clip(state) == 0)
				return;

			state->surf_addr += state->x + state->y * state->surf_pitch;

			uint w, h;
			byte *src, *dst, *dstPtr;

			state->dl = state->width;
			state->dh = state->height;

			vc10_skip_cols(state);


			if (state->flags & kDFMasked) {
				if (getGameType() == GType_FF && !getBitFlag(81)) {
					if (state->x  > _feebleRect.right)
						return;
					if (state->y > _feebleRect.bottom)
						return;
					if (state->x + state->width < _feebleRect.left)
						return;
					if (state->y + state->height < _feebleRect.top)
						return;
				}

				dstPtr = state->surf_addr;
				w = 0;
				do {
					byte color;

					src = vc10_depackColumn(state);
					dst = dstPtr;

					h = 0;
					do {
						color = *src;
						if (color)
							*dst = color;
						dst += _screenWidth;
						src++;
					} while (++h != state->draw_height);
					dstPtr++;
				} while (++w != state->draw_width);
			} else {
				dstPtr = state->surf_addr;
				w = 0;
				do {
					byte color;

					src = vc10_depackColumn(state);
					dst = dstPtr;

					h = 0;
					do {
						color = *src;
						if ((state->flags & kDFNonTrans) || color != 0)
							*dst = color;
						dst += _screenWidth;
						src++;
					} while (++h != state->draw_height);
					dstPtr++;
				} while (++w != state->draw_width);
			}
		}
	} else {
		if (drawImages_clip(state) == 0)
			return;

		state->surf_addr += state->x + state->y * state->surf_pitch;

		const byte *src;
		byte *dst;
		uint count;

		src = state->depack_src + state->width * state->y_skip;
		dst = state->surf_addr;
		do {
			for (count = 0; count != state->draw_width; count++) {
				byte color;
				color = src[count + state->x_skip];
				if (color) {
					if ((state->flags & kDFShaded) && color == 220)
						color = 244;

					dst[count] = color;
				}
			}
			dst += _screenWidth;
			src += state->width;
		} while (--state->draw_height);
	} 
}

void AGOSEngine::drawImages(VC10_state *state) {
	const uint16 *vlut = &_videoWindows[_windowNum * 4];

	if (drawImages_clip(state) == 0)
		return;

	uint xoffs, yoffs;
	if (getGameType() == GType_ELVIRA1) {
		//if (_windowNum != 2 && _windowNum != 3 && _windowNum != 6) {
		//	xoffs = ((vlut[0] - _videoWindows[16]) * 2 + state->x) * 8;
		//	yoffs = (vlut[1] - _videoWindows[17] + state->y);
		//} else {
			xoffs = (vlut[0] * 2 + state->x) * 8;
			yoffs = vlut[1] + state->y;
		//}
	} else if (getGameType() == GType_ELVIRA2) {
		//if (_windowNum == 4 || _windowNum >= 10) {
		//	xoffs = ((vlut[0] - _videoWindows[16]) * 2 + state->x) * 8;
		//	yoffs = (vlut[1] - _videoWindows[17] + state->y);
		//} else {
			xoffs = (vlut[0] * 2 + state->x) * 8;
			yoffs = vlut[1] + state->y;
		//}
	} else if (getGameType() == GType_WW) {
		//if (_windowNum == 4 || (_windowNum >= 10 && _windowNum < 28)) {
		//	xoffs = ((vlut[0] - _videoWindows[16]) * 2 + state->x) * 8;
		//	yoffs = (vlut[1] - _videoWindows[17] + state->y);
		//} else {
			xoffs = (vlut[0] * 2 + state->x) * 8;
			yoffs = vlut[1] + state->y;
		//}
	} else if (getGameType() == GType_SIMON1 && (_subroutine == 2923 || _subroutine == 2926)) {
		// Allow one section of Simon the Sorcerer 1 introduction to be displayed
		// in lower half of screen
		xoffs = state->x * 8;
		yoffs = state->y;
	} else {
		xoffs = ((vlut[0] - _videoWindows[16]) * 2 + state->x) * 8;
		yoffs = (vlut[1] - _videoWindows[17] + state->y);
	}

	state->surf2_addr += xoffs + yoffs * state->surf_pitch;
	state->surf_addr += xoffs + yoffs * state->surf2_pitch;

	if (state->flags & kDFMasked) {
		byte *mask, *src, *dst;
		byte h;
		uint w;

		state->x_skip *= 4;
		state->dl = state->width;
		state->dh = state->height;

		vc10_skip_cols(state);

		w = 0;
		do {
			mask = vc10_depackColumn(state);	/* esi */
			src = state->surf2_addr + w * 2;	/* ebx */
			dst = state->surf_addr + w * 2;		/* edi */

			h = state->draw_height;
			if ((getGameType() == GType_SIMON1) && getBitFlag(88)) {
				/* transparency */
				do {
					if (mask[0] & 0xF0) {
						if ((dst[0] & 0x0F0) == 0x20)
							dst[0] = src[0];
					}
					if (mask[0] & 0x0F) {
						if ((dst[1] & 0x0F0) == 0x20)
							dst[1] = src[1];
					}
					mask++;
					dst += state->surf_pitch;
					src += state->surf2_pitch;
				} while (--h);
			} else {
				/* no transparency */
				do {
					if (mask[0] & 0xF0)
						dst[0] = src[0];
					if (mask[0] & 0x0F)
						dst[1] = src[1];
					mask++;
					dst += state->surf_pitch;
					src += state->surf2_pitch;
				} while (--h);
			}
		} while (++w != state->draw_width);
	} else if ((((_lockWord & 0x20) && state->palette == 0) || state->palette == 0xC0) &&
		(getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
		getPlatform() != Common::kPlatformAmiga) {
		const byte *src;
		byte *dst;
		uint h, i;

		if (state->flags & kDFCompressed) {
			byte *dstPtr = state->surf_addr;
			src = state->depack_src;
			/* AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD EEEEEEEE
			 * aaaaabbb bbcccccd ddddeeee efffffgg ggghhhhh
			 */

			do {
				uint count = state->draw_width / 4;

				dst = dstPtr;
				do {
					uint32 bits = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | (src[3]);
					byte color;

					color = (byte)((bits >> (32 - 5)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[0] = color;
					color = (byte)((bits >> (32 - 10)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[1] = color;
					color = (byte)((bits >> (32 - 15)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[2] = color;
					color = (byte)((bits >> (32 - 20)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[3] = color;
					color = (byte)((bits >> (32 - 25)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[4] = color;
					color = (byte)((bits >> (32 - 30)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[5] = color;

					bits = (bits << 8) | src[4];

					color = (byte)((bits >> (40 - 35)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[6] = color;
					color = (byte)((bits) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[7] = color;

					dst += 8;
					src += 5;
				} while (--count);
				dstPtr += _screenWidth;
			} while (--state->draw_height);
		} else {
			src = state->depack_src + (state->width * state->y_skip * 16) + (state->x_skip * 8);
			dst = state->surf_addr;

			state->draw_width *= 2;

			h = state->draw_height;
			do {
				for (i = 0; i != state->draw_width; i++)
					if ((state->flags & kDFNonTrans) || src[i])
						dst[i] = src[i];
				dst += _screenWidth;
				src += state->width * 16;
			} while (--h);
		}
	} else {
		if (getGameType() == GType_SIMON2 && state->flags & kDFUseFrontBuf && getBitFlag(171)) {
			state->surf_addr = state->surf2_addr;
			state->surf_pitch = state->surf2_pitch;
		}

		if (state->flags & kDFCompressed) {
			uint w, h;
			byte *src, *dst, *dstPtr;

			state->x_skip *= 4;				/* reached */

			state->dl = state->width;
			state->dh = state->height;

			vc10_skip_cols(state);

			dstPtr = state->surf_addr;
			if (!(state->flags & kDFNonTrans) && (state->flags & 0x40)) { /* reached */
				dstPtr += vcReadVar(252);
			}
			w = 0;
			do {
				byte color;

				src = vc10_depackColumn(state);
				dst = dstPtr;

				h = 0;
				do {
					color = (*src / 16);
					if ((state->flags & kDFNonTrans) || color != 0)
						dst[0] = color | state->palette;
					color = (*src & 15);
					if ((state->flags & kDFNonTrans) || color != 0)
						dst[1] = color | state->palette;
					dst += _screenWidth;
					src++;
				} while (++h != state->draw_height);
				dstPtr += 2;
			} while (++w != state->draw_width);
		} else {
			const byte *src;
			byte *dst;
			uint count;

			src = state->depack_src + (state->width * state->y_skip) * 8;
			dst = state->surf_addr;
			state->x_skip *= 4;

			do {
				for (count = 0; count != state->draw_width; count++) {
					byte color;
					color = (src[count + state->x_skip] / 16);
					if ((state->flags & kDFNonTrans) || color)
						dst[count * 2] = color | state->palette;
					color = (src[count + state->x_skip] & 15);
					if ((state->flags & kDFNonTrans) || color)
						dst[count * 2 + 1] = color | state->palette;
				}
				dst += _screenWidth;
				src += state->width * 8;
			} while (--state->draw_height);
		}
	}
}

void AGOSEngine::horizontalScroll(VC10_state *state) {
	const byte *src;
	byte *dst;
	int w;

	if (getGameType() == GType_FF)
		_scrollXMax = state->width - 640;
	else
		_scrollXMax = state->width * 2 - 40;
	_scrollYMax = 0;
	_scrollImage = state->depack_src;
	_scrollHeight = state->height;
	if (_variableArrayPtr[34] < 0)
		state->x = _variableArrayPtr[251];

	_scrollX = state->x;

	vcWriteVar(251, _scrollX);

	dst = getBackBuf();

	if (getGameType() == GType_FF)
		src = state->depack_src + _scrollX / 2;
	else
		src = state->depack_src + _scrollX * 4;

	for (w = 0; w < _screenWidth; w += 8) {
		decodeColumn(dst, src + readUint32Wrapper(src), state->height);
		dst += 8;
		src += 4;
	}
}

void AGOSEngine::verticalScroll(VC10_state *state) {
	const byte *src;
	byte *dst;
	int h;

	_scrollXMax = 0;
	_scrollYMax = state->height - 480;
	_scrollImage = state->depack_src;
	_scrollWidth = state->width;
	if (_variableArrayPtr[34] < 0)
		state->y = _variableArrayPtr[250];

	_scrollY = state->y;

	vcWriteVar(250, _scrollY);

	dst = getBackBuf();
	src = state->depack_src + _scrollY / 2;

	for (h = 0; h < _screenHeight; h += 8) {
		decodeRow(dst, src + READ_LE_UINT32(src), state->width);
		dst += 8 * state->width;
		src += 4;
	}
}

void AGOSEngine::scaleClip(int16 h, int16 w, int16 y, int16 x, int16 scrollY) {
	Common::Rect srcRect, dstRect;
	float factor, xscale;

	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = w;
	srcRect.bottom = h;

	if (scrollY > _baseY)
		factor = 1 + ((scrollY - _baseY) * _scale);
	else
		factor = 1 - ((_baseY - scrollY) * _scale);

	xscale = ((w * factor) / 2);

	dstRect.left   = (int16)(x - xscale);
	if (dstRect.left > _screenWidth - 1)
		return;
	dstRect.top    = (int16)(y - (h * factor));
	if (dstRect.top > _screenHeight - 1)
		return;

	dstRect.right  = (int16)(x + xscale);
	dstRect.bottom = y;

	_feebleRect = dstRect;

	_variableArray[20] = _feebleRect.top;
	_variableArray[21] = _feebleRect.left;
	_variableArray[22] = _feebleRect.bottom;
	_variableArray[23] = _feebleRect.right;

	debug(5, "Left %d Right %d Top %d Bottom %d", dstRect.left, dstRect.right, dstRect.top, dstRect.bottom);

	// Unlike normal rectangles in ScummVM, it seems that in the case of
	// the destination rectangle the bottom and right coordinates are
	// considered to be inside the rectangle. For the source rectangle,
	// I believe that they are not.

	int scaledW = dstRect.width() + 1;
	int scaledH = dstRect.height() + 1;

	byte *src = getScaleBuf();
	byte *dst = getBackBuf();

	dst += _dxSurfacePitch * dstRect.top + dstRect.left;

	for (int dstY = 0; dstY < scaledH; dstY++) {
		if (dstRect.top + dstY >= 0 && dstRect.top + dstY < _screenHeight) {
			int srcY = (dstY * h) / scaledH;
			byte *srcPtr = src + _dxSurfacePitch * srcY;
			byte *dstPtr = dst + _dxSurfacePitch * dstY;
			for (int dstX = 0; dstX < scaledW; dstX++) {
				if (dstRect.left + dstX >= 0 && dstRect.left + dstX < _screenWidth) {
					int srcX = (dstX * w) / scaledW;
					if (srcPtr[srcX])
						dstPtr[dstX] = srcPtr[srcX];
				}
			}
		}
	}
}

void AGOSEngine::vc11() {
	uint a = vcReadNextWord();
	debug(0, "vc11: stub (%d)", a);
}

void AGOSEngine::vc11_clearPathFinder() {
	memset(&_pathFindArray, 0, sizeof(_pathFindArray));
}

void AGOSEngine::vc12_delay() {
	VgaSprite *vsp = findCurSprite();
	uint16 num;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		num = vcReadNextByte();
	} else if (getGameType() == GType_SIMON2) {
		num = vcReadNextByte() * _frameRate;
	} else {
		num = vcReadVarOrWord() * _frameRate;
	}

	// Work around to allow inventory arrows to be
	// shown in some versions of Simon the Sorcerer 1
	if ((getGameType() == GType_SIMON1) && vsp->id == 128)
		num = 0;
	else
		num += _vgaBaseDelay;

	addVgaEvent(num, _vcPtr, _vgaCurSpriteId, _vgaCurZoneNum);
	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void AGOSEngine::vc13_addToSpriteX() {
	VgaSprite *vsp = findCurSprite();
	vsp->x += (int16)vcReadNextWord();
	_vgaSpriteChanged++;
}

void AGOSEngine::vc14_addToSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y += (int16)vcReadNextWord();
	_vgaSpriteChanged++;
}

void AGOSEngine::vc15_sync() {
	VgaSleepStruct *vfs = _vgaSleepStructs, *vfs_tmp;
	uint16 id = vcReadNextWord();
	while (vfs->ident != 0) {
		if (vfs->ident == id) {
			addVgaEvent(_vgaBaseDelay, vfs->code_ptr, vfs->sprite_id, vfs->cur_vga_file);
			vfs_tmp = vfs;
			do {
				memcpy(vfs_tmp, vfs_tmp + 1, sizeof(VgaSleepStruct));
				vfs_tmp++;
			} while (vfs_tmp->ident != 0);
		} else {
			vfs++;
		}
	}

	_lastVgaWaitFor = id;
	/* clear a wait event */
	if (id == _vgaWaitFor)
		_vgaWaitFor = 0;
}

void AGOSEngine::vc16_waitSync() {
	VgaSleepStruct *vfs = _vgaSleepStructs;
	while (vfs->ident)
		vfs++;

	vfs->ident = vcReadNextWord();
	vfs->code_ptr = _vcPtr;
	vfs->sprite_id = _vgaCurSpriteId;
	vfs->cur_vga_file = _vgaCurZoneNum;

	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void AGOSEngine::vc17_waitEnd() {
	// TODO
	uint a = vcReadNextWord();
	debug(0, "vc17_waitEnd: stub (%d)", a);
}

void AGOSEngine::vc17_setPathfinderItem() {
	uint16 a = vcReadNextWord();
	_pathFindArray[a - 1] = (const uint16 *)_vcPtr;

	int end = (getGameType() == GType_FF || getGameType() == GType_PP) ? 9999 : 999;
	while (readUint16Wrapper(_vcPtr) != end)
		_vcPtr += 4;
	_vcPtr += 2;
}

void AGOSEngine::vc18_jump() {
	int16 offs = vcReadNextWord();
	_vcPtr += offs;
}

void AGOSEngine::vc19_loop() {
	// TODO
	debug(0, "vc19_loop: stub");
}

void AGOSEngine::vc20_setRepeat() {
	/* FIXME: This opcode is somewhat strange: it first reads a BE word from
	 * the script (advancing the script pointer in doing so); then it writes
	 * back the same word, this time as LE, into the script.
	 */
	uint16 a = vcReadNextWord();
	WRITE_LE_UINT16(const_cast<byte *>(_vcPtr), a);
	_vcPtr += 2;
}

void AGOSEngine::vc21_endRepeat() {
	int16 a = vcReadNextWord();
	const byte *tmp = _vcPtr + a;
	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP)
		tmp += 3;
	else
		tmp += 4;

	uint16 val = READ_LE_UINT16(tmp);
	if (val != 0) {
		// Decrement counter
		WRITE_LE_UINT16(const_cast<byte *>(tmp), val - 1);
		_vcPtr = tmp + 2;
	}
}

void AGOSEngine::vc22_setSpritePaletteOld() {
	byte *offs, *palptr, *src;
	uint16 b, num;

	b = vcReadNextWord();

	num = 16;

	palptr = _displayPalette;
	_bottomPalette = 1;

	if (getGameType() == GType_ELVIRA1) {
		if (b >= 1000) {
			b -= 1000;
			_bottomPalette = 0;
		} else {
			const byte extraColors[19 * 3] = {
				40,  0,  0,   24, 24, 16,   48, 48, 40,
				 0,  0,  0,   16,  0,  0,    8,  8,  0,
				48, 24,  0,   56, 40,  0,    0,  0, 24,
				 8, 16, 24,   24, 32, 40,   16, 24,  0,
				24,  8,  0,   16, 16,  0,   40, 40, 32,
				32, 32, 24,   40,  0,  0,   24, 24, 16,
				48, 48, 40
			};

			num = 13;

			for (int i = 0; i < 19; i++) {
				palptr[(13 + i) * 4 + 0] = extraColors[i * 3 + 0] * 4;
				palptr[(13 + i) * 4 + 1] = extraColors[i * 3 + 1] * 4;
				palptr[(13 + i) * 4 + 2] = extraColors[i * 3 + 2] * 4;
				palptr[(13 + i) * 4 + 3] = 0;
			}
		}
	}

	offs = _curVgaFile1 + READ_BE_UINT16(_curVgaFile1 + 6);
	src = offs + b * 32;

	do {
		uint16 color = READ_BE_UINT16(src);
		palptr[0] = ((color & 0xf00) >> 8) * 32;
		palptr[1] = ((color & 0x0f0) >> 4) * 32;
		palptr[2] = ((color & 0x00f) >> 0) * 32;
		palptr[3] = 0;

		palptr += 4;
		src += 2;
	} while (--num);

	_paletteFlag = 2;
	_vgaSpriteChanged++;
}

void AGOSEngine::vc22_setSpritePaletteNew() {
	byte *offs, *palptr, *src;
	uint16 a = 0, b, num, palSize;

	a = vcReadNextWord();
	b = vcReadNextWord();

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		num = 256;
		palSize = 768;

		palptr = _displayPalette;
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		num = a == 0 ? 32 : 16;
		palSize = 96;

		palptr = &_displayPalette[(a * 64)];
	}

	offs = _curVgaFile1 + 6;
	src = offs + b * palSize;

	do {
		palptr[0] = src[0] * 4;
		palptr[1] = src[1] * 4;
		palptr[2] = src[2] * 4;
		palptr[3] = 0;

		palptr += 4;
		src += 3;
	} while (--num);

	_paletteFlag = 2;
	_vgaSpriteChanged++;
}

void AGOSEngine::vc23_setSpritePriority() {
	VgaSprite *vsp = findCurSprite(), *vus2;
	uint16 pri = vcReadNextWord();
	VgaSprite bak;

	if (vsp->id == 0)
		return;

	memcpy(&bak, vsp, sizeof(bak));
	bak.priority = pri;
	bak.windowNum |= 0x8000;

	vus2 = vsp;

	if (vsp != _vgaSprites && pri < vsp[-1].priority) {
		do {
			vsp--;
		} while (vsp != _vgaSprites && pri < vsp[-1].priority);
		do {
			memcpy(vus2, vus2 - 1, sizeof(VgaSprite));
		} while (--vus2 != vsp);
		memcpy(vus2, &bak, sizeof(VgaSprite));
	} else if (vsp[1].id != 0 && pri >= vsp[1].priority) {
		do {
			vsp++;
		} while (vsp[1].id != 0 && pri >= vsp[1].priority);
		do {
			memcpy(vus2, vus2 + 1, sizeof(VgaSprite));
		} while (++vus2 != vsp);
		memcpy(vus2, &bak, sizeof(VgaSprite));
	} else {
		vsp->priority = pri;
	}
	_vgaSpriteChanged++;
}

void AGOSEngine::vc24_setSpriteXY() {
	VgaSprite *vsp = findCurSprite();

	if (getGameType() == GType_ELVIRA2) {
		vsp->image = vcReadNextWord();
	} else {
		vsp->image = vcReadVarOrWord();
	}

	vsp->x += (int16)vcReadNextWord();
	vsp->y += (int16)vcReadNextWord();
	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP) {
		vsp->flags = vcReadNextByte();
	} else {
		vsp->flags = vcReadNextWord();
	}

	_vgaSpriteChanged++;
}

void AGOSEngine::vc25_halt_sprite() {
	VgaSprite *vsp = findCurSprite();
	while (vsp->id != 0) {
		memcpy(vsp, vsp + 1, sizeof(VgaSprite));
		vsp++;
	}
	_vcPtr = (byte *)&_vc_get_out_of_code;
	_vgaSpriteChanged++;
}

void AGOSEngine::vc26_setSubWindow() {
	uint16 *as = &_videoWindows[vcReadNextWord() * 4]; // number
	as[0] = vcReadNextWord(); // x
	as[1] = vcReadNextWord(); // y
	as[2] = vcReadNextWord(); // width
	as[3] = vcReadNextWord(); // height
}

void AGOSEngine::vc27_resetSprite() {
	VgaSprite bak, *vsp;
	VgaSleepStruct *vfs;
	VgaTimerEntry *vte, *vte2;

	_lockWord |= 8;

	_lastVgaWaitFor = 0;

	memset(&bak, 0, sizeof(bak));

	vsp = _vgaSprites;
	while (vsp->id) {
		if ((getGameType() == GType_SIMON1) && vsp->id == 128) {
			memcpy(&bak, vsp, sizeof(VgaSprite));
		}
		vsp->id = 0;
		vsp++;
	}

	if (bak.id != 0)
		memcpy(_vgaSprites, &bak, sizeof(VgaSprite));

	vfs = _vgaSleepStructs;
	while (vfs->ident) {
		vfs->ident = 0;
		vfs++;
	}

	vte = _vgaTimerList;
	while (vte->delay) {
		if ((getGameType() == GType_SIMON1) && vte->sprite_id == 128) {
			vte++;
		} else {
			vte2 = vte;
			while (vte2->delay) {
				memcpy(vte2, vte2 + 1, sizeof(VgaTimerEntry));
				vte2++;
			}
		}
	}

	vcWriteVar(254, 0);

	if (getGameType() == GType_FF || getGameType() == GType_PP)
		setBitFlag(42, true);

	_lockWord &= ~8;
}

void AGOSEngine::vc28_playSFX() {
	// TODO
	uint a = vcReadNextWord();
	uint b = vcReadNextWord();
	uint c = vcReadNextWord();
	uint d = vcReadNextWord();
	debug(0, "vc37_pokePalette: stub (%d, %d, %d, %d)", a, b, c, d);
}

void AGOSEngine::vc29_stopAllSounds() {
	if (getGameType() != GType_PP)
		_sound->stopVoice();

	_sound->stopAllSfx();
}

void AGOSEngine::vc30_setFrameRate() {
	_frameRate = vcReadNextWord();
}

void AGOSEngine::vc31_setWindow() {
	_windowNum = vcReadNextWord();
}

void AGOSEngine::vc32_saveScreen() {
	// TODO
	debug(0, "vc32_saveScreen: stub");
}

void AGOSEngine::vc32_copyVar() {
	uint16 a = vcReadVar(vcReadNextWord());
	vcWriteVar(vcReadNextWord(), a);
}

void AGOSEngine::vc33_setMouseOn() {
	if (_mouseHideCount != 0) {
		_mouseHideCount = 1;
		mouseOn();
	}
}

void AGOSEngine::vc34_setMouseOff() {
	mouseOff();
	_mouseHideCount = 200;
	_leftButtonDown = 0;
}

void AGOSEngine::clearWindow(uint num, uint color) {
	if (getGameType() == GType_ELVIRA1) {
		if (num == 2 || num == 6)
			return;
	} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
		if (num != 4 && num < 10)
			return;
	}

	if (num == 3) {
		memset(getBackBuf(), 0, _screenWidth * _screenHeight);
	} else { 
		const uint16 *vlut = &_videoWindows[num * 4];
		byte *dst = getBackBuf() + vlut[0] * 16 + vlut[1] * _dxSurfacePitch;

		for (uint h = 0; h < vlut[3]; h++) {
			memset(dst, 0, vlut[2] * 16);
			dst += _screenWidth;
		}
	}
}

void AGOSEngine::vc35_clearWindow() {
	uint16 num = vcReadNextWord();
	uint16 color = vcReadNextWord();
	clearWindow(num, color);
}

void AGOSEngine::vc36_setWindowImage() {
	_updateScreen = false;
	uint16 vga_res = vcReadNextWord();
	uint16 windowNum = vcReadNextWord();

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		_copyPartialMode = 2;
	} else if (getGameType() == GType_SIMON1) {
		if (windowNum == 16) {
			_copyPartialMode = 2;
		} else {
			set_video_mode_internal(windowNum, vga_res);
		}
	} else {
		set_video_mode_internal(windowNum, vga_res);
	}
}

void AGOSEngine::vc37_pokePalette() {
	// TODO
	uint a = vcReadNextWord();
	uint b = vcReadNextWord();
	debug(0, "vc37_pokePalette: stub (%d, %d)", a, b);
}

void AGOSEngine::vc37_addToSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y += vcReadVar(vcReadNextWord());
	_vgaSpriteChanged++;
}

void AGOSEngine::vc38_skipIfVarZero() {
	uint16 var;
	if (getGameType() == GType_PP)
		var = vcReadVarOrWord();
	else
		var = vcReadNextWord();

	if (vcReadVar(var) == 0)
		vcSkipNextInstruction();
}

void AGOSEngine::vc39_setVar() {
	uint16 var;
	if (getGameType() == GType_PP)
		var = vcReadVarOrWord();
	else
		var = vcReadNextWord();

	int16 value = vcReadNextWord();
	vcWriteVar(var, value);
}

void AGOSEngine::vc40() {
	uint16 var = vcReadNextWord();
	int16 value = vcReadVar(var) + vcReadNextWord();

	if ((getGameType() == GType_SIMON2) && var == 15 && !getBitFlag(80)) {
		int16 tmp;

		if (_scrollCount != 0) {
			if (_scrollCount >= 0)
				goto no_scroll;
			_scrollCount = 0;
		} else {
			if (_scrollFlag != 0)
				goto no_scroll;
		}

		if (value - _scrollX >= 30) {
			_scrollCount = 20;
			tmp = _scrollXMax - _scrollX;
			if (tmp < 20)
				_scrollCount = tmp;
			addVgaEvent(6, NULL, 0, 0);	 /* scroll event */
		}
	}
no_scroll:;

	vcWriteVar(var, value);
}

void AGOSEngine::vc41() {
	uint16 var = vcReadNextWord();
	int16 value = vcReadVar(var) - vcReadNextWord();

	if ((getGameType() == GType_SIMON2) && var == 15 && !getBitFlag(80)) {
		if (_scrollCount != 0) {
			if (_scrollCount < 0)
				goto no_scroll;
			_scrollCount = 0;
		} else {
			if (_scrollFlag != 0)
				goto no_scroll;
		}

		if ((uint16)(value - _scrollX) < 11) {
			_scrollCount = -20;
			if (_scrollX < 20)
				_scrollCount = -_scrollX;
			addVgaEvent(6, NULL, 0, 0);	 /* scroll event */
		}
	}
no_scroll:;

	vcWriteVar(var, value);
}

void AGOSEngine::vc42_delayIfNotEQ() {
	uint16 val = vcReadVar(vcReadNextWord());
	if (val != vcReadNextWord()) {

		addVgaEvent(_frameRate + 1, _vcPtr - 4, _vgaCurSpriteId, _vgaCurZoneNum);
		_vcPtr = (byte *)&_vc_get_out_of_code;
	}
}

void AGOSEngine::vc43_skipIfBitClear() {
	if (!getBitFlag(vcReadNextWord())) {
		vcSkipNextInstruction();
	}
}

void AGOSEngine::vc44_skipIfBitSet() {
	if (getBitFlag(vcReadNextWord())) {
		vcSkipNextInstruction();
	}
}

void AGOSEngine::vc45() {
	// TODO
	uint num = vcReadNextWord();
	uint color = vcReadNextWord();
	debug(0, "vc45: window %d color %d", num, color);
}

void AGOSEngine::vc45_setSpriteX() {
	VgaSprite *vsp = findCurSprite();
	vsp->x = vcReadVar(vcReadNextWord());
	_vgaSpriteChanged++;
}

void AGOSEngine::vc46() {
	// TODO
	uint a = vcReadNextWord();
	debug(0, "vc46: stub (%d)", a);
}

void AGOSEngine::vc46_setSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y = vcReadVar(vcReadNextWord());
	_vgaSpriteChanged++;
}

void AGOSEngine::vc47() {
	// TODO
	uint a = vcReadNextWord();
	debug(0, "vc47: stub (%d)", a);
}

void AGOSEngine::vc47_addToVar() {
	uint16 var = vcReadNextWord();
	vcWriteVar(var, vcReadVar(var) + vcReadVar(vcReadNextWord()));
}

void AGOSEngine::vc48() {
	// TODO
	uint a = vcReadNextWord();
	debug(0, "vc48: stub (%d)", a);
}

void AGOSEngine::vc48_setPathFinder() {
	uint16 a = (uint16)_variableArrayPtr[12];
	const uint16 *p = _pathFindArray[a - 1];

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		VgaSprite *vsp = findCurSprite();
		int16 x, y, ydiff;
		int16 x1, y1, x2, y2;
		uint pos = 0;

		x = vsp->x;
		while (x >= (int16)readUint16Wrapper(p + 2)) {
			p += 2;
			pos++;
		}

		x1 = readUint16Wrapper(p);
		y1 = readUint16Wrapper(p + 1);
		x2 = readUint16Wrapper(p + 2);
		y2 = readUint16Wrapper(p + 3);

		if (x2 != 9999) {
			ydiff = y2 - y1;
			if (ydiff < 0) {
				ydiff = -ydiff;
				x = vsp->x & 7;
				ydiff *= x;
				ydiff /= 8;
				ydiff = -ydiff;	
			} else {
				x = vsp->x & 7;
				ydiff *= x;
				ydiff /= 8;
			}
			y1 += ydiff;
		}

		y = vsp->y;
		vsp->y = y1;
		checkScrollY(y1 - y, y1);

		_variableArrayPtr[11] = x1;
		_variableArrayPtr[13] = pos;
	} else {
		uint b = (uint16)_variableArray[13];
		p += b * 2 + 1;
		int c = _variableArray[14];

		int step;
		int y1, y2;
		int16 *vp;

		step = 2;
		if (c < 0) {
			c = -c;
			step = -2;
		}

		vp = &_variableArray[20];

		do {
			y2 = readUint16Wrapper(p);
			p += step;
			y1 = readUint16Wrapper(p) - y2;

			vp[0] = y1 / 2;
			vp[1] = y1 - (y1 / 2);

			vp += 2;
		} while (--c);
	}
}

void AGOSEngine::setBitFlag(uint bit, bool value) {
	uint16 *bits = &_bitArray[bit / 16];
	*bits = (*bits & ~(1 << (bit & 15))) | (value << (bit & 15));
}

bool AGOSEngine::getBitFlag(uint bit) {
	uint16 *bits = &_bitArray[bit / 16];
	return (*bits & (1 << (bit & 15))) != 0;
}

void AGOSEngine::vc49_setBit() {
	uint16 bit = vcReadNextWord();
	if (getGameType() == GType_FF && bit == 82) {
		_variableArrayPtr = _variableArray2;
	}
	setBitFlag(bit, true);
}

void AGOSEngine::vc50_clearBit() {
	uint16 bit = vcReadNextWord();
	if (getGameType() == GType_FF && bit == 82) {
		_variableArrayPtr = _variableArray;
	}
	setBitFlag(bit, false);
}

void AGOSEngine::vc51_enableBox() {
	enableBox(vcReadNextWord());
}

void AGOSEngine::vc52_playSound() {
	bool ambient = false;

	uint16 sound = vcReadNextWord();
	if (sound >= 0x8000) {
		ambient = true;
		sound = -sound;
	}

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		int16 pan = vcReadNextWord();
		int16 vol = vcReadNextWord();

		if (ambient)
			loadSound(sound, pan, vol, 2);
		else
			loadSound(sound, pan, vol, 1);
	} else if (getGameType() == GType_SIMON2) {
		if (ambient)
			_sound->playAmbient(sound);
		else
			_sound->playEffects(sound);
	} else if (getFeatures() & GF_TALKIE) {
		_sound->playEffects(sound);
	} else if (getGameId() == GID_SIMON1DOS) {
		playSting(sound);
	}
}

void AGOSEngine::vc53_dissolveIn() {
	// TODO
	debug(0, "vc53_dissolveIn: stub");
}

void AGOSEngine::vc53_panSFX() {
	VgaSprite *vsp = findCurSprite();
	int pan;

	uint16 sound = vcReadNextWord();
	int16 xoffs = vcReadNextWord();
	int16 vol = vcReadNextWord();

	pan = (vsp->x - _scrollX + xoffs) * 8 - 2560;
	if (pan < -10000)
		pan = -10000;
	if (pan > 10000)
		pan = 10000;

	loadSound(sound, 0, vol, 1);

	if (xoffs != 2)
		xoffs |= 0x10;

	addVgaEvent(10, NULL, _vgaCurSpriteId, _vgaCurZoneNum, xoffs); /* pan event */
	debug(0, "vc53_panSFX: snd %d xoffs %d vol %d", sound, xoffs, vol);
}

void AGOSEngine::vc54_dissolveOut() {
	// TODO
	debug(0, "vc54_dissolveOut: stub");
}

void AGOSEngine::vc55_moveBox() {
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);
	uint16 id = vcReadNextWord();
	int16 x = vcReadNextWord();
	int16 y = vcReadNextWord();

	for (;;) {
		if (ha->id == id) {
			ha->x += x;
			ha->y += y;
			break;
		}
		ha++;
		if (!--count)
			break;
	}

	_needHitAreaRecalc++;
}

void AGOSEngine::vc56_fullScreen() {
	byte *src = _curVgaFile2 + 32;
	byte *dst = getBackBuf();

	uint8 palette[1024];
	for (int i = 0; i < 256; i++) {
		palette[i * 4 + 0] = *src++ * 4;
		palette[i * 4 + 1] = *src++ * 4;
		palette[i * 4 + 2] = *src++ * 4;
		palette[i * 4 + 3] = 0;
	}

	_system->setPalette(palette, 0, 256);
	memcpy(dst, src, _screenHeight * _screenWidth);
}

void AGOSEngine::vc56_delayLong() {
	uint16 num = vcReadVarOrWord() * _frameRate;

	addVgaEvent(num + _vgaBaseDelay, _vcPtr, _vgaCurSpriteId, _vgaCurZoneNum);
	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void AGOSEngine::vc57_blackPalette() {
	uint8 palette[1024];
	memset(palette, 0, sizeof(palette));
	_system->setPalette(palette, 0, 256);
}

void AGOSEngine::vc58() {
	uint16 sprite = _vgaCurSpriteId;
	uint16 file = _vgaCurZoneNum;
	const byte *vcPtrOrg;
	uint16 tmp;

	_vgaCurZoneNum = vcReadNextWord();
	_vgaCurSpriteId = vcReadNextWord();

	tmp = to16Wrapper(vcReadNextWord());

	vcPtrOrg = _vcPtr;
	_vcPtr = (byte *)&tmp;
	vc23_setSpritePriority();

	_vcPtr = vcPtrOrg;
	_vgaCurSpriteId = sprite;
	_vgaCurZoneNum = file;
}

void AGOSEngine::vc59() {
	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP) {
		uint16 file = vcReadNextWord();
		uint16 start = vcReadNextWord();
		uint16 end = vcReadNextWord() + 1;

		do {
			vc_kill_sprite(file, start);
		} while (++start != end);
	} else if (getGameType() == GType_SIMON1) {
		if (!_sound->isVoiceActive())
			vcSkipNextInstruction();
	} else {
		// Skip if not EGA
		vcSkipNextInstruction();
	}
}

void AGOSEngine::vc_kill_sprite(uint file, uint sprite) {
	uint16 old_sprite_id, old_cur_file_id;
	VgaSleepStruct *vfs;
	VgaSprite *vsp;
	VgaTimerEntry *vte;
	const byte *vcPtrOrg;

	old_sprite_id = _vgaCurSpriteId;
	old_cur_file_id = _vgaCurZoneNum;
	vcPtrOrg = _vcPtr;

	_vgaCurZoneNum = file;
	_vgaCurSpriteId = sprite;

	vfs = _vgaSleepStructs;
	while (vfs->ident != 0) {
		if (vfs->sprite_id == _vgaCurSpriteId && ((getGameType() == GType_SIMON1) || vfs->cur_vga_file == _vgaCurZoneNum)) {
			while (vfs->ident != 0) {
				memcpy(vfs, vfs + 1, sizeof(VgaSleepStruct));
				vfs++;
			}
			break;
		}
		vfs++;
	}

	vsp = findCurSprite();
	if (vsp->id) {
		vc25_halt_sprite();

		vte = _vgaTimerList;
		while (vte->delay != 0) {
			if (vte->sprite_id == _vgaCurSpriteId && ((getGameType() == GType_SIMON1) || vte->cur_vga_file == _vgaCurZoneNum)) {
				deleteVgaEvent(vte);
				break;
			}
			vte++;
		}
	}

	_vgaCurZoneNum = old_cur_file_id;
	_vgaCurSpriteId = old_sprite_id;
	_vcPtr = vcPtrOrg;
}

void AGOSEngine::vc60_killSprite() {
	uint16 sprite, zoneNum;

	if (getGameType() == GType_PP) {
		zoneNum = vcReadNextWord();
		sprite = vcReadVarOrWord();
	} else if (getGameType() == GType_SIMON2 || getGameType() == GType_FF) {
		zoneNum = vcReadNextWord();
		sprite = vcReadNextWord();
	} else {
		zoneNum = _vgaCurZoneNum;
		sprite = vcReadNextWord();
	}

	vc_kill_sprite(zoneNum, sprite);
}

void AGOSEngine::vc61() {
	uint16 a = vcReadNextWord();
	byte *src, *dst;

	if (a == 6) {
		src = _curVgaFile2 + 800;
		dst = getBackBuf();
		memcpy(dst, src, 64000);
		a = 4;
	}

	src = _curVgaFile2 + 3360;
	dst = getBackBuf() + 3840;

	uint tmp = a;
	while (tmp--) {
		src += 1712;
		dst += 1536;
	}

	src += 800;

	if (a != 5) {
	}

	if (a == 6) {
	}
}

void AGOSEngine::vc61_setMaskImage() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadVarOrWord();
	vsp->x += vcReadNextWord();
	vsp->y += vcReadNextWord();
	vsp->flags = kDFMasked | kDFUseFrontBuf;

	_vgaSpriteChanged++;
}

void AGOSEngine::vc62_fastFadeOut() {
	vc29_stopAllSounds();

	if (!_fastFadeOutFlag) {
		uint i, fadeSize, fadeCount;

		//if (getGameType() != GType_WW)
		//	_fastFadeOutFlag = true;

		_fastFadeCount = 256;
		if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
			if (_windowNum == 4)
				_fastFadeCount = 208;
		}

		memcpy(_videoBuf1, _currentPalette, _fastFadeCount * 4);

		if ((getGameType() == GType_FF || getGameType() == GType_PP) && !getBitFlag(75)) {
			fadeCount = 32;
			fadeSize = 8;
		} else {
			fadeCount = 4;
			fadeSize = 64;
		}

		for (i = fadeCount; i != 0; --i) {
			paletteFadeOut(_videoBuf1, _fastFadeCount, fadeSize);
			_system->setPalette(_videoBuf1, 0, _fastFadeCount);
			delay(5);
		}

		if (getGameType() == GType_SIMON1) {
			uint16 params[5];						/* parameters to vc10_draw */
			VgaSprite *vsp;
			VgaPointersEntry *vpe;
			const byte *vcPtrOrg = _vcPtr;

			vsp = _vgaSprites;
			while (vsp->id != 0) {
				if (vsp->id == 128) {
					byte *old_file_1 = _curVgaFile1;
					byte *old_file_2 = _curVgaFile2;
					uint palmode = _windowNum;

					vpe = &_vgaBufferPointers[vsp->zoneNum];
					_curVgaFile1 = vpe->vgaFile1;
					_curVgaFile2 = vpe->vgaFile2;
					_windowNum = vsp->windowNum;

					params[0] = READ_BE_UINT16(&vsp->image);
					params[1] = READ_BE_UINT16(&vsp->palette);
					params[2] = READ_BE_UINT16(&vsp->x);
					params[3] = READ_BE_UINT16(&vsp->y);
					params[4] = READ_BE_UINT16(&vsp->flags);
					_vcPtr = (byte *)params;
					vc10_draw();

					_windowNum = palmode;
					_curVgaFile1 = old_file_1;
					_curVgaFile2 = old_file_2;
					break;
				}
				vsp++;
			}
			_vcPtr = vcPtrOrg;
		}

		// Allow one section of Simon the Sorcerer 1 introduction to be displayed
		// in lower half of screen
		if ((getGameType() == GType_SIMON1) && (_subroutine == 2923 || _subroutine == 2926)) {
			dx_clear_surfaces(200);
		} else if (getGameType() == GType_FF || getGameType() == GType_PP) {
			dx_clear_surfaces(480);
		} else {
			dx_clear_surfaces(_windowNum == 4 ? 134 : 200);
		}
	}
	if (getGameType() == GType_SIMON2) {
		if (_nextMusicToPlay != -1)
			loadMusic(_nextMusicToPlay);
	}
}

void AGOSEngine::vc63_fastFadeIn() {
	if (getGameType() == GType_PP) {
		_fastFadeInFlag = 256;
		if (getBitFlag(100)) {
			startOverlayAnims();
		} else if (getBitFlag(103)) {
			printf("NameAndTime\n");
		} else if (getBitFlag(104)) {
			printf("HiScoreTable\n");
		}
	} else if (getGameType() == GType_FF) {
		_fastFadeInFlag = 256;
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		_fastFadeInFlag = 208;
		if (_windowNum != 4) {
			_fastFadeInFlag = 256;
		}
	}
	_fastFadeOutFlag = false;
}

void AGOSEngine::vc64_skipIfSpeechEnded() {
	if ((getGameType() == GType_SIMON2 && _subtitles && _language != Common::HB_ISR) ||
		!_sound->isVoiceActive()) {
		vcSkipNextInstruction();
	}
}

void AGOSEngine::vc65_slowFadeIn() {
	_fastFadeInFlag = 624;
	_fastFadeCount = 208;
	if (_windowNum != 4) {
		_fastFadeInFlag = 768;
		_fastFadeCount = 256;
	}
	_fastFadeInFlag |= 0x8000;
	_fastFadeOutFlag = false;
}

void AGOSEngine::vc66_skipIfNotEqual() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();

	if (vcReadVar(a) != vcReadVar(b))
		vcSkipNextInstruction();
}

void AGOSEngine::vc67_skipIfGE() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();

	if (vcReadVar(a) >= vcReadVar(b))
		vcSkipNextInstruction();
}

void AGOSEngine::vc68_skipIfLE() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();

	if (vcReadVar(a) <= vcReadVar(b))
		vcSkipNextInstruction();
}

void AGOSEngine::vc69_playTrack() {
	int16 track = vcReadNextWord();
	int16 loop = vcReadNextWord();

	// Jamieson630:
	// This is a "play track". The original
	// design stored the track to play if one was
	// already in progress, so that the next time a
	// "fill MIDI stream" event occured, the MIDI
	// player would find the change and switch
	// tracks. We use a different architecture that
	// allows for an immediate response here, but
	// we'll simulate the variable changes so other
	// scripts don't get thrown off.
	// NOTE: This opcode looks very similar in function
	// to vc72(), except that vc72() may allow for
	// specifying a non-valid track number (999 or -1)
	// as a means of stopping what music is currently
	// playing.
	midi.setLoop(loop != 0);
	midi.startTrack(track);
}

void AGOSEngine::vc70_queueMusic() {
	// Simon2
	uint16 track = vcReadNextWord();
	uint16 loop = vcReadNextWord();

	// Jamieson630:
	// This sets the "on end of track" action.
	// It specifies whether to loop the current
	// track and, if not, whether to switch to
	// a different track upon completion.
	if (track != 0xFFFF && track != 999)
		midi.queueTrack(track, loop != 0);
	else
		midi.setLoop(loop != 0);
}

void AGOSEngine::vc71_checkMusicQueue() {
	// Jamieson630:
	// This command skips the next instruction
	// unless (1) there is a track playing, AND
	// (2) there is a track queued to play after it.
	if (!midi.isPlaying (true))
		vcSkipNextInstruction();
}

void AGOSEngine::vc72_play_track_2() {
	// Jamieson630:
	// This is a "play or stop track". Note that
	// this opcode looks very similar in function
	// to vc69(), except that this opcode may allow
	// for specifying a track of 999 or -1 in order to
	// stop the music. We'll code it that way for now.

	// NOTE: It's possible that when "stopping" a track,
	// we're supposed to just go on to the next queued
	// track, if any. Must find out if there is ANY
	// case where this is used to stop a track in the
	// first place.

	int16 track = vcReadNextWord();
	int16 loop = vcReadNextWord();

	if (track == -1 || track == 999) {
		midi.stop();
	} else {
		midi.setLoop (loop != 0);
		midi.startTrack (track);
	}
}

void AGOSEngine::vc73_setMark() {
	_marks |= (1 << vcReadNextWord());
}

void AGOSEngine::vc74_clearMark() {
	_marks &= ~(1 << vcReadNextWord());
}

int AGOSEngine::getScale(int16 y, int16 x) {
	int16 z;

	if (y > _baseY) {
		return((int16)(x * (1 + ((y - _baseY) * _scale))));
	} else {	
		if (x == 0)
			return(0);
		if (x < 0) {
			z = ((int16)((x * (1 - ((_baseY - y)* _scale))) - 0.5));
			if (z >- 2)
				return(-2);
			return(z);
		}

		z = ((int16)((x * (1 - ((_baseY - y) * _scale))) + 0.5));
		if (z < 2)
			return(2);

		return(z);
	}
}

void AGOSEngine::vc75_setScale() {
	_baseY = vcReadNextWord();
	_scale = (float)vcReadNextWord() / 1000000.;
}

void AGOSEngine::vc76_setScaleXOffs() {
	if (getGameType() == GType_PP && getBitFlag(120)) {
		VgaSprite *vsp1, *vsp2;
		uint16 old_file_1, tmp1, tmp2;

		old_file_1 = _vgaCurSpriteId;

		_vgaCurSpriteId = vcReadVar(vcReadNextWord());
		 vsp1 = findCurSprite();
		_vgaCurSpriteId = vcReadVar(vcReadNextWord());
		 vsp2 = findCurSprite();

		tmp1 = vsp1->x;
		tmp2 = vsp2->x;
		vsp1->x = tmp2;
		vsp2->x = tmp1;
		tmp1 = vsp1->y;
		tmp2 = vsp2->y;
		vsp1->y = tmp2;
		vsp2->y = tmp1;

		_vgaCurSpriteId = old_file_1;
		_vcPtr += 2;
	} else {
		VgaSprite *vsp = findCurSprite();

		vsp->image = vcReadNextWord();
		int16 x = vcReadNextWord();
		uint16 var = vcReadNextWord();

		vsp->x += getScale(vsp->y, x);
		_variableArrayPtr[var] = vsp->x;

		checkScrollX(x, vsp->x);

		vsp->flags = kDFScaled;
	}
}

void AGOSEngine::vc77_setScaleYOffs() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadNextWord();
	int16 y = vcReadNextWord();
	uint16 var = vcReadNextWord();

	vsp->y += getScale(vsp->y, y);
	_variableArrayPtr[var] = vsp->y;

	if (y != 0) 
		checkScrollY(y, vsp->y);

	vsp->flags = kDFScaled;
}

void AGOSEngine::vc78_computeXY() {
	VgaSprite *vsp = findCurSprite();

	uint16 a = (uint16)_variableArrayPtr[12];
	uint16 b = (uint16)_variableArrayPtr[13];

	const uint16 *p = _pathFindArray[a - 1];
	p += b * 2;

	uint16 posx = readUint16Wrapper(p);
	_variableArrayPtr[15] = posx;
	vsp->x = posx;

	uint16 posy = readUint16Wrapper(p + 1);
	_variableArrayPtr[16] = posy;
	vsp->y = posy;

	if (getGameType() == GType_FF) {
		setBitFlag(85, false);
		if (getBitFlag(74)) {
			centreScroll();
		}
	}
}

void AGOSEngine::vc79_computePosNum() {
	uint a = (uint16)_variableArrayPtr[12];
	const uint16 *p = _pathFindArray[a - 1];
	uint pos = 0;

	int16 y = _variableArrayPtr[16];
	while (y >= (int16)readUint16Wrapper(p + 1)) {
		p += 2;
		pos++;
	}

	_variableArrayPtr[13] = pos;
}

void AGOSEngine::vc80_setOverlayImage() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadVarOrWord();

	vsp->x += vcReadNextWord();
	vsp->y += vcReadNextWord();
	vsp->flags = kDFOverlayed;

	_vgaSpriteChanged++;
}

void AGOSEngine::vc81_setRandom() {
	uint16 var = vcReadNextWord();
	uint16 value = vcReadNextWord();

	_variableArray[var] = _rnd.getRandomNumber(value - 1);
}

void AGOSEngine::vc82_getPathValue() {
	uint8 val;

	uint16 var = vcReadNextWord();

	if (getGameType() == GType_FF && getBitFlag(82)) {
		val = _pathValues1[_GPVCount1++];
	} else {
		val = _pathValues[_GPVCount++];
	}

	vcWriteVar(var, val);
}

void AGOSEngine::vc83_playSoundLoop() {
	uint16 sound = vcReadNextWord();
	int16 vol = vcReadNextWord();
	int16 pan = vcReadNextWord();

	loadSound(sound, pan, vol, 3);
}

void AGOSEngine::vc84_stopSoundLoop() {
	_sound->stopSfx5();
}

// Scrolling functions for Feeble Files
void AGOSEngine::checkScrollX(int16 x, int16 xpos) {
	if (_scrollXMax == 0 || x == 0)
		return;

	if ((getGameType() == GType_FF) && (getBitFlag(80) || getBitFlag(82)))
		return;

	int16 tmp;
	if (x > 0) {
		if (_scrollCount != 0) {
			if (_scrollCount >= 0)
				return;
			_scrollCount = 0;
		} else {
			if (_scrollFlag != 0)
				return;
		}

		if (xpos - _scrollX >= 480) {
			_scrollCount = 320;
			tmp = _scrollXMax - _scrollX;
			if (tmp < 320)
				_scrollCount = tmp;
		}
	} else {
		if (_scrollCount != 0) {
			if (_scrollCount < 0)
				return;
			_scrollCount = 0;
		} else {
			if (_scrollFlag != 0)
				return;
		}

		if (xpos - _scrollX < 161) {
			_scrollCount = -320;
			if (_scrollX < 320)
				_scrollCount = -_scrollX;
		}
	}
}

void AGOSEngine::checkScrollY(int16 y, int16 ypos) {
	if (_scrollYMax == 0)
		return;

	if (getGameType() == GType_FF && getBitFlag(80))
		return;

	int16 tmp;
	if (y >= 0) {
		if (_scrollCount != 0) {
			if (_scrollCount >= 0)
				return;
		} else {
			if (_scrollFlag != 0)
				return;
		}

		if (ypos - _scrollY >= 440) {
			_scrollCount = 240;
			tmp = _scrollYMax - _scrollY;
			if (tmp < 240)
				_scrollCount = tmp;
		}
	} else {
		if (_scrollCount != 0) {
			if (_scrollCount < 0)
				return;
		} else {
			if (_scrollFlag != 0)
				return;
		}

		if (ypos - _scrollY < 100) {
			_scrollCount = -240;
			if (_scrollY < 240)
				_scrollCount = -_scrollY;
		}
	}
}

void AGOSEngine::centreScroll() {
	int16 x, y, tmp;

	if (_scrollXMax != 0) {
		_scrollCount = 0;
		x = _variableArray[15] - _scrollX;
		if (x < 17 || (getBitFlag(85) && x < 320)) {
			x -= 320;
			if (_scrollX < -x)
				x = -_scrollX;
			_scrollCount = x;
		} else if ((getBitFlag(85) && x >= 320) || x >= 624) {
			x -= 320;
			tmp = _scrollXMax - _scrollX;
			if (tmp < x)
				x = tmp;
			_scrollCount = x;
		}
	} else if (_scrollYMax != 0) {
		_scrollCount = 0;
		y = _variableArray[16] - _scrollY;
		if (y < 30) {
			y -= 240;
			if (_scrollY < -y)
				y = -_scrollY;
			_scrollCount = y;
		} else if (y >= 460) {
			y -= 240;
			tmp = _scrollYMax - _scrollY;
			if (tmp < y)
				y = tmp;
			_scrollCount = y;
		}
	}
}

void AGOSEngine::startOverlayAnims() {
	VgaSprite *vsp = _vgaSprites;
	uint16 zoneNum;
	int i;

	zoneNum = _variableArray[999];
	
	for (i = 0; i < 600; i++) {
		if (_variableArray[1000 + i] < 100)
			continue;

		while (vsp->id)
			vsp++;

		vsp->windowNum = 4;
		vsp->priority = 4;
		vsp->flags = 0;
		vsp->palette = 0;
		vsp->image = _variableArray[1000 + i];
		if (i >= 300) {
			vsp->y = ((i - 300) / 20) * 32;
			vsp->x = ((i - 300) % 20) * 32;
		} else {
			vsp->y = (i / 20) * 32;
			vsp->x = (i % 20) * 32;
		}
		vsp->id = 1000 + i;
		vsp->zoneNum = zoneNum;
	}
}

void AGOSEngine::startAnOverlayAnim() {
	VgaSprite *vsp = _vgaSprites;
	const byte *vcPtrOrg;
	uint16 a, sprite, file, tmp, zoneNum;
	int16 x;

	zoneNum = _variableArray[999];

	_vcPtr += 4;
	a = vcReadNextWord();
	_vcPtr += 6;

	while (vsp->id)
		vsp++;

	vsp->windowNum = 4;
	vsp->priority = 20;
	vsp->flags = 0;
	vsp->palette = 0;
	vsp->image = vcReadVar(vcReadVar(a));

	x = vcReadVar(a) - 1300;
	if (x < 0) {
		x += 300;
		vsp->priority = 10;
	}

	vsp->y = x / 20 * 32;
	vsp->x = x % 20 * 32;;
	vsp->id = vcReadVar(a);
	vsp->zoneNum = zoneNum;

	sprite = _vgaCurSpriteId;
	file = _vgaCurZoneNum;

	_vgaCurZoneNum = vsp->zoneNum;
	_vgaCurSpriteId = vsp->id;

	tmp = to16Wrapper(vsp->priority);

	vcPtrOrg = _vcPtr;
	_vcPtr = (byte *)&tmp;
	vc23_setSpritePriority();

	_vcPtr = vcPtrOrg;
	_vgaCurSpriteId = sprite;
	_vgaCurZoneNum = file;
}

} // End of namespace AGOS
