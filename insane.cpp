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

#if !defined(__APPLE__CW)
#define NEED_SDL_HEADERS
#endif


#include "stdafx.h"
#include "scumm.h"

#define SWAP2(a) ((((a)>>24)&0xFF) | (((a)>>8)&0xFF00) | (((a)<<8)&0xFF0000) | (((a)<<24)&0xFF000000))

void invalidblock(uint32 tag) {
	error("Encountered invalid block %c%c%c%c", tag>>24, tag>>16, tag>>8, tag);
}

uint32 SmushPlayer::nextBE32() {
	uint32 a = *((uint32*)_cur);
	_cur += sizeof(uint32);

	return SWAP2(a);
}

void SmushPlayer::fileRead(void *mem, int len) {
	if (fread(mem, len,1, _in) != 1)
		error("EOF while reading");

}

uint32 SmushPlayer::fileReadBE32() {
	uint32 number;

	fileRead(&number, sizeof(number));
	return SWAP2(number);
}

uint32 SmushPlayer::fileReadLE32() {
	uint32 number;

	fileRead(&number, sizeof(number));
	return number;
}

void SmushPlayer::openFile(byte* fileName) {
	byte buf[100];

	sprintf((char*)buf,"%sVIDEO/%s",(char*)sm->_gameDataPath,(char*)fileName);
	_in = fopen((char*)buf, "rb");

	if(_in==NULL){
        	sprintf((char*)buf,"%svideo/%s",(char*)sm->_gameDataPath,(char*)fileName);
		_in = fopen((char*)buf, "rb");
		
	}
}

void SmushPlayer::nextBlock() {
	_blockTag = fileReadBE32();
	_blockSize = fileReadBE32();

	if (_block != NULL)
		free(_block);

	_block = (byte*)malloc(_blockSize);

	if (_block==NULL)
		error("cannot allocate memory");

	fileRead(_block, _blockSize);
}

bool SmushPlayer::parseTag() {
	switch(nextBlock(), _blockTag) {

	case 'AHDR':
		parseAHDR();
		break;

	case 'FRME':
		parseFRME();
		break;
	
	default:
		invalidblock(_blockTag);
	}

	return true;
}



void SmushPlayer::parseAHDR() {
	memcpy(_fluPalette, _block, 0x300);
	_paletteChanged = true;

	printf("parse AHDR\n");
}



void SmushPlayer::parseNPAL() {
	memcpy(_fluPalette, _cur, 0x300);

	_paletteChanged = true;
}



void codec1(CodecData *cd) {
	uint y = cd->y;
	byte *src = cd->src;
	byte *dest= cd->out;
	uint h = cd->h;

	if (!h || !cd->w)
		return;

	dest += cd->y * cd->pitch;

	do {
		byte color;
		uint len, num;
		uint x;

		if ((uint)y >= (uint)cd->outheight) {
			src += *(uint16*)(src) + 2;
			continue;
		}

		len = cd->w;
		x = cd->x;
		src += 2;

		do {
			byte code = *src++;

			num = (code>>1)+1;
			if (num>len) num=len;
			len -= num;

			if (code&1) {
				color = *src++;
//				if ((color = *src++)!=0) {
					do {
						if ((uint)x < (uint)cd->outwidth)
							dest[x] = color;
					} while (++x,--num);
//				} else {
//					x += num;
//				}
			} else {
				do {
					color = *src++;
					if (/*(color=*src++) != 0 &&*/ (uint)x < (uint)cd->outwidth)
						dest[x] = color;
				} while (++x,--num);
			}
		} while (len);
	} while (dest += cd->pitch,y++,--h);
}



void codec37_bompdepack(byte *dst, byte *src, int len) {
	byte code;
	byte color;
	int num;

	do {
		code = *src++;
		if (code & 1) {
			num = (code>>1) + 1;
			color = *src++;
			memset(dst, color, num);
			dst += num;
		} else {
			num = (code>>1) + 1;
			memcpy(dst,src,num);
			dst += num;
			src += num;
		}
	} while (len -= num);
}

