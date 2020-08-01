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

	inline void SetLoopStatus(int loopStat) { _loop.loopStatus = loopStat; }
	inline void SetLoopOffset(uint32 loopStart) { _loop.loopStart = loopStart; }
	inline void SetLoopLength(uint32 theLoopLength) { _loop.loopLength = theLoopLength; }

public:
	WAVE_TYPE _waveType;
	uint32 _dataOff;  // offset of original sample data
	uint32 _dataLength;
	uint16 _bps;      // bits per sample
	uint32 _rate;     // sample rate in herz (samples per second)
	uint8 _channels;  // mono or stereo?
	uint32 _ulUncompressedSize;

	bool _bPSXLoopInfoPrioritizing;
	Loop _loop;

	int8 _unityKey;
	short _fineTune;
	double _volume;  // as percent of full volume.  This will be converted to attenuation for SynthFile

	VGMSampColl *_parSampColl;
	Common::String _sampName;
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
	bool _bLoaded;

	uint32 _sampDataOffset;  // offset of the beginning of the sample data.  Used for
	// rgn->sampOffset matching
	VGMInstrSet *_parInstrSet;
	Common::Array<VGMSamp *> _samples;
};

#endif // AUDIO_SOUNDFONT_VGMSAMP_H
