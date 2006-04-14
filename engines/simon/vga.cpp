/* ScummVM - Scumm Interpreter
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
#include "simon/simon.h"
#include "simon/intern.h"
#include "simon/vga.h"

#include "common/system.h"

namespace Simon {

// Opcode tables
void SimonEngine::setupVgaOpcodes() {
	static const VgaOpcodeProc vga_opcode_table[] = {
		NULL,
		&SimonEngine::vc1_fadeOut,
		&SimonEngine::vc2_call,
		&SimonEngine::vc3_loadSprite,
		&SimonEngine::vc4_fadeIn,
		&SimonEngine::vc5_skip_if_neq,
		&SimonEngine::vc6_skip_ifn_sib_with_a,
		&SimonEngine::vc7_skip_if_sib_with_a,
		&SimonEngine::vc8_skip_if_parent_is,
		&SimonEngine::vc9_skip_if_unk3_is,
		&SimonEngine::vc10_draw,
		&SimonEngine::vc11_clearPathFinder,
		&SimonEngine::vc12_delay,
		&SimonEngine::vc13_addToSpriteX,
		&SimonEngine::vc14_addToSpriteY,
		&SimonEngine::vc15_wakeup_id,
		&SimonEngine::vc16_sleep_on_id,
		&SimonEngine::vc17_setPathfinderItem,
		&SimonEngine::vc18_jump,
		&SimonEngine::vc19_chain_to_script,
		&SimonEngine::vc20_setRepeat,
		&SimonEngine::vc21_endRepeat,
		&SimonEngine::vc22_setSpritePalette,
		&SimonEngine::vc23_setSpritePriority,
		&SimonEngine::vc24_setSpriteXY,
		&SimonEngine::vc25_halt_sprite,
		&SimonEngine::vc26_setSubWindow,
		&SimonEngine::vc27_resetSprite,
		&SimonEngine::vc28_dummy_op,
		&SimonEngine::vc29_stopAllSounds,
		&SimonEngine::vc30_setFrameRate,
		&SimonEngine::vc31_setWindow,
		&SimonEngine::vc32_copyVar,
		&SimonEngine::vc33_setMouseOn,
		&SimonEngine::vc34_setMouseOff,
		&SimonEngine::vc35_clearWindow,
		&SimonEngine::vc36_setWindowImage,
		&SimonEngine::vc37_addToSpriteY,
		&SimonEngine::vc38_skipIfVarZero,
		&SimonEngine::vc39_setVar,
		&SimonEngine::vc40,
		&SimonEngine::vc41,
		&SimonEngine::vc42_delayIfNotEQ,
		&SimonEngine::vc43_skipIfBitClear,
		&SimonEngine::vc44_skipIfBitSet,
		&SimonEngine::vc45_setSpriteX,
		&SimonEngine::vc46_setSpriteY,
		&SimonEngine::vc47_addToVar,
		&SimonEngine::vc48_setPathFinder,
		&SimonEngine::vc49_setBit,
		&SimonEngine::vc50_clearBit,
		&SimonEngine::vc51_clear_hitarea_bit_0x40,
		&SimonEngine::vc52_playSound,
		&SimonEngine::vc53_no_op,
		&SimonEngine::vc54_no_op,
		&SimonEngine::vc55_offset_hit_area,
		&SimonEngine::vc56_delay,
		&SimonEngine::vc57_no_op,
		&SimonEngine::vc58,
		&SimonEngine::vc59,
		&SimonEngine::vc60_killSprite,
		&SimonEngine::vc61_setMaskImage,
		&SimonEngine::vc62_fastFadeOut,
		&SimonEngine::vc63_fastFadeIn,
		&SimonEngine::vc64_skipIfSpeechEnded,
		&SimonEngine::vc65_slowFadeIn,
		&SimonEngine::vc66_skipIfNotEqual,
		&SimonEngine::vc67_skipIfGE,
		&SimonEngine::vc68_skipIfLE,
		&SimonEngine::vc69_playTrack,
		&SimonEngine::vc70_queueMusic,
		&SimonEngine::vc71_checkMusicQueue,
		&SimonEngine::vc72_play_track_2,
		&SimonEngine::vc73_setMark,
		&SimonEngine::vc74_clearMark,
		&SimonEngine::vc75_setScale,
		&SimonEngine::vc76_setScaleXOffs,
		&SimonEngine::vc77_setScaleYOffs,
		&SimonEngine::vc78_computeXY,
		&SimonEngine::vc79_computePosNum,
		&SimonEngine::vc80_setOverlayImage,
		&SimonEngine::vc81_setRandom,
		&SimonEngine::vc82_getPathValue,
		&SimonEngine::vc83_playSoundLoop,
		&SimonEngine::vc84_stopSoundLoop,
	};
	
	_vga_opcode_table = vga_opcode_table;
}

// Script parser
void SimonEngine::run_vga_script() {
	for (;;) {
		uint opcode;

		if (_continousVgaScript) {
			if (_vcPtr != (const byte *)&_vc_get_out_of_code) {
				fprintf(_dumpFile, "%.5d %.5X: %5d %4d ", _vgaTickCounter, _vcPtr - _curVgaFile1, _vgaCurSpriteId, _vgaCurZoneNum);
				dump_video_script(_vcPtr, true);
			}
		}

		if (getGameType() == GType_SIMON1) {
			opcode = READ_BE_UINT16(_vcPtr);
			_vcPtr += 2;
		} else {
			opcode = *_vcPtr++;
		}

		if (opcode >= NUM_VIDEO_OP_CODES)
			error("Invalid VGA opcode '%d' encountered", opcode);

		if (opcode == 0)
			return;

		(this->*_vga_opcode_table[opcode]) ();
	}
}

int SimonEngine::vcReadVarOrWord() {
	int16 var = vcReadNextWord();
	if (var < 0)
		var = vcReadVar(-var);
	return var;
}

uint SimonEngine::vcReadNextWord() {
	uint a;
	a = readUint16Wrapper(_vcPtr);
	_vcPtr += 2;
	return a;
}

uint SimonEngine::vcReadNextByte() {
	return *_vcPtr++;
}

void SimonEngine::vcSkipNextInstruction() {
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

	if (getGameType() == GType_FF) {
		uint opcode = vcReadNextByte();
		_vcPtr += opcodeParamLenFeebleFiles[opcode];
	} else if (getGameType() == GType_SIMON2) {
		uint opcode = vcReadNextByte();
		_vcPtr += opcodeParamLenSimon2[opcode];
	} else {
		uint opcode = vcReadNextWord();
		_vcPtr += opcodeParamLenSimon1[opcode];
	}

	if (_continousVgaScript)
		fprintf(_dumpFile, "; skipped\n");
}

// VGA Script commands
void SimonEngine::vc1_fadeOut() {
	/* dummy opcode */
	_vcPtr += 6;
}