void codec37_proc5(byte *dst, byte *src, int next_offs, int bw, int bh, int pitch, int16 *table) {
	byte code, *tmp;
	int i;

	if (pitch != 320) {
		warning("invalid pitch");
		return;
	}

	do {
	i = bw;
		do {
			code = *src++;
			if (code==0xFF) {
				*(uint32*)(dst+0) = ((uint32*)src)[0];
				*(uint32*)(dst+320) = ((uint32*)src)[1];
				*(uint32*)(dst+320*2) = ((uint32*)src)[2];
				*(uint32*)(dst+320*3) = ((uint32*)src)[3];
				src += 16;
				dst += 4;
			} else {
				tmp = dst + table[code] + next_offs;
				*(uint32*)(dst+0) = *(uint32*)(tmp);
				*(uint32*)(dst+320) = *(uint32*)(tmp+320);
				*(uint32*)(dst+320*2) = *(uint32*)(tmp+320*2);
				*(uint32*)(dst+320*3) = *(uint32*)(tmp+320*3);
				dst += 4;
			}
		} while(--i);
		dst += 320*4 - 320;
	} while (--bh);
}	



static const int8 maketable_bytes[] = {
0, 0, 1, 0, 2, 0, 3, 0, 5, 0, 8, 0, 13, 0, 21, 0,
-1, 0, -2, 0, -3, 0, -5, 0, -8, 0, -13, 0, -17, 0, -21, 0,
0, 1, 1, 1, 2, 1, 3, 1, 5, 1, 8, 1, 13, 1, 21, 1,
-1, 1, -2, 1, -3, 1, -5, 1, -8, 1, -13, 1, -17, 1, -21, 1,
0, 2, 1, 2, 2, 2, 3, 2, 5, 2, 8, 2, 13, 2, 21, 2,
-1, 2, -2, 2, -3, 2, -5, 2, -8, 2, -13, 2, -17, 2, -21, 2,
0, 3, 1, 3, 2, 3, 3, 3, 5, 3, 8, 3, 13, 3, 21, 3,
-1, 3, -2, 3, -3, 3, -5, 3, -8, 3, -13, 3, -17, 3, -21, 3,
0, 5, 1, 5, 2, 5, 3, 5, 5, 5, 8, 5, 13, 5, 21, 5,
-1, 5, -2, 5, -3, 5, -5, 5, -8, 5, -13, 5, -17, 5, -21, 5,
0, 8, 1, 8, 2, 8, 3, 8, 5, 8, 8, 8, 13, 8, 21, 8,
-1, 8, -2, 8, -3, 8, -5, 8, -8, 8, -13, 8, -17, 8, -21, 8,
0, 13, 1, 13, 2, 13, 3, 13, 5, 13, 8, 13, 13, 13, 21, 13,
-1, 13, -2, 13, -3, 13, -5, 13, -8, 13, -13, 13, -17, 13, -21, 13,
0, 21, 1, 21, 2, 21, 3, 21, 5, 21, 8, 21, 13, 21, 21, 21,
-1, 21, -2, 21, -3, 21, -5, 21, -8, 21, -13, 21, -17, 21, -21, 21,
0, -1, 1, -1, 2, -1, 3, -1, 5, -1, 8, -1, 13, -1, 21, -1,
-1, -1, -2, -1, -3, -1, -5, -1, -8, -1, -13, -1, -17, -1, -21, -1,
0, -2, 1, -2, 2, -2, 3, -2, 5, -2, 8, -2, 13, -2, 21, -2,
-1, -2, -2, -2, -3, -2, -5, -2, -8, -2, -13, -2, -17, -2, -21, -2,
0, -3, 1, -3, 2, -3, 3, -3, 5, -3, 8, -3, 13, -3, 21, -3,
-1, -3, -2, -3, -3, -3, -5, -3, -8, -3, -13, -3, -17, -3, -21, -3,
0, -5, 1, -5, 2, -5, 3, -5, 5, -5, 8, -5, 13, -5, 21, -5,
-1, -5, -2, -5, -3, -5, -5, -5, -8, -5, -13, -5, -17, -5, -21, -5,
0, -8, 1, -8, 2, -8, 3, -8, 5, -8, 8, -8, 13, -8, 21, -8,
-1, -8, -2, -8, -3, -8, -5, -8, -8, -8, -13, -8, -17, -8, -21, -8,
0, -13, 1, -13, 2, -13, 3, -13, 5, -13, 8, -13, 13, -13, 21, -13,
-1, -13, -2, -13, -3, -13, -5, -13, -8, -13, -13, -13, -17, -13, -21, -13,
0, -17, 1, -17, 2, -17, 3, -17, 5, -17, 8, -17, 13, -17, 21, -17,
-1, -17, -2, -17, -3, -17, -5, -17, -8, -17, -13, -17, -17, -17, -21, -17,
0, -21, 1, -21, 2, -21, 3, -21, 5, -21, 8, -21, 13, -21, 21, -21,
-1, -21, -2, -21, -3, -21, -5, -21, -8, -21, -13, -21, -17, -21, 0, 0,
-8, -29, 8, -29, -18, -25, 17, -25, 0, -23, -6, -22, 6, -22, -13, -19,
12, -19, 0, -18, 25, -18, -25, -17, -5, -17, 5, -17, -10, -15, 10, -15,
0, -14, -4, -13, 4, -13, 19, -13, -19, -12, -8, -11, -2, -11, 0, -11,
2, -11, 8, -11, -15, -10, -4, -10, 4, -10, 15, -10, -6, -9, -1, -9,
1, -9, 6, -9, -29, -8, -11, -8, -8, -8, -3, -8, 3, -8, 8, -8,
11, -8, 29, -8, -5, -7, -2, -7, 0, -7, 2, -7, 5, -7, -22, -6,
-9, -6, -6, -6, -3, -6, -1, -6, 1, -6, 3, -6, 6, -6, 9, -6,
22, -6, -17, -5, -7, -5, -4, -5, -2, -5, 0, -5, 2, -5, 4, -5,
7, -5, 17, -5, -13, -4, -10, -4, -5, -4, -3, -4, -1, -4, 0, -4,
1, -4, 3, -4, 5, -4, 10, -4, 13, -4, -8, -3, -6, -3, -4, -3,
-3, -3, -2, -3, -1, -3, 0, -3, 1, -3, 2, -3, 4, -3, 6, -3,
8, -3, -11, -2, -7, -2, -5, -2, -3, -2, -2, -2, -1, -2, 0, -2,
1, -2, 2, -2, 3, -2, 5, -2, 7, -2, 11, -2, -9, -1, -6, -1,
-4, -1, -3, -1, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, 3, -1,
4, -1, 6, -1, 9, -1, -31, 0, -23, 0, -18, 0, -14, 0, -11, 0,
-7, 0, -5, 0, -4, 0, -3, 0, -2, 0, -1, 0, 0, -31, 1, 0,
2, 0, 3, 0, 4, 0, 5, 0, 7, 0, 11, 0, 14, 0, 18, 0,
23, 0, 31, 0, -9, 1, -6, 1, -4, 1, -3, 1, -2, 1, -1, 1,
0, 1, 1, 1, 2, 1, 3, 1, 4, 1, 6, 1, 9, 1, -11, 2,
-7, 2, -5, 2, -3, 2, -2, 2, -1, 2, 0, 2, 1, 2, 2, 2,
3, 2, 5, 2, 7, 2, 11, 2, -8, 3, -6, 3, -4, 3, -2, 3,
-1, 3, 0, 3, 1, 3, 2, 3, 3, 3, 4, 3, 6, 3, 8, 3,
-13, 4, -10, 4, -5, 4, -3, 4, -1, 4, 0, 4, 1, 4, 3, 4,
5, 4, 10, 4, 13, 4, -17, 5, -7, 5, -4, 5, -2, 5, 0, 5,
2, 5, 4, 5, 7, 5, 17, 5, -22, 6, -9, 6, -6, 6, -3, 6,
-1, 6, 1, 6, 3, 6, 6, 6, 9, 6, 22, 6, -5, 7, -2, 7,
0, 7, 2, 7, 5, 7, -29, 8, -11, 8, -8, 8, -3, 8, 3, 8,
8, 8, 11, 8, 29, 8, -6, 9, -1, 9, 1, 9, 6, 9, -15, 10,
-4, 10, 4, 10, 15, 10, -8, 11, -2, 11, 0, 11, 2, 11, 8, 11,
19, 12, -19, 13, -4, 13, 4, 13, 0, 14, -10, 15, 10, 15, -5, 17,
5, 17, 25, 17, -25, 18, 0, 18, -12, 19, 13, 19, -6, 22, 6, 22,
0, 23, -17, 25, 18, 25, -8, 29, 8, 29, 0, 31, 0, 0, -6, -22,
6, -22, -13, -19, 12, -19, 0, -18, -5, -17, 5, -17, -10, -15, 10, -15,
0, -14, -4, -13, 4, -13, 19, -13, -19, -12, -8, -11, -2, -11, 0, -11,
2, -11, 8, -11, -15, -10, -4, -10, 4, -10, 15, -10, -6, -9, -1, -9,
1, -9, 6, -9, -11, -8, -8, -8, -3, -8, 0, -8, 3, -8, 8, -8,
11, -8, -5, -7, -2, -7, 0, -7, 2, -7, 5, -7, -22, -6, -9, -6,
-6, -6, -3, -6, -1, -6, 1, -6, 3, -6, 6, -6, 9, -6, 22, -6,
-17, -5, -7, -5, -4, -5, -2, -5, -1, -5, 0, -5, 1, -5, 2, -5,
4, -5, 7, -5, 17, -5, -13, -4, -10, -4, -5, -4, -3, -4, -2, -4,
-1, -4, 0, -4, 1, -4, 2, -4, 3, -4, 5, -4, 10, -4, 13, -4,
-8, -3, -6, -3, -4, -3, -3, -3, -2, -3, -1, -3, 0, -3, 1, -3,
2, -3, 3, -3, 4, -3, 6, -3, 8, -3, -11, -2, -7, -2, -5, -2,
-4, -2, -3, -2, -2, -2, -1, -2, 0, -2, 1, -2, 2, -2, 3, -2,
4, -2, 5, -2, 7, -2, 11, -2, -9, -1, -6, -1, -5, -1, -4, -1,
-3, -1, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4, -1,
5, -1, 6, -1, 9, -1, -23, 0, -18, 0, -14, 0, -11, 0, -7, 0,
-5, 0, -4, 0, -3, 0, -2, 0, -1, 0, 0, -23, 1, 0, 2, 0,
3, 0, 4, 0, 5, 0, 7, 0, 11, 0, 14, 0, 18, 0, 23, 0,
-9, 1, -6, 1, -5, 1, -4, 1, -3, 1, -2, 1, -1, 1, 0, 1,
1, 1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 9, 1, -11, 2,
-7, 2, -5, 2, -4, 2, -3, 2, -2, 2, -1, 2, 0, 2, 1, 2,
2, 2, 3, 2, 4, 2, 5, 2, 7, 2, 11, 2, -8, 3, -6, 3,
-4, 3, -3, 3, -2, 3, -1, 3, 0, 3, 1, 3, 2, 3, 3, 3,
4, 3, 6, 3, 8, 3, -13, 4, -10, 4, -5, 4, -3, 4, -2, 4,
-1, 4, 0, 4, 1, 4, 2, 4, 3, 4, 5, 4, 10, 4, 13, 4,
-17, 5, -7, 5, -4, 5, -2, 5, -1, 5, 0, 5, 1, 5, 2, 5,
4, 5, 7, 5, 17, 5, -22, 6, -9, 6, -6, 6, -3, 6, -1, 6,
1, 6, 3, 6, 6, 6, 9, 6, 22, 6, -5, 7, -2, 7, 0, 7,
2, 7, 5, 7, -11, 8, -8, 8, -3, 8, 0, 8, 3, 8, 8, 8,
11, 8, -6, 9, -1, 9, 1, 9, 6, 9, -15, 10, -4, 10, 4, 10,
15, 10, -8, 11, -2, 11, 0, 11, 2, 11, 8, 11, 19, 12, -19, 13,
-4, 13, 4, 13, 0, 14, -10, 15, 10, 15, -5, 17, 5, 17, 0, 18,
-12, 19, 13, 19, -6, 22, 6, 22, 0, 23,
};

