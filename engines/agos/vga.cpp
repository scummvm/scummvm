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
	op[5] = &AGOSEngine::vc5_ifEqual;
	op[6] = &AGOSEngine::vc6_ifObjectHere;
	op[7] = &AGOSEngine::vc7_ifObjectNotHere;
	op[8] = &AGOSEngine::vc8_ifObjectIsAt;
	op[9] = &AGOSEngine::vc9_ifObjectStateIs;
	op[10] = &AGOSEngine::vc10_draw;
	op[12] = &AGOSEngine::vc12_delay;
	op[13] = &AGOSEngine::vc13_addToSpriteX;
	op[14] = &AGOSEngine::vc14_addToSpriteY;
	op[15] = &AGOSEngine::vc15_sync;
	op[16] = &AGOSEngine::vc16_waitSync;
	op[18] = &AGOSEngine::vc18_jump;
	op[20] = &AGOSEngine::vc20_setRepeat;
	op[21] = &AGOSEngine::vc21_endRepeat;
	op[23] = &AGOSEngine::vc23_setPriority;
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
	op[38] = &AGOSEngine::vc38_ifVarNotZero;
	op[39] = &AGOSEngine::vc39_setVar;
	op[40] = &AGOSEngine::vc40;
	op[41] = &AGOSEngine::vc41;
	op[42] = &AGOSEngine::vc42_delayIfNotEQ;
	op[43] = &AGOSEngine::vc43_ifBitSet;
	op[44] = &AGOSEngine::vc44_ifBitClear;
	op[45] = &AGOSEngine::vc45_setSpriteX;
	op[46] = &AGOSEngine::vc46_setSpriteY;
	op[47] = &AGOSEngine::vc47_addToVar;
	op[49] = &AGOSEngine::vc49_setBit;
	op[50] = &AGOSEngine::vc50_clearBit;
	op[51] = &AGOSEngine::vc51_enableBox;
	op[52] = &AGOSEngine::vc52_playSound;
	op[55] = &AGOSEngine::vc55_moveBox;
}