void SimonEngine::vc2_call() {
	VgaPointersEntry *vpe;
	uint num;
	uint res;
	byte *old_file_1, *old_file_2;
	byte *b, *bb;
	const byte *vc_ptr_org;

	num = vcReadVarOrWord();
	if (getGameType() == GType_FF)
		num &= 0xFFFF;

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
	if (getGameType() == GType_FF) {
		b = bb + READ_LE_UINT16(&((VgaFileHeader_Feeble *) bb)->hdr2_start);
		b = bb + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) b)->imageTable);

		while (READ_LE_UINT16(&((ImageHeader_Feeble *) b)->id) != num)
			b += sizeof(ImageHeader_Feeble);
	} else {
		b = bb + READ_BE_UINT16(&((VgaFileHeader_Simon *) bb)->hdr2_start);
		b = bb + READ_BE_UINT16(&((VgaFileHeader2_Simon *) b)->imageTable);

		while (READ_BE_UINT16(&((ImageHeader_Simon *) b)->id) != num)
			b += sizeof(ImageHeader_Simon);
	}

	vc_ptr_org = _vcPtr;

	if (getGameType() == GType_FF) {
		_vcPtr = _curVgaFile1 + READ_LE_UINT16(&((ImageHeader_Feeble *) b)->scriptOffs);
	} else {
		_vcPtr = _curVgaFile1 + READ_BE_UINT16(&((ImageHeader_Simon *) b)->scriptOffs);
	}

	//dump_vga_script(_vcPtr, res, num);
	run_vga_script();

	_curVgaFile1 = old_file_1;
	_curVgaFile2 = old_file_2;

	_vcPtr = vc_ptr_org;
}

void SimonEngine::vc3_loadSprite() {
	uint16 windowNum, zoneNum, palette, x, y, vgaSpriteId;
	uint16 res;
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p, *pp;
	byte *old_file_1;

	windowNum = vcReadNextWord();		/* 0 */

	if (getGameType() == GType_SIMON1) {
		vgaSpriteId = vcReadNextWord();	/* 2 */
		zoneNum = vgaSpriteId / 100;
	} else {
		zoneNum = vcReadNextWord();	/* 0 */
		vgaSpriteId = vcReadNextWord();	/* 2 */
	}

	x = vcReadNextWord();			/* 4 */
	y = vcReadNextWord();			/* 6 */
	palette = vcReadNextWord();		/* 8 */

	/* 2nd param ignored with simon1 */
	if (isSpriteLoaded(vgaSpriteId, zoneNum))
		return;

	vsp = _vgaSprites;
	while (vsp->id)
		vsp++;

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
	if (getGameType() == GType_FF) {
		p = pp + READ_LE_UINT16(&((VgaFileHeader_Feeble *) pp)->hdr2_start);
		p = pp + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) p)->animationTable);

		while (READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->id) != vgaSpriteId)
			p += sizeof(AnimationHeader_Feeble);
	} else {
		p = pp + READ_BE_UINT16(&((VgaFileHeader_Simon *) pp)->hdr2_start);
		p = pp + READ_BE_UINT16(&((VgaFileHeader2_Simon *) p)->animationTable);

		while (READ_BE_UINT16(&((AnimationHeader_Simon *) p)->id) != vgaSpriteId)
			p += sizeof(AnimationHeader_Simon);
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
		if (getGameType() == GType_FF) {
			dump_vga_script(_curVgaFile1 + READ_LE_UINT16(&((AnimationHeader_Feeble*)p)->scriptOffs), res, vgaSpriteId);
		} else {
			dump_vga_script(_curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_Simon*)p)->scriptOffs), res, vgaSpriteId);

		}
	}

	if (getGameType() == GType_FF) {
		add_vga_timer(VGA_DELAY_BASE, _curVgaFile1 + READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->scriptOffs), vgaSpriteId, res);
	} else {
		add_vga_timer(VGA_DELAY_BASE, _curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_Simon *) p)->scriptOffs), vgaSpriteId, res);
	}

	_curVgaFile1 = old_file_1;
}

void SimonEngine::vc4_fadeIn() {
	/* dummy opcode */
	_vcPtr += 6;
}

void SimonEngine::vc5_skip_if_neq() {
	uint var = vcReadNextWord();
	uint value = vcReadNextWord();
	if (vcReadVar(var) != value)
		vcSkipNextInstruction();
}

void SimonEngine::vc6_skip_ifn_sib_with_a() {
	if (!itemIsSiblingOf(vcReadNextWord()))
		vcSkipNextInstruction();
}

void SimonEngine::vc7_skip_if_sib_with_a() {
	if (itemIsSiblingOf(vcReadNextWord()))
		vcSkipNextInstruction();
}

void SimonEngine::vc8_skip_if_parent_is() {
	uint a = vcReadNextWord();
	uint b = vcReadNextWord();
	if (!itemIsParentOf(a, b))
		vcSkipNextInstruction();
}

void SimonEngine::vc9_skip_if_unk3_is() {
	uint a = vcReadNextWord();
	uint b = vcReadNextWord();
	if (!vc_maybe_skip_proc_1(a, b))
		vcSkipNextInstruction();
}

byte *vc10_depack_column(VC10_state * vs) {
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
		vc10_depack_column(vs);
		vs->x_skip--;
	}
}

byte *SimonEngine::vc10_uncompressFlip(const byte *src, uint w, uint h) {
	w *= 8;

	byte *src_org, *dst_org;
	byte color;
	int8 cur = -0x80;
	uint i, w_cur = w;

	dst_org = _videoBuf1 + w;

	do {
		byte *dst = dst_org;
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
		dst_org++;
	} while (--w_cur);


	src_org = dst_org = _videoBuf1 + w;

	do {
		byte *dst = dst_org;
		for (i = 0; i != w; ++i) {
			byte b = src_org[i];
			b = (b >> 4) | (b << 4);
			*--dst = b;
		}

		src_org += w;
		dst_org += w;
	} while (--h);

	return _videoBuf1;
}

