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
	SynthWave *AddWave(uint16 formatTag, uint16 channels, int samplesPerSec, int aveBytesPerSec,
					   uint16 blockAlign, uint16 bitsPerSample, uint32 waveDataSize,
					   uint8 *waveData, Common::String name = "Unnamed Wave");

public:
	Common::Array<SynthInstr *> _vInstrs;
	Common::Array<SynthWave *> _vWaves;
	Common::String _name;
};

class SynthInstr {
public:
	SynthInstr(uint32 bank, uint32 instrument, Common::String instrName);
	~SynthInstr(void);

	SynthRgn *AddRgn(void);

public:
	uint32 _ulBank;
	uint32 _ulInstrument;

	Common::Array<SynthRgn *> _vRgns;
	Common::String _name;
};

class SynthRgn {
public:
	SynthRgn()
			: _usKeyLow(0),
			  _usKeyHigh(0),
			  _usVelLow(0),
			  _usVelHigh(0),
			  _sampinfo(NULL),
			  _art(NULL),
			  _fusOptions(0),
			  _usPhaseGroup(0),
			  _channel(0),
			  _tableIndex(0) {}

	~SynthRgn();

	SynthArt *AddArt(void);
	SynthSampInfo *AddSampInfo(void);
	void SetRanges(uint16 keyLow = 0, uint16 keyHigh = 0x7F, uint16 velLow = 0,
				   uint16 velHigh = 0x7F);
	void SetWaveLinkInfo(uint16 options, uint16 phaseGroup, uint32 theChannel,
						 uint32 theTableIndex);

public:
	uint16 _usKeyLow;
	uint16 _usKeyHigh;
	uint16 _usVelLow;
	uint16 _usVelHigh;

	uint16 _fusOptions;
	uint16 _usPhaseGroup;
	uint32 _channel;
	uint32 _tableIndex;

	SynthSampInfo *_sampinfo;
	SynthArt *_art;
};

class SynthArt {
public:
	SynthArt() : _pan(0.0), _attack_time(0.0), _decay_time(0.0),
		_sustain_lev(0.0), _sustain_time(0.0), _release_time(0.0),
		_attack_transform(no_transform), _release_transform(no_transform) {}
	~SynthArt();

	void AddADSR(double attack, Transform atk_transform, double decay, double sustain_lev,
				 double sustain_time, double release_time, Transform rls_transform);
	void AddPan(double pan);

	double _pan;  // -100% = left channel 100% = right channel 0 = 50/50

	double _attack_time;  // rate expressed as seconds from 0 to 100% level
	double _decay_time;   // rate expressed as seconds from 100% to 0% level, even though the sustain
	                     // level isn't necessarily 0%
	double _sustain_lev;  // db of attenuation at sustain level
	double _sustain_time;  // this is part of the PSX envelope (and can actually be positive), but is
	                      // not in DLS or SF2.  from 100 to 0, like release
	double _release_time;  // rate expressed as seconds from 100% to 0% level, even though the
	                      // sustain level may not be 100%
	Transform _attack_transform;
	Transform _release_transform;

private:
};

class SynthSampInfo {
public:
	SynthSampInfo() : _usUnityNote(0), _sFineTune(0), _attenuation(0.0),
		_cSampleLoops(0), _ulLoopStart(0), _ulLoopLength(0) {}
	~SynthSampInfo() {}

	void SetLoopInfo(Loop &loop, VGMSamp *samp);
	void SetPitchInfo(uint16 unityNote, int16 fineTune, double attenuation);

public:
	uint16 _usUnityNote;
	int16 _sFineTune;
	double _attenuation;  // in decibels.
	int8 _cSampleLoops;

	uint32 _ulLoopStart;
	uint32 _ulLoopLength;
};

class SynthWave {
public:
	SynthWave(void) : _sampinfo(NULL), _data(NULL), _name("Untitled Wave") {
		RiffFile::AlignName(_name);
	}

	SynthWave(uint16 formatTag, uint16 channels, int samplesPerSec, int aveBytesPerSec,
			  uint16 blockAlign, uint16 bitsPerSample, uint32 waveDataSize, uint8 *waveData,
			  Common::String waveName = "Untitled Wave")
			: _wFormatTag(formatTag),
			  _wChannels(channels),
			  _dwSamplesPerSec(samplesPerSec),
			  _dwAveBytesPerSec(aveBytesPerSec),
			  _wBlockAlign(blockAlign),
			  _wBitsPerSample(bitsPerSample),
			  _dataSize(waveDataSize),
			  _data(waveData),
			  _sampinfo(NULL),
			  _name(waveName) {
		RiffFile::AlignName(_name);
	}

	~SynthWave(void);

	SynthSampInfo *AddSampInfo(void);
	void ConvertTo16bitSigned();

public:
	SynthSampInfo *_sampinfo;

	uint16 _wFormatTag;
	uint16 _wChannels;
	uint32 _dwSamplesPerSec;
	uint32 _dwAveBytesPerSec;
	uint16 _wBlockAlign;
	uint16 _wBitsPerSample;

	uint32 _dataSize;
	uint8 *_data;

	Common::String _name;
};

#endif // AUDIO_SOUNDFONT_SYNTHFILE_H
