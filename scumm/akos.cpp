/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/akos.h"
#include "scumm/bomp.h"
#include "scumm/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/sound.h"

namespace Scumm {

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct AkosHeader {
	byte unk_1[2];
	byte flags;
	byte unk_2;
	uint16 num_anims;
	uint16 unk_3;
	uint16 codec;
} GCC_PACK;

struct AkosOffset {
	uint32 akcd;
	uint16 akci;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif


enum AkosOpcodes {
	AKC_Return = 0xC001,
	AKC_SetVar = 0xC010,
	AKC_CmdQue3 = 0xC015,
	AKC_ComplexChan = 0xC020,
	AKC_Jump = 0xC030,
	AKC_JumpIfSet = 0xC031,
	AKC_AddVar = 0xC040,
	AKC_Ignore = 0xC050,
	AKC_IncVar = 0xC060,
	AKC_CmdQue3Quick = 0xC061,
	AKC_JumpStart = 0xC070,
	AKC_JumpE = 0xC070,
	AKC_JumpNE = 0xC071,
	AKC_JumpL = 0xC072,
	AKC_JumpLE = 0xC073,
	AKC_JumpG = 0xC074,
	AKC_JumpGE = 0xC075,
	AKC_StartAnim = 0xC080,
	AKC_StartVarAnim = 0xC081,
	AKC_Random = 0xC082,
	AKC_SetActorClip = 0xC083,
	AKC_StartAnimInActor = 0xC084,
	AKC_SetVarInActor = 0xC085,
	AKC_HideActor = 0xC086,
	AKC_SetDrawOffs = 0xC087,
	AKC_JumpTable = 0xC088,
	AKC_SoundStuff = 0xC089,
	AKC_Flip = 0xC08A,
	AKC_Cmd3 = 0xC08B,
	AKC_Ignore3 = 0xC08C,
	AKC_Ignore2 = 0xC08D,
	AKC_SkipStart = 0xC090,
	AKC_SkipE = 0xC090,
	AKC_SkipNE = 0xC091,
	AKC_SkipL = 0xC092,
	AKC_SkipLE = 0xC093,
	AKC_SkipG = 0xC094,
	AKC_SkipGE = 0xC095,
	AKC_ClearFlag = 0xC09F,
	AKC_EndSeq = 0xC0FF
};

static bool akos_compare(int a, int b, byte cmd) {
	switch (cmd) {
	case 0:
		return a == b;
	case 1:
		return a != b;
	case 2:
		return a < b;
	case 3:
		return a <= b;
	case 4:
		return a > b;
	default:
		return a >= b;
	}
}

bool ScummEngine::akos_hasManyDirections(int costume) {
	byte *akos;
	const AkosHeader *akhd;

	akos = getResourceAddress(rtCostume, costume);
	assert(akos);

	akhd = (const AkosHeader *)findResourceData(MKID('AKHD'), akos);
	return (akhd->flags & 2) != 0;
}

int ScummEngine::akos_frameToAnim(Actor *a, int frame) {
	if (_version >= 7 && akos_hasManyDirections(a->costume))
		return toSimpleDir(1, a->getFacing()) + frame * 8;
	else
		return newDirToOldDir(a->getFacing()) + frame * 4;
}

void ScummEngine::akos_decodeData(Actor *a, int frame, uint usemask) {
	uint anim;
	const byte *akos, *r;
	const AkosHeader *akhd;
	uint offs;
	int i;
	byte code;
	uint16 start, len;
	uint16 mask;

	if (a->costume == 0)
		return;

	anim = akos_frameToAnim(a, frame);

	akos = getResourceAddress(rtCostume, a->costume);
	assert(akos);

	akhd = (const AkosHeader *)findResourceData(MKID('AKHD'), akos);

	if (anim >= READ_LE_UINT16(&akhd->num_anims))
		return;

	r = findResourceData(MKID('AKCH'), akos);
	assert(r);

	offs = READ_LE_UINT16(r + anim * sizeof(uint16));
	if (offs == 0)
		return;
	r += offs;

	i = 0;
	mask = READ_LE_UINT16(r);
	r += sizeof(uint16);
	do {
		if (mask & 0x8000) {
			code = *r++;
			if (usemask & 0x8000) {
				switch (code) {
				case 1:
					a->cost.active[i] = 0;
					a->cost.frame[i] = frame;
					a->cost.end[i] = 0;
					a->cost.start[i] = 0;
					a->cost.curpos[i] = 0;
					break;
				case 4:
					a->cost.stopped |= 1 << i;
					break;
				case 5:
					a->cost.stopped &= ~(1 << i);
					break;
				default:
					start = READ_LE_UINT16(r);
					len = READ_LE_UINT16(r + sizeof(uint16));
					r += sizeof(uint16) * 2;

					a->cost.active[i] = code;
					a->cost.frame[i] = frame;
					a->cost.end[i] = start + len;
					a->cost.start[i] = start;
					a->cost.curpos[i] = start;
					break;
				}
			} else {
				if (code != 1 && code != 4 && code != 5)
					r += sizeof(uint16) * 2;
			}
		}
		i++;
		mask <<= 1;
		usemask <<= 1;
	} while ((uint16)mask);
}

void AkosRenderer::setPalette(byte *new_palette) {
	const byte *the_akpl;
	uint size, i;

	the_akpl = _vm->findResourceData(MKID('AKPL'), akos);
	size = _vm->getResourceDataSize(akpl);

	if (size > 256)
		error("akos_setPalette: %d is too many colors", size);

	for (i = 0; i < size; i++) {
		palette[i] = new_palette[i] != 0xFF ? new_palette[i] : the_akpl[i];
	}

	if (size == 256) {
		byte color = new_palette[0];
		if (color == 255) {
			palette[0] = color;
		} else {
			_vm->_bompActorPalettePtr = palette;
		}
	}
}

void AkosRenderer::setCostume(int costume) {
	akos = _vm->getResourceAddress(rtCostume, costume);
	assert(akos);

	akhd = (const AkosHeader *) _vm->findResourceData(MKID('AKHD'), akos);
	akof = (const AkosOffset *) _vm->findResourceData(MKID('AKOF'), akos);
	akci = _vm->findResourceData(MKID('AKCI'), akos);
	aksq = _vm->findResourceData(MKID('AKSQ'), akos);
	akcd = _vm->findResourceData(MKID('AKCD'), akos);
	akpl = _vm->findResourceData(MKID('AKPL'), akos);
	codec = READ_LE_UINT16(&akhd->codec);
}

void AkosRenderer::setFacing(Actor *a) {
	_mirror = (newDirToOldDir(a->getFacing()) != 0 || akhd->flags & 1);
	if (a->flip)
		_mirror = !_mirror;
}

byte AkosRenderer::drawLimb(const CostumeData &cost, int limb) {
	uint code;
	const byte *p;
	const AkosOffset *off;
	const CostumeInfo *costumeInfo;
	uint i, extra;
	byte result = 0;
	int xmoveCur, ymoveCur;

	if (_skipLimb)
		return 0;

	if (!cost.active[limb] || cost.stopped & (1 << limb))
		return 0;

	p = aksq + cost.curpos[limb];

	code = p[0];
	if (code & 0x80)
		code = (code << 8) | p[1];

	if (code == AKC_Return || code == AKC_EndSeq)
		return 0;

	if (code != AKC_ComplexChan) {
		off = akof + (code & 0xFFF);

		assert((code & 0xFFF) * 6 < READ_BE_UINT32((const byte *)akof - 4) - 8);
		assert((code & 0x7000) == 0);

		_srcptr = akcd + READ_LE_UINT32(&off->akcd);
		costumeInfo = (const CostumeInfo *) (akci + READ_LE_UINT16(&off->akci));

		_width = READ_LE_UINT16(&costumeInfo->width);
		_height = READ_LE_UINT16(&costumeInfo->height);
		xmoveCur = _xmove + (int16)READ_LE_UINT16(&costumeInfo->rel_x);
		ymoveCur = _ymove + (int16)READ_LE_UINT16(&costumeInfo->rel_y);
		_xmove += (int16)READ_LE_UINT16(&costumeInfo->move_x);
		_ymove -= (int16)READ_LE_UINT16(&costumeInfo->move_y);

		switch (codec) {
		case 1:
			result |= codec1(xmoveCur, ymoveCur);
			break;
		case 5:
			result |= codec5(xmoveCur, ymoveCur);
			break;
		case 16:
			result |= codec16(xmoveCur, ymoveCur);
			break;
		default:
			error("akos_drawCostumeChannel: invalid codec %d", codec);
		}
	} else {
		extra = p[2];
		p += 3;

		for (i = 0; i != extra; i++) {
			code = p[4];
			if (code & 0x80)
				code = ((code & 0xF) << 8) | p[5];
			off = akof + code;

			_srcptr = akcd + READ_LE_UINT32(&off->akcd);
			costumeInfo = (const CostumeInfo *) (akci + READ_LE_UINT16(&off->akci));

			_width = READ_LE_UINT16(&costumeInfo->width);
			_height = READ_LE_UINT16(&costumeInfo->height);

			xmoveCur = _xmove + (int16)READ_LE_UINT16(p + 0);
			ymoveCur = _ymove + (int16)READ_LE_UINT16(p + 2);

			p += (p[4] & 0x80) ? 6 : 5;

			switch (codec) {
			case 1:
				result |= codec1(xmoveCur, ymoveCur);
				break;
			case 5:
				result |= codec5(xmoveCur, ymoveCur);
				break;
			case 16:
				result |= codec16(xmoveCur, ymoveCur);
				break;
			default:
				error("akos_drawCostumeChannel: invalid codec %d", codec);
			}
		}
	}

	return result;
}

void AkosRenderer::codec1_genericDecode() {
	const byte *mask, *src;
	byte *dst;
	byte len, maskbit;
	uint y, color, height, pcolor;
	const byte *scaleytab;
	bool masked;
	bool skip_column = false;

	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;
	height = _height;

	scaleytab = &v1.scaletable[v1.scaleYindex];
	maskbit = revBitMask[v1.x & 7];
	mask = v1.mask_ptr + v1.x / 8;

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;

		do {
			if (*scaleytab++ < _scaleY) {
				if (_actorHitMode) {
					if (color && (int16) y == _actorHitY && v1.x == _actorHitX) {
						_actorHitResult = true;
						return;
					}
				} else {
					masked = (y < _outheight) && v1.mask_ptr && ((mask[0] | mask[v1.imgbufoffs]) & maskbit);

					if (color && y < _outheight && !masked && !skip_column) {
						pcolor = palette[color];
						if (_shadow_mode == 1) {
							if (pcolor == 13)	
								pcolor = _shadow_table[*dst];
						} else if (_shadow_mode == 2) {
							warning("codec1_spec2"); // TODO
						} else if (_shadow_mode == 3) {
							if (pcolor < 8) {
								pcolor = (pcolor << 8) + *dst;
								pcolor = _shadow_table[pcolor];
							}
						}
						*dst = pcolor;
					}
				}
				dst += _outwidth;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--v1.skip_width)
					return;
				height = _height;
				y = v1.y;

				scaleytab = &v1.scaletable[v1.scaleYindex];

				if (v1.scaletable[v1.scaleXindex] < _scaleX) {
					v1.x += v1.scaleXstep;
					if (v1.x < 0 || v1.x >= (int)_outwidth)
						return;
					maskbit = revBitMask[v1.x & 7];
					v1.destptr += v1.scaleXstep;
					skip_column = false;
				} else
					skip_column = true;
				v1.scaleXindex += v1.scaleXstep;
				dst = v1.destptr;
				mask = v1.mask_ptr + v1.x / 8;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

#ifdef __PALM_OS__
const byte *defaultScaleTable;
#else
const byte defaultScaleTable[768] = {
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFE,

	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFE,

	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};
#endif

byte AkosRenderer::codec1(int xmoveCur, int ymoveCur) {
	int num_colors;
	bool use_scaling;
	int i, j;
	int skip = 0, startScaleIndexX, startScaleIndexY;
	Common::Rect rect;
	int cur_x, cur_y;
	int step;
	byte drawFlag = 1;

	/* implement custom scale table */

	v1.scaletable = defaultScaleTable;

	if (_vm->VAR_CUSTOMSCALETABLE != 0xFF && _vm->isGlobInMemory(rtString, _vm->VAR(_vm->VAR_CUSTOMSCALETABLE))) {
		v1.scaletable = _vm->getStringAddressVar(_vm->VAR_CUSTOMSCALETABLE);
	}

	// Setup color decoding variables
	num_colors = _vm->getResourceDataSize(akpl);
	if (num_colors == 32) {
		v1.mask = (1 << 3) - 1;
		v1.shr = 3;
	} else if (num_colors == 64) {
		v1.mask = (1 << 2) - 1;
		v1.shr = 2;
	} else {
		v1.mask = (1 << 4) - 1;
		v1.shr = 4;
	}

	use_scaling = (_scaleX != 0xFF) || (_scaleY != 0xFF);

	cur_x = _actorX;
	cur_y = _actorY;

	if (use_scaling) {

		/* Scale direction */
		v1.scaleXstep = -1;
		if (xmoveCur < 0) {
			xmoveCur = -xmoveCur;
			v1.scaleXstep = 1;
		}

		if (_mirror) {
			/* Adjust X position */
			startScaleIndexX = 0x180 - xmoveCur;
			j = startScaleIndexX;
			for (i = 0; i < xmoveCur; i++) {
				if (v1.scaletable[j++] < _scaleX)
					cur_x -= v1.scaleXstep;
			}

			rect.left = rect.right = cur_x;

			j = startScaleIndexX;
			for (i = 0, skip = 0; i < _width; i++) {
				if (rect.right < 0) {
					skip++;
					startScaleIndexX = j;
				}
				if (v1.scaletable[j++] < _scaleX)
					rect.right++;
			}
		} else {
			/* No mirror */
			/* Adjust X position */
			startScaleIndexX = 0x180 + xmoveCur;
			j = startScaleIndexX;
			for (i = 0; i < xmoveCur; i++) {
				if (v1.scaletable[j++] < _scaleX)
					cur_x += v1.scaleXstep;
			}

			rect.left = rect.right = cur_x;

			j = startScaleIndexX;
			for (i = 0, skip = 0; i < _width; i++) {
				if (rect.left >= (int)_outwidth) {
					startScaleIndexX = j;
					skip++;
				}
				if (v1.scaletable[j--] < _scaleX)
					rect.left--;
			}
		}

		if (skip)
			skip--;

		step = -1;
		if (ymoveCur < 0) {
			ymoveCur = -ymoveCur;
			step = -step;
		}

		startScaleIndexY = 0x180 - ymoveCur;
		for (i = 0; i < ymoveCur; i++) {
			if (v1.scaletable[startScaleIndexY++] < _scaleY)
				cur_y -= step;
		}

		rect.top = rect.bottom = cur_y;
		startScaleIndexY = 0x180 - ymoveCur;
		for (i = 0; i < _height; i++) {
			if (v1.scaletable[startScaleIndexY++] < _scaleY)
				rect.bottom++;
		}

		startScaleIndexY = 0x180 - ymoveCur;
	} else {
		if (!_mirror)
			xmoveCur = -xmoveCur;

		cur_x += xmoveCur;
		cur_y += ymoveCur;

		if (_mirror) {
			rect.left = cur_x;
			rect.right = cur_x + _width;
		} else {
			rect.right = cur_x;
			rect.left = cur_x - _width;
		}

		rect.top = cur_y;
		rect.bottom = cur_y + _height;

		startScaleIndexX = 0x180;
		startScaleIndexY = 0x180;
	}

	v1.scaleXindex = startScaleIndexX;
	v1.scaleYindex = startScaleIndexY;
	v1.skip_width = _width;
	v1.scaleXstep = _mirror ? 1 : -1;

	if (_actorHitMode) {
		if (_actorHitX < rect.left || _actorHitX >= rect.right || _actorHitY < rect.top || _actorHitY >= rect.bottom)
			return 0;
	} else
		_vm->markRectAsDirty(kMainVirtScreen, rect, _actorID);

	if (rect.top >= (int)_outheight || rect.bottom <= 0)
		return 0;

	if (rect.left >= (int)_outwidth || rect.right <= 0)
		return 0;

	v1.replen = 0;

	if (_mirror) {
		if (!use_scaling)
			skip = -cur_x;
		if (skip > 0) {
			v1.skip_width -= skip;
			codec1_ignorePakCols(skip);
			cur_x = 0;
		} else {
			skip = rect.right - _outwidth;
			if (skip <= 0) {
				drawFlag = 2;
			} else {
				v1.skip_width -= skip;
			}
		}
	} else {
		if (!use_scaling)
			skip = rect.right - _outwidth + 1;
		if (skip > 0) {
			v1.skip_width -= skip;
			codec1_ignorePakCols(skip);
			cur_x = _outwidth - 1;
		} else {
			skip = -1 - rect.left;
			if (skip <= 0) {
				drawFlag = 2;
			} else {
				v1.skip_width -= skip;
			}
		}
	}

	v1.x = cur_x;
	v1.y = cur_y;

	if (v1.skip_width <= 0 || _height <= 0)
		return 0;

	if ((uint) rect.top > _outheight)
		rect.top = 0;

	if ((uint) rect.bottom > _outheight)
		rect.bottom = _outheight;

	if (_draw_top > rect.top)
		_draw_top = rect.top;
	if (_draw_bottom < rect.bottom)
		_draw_bottom = rect.bottom;

	v1.destptr = _outptr + v1.y * _outwidth + v1.x;

	v1.mask_ptr = _vm->getMaskBuffer(0, v1.y, 0);
	v1.imgbufoffs = _vm->gdi._imgBufOffs[_zbuf];

	codec1_genericDecode();
	
	return drawFlag;
}


byte AkosRenderer::codec5(int xmoveCur, int ymoveCur) {
	Common::Rect clip;
	int32 maxw, maxh;

	if (_actorHitMode) {
		warning("codec5: _actorHitMode not yet implemented");
		return 0;
	}

	if (!_mirror) {
		clip.left = (_actorX - xmoveCur - _width) + 1;
	} else {
		clip.left = _actorX + xmoveCur - 1;
	}

	clip.top = _actorY + ymoveCur;
	clip.right = (clip.left + _width) - 1;
	clip.bottom = (clip.top + _height) - 1;
	maxw = _outwidth - 1;
	maxh = _outheight - 1;

	_vm->markRectAsDirty(kMainVirtScreen, clip.left, clip.right + 1, clip.top, clip.bottom + 1, _actorID);

	clip.clip(maxw, maxh);

	if ((clip.left >= clip.right) || (clip.top >= clip.bottom))
		return 0;

	if (_draw_top > clip.top)
		_draw_top = clip.top;
	if (_draw_bottom < clip.bottom)
		_draw_bottom = clip.bottom + 1;

	BompDrawData bdd;

	bdd.srcwidth = _width;
	bdd.srcheight = _height;
	bdd.out = _outptr;
	bdd.outwidth = _outwidth;
	bdd.outheight = _outheight;
	bdd.dataptr = _srcptr;
	bdd.scale_x = 255;
	bdd.scale_y = 255;
	bdd.shadowMode = _shadow_mode;

	bdd.scalingXPtr = NULL;
	bdd.scalingYPtr = NULL;
	bdd.scaleRight = 0;
	bdd.scaleBottom = 0;

	if (!_mirror) {
		bdd.x = (_actorX - xmoveCur - _width) + 1;
	} else {
		bdd.x = _actorX + xmoveCur;
	}
	bdd.y = _actorY + ymoveCur;

	if (_zbuf != 0) {
		bdd.maskPtr = _vm->getMaskBuffer(0, 0, _zbuf);
		_vm->drawBomp(bdd, !_mirror);
	} else {
		_vm->drawBomp(bdd, !_mirror);
	}

	_vm->_bompActorPalettePtr = NULL;
	
	return 0;
}

void AkosRenderer::akos16SetupBitReader(const byte *src) {
	akos16.unk5 = 0;
	akos16.numbits = 16;
	akos16.mask = (1 << *src) - 1;
	akos16.shift = *(src);
	akos16.color = *(src + 1);
	akos16.bits = (*(src + 2) | *(src + 3) << 8);
	akos16.dataptr = src + 4;
}

#define AKOS16_FILL_BITS()                                        \
        if (akos16.numbits <= 8) {                                \
          akos16.bits |= (*akos16.dataptr++) << akos16.numbits;   \
          akos16.numbits += 8;                                    \
        }

#define AKOS16_EAT_BITS(n)                                        \
		akos16.numbits -= (n);                                    \
		akos16.bits >>= (n);


void AkosRenderer::akos16SkipData(int32 numbytes) {
	akos16DecodeLine(0, numbytes, 0);
}

void AkosRenderer::akos16DecodeLine(byte *buf, int32 numbytes, int32 dir) {
	uint16 bits, tmp_bits;

	while (numbytes != 0) {
		if (buf) {
			*buf = akos16.color;
			buf += dir;
		}
		
		if (akos16.unk5 == 0) {
			AKOS16_FILL_BITS()
			bits = akos16.bits & 3;
			if (bits & 1) {
				AKOS16_EAT_BITS(2)
				if (bits & 2) {
					tmp_bits = akos16.bits & 7;
					AKOS16_EAT_BITS(3)
					if (tmp_bits != 4) {
						akos16.color += (tmp_bits - 4);
					} else {
						akos16.unk5 = 1;
						AKOS16_FILL_BITS()
						akos16.unk6 = (akos16.bits & 0xff) - 1;
						AKOS16_EAT_BITS(8)
						AKOS16_FILL_BITS()
					}
				} else {
					AKOS16_FILL_BITS()
					akos16.color = ((byte)akos16.bits) & akos16.mask;
					AKOS16_EAT_BITS(akos16.shift)
					AKOS16_FILL_BITS()
				}
			} else {
				AKOS16_EAT_BITS(1);
			}
		} else {
			if (--akos16.unk6 == 0) {
				akos16.unk5 = 0;
			}
		}
		numbytes--;
	}
}

void AkosRenderer::akos16Decompress(byte *dest, int32 pitch, const byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency) {
	byte *tmp_buf = akos16.buffer;

	if (dir < 0) {
		dest -= (t_width - 1);
		tmp_buf += (t_width - 1);
	}

	akos16SetupBitReader(src);

	if (numskip_before != 0) {
		akos16SkipData(numskip_before);
	}

	assert(t_height > 0);
	assert(t_width > 0);
	while (t_height--) {
		akos16DecodeLine(tmp_buf, t_width, dir);
		bompApplyShadow(_shadow_mode, _shadow_table, akos16.buffer, dest, t_width, transparency);

		if (numskip_after != 0)	{
			akos16SkipData(numskip_after);
		}
		dest += pitch;
	}
}

void AkosRenderer::akos16DecompressMask(byte *dest, int32 pitch, const byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency, byte * maskptr, int32 bitpos_start) {
	byte *tmp_buf = akos16.buffer;
	int maskpitch;

	if (dir < 0) {
		dest -= (t_width - 1);
		tmp_buf += (t_width - 1);
	}

	akos16SetupBitReader(src);

	if (numskip_before != 0) {
		akos16SkipData(numskip_before);
	}

	maskpitch = _numStrips;

	assert(t_height > 0);
	assert(t_width > 0);
	while (t_height--) {
		akos16DecodeLine(tmp_buf, t_width, dir);
		bompApplyMask(akos16.buffer, maskptr, (byte)bitpos_start, t_width, transparency);
		bompApplyShadow(_shadow_mode, _shadow_table, akos16.buffer, dest, t_width, transparency);

		if (numskip_after != 0)	{
			akos16SkipData(numskip_after);
		}
		dest += pitch;
		maskptr += maskpitch;
	}
}

byte AkosRenderer::codec16(int xmoveCur, int ymoveCur) {
	Common::Rect clip;
	int32 maxw, maxh;
	int32 skip_x, skip_y, cur_x, cur_y;
	byte transparency;
	//FIXME There should be better method to determine transparency in HE games
	if (_vm->_features & GF_HUMONGOUS) 
		transparency = (_vm->_gameId == GID_FBEAR) ? 5 : 0;
	else
		transparency = 255;

	if (_actorHitMode) {
		warning("codec16: _actorHitMode not yet implemented");
		return 0;
	}
	
	if (!_mirror) {
		clip.left = (_actorX - xmoveCur - _width) + 1;
	} else {
		clip.left = _actorX + xmoveCur;
	}

	clip.top = ymoveCur + _actorY;
	clip.right = (clip.left + _width) - 1;
	clip.bottom = (clip.top + _height) - 1;
	maxw = _outwidth - 1;
	maxh = _outheight - 1;

	skip_x = 0;
	skip_y = 0;
	cur_x = _width - 1;
	cur_y = _height - 1;

	_vm->markRectAsDirty(kMainVirtScreen, clip.left, clip.right + 1, clip.top, clip.bottom + 1, _actorID);

	if (clip.left < 0) {
		skip_x = -clip.left;
		clip.left = 0;
	}

	if (clip.right > maxw) {
		cur_x -= clip.right - maxw;
		clip.right = maxw;
	}

	if (clip.top < 0) {
		skip_y -= clip.top;
		clip.top = 0;
	}

	if (clip.bottom > maxh) {
		cur_y -= clip.bottom - maxh;
		clip.bottom = maxh;
	}

	if ((clip.left >= clip.right) || (clip.top >= clip.bottom))
		return 0;

	if (_draw_top > clip.top)
		_draw_top = clip.top;
	if (_draw_bottom < clip.bottom)
		_draw_bottom = clip.bottom + 1;

	int32 width_unk, height_unk;

	height_unk = clip.top;
	int32 pitch = _outwidth;

	int32 dir;

	if (!_mirror) {
		dir = -1;

		int tmp_skip_x = skip_x;
		skip_x = _width - 1 - cur_x;
		cur_x = _width - 1 - tmp_skip_x;
		width_unk = clip.right;
	} else {
		dir = 1;
		width_unk = clip.left;
	}

	int32 out_height;

	out_height = cur_y - skip_y;
	if (out_height < 0) {
		out_height = -out_height;
	}
	out_height++;

	cur_x -= skip_x;
	if (cur_x < 0) {
		cur_x = -cur_x;
	}
	cur_x++;

	int32 numskip_before = skip_x + (skip_y * _width);
	int32 numskip_after = _width - cur_x;

	byte *dest = _outptr + width_unk + height_unk * _outwidth;

	if (_zbuf == 0) {
		akos16Decompress(dest, pitch, _srcptr, cur_x, out_height, dir, numskip_before, numskip_after, transparency);
	} else {
		byte *ptr = _vm->getMaskBuffer(clip.left, clip.top, _zbuf);
		akos16DecompressMask(dest, pitch, _srcptr, cur_x, out_height, dir, numskip_before, numskip_after, transparency, ptr, clip.left / 8);
	}
	
	return 0;
}

bool ScummEngine::akos_increaseAnims(const byte *akos, Actor *a) {
	const byte *aksq, *akfo;
	int i;
	uint size;
	bool result;

	aksq = findResourceData(MKID('AKSQ'), akos);
	akfo = findResourceData(MKID('AKFO'), akos);

	size = getResourceDataSize(akfo) / 2;

	result = false;
	for (i = 0; i < 16; i++) {
		if (a->cost.active[i] != 0)
			result |= akos_increaseAnim(a, i, aksq, (const uint16 *)akfo, size);
	}
	return result;
}

#define GW(o) ((int16)READ_LE_UINT16(aksq+curpos+(o)))
#define GUW(o) READ_LE_UINT16(aksq+curpos+(o))
#define GB(o) aksq[curpos+(o)]

bool ScummEngine::akos_increaseAnim(Actor *a, int chan, const byte *aksq, const uint16 *akfo, int numakfo) {
	byte active;
	uint old_curpos, curpos, end;
	uint code;
	bool flag_value;
	int tmp, tmp2;

	active = a->cost.active[chan];
	end = a->cost.end[chan];
	old_curpos = curpos = a->cost.curpos[chan];
	flag_value = false;

	do {

		code = aksq[curpos];
		if (code & 0x80)
			code = (code << 8) | aksq[curpos + 1];

		switch (active) {
		case 6:
			switch (code) {
			case AKC_JumpIfSet:
			case AKC_AddVar:
			case AKC_SetVar:
			case AKC_SkipGE:
			case AKC_SkipG:
			case AKC_SkipLE:
			case AKC_SkipL:

			case AKC_SkipNE:
			case AKC_SkipE:
				curpos += 5;
				break;
			case AKC_JumpTable:
			case AKC_SetActorClip:
			case AKC_Ignore3:
			case AKC_Ignore2:
			case AKC_Ignore:
			case AKC_StartAnim:
			case AKC_StartVarAnim:
			case AKC_CmdQue3:
				curpos += 3;
				break;
			case AKC_SoundStuff:
				if (_features & GF_HUMONGOUS) 
					curpos += 6;
				else
					curpos += 8;
				break;
			case AKC_Cmd3:
			case AKC_SetVarInActor:
			case AKC_SetDrawOffs:
				curpos += 6;
				break;
			case AKC_ClearFlag:
			case AKC_HideActor:
			case AKC_IncVar:
			case AKC_CmdQue3Quick:
			case AKC_Return:
			case AKC_EndSeq:
				curpos += 2;
				break;
			case AKC_JumpGE:
			case AKC_JumpG:
			case AKC_JumpLE:
			case AKC_JumpL:
			case AKC_JumpNE:
			case AKC_JumpE:
			case AKC_Random:
				curpos += 7;
				break;
			case AKC_Flip:
			case AKC_Jump:
			case AKC_StartAnimInActor:
				curpos += 4;
				break;
			case AKC_ComplexChan:
				curpos += 3;
				tmp = aksq[curpos - 1];
				while (--tmp >= 0) {
					curpos += 4;
					curpos += (aksq[curpos] & 0x80) ? 2 : 1;
				}
				break;
			default:
				if ((code & 0xC000) == 0xC000)
					error("akos_increaseAnim: invalid code %x", code);
				curpos += (code & 0x8000) ? 2 : 1;
			}
			break;
		case 2:
			curpos += (code & 0x8000) ? 2 : 1;
			if (curpos > end)
				curpos = a->cost.start[chan];
			break;
		case 3:
			if (curpos != end)
				curpos += (code & 0x8000) ? 2 : 1;
			break;
		}

		code = aksq[curpos];
		if (code & 0x80)
			code = (code << 8) | aksq[curpos + 1];

		if (flag_value && code != AKC_ClearFlag)
			continue;

		switch (code) {
		case AKC_StartAnimInActor:
			akos_queCommand(4, derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:29"), a->getAnimVar(GB(3)), 0);
			continue;

		case AKC_Random:
			a->setAnimVar(GB(6), _rnd.getRandomNumberRng(GW(2), GW(4)));
			continue;
		case AKC_JumpGE:
		case AKC_JumpG:
		case AKC_JumpLE:
		case AKC_JumpL:
		case AKC_JumpNE:
		case AKC_JumpE:
			if (akos_compare(a->getAnimVar(GB(4)), GW(5), code - AKC_JumpStart) != 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_IncVar:
			a->setAnimVar(0, a->getAnimVar(0) + 1);
			continue;
		case AKC_SetVar:
			a->setAnimVar(GB(4), GW(2));
			continue;
		case AKC_AddVar:
			a->setAnimVar(GB(4), a->getAnimVar(GB(4)) + GW(2));
			continue;
		case AKC_Flip:
			a->flip = GW(2) != 0;
			continue;
		case AKC_CmdQue3:
			if (_features & GF_HUMONGOUS)
				tmp = GB(2);
			else
				tmp = GB(2) - 1;
			if ((uint) tmp < 8)
				akos_queCommand(3, a, a->sound[tmp], 0);
			continue;
		case AKC_CmdQue3Quick:
			akos_queCommand(3, a, a->sound[0], 0);
			continue;
		case AKC_StartAnim:
			akos_queCommand(4, a, GB(2), 0);
			continue;
		case AKC_StartVarAnim:
			akos_queCommand(4, a, a->getAnimVar(GB(2)), 0);
			continue;
		case AKC_SetVarInActor:
			derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:9")->setAnimVar(GB(3), GW(4));
			continue;
		case AKC_HideActor:
			akos_queCommand(1, a, 0, 0);
			continue;
		case AKC_SetActorClip:
			akos_queCommand(5, a, GB(2), 0);
			continue;
		case AKC_SoundStuff:
			if (_features & GF_HUMONGOUS)
				continue;
			tmp = GB(2) - 1;
			if (tmp >= 8)
				continue;
			tmp2 = GB(4);
			if (tmp2 < 1 || tmp2 > 3)
				error("akos_increaseAnim:8 invalid code %d", tmp2);
			akos_queCommand(tmp2 + 6, a, a->sound[tmp], GB(6));
			continue;
		case AKC_SetDrawOffs:
			akos_queCommand(6, a, GW(2), GW(4));
			continue;
		case AKC_JumpTable:
			if (akfo == NULL)
				error("akos_increaseAnim: no AKFO table");
			tmp = a->getAnimVar(GB(2)) - 1;
			if (tmp < 0 || tmp > numakfo - 1)
				error("akos_increaseAnim: invalid jump value %d", tmp);
			curpos = READ_LE_UINT16(&akfo[tmp]);
			break;
		case AKC_JumpIfSet:
			if (!a->getAnimVar(GB(4)))
				continue;
			a->setAnimVar(GB(4), 0);
			curpos = GUW(2);
			break;

		case AKC_ClearFlag:
			flag_value = false;
			continue;

		case AKC_Jump:
			curpos = GUW(2);
			break;

		case AKC_Return:
		case AKC_EndSeq:
		case AKC_ComplexChan:
			break;

		case AKC_Cmd3:
		case AKC_Ignore:
		case AKC_Ignore2:
		case AKC_Ignore3:
			continue;

		case AKC_SkipE:
		case AKC_SkipNE:
		case AKC_SkipL:
		case AKC_SkipLE:
		case AKC_SkipG:
		case AKC_SkipGE:
			if (akos_compare(a->getAnimVar(GB(4)), GW(2), code - AKC_SkipStart) == 0)
				flag_value = true;
			continue;

		default:
			if ((code & 0xC000) == 0xC000)
				error("Undefined uSweat token %X", code);
		}
		break;
	} while (1);

	int code2 = aksq[curpos];
	if (code2 & 0x80)
		code2 = (code2 << 8) | aksq[curpos + 1];
	assert((code2 & 0xC000) != 0xC000 || code2 == AKC_ComplexChan || code2 == AKC_Return || code2 == AKC_EndSeq);

	a->cost.curpos[chan] = curpos;

	return curpos != old_curpos;
}

void ScummEngine::akos_queCommand(byte cmd, Actor *a, int param_1, int param_2) {
	switch (cmd) {
	case 1:
		a->putActor(0, 0, 0);
		break;
	case 2:
		warning("unimplemented akos_queCommand(2,%d,%d,%d)", a->number, param_1, param_2);
		// start script token in actor
		break;
	case 3:
		if (param_1 != 0) {
			if (_features & GF_DIGI_IMUSE)
				_imuseDigital->startSfx(param_1);
			else
				_sound->addSoundToQueue(param_1);
		}
		break;
	case 4:
		a->startAnimActor(param_1);
		break;
	case 5:
		a->forceClip = param_1;
		break;
	case 6:
		a->offs_x = param_1;
		a->offs_y = param_2;
		break;
	case 7:
		if (param_1 != 0) {
			if (_imuseDigital) {
				_imuseDigital->setVolume(param_1, param_2);
			}
		}
		break;
	case 8:
		if (param_1 != 0) {
			if (_imuseDigital) {
				_imuseDigital->setPan(param_1, param_2);
			}
		}
		break;
	case 9:
		if (param_1 != 0) {
			if (_imuseDigital) {
//				_imuseDigital->setPriority(param_1, param_2);
			}
		}
		break;

	default:
		warning("akos_queCommand(%d,%d,%d,%d)", cmd, a->number, param_1, param_2);
	}
}

} // End of namespace Scumm

#ifdef __PALM_OS__
#include "scumm_globals.h"

_GINIT(Akos)
_GSETPTR(Scumm::defaultScaleTable, GBVARS_DEFAULTSCALETABLE_INDEX, byte, GBVARS_SCUMM)
_GEND

_GRELEASE(Akos)
_GRELEASEPTR(GBVARS_DEFAULTSCALETABLE_INDEX, GBVARS_SCUMM)
_GEND

#endif