void codec37_maketable(PersistentCodecData37 *pcd, int pitch, byte idx) {
	int i,j;

	if (pcd->table_last_pitch==pitch && pcd->table_last_flags==idx)
		return;

	pcd->table_last_pitch = pitch;
	pcd->table_last_flags = idx;

	assert(idx*255 + 254 < (int)(sizeof(maketable_bytes)/2));

	for(i=0; i<255; i++) {
		j = i + idx*255;
		pcd->table1[i] = maketable_bytes[j*2+1] * pitch + maketable_bytes[j*2];
	}
}



void codec37(CodecData *cd, PersistentCodecData37 *pcd) {
	int width_in_blocks, height_in_blocks;
	int src_pitch;
	byte *curbuf;
	uint size;
	bool result = false;

	width_in_blocks = (cd->w + 3) >> 2;
	height_in_blocks = (cd->h + 3) >> 2;
	src_pitch = width_in_blocks * 4;

	codec37_maketable(pcd, src_pitch, cd->src[1]);

	switch(cd->src[0]) {
	case 0: {
		curbuf = pcd->deltaBufs[pcd->curtable];
		memset(pcd->deltaBuf, 0, curbuf - pcd->deltaBuf);
		size = *(uint32*)(cd->src + 4);
		memset(curbuf + size, 0, pcd->deltaBuf + pcd->deltaSize - curbuf - size);
		memcpy(curbuf, cd->src + 16, size);
		break;
	}

	case 2: {
		size = *(uint32*)(cd->src + 4);
		curbuf = pcd->deltaBufs[pcd->curtable];
		if(size==64000)
			codec37_bompdepack(curbuf, cd->src+16, size);
		else
			return;

		memset(pcd->deltaBuf, 0, curbuf - pcd->deltaBuf);
		memset(curbuf + size, 0, pcd->deltaBuf + pcd->deltaSize - curbuf - size);
		break;
	}

	case 3: {
		uint16 number = *(uint16*)(cd->src + 2);

		if ( number && pcd->flags+1 != number)
			break;

		if (number&1 && cd->src[12]&1 && cd->flags&0x10) {
			result=true;
			break;
		}

		if ((number&1) || !(cd->src[12]&1)) {
			pcd->curtable ^= 1;			
		}

		codec37_proc5(pcd->deltaBufs[pcd->curtable], cd->src+16, 
			pcd->deltaBufs[pcd->curtable^1] - pcd->deltaBufs[pcd->curtable],
			width_in_blocks, height_in_blocks, src_pitch,
			pcd->table1);
		break;
	}

	case 1:
	case 4:
		warning("code %d", cd->src[0]);
		return;

	default:
		error("codec37 default case");
	}

	pcd->flags = *(uint16*)(cd->src + 2);

	if (result) {
		pcd->curtable ^= 1;
	} else {
		memcpy(cd->out, pcd->deltaBufs[pcd->curtable], 320*200);
	}
}