void AGOSEngine::setupElvira1VideoOpcodes(VgaOpcodeProc *op) {
	op[1] = &AGOSEngine::vc1_fadeOut;
	op[2] = &AGOSEngine::vc2_call;
	op[3] = &AGOSEngine::vc3_loadSprite;
	op[4] = &AGOSEngine::vc4_fadeIn;
	op[5] = &AGOSEngine::vc5_ifEqual;
	op[6] = &AGOSEngine::vc6_ifObjectHere;
	op[7] = &AGOSEngine::vc7_ifObjectNotHere;
	op[8] = &AGOSEngine::vc8_ifObjectIsAt;
	op[9] = &AGOSEngine::vc9_ifObjectStateIs;
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
	op[23] = &AGOSEngine::vc22_setPaletteOld;
	op[24] = &AGOSEngine::vc23_setPriority;
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
	op[51] = &AGOSEngine::vc38_ifVarNotZero;
	op[52] = &AGOSEngine::vc39_setVar;
	op[53] = &AGOSEngine::vc40;
	op[54] = &AGOSEngine::vc41;
	op[56] = &AGOSEngine::vc42_delayIfNotEQ;
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

// VGA Script parser
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

bool AGOSEngine::isSpriteLoaded(uint16 id, uint16 zoneNum) {
	VgaSprite *vsp = _vgaSprites;
	while (vsp->id) {
		if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP) {
			if (vsp->id == id && vsp->zoneNum == zoneNum)
				return true;
		} else {
			if (vsp->id == id)
				return true;
		}
		vsp++;
	}
	return false;
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

bool AGOSEngine::getBitFlag(uint bit) {
	uint16 *bits = &_bitArray[bit / 16];
	return (*bits & (1 << (bit & 15))) != 0;
}

void AGOSEngine::setBitFlag(uint bit, bool value) {
	uint16 *bits = &_bitArray[bit / 16];
	*bits = (*bits & ~(1 << (bit & 15))) | (value << (bit & 15));
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
		0, 6,  2, 10, 6, 4, 2, 2,
		4, 4,  8,  2, 2, 2, 2, 2,
		2, 2,  2,  0, 4, 2, 2, 2,
		8, 0, 10,  0, 8, 0, 2, 2,
		0, 0,  0,  4, 4, 4, 2, 4,
		4, 4,  4,  2, 2, 4, 2, 2,
		2, 2,  2,  2, 2, 4, 6, 6,
		0, 0,  0,  0, 2, 2, 0, 0,
	};

	static const byte opcodeParamLenSimon1[] = {
		0, 6,  2, 10, 6, 4, 2, 2,
		4, 4, 10,  0, 2, 2, 2, 2,
		2, 0,  2,  0, 4, 2, 4, 2,
		8, 0, 10,  0, 8, 0, 2, 2,
		4, 0,  0,  4, 4, 2, 2, 4,
		4, 4,  4,  2, 2, 2, 2, 4,
		0, 2,  2,  2, 2, 4, 6, 6,
		0, 0,  0,  0, 2, 6, 0, 0,
	};

	static const byte opcodeParamLenSimon2[] = {
		0, 6,  2, 12, 6, 4, 2, 2,
		4, 4,  9,  0, 1, 2, 2, 2,
		2, 0,  2,  0, 4, 2, 4, 2,
		7, 0, 10,  0, 8, 0, 2, 2,
		4, 0,  0,  4, 4, 2, 2, 4,
		4, 4,  4,  2, 2, 2, 2, 4,
		0, 2,  2,  2, 2, 4, 6, 6,
		2, 0,  6,  6, 4, 6, 0, 0,
		0, 0,  4,  4, 4, 4, 4, 0,
		4, 2,  2
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
	uint16 num;
	byte *old_file_1, *old_file_2;

	if (getGameType() == GType_ELVIRA2) {
		num = vcReadNextWord();
	} else {
		num = vcReadVarOrWord();
	}

	old_file_1 = _curVgaFile1;
	old_file_2 = _curVgaFile2;

	setImage(num, true);

	_curVgaFile1 = old_file_1;
	_curVgaFile2 = old_file_2;
}

void AGOSEngine::vc3_loadSprite() {
	uint16 windowNum, zoneNum, palette, x, y, vgaSpriteId;
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

	old_file_1 = _curVgaFile1;

	animate(windowNum, zoneNum, vgaSpriteId, x, y, palette, true);

	_curVgaFile1 = old_file_1;
}

void AGOSEngine::vc4_fadeIn() {
	/* dummy opcode */
	_vcPtr += 6;
}

void AGOSEngine::vc5_ifEqual() {
	uint16 var;

	if (getGameType() == GType_PP)
		var = vcReadVarOrWord();
	else
		var = vcReadNextWord();

	uint16 value = vcReadNextWord();
	if (vcReadVar(var) != value)
		vcSkipNextInstruction();
}

void AGOSEngine::vc6_ifObjectHere() {
	if (!itemIsSiblingOf(vcReadNextWord()))
		vcSkipNextInstruction();
}

void AGOSEngine::vc7_ifObjectNotHere() {
	if (itemIsSiblingOf(vcReadNextWord()))
		vcSkipNextInstruction();
}

void AGOSEngine::vc8_ifObjectIsAt() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();
	if (!itemIsParentOf(a, b))
		vcSkipNextInstruction();
}

void AGOSEngine::vc9_ifObjectStateIs() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();
	if (!vc_maybe_skip_proc_1(a, b))
		vcSkipNextInstruction();
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
		dumpSingleBitmap(_vgaCurZoneNum, state.image, state.depack_src, width, height,
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
	VgaSleepStruct *vfs = _waitSyncTable, *vfs_tmp;
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
	VgaSleepStruct *vfs = _waitSyncTable;
	while (vfs->ident)
		vfs++;

	vfs->ident = vcReadNextWord();
	vfs->code_ptr = _vcPtr;
	vfs->sprite_id = _vgaCurSpriteId;
	vfs->cur_vga_file = _vgaCurZoneNum;

	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void AGOSEngine::checkWaitEndTable() {
	VgaSleepStruct *vfs = _waitEndTable, *vfs_tmp;
	while (vfs->ident != 0) {
		if (vfs->ident == _vgaCurSpriteId) {
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
}

void AGOSEngine::vc17_waitEnd() {
	VgaSleepStruct *vfs = _waitEndTable;
	while (vfs->ident)
		vfs++;

	vfs->ident = vcReadNextWord();
	vfs->code_ptr = _vcPtr;
	vfs->sprite_id = _vgaCurSpriteId;
	vfs->cur_vga_file = _vgaCurZoneNum;

	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void AGOSEngine::vc18_jump() {
	int16 offs = vcReadNextWord();
	_vcPtr += offs;
}

void AGOSEngine::vc19_loop() {
	uint16 count;
	byte *b, *bb;

	bb = _curVgaFile1;
	b = _curVgaFile1 + READ_BE_UINT16(bb + 10);
	b += 20;

	count = READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->animationCount);
	b = bb + READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->animationTable);

	while (count--) {
		if (READ_BE_UINT16(&((AnimationHeader_WW *) b)->id) == _vgaCurSpriteId)
			break;
		b += sizeof(AnimationHeader_WW);
	}
	assert(READ_BE_UINT16(&((AnimationHeader_WW *) b)->id) == _vgaCurSpriteId);

	_vcPtr = _curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_WW *) b)->scriptOffs);
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

