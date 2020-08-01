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
#include "vgminstrset.h"
#include "vgmsamp.h"

using namespace std;

// ***********
// VGMInstrSet
// ***********

VGMInstrSet::VGMInstrSet(RawFile *file, uint32 offset, uint32 length, Common::String theName,
						 VGMSampColl *theSampColl)
		: VGMFile(file, offset, length, theName),
		  _sampColl(theSampColl) {
	AddContainer<VGMInstr>(_aInstrs);
}

VGMInstrSet::~VGMInstrSet() {
	DeleteVect<VGMInstr>(_aInstrs);
	delete _sampColl;
}

bool VGMInstrSet::Load() {
	if (!GetHeaderInfo())
		return false;
	if (!GetInstrPointers())
		return false;
	if (!LoadInstrs())
		return false;

	if (_aInstrs.size() == 0)
		return false;

	if (_sampColl != NULL) {
		if (!_sampColl->Load()) {
			error("Failed to load VGMSampColl");
		}
	}

	return true;
}

bool VGMInstrSet::GetHeaderInfo() {
	return true;
}

bool VGMInstrSet::GetInstrPointers() {
	return true;
}

bool VGMInstrSet::LoadInstrs() {
	size_t nInstrs = _aInstrs.size();
	for (size_t i = 0; i < nInstrs; i++) {
		if (!_aInstrs[i]->LoadInstr())
			return false;
	}
	return true;
}

// ********
// VGMInstr
// ********

VGMInstr::VGMInstr(VGMInstrSet *instrSet, uint32 offset, uint32 length, uint32 theBank,
				   uint32 theInstrNum, const Common::String &name)
		: VGMContainerItem(instrSet, offset, length, name),
		  _parInstrSet(instrSet),
		  _bank(theBank),
		  _instrNum(theInstrNum) {
	AddContainer<VGMRgn>(_aRgns);
}

VGMInstr::~VGMInstr() {
	DeleteVect<VGMRgn>(_aRgns);
}

bool VGMInstr::LoadInstr() {
	return true;
}
