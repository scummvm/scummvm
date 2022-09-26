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

#define AKOS_BYLE_RLE_CODEC   1
#define AKOS_CDAT_RLE_CODEC   5
#define AKOS_RUN_MAJMIN_CODEC 16
#define AKOS_TRLE_CODEC       32

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

enum AkosSequenceCodes {
	// Auxiliary uSweat tokens:
	AKC_ExtendBit =     0x80,
	AKC_ExtendWordBit = 0x8000,

	// Opcode uSweat tokens:
	AKC_CommandMask = 0xC000,

	AKC_EmptyCel =                   (AKC_CommandMask | 0x0001),

	AKC_SetVar =                     (AKC_CommandMask | 0x0010),
	AKC_StartSound =                 (AKC_CommandMask | 0x0015),
	AKC_IfSoundInVarRunningGoTo =    (AKC_CommandMask | 0x0016),
	AKC_IfNotSoundInVarRunningGoTo = (AKC_CommandMask | 0x0017),
	AKC_IfSoundRunningGoTo =         (AKC_CommandMask | 0x0018),
	AKC_IfNotSoundRunningGoTo =      (AKC_CommandMask | 0x0019),

	AKC_DrawMany =                   (AKC_CommandMask | 0x0020),
	AKC_CondDrawMany =               (AKC_CommandMask | 0x0021),
	AKC_CondRelativeOffsetDrawMany = (AKC_CommandMask | 0x0022),
	AKC_RelativeOffsetDrawMany =     (AKC_CommandMask | 0x0025),

	AKC_GoToState =                  (AKC_CommandMask | 0x0030),
	AKC_IfVarGoTo =                  (AKC_CommandMask | 0x0031),

	AKC_AddVar =                     (AKC_CommandMask | 0x0040),
	AKC_SoftSound =                  (AKC_CommandMask | 0x0042),
	AKC_SoftVarSound =               (AKC_CommandMask | 0x0044),
	AKC_SetUserCondition =           (AKC_CommandMask | 0x0045),
	AKC_SetVarToUserCondition =      (AKC_CommandMask | 0x0046),
	AKC_SetTalkCondition =           (AKC_CommandMask | 0x0047),
	AKC_SetVarToTalkCondition =      (AKC_CommandMask | 0x0048),
	AKC_StartScript =                (AKC_CommandMask | 0x0050),

	AKC_IncVar =                     (AKC_CommandMask | 0x0060),
	AKC_StartSound_SpecialCase =     (AKC_CommandMask | 0x0061),

	AKC_IfVarEQJump =                (AKC_CommandMask | 0x0070),
	AKC_IfVarNEJump =                (AKC_CommandMask | 0x0071),
	AKC_IfVarLTJump =                (AKC_CommandMask | 0x0072),
	AKC_IfVarLEJump =                (AKC_CommandMask | 0x0073),
	AKC_IfVarGTJump =                (AKC_CommandMask | 0x0074),
	AKC_IfVarGEJump =                (AKC_CommandMask | 0x0075),

	AKC_StartAnim =                  (AKC_CommandMask | 0x0080),
	AKC_StartVarAnim =               (AKC_CommandMask | 0x0081),
	AKC_SetVarRandom =               (AKC_CommandMask | 0x0082),
	AKC_SetActorZClipping =          (AKC_CommandMask | 0x0083),
	AKC_StartActorAnim =             (AKC_CommandMask | 0x0084),
	AKC_SetActorVar =                (AKC_CommandMask | 0x0085),
	AKC_HideActor =                  (AKC_CommandMask | 0x0086),
	AKC_SetDrawOffs =                (AKC_CommandMask | 0x0087),
	AKC_JumpToOffsetInVar =          (AKC_CommandMask | 0x0088),
	AKC_SoundStuff =                 (AKC_CommandMask | 0x0089),
	AKC_Flip =                       (AKC_CommandMask | 0x008A),
	AKC_StartActionOn =              (AKC_CommandMask | 0x008B),
	AKC_StartScriptVar =             (AKC_CommandMask | 0x008C),
	AKC_StartSoundVar =              (AKC_CommandMask | 0x008D),
	AKC_DisplayAuxFrame =            (AKC_CommandMask | 0x008E),

	AKC_IfVarEQDo =                  (AKC_CommandMask | 0x0090),
	AKC_IfVarNEDo =                  (AKC_CommandMask | 0x0091),
	AKC_IfVarLTDo =                  (AKC_CommandMask | 0x0092),
	AKC_IfVarLEDo =                  (AKC_CommandMask | 0x0093),
	AKC_IfVarGTDo =                  (AKC_CommandMask | 0x0094),
	AKC_IfVarGEDo =                  (AKC_CommandMask | 0x0095),
	AKC_EndOfIfDo =                  (AKC_CommandMask | 0x009F),

	AKC_StartActorTalkie =           (AKC_CommandMask | 0x00A0),
	AKC_IfTalkingGoTo =              (AKC_CommandMask | 0x00A1),
	AKC_IfNotTalkingGoTo =           (AKC_CommandMask | 0x00A2),
	AKC_StartTalkieInVar =           (AKC_CommandMask | 0x00A3),
	AKC_IfAnyTalkingGoTo =           (AKC_CommandMask | 0x00A4),
	AKC_IfNotAnyTalkingGoTo =        (AKC_CommandMask | 0x00A5),
	AKC_IfTalkingPickGoTo =          (AKC_CommandMask | 0x00A6),
	AKC_IfNotTalkingPickGoTo =       (AKC_CommandMask | 0x00A7),

	AKC_EndSeq =                     (AKC_CommandMask | 0x00FF)
};

enum AkosQueuedCommands {
	AKQC_PutActorInTheVoid = 1,
	AKQC_StartSound = 3,
	AKQC_StartAnimation = 4,
	AKQC_SetZClipping = 5,
	AKQC_SetXYOffset = 6,
	AKQC_DisplayAuxFrame = 7,
	AKQC_StartTalkie = 8,
	AKQC_SoftStartSound = 9,

	// For V7-8
	AKQC_SetSoundVolume = 7,
	AKQC_SetSoundPan = 8,
	AKQC_SetSoundPriority = 9
};

enum AkosAnimTypes {
	AKAT_Empty =          0x00,
	AKAT_KillLayer =      0x01,
	AKAT_LoopLayer =      0x02,
	AKAT_RunLayer =       0x03,
	AKAT_LayerInvisible = 0x04,
	AKAT_LayerVisible =   0x05,
	AKAT_AlwaysRun =      0x06,
	AKAT_UserConstant =   0x07,
	AKAT_DeltaAnim =      0x08
};

#define GW(o) ((int16)READ_LE_UINT16(aksq + curState + (o)))
#define GUW(o) READ_LE_UINT16(aksq + curState + (o))
#define GB(o) aksq[curState + (o)]

} // End of namespace Scumm

#endif
