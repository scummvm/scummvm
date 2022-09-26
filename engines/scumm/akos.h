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
	AKC_EmptyCel = 0xC001,
	AKC_SetVar = 0xC010,
	AKC_StartSound = 0xC015,
	AKC_IfSoundInVarRunningGoTo = 0xC016,
	AKC_IfNotSoundInVarRunningGoTo = 0xC017,
	AKC_IfSoundRunningGoTo = 0xC018,
	AKC_IfNotSoundRunningGoTo = 0xC019,
	AKC_DrawMany = 0xC020,
	AKC_CondDrawMany = 0xC021,
	AKC_CondRelativeOffsetDrawMany = 0xC022,
	AKC_RelativeOffsetDrawMany = 0xC025,
	AKC_GoToState = 0xC030,
	AKC_IfVarGoTo = 0xC031,
	AKC_AddVar = 0xC040,
	AKC_SoftSound = 0xC042,
	AKC_SoftVarSound = 0xC044,
	AKC_SetUserCondition = 0xC045,
	AKC_SetVarToUserCondition = 0xC046,
	AKC_SetTalkCondition = 0xC047,
	AKC_SetVarToTalkCondition = 0xC048,
	AKC_StartScript = 0xC050,
	AKC_IncVar = 0xC060,
	AKC_StartSound_SpecialCase = 0xC061,
	AKC_ConditionalJumpStart = 0xC070,
	AKC_IfVarEQJump = 0xC070,
	AKC_IfVarNEJump = 0xC071,
	AKC_IfVarLTJump = 0xC072,
	AKC_IfVarLEJump = 0xC073,
	AKC_IfVarGTJump = 0xC074,
	AKC_IfVarGEJump = 0xC075,
	AKC_StartAnim = 0xC080,
	AKC_StartVarAnim = 0xC081,
	AKC_SetVarRandom = 0xC082,
	AKC_SetActorZClipping = 0xC083,
	AKC_StartActorAnim = 0xC084,
	AKC_SetActorVar = 0xC085,
	AKC_HideActor = 0xC086,
	AKC_SetDrawOffs = 0xC087,
	AKC_JumpToOffsetInVar = 0xC088,
	AKC_SoundStuff = 0xC089,
	AKC_Flip = 0xC08A,
	AKC_StartActionOn = 0xC08B,
	AKC_StartScriptVar = 0xC08C,
	AKC_StartSoundVar = 0xC08D,
	AKC_DisplayAuxFrame = 0xC08E,
	AKC_ConditionalDoStart = 0xC090,
	AKC_IfVarEQDo = 0xC090,
	AKC_IfVarNEDo = 0xC091,
	AKC_IfVarLTDo = 0xC092,
	AKC_IfVarLEDo = 0xC093,
	AKC_IfVarGTDo = 0xC094,
	AKC_IfVarGEDo = 0xC095,
	AKC_EndOfIfDo = 0xC09F,
	AKC_StartActorTalkie = 0xC0A0,
	AKC_IfTalkingGoTo = 0xC0A1,
	AKC_IfNotTalkingGoTo = 0xC0A2,
	AKC_StartTalkieInVar = 0xC0A3,
	AKC_IfAnyTalkingGoTo = 0xC0A4,
	AKC_IfNotAnyTalkingGoTo = 0xC0A5,
	AKC_IfTalkingPickGoTo = 0xC0A6,
	AKC_IfNotTalkingPickGoTo = 0xC0A7,
	AKC_EndSeq = 0xC0FF
};

#define GW(o) ((int16)READ_LE_UINT16(aksq+curpos+(o)))
#define GUW(o) READ_LE_UINT16(aksq+curpos+(o))
#define GB(o) aksq[curpos+(o)]

} // End of namespace Scumm

#endif
