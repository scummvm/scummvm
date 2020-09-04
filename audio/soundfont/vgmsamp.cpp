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

#include "vgmsamp.h"

// *******
// VGMSamp
// *******

VGMSamp::VGMSamp(VGMSampColl *sampColl, uint32 offset, uint32 length, uint32 dataOffset,
				 uint32 dataLen, uint8 nChannels, uint16 theBPS, uint32 theRate,
				 Common::String theName)
		: _parSampColl(sampColl),
		  _sampName(theName),
		  VGMItem(sampColl->_vgmfile, offset, length),
		  _dataOff(dataOffset),
		  _dataLength(dataLen),
		  _bps(theBPS),
		  _rate(theRate),
		  _ulUncompressedSize(0),
		  _channels(nChannels),
		  _unityKey(-1),
		  _fineTune(0),
		  _volume(-1),
		  _waveType(WT_UNDEFINED),
		  _bPSXLoopInfoPrioritizing(false) {
	_name = _sampName;  // I would do this in the initialization list, but VGMItem()
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
		  _parInstrSet(NULL),
		  _bLoaded(false),
		  _sampDataOffset(0) {
	AddContainer<VGMSamp>(_samples);
}

VGMSampColl::VGMSampColl(RawFile *rawfile, VGMInstrSet *instrset,
						 uint32 offset, uint32 length, Common::String theName)
		: VGMFile(rawfile, offset, length, theName),
		  _parInstrSet(instrset),
		  _bLoaded(false),
		  _sampDataOffset(0) {
	AddContainer<VGMSamp>(_samples);
}

VGMSampColl::~VGMSampColl(void) {
	DeleteVect<VGMSamp>(_samples);
}

bool VGMSampColl::Load() {
	if (_bLoaded)
		return true;
	if (!GetHeaderInfo())
		return false;
	if (!GetSampleInfo())
		return false;

	if (_samples.size() == 0)
		return false;

	if (_unLength == 0) {
		for (Common::Array<VGMSamp *>::iterator itr = _samples.begin(); itr != _samples.end(); ++itr) {
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

	_bLoaded = true;
	return true;
}
