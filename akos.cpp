/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

bool Scumm::akos_hasManyDirections(Actor *a) {
	if (_features & GF_NEW_COSTUMES) {	
		byte *akos;
		AkosHeader *akhd;

		akos = getResourceAddress(rtCostume, a->costume);
		assert(akos);

		akhd = (AkosHeader*)findResourceData(MKID('AKHD'), akos);
		return (akhd->flags&2) != 0;
	}
	return 0;
}

int Scumm::akos_findManyDirection(int16 ManyDirection, uint16 facing)
{
	int32 direction;
	int32 temp;
	int32 temp_facing;
	temp=many_direction_tab[ManyDirection];
	direction=temp + ManyDirection * 8;
	do{
		if(facing>=many_direction_tab[direction+1])
		{
			if(facing<=many_direction_tab[direction+2])
			{
				return(temp);
			}
		}
		
		--temp;
		--direction;
		
	}while(temp);

	return(temp);
}


int Scumm::akos_frameToAnim(Actor *a, int frame) {
	bool ManyDirection;

	ManyDirection = akos_hasManyDirections(a);
	
	if (ManyDirection){
		frame*=many_direction_tab[ManyDirection];
		return akos_findManyDirection(ManyDirection, a->facing) + frame;
	} else {
		return newDirToOldDir(a->facing) + frame * 4;
	}
}

void Scumm::akos_decodeData(Actor *a, int frame, uint usemask) {
	uint anim;
	byte *akos,*r;
	AkosHeader *akhd;
	uint offs;
	int i;
	byte code;
	uint16 start,len;
	uint16 mask;
		
	if (a->costume==0)
		return;

	anim = akos_frameToAnim(a, frame);

	akos = getResourceAddress(rtCostume, a->costume);
	assert(akos);

	akhd = (AkosHeader*)findResourceData(MKID('AKHD'), akos);

	if (anim>=READ_LE_UINT16(&akhd->num_anims))
		return;

	r = findResourceData(MKID('AKCH'), akos);
	assert(r);

	offs = READ_LE_UINT16(r + anim * sizeof(uint16));
	if (offs==0)
		return;
	r += offs;

	i = 0;
	mask = READ_LE_UINT16(r);
	r+=sizeof(uint16);
	do {
		if (mask&0x8000) {
			code = *r++;
			if (usemask&0x8000) {
				switch(code) {
				case 1:
					a->cost.active[i] = 0;
					a->cost.frame[i] = frame;
					a->cost.end[i] = 0;
					a->cost.start[i] = 0;
					a->cost.curpos[i] = 0;
					break;
				case 4:
					a->cost.stopped |= 1<<i;
					break;
				case 5:
					a->cost.stopped &= ~(1<<i);
					break;
				default:
					start = READ_LE_UINT16(r);
					len = READ_LE_UINT16(r+sizeof(uint16));
					r+=sizeof(uint16)*2;

					a->cost.active[i] = code;
					a->cost.frame[i] = frame;
					a->cost.end[i] = start + len;
					a->cost.start[i] = start;
					a->cost.curpos[i] = start;
					break;
				}
			} else {
				if (code!=1 && code!=4 && code!=5)
					r+=sizeof(uint16)*2;
			}
		}
		i++;
		mask<<=1;
		usemask<<=1;
	} while ((uint16)mask);
}

void Scumm::akos_setPalette(AkosRenderer *ar, byte *palette) {
	byte *akpl;
	uint size, i;

	akpl = findResourceData(MKID('AKPL'), ar->akos);
	size = getResourceDataSize(akpl);

	if (size > 256)
		error("akos_setPalette: %d is too many colors", size);

	for(i=0; i<size; i++) {
		ar->palette[i] = palette[i]!=0xFF ? palette[i] : akpl[i];
	}
}

void Scumm::akos_setCostume(AkosRenderer *ar, int costume) {
	ar->akos = getResourceAddress(rtCostume, costume);
	assert(ar->akos);

	ar->akhd = (AkosHeader*)findResourceData(MKID('AKHD'), ar->akos);
	ar->akof = (AkosOffset*)findResourceData(MKID('AKOF'), ar->akos);
	ar->akci = findResourceData(MKID('AKCI'), ar->akos);
	ar->aksq = findResourceData(MKID('AKSQ'), ar->akos);
	ar->akcd = findResourceData(MKID('AKCD'), ar->akos);
	ar->akpl = findResourceData(MKID('AKPL'), ar->akos);
	ar->codec = READ_LE_UINT16(&ar->akhd->codec);
}

