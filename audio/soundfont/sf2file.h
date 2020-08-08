/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included VGMTrans_LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_SF2FILE_H
#define AUDIO_SOUNDFONT_SF2FILE_H

#include "common/endian.h"
#include "common/scummsys.h"
#include "common/array.h"
#include "common/str.h"
#include "rifffile.h"

typedef enum {
	// Oscillator
	startAddrsOffset,  // sample start address -4 (0 to 0xffffff)   0
	endAddrsOffset,
	startloopAddrsOffset,  // loop start address -4 (0 to 0xffffff)
	endloopAddrsOffset,    // loop end address -3 (0 to 0xffffff)

	// Pitch
	startAddrsCoarseOffset,  // CHANGED FOR SF2
	modLfoToPitch,           // main fm: lfo1-> pitch                     5
	vibLfoToPitch,           // aux fm:  lfo2-> pitch
	modEnvToPitch,           // pitch env: env1(aux)-> pitch

	// Filter
	initialFilterFc,   // initial filter cutoff
	initialFilterQ,    // filter Q
	modLfoToFilterFc,  // filter modulation: lfo1 -> filter cutoff  10
	modEnvToFilterFc,  // filter env: env1(aux)-> filter cutoff

	// Amplifier
	endAddrsCoarseOffset,  // CHANGED FOR SF2
	modLfoToVolume,        // tremolo: lfo1-> volume
	unused1,

	// Effects
	chorusEffectsSend,  // chorus                                    15
	reverbEffectsSend,  // reverb
	pan,
	unused2,
	unused3,
	unused4,  //                                          20

	// Main lfo1
	delayModLFO,  // delay 0x8000-n*(725us)
	freqModLFO,   // frequency

	// Aux lfo2
	delayVibLFO,  // delay 0x8000-n*(725us)
	freqVibLFO,   // frequency

	// Env1(aux/value)
	delayModEnv,    // delay 0x8000 - n(725us)                   25
	attackModEnv,   // attack
	holdModEnv,     // hold
	decayModEnv,    // decay
	sustainModEnv,  // sustain
	releaseModEnv,  // release                                   30
	keynumToModEnvHold,
	keynumToModEnvDecay,

	// Env2(ampl/vol)
	delayVolEnv,    // delay 0x8000 - n(725us)
	attackVolEnv,   // attack
	holdVolEnv,     // hold                                      35
	decayVolEnv,    // decay
	sustainVolEnv,  // sustain
	releaseVolEnv,  // release
	keynumToVolEnvHold,
	keynumToVolEnvDecay,  //                                          40

	// Preset
	instrument,
	reserved1,
	keyRange,
	velRange,
	startloopAddrCoarseOffset,  // CHANGED FOR SF2                       45
	keynum,
	velocity,
	initialAttenuation,  // CHANGED FOR SF2
	reserved2,
	endloopAddrsCoarseOffset,  // CHANGED FOR SF2                       50
	coarseTune,
	fineTune,
	sampleID,
	sampleModes,  // CHANGED FOR SF2
	reserved3,    //                                      55
	scaleTuning,
	exclusiveClass,
	overridingRootKey,
	unused5,
	endOper  //                                      60
} SFGeneratorType;

typedef uint16 SFGenerator;

typedef enum {
	/* Start of MIDI modulation operators */
	cc1_Mod,
	cc7_Vol,
	cc10_Pan,
	cc64_Sustain,
	cc91_Reverb,
	cc93_Chorus,

	ccPitchBend,
	ccIndirectModX,
	ccIndirectModY,

	endMod
} SFModulatorType;

typedef uint16 SFModulator;

typedef enum {
	linear
} SFTransformType;

typedef uint16 SFTransform;
/*
#define monoSample      0x0001
#define rightSample     0x0002
#define leftSample      0x0004
#define linkedSample    0x0008

#define ROMSample       0x8000          //32768
#define ROMMonoSample   0x8001          //32769
#define ROMRightSample  0x8002          //32770
#define ROMLeftSample   0x8004          //32772
#define ROMLinkedSample 0x8008          //32776
*/

// enum scaleTuning
//{
//    equalTemp,
//    fiftyCents
//};
//
// enum SFSampleField          //used by Sample Read Module
//{
//    NAME_FIELD = 1,
//    START_FIELD,
//    END_FIELD,
//    START_LOOP_FIELD,
//    END_LOOP_FIELD,
//    SMPL_RATE_FIELD,
//    ORG_KEY_FIELD,
//    CORRECTION_FIELD,
//    SMPL_LINK_FIELD,
//    SMPL_TYPE_FIELD
//};
//
// enum SFInfoChunkField       //used by Bank Read Module
//{
//    IFIL_FIELD = 1,
//    IROM_FIELD,
//    IVER_FIELD,
//    ISNG_FIELD,
//    INAM_FIELD,
//    IPRD_FIELD,
//    IENG_FIELD,
//    ISFT_FIELD,
//    ICRD_FIELD,
//    ICMT_FIELD,
//    ICOP_FIELD
//};

static inline void uint16Write(uint8 **buffer, uint16 value) {
	WRITE_LE_UINT16(*buffer, value);
	*buffer += 2;
}

static inline void uint32Write(uint8 **buffer, uint32 value) {
	WRITE_LE_UINT32(*buffer, value);
	*buffer += 4;
}

static inline void writeBytes(uint8 **buffer, byte *bytes, uint32 size) {
	memcpy(*buffer, bytes, size);
	*buffer += size;
}

