/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "scumm.h"
#include "actor.h"
#include "akos.h"

bool Scumm::akos_hasManyDirections(Actor *a)
{
	if (_features & GF_NEW_COSTUMES) {
		byte *akos;
		AkosHeader *akhd;

		akos = getResourceAddress(rtCostume, a->costume);
		assert(akos);

		akhd = (AkosHeader *) findResourceData(MKID('AKHD'), akos);
		return (akhd->flags & 2) != 0;
	}
	return 0;
}

int Scumm::akos_frameToAnim(Actor *a, int frame)
{
	if (akos_hasManyDirections(a))
		return toSimpleDir(1, a->facing) + frame * 8;
	else
		return newDirToOldDir(a->facing) + frame * 4;
}

void Scumm::akos_decodeData(Actor *a, int frame, uint usemask)
{
	uint anim;
	byte *akos, *r;
	AkosHeader *akhd;
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

	akhd = (AkosHeader *) findResourceData(MKID('AKHD'), akos);

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

void AkosRenderer::setPalette(byte *new_palette)
{
	byte *the_akpl;
	uint size, i;

	the_akpl = _vm->findResourceData(MKID('AKPL'), akos);
	size = _vm->getResourceDataSize(akpl);

	if (size > 256)
		error("akos_setPalette: %d is too many colors", size);

	for (i = 0; i < size; i++) {
		palette[i] = new_palette[i] != 0xFF ? new_palette[i] : the_akpl[i];
	}
}

void AkosRenderer::setCostume(int costume)
{
	akos = _vm->getResourceAddress(rtCostume, costume);
	assert(akos);

	akhd = (AkosHeader *) _vm->findResourceData(MKID('AKHD'), akos);
	akof = (AkosOffset *) _vm->findResourceData(MKID('AKOF'), akos);
	akci = _vm->findResourceData(MKID('AKCI'), akos);
	aksq = _vm->findResourceData(MKID('AKSQ'), akos);
	akcd = _vm->findResourceData(MKID('AKCD'), akos);
	akpl = _vm->findResourceData(MKID('AKPL'), akos);
	codec = READ_LE_UINT16(&akhd->codec);
}

void AkosRenderer::setFacing(Actor *a)
{
	mirror = (newDirToOldDir(a->facing) != 0 || akhd->flags & 1);
	if (a->flip)
		mirror ^= 1;
}

bool AkosRenderer::drawCostume()
{
	int i;
	bool result = false;

	move_x = move_y = 0;
	for (i = 0; i < 16; i++)
		result |= drawCostumeChannel(i);
	return result;
}

bool AkosRenderer::drawCostumeChannel(int chan)
{
	uint code;
	byte *p;
	AkosOffset *off;
	AkosCI *the_akci;
	uint i, extra;

	if (!cd->active[chan] || cd->stopped & (1 << chan))
		return false;

	p = aksq + cd->curpos[chan];

	code = p[0];
	if (code & 0x80)
		code = (code << 8) | p[1];

	if (code == AKC_Return)
		return false;

	if (code != AKC_ComplexChan) {
		off = akof + (code & 0xFFF);

		assert((code & 0xFFF) * 6 < READ_BE_UINT32_UNALIGNED((byte *)akof - 4) - 8);
		assert((code & 0x7000) == 0);

		srcptr = akcd + READ_LE_UINT32(&off->akcd);
		the_akci = (AkosCI *) (akci + READ_LE_UINT16(&off->akci));

		move_x_cur = move_x + (int16)READ_LE_UINT16(&the_akci->rel_x);
		move_y_cur = move_y + (int16)READ_LE_UINT16(&the_akci->rel_y);
		width = READ_LE_UINT16(&the_akci->width);
		height = READ_LE_UINT16(&the_akci->height);
		move_x += (int16)READ_LE_UINT16(&the_akci->move_x);
		move_y -= (int16)READ_LE_UINT16(&the_akci->move_y);

		switch (codec) {
		case 1:
			codec1();
			break;
		case 5:
			codec5();
			break;
		case 16:
			codec16();
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

			srcptr = akcd + READ_LE_UINT32(&off->akcd);
			the_akci = (AkosCI *) (akci + READ_LE_UINT16(&off->akci));

			move_x_cur = move_x + (int16)READ_LE_UINT16(p + 0);
			move_y_cur = move_y + (int16)READ_LE_UINT16(p + 2);

			p += (p[4] & 0x80) ? 6 : 5;

			width = READ_LE_UINT16(&the_akci->width);
			height = READ_LE_UINT16(&the_akci->height);

			switch (codec) {
			case 1:
				codec1();
				break;
			case 5:
				codec5();
				break;
			case 16:
				codec16();
				break;
			default:
				error("akos_drawCostumeChannel: invalid codec %d", codec);
			}
		}
	}

	return true;
}

void akos_c1_0y_decode(AkosRenderer * ar)
{
	byte len, color;
	byte *src, *dst;
	int height;
	uint y;
	uint scrheight;

	len = ar->v1.replen;
	src = ar->srcptr;
	dst = ar->v1.destptr;
	color = ar->v1.repcolor;
	scrheight = ar->outheight;
	height = ar->height;
	y = ar->v1.y;

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> ar->v1.shl;
		len &= ar->v1.mask;
		if (!len)
			len = *src++;

		do {
			if (color && y < scrheight) {
				*dst = ar->palette[color];
			}

			dst += ar->outwidth;
			y++;
			if (!--height) {
				if (!--ar->v1.skip_width)
					return;
				dst -= ar->v1.y_pitch;
				height = ar->height;
				y = ar->v1.y;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void akos_generic_decode(AkosRenderer * ar)
{
	byte *src, *dst;
	byte len, height, maskbit;
	uint y, color;
	const byte *scaleytab, *mask;


	y = ar->v1.y;

	len = ar->v1.replen;
	src = ar->srcptr;
	dst = ar->v1.destptr;
	color = ar->v1.repcolor;
	height = ar->height;

	scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];
	maskbit = revBitMask[ar->v1.x & 7];
	mask = ar->v1.mask_ptr + (ar->v1.x >> 3);

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> ar->v1.shl;
		len &= ar->v1.mask;
		if (!len)
			len = *src++;

		do {
			if (*scaleytab++ < ar->scale_y) {
				if (color && y < ar->outheight
						&& (!ar->v1.mask_ptr || !((mask[0] | mask[ar->v1.imgbufoffs]) & maskbit))) {
					*dst = ar->palette[color];
				}
				mask += 40;
				dst += ar->outwidth;
				y++;
			}
			if (!--height) {
				if (!--ar->v1.skip_width)
					return;
				height = ar->height;
				y = ar->v1.y;

				scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];

				if (ar->v1.scaletable[ar->v1.tmp_x] < ar->scale_x) {
					ar->v1.x += ar->v1.scaleXstep;
					if (ar->v1.x >= g_scumm->_realWidth)
						return;
					maskbit = revBitMask[ar->v1.x & 7];
					ar->v1.destptr += ar->v1.scaleXstep;
				}
				mask = ar->v1.mask_ptr + (ar->v1.x >> 3);
				ar->v1.tmp_x += ar->v1.scaleXstep;
				dst = ar->v1.destptr;
			}
		StartPos:;
		} while (--len);
	} while (1);
}


void akos_c1_spec1(AkosRenderer * ar)
{
	byte *src, *dst;
	byte len, height, pcolor, maskbit;
	uint y, color;
	const byte *scaleytab, *mask;


	y = ar->v1.y;

	len = ar->v1.replen;
	src = ar->srcptr;
	dst = ar->v1.destptr;
	color = ar->v1.repcolor;
	height = ar->height;

	scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];
	maskbit = revBitMask[ar->v1.x & 7];
	mask = ar->v1.mask_ptr + (ar->v1.x >> 3);

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> ar->v1.shl;
		len &= ar->v1.mask;
		if (!len)
			len = *src++;

		do {
			if (*scaleytab++ < ar->scale_y) {
				if (color && y < ar->outheight
						&& (!ar->v1.mask_ptr || !((mask[0] | mask[ar->v1.imgbufoffs]) & maskbit))) {
					pcolor = ar->palette[color];
					if (pcolor == 13)
						pcolor = ar->shadow_table[*dst];
					*dst = pcolor;
				}
				mask += 40;
				dst += ar->outwidth;
				y++;
			}
			if (!--height) {
				if (!--ar->v1.skip_width)
					return;
				height = ar->height;
				y = ar->v1.y;

				scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];

				if (ar->v1.scaletable[ar->v1.tmp_x] < ar->scale_x) {
					ar->v1.x += ar->v1.scaleXstep;
					if (ar->v1.x >= g_scumm->_realWidth)
						return;
					maskbit = revBitMask[ar->v1.x & 7];
					ar->v1.destptr += ar->v1.scaleXstep;
				}
				mask = ar->v1.mask_ptr + (ar->v1.x >> 3);
				ar->v1.tmp_x += ar->v1.scaleXstep;
				dst = ar->v1.destptr;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void akos_c1_spec3(AkosRenderer * ar)
{
	byte *src, *dst;
	byte len, height, maskbit;
	uint y, color, pcolor;
	const byte *scaleytab, *mask;


	y = ar->v1.y;

	len = ar->v1.replen;
	src = ar->srcptr;
	dst = ar->v1.destptr;
	color = ar->v1.repcolor;
	height = ar->height;

	scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];
	maskbit = revBitMask[ar->v1.x & 7];
	mask = ar->v1.mask_ptr + (ar->v1.x >> 3);

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> ar->v1.shl;
		len &= ar->v1.mask;
		if (!len)
			len = *src++;

		do {
			if (*scaleytab++ < ar->scale_y) {
				if (color && y < ar->outheight
						&& (!ar->v1.mask_ptr || !((mask[0] | mask[ar->v1.imgbufoffs]) & maskbit))) {
					pcolor = ar->palette[color];
					if (pcolor < 8) {
						pcolor = (pcolor << 8) + *dst;
						*dst = ar->shadow_table[pcolor];
					} else {
						*dst = pcolor;
					}
				}
				mask += 40;
				dst += ar->outwidth;
				y++;
			}
			if (!--height) {
				if (!--ar->v1.skip_width)
					return;
				height = ar->height;
				y = ar->v1.y;

				scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];

				if (ar->v1.scaletable[ar->v1.tmp_x] < ar->scale_x) {
					ar->v1.x += ar->v1.scaleXstep;
					if (ar->v1.x >= g_scumm->_realWidth)
						return;
					maskbit = revBitMask[ar->v1.x & 7];
					ar->v1.destptr += ar->v1.scaleXstep;
				}
				mask = ar->v1.mask_ptr + (ar->v1.x >> 3);
				ar->v1.tmp_x += ar->v1.scaleXstep;
				dst = ar->v1.destptr;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

const byte default_scale_table[768] = {
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


void AkosRenderer::codec1()
{
	int num_colors;
	bool use_scaling;
	int i, j;
	int cur_x, x_right, x_left, skip = 0, tmp_x, tmp_y;
	int cur_y, y_top, y_bottom;
	bool y_clipping;
	bool masking;
	int step;


	/* implement custom scale table */



	if (_vm->isGlobInMemory(rtString, _vm->_vars[_vm->VAR_CUSTOMSCALETABLE])) {
		v1.scaletable = _vm->getStringAddressVar(_vm->VAR_CUSTOMSCALETABLE);
	} else {
		v1.scaletable = default_scale_table;
	}

	/* Setup color decoding variables */
	num_colors = _vm->getResourceDataSize(akpl);
	if (num_colors == 32) {
		v1.mask = (1 << 3) - 1;
		v1.shl = 3;
	} else if (num_colors == 64) {
		v1.mask = (1 << 2) - 1;
		v1.shl = 2;
	} else {
		v1.mask = (1 << 4) - 1;
		v1.shl = 4;
	}

	use_scaling = (scale_x != 0xFF) || (scale_y != 0xFF);

	cur_x = x;
	cur_y = y;

	if (use_scaling) {

		/* Scale direction */
		v1.scaleXstep = -1;
		if (move_x_cur < 0) {
			move_x_cur = -move_x_cur;
			v1.scaleXstep = 1;
		}

		if (mirror) {
			/* Adjust X position */
			tmp_x = 0x180 - move_x_cur;
			j = tmp_x;
			for (i = 0; i < move_x_cur; i++) {
				if (v1.scaletable[j++] < scale_x)
					cur_x -= v1.scaleXstep;
			}

			x_left = x_right = cur_x;

			j = tmp_x;
			for (i = 0, skip = 0; i < width; i++) {
				if (x_right < 0) {
					skip++;
					tmp_x = j;
				}
				if (v1.scaletable[j++] < scale_x)
					x_right++;
			}
		} else {
			/* No mirror */
			/* Adjust X position */
			tmp_x = 0x180 + move_x_cur;
			j = tmp_x;
			for (i = 0; i < move_x_cur; i++) {
				if (v1.scaletable[j++] < scale_x)
					cur_x += v1.scaleXstep;
			}

			x_left = x_right = cur_x;

			j = tmp_x;
			for (i = 0, skip = 0; i < width; i++) {
				if (x_left >= (int)outwidth) {
					tmp_x = j;
					skip++;
				}
				if (v1.scaletable[j--] < scale_x)
					x_left--;
			}
		}

		if (skip)
			skip--;

		step = -1;
		if (move_y_cur < 0) {
			move_y_cur = -move_y_cur;
			step = -step;
		}

		tmp_y = 0x180 - move_y_cur;
		for (i = 0; i < move_y_cur; i++) {
			if (v1.scaletable[tmp_y++] < scale_y)
				cur_y -= step;
		}

		y_top = y_bottom = cur_y;
		tmp_y = 0x180 - move_y_cur;
		for (i = 0; i < height; i++) {
			if (v1.scaletable[tmp_y++] < scale_y)
				y_bottom++;
		}

		tmp_y = 0x180 - move_y_cur;
	} else {
		if (!mirror)
			move_x_cur = -move_x_cur;

		cur_x += move_x_cur;
		cur_y += move_y_cur;

		if (mirror) {
			x_left = cur_x;
			x_right = cur_x + width;
		} else {
			x_right = cur_x;
			x_left = cur_x - width;
		}

		y_top = cur_y;
		y_bottom = cur_y + height;

		tmp_x = 0x180;
		tmp_y = 0x180;
	}

	v1.tmp_x = tmp_x;
	v1.tmp_y = tmp_y;
	v1.skip_width = width;

	v1.scaleXstep = -1;
	if (mirror)
		v1.scaleXstep = -v1.scaleXstep;

	if ((int) y_top >= (int) outheight || y_bottom <= 0)
		return;

	if ((int)x_left >= (int)outwidth || x_right <= 0)
		return;

	v1.replen = 0;
	v1.y_pitch = height * outwidth;

	if (mirror) {
		v1.y_pitch--;
		if (!use_scaling)
			skip = -cur_x;
		if (skip > 0) {
			v1.skip_width -= skip;
			codec1_ignorePakCols(skip);
			cur_x = 0;
		} else {
			skip = x_right - outwidth;
			if (skip > 0)
				v1.skip_width -= skip;
		}
	} else {
		v1.y_pitch++;
		if (!use_scaling) {
			skip = x_right - outwidth + 1;
		}
		if (skip > 0) {
			v1.skip_width -= skip;
			codec1_ignorePakCols(skip);
			cur_x = outwidth - 1;
		} else {
			skip = -1 - x_left;
			if (skip > 0)
				v1.skip_width -= skip;
		}
	}

	v1.x = cur_x;
	v1.y = cur_y;

	if (v1.skip_width <= 0 || height <= 0)
		return;

	_vm->updateDirtyRect(0, x_left, x_right, y_top, y_bottom, 1 << dirty_id);

	y_clipping = ((uint) y_bottom > outheight || y_top < 0);

	if ((uint) y_top > (uint) outheight)
		y_top = 0;

	if ((uint) y_bottom > (uint) outheight)
		y_bottom = outheight;

	if (y_top < draw_top)
		draw_top = y_top;
	if (y_bottom > draw_bottom)
		draw_bottom = y_bottom;

	if (cur_x == -1)
		cur_x = 0;									/* ?? */

	v1.destptr = outptr + cur_x + cur_y * outwidth;

	masking = false;
	if (clipping) {
		masking = _vm->isMaskActiveAt(x_left, y_top, x_right, y_bottom,
																	_vm->getResourceAddress(rtBuffer, 9) +
																	_vm->gdi._imgBufOffs[clipping] + _vm->_screenStartStrip) != 0;
	}

	v1.mask_ptr = NULL;

	if (masking || charsetmask || shadow_mode) {
		v1.mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + cur_y * 40 + _vm->_screenStartStrip;
		v1.imgbufoffs = _vm->gdi._imgBufOffs[clipping];
		if (!charsetmask && masking) {
			v1.mask_ptr += v1.imgbufoffs;
			v1.imgbufoffs = 0;
		}
	}

	switch (shadow_mode) {
	case 1:
		akos_c1_spec1(this);
		return;
	case 2:
//    akos_c1_spec2(this);
		warning("akos_c1_spec2");
		return;
	case 3:
		akos_c1_spec3(this);
		return;
	}

	akos_generic_decode(this);

//  akos_c1_0y_decode(this);

#if 0

	switch (((byte)y_clipping << 3) | ((byte)use_scaling << 2) | ((byte)masking << 1) | (byte)charsetmask) {
	case 0:
		akos_c1_0_decode(this);
		break;
	case 0 + 8:
		akos_c1_0y_decode(this);
		break;
	case 2:
	case 1:
		akos_c1_12_decode(this);
		break;
	case 2 + 8:
	case 1 + 8:
		akos_c1_12y_decode(this);
		break;
	case 3 + 8:
	case 3:
		akos_c1_3_decode(this);
		break;
	case 4:
		akos_c1_4_decode(this);
		break;
	case 4 + 8:
		akos_c1_4y_decode(this);
		break;
	case 6:
	case 5:
		akos_c1_56_decode(this);
		break;
	case 6 + 8:
	case 5 + 8:
		akos_c1_56y_decode(this);
		break;
	case 7:
	case 7 + 8:
		akos_c1_7_decode(this);
		break;
	}
#endif
}


void AkosRenderer::codec1_ignorePakCols(int num)
{
	int n;
	byte repcolor;
	byte replen;
	byte *src;

	n = height;
	if (num > 1)
		n *= num;
	src = srcptr;
	do {
		repcolor = *src++;
		replen = repcolor & v1.mask;
		if (replen == 0) {
			replen = *src++;
		}
		do {
			if (!--n) {
				v1.repcolor = repcolor >> v1.shl;
				v1.replen = replen;
				srcptr = src;
				return;
			}
		} while (--replen);
	} while (1);
}


void AkosRenderer::codec5()
{
	VirtScreen *vs;

	int left, right, top, bottom;
	int clip_left, clip_right, clip_top, clip_bottom;

	byte *src, *dest;
	int src_x, src_y;
	int dst_x, dst_y;

	bool masking;
	byte maskbit;
	byte *mask = NULL;

	// I don't know if this is complete. It used to simply call drawBomp()
	// to draw an unscaled image, but I don't know if that was because it
	// will never have to scale, or if it's because until quite recently
	// drawBomp() didn't know how to scale images.
	//
	// What I do know is that drawBomp() doesn't care about masking and
	// shadows, and these are both needed for Full Throttle and The Dig.
	
	vs = &_vm->virtscr[0];

	if (!mirror) {
		left = (x - move_x_cur - width) + 1;
	} else {
		left = x + move_x_cur - 1;
	}

	right = left + width;
	top = y + move_y_cur;
	bottom = top + height;

	if (left >= _vm->_realWidth || top >= _vm->_realHeight)
		return;

	// The actual drawing code shouldn't survive even if the image is
	// partially outside the screen, but something before that seems to
	// be less tolerant...

	clip_left = (left >= 0) ? left : 0;
	clip_right = (right > _vm->_realWidth) ? _vm->_realWidth : right;
	clip_top = (top >= 0) ? top : 0;
	clip_bottom = (bottom > _vm->_realHeight) ? _vm->_realHeight : bottom;
	
	if (clip_top < draw_top)
		draw_top = clip_top;
	if (clip_bottom > draw_bottom)
		draw_bottom = clip_bottom;

	_vm->updateDirtyRect(0, clip_left, clip_right, clip_top, clip_bottom, 1 << dirty_id);

	masking = false;
	if (clipping) {
		masking = _vm->isMaskActiveAt(clip_left, clip_top, clip_right, clip_bottom,
			_vm->getResourceAddress(rtBuffer, 9) +
			_vm->gdi._imgBufOffs[clipping] +
			_vm->_screenStartStrip) != 0;
	}

	v1.mask_ptr = NULL;

	if (masking || charsetmask) {
		v1.mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + _vm->_screenStartStrip;
		v1.imgbufoffs = _vm->gdi._imgBufOffs[clipping];
		if (!charsetmask && masking) {
			v1.mask_ptr += v1.imgbufoffs;
			v1.imgbufoffs = 0;
		}
	}

	src = srcptr;
	dest = outptr;

	for (src_y = 0, dst_y = top; src_y < height; src_y++) {
		byte code, color;
		uint num, i;
		byte *d = dest + dst_y * _vm->_realWidth + left;
		byte *s;
		uint data_length;

		data_length = READ_LE_UINT16(src) + 2;

		if (dst_y < 0 || dst_y >= _vm->_realHeight) {
			src += data_length;
			dst_y++;
			continue;
		}

		src_x = 0;
		dst_x = left;
		s = src + 2;

		while (src_x < width) {
			code = *s++;
			num = (code >> 1) + 1;
			if (code & 1) {
				color = *s++;
				for (i = 0; i < num; i++) {
					if (dst_x >= 0 && dst_x < _vm->_realWidth) {
						if (color != 255) {
							if (v1.mask_ptr)
								mask = v1.mask_ptr + 40 * dst_y + (dst_x >> 3);
							maskbit = revBitMask[dst_x & 7];
							if (shadow_mode && color == 13)
								color = shadow_table[*d];
							if (!mask || !((mask[0] | mask[v1.imgbufoffs]) & maskbit))
								*d = color;
						}
					}
					d++;
					dst_x++;
					src_x++;
				}
			} else {
				for (i = 0; i < num; i++) {
					color = s[i];
					if (dst_x >= 0 && dst_x < _vm->_realWidth) {
						if (color != 255) {
							if (v1.mask_ptr)
								mask = v1.mask_ptr + 40 * dst_y + (dst_x >> 3);
							maskbit = revBitMask[dst_x & 7];
							if (shadow_mode && color == 13)
								color = shadow_table[*d];
							if (!mask || !((mask[0] | mask[v1.imgbufoffs]) & maskbit))
								*d = color;
						}
					}
					d++;
					dst_x++;
					src_x++;
				}
				s += num;
			}
		}
		src += data_length;
		dst_y++;
	}
}

void AkosRenderer::codec16()
{
	warning("akos_codec16: not implemented");
}


bool Scumm::akos_increaseAnims(byte *akos, Actor *a)
{
	byte *aksq, *akfo;
	int i;
	uint size;
	bool result;

	aksq = findResourceData(MKID('AKSQ'), akos);
	akfo = findResourceData(MKID('AKFO'), akos);

	size = getResourceDataSize(akfo) >> 1;

	result = false;
	for (i = 0; i != 0x10; i++) {
		if (a->cost.active[i] != 0)
			result |= akos_increaseAnim(a, i, aksq, (uint16 *)akfo, size);
	}
	return result;
}


#define GW(o) ((int16)READ_LE_UINT16(aksq+curpos+(o)))
#define GUW(o) READ_LE_UINT16(aksq+curpos+(o))
#define GB(o) aksq[curpos+(o)]

bool Scumm::akos_increaseAnim(Actor *a, int chan, byte *aksq, uint16 *akfo, int numakfo)
{
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
			case AKC_CmdQue3:
				curpos += 3;
				break;
			case AKC_SoundStuff:
				curpos += 8;
				break;
			case AKC_Cmd3:
			case AKC_SetVarInActor:
			case AKC_SetDrawOffs:
				curpos += 6;
				break;
			case AKC_ClearFlag:
			case AKC_HideActor:
			case AKC_CmdQue3Quick:
			case AKC_Return:
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
				if (code & 0xC000)
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
			akos_queCommand(4, derefActorSafe(a->getAnimVar(GB(2)),
																				"akos_increaseAnim:29"), a->getAnimVar(GB(3)), 0);
			continue;

		case AKC_Random:
			a->setAnimVar(GB(6), getRandomNumberRng(GW(2), GW(4)));
			continue;
		case AKC_SkipGE:
		case AKC_SkipG:
		case AKC_SkipLE:
		case AKC_SkipL:
		case AKC_SkipNE:
		case AKC_SkipE:
			if (!akos_compare(a->getAnimVar(GB(4)), GW(2), code - AKC_SkipStart))
				flag_value = true;
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
			tmp = GB(2);
			if ((uint) tmp < 8)
				akos_queCommand(3, a, a->sound[tmp], 0);
			continue;
		case AKC_CmdQue3Quick:
			akos_queCommand(3, a, a->sound[1], 0);
			continue;
		case AKC_StartAnim:
			akos_queCommand(4, a, GB(2), 0);
			continue;
		case AKC_StartVarAnim:
			akos_queCommand(4, a, a->getAnimVar(GB(2)), 0);
			continue;
		case AKC_SetVarInActor:
			derefActorSafe(a->getAnimVar(GB(2)), "akos_increaseAnim:9")->setAnimVar(GB(3), GW(4)
				);
			continue;
		case AKC_HideActor:
			akos_queCommand(1, a, 0, 0);
			continue;
		case AKC_SetActorClip:
			akos_queCommand(5, a, GB(2), 0);
			continue;
		case AKC_SoundStuff:
			tmp = GB(2);
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
			if (tmp < 0 || tmp >= numakfo - 1)
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
		case AKC_ComplexChan:
			break;

		case AKC_Ignore:
		case AKC_Ignore2:
		case AKC_Ignore3:
			continue;

		case AKC_JumpE:
		case AKC_JumpNE:
		case AKC_JumpL:
		case AKC_JumpLE:
		case AKC_JumpG:
		case AKC_JumpGE:
			if (!akos_compare(a->getAnimVar(GB(4)), GW(2), code - AKC_JumpStart))
				continue;
			curpos = GUW(2);
			break;

		default:
			if ((code & 0xC000) == 0xC000)
				error("Undefined uSweat token %X", code);
		}
		break;
	} while (1);

	int code2 = aksq[curpos];
	if (code2 & 0x80)
		code2 = (code2 << 8) | aksq[curpos + 1];
	assert((code2 & 0xC000) != 0xC000 || code2 == AKC_ComplexChan || code2 == AKC_Return);

	a->cost.curpos[chan] = curpos;

	return curpos != old_curpos;
}

void Scumm::akos_queCommand(byte cmd, Actor *a, int param_1, int param_2)
{
//  warning("akos_queCommand(%d,%d,%d,%d)", cmd, a->number, param_1, param_2);
}


bool Scumm::akos_compare(int a, int b, byte cmd)
{
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
