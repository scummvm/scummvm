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

#include "common.h"
#include "vgmitem.h"
#include "vgminstrset.h"

using namespace std;

VGMItem::VGMItem() : _dwOffset(0), _unLength(0), _vgmfile(nullptr) {}

VGMItem::VGMItem(VGMFile *thevgmfile, uint32 theOffset, uint32 theLength, const Common::String theName)
		: _vgmfile(thevgmfile),
		  _name(theName),
		  _dwOffset(theOffset),
		  _unLength(theLength) {}

VGMItem::~VGMItem() {}

RawFile *VGMItem::GetRawFile() {
	return _vgmfile->_rawfile;
}

uint32 VGMItem::GetBytes(uint32 nIndex, uint32 nCount, void *pBuffer) {
	return _vgmfile->GetBytes(nIndex, nCount, pBuffer);
}

uint8 VGMItem::GetByte(uint32 offset) {
	return _vgmfile->GetByte(offset);
}

uint16 VGMItem::GetShort(uint32 offset) {
	return _vgmfile->GetShort(offset);
}

//  ****************
//  VGMContainerItem
//  ****************

VGMContainerItem::VGMContainerItem() : VGMItem() {
	AddContainer(_headers);
	AddContainer(_localitems);
}

VGMContainerItem::VGMContainerItem(VGMFile *thevgmfile, uint32 theOffset, uint32 theLength,
								   const Common::String theName)
		: VGMItem(thevgmfile, theOffset, theLength, theName) {
	AddContainer(_headers);
	AddContainer(_localitems);
}

VGMContainerItem::~VGMContainerItem() {
	DeleteVect(_headers);
	DeleteVect(_localitems);
}

VGMHeader *VGMContainerItem::AddHeader(uint32 offset, uint32 length, const Common::String &name) {
	VGMHeader *header = new VGMHeader(this, offset, length, name);
	_headers.push_back(header);
	return header;
}

void VGMContainerItem::AddSimpleItem(uint32 offset, uint32 length, const Common::String &name) {
	_localitems.push_back(new VGMItem(this->_vgmfile, offset, length, name));
}

// *********
// VGMFile
// *********

VGMFile::VGMFile(RawFile *theRawFile, uint32 offset,
				 uint32 length, Common::String theName)
		: VGMContainerItem(this, offset, length, theName),
		  _rawfile(theRawFile) {}

VGMFile::~VGMFile(void) {}

bool VGMFile::LoadVGMFile() {
	bool val = Load();
	if (!val)
		return false;

	return val;
}

// These functions are common to all VGMItems, but no reason to refer to vgmfile
// or call GetRawFile() if the item itself is a VGMFile
RawFile *VGMFile::GetRawFile() {
	return _rawfile;
}

uint32 VGMFile::GetBytes(uint32 nIndex, uint32 nCount, void *pBuffer) {
	// if unLength != 0, verify that we're within the bounds of the file, and truncate num read
	// bytes to end of file
	if (_unLength != 0) {
		uint32 endOff = _dwOffset + _unLength;
		assert(nIndex >= _dwOffset && nIndex < endOff);
		if (nIndex + nCount > endOff)
			nCount = endOff - nIndex;
	}

	return _rawfile->GetBytes(nIndex, nCount, pBuffer);
}

// *********
// VGMHeader
// *********

VGMHeader::VGMHeader(VGMItem *parItem, uint32 offset, uint32 length, const Common::String &name)
		: VGMContainerItem(parItem->_vgmfile, offset, length, name) {}

VGMHeader::~VGMHeader() {}

// ******
// VGMRgn
// ******

VGMRgn::VGMRgn(VGMInstr *instr, uint32 offset, uint32 length, Common::String name)
		: VGMContainerItem(instr->_parInstrSet, offset, length, name),
		  _keyLow(0),
		  _keyHigh(127),
		  _velLow(0),
		  _velHigh(127),
		  _unityKey(-1),
		  _fineTune(0),
		  _sampNum(0),
		  _sampCollPtr(nullptr),
		  _volume(-1),
		  _pan(0.5),
		  _attack_time(0),
		  _decay_time(0),
		  _release_time(0),
		  _sustain_level(-1),
		  _sustain_time(0),
		  _attack_transform(no_transform),
		  _release_transform(no_transform),
		  _parInstr(instr) {
	AddContainer<VGMRgnItem>(_items);
}

VGMRgn::~VGMRgn() {
	DeleteVect<VGMRgnItem>(_items);
}

void VGMRgn::SetPan(uint8 p) {
	if (p == 127) {
		_pan = 1.0;
	} else if (p == 0) {
		_pan = 0;
	} else if (p == 64) {
		_pan = 0.5;
	} else {
		_pan = _pan / 127.0;
	}
}

void VGMRgn::AddGeneralItem(uint32 offset, uint32 length, const Common::String &name) {
	_items.push_back(new VGMRgnItem(this, VGMRgnItem::RIT_GENERIC, offset, length, name));
}

// assumes pan is given as 0-127 value, converts it to our double -1.0 to 1.0 format
void VGMRgn::AddPan(uint8 p, uint32 offset, uint32 length) {
	SetPan(p);
	_items.push_back(new VGMRgnItem(this, VGMRgnItem::RIT_PAN, offset, length, "Pan"));
}

void VGMRgn::AddVolume(double vol, uint32 offset, uint32 length) {
	_volume = vol;
	_items.push_back(new VGMRgnItem(this, VGMRgnItem::RIT_VOL, offset, length, "Volume"));
}

void VGMRgn::AddUnityKey(int8 uk, uint32 offset, uint32 length) {
	this->_unityKey = uk;
	_items.push_back(new VGMRgnItem(this, VGMRgnItem::RIT_UNITYKEY, offset, length, "Unity Key"));
}

void VGMRgn::AddKeyLow(uint8 kl, uint32 offset, uint32 length) {
	_keyLow = kl;
	_items.push_back(
			new VGMRgnItem(this, VGMRgnItem::RIT_KEYLOW, offset, length, "Note Range: Low Key"));
}

void VGMRgn::AddKeyHigh(uint8 kh, uint32 offset, uint32 length) {
	_keyHigh = kh;
	_items.push_back(
			new VGMRgnItem(this, VGMRgnItem::RIT_KEYHIGH, offset, length, "Note Range: High Key"));
}

void VGMRgn::AddSampNum(int sn, uint32 offset, uint32 length) {
	_sampNum = sn;
	_items.push_back(new VGMRgnItem(this, VGMRgnItem::RIT_SAMPNUM, offset, length, "Sample Number"));
}

// **********
// VGMRgnItem
// **********

VGMRgnItem::VGMRgnItem(VGMRgn *rgn, RgnItemType theType, uint32 offset, uint32 length,
					   const Common::String &name)
		: VGMItem(rgn->_vgmfile, offset, length, name), _type(theType) {}