void Scumm::akos_setFacing(AkosRenderer *ar, Actor *a) {
	ar->mirror = (newDirToOldDir(a->facing)!=0 || ar->akhd->flags&1);
	if (a->flip)
		ar->mirror ^= 1;
}

bool Scumm::akos_drawCostume(AkosRenderer *ar) {
	int i;
	bool result = false;

	ar->move_x = ar->move_y = 0;
	for(i=0; i<16; i++)
		result |= akos_drawCostumeChannel(ar, i);
	return result;
}

bool Scumm::akos_drawCostumeChannel(AkosRenderer *ar, int chan) {
	uint code;
	byte *p;
	AkosOffset *off;
	AkosCI *akci;
	uint i,extra;

	if (!ar->cd->active[chan] || ar->cd->stopped&(1<<chan))
		return false;

	p = ar->aksq + ar->cd->curpos[chan];

	code = p[0];
	if (code & 0x80) code = (code<<8)|p[1];

	if (code==AKC_Return)
		return false;

	if (code!=AKC_ComplexChan) {
		off = ar->akof + (code & 0xFFF);

		assert( (code & 0xFFF)*6 < READ_BE_UINT32_UNALIGNED((byte*)ar->akof - 4)-8 );

		assert( (code&0x7000) == 0);

		ar->srcptr = ar->akcd + READ_LE_UINT32(&off->akcd);
		akci = (AkosCI*)(ar->akci + READ_LE_UINT16(&off->akci));

		ar->move_x_cur = ar->move_x + (int16)READ_LE_UINT16(&akci->rel_x);
		ar->move_y_cur = ar->move_y + (int16)READ_LE_UINT16(&akci->rel_y);
		ar->width = READ_LE_UINT16(&akci->width);
		ar->height = READ_LE_UINT16(&akci->height);
		ar->move_x += (int16)READ_LE_UINT16(&akci->move_x);
		ar->move_y -= (int16)READ_LE_UINT16(&akci->move_y);

		switch(ar->codec) {
		case 1:
			akos_codec1(ar);
			break;
		case 5:
			akos_codec5(ar);
			break;
		case 16:
			akos_codec16(ar);
			break;
		default:
			error("akos_drawCostumeChannel: invalid codec %d", ar->codec);
		}
	} else {
		extra = p[2];
		p+=3;

		for(i=0; i!=extra; i++) {
			code = p[4];
			if (code&0x80) code = ((code&0xF)<<8)|p[5];
			off = ar->akof + code;

			ar->srcptr = ar->akcd + READ_LE_UINT32(&off->akcd);
			akci = (AkosCI*)(ar->akci + READ_LE_UINT16(&off->akci));

			ar->move_x_cur = ar->move_x + (int16)READ_LE_UINT16(p+0);
			ar->move_y_cur = ar->move_y + (int16)READ_LE_UINT16(p+2);
			
			p += (p[4]&0x80) ? 6 : 5;

			ar->width = READ_LE_UINT16(&akci->width);
			ar->height = READ_LE_UINT16(&akci->height);

			switch(ar->codec) {
			case 1:
				akos_codec1(ar);
				break;
			case 5:
				akos_codec5(ar);
				break;
			case 16:
				akos_codec16(ar);
				break;
			default:
				error("akos_drawCostumeChannel: invalid codec %d", ar->codec);
			}
		}
	}

	return true;
}

void akos_c1_0y_decode(AkosRenderer *ar) {
	byte len,color;
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

	if (len) goto StartPos;

	do {
		len = *src++;
		color = len>>ar->v1.shl;
		len &= ar->v1.mask;
		if (!len) len = *src++;

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

void akos_generic_decode(AkosRenderer *ar) {
	byte *src,*dst;
	byte len,height, maskbit;
	uint y, color;	
	const byte *scaleytab, *mask;


	y = ar->v1.y;

	len = ar->v1.replen;
	src = ar->srcptr;
	dst = ar->v1.destptr;
	color = ar->v1.repcolor;
	height = ar->height;

	scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];
	maskbit = revBitMask[ar->v1.x&7];
	mask = ar->v1.mask_ptr + (ar->v1.x>>3);

	if (len) goto StartPos;

	do {
		len = *src++;
		color = len>>ar->v1.shl;
		len &= ar->v1.mask;
		if (!len) len = *src++;
		
		do {
			if (*scaleytab++ < ar->scale_y) {
				if (color && y < ar->outheight && (!ar->v1.mask_ptr || !((mask[0]|mask[ar->v1.imgbufoffs]) & maskbit)) ) {
					*dst = ar->palette[color];
				}
				mask += 40;
				dst += ar->outwidth;
				y++;
			}
			if (!--height) {
				if(!--ar->v1.skip_width)
					return;
				height = ar->height;
				y = ar->v1.y;

				scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];
								
				if (ar->v1.scaletable[ar->v1.tmp_x] < ar->scale_x) {
					ar->v1.x += ar->v1.scaleXstep;
					if ((uint)ar->v1.x >= 320)
						return;
					maskbit = revBitMask[ar->v1.x&7];
					ar->v1.destptr += ar->v1.scaleXstep;
				}
				mask = ar->v1.mask_ptr + (ar->v1.x>>3);
				ar->v1.tmp_x += ar->v1.scaleXstep;
				dst = ar->v1.destptr;
			}
StartPos:;
		} while (--len);
	} while(1);
}


