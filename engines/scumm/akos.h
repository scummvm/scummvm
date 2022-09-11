/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCUMM_AKOS_H
#define SCUMM_AKOS_H

#include "scumm/base-costume.h"

namespace Scumm {

struct CostumeData;
struct AkosHeader;
struct AkosOffset;

class AkosCostumeLoader : public BaseCostumeLoader {
protected:
	const byte *_akos;

public:
	AkosCostumeLoader(ScummEngine *vm) : BaseCostumeLoader(vm) {}

	void loadCostume(int id) override;
	byte increaseAnims(Actor *a) override;
	void costumeDecodeData(Actor *a, int frame, uint usemask) override;

	//void animateLimb(int limb, int f);
	bool hasManyDirections(int id) {
		loadCostume(id);
		return hasManyDirections();
	}

protected:
	bool hasManyDirections();
};

class AkosRenderer : public BaseCostumeRenderer {
protected:
	uint16 _codec;

	// actor _palette
	uint16 _palette[256];
	bool _useBompPalette;

	// pointer to various parts of the costume resource
	const AkosHeader *akhd;	// header

	const byte *akpl;		// palette data
	const byte *akci;		// CostumeInfo table
	const byte *aksq;		// command sequence
	const AkosOffset *akof;	// offsets into ci and cd table
	const byte *akcd;		// costume data (contains the data for the codecs)

	const byte *akct;		// HE specific: condition table
	const byte *rgbs;		// HE specific: RGB table
	const uint8 *xmap;		// HE specific: shadow color table

	struct {
		bool repeatMode;
		int repeatCount;
		byte mask;
		byte color;
		byte shift;
		uint16 bits;
		byte numbits;
		const byte *dataptr;
		byte buffer[336];
	} _akos16;

public:
	AkosRenderer(ScummEngine *scumm) : BaseCostumeRenderer(scumm) {
		_useBompPalette = false;
		akhd = 0;
		akpl = 0;
		akci = 0;
		aksq = 0;
		akof = 0;
		akcd = 0;
		akct = 0;
		rgbs = 0;
		xmap = 0;
		_actorHitMode = false;
	}

	bool _actorHitMode;
	int16 _actorHitX, _actorHitY;
	bool _actorHitResult;

	void setPalette(uint16 *_palette) override;
	void setFacing(const Actor *a) override;
	void setCostume(int costume, int shadow) override;

protected:
	byte drawLimb(const Actor *a, int limb) override;

	byte codec1(int xmoveCur, int ymoveCur);
	void codec1_genericDecode(Codec1 &v1);
	byte codec5(int xmoveCur, int ymoveCur);
	byte codec16(int xmoveCur, int ymoveCur);
	byte codec32(int xmoveCur, int ymoveCur);
	void akos16SetupBitReader(const byte *src);
	void akos16SkipData(int32 numskip);
	void akos16DecodeLine(byte *buf, int32 numbytes, int32 dir);
	void akos16Decompress(byte *dest, int32 pitch, const byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency, int maskLeft, int maskTop, int zBuf);

	void markRectAsDirty(Common::Rect rect);
};

enum AkosOpcodes {
	AKC_Return = 0xC001,
	AKC_SetVar = 0xC010,
	AKC_CmdQue3 = 0xC015,
	AKC_C016 = 0xC016,
	AKC_C017 = 0xC017,
	AKC_C018 = 0xC018,
	AKC_C019 = 0xC019,
	AKC_ComplexChan = 0xC020,
	AKC_C021 = 0xC021,
	AKC_C022 = 0xC022,
	AKC_ComplexChan2 = 0xC025,
	AKC_Jump = 0xC030,
	AKC_JumpIfSet = 0xC031,
	AKC_AddVar = 0xC040,
	AKC_C042 = 0xC042,
	AKC_C044 = 0xC044,
	AKC_C045 = 0xC045,
	AKC_C046 = 0xC046,
	AKC_C047 = 0xC047,
	AKC_C048 = 0xC048,
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
	AKC_C08E = 0xC08E,
	AKC_SkipStart = 0xC090,
	AKC_SkipE = 0xC090,
	AKC_SkipNE = 0xC091,
	AKC_SkipL = 0xC092,
	AKC_SkipLE = 0xC093,
	AKC_SkipG = 0xC094,
	AKC_SkipGE = 0xC095,
	AKC_ClearFlag = 0xC09F,
	AKC_C0A0 = 0xC0A0,
	AKC_C0A1 = 0xC0A1,
	AKC_C0A2 = 0xC0A2,
	AKC_C0A3 = 0xC0A3,
	AKC_C0A4 = 0xC0A4,
	AKC_C0A5 = 0xC0A5,
	AKC_C0A6 = 0xC0A6,
	AKC_C0A7 = 0xC0A7,
	AKC_EndSeq = 0xC0FF
};

#define GW(o) ((int16)READ_LE_UINT16(aksq+curpos+(o)))
#define GUW(o) READ_LE_UINT16(aksq+curpos+(o))
#define GB(o) aksq[curpos+(o)]

} // End of namespace Scumm

#endif
