/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project 
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

#include "base-costume.h"

struct CostumeData;
struct AkosHeader;
struct AkosOffset;

class AkosRenderer : public BaseCostumeRenderer {
protected:
	uint16 codec;

	// actor palette
	byte palette[256];

	// pointer to various parts of the costume resource
	const byte *akos;
	const AkosHeader *akhd;

	const byte *akpl, *akci, *aksq;
	const AkosOffset *akof;
	const byte *akcd;

	struct {
		byte unk5;
		int unk6;
		byte mask;
		byte color;
		byte shift;
		uint16 bits;
		byte numbits;
		const byte *dataptr;
		byte buffer[336];
	} akos16;

public:
	AkosRenderer(Scumm *scumm) : BaseCostumeRenderer(scumm) {
		akos = 0;
		akhd = 0;
		akpl = 0;
		akci = 0;
		aksq = 0;
		akof = 0;
		akcd = 0;
		_actorHitMode = false;
	}

	bool _actorHitMode;
	int16 _actorHitX, _actorHitY;
	bool _actorHitResult;

	void setPalette(byte *palette);
	void setFacing(Actor *a);
	void setCostume(int costume);

protected:
	byte drawLimb(const CostumeData &cost, int limb);

	byte codec1(int xmoveCur, int ymoveCur);
	void codec1_genericDecode();

	byte codec5(int xmoveCur, int ymoveCur);

	byte codec16(int xmoveCur, int ymoveCur);
	void akos16SetupBitReader(const byte *src);
	void akos16SkipData(int32 numskip);
	void akos16DecodeLine(byte *buf, int32 numbytes, int32 dir);
	void akos16Decompress(byte *dest, int32 pitch, const byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency);
	void akos16DecompressMask(byte *dest, int32 pitch, const byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency, byte *maskptr, int32 bitpos_start);
};

#endif