void akos_c1_spec1(AkosRenderer *ar) {
	byte *src,*dst;
	byte len,height,pcolor, maskbit;
	uint y, color;	
	const byte *scaleytab, *mask;


	y = ar->v1.y;

	len = ar->v1.replen;
	src = ar->srcptr;
	dst = ar->v1.destptr;
	color = ar->v1.repcolor;
	height = ar->height;

	scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];
	maskbit = revBitMask[ar->v1.x&7];
	mask = ar->v1.mask_ptr + (ar->v1.x>>3);

	if (len) goto StartPos;

	do {
		len = *src++;
		color = len>>ar->v1.shl;
		len &= ar->v1.mask;
		if (!len) len = *src++;
		
		do {
			if (*scaleytab++ < ar->scale_y) {
				if (color && y < ar->outheight && (!ar->v1.mask_ptr || !((mask[0]|mask[ar->v1.imgbufoffs]) & maskbit)) ) {
					pcolor = ar->palette[color];
					if (pcolor==13)
						pcolor = ar->shadow_table[*dst];
					*dst = pcolor;
				}
				mask += 40;
				dst += ar->outwidth;
				y++;
			}
			if (!--height) {
				if(!--ar->v1.skip_width)
					return;
				height = ar->height;
				y = ar->v1.y;

				scaleytab = &ar->v1.scaletable[ar->v1.tmp_y];
								
				if (ar->v1.scaletable[ar->v1.tmp_x] < ar->scale_x) {
					ar->v1.x += ar->v1.scaleXstep;
					if ((uint)ar->v1.x >= 320)
						return;
					maskbit = revBitMask[ar->v1.x&7];
					ar->v1.destptr += ar->v1.scaleXstep;
				}
				mask = ar->v1.mask_ptr + (ar->v1.x>>3);
				ar->v1.tmp_x += ar->v1.scaleXstep;
				dst = ar->v1.destptr;
			}
StartPos:;
		} while (--len);
	} while(1);
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