void codec37_init(PersistentCodecData37 *pcd, int width, int height) {
	pcd->width = width;
	pcd->height = height;
	pcd->deltaSize = width*height*2+0x3E00+0xBA00;
	pcd->deltaBuf = (byte*)calloc(pcd->deltaSize, 1);
	pcd->deltaBufs[0] = pcd->deltaBuf + 0x3E00;
	pcd->deltaBufs[1] = pcd->deltaBuf + width * height + 0xBA00;
	pcd->curtable = 0;
	pcd->table1 = (int16*)calloc(255,sizeof(uint16));
}

void SmushPlayer::parseFOBJ() {
	byte codec;
	CodecData cd;

	cd.out = _renderBitmap;
	cd.pitch = cd.outwidth = 320;
	cd.outheight = 200;
	cd.y = 0;
	cd.x = 0;
	cd.src = _cur + 0xE;
	cd.w = *(uint16*)(_cur + 6);
	cd.h = *(uint16*)(_cur + 8);
	
	codec = _cur[0];

	switch(codec) {
	case 1:
		codec1(&cd);
		break;
	case 37:
		codec37(&cd, &pcd37);
		break;
	default:
		error("invalid codec %d", codec);
	}
}

void SmushPlayer::parsePSAD() {
	//printf("parse PSAD\n");
}

