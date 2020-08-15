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

#include "common/debug.h"
#include "common/scummsys.h"
#include "vab.h"
#include "psxspu.h"

using namespace std;

Vab::Vab(RawFile *file, uint32 offset) : VGMInstrSet(file, offset) {}

Vab::~Vab() {}

bool Vab::GetHeaderInfo() {
	uint32 nEndOffset = GetEndOffset();
	uint32 nMaxLength = nEndOffset - _dwOffset;

	if (nMaxLength < 0x20) {
		return false;
	}

	_name = "VAB";

	VGMHeader *vabHdr = AddHeader(_dwOffset, 0x20, "VAB Header");
	vabHdr->AddSimpleItem(_dwOffset + 0x00, 4, "ID");
	vabHdr->AddSimpleItem(_dwOffset + 0x04, 4, "Version");
	vabHdr->AddSimpleItem(_dwOffset + 0x08, 4, "VAB ID");
	vabHdr->AddSimpleItem(_dwOffset + 0x0c, 4, "Total Size");
	vabHdr->AddSimpleItem(_dwOffset + 0x10, 2, "Reserved");
	vabHdr->AddSimpleItem(_dwOffset + 0x12, 2, "Number of Programs");
	vabHdr->AddSimpleItem(_dwOffset + 0x14, 2, "Number of Tones");
	vabHdr->AddSimpleItem(_dwOffset + 0x16, 2, "Number of VAGs");
	vabHdr->AddSimpleItem(_dwOffset + 0x18, 1, "Master Volume");
	vabHdr->AddSimpleItem(_dwOffset + 0x19, 1, "Master Pan");
	vabHdr->AddSimpleItem(_dwOffset + 0x1a, 1, "Bank Attributes 1");
	vabHdr->AddSimpleItem(_dwOffset + 0x1b, 1, "Bank Attributes 2");
	vabHdr->AddSimpleItem(_dwOffset + 0x1c, 4, "Reserved");

	return true;
}