void Scumm::akos_codec1(AkosRenderer *ar) {
	int num_colors;
	bool use_scaling;
	int i,j;
	int x,x_right,x_left,skip,tmp_x,tmp_y;
	int y,y_top,y_bottom;
	bool y_clipping;
	bool charsetmask;
	bool masking;
	int step;

	/* implement custom scale table */
	ar->v1.scaletable = default_scale_table;

	/* Setup color decoding variables */
	num_colors = getResourceDataSize(ar->akpl);
	if (num_colors == 32) {
		ar->v1.mask = (1<<3)-1;
		ar->v1.shl = 3;
	} else if (num_colors == 64) {
		ar->v1.mask = (1<<2)-1;
		ar->v1.shl = 2;
	} else {
		ar->v1.mask = (1<<4)-1;
		ar->v1.shl = 4;
	}

	use_scaling = (ar->scale_x!=0xFF) || (ar->scale_y!=0xFF);

	x = ar->x;
	y = ar->y;

	if (use_scaling) {
		
		/* Scale direction */
		ar->v1.scaleXstep = -1;
		if (ar->move_x_cur < 0) {
			ar->move_x_cur = -ar->move_x_cur;
			ar->v1.scaleXstep = 1;
		}

		if (ar->mirror) {
			/* Adjust X position */
			tmp_x = 0x180 - ar->move_x_cur;
			j = tmp_x;
			for(i=0; i<ar->move_x_cur; i++) {
				if (ar->v1.scaletable[j++] < ar->scale_x)
					x -= ar->v1.scaleXstep;
			}

			x_left = x_right = x;

			j = tmp_x;
			for(i=0,skip=0; i<ar->width; i++) {
				if (x_right < 0) {
					skip++;
					tmp_x = j;
				}
				if (ar->v1.scaletable[j++] < ar->scale_x)
					x_right++;
			}
		} else {
			/* No mirror */
			/* Adjust X position */
			tmp_x = 0x180 + ar->move_x_cur;
			j = tmp_x;
			for(i=0; i<ar->move_x_cur; i++) {
				if (ar->v1.scaletable[j++] < ar->scale_x)
					x += ar->v1.scaleXstep;
			}

			x_left = x_right = x;

			j = tmp_x;
			for(i=0,skip=0; i<ar->width; i++) {
				if (x_left >= (int)ar->outwidth) {
					tmp_x = j;
					skip++;
					
				}
				if (ar->v1.scaletable[j--] < ar->scale_x)
					x_left--;
			}
		}

		if (skip) skip--;

		step = -1;
		if (ar->move_y_cur < 0) {
			ar->move_y_cur = -ar->move_y_cur;
			step = -step;
		}

		tmp_y = 0x180 - ar->move_y_cur;
		for(i=0; i<ar->move_y_cur; i++) {
			if (ar->v1.scaletable[tmp_y++] < ar->scale_y)
				y -= step;
		}
		
		y_top = y_bottom = y;
		tmp_y = 0x180 - ar->move_y_cur;
		for(i=0; i<ar->height; i++) {
			if (ar->v1.scaletable[tmp_y++] < ar->scale_y)
				y_bottom++;
		}

		tmp_y = 0x180 - ar->move_y_cur;
	} else {
		if (!ar->mirror)
			ar->move_x_cur = -ar->move_x_cur;
		
		x += ar->move_x_cur;
		y += ar->move_y_cur;

		if (ar->mirror) {
			x_left = x;
			x_right = x + ar->width;
		} else {
			x_right = x;
			x_left = x - ar->width;
		}

		y_top = y;
		y_bottom = y + ar->height;

		tmp_x = 0x180;
		tmp_y = 0x180;
	}

	ar->v1.tmp_x = tmp_x;
	ar->v1.tmp_y = tmp_y;
	ar->v1.skip_width = ar->width;
	
	ar->v1.scaleXstep = -1;
	if (ar->mirror)
		ar->v1.scaleXstep = -ar->v1.scaleXstep;

	if ((uint)y_top >= (uint)ar->outheight || y_bottom <= 0)
		return;

	if ((int)x_left >= (int)ar->outwidth || x_right <= 0)
		return;

	ar->v1.replen = 0;
	ar->v1.y_pitch = ar->height * ar->outwidth;
	
	if (ar->mirror) {
		ar->v1.y_pitch--;
		if (!use_scaling)
			skip = -x;
		if (skip > 0) {
			ar->v1.skip_width -= skip;
			akos_codec1_ignorePakCols(ar, skip);
			x = 0;
		} else {
			skip = x_right - ar->outwidth;
			if (skip > 0)
				ar->v1.skip_width -= skip;
		}
	} else {
		ar->v1.y_pitch++;
		if (!use_scaling) {
			skip = x_right - ar->outwidth + 1;
		}
		if (skip > 0) {
			ar->v1.skip_width -= skip;
			akos_codec1_ignorePakCols(ar, skip);
			x = ar->outwidth - 1;
		} else {
			skip = -1 - x_left;
			if (skip > 0)
				ar->v1.skip_width -= skip;
		}
	}

	ar->v1.x = x;
	ar->v1.y = y;

	if (ar->v1.skip_width <= 0 || ar->height<=0)
		return;

	updateDirtyRect(0, x_left, x_right, y_top, y_bottom, 1<<ar->dirty_id);

	y_clipping = ((uint)y_bottom > ar->outheight || y_top < 0);

	if ( (uint)y_top > (uint)ar->outheight)
		y_top = 0;
	
	if ( (uint) y_bottom > (uint)ar->outheight)
		y_bottom = ar->outheight;

	if (y_top < ar->draw_top)
		ar->draw_top = y_top;
	if (y_bottom > ar->draw_bottom)
		ar->draw_bottom = y_bottom;

	if (x==-1) x=0; /* ?? */

	ar->v1.destptr = ar->outptr + x + y * ar->outwidth;

	charsetmask = ar->charsetmask;
	masking = false;
	if (ar->clipping) {
		masking = isMaskActiveAt(x_left, y_top, x_right, y_bottom,
			getResourceAddress(rtBuffer, 9) + gdi._imgBufOffs[ar->clipping] + _screenStartStrip
		) != 0;
	}
	
	ar->v1.mask_ptr = NULL;

	if (masking || charsetmask || ar->shadow_mode) {
		ar->v1.mask_ptr = getResourceAddress(rtBuffer, 9) + y*40 + _screenStartStrip;
		ar->v1.imgbufoffs = gdi._imgBufOffs[ar->clipping];
		if (!charsetmask && masking) {
			ar->v1.mask_ptr += ar->v1.imgbufoffs;
			ar->v1.imgbufoffs = 0;
		}
	}

	switch(ar->shadow_mode) {
	case 1:
		akos_c1_spec1(ar);
		return;
	case 2:
//		akos_c1_spec2(ar);
		return;
	case 3:
//		akos_c1_spec3(ar);
		return;
	}

	akos_generic_decode(ar);
	
	//	akos_c1_0y_decode(ar);

#if 0

	switch(((byte)y_clipping<<3) | ((byte)use_scaling<<2) | ((byte)masking<<1) | (byte)charsetmask) {
	case 0: akos_c1_0_decode(ar); break;
	case 0+8: akos_c1_0y_decode(ar); break;
	case 2:
	case 1: akos_c1_12_decode(ar); break;
	case 2+8:
	case 1+8: akos_c1_12y_decode(ar); break;
	case 3+8:
	case 3: akos_c1_3_decode(ar); break;
	case 4: akos_c1_4_decode(ar); break;
	case 4+8: akos_c1_4y_decode(ar); break;
	case 6:
	case 5: akos_c1_56_decode(ar); break;
	case 6+8:
	case 5+8: akos_c1_56y_decode(ar); break;
	case 7:
	case 7+8: akos_c1_7_decode(ar); break;
	}
#endif
}