void SmushPlayer::parseTRES() {
//	printf("parse TRES\n");
}

void SmushPlayer::parseXPAL() {
	int num;
	int i;

	num = *(uint16*)(_cur + 2);
	if (num==0 || num==0x200) {
		if (num==0x200)
			memcpy(_fluPalette, _cur + 0x604, 0x300);

		for(i=0; i<0x300; i++) {
			_fluPalMul129[i] = _fluPalette[i] * 129;
			_fluPalWords[i] = *(uint16*)(_cur + 4 + i*2);
		}
		return;
	}

	for(i=0; i<0x300; i++) {
		_fluPalMul129[i] += _fluPalWords[i];
		_fluPalette[i] = _fluPalMul129[i]>>7;
	}

	_paletteChanged = true;
}

void SmushPlayer::parseFRME() {
	_cur = _block;
	
	do {
		_frmeTag = nextBE32();
		_frmeSize = nextBE32();

		switch(_frmeTag) {
		case 'NPAL':
			parseNPAL();
			break;
		case 'FOBJ':
			parseFOBJ();
			break;
		case 'PSAD':
			parsePSAD();
			break;
		case 'TRES':
			parseTRES();
			break;
		case 'XPAL':
			parseXPAL();
			break;
		case 'IACT':
			parseTRES();
			break;
		case 'STOR':
		case 'FTCH':
			break;

		default:
			invalidblock(_frmeTag);
		}

		_cur += (_frmeSize + 1) & ~1;
	} while (_cur + 4 < _block + _blockSize);
}

