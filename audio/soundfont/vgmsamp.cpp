/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */

#include "vgmsamp.h"

// *******
// VGMSamp
// *******

VGMSamp::VGMSamp(VGMSampColl *sampColl, uint32 offset, uint32 length, uint32 dataOffset,
				 uint32 dataLen, uint8 nChannels, uint16 theBPS, uint32 theRate,
				 Common::String theName)
		: parSampColl(sampColl),
		  sampName(theName),
		  VGMItem(sampColl->_vgmfile, offset, length),
		  dataOff(dataOffset),
		  dataLength(dataLen),
		  bps(theBPS),
		  rate(theRate),
		  ulUncompressedSize(0),
		  channels(nChannels),
		  pan(0),
		  unityKey(-1),
		  fineTune(0),
		  volume(-1),
		  waveType(WT_UNDEFINED),
		  bPSXLoopInfoPrioritizing(false) {
	_name = sampName;  // I would do this in the initialization list, but VGMItem()
	// constructor is called before sampName is initialized,
	// so data() ends up returning a bad pointer
}

VGMSamp::~VGMSamp() {}

double VGMSamp::GetCompressionRatio() {
	return 1.0;
}

// ***********
// VGMSampColl
// ***********

VGMSampColl::VGMSampColl(RawFile *rawfile, uint32 offset, uint32 length,
						 Common::String theName)
		: VGMFile(rawfile, offset, length, theName),
		  parInstrSet(NULL),
		  bLoaded(false),
		  sampDataOffset(0) {
	AddContainer<VGMSamp>(samples);
}

VGMSampColl::VGMSampColl(RawFile *rawfile, VGMInstrSet *instrset,
						 uint32 offset, uint32 length, Common::String theName)
		: VGMFile(rawfile, offset, length, theName),
		  parInstrSet(instrset),
		  bLoaded(false),
		  sampDataOffset(0) {
	AddContainer<VGMSamp>(samples);
}

VGMSampColl::~VGMSampColl(void) {
	DeleteVect<VGMSamp>(samples);
}

bool VGMSampColl::Load() {
	if (bLoaded)
		return true;
	if (!GetHeaderInfo())
		return false;
	if (!GetSampleInfo())
		return false;

	if (samples.size() == 0)
		return false;

	if (_unLength == 0) {
		for (Common::Array<VGMSamp *>::iterator itr = samples.begin(); itr != samples.end(); ++itr) {
			VGMSamp *samp = (*itr);

			// Some formats can have negative sample offset
			// For example, Konami's SNES format and Hudson's SNES format
			// TODO: Fix negative sample offset without breaking instrument
			// assert(dwOffset <= samp->dwOffset);

			// if (dwOffset > samp->dwOffset)
			//{
			//	unLength += samp->dwOffset - dwOffset;
			//	dwOffset = samp->dwOffset;
			//}

			if (_dwOffset + _unLength < samp->_dwOffset + samp->_unLength) {
				_unLength = (samp->_dwOffset + samp->_unLength) - _dwOffset;
			}
		}
	}

	bLoaded = true;
	return true;
}