byte *SimonEngine::vc10_flip(const byte *src, uint w, uint h) {
	if (src == _vc10BasePtrOld)
		return _videoBuf1;

	_vc10BasePtrOld = src;

	byte *dst_org, *src_org;
	uint i;

	w *= 8;
	src_org = dst_org = _videoBuf1 + w;

	do {
		byte *dst = dst_org;
		for (i = 0; i != w; ++i) {
			byte b = src_org[i];
			b = (b >> 4) | (b << 4);
			*--dst = b;
		}

		src_org += w;
		dst_org += w;
	} while (--h);

	return _videoBuf1;
}

/* must not be const */
static uint16 _video_windows[128] = {
	0, 0, 20, 200,
	0, 0, 3, 136,
	17, 0, 3, 136,
	0, 0, 20, 200,
	0, 0, 20, 134
};

/* simon2 specific */
void SimonEngine::decodeColumn(byte *dst, const byte *src, int height) {
	const uint pitch = _dxSurfacePitch;
	int8 reps = (int8)0x80;
	byte color;
	byte *dst_org = dst;
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
					dst = ++dst_org;
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
					dst = ++dst_org;
					h = height;
				}
			} while (++reps != 0);
		}
	}
}

void SimonEngine::decodeRow(byte *dst, const byte *src, int width) {
	const uint pitch = _dxSurfacePitch;
	int8 reps = (int8)0x80;
	byte color;
	byte *dst_org = dst;
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
					dst_org += pitch;
					dst = dst_org;
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
					dst_org += pitch;
					dst = dst_org;
					w = width;
				}
			} while (++reps != 0);
		}
	}
}

