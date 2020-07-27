/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_VGMSAMP_H
#define AUDIO_SOUNDFONT_VGMSAMP_H

#include "common.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "vgmitem.h"

class VGMSampColl;
class VGMInstrSet;

enum WAVE_TYPE {
	WT_UNDEFINED, WT_PCM8, WT_PCM16
};

class VGMSamp : public VGMItem {
public:

	VGMSamp(VGMSampColl *sampColl, uint32 offset = 0, uint32 length = 0,
			uint32 dataOffset = 0, uint32 dataLength = 0, uint8 channels = 1,
			uint16 bps = 16, uint32 rate = 0, Common::String name = "Sample");
	virtual ~VGMSamp();

	virtual double GetCompressionRatio();  // ratio of space conserved.  should generally be > 1
	// used to calculate both uncompressed sample size and loopOff after conversion
	virtual void ConvertToStdWave(uint8 *buf) {};

	inline void SetLoopStatus(int loopStat) { loop.loopStatus = loopStat; }
	inline void SetLoopOffset(uint32 loopStart) { loop.loopStart = loopStart; }
	inline void SetLoopLength(uint32 theLoopLength) { loop.loopLength = theLoopLength; }

public:
	WAVE_TYPE waveType;
	uint32 dataOff;  // offset of original sample data
	uint32 dataLength;
	uint16 bps;      // bits per sample
	uint32 rate;     // sample rate in herz (samples per second)
	uint8 channels;  // mono or stereo?
	uint32 ulUncompressedSize;

	bool bPSXLoopInfoPrioritizing;
	Loop loop;

	int8_t unityKey;
	short fineTune;
	double volume;  // as percent of full volume.  This will be converted to attenuation for SynthFile

	long pan;

	VGMSampColl *parSampColl;
	Common::String sampName;
};

class VGMSampColl : public VGMFile {
public:
	VGMSampColl(RawFile *rawfile, uint32 offset, uint32 length = 0,
				Common::String theName = "VGMSampColl");
	VGMSampColl(RawFile *rawfile, VGMInstrSet *instrset, uint32 offset,
				uint32 length = 0, Common::String theName = "VGMSampColl");
	virtual ~VGMSampColl(void);

	virtual bool Load();
	virtual bool GetHeaderInfo() { return true; }  // retrieve any header data
	virtual bool GetSampleInfo() { return true; }  // retrieve sample info, including pointers to data, # channels, rate, etc.

protected:

public:
	bool bLoaded;

	uint32 sampDataOffset;  // offset of the beginning of the sample data.  Used for
	// rgn->sampOffset matching
	VGMInstrSet *parInstrSet;
	Common::Array<VGMSamp *> samples;
};

#endif // AUDIO_SOUNDFONT_VGMSAMP_H