void Scumm::akos_codec1_ignorePakCols(AkosRenderer *ar, int num) {
	int n;
	byte repcolor;
	byte replen;
	byte *src;

	n = ar->height;
	if (num>1) n *= num;
	src = ar->srcptr;
	do {
		repcolor = *src++;
		replen = repcolor & ar->v1.mask;
		if (replen==0) {
			replen = *src++;
		}
		do {
			if (!--n) {
				ar->v1.repcolor = repcolor>>ar->v1.shl;
				ar->v1.replen = replen;
				ar->srcptr = src;
				return;
			}
		} while (--replen);
	} while (1);
}


void Scumm::akos_codec5(AkosRenderer *ar) {
	warning("akos_codec5: not implemented");
}

void Scumm::akos_codec16(AkosRenderer *ar) {
	warning("akos_codec16: not implemented");
}


bool Scumm::akos_increaseAnims(byte *akos, Actor *a) {
	byte *aksq, *akfo;
	int i;
	uint size;
	bool result;

	aksq = findResourceData(MKID('AKSQ'), akos);
	akfo = findResourceData(MKID('AKFO'), akos);
	
	size = getResourceDataSize(akfo)>>1;
	
	result = false;
	for(i=0;i!=0x10;i++) {
		if (a->cost.active[i]!=0)
			result|=akos_increaseAnim(a, i, aksq, (uint16*)akfo, size);
	}
	return result;
}


#define GW(o) ((int16)READ_LE_UINT16(aksq+curpos+(o)))
#define GUW(o) READ_LE_UINT16(aksq+curpos+(o))
#define GB(o) aksq[curpos+(o)]

