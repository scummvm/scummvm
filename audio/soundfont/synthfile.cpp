/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */

#include "synthfile.h"
#include "vgmsamp.h"

using namespace std;

//  **********************************************************************************
//  SynthFile - An intermediate class to lay out all of the the data necessary for Coll conversion
//				to DLS or SF2 formats.  Currently, the structure is identical to
//DLS.
//  **********************************************************************************

SynthFile::SynthFile(Common::String synth_name) : name(synth_name) {}

SynthFile::~SynthFile() {
	DeleteVect(vInstrs);
	DeleteVect(vWaves);
}

SynthInstr *SynthFile::AddInstr(uint32 bank, uint32 instrNum) {
	Common::String str = Common::String::format("Instr bnk%d num%d", bank, instrNum);
	vInstrs.insert(vInstrs.end(), new SynthInstr(bank, instrNum, str));
	return vInstrs.back();
}

SynthInstr *SynthFile::AddInstr(uint32 bank, uint32 instrNum, Common::String instrName) {
	vInstrs.insert(vInstrs.end(), new SynthInstr(bank, instrNum, instrName));
	return vInstrs.back();
}

void SynthFile::DeleteInstr(uint32 bank, uint32 instrNum) {}

SynthWave *SynthFile::AddWave(uint16 formatTag, uint16 channels, int samplesPerSec,
							  int aveBytesPerSec, uint16 blockAlign, uint16 bitsPerSample,
							  uint32 waveDataSize, unsigned char *waveData, Common::String WaveName) {
	vWaves.insert(vWaves.end(),
				  new SynthWave(formatTag, channels, samplesPerSec, aveBytesPerSec, blockAlign,
								bitsPerSample, waveDataSize, waveData, WaveName));
	return vWaves.back();
}

//  **********
//  SynthInstr
//  **********

SynthInstr::SynthInstr(uint32 bank, uint32 instrument)
		: ulBank(bank), ulInstrument(instrument) {
	name = Common::String::format("Instr bnk %d num %d", bank, instrument);
	// RiffFile::AlignName(name);
}

SynthInstr::SynthInstr(uint32 bank, uint32 instrument, Common::String instrName)
		: ulBank(bank), ulInstrument(instrument), name(instrName) {
	// RiffFile::AlignName(name);
}

SynthInstr::SynthInstr(uint32 bank, uint32 instrument, Common::String instrName,
					   Common::Array<SynthRgn *> listRgns)
		: ulBank(bank), ulInstrument(instrument), name(instrName) {
	// RiffFile::AlignName(name);
	vRgns = listRgns;
}

SynthInstr::~SynthInstr() {
	DeleteVect(vRgns);
}

SynthRgn *SynthInstr::AddRgn(void) {
	vRgns.insert(vRgns.end(), new SynthRgn());
	return vRgns.back();
}

SynthRgn *SynthInstr::AddRgn(SynthRgn rgn) {
	SynthRgn *newRgn = new SynthRgn();
	*newRgn = rgn;
	vRgns.insert(vRgns.end(), newRgn);
	return vRgns.back();
}

//  ********
//  SynthRgn
//  ********

SynthRgn::~SynthRgn(void) {
	if (sampinfo)
		delete sampinfo;
	if (art)
		delete art;
}

SynthArt *SynthRgn::AddArt(void) {
	art = new SynthArt();
	return art;
}

SynthSampInfo *SynthRgn::AddSampInfo(void) {
	sampinfo = new SynthSampInfo();
	return sampinfo;
}

void SynthRgn::SetRanges(uint16 keyLow, uint16 keyHigh, uint16 velLow, uint16 velHigh) {
	usKeyLow = keyLow;
	usKeyHigh = keyHigh;
	usVelLow = velLow;
	usVelHigh = velHigh;
}

void SynthRgn::SetWaveLinkInfo(uint16 options, uint16 phaseGroup, uint32 theChannel,
							   uint32 theTableIndex) {
	fusOptions = options;
	usPhaseGroup = phaseGroup;
	channel = theChannel;
	tableIndex = theTableIndex;
}

//  ********
//  SynthArt
//  ********

SynthArt::~SynthArt() {
}

void SynthArt::AddADSR(double attack, Transform atk_transform, double decay, double sustain_level,
					   double sustain, double release, Transform rls_transform) {
	this->attack_time = attack;
	this->attack_transform = atk_transform;
	this->decay_time = decay;
	this->sustain_lev = sustain_level;
	this->sustain_time = sustain;
	this->release_time = release;
	this->release_transform = rls_transform;
}

void SynthArt::AddPan(double thePan) {
	this->pan = thePan;
}

//  *************
//  SynthSampInfo
//  *************

void SynthSampInfo::SetLoopInfo(Loop &loop, VGMSamp *samp) {
	const int origFormatBytesPerSamp = samp->bps / 8;
	double compressionRatio = samp->GetCompressionRatio();

	// If the sample loops, but the loop length is 0, then assume the length should
	// extend to the end of the sample.
	if (loop.loopStatus && loop.loopLength == 0)
		loop.loopLength = samp->dataLength - loop.loopStart;

	cSampleLoops = loop.loopStatus;
	ulLoopType = loop.loopType;
	ulLoopStart = (loop.loopStartMeasure == LM_BYTES)
				  ? (uint32) ((loop.loopStart * compressionRatio) / origFormatBytesPerSamp)
				  : loop.loopStart;
	ulLoopLength = (loop.loopLengthMeasure == LM_BYTES)
				   ? (uint32) ((loop.loopLength * compressionRatio) / origFormatBytesPerSamp)
				   : loop.loopLength;
}

void SynthSampInfo::SetPitchInfo(uint16 unityNote, short fineTune, double atten) {
	usUnityNote = unityNote;
	sFineTune = fineTune;
	attenuation = atten;
}

//  *********
//  SynthWave
//  *********

void SynthWave::ConvertTo16bitSigned() {
	if (wBitsPerSample == 8) {
		this->wBitsPerSample = 16;
		this->wBlockAlign = 16 / 8 * this->wChannels;
		this->dwAveBytesPerSec *= 2;

		int16_t *newData = new int16_t[this->dataSize];
		for (unsigned int i = 0; i < this->dataSize; i++)
			newData[i] = ((int16_t) this->data[i] - 128) << 8;
		delete[] this->data;
		this->data = (uint8 *) newData;
		this->dataSize *= 2;
	}
}

SynthWave::~SynthWave() {
	delete sampinfo;
	delete[] data;
}

SynthSampInfo *SynthWave::AddSampInfo(void) {
	sampinfo = new SynthSampInfo();
	return sampinfo;
}
