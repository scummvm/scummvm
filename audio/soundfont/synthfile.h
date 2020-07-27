/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_SYNTHFILE_H
#define AUDIO_SOUNDFONT_SYNTHFILE_H

#include "common/scummsys.h"
#include "common/list.h"
#include "common/str.h"
#include "common/array.h"
#include "rifffile.h"

struct Loop;
class VGMSamp;

class SynthInstr;
class SynthRgn;
class SynthArt;
class SynthConnectionBlock;
class SynthSampInfo;
class SynthWave;

typedef enum {
	no_transform, concave_transform
} Transform;

class SynthFile {
public:
	SynthFile(const Common::String synth_name = "Instrument Set");

	~SynthFile();

	SynthInstr *AddInstr(uint32 bank, uint32 instrNum);
	SynthInstr *AddInstr(uint32 bank, uint32 instrNum, Common::String Name);
	void DeleteInstr(uint32 bank, uint32 instrNum);
	SynthWave *AddWave(uint16 formatTag, uint16 channels, int samplesPerSec, int aveBytesPerSec,
					   uint16 blockAlign, uint16 bitsPerSample, uint32 waveDataSize,
					   uint8 *waveData, Common::String name = "Unnamed Wave");
	void SetName(Common::String synth_name);

	// int WriteDLSToBuffer(Common::Array<uint8> &buf);
	// bool SaveDLSFile(const char* filepath);

public:
	Common::Array<SynthInstr *> vInstrs;
	Common::Array<SynthWave *> vWaves;
	Common::String name;
};

class SynthInstr {
public:
	SynthInstr(void);
	SynthInstr(uint32 bank, uint32 instrument);
	SynthInstr(uint32 bank, uint32 instrument, Common::String instrName);
	SynthInstr(uint32 bank, uint32 instrument, Common::String instrName,
			   Common::Array<SynthRgn *> listRgns);
	~SynthInstr(void);

	void AddRgnList(Common::Array<SynthRgn> &RgnList);
	SynthRgn *AddRgn(void);
	SynthRgn *AddRgn(SynthRgn rgn);

public:
	uint32 ulBank;
	uint32 ulInstrument;

	Common::Array<SynthRgn *> vRgns;
	Common::String name;
};

class SynthRgn {
public:
	SynthRgn()
			: usKeyLow(0),
			  usKeyHigh(0),
			  usVelLow(0),
			  usVelHigh(0),
			  sampinfo(NULL),
			  art(NULL),
			  fusOptions(0),
			  usPhaseGroup(0),
			  channel(0),
			  tableIndex(0) {}

	~SynthRgn();

	SynthArt *AddArt(void);
	SynthArt *AddArt(Common::Array<SynthConnectionBlock *> connBlocks);
	SynthSampInfo *AddSampInfo(void);
	SynthSampInfo *AddSampInfo(SynthSampInfo wsmp);
	void SetRanges(uint16 keyLow = 0, uint16 keyHigh = 0x7F, uint16 velLow = 0,
				   uint16 velHigh = 0x7F);
	void SetWaveLinkInfo(uint16 options, uint16 phaseGroup, uint32 theChannel,
						 uint32 theTableIndex);

public:
	uint16 usKeyLow;
	uint16 usKeyHigh;
	uint16 usVelLow;
	uint16 usVelHigh;

	uint16 fusOptions;
	uint16 usPhaseGroup;
	uint32 channel;
	uint32 tableIndex;

	SynthSampInfo *sampinfo;
	SynthArt *art;
};

class SynthArt {
public:
	SynthArt() {}
	~SynthArt();

	void AddADSR(double attack, Transform atk_transform, double decay, double sustain_lev,
				 double sustain_time, double release_time, Transform rls_transform);
	void AddPan(double pan);

	double pan;  // -100% = left channel 100% = right channel 0 = 50/50

	double attack_time;  // rate expressed as seconds from 0 to 100% level
	double decay_time;   // rate expressed as seconds from 100% to 0% level, even though the sustain
	                     // level isn't necessarily 0%
	double sustain_lev;  // db of attenuation at sustain level
	double sustain_time;  // this is part of the PSX envelope (and can actually be positive), but is
	                      // not in DLS or SF2.  from 100 to 0, like release
	double release_time;  // rate expressed as seconds from 100% to 0% level, even though the
	                      // sustain level may not be 100%
	Transform attack_transform;
	Transform release_transform;

private:
};

class SynthSampInfo {
public:
	SynthSampInfo() {}

	SynthSampInfo(uint16 unityNote, int16 fineTune, double atten, int8 sampleLoops,
				  uint32 loopType, uint32 loopStart, uint32 loopLength)
			: usUnityNote(unityNote),
			  sFineTune(fineTune),
			  attenuation(atten),
			  cSampleLoops(sampleLoops),
			  ulLoopType(loopType),
			  ulLoopStart(loopStart),
			  ulLoopLength(loopLength) {}

	~SynthSampInfo() {}

	void SetLoopInfo(Loop &loop, VGMSamp *samp);
	// void SetPitchInfo(uint16 unityNote, int16 fineTune, double attenuation);
	void SetPitchInfo(uint16 unityNote, int16 fineTune, double attenuation);

public:
	uint16 usUnityNote;
	int16 sFineTune;
	double attenuation;  // in decibels.
	int8 cSampleLoops;

	uint32 ulLoopType;
	uint32 ulLoopStart;
	uint32 ulLoopLength;
};

class SynthWave {
public:
	SynthWave(void) : sampinfo(NULL), data(NULL), name("Untitled Wave") {
		RiffFile::AlignName(name);
	}

	SynthWave(uint16 formatTag, uint16 channels, int samplesPerSec, int aveBytesPerSec,
			  uint16 blockAlign, uint16 bitsPerSample, uint32 waveDataSize, uint8 *waveData,
			  Common::String waveName = "Untitled Wave")
			: wFormatTag(formatTag),
			  wChannels(channels),
			  dwSamplesPerSec(samplesPerSec),
			  dwAveBytesPerSec(aveBytesPerSec),
			  wBlockAlign(blockAlign),
			  wBitsPerSample(bitsPerSample),
			  dataSize(waveDataSize),
			  data(waveData),
			  sampinfo(NULL),
			  name(waveName) {
		RiffFile::AlignName(name);
	}

	~SynthWave(void);

	SynthSampInfo *AddSampInfo(void);

	void ConvertTo16bitSigned();

public:
	SynthSampInfo *sampinfo;

	uint16 wFormatTag;
	uint16 wChannels;
	uint32 dwSamplesPerSec;
	uint32 dwAveBytesPerSec;
	uint16 wBlockAlign;
	uint16 wBitsPerSample;

	uint32 dataSize;
	uint8 *data;

	Common::String name;
};

#endif // AUDIO_SOUNDFONT_SYNTHFILE_H