bool Scumm::akos_increaseAnim(Actor *a, int chan, byte *aksq, uint16 *akfo, int numakfo) {
	byte active;
	uint old_curpos, curpos,end;
	uint code;
	bool flag_value;
	int tmp,tmp2;

	active = a->cost.active[chan];
	end = a->cost.end[chan];
	old_curpos = curpos = a->cost.curpos[chan];
	flag_value = false;

	do {

		code = aksq[curpos];
		if (code & 0x80) code = (code<<8)|aksq[curpos+1];

		switch(active) {
		case 6:
			switch(code) {
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
				tmp = aksq[curpos-1];
				while (--tmp >= 0) {
					curpos += 4;
					curpos += (aksq[curpos]&0x80) ? 2 : 1;
				}
				break;
			default:
				if (code&0xC000)
					error("akos_increaseAnim: invalid code %x", code);
				curpos += (code&0x8000) ? 2 : 1;
			}
			break;
		case 2:
			curpos += (code&0x8000) ? 2 : 1;
			if (curpos > end)
				curpos = a->cost.start[chan];
			break;
		case 3:
			if (curpos != end)
				curpos += (code&0x8000) ? 2 : 1;
			break;
		}

		code = aksq[curpos];
		if (code & 0x80) code = (code<<8)|aksq[curpos+1];

		if (flag_value && code!=AKC_ClearFlag)
			continue;

		switch(code) {
		case AKC_StartAnimInActor:
			akos_queCommand(4, 
				derefActorSafe(getAnimVar(a, GB(2)), "akos_increaseAnim:29"),
				getAnimVar(a, GB(3)),
				0
			);
			continue;

		case AKC_Random:
			setAnimVar(a, GB(6), getRandomNumberRng(GW(2),GW(4)));
			continue;
		case AKC_SkipGE:
		case AKC_SkipG:
		case AKC_SkipLE:
		case AKC_SkipL:
		case AKC_SkipNE:
		case AKC_SkipE:
			if ( !akos_compare(getAnimVar(a, GB(4)),GW(2), code-AKC_SkipStart) )
				flag_value = true;
			continue;
		case AKC_IncVar:
			setAnimVar(a, 0, getAnimVar(a, 0)+1);
			continue;
		case AKC_SetVar:
			setAnimVar(a, GB(4), GW(2));
			continue;
		case AKC_AddVar:
			setAnimVar(a, GB(4), getAnimVar(a, GB(4)) + GW(2) );
			continue;
		case AKC_Flip:
			a->flip = GW(2) != 0;
			continue;
		case AKC_CmdQue3:
			tmp = GB(2);
			if ((uint)tmp < 8)
				akos_queCommand(3, a, a->sound[tmp], 0);
			continue;
		case AKC_CmdQue3Quick:
			akos_queCommand(3, a, a->sound[1], 0);
			continue;
		case AKC_StartAnim:
			akos_queCommand(4, a, GB(2), 0);
			continue;
		case AKC_StartVarAnim:
			akos_queCommand(4, a, getAnimVar(a, GB(2)), 0);
			continue;
		case AKC_SetVarInActor:
			setAnimVar(
				derefActorSafe(getAnimVar(a, GB(2)),"akos_increaseAnim:9"),
				GB(3), GW(4)
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
			if (tmp2<1 || tmp2>3)
				error("akos_increaseAnim:8 invalid code %d", tmp2);
			akos_queCommand(tmp2+6, a, a->sound[tmp], GB(6));
			continue;
		case AKC_SetDrawOffs:
			akos_queCommand(6, a, GW(2), GW(4));
			continue;
		case AKC_JumpTable:
			if (akfo==NULL)
				error("akos_increaseAnim: no AKFO table");
			tmp = getAnimVar(a, GB(2)) - 1;
			if (tmp<0 || tmp >= numakfo-1)
				error("akos_increaseAnim: invalid jump value %d",tmp);
			curpos = READ_LE_UINT16(&akfo[tmp]);
			break;
		case AKC_JumpIfSet:
			if (!getAnimVar(a, GB(4)))
				continue;
			setAnimVar(a, GB(4), 0);
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
			if ( !akos_compare(getAnimVar(a, GB(4)), GW(2), code - AKC_JumpStart) )
				continue;
			curpos = GUW(2);
			break;

		default:
			if ((code&0xC000)==0xC000)
				error("Undefined uSweat token %X", code);
		}
		break;
	} while(1);

	int code2 = aksq[curpos];
	if (code2 & 0x80) code2 = (code2<<8)|aksq[curpos+1];
	assert((code2&0xC000)!=0xC000 || code2==AKC_ComplexChan || code2==AKC_Return);

	a->cost.curpos[chan] = curpos;

	return curpos != old_curpos;
}

void Scumm::akos_queCommand(byte cmd, Actor *a, int param_1, int param_2) {
//	warning("akos_queCommand(%d,%d,%d,%d)", cmd, a->number, param_1, param_2);
}


bool Scumm::akos_compare(int a, int b, byte cmd) {
	switch(cmd) {
	case 0: return a==b;
	case 1: return a!=b;
	case 2: return a<b;
	case 3: return a<=b;
	case 4: return a>b;
	default: return a>=b;
	}
}

int Scumm::getAnimVar(Actor *a, byte var) {
	return a->animVariable[var];
}

void Scumm::setAnimVar(Actor *a, byte var, int value) {
	a->animVariable[var] = value;
}