bool Vab::GetInstrPointers() {
	uint32 nEndOffset = GetEndOffset();

	uint32 offProgs = _dwOffset + 0x20;
	uint32 offToneAttrs = offProgs + (16 * 128);

	uint16 numPrograms = GetShort(_dwOffset + 0x12);
	uint16 numVAGs = GetShort(_dwOffset + 0x16);

	uint32 offVAGOffsets = offToneAttrs + (32 * 16 * numPrograms);

	VGMHeader *progsHdr = AddHeader(offProgs, 16 * 128, "Program Table");
	VGMHeader *toneAttrsHdr = AddHeader(offToneAttrs, 32 * 16, "Tone Attributes Table");

	if (numPrograms > 128) {
		debug("Too many programs %x, offset %x", numPrograms, _dwOffset);
		return false;
	}
	if (numVAGs > 255) {
		debug("Too many VAGs %x, offset %x", numVAGs, _dwOffset);
		return false;
	}

	// Load each instruments.
	//
	// Rule 1. Valid instrument pointers are not always sequentially located from 0 to (numProgs -
	// 1). Number of tones can be 0. That's an empty instrument. We need to ignore it. See Clock
	// Tower PSF for example.
	//
	// Rule 2. Do not load programs more than number of programs. Even if a program table value is
	// provided. Otherwise an out-of-order access can be caused in Tone Attributes Table. See the
	// swimming event BGM of Aitakute... ~your smiles in my heart~ for example. (github issue #115)
	uint32 numProgramsLoaded = 0;
	for (uint32 progIndex = 0; progIndex < 128 && numProgramsLoaded < numPrograms; progIndex++) {
		uint32 offCurrProg = offProgs + (progIndex * 16);
		uint32 offCurrToneAttrs = offToneAttrs + (uint32) (_aInstrs.size() * 32 * 16);

		if (offCurrToneAttrs + (32 * 16) > nEndOffset) {
			break;
		}

		uint8 numTonesPerInstr = GetByte(offCurrProg);
		if (numTonesPerInstr > 32) {
			debug("Program %x contains too many tones (%d)", progIndex, numTonesPerInstr);
		} else if (numTonesPerInstr != 0) {
			VabInstr *newInstr = new VabInstr(this, offCurrToneAttrs, 0x20 * 16, 0, progIndex);
			_aInstrs.push_back(newInstr);
			newInstr->_tones = GetByte(offCurrProg + 0);

			VGMHeader *progHdr = progsHdr->AddHeader(offCurrProg, 0x10, "Program");
			progHdr->AddSimpleItem(offCurrProg + 0x00, 1, "Number of Tones");
			progHdr->AddSimpleItem(offCurrProg + 0x01, 1, "Volume");
			progHdr->AddSimpleItem(offCurrProg + 0x02, 1, "Priority");
			progHdr->AddSimpleItem(offCurrProg + 0x03, 1, "Mode");
			progHdr->AddSimpleItem(offCurrProg + 0x04, 1, "Pan");
			progHdr->AddSimpleItem(offCurrProg + 0x05, 1, "Reserved");
			progHdr->AddSimpleItem(offCurrProg + 0x06, 2, "Attribute");
			progHdr->AddSimpleItem(offCurrProg + 0x08, 4, "Reserved");
			progHdr->AddSimpleItem(offCurrProg + 0x0c, 4, "Reserved");

			newInstr->_masterVol = GetByte(offCurrProg + 0x01);

			toneAttrsHdr->_unLength = offCurrToneAttrs + (32 * 16) - offToneAttrs;

			numProgramsLoaded++;
		}
	}

	if ((offVAGOffsets + 2 * 256) <= nEndOffset) {
		char name[256];
		Common::Array<SizeOffsetPair> vagLocations;
		uint32 totalVAGSize = 0;
		VGMHeader *vagOffsetHdr = AddHeader(offVAGOffsets, 2 * 256, "VAG Pointer Table");

		uint32 vagStartOffset = GetShort(offVAGOffsets) * 8;
		vagOffsetHdr->AddSimpleItem(offVAGOffsets, 2, "VAG Size /8 #0");
		totalVAGSize = vagStartOffset;

		for (uint32 i = 0; i < numVAGs; i++) {
			uint32 vagOffset;
			uint32 vagSize;

			if (i == 0) {
				vagOffset = vagStartOffset;
				vagSize = GetShort(offVAGOffsets + (i + 1) * 2) * 8;
			} else {
				vagOffset = vagStartOffset + vagLocations[i - 1].offset + vagLocations[i - 1].size;
				vagSize = GetShort(offVAGOffsets + (i + 1) * 2) * 8;
			}

			snprintf(name, sizeof(name), "VAG Size /8 #%u", i + 1);
			vagOffsetHdr->AddSimpleItem(offVAGOffsets + (i + 1) * 2, 2, name);

			if (vagOffset + vagSize <= nEndOffset) {
				vagLocations.push_back(SizeOffsetPair(vagOffset, vagSize));
				totalVAGSize += vagSize;
			} else {
				debug("VAG #%d at %x with size %x) is invalid", i + 1, vagOffset, vagSize);
			}
		}
		_unLength = (offVAGOffsets + 2 * 256) - _dwOffset;

		// single VAB file?
		uint32 offVAGs = offVAGOffsets + 2 * 256;
		if (_dwOffset == 0 && vagLocations.size() != 0) {
			// load samples as well
			PSXSampColl *newSampColl =
					new PSXSampColl(this, offVAGs, totalVAGSize, vagLocations);
			if (newSampColl->LoadVGMFile()) {
				this->_sampColl = newSampColl;
			} else {
				delete newSampColl;
			}
		}
	}

	return true;
}

// ********
// VabInstr
// ********

