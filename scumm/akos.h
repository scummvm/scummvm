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

#ifndef AKOS_H
#define AKOS_H


#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct CostumeData;

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

struct AkosCI {
	uint16 width, height;
	int16 rel_x, rel_y;
	int16 move_x, move_y;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif

struct AkosRenderer {
public:
	byte _dirty_id;

	byte shadow_mode;
	byte *shadow_table;

	int _x, _y;
	byte _scaleX, _scaleY;
	byte clipping;
	bool charsetmask;	// FIXME - it seems charsetmask is only set once, in actor.cpp, to true. So can we get rid of it?!?

	int draw_top, draw_bottom;

	byte *outptr;
	uint outwidth, outheight;

protected:
	Scumm *_vm;
	int32 _numStrips;

	uint16 codec;
	bool mirror;									/* draw actor mirrored */

	/* pointer to various parts of the costume resource */
	byte *akos;
	AkosHeader *akhd;

	/* current move offset */
	int move_x, move_y;
	/* movement of cel to decode */
	int move_x_cur, move_y_cur;
	/* width and height of cel to decode */
	int _width, _height;

	byte *srcptr;

	byte *akpl, *akci, *aksq;
	AkosOffset *akof;
	byte *akcd;

	byte palette[256];

	struct {
		/* codec stuff */
		const byte *scaletable;
		byte mask, shl;
		bool doContinue;
		byte repcolor;
		byte replen;
		int scaleXstep;
		int x, y;
		int tmp_x, tmp_y;
		int y_pitch;
		int skip_width;
		byte *destptr;
		byte *mask_ptr;
		int imgbufoffs;
	} v1;

	struct {
		byte unk5;
		int unk6;
		byte mask;
		byte color;
		byte shift;
		uint16 bits;
		byte numbits;
		byte *dataptr;
		byte buffer[336];
	} akos16;

public:
	// Constructor, sets all data to 0
	AkosRenderer(Scumm *scumm) {
		memset(this, 0, sizeof(AkosRenderer));
		_vm = scumm;
		_numStrips = _vm->gdi._numStrips;
	}

	void setPalette(byte *palette);
	void setFacing(Actor *a);
	void setCostume(int costume);

	bool drawCostume(const CostumeData &cost);

protected:
	bool drawLimb(const CostumeData &cost, int limb);

	void codec1();
	void codec1_spec1();
	void codec1_spec2();
	void codec1_spec3();
	void codec1_genericDecode();
	void codec1_ignorePakCols(int num);

	void codec5();

	void codec16();
	void akos16SetupBitReader(byte *src);
	void akos16PutOnScreen(byte *dest, byte *src, byte transparency, int32 count);
	void akos16SkipData(int32 numskip);
	void akos16DecodeLine(byte *buf, int32 numbytes, int32 dir);
	void akos16ApplyMask(byte *dest, byte *maskptr, byte bits, int32 count, byte fillwith);
	void akos16Decompress(byte *dest, int32 pitch, byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency);
	void akos16DecompressMask(byte *dest, int32 pitch, byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency, byte *maskptr, int32 bitpos_start);
};

#endif