void SimonEngine::vc10_draw() {
	byte *p2;
	uint width, height;
	byte flags;
	VC10_state state;

	state.image = (int16)vcReadNextWord();
	if (state.image == 0)
		return;

	if (getGameType() == GType_FF) {
		state.palette = (_vcPtr[0] * 16);
	} else {
		state.palette = (_vcPtr[1] * 16);
	}
	_vcPtr += 2;
	state.x = (int16)vcReadNextWord();
	state.x -= _scrollX;

	state.y = (int16)vcReadNextWord();
	state.y -= _scrollY;

	if (getGameType() == GType_SIMON1) {
		state.flags = vcReadNextWord();
	} else {
		state.flags = vcReadNextByte();
	}

	if (state.image < 0)
		state.image = vcReadVar(-state.image);

	p2 = _curVgaFile2 + state.image * 8;
	state.depack_src = _curVgaFile2 + readUint32Wrapper(p2);
	if (getGameType() == GType_FF) {
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
	if (getGameType() == GType_FF) {
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

	uint maxWidth = (getGameType() == GType_FF) ? 640 : 20;
	if ((getGameType() == GType_SIMON2 || getGameType() == GType_FF) && width > maxWidth) {
		horizontalScroll(&state);
		return;
	}
	if (getGameType() == GType_FF && height > 480) {
		verticalScroll(&state);
		return;
	}

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
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

	if (getGameType() == GType_FF) {
		drawImages_Feeble(&state);
	} else {
		drawImages(&state);
	}
}

bool SimonEngine::drawImages_clip(VC10_state *state) {
	const uint16 *vlut;
	uint maxWidth, maxHeight;
	int cur;

	vlut = &_video_windows[_windowNum * 4];

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
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

	maxWidth = (getGameType() == GType_FF) ? 640 : (vlut[2] * 2);
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

	maxHeight = (getGameType() == GType_FF) ? 480 : vlut[3];
	cur += state->draw_height - maxHeight;
	if (cur > 0) {
		do {
			if (!--state->draw_height)
				return 0;
		} while (--cur);
	}

	assert(state->draw_width != 0 && state->draw_height != 0);

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		state->draw_width *= 4;
	}

	return 1;
}

void SimonEngine::drawImages_Feeble(VC10_state *state) {
	if (state->flags & kDFCompressed) {
		if (state->flags & kDFScaled) {
			state->surf_addr = getScaleBuf();
			state->surf_pitch = _dxSurfacePitch;

			uint w, h;
			byte *src, *dst, *dst_org;

			state->dl = state->width;
			state->dh = state->height;

			dst_org = state->surf_addr;
			w = 0;
			do {
				src = vc10_depack_column(state);
				dst = dst_org;

				h = 0;
				do {
					*dst = *src;
					dst += _screenWidth;
					src++;
				} while (++h != state->draw_height);
				dst_org++;
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
			byte *src, *dst, *dst_org;

			state->dl = state->width;
			state->dh = state->height;

			dst_org = state->surf_addr;
			w = 0;
			do {
				byte color;

				src = vc10_depack_column(state);
				dst = dst_org;

				h = 0;
				do {
					color = *src;
					if (color != 0)
						*dst = color;
					dst += _screenWidth;
					src++;
				} while (++h != state->draw_height);
				dst_org++;
			} while (++w != state->draw_width);

			if (_vgaCurSpritePriority % 10 == 9) {
				scaleClip(_scaleHeight, _scaleWidth, _scaleY, _scaleX, _scaleY + _scrollY);
			}
		} else {
			if (drawImages_clip(state) == 0)
				return;

			state->surf_addr += state->x + state->y * state->surf_pitch;

			uint w, h;
			byte *src, *dst, *dst_org;

			state->dl = state->width;
			state->dh = state->height;

			vc10_skip_cols(state);


			if (state->flags & kDFMasked) {
				if (getBitFlag(81) == false) {
					if (state->x  > _feebleRect.right)
						return;
					if (state->y > _feebleRect.bottom)
						return;
					if (state->x + state->width < _feebleRect.left)
						return;
					if (state->y + state->height < _feebleRect.top)
						return;
				}

				dst_org = state->surf_addr;
				w = 0;
				do {
					byte color;

					src = vc10_depack_column(state);
					dst = dst_org;

					h = 0;
					do {
						color = *src;
						if (color)
							*dst = color;
						dst += _screenWidth;
						src++;
					} while (++h != state->draw_height);
					dst_org++;
				} while (++w != state->draw_width);
			} else {
				dst_org = state->surf_addr;
				w = 0;
				do {
					byte color;

					src = vc10_depack_column(state);
					dst = dst_org;

					h = 0;
					do {
						color = *src;
						if ((state->flags & kDFNonTrans) || color != 0)
							*dst = color;
						dst += _screenWidth;
						src++;
					} while (++h != state->draw_height);
					dst_org++;
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

void SimonEngine::drawImages(VC10_state *state) {
	const uint16 *vlut = &_video_windows[_windowNum * 4];

	if (drawImages_clip(state) == 0)
		return;

	uint offs, offs2;
	// Allow one section of Simon the Sorcerer 1 introduction to be displayed
	// in lower half of screen
	if ((getGameType() == GType_SIMON1) && _subroutine == 2926) {
		offs = ((vlut[0]) * 2 + state->x) * 8;
		offs2 = (vlut[1] + state->y);
	} else {
		offs = ((vlut[0] - _video_windows[16]) * 2 + state->x) * 8;
		offs2 = (vlut[1] - _video_windows[17] + state->y);
	}

	state->surf2_addr += offs + offs2 * state->surf2_pitch;
	state->surf_addr += offs + offs2 * state->surf_pitch;

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
			mask = vc10_depack_column(state);	/* esi */
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

		/* vc10_helper_5 */
	} else if (((_lockWord & 0x20) && state->palette == 0) || state->palette == 0xC0) {
		const byte *src;
		byte *dst;
		uint h, i;

		if (!(state->flags & kDFCompressed)) {
			src = state->depack_src + (state->width * state->y_skip * 16) + (state->x_skip * 8);
			dst = state->surf_addr;

			state->draw_width *= 2;

			if (state->flags & kDFNonTrans) {
				/* no transparency */
				h = state->draw_height;
				do {
					memcpy(dst, src, state->draw_width);
					dst += _screenWidth;
					src += state->width * 16;
				} while (--h);
			} else {
				/* transparency */
				h = state->draw_height;
				do {
					for (i = 0; i != state->draw_width; i++)
						if (src[i])
							dst[i] = src[i];
					dst += _screenWidth;
					src += state->width * 16;
				} while (--h);
			}

		} else {
			byte *dst_org = state->surf_addr;
			src = state->depack_src;
			/* AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD EEEEEEEE
			 * aaaaabbb bbcccccd ddddeeee efffffgg ggghhhhh
			 */

			if (state->flags & kDFNonTrans) {
				/* no transparency */
				do {
					uint count = state->draw_width / 4;

					dst = dst_org;
					do {
						uint32 bits = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | (src[3]);

						dst[0] = (byte)((bits >> (32 - 5)) & 31);
						dst[1] = (byte)((bits >> (32 - 10)) & 31);
						dst[2] = (byte)((bits >> (32 - 15)) & 31);
						dst[3] = (byte)((bits >> (32 - 20)) & 31);
						dst[4] = (byte)((bits >> (32 - 25)) & 31);
						dst[5] = (byte)((bits >> (32 - 30)) & 31);

						bits = (bits << 8) | src[4];

						dst[6] = (byte)((bits >> (40 - 35)) & 31);
						dst[7] = (byte)((bits) & 31);

						dst += 8;
						src += 5;
					} while (--count);
					dst_org += _screenWidth;
				} while (--state->draw_height);
			} else {
				/* transparency */
				do {
					uint count = state->draw_width / 4;

					dst = dst_org;
					do {
						uint32 bits = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | (src[3]);
						byte tmp;

						tmp = (byte)((bits >> (32 - 5)) & 31);
						if (tmp)
							dst[0] = tmp;
						tmp = (byte)((bits >> (32 - 10)) & 31);
						if (tmp)
							dst[1] = tmp;
						tmp = (byte)((bits >> (32 - 15)) & 31);
						if (tmp)
							dst[2] = tmp;
						tmp = (byte)((bits >> (32 - 20)) & 31);
						if (tmp)
							dst[3] = tmp;
						tmp = (byte)((bits >> (32 - 25)) & 31);
						if (tmp)
							dst[4] = tmp;
						tmp = (byte)((bits >> (32 - 30)) & 31);
						if (tmp)
							dst[5] = tmp;

						bits = (bits << 8) | src[4];

						tmp = (byte)((bits >> (40 - 35)) & 31);
						if (tmp)
							dst[6] = tmp;
						tmp = (byte)((bits) & 31);
						if (tmp)
							dst[7] = tmp;

						dst += 8;
						src += 5;
					} while (--count);
					dst_org += _screenWidth;
				} while (--state->draw_height);
			}
		}
		/* vc10_helper_4 */
	} else {
		if (getGameType() == GType_SIMON2 && state->flags & kDFUseFrontBuf && getBitFlag(171)) {
			state->surf_addr = state->surf2_addr;
			state->surf_pitch = state->surf2_pitch;
		}

		if (state->flags & kDFCompressed) {
			uint w, h;
			byte *src, *dst, *dst_org;

			state->x_skip *= 4;				/* reached */

			state->dl = state->width;
			state->dh = state->height;

			vc10_skip_cols(state);

			if (state->flags & kDFNonTrans) {
				dst_org = state->surf_addr;
				w = 0;
				do {
					src = vc10_depack_column(state);
					dst = dst_org;

					h = 0;
					do {
						dst[0] = (*src / 16) | state->palette;
						dst[1] = (*src & 15) | state->palette;
						dst += _screenWidth;
						src++;
					} while (++h != state->draw_height);
					dst_org += 2;
				} while (++w != state->draw_width);
			} else {
				dst_org = state->surf_addr;
				if (state->flags & 0x40) {		/* reached */
					dst_org += vcReadVar(252);
				}
				w = 0;
				do {
					byte color;

					src = vc10_depack_column(state);
					dst = dst_org;

					h = 0;
					do {
						color = (*src / 16);
						if (color)
							dst[0] = color | state->palette;
						color = (*src & 15);
						if (color)
							dst[1] = color | state->palette;
						dst += _screenWidth;
						src++;
					} while (++h != state->draw_height);
					dst_org += 2;
				} while (++w != state->draw_width);
			}
			/* vc10_helper_6 */
		} else {
			const byte *src;
			byte *dst;
			uint count;

			src = state->depack_src + (state->width * state->y_skip) * 8;
			dst = state->surf_addr;
			state->x_skip *= 4;
			if (state->flags & kDFNonTrans) {
				do {
					for (count = 0; count != state->draw_width; count++) {
						dst[count * 2] = (src[count + state->x_skip] / 16) | state->palette;
						dst[count * 2 + 1] = (src[count + state->x_skip] & 15) | state->palette;
					}
					dst += _screenWidth;
					src += state->width * 8;
				} while (--state->draw_height);
			} else {
				do {
					for (count = 0; count != state->draw_width; count++) {
						byte color;
						color = (src[count + state->x_skip] / 16);
						if (color)
							dst[count * 2] = color | state->palette;
						color = (src[count + state->x_skip] & 15);
						if (color)
							dst[count * 2 + 1] = color | state->palette;
					}
					dst += _screenWidth;
					src += state->width * 8;
				} while (--state->draw_height);

			}

			/* vc10_helper_7 */
		}
	}
}

void SimonEngine::horizontalScroll(VC10_state *state) {
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
	if (_variableArray[34] < 0)
		state->x = _variableArray[251];

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

void SimonEngine::verticalScroll(VC10_state *state) {
	debug(0, "Vertical scrolling not supported");

	const byte *src;
	byte *dst;
	int h;

	_scrollXMax = 0;
	_scrollYMax = state->height - 480;
	_scrollImage = state->depack_src;
	_scrollWidth = state->width;
	if (_variableArray[34] < 0)
		state->y = _variableArray[250];

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

void SimonEngine::scaleClip(int16 h, int16 w, int16 y, int16 x, int16 scrollY) {
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

void SimonEngine::vc11_clearPathFinder() {
	memset(&_pathFindArray, 0, sizeof(_pathFindArray));
}

void SimonEngine::vc12_delay() {
	VgaSprite *vsp = findCurSprite();
	uint num;

	if (getGameType() == GType_SIMON1) {
		num = vcReadVarOrWord();
	} else {
		num = vcReadNextByte() * _frameRate;
	}

	// Work around to allow inventory arrows to be
	// shown in some versions of Simon the Sorcerer 1
	if ((getGameType() == GType_SIMON1) && vsp->id == 128)
		num = 0;
	else
		num += VGA_DELAY_BASE;

	add_vga_timer(num, _vcPtr, _vgaCurSpriteId, _vgaCurZoneNum);
	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void SimonEngine::vc13_addToSpriteX() {
	VgaSprite *vsp = findCurSprite();
	vsp->x += (int16)vcReadNextWord();
	_vgaSpriteChanged++;
}

void SimonEngine::vc14_addToSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y += (int16)vcReadNextWord();
	_vgaSpriteChanged++;
}

void SimonEngine::vc15_wakeup_id() {
	VgaSleepStruct *vfs = _vgaSleepStructs, *vfs_tmp;
	uint16 id = vcReadNextWord();
	while (vfs->ident != 0) {
		if (vfs->ident == id) {
			add_vga_timer(VGA_DELAY_BASE, vfs->code_ptr, vfs->sprite_id, vfs->cur_vga_file);
			vfs_tmp = vfs;
			do {
				memcpy(vfs_tmp, vfs_tmp + 1, sizeof(VgaSleepStruct));
				vfs_tmp++;
			} while (vfs_tmp->ident != 0);
		} else {
			vfs++;
		}
	}

	/* clear a wait event */
	if (id == _vgaWaitFor)
		_vgaWaitFor = 0;
}

void SimonEngine::vc16_sleep_on_id() {
	VgaSleepStruct *vfs = _vgaSleepStructs;
	while (vfs->ident)
		vfs++;

	vfs->ident = vcReadNextWord();
	vfs->code_ptr = _vcPtr;
	vfs->sprite_id = _vgaCurSpriteId;
	vfs->cur_vga_file = _vgaCurZoneNum;

	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void SimonEngine::vc17_setPathfinderItem() {
	uint a = vcReadNextWord();
	_pathFindArray[a - 1] = (const uint16 *)_vcPtr;

	int end = (getGameType() == GType_FF) ? 9999 : 999;
	while (readUint16Wrapper(_vcPtr) != end)
		_vcPtr += 4;
	_vcPtr += 2;
}

void SimonEngine::vc18_jump() {
	int16 offs = vcReadNextWord();
	_vcPtr += offs;
}

/* chain to script? */
void SimonEngine::vc19_chain_to_script() {
	/* unused */
	error("vc19_chain_to_script: not implemented");
}

/* helper routines */

void SimonEngine::vc20_setRepeat() {
	/* FIXME: This opcode is somewhat strange: it first reads a BE word from
	 * the script (advancing the script pointer in doing so); then it writes
	 * back the same word, this time as LE, into the script.
	 */
	uint16 a = vcReadNextWord();
	WRITE_LE_UINT16(const_cast<byte *>(_vcPtr), a);
	_vcPtr += 2;
}

void SimonEngine::vc21_endRepeat() {
	int16 a = vcReadNextWord();
	const byte *tmp = _vcPtr + a;
	if (getGameType() == GType_SIMON1)
		tmp += 4;
	else
		tmp += 3;

	uint16 val = READ_LE_UINT16(tmp);
	if (val != 0) {
		// Decrement counter
		WRITE_LE_UINT16(const_cast<byte *>(tmp), val - 1);
		_vcPtr = tmp + 2;
	}
}

void SimonEngine::vc22_setSpritePalette() {
	uint a = vcReadNextWord();
	uint b = vcReadNextWord();
	uint num = a == 0 ? 32 : 16;
	uint palSize = 96;
	byte *palptr, *src;

	if (getGameType() == GType_FF) {
		a = 0;
		num = 256;
		palSize = 768;
	}

	palptr = &_palette[(a * 64)];
	src = _curVgaFile1 + 6 + b * palSize;

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

void SimonEngine::vc23_setSpritePriority() {
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

void SimonEngine::vc24_setSpriteXY() {
	VgaSprite *vsp = findCurSprite();
	vsp->image = vcReadVarOrWord();

	vsp->x += (int16)vcReadNextWord();
	vsp->y += (int16)vcReadNextWord();
	if (getGameType() == GType_SIMON1) {
		vsp->flags = vcReadNextWord();
	} else {
		vsp->flags = vcReadNextByte();
	}

	_vgaSpriteChanged++;
}

void SimonEngine::vc25_halt_sprite() {
	VgaSprite *vsp = findCurSprite();
	while (vsp->id != 0) {
		memcpy(vsp, vsp + 1, sizeof(VgaSprite));
		vsp++;
	}
	_vcPtr = (byte *)&_vc_get_out_of_code;
	_vgaSpriteChanged++;
}

void SimonEngine::vc26_setSubWindow() {
	uint16 *as = &_video_windows[vcReadNextWord() * 4]; // number
	as[0] = vcReadNextWord(); // x
	as[1] = vcReadNextWord(); // y
	as[2] = vcReadNextWord(); // width
	as[3] = vcReadNextWord(); // height
}

void SimonEngine::vc27_resetSprite() {
	VgaSprite bak, *vsp;
	VgaSleepStruct *vfs;
	VgaTimerEntry *vte, *vte2;

	_lockWord |= 8;

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
		if ((getGameType() == GType_SIMON1) && vsp->id == 128) {
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

	if (getGameType() == GType_FF)
		vcWriteVar(42, 1);

	_lockWord &= ~8;
}

void SimonEngine::vc28_dummy_op() {
	/* unused */
	_vcPtr += 8;
}

void SimonEngine::vc29_stopAllSounds() {
	_sound->stopAll();
}

void SimonEngine::vc30_setFrameRate() {
	_frameRate = vcReadNextWord();
}

void SimonEngine::vc31_setWindow() {
	_windowNum = vcReadNextWord();
}

uint SimonEngine::vcReadVar(uint var) {
	assert(var < 255);
	return (uint16)_variableArray[var];
}

void SimonEngine::vcWriteVar(uint var, int16 value) {
	_variableArray[var] = value;
}

void SimonEngine::vc32_copyVar() {
	uint16 a = vcReadVar(vcReadNextWord());
	vcWriteVar(vcReadNextWord(), a);
}

void SimonEngine::vc33_setMouseOn() {
	if (_mouseHideCount != 0) {
		_mouseHideCount = 1;
		mouseOn();
	}
}

void SimonEngine::vc34_setMouseOff() {
	mouseOff();
	_mouseHideCount = 200;
	_leftButtonDown = 0;
}

void SimonEngine::vc35_clearWindow() {
	/* unused */
	_vcPtr += 4;
	_vgaSpriteChanged++;
}

void SimonEngine::vc36_setWindowImage() {
	_updateScreen = false;
	uint vga_res = vcReadNextWord();
	uint windowNum = vcReadNextWord();

	if (getGameType() == GType_FF) {
		// TODO
	} else if (getGameType() == GType_SIMON2) {
		set_video_mode_internal(windowNum, vga_res);
	} else {
		if (windowNum == 16) {
			_copyPartialMode = 2;
		} else {
			set_video_mode_internal(windowNum, vga_res);
		}
	}
}

void SimonEngine::vc37_addToSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y += vcReadVar(vcReadNextWord());
	_vgaSpriteChanged++;
}

void SimonEngine::vc38_skipIfVarZero() {
	uint var = vcReadNextWord();
	if (vcReadVar(var) == 0)
		vcSkipNextInstruction();
}

void SimonEngine::vc39_setVar() {
	uint var = vcReadNextWord();
	int16 value = vcReadNextWord();
	vcWriteVar(var, value);
}

void SimonEngine::vc40() {
	uint var = vcReadNextWord();
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
			add_vga_timer(6, NULL, 0, 0);	/* special timer */
		}
	}
no_scroll:;

	vcWriteVar(var, value);
}

void SimonEngine::vc41() {
	uint var = vcReadNextWord();
	int16 value = vcReadVar(var) - vcReadNextWord();

	if ((getGameType() == GType_SIMON2) && var == 15 && !getBitFlag(80)) {
		int16 tmp;

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
			tmp = _scrollXMax - _scrollX;
			if (_scrollX < 20)
				_scrollCount = -_scrollX;
			add_vga_timer(6, NULL, 0, 0);	/* special timer */
		}
	}
no_scroll:;

	vcWriteVar(var, value);
}

void SimonEngine::vc42_delayIfNotEQ() {
	uint val = vcReadVar(vcReadNextWord());
	if (val != vcReadNextWord()) {

		add_vga_timer(_frameRate + 1, _vcPtr - 4, _vgaCurSpriteId, _vgaCurZoneNum);
		_vcPtr = (byte *)&_vc_get_out_of_code;
	}
}

void SimonEngine::vc43_skipIfBitClear() {
	if (!getBitFlag(vcReadNextWord())) {
		vcSkipNextInstruction();
	}
}

void SimonEngine::vc44_skipIfBitSet() {
	if (getBitFlag(vcReadNextWord())) {
		vcSkipNextInstruction();
	}
}

void SimonEngine::vc45_setSpriteX() {
	VgaSprite *vsp = findCurSprite();
	vsp->x = vcReadVar(vcReadNextWord());
	_vgaSpriteChanged++;
}

void SimonEngine::vc46_setSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y = vcReadVar(vcReadNextWord());
	_vgaSpriteChanged++;
}

void SimonEngine::vc47_addToVar() {
	uint var = vcReadNextWord();
	vcWriteVar(var, vcReadVar(var) + vcReadVar(vcReadNextWord()));
}

void SimonEngine::vc48_setPathFinder() {
	uint a = (uint16)_variableArray[12];
	const uint16 *p = _pathFindArray[a - 1];

	if (getGameType() == GType_FF) {
		VgaSprite *vsp = findCurSprite();
		int16 x, x2, y, y1, y2, ydiff;
		uint pos = 0;

		while (vsp->x >= readUint16Wrapper(p + 2)) {
			p += 2;
			pos++;
		}

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

		_variableArray[11] = readUint16Wrapper(p);
		_variableArray[13] = pos;
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

void SimonEngine::setBitFlag(uint bit, bool value) {
	uint16 *bits = &_bitArray[bit / 16];
	*bits = (*bits & ~(1 << (bit & 15))) | (value << (bit & 15));
}

bool SimonEngine::getBitFlag(uint bit) {
	uint16 *bits = &_bitArray[bit / 16];
	return (*bits & (1 << (bit & 15))) != 0;
}

void SimonEngine::vc49_setBit() {
	setBitFlag(vcReadNextWord(), true);
}

void SimonEngine::vc50_clearBit() {
	setBitFlag(vcReadNextWord(), false);
}

void SimonEngine::vc51_clear_hitarea_bit_0x40() {
	clear_hitarea_bit_0x40(vcReadNextWord());
}

void SimonEngine::vc52_playSound() {
	bool ambient = false;

	uint16 sound = vcReadNextWord();
	if (sound >= 0x8000) {
		ambient = true;
		sound = -sound;
	}

	if (getGameType() == GType_FF) {
		uint16 pan = vcReadNextWord();
		uint16 vol = vcReadNextWord();
		loadSound(sound, pan, vol, ambient);
	} else if (getGameType() == GType_SIMON2) {
		if (ambient) {
			_sound->playAmbient(sound);
		} else {
			_sound->playEffects(sound);
		}
	} else if (getFeatures() & GF_TALKIE) {
		_sound->playEffects(sound);
	} else if (getGameId() == GID_SIMON1DOS) {
		playSting(sound);
	}
}

void SimonEngine::vc53_no_op() {
	// Start sound effect, panning it with the animation
	int snd = vcReadNextWord();
	int xoffs = vcReadNextWord();
	int vol = vcReadNextWord();
	debug(0, "STUB: vc53_no_op: snd %d xoffs %d vol %d", snd, xoffs, vol);
}

void SimonEngine::vc54_no_op() {
	/* unused */
	_vcPtr += 6;
}

void SimonEngine::vc55_offset_hit_area() {
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

void SimonEngine::vc56_delay() {
	uint num = vcReadVarOrWord() * _frameRate;

	add_vga_timer(num + VGA_DELAY_BASE, _vcPtr, _vgaCurSpriteId, _vgaCurZoneNum);
	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void SimonEngine::vc59() {
	if (getGameType() == GType_SIMON1) {
		if (!_sound->isVoiceActive())
			vcSkipNextInstruction();
	} else {
		uint file = vcReadNextWord();
		uint start = vcReadNextWord();
		uint end = vcReadNextWord() + 1;

		do {
			vc_kill_sprite(file, start);
		} while (++start != end);
	}
}

void SimonEngine::vc58() {
	uint sprite = _vgaCurSpriteId;
	uint file = _vgaCurZoneNum;
	const byte *vc_ptr_org;
	uint16 tmp;

	_vgaCurZoneNum = vcReadNextWord();
	_vgaCurSpriteId = vcReadNextWord();

	tmp = to16Wrapper(vcReadNextWord());

	vc_ptr_org = _vcPtr;
	_vcPtr = (byte *)&tmp;
	vc23_setSpritePriority();

	_vcPtr = vc_ptr_org;
	_vgaCurSpriteId = sprite;
	_vgaCurZoneNum = file;
}

void SimonEngine::vc57_no_op() {
	/* unused */
}

void SimonEngine::vc_kill_sprite(uint file, uint sprite) {
	uint16 old_sprite_id, old_cur_file_id;
	VgaSleepStruct *vfs;
	VgaSprite *vsp;
	VgaTimerEntry *vte;
	const byte *vc_ptr_org;

	old_sprite_id = _vgaCurSpriteId;
	old_cur_file_id = _vgaCurZoneNum;
	vc_ptr_org = _vcPtr;

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
				delete_vga_timer(vte);
				break;
			}
			vte++;
		}
	}

	_vgaCurZoneNum = old_cur_file_id;
	_vgaCurSpriteId = old_sprite_id;
	_vcPtr = vc_ptr_org;
}

void SimonEngine::vc60_killSprite() {
	uint file;

	if (getGameType() == GType_SIMON1) {
		file = _vgaCurZoneNum;
	} else {
		file = vcReadNextWord();
	}
	uint sprite = vcReadNextWord();
	vc_kill_sprite(file, sprite);
}

void SimonEngine::vc61_setMaskImage() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadVarOrWord();

	vsp->x += vcReadNextWord();
	vsp->y += vcReadNextWord();
	vsp->flags = kDFMasked | kDFUseFrontBuf;

	_vgaSpriteChanged++;
}

void SimonEngine::vc62_fastFadeOut() {
	uint i;

	vc29_stopAllSounds();

	if (!_fastFadeOutFlag) {
		_fastFadeOutFlag = true;

		_videoNumPalColors = 256;
		if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
			if (_windowNum == 4)
				_videoNumPalColors = 208;
		}

		memcpy(_videoBuf1, _paletteBackup, _videoNumPalColors * 4);
		for (i = NUM_PALETTE_FADEOUT; i != 0; --i) {
			palette_fadeout((uint32 *)_videoBuf1, _videoNumPalColors);
			_system->setPalette(_videoBuf1, 0, _videoNumPalColors);
			if (_fade)
				_system->updateScreen();
			delay(5);
		}

		if (getGameType() == GType_SIMON1) {
			uint16 params[5];						/* parameters to vc10_draw */
			VgaSprite *vsp;
			VgaPointersEntry *vpe;
			const byte *vc_ptr_org = _vcPtr;

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
			_vcPtr = vc_ptr_org;
		}

		// Allow one section of Simon the Sorcerer 1 introduction to be displayed
		// in lower half of screen
		if ((getGameType() == GType_SIMON1) && (_subroutine == 2923 || _subroutine == 2926)) {
			dx_clear_surfaces(200);
		} else if (getGameType() == GType_FF) {
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

void SimonEngine::vc63_fastFadeIn() {
	if (getGameType() == GType_FF) {
			_paletteColorCount = 256;
	} else {
		_paletteColorCount = 208;
		if (_windowNum != 4) {
			_paletteColorCount = 256;
		}
	}
	_fastFadeOutFlag = false;
}

void SimonEngine::vc64_skipIfSpeechEnded() {
	if (!_sound->isVoiceActive() || (_subtitles && _language != Common::HB_ISR))
		vcSkipNextInstruction();
}

void SimonEngine::vc65_slowFadeIn() {
	_paletteColorCount = 624;
	_videoNumPalColors = 208;
	if (_windowNum != 4) {
		_paletteColorCount = 768;
		_videoNumPalColors = 256;
	}
	_paletteColorCount |= 0x8000;
	_fastFadeOutFlag = false;
}

void SimonEngine::vc66_skipIfNotEqual() {
	uint a = vcReadNextWord();
	uint b = vcReadNextWord();

	if (vcReadVar(a) != vcReadVar(b))
		vcSkipNextInstruction();
}

void SimonEngine::vc67_skipIfGE() {
	uint a = vcReadNextWord();
	uint b = vcReadNextWord();

	if (vcReadVar(a) >= vcReadVar(b))
		vcSkipNextInstruction();
}

void SimonEngine::vc68_skipIfLE() {
	uint a = vcReadNextWord();
	uint b = vcReadNextWord();

	if (vcReadVar(a) <= vcReadVar(b))
		vcSkipNextInstruction();
}

void SimonEngine::vc69_playTrack() {
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

void SimonEngine::vc70_queueMusic() {
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

void SimonEngine::vc71_checkMusicQueue() {
	// Jamieson630:
	// This command skips the next instruction
	// unless (1) there is a track playing, AND
	// (2) there is a track queued to play after it.
	if (!midi.isPlaying (true))
		vcSkipNextInstruction();
}

void SimonEngine::vc72_play_track_2() {
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

void SimonEngine::vc73_setMark() {
	_marks |= (1 << vcReadNextWord());
}

void SimonEngine::vc74_clearMark() {
	_marks &= ~(1 << vcReadNextWord());
}

int SimonEngine::getScale(int y, int x) {
	int z;

	if (y > _baseY) {
		return((int)(x * (1 + ((y - _baseY) * _scale))));
	} else {	
		if (x == 0)
			return(0);
		if (x < 0) {
			z = ((int)((x * (1 - ((_baseY - y)* _scale))) - 0.5));
			if (z >- 2)
				return(-2);
			return(z);
		}

		z=((int)((x * (1 - ((_baseY-y) * _scale))) + 0.5));
		if (z < 2)
			return(2);

		return(z);
	}
}

void SimonEngine::vc75_setScale() {
	_baseY = vcReadNextWord();
	_scale = (float)vcReadNextWord() / 1000000.;
}

void SimonEngine::vc76_setScaleXOffs() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadNextWord();
	int16 x = vcReadNextWord();
	int var = vcReadNextWord();

	vsp->x += getScale(vsp->y, x);
	_variableArray[var] = vsp->x;

	checkScrollX(x, vsp->x);

	vsp->flags = kDFScaled;
}

void SimonEngine::vc77_setScaleYOffs() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadNextWord();
	int16 y = vcReadNextWord();
	int var = vcReadNextWord();

	vsp->y += getScale(vsp->y, y);
	_variableArray[var] = vsp->y;

	if (y != 0) 
		checkScrollY(y, vsp->y);

	vsp->flags = kDFScaled;
}

void SimonEngine::vc78_computeXY() {
	VgaSprite *vsp = findCurSprite();

	uint a = (uint16)_variableArray[12];
	uint b = (uint16)_variableArray[13];

	const uint16 *p = _pathFindArray[a - 1];
	p += b * 2;

	uint16 posx = readUint16Wrapper(p);
	_variableArray[15] = posx;
	vsp->x = posx;

	uint16 posy = readUint16Wrapper(p + 1);
	_variableArray[16] = posy;
	vsp->y = posy;

	setBitFlag(85, false);
	if (getBitFlag(74)) {
		centreScroll();
	}
}

void SimonEngine::vc79_computePosNum() {
	uint a = (uint16)_variableArray[12];
	const uint16 *p = _pathFindArray[a - 1];
	uint pos = 0;

	int16 y = _variableArray[16];
	while(y > readUint16Wrapper(p + 1)) {
		p += 2;
		pos++;
	}

	_variableArray[13] = pos;
}

void SimonEngine::vc80_setOverlayImage() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadVarOrWord();

	vsp->x += vcReadNextWord();
	vsp->y += vcReadNextWord();
	vsp->flags = kDFOverlayed;

	_vgaSpriteChanged++;
}

void SimonEngine::vc81_setRandom() {
	uint var = vcReadNextWord();
	uint value = vcReadNextWord();
	writeVariable(var, _rnd.getRandomNumber(value - 1));
}

void SimonEngine::vc82_getPathValue() {
	uint8 val;

	uint16 var = vcReadNextWord();

	if (getBitFlag(82) == true) {
		val = _pathValues1[_GPVCount1++];
	} else {
		val = _pathValues[_GPVCount++];
	}

	writeVariable(var, val);
}

void SimonEngine::vc83_playSoundLoop() {
	// Start looping sound effect
	int snd = vcReadNextWord();
	int vol = vcReadNextWord();
	int pan = vcReadNextWord();
	debug(0, "STUB: vc83_playSoundLoop: snd %d vol %d pan %d", snd, vol, pan);
}

void SimonEngine::vc84_stopSoundLoop() {
	// Stop looping sound effect
	debug(0, "STUB: vc84_stopSoundLoop");
}

// Scrolling functions for Feeble Files
void SimonEngine::checkScrollX(int x, int xpos) {
	if (_scrollXMax == 0 || getBitFlag(80) || getBitFlag(82) || x == 0)
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

		if ((uint16)(xpos - _scrollX) < 161) {
			_scrollCount = -320;
			tmp = _scrollXMax - _scrollX;
			if (_scrollX < 320)
				_scrollCount = -_scrollX;
		}
	}
}

void SimonEngine::checkScrollY(int y, int ypos) {
	if (_scrollYMax == 0 || getBitFlag(80))
		return;

	int16 tmp;
	if (y > 0) {
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

		if ((uint16)(ypos - _scrollY) < 100) {
			_scrollCount = -240;
			tmp = _scrollYMax - _scrollY;
			if (_scrollY < 240)
				_scrollCount = -_scrollY;
		}
	}
}

void SimonEngine::centreScroll() {
	int16 x, y, tmp;

	if (_scrollXMax != 0) {
		_scrollCount = 0;
		x = _variableArray[15] - _scrollX;
		if (getBitFlag(85) || x >= 624) {
			x -= 320;
			tmp = _scrollXMax - _scrollX;
			if (tmp < x)
				x = tmp;
			_scrollCount = x;
		} else if (x < 17) {
			x -= 320;
			if (_scrollX < -x)
				x = -_scrollX;
			_scrollCount = x;
		}
	} else if (_scrollYMax != 0) {
		_scrollCount = 0;
		y = _variableArray[16] - _scrollY;
		if (y >= 460) {
			y -= 240;
			tmp = _scrollYMax - _scrollY;
			if (tmp < y)
				y = tmp;
			_scrollCount = y;
		} else if (y < 30) {
			y -= 240;
			if (_scrollY < -y)
				y = -_scrollY;
			_scrollCount = y;
		}
	}
}

} // End of namespace Simon