#pragma pack(push) /* push current alignment to stack */
#pragma pack(2)    /* set alignment to 2 byte boundary */

struct sfVersionTag {
	uint16 wMajor;
	uint16 wMinor;

	uint8 *write(uint8 *buffer) {
		uint16Write(&buffer, wMajor);
		uint16Write(&buffer, wMinor);
		return buffer;
	}
};

struct sfPresetHeader {
	char achPresetName[20];
	uint16 wPreset;
	uint16 wBank;
	uint16 wPresetBagNdx;
	uint32 dwLibrary;
	uint32 dwGenre;
	uint32 dwMorphology;

	uint8 *write(uint8 *buffer) {
		writeBytes(&buffer, (byte *)achPresetName, 20);
		uint16Write(&buffer, wPreset);
		uint16Write(&buffer, wBank);
		uint16Write(&buffer, wPresetBagNdx);
		uint32Write(&buffer, dwLibrary);
		uint32Write(&buffer, dwGenre);
		uint32Write(&buffer, dwMorphology);
		return buffer;
	}
};

struct sfPresetBag {
	uint16 wGenNdx;
	uint16 wModNdx;

	uint8 *write(uint8 *buffer) {
		uint16Write(&buffer, wGenNdx);
		uint16Write(&buffer, wModNdx);
		return buffer;
	}
};

struct sfModList {
	SFModulator sfModSrcOper;
	SFGenerator sfModDestOper;
	int16 modAmount;
	SFModulator sfModAmtSrcOper;
	SFTransform sfModTransOper;

	uint8 *write(uint8 *buffer) {
		uint16Write(&buffer, sfModSrcOper);
		uint16Write(&buffer, sfModDestOper);
		uint16Write(&buffer, modAmount);
		uint16Write(&buffer, sfModAmtSrcOper);
		uint16Write(&buffer, sfModTransOper);
		return buffer;
	}
};

struct genAmountType {
	byte data[2];

	void setRangeLo(uint8 lo) { data[0] = lo; }
	void setRangeHi(uint8 hi) { data[1] = hi; }
	void setShAmount(int16 shAmount) { WRITE_LE_INT16(data, shAmount); }
	void setwAmount(int16 wAmount) { WRITE_LE_UINT16(data, wAmount); }

	uint8 *write(uint8 *buffer) {
		writeBytes(&buffer, (byte *)data, 2);
		return buffer;
	}
};

struct sfGenList {
	SFGenerator sfGenOper;
	genAmountType genAmount;

	uint8 *write(uint8 *buffer) {
		uint16Write(&buffer, sfGenOper);
		return genAmount.write(buffer);
	}
};

struct sfInstGenList {
	SFGenerator sfGenOper;
	genAmountType genAmount;

	uint8 *write(uint8 *buffer) {
		uint16Write(&buffer, sfGenOper);
		return genAmount.write(buffer);
	}
};

struct sfInst {
	char achInstName[20];
	uint16 wInstBagNdx;

	uint8 *write(uint8 *buffer) {
		writeBytes(&buffer, (byte *)achInstName, 20);
		uint16Write(&buffer, wInstBagNdx);
		return buffer;
	}
};

struct sfInstBag {
	uint16 wInstGenNdx;
	uint16 wInstModNdx;

	uint8 *write(uint8 *buffer) {
		uint16Write(&buffer, wInstGenNdx);
		uint16Write(&buffer, wInstModNdx);
		return buffer;
	}
};

typedef enum {
	monoSample = 1,
	rightSample = 2,
	leftSample = 4,
	linkedSample = 8,
	RomMonoSample = 0x8001,
	RomRightSample = 0x8002,
	RomLeftSample = 0x8004,
	RomLinkedSample = 0x8008
} SFSampleLinkType;

typedef uint16 SFSampleLink;

struct sfSample {
	char achSampleName[20];
	uint32 dwStart;
	uint32 dwEnd;
	uint32 dwStartloop;
	uint32 dwEndloop;
	uint32 dwSampleRate;
	uint8 byOriginalKey;
	char chCorrection;
	uint16 wSampleLink;
	SFSampleLink sfSampleType;

	uint8 *write(uint8 *buffer) {
		writeBytes(&buffer, (byte *)achSampleName, 20);
		uint32Write(&buffer, dwStart);
		uint32Write(&buffer, dwEnd);
		uint32Write(&buffer, dwStartloop);
		uint32Write(&buffer, dwEndloop);
		uint32Write(&buffer, dwSampleRate);
		*buffer++ = byOriginalKey;
		*buffer++ = chCorrection;
		uint16Write(&buffer, wSampleLink);
		uint16Write(&buffer, sfSampleType);
		return buffer;
	}
};

#pragma pack(pop) /* restore original alignment from stack */

class SF2StringChunk : public Chunk {
public:
	SF2StringChunk(Common::String ckSig, Common::String info) : Chunk(ckSig) {
		SetData(info.c_str(), (uint32) info.size());
	}
};

class SF2InfoListChunk : public LISTChunk {
public:
	SF2InfoListChunk(Common::String name);
};

class SF2sdtaChunk : public LISTChunk {
public:
	SF2sdtaChunk();
};

inline void WriteLIST(Common::Array<uint8> &buf, Common::String listName, uint32 listSize);
inline void AlignName(Common::String &name);

class SynthFile;

class SF2File : public RiffFile {
public:
	SF2File(SynthFile *synthfile);
	~SF2File(void);

	const void *SaveToMem();
};

#endif // AUDIO_SOUNDFONT_SF2FILE_H