void SmushPlayer::init() {
	_renderBitmap = sm->_videoBuffer;
	codec37_init(&pcd37, 320, 200);
}

void SmushPlayer::go() {
	while (parseTag()) {}
}

void SmushPlayer::setPalette() {
	int i;

	for(i=0;i<768;i++)
		sm->_currentPalette[i]=_fluPalette[i];
}

void SmushPlayer::startVideo(short int arg, byte* videoFile)
{
	int frameIndex=0;

	_in=NULL;
	_paletteChanged=0;
	_block=NULL;
	_blockTag=0;
	_blockSize=0;
	_cur=NULL;
	_renderBitmap=NULL;
	_frameSize=0;
	_frmeTag=0;
	_frmeSize=0;
	_deltaBuf=NULL;
	_deltaBufSize=0;

	pcd37.deltaBuf=NULL;
	pcd37.deltaBufs[0]=NULL;
	pcd37.deltaBufs[1]=NULL;
	pcd37.deltaSize=0;
	pcd37.width=0;
	pcd37.height=0;
	pcd37.curtable=0;
	pcd37.unk2=0;
	pcd37.unk3=0;
	pcd37.flags=0;
	pcd37.table1=NULL;
	pcd37.table_last_pitch=0;
	pcd37.table_last_flags=0;

	init();
	openFile(videoFile);

	if(_in==NULL)
		return;

	if (fileReadBE32() != 'ANIM')
		error("file is not an anim");

	fileSize=fileReadBE32();

	sm->videoFinished = 0;
	sm->_insaneState = 1;

	do {
		if(ftell(_in)>=fileSize )
			return;
#ifdef INSANE_DEBUG
		warning("Playing frame %d",frameIndex);
#endif
		
		parseTag();
		frameIndex++;

		if (_paletteChanged) {
			_paletteChanged = false;
			setPalette();
			sm->setDirtyColors(0, 255);
		}

		blitToScreen(sm,sm->_videoBuffer, 0, 0, 320 ,200);
		updateScreen(sm);

		sm->delta = sm->_system->waitTick(sm->delta);
	
		sm->processKbd();
		
	} while (!sm->videoFinished);

	sm->_insaneState = 0;
}



