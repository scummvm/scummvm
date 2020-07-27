/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
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