void AGOSEngine::vc22_setPaletteOld() {
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

void AGOSEngine::vc23_setPriority() {
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
	checkWaitEndTable();

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
		if ((getGameType() == GType_SIMON1 && vsp->id == 128) ||
			(getGameType() == GType_ELVIRA2 && vsp->id == 100)) {
			memcpy(&bak, vsp, sizeof(VgaSprite));
		}
		vsp->id = 0;
		vsp++;
	}

	if (bak.id != 0)
		memcpy(_vgaSprites, &bak, sizeof(VgaSprite));

	vfs = _waitEndTable;
	while (vfs->ident) {
		vfs->ident = 0;
		vfs++;
	}

	vfs = _waitSyncTable;
	while (vfs->ident) {
		vfs->ident = 0;
		vfs++;
	}

	vte = _vgaTimerList;
	while (vte->delay) {
		if ((getGameType() == GType_SIMON1 && vte->sprite_id == 128) ||
			(getGameType() == GType_ELVIRA2 && vte->sprite_id == 100)) {
			vte++;
		} else {
			vte2 = vte;
			while (vte2->delay) {
				memcpy(vte2, vte2 + 1, sizeof(VgaTimerEntry));
				vte2++;
			}
		}
	}

	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP)
		vcWriteVar(254, 0);

	if (getGameType() == GType_FF || getGameType() == GType_PP)
		setBitFlag(42, true);

	_lockWord &= ~8;
}

void AGOSEngine::vc28_playSFX() {
	byte *dst;
	uint sound, channels, frequency, flags;
	uint offs, size;

	sound = vcReadNextWord();
	channels = vcReadNextWord();
	frequency = vcReadNextWord();
	flags = vcReadNextWord();

	debug(0, "vc28_playSFX: (%d, %d, %d, %d)", sound, channels, frequency, flags);

	if (_curSfxFile == NULL)
		return;

	dst = _curSfxFile;
	if (getGameType() == GType_WW) {
		uint tmp = sound;
		while (tmp--)
			dst += READ_LE_UINT16(dst) + 4;

		size = READ_LE_UINT16(dst);
		offs = 4;
	} else {
		while (READ_BE_UINT16(dst + 6) != sound)
			dst += 12;

		size = READ_BE_UINT16(dst + 2);
		offs = READ_BE_UINT32(dst + 8);
	}

	_sound->playRawData(dst + offs, sound, size);
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

void AGOSEngine::vc33_setMouseOn() {
	if (_mouseHideCount != 0) {
		_mouseHideCount = 1;
		mouseOn();
	}
}

void AGOSEngine::vc34_setMouseOff() {
	// FIXME
	if (getGameType() != GType_ELVIRA1) {
		mouseOff();
		_mouseHideCount = 200;
	}
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
			setWindowImage(windowNum, vga_res);
		}
	} else {
		setWindowImage(windowNum, vga_res);
	}
}

void AGOSEngine::vc37_pokePalette() {
	uint16 offs = vcReadNextWord();
	uint16 color = vcReadNextWord();

	byte *palptr = _displayPalette + offs * 4;
	palptr[0] = ((color & 0xf00) >> 8) * 32;
	palptr[1] = ((color & 0x0f0) >> 4) * 32;
	palptr[2] = ((color & 0x00f) >> 0) * 32;
	palptr[3] = 0;

	_paletteFlag = 2;
	_vgaSpriteChanged++;
}

void AGOSEngine::vc38_ifVarNotZero() {
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

} // End of namespace AGOS
