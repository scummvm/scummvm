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

#include "synthfile.h"
#include "vgmsamp.h"

using namespace std;

//  **********************************************************************************
//  SynthFile - An intermediate class to lay out all of the the data necessary for Coll conversion
//				to DLS or SF2 formats.  Currently, the structure is identical to
//DLS.
//  **********************************************************************************

SynthFile::SynthFile(Common::String synth_name) : _name(synth_name) {}

SynthFile::~SynthFile() {
	DeleteVect(_vInstrs);
	DeleteVect(_vWaves);
}

SynthInstr *SynthFile::AddInstr(uint32 bank, uint32 instrNum) {
	Common::String str = Common::String::format("Instr bnk%d num%d", bank, instrNum);
	_vInstrs.insert(_vInstrs.end(), new SynthInstr(bank, instrNum, str));
	return _vInstrs.back();
}

SynthWave *SynthFile::AddWave(uint16 formatTag, uint16 channels, int samplesPerSec,
							  int aveBytesPerSec, uint16 blockAlign, uint16 bitsPerSample,
							  uint32 waveDataSize, unsigned char *waveData, Common::String WaveName) {
	_vWaves.insert(_vWaves.end(),
				   new SynthWave(formatTag, channels, samplesPerSec, aveBytesPerSec, blockAlign,
								bitsPerSample, waveDataSize, waveData, WaveName));
	return _vWaves.back();
}

//  **********
//  SynthInstr
//  **********

SynthInstr::SynthInstr(uint32 bank, uint32 instrument, Common::String instrName)
		: _ulBank(bank), _ulInstrument(instrument), _name(instrName) {
	// RiffFile::AlignName(name);
}

SynthInstr::~SynthInstr() {
	DeleteVect(_vRgns);
}

SynthRgn *SynthInstr::AddRgn(void) {
	_vRgns.insert(_vRgns.end(), new SynthRgn());
	return _vRgns.back();
}

//  ********
//  SynthRgn
//  ********

SynthRgn::~SynthRgn(void) {
	if (_sampinfo)
		delete _sampinfo;
	if (_art)
		delete _art;
}

SynthArt *SynthRgn::AddArt(void) {
	_art = new SynthArt();
	return _art;
}

SynthSampInfo *SynthRgn::AddSampInfo(void) {
	_sampinfo = new SynthSampInfo();
	return _sampinfo;
}

void SynthRgn::SetRanges(uint16 keyLow, uint16 keyHigh, uint16 velLow, uint16 velHigh) {
	_usKeyLow = keyLow;
	_usKeyHigh = keyHigh;
	_usVelLow = velLow;
	_usVelHigh = velHigh;
}

void SynthRgn::SetWaveLinkInfo(uint16 options, uint16 phaseGroup, uint32 theChannel,
							   uint32 theTableIndex) {
	_fusOptions = options;
	_usPhaseGroup = phaseGroup;
	_channel = theChannel;
	_tableIndex = theTableIndex;
}

//  ********
//  SynthArt
//  ********

SynthArt::~SynthArt() {
}

void SynthArt::AddADSR(double attack, Transform atk_transform, double decay, double sustain_level,
					   double sustain, double release, Transform rls_transform) {
	this->_attack_time = attack;
	this->_attack_transform = atk_transform;
	this->_decay_time = decay;
	this->_sustain_lev = sustain_level;
	this->_sustain_time = sustain;
	this->_release_time = release;
	this->_release_transform = rls_transform;
}

void SynthArt::AddPan(double thePan) {
	this->_pan = thePan;
}

//  *************
//  SynthSampInfo
//  *************

void SynthSampInfo::SetLoopInfo(Loop &loop, VGMSamp *samp) {
	const int origFormatBytesPerSamp = samp->_bps / 8;
	double compressionRatio = samp->GetCompressionRatio();

	// If the sample loops, but the loop length is 0, then assume the length should
	// extend to the end of the sample.
	if (loop.loopStatus && loop.loopLength == 0)
		loop.loopLength = samp->_dataLength - loop.loopStart;

	_cSampleLoops = loop.loopStatus;
	_ulLoopStart = (loop.loopStartMeasure == LM_BYTES)
				  ? (uint32) ((loop.loopStart * compressionRatio) / origFormatBytesPerSamp)
				  : loop.loopStart;
	_ulLoopLength = (loop.loopLengthMeasure == LM_BYTES)
				   ? (uint32) ((loop.loopLength * compressionRatio) / origFormatBytesPerSamp)
				   : loop.loopLength;
}

void SynthSampInfo::SetPitchInfo(uint16 unityNote, short fineTune, double atten) {
	_usUnityNote = unityNote;
	_sFineTune = fineTune;
	_attenuation = atten;
}

//  *********
//  SynthWave
//  *********

void SynthWave::ConvertTo16bitSigned() {
	if (_wBitsPerSample == 8) {
		this->_wBitsPerSample = 16;
		this->_wBlockAlign = 16 / 8 * this->_wChannels;
		this->_dwAveBytesPerSec *= 2;

		int16 *newData = new int16[this->_dataSize];
		for (unsigned int i = 0; i < this->_dataSize; i++)
			newData[i] = ((int16) this->_data[i] - 128) << 8;
		delete[] this->_data;
		this->_data = (uint8 *) newData;
		this->_dataSize *= 2;
	}
}

SynthWave::~SynthWave() {
	delete _sampinfo;
	delete[] _data;
}

SynthSampInfo *SynthWave::AddSampInfo(void) {
	_sampinfo = new SynthSampInfo();
	return _sampinfo;
}