VabInstr::VabInstr(VGMInstrSet *instrSet, uint32 offset, uint32 length, uint32 theBank,
				   uint32 theInstrNum, const Common::String &name)
		: VGMInstr(instrSet, offset, length, theBank, theInstrNum, name), _tones(0), _masterVol(127) {}

VabInstr::~VabInstr() {}

bool VabInstr::LoadInstr() {
	int8 numRgns = _tones;
	for (int i = 0; i < numRgns; i++) {
		VabRgn *rgn = new VabRgn(this, _dwOffset + i * 0x20);
		if (!rgn->LoadRgn()) {
			delete rgn;
			return false;
		}
		_aRgns.push_back(rgn);
	}
	return true;
}

// ******
// VabRgn
// ******

VabRgn::VabRgn(VabInstr *instr, uint32 offset) : _ADSR1(0), _ADSR2(0), VGMRgn(instr, offset) {}

bool VabRgn::LoadRgn() {
	VabInstr *instr = (VabInstr *) _parInstr;
	_unLength = 0x20;

	AddGeneralItem(_dwOffset, 1, "Priority");
	AddGeneralItem(_dwOffset + 1, 1, "Mode (use reverb?)");
	AddVolume((GetByte(_dwOffset + 2) * instr->_masterVol) / (127.0 * 127.0), _dwOffset + 2, 1);
	AddPan(GetByte(_dwOffset + 3), _dwOffset + 3);
	AddUnityKey(GetByte(_dwOffset + 4), _dwOffset + 4);
	AddGeneralItem(_dwOffset + 5, 1, "Pitch Tune");
	AddKeyLow(GetByte(_dwOffset + 6), _dwOffset + 6);
	AddKeyHigh(GetByte(_dwOffset + 7), _dwOffset + 7);
	AddGeneralItem(_dwOffset + 8, 1, "Vibrato Width");
	AddGeneralItem(_dwOffset + 9, 1, "Vibrato Time");
	AddGeneralItem(_dwOffset + 10, 1, "Portamento Width");
	AddGeneralItem(_dwOffset + 11, 1, "Portamento Holding Time");
	AddGeneralItem(_dwOffset + 12, 1, "Pitch Bend Min");
	AddGeneralItem(_dwOffset + 13, 1, "Pitch Bend Max");
	AddGeneralItem(_dwOffset + 14, 1, "Reserved");
	AddGeneralItem(_dwOffset + 15, 1, "Reserved");
	AddGeneralItem(_dwOffset + 16, 2, "ADSR1");
	AddGeneralItem(_dwOffset + 18, 2, "ADSR2");
	AddGeneralItem(_dwOffset + 20, 2, "Parent Program");
	AddSampNum(GetShort(_dwOffset + 22) - 1, _dwOffset + 22, 2);
	AddGeneralItem(_dwOffset + 24, 2, "Reserved");
	AddGeneralItem(_dwOffset + 26, 2, "Reserved");
	AddGeneralItem(_dwOffset + 28, 2, "Reserved");
	AddGeneralItem(_dwOffset + 30, 2, "Reserved");
	_ADSR1 = GetShort(_dwOffset + 16);
	_ADSR2 = GetShort(_dwOffset + 18);
	if ((int) _sampNum < 0)
		_sampNum = 0;

	if (_keyLow > _keyHigh) {
		debug("Low key higher than high key %d > %d (at %x)", _keyLow, _keyHigh, _dwOffset);
		return false;
	}

	// gocha: AFAIK, the valid range of pitch is 0-127. It must not be negative.
	// If it exceeds 127, driver clips the value and it will become 127. (In Hokuto no Ken, at
	// least) I am not sure if the interpretation of this value depends on a driver or VAB version.
	// The following code takes the byte as signed, since it could be a typical extended
	// implementation.
	int8 ft = (int8) GetByte(_dwOffset + 5);
	double cents = ft * 100.0 / 128.0;
	SetFineTune((int16) cents);

	PSXConvADSR<VabRgn>(this, _ADSR1, _ADSR2, false);
	return true;
}
