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

#include "common/scummsys.h"
#include "common/str.h"
#include "sf2file.h"
#include "synthfile.h"

using namespace std;

// Convert a pan value where 0 = left 0.5 = center and 1 = right to
// 0.1% units where -50% = left 0 = center 50% = right (shared by DLS and SF2)
long ConvertPercentPanTo10thPercentUnits(double percentPan) {
	return round(percentPan * 1000) - 500;
}

double SecondsToTimecents(double secs) {
	return log(secs) / log((double) 2) * 1200;
}

SF2InfoListChunk::SF2InfoListChunk(Common::String name) : LISTChunk("INFO") {
	// Add the child info chunks
	Chunk *ifilCk = new Chunk("ifil");
	ifilCk->_size = sizeof(sfVersionTag);
	ifilCk->_data = new uint8[ifilCk->_size];
	sfVersionTag versionTag;  // soundfont version 2.01
	versionTag.wMajor = 2;
	versionTag.wMinor = 1;
	versionTag.write(ifilCk->_data);
	AddChildChunk(ifilCk);
	AddChildChunk(new SF2StringChunk("isng", "EMU8000"));
	AddChildChunk(new SF2StringChunk("INAM", name));
	AddChildChunk(new SF2StringChunk("ISFT", Common::String("ScummVM")));
}

//  *******
//  SF2File
//  *******

SF2File::SF2File(SynthFile *synthfile) : RiffFile(synthfile->_name, "sfbk") {
	//***********
	// INFO chunk
	//***********
	AddChildChunk(new SF2InfoListChunk(_name));

	// sdta chunk and its child smpl chunk containing all samples
	LISTChunk *sdtaCk = new LISTChunk("sdta");
	Chunk *smplCk = new Chunk("smpl");

	// Concatanate all of the samples together and add the result to the smpl chunk data
	size_t numWaves = synthfile->_vWaves.size();
	smplCk->_size = 0;
	for (size_t i = 0; i < numWaves; i++) {
		SynthWave *wave = synthfile->_vWaves[i];
		wave->ConvertTo16bitSigned();
		smplCk->_size +=
				wave->_dataSize + (46 * 2);  // plus the 46 padding samples required by sf2 spec
	}
	smplCk->_data = new uint8[smplCk->_size];
	uint32 bufPtr = 0;
	for (size_t i = 0; i < numWaves; i++) {
		SynthWave *wave = synthfile->_vWaves[i];

		memcpy(smplCk->_data + bufPtr, wave->_data, wave->_dataSize);
		memset(smplCk->_data + bufPtr + wave->_dataSize, 0, 46 * 2);
		bufPtr += wave->_dataSize + (46 * 2);  // plus the 46 padding samples required by sf2 spec
	}

	sdtaCk->AddChildChunk(smplCk);
	this->AddChildChunk(sdtaCk);

	//***********
	// pdta chunk
	//***********

	LISTChunk *pdtaCk = new LISTChunk("pdta");

	//***********
	// phdr chunk
	//***********
	Chunk *phdrCk = new Chunk("phdr");
	size_t numInstrs = synthfile->_vInstrs.size();
	phdrCk->_size = (uint32) ((numInstrs + 1) * sizeof(sfPresetHeader));
	phdrCk->_data = new uint8[phdrCk->_size];

	for (size_t i = 0; i < numInstrs; i++) {
		SynthInstr *instr = synthfile->_vInstrs[i];

		sfPresetHeader presetHdr;
		memset(&presetHdr, 0, sizeof(sfPresetHeader));
		memcpy(presetHdr.achPresetName, instr->_name.c_str(),
			   MIN((unsigned long) instr->_name.size(), (unsigned long) 20));
		presetHdr.wPreset = (uint16) instr->_ulInstrument;

		// Despite being a 16-bit value, SF2 only supports banks up to 127. Since
		// it's pretty common to have either MSB or LSB be 0, we'll use whatever
		// one is not zero, with preference for MSB.
		uint16 bank16 = (uint16) instr->_ulBank;

		if ((bank16 & 0xFF00) == 0) {
			presetHdr.wBank = bank16 & 0x7F;
		} else {
			presetHdr.wBank = (bank16 >> 8) & 0x7F;
		}
		presetHdr.wPresetBagNdx = (uint16) i;
		presetHdr.dwLibrary = 0;
		presetHdr.dwGenre = 0;
		presetHdr.dwMorphology = 0;

		presetHdr.write(phdrCk->_data + (i * sizeof(sfPresetHeader)));
	}
	//  add terminal sfPresetBag
	sfPresetHeader presetHdr;
	memset(&presetHdr, 0, sizeof(sfPresetHeader));
	presetHdr.wPresetBagNdx = (uint16) numInstrs;
	presetHdr.write(phdrCk->_data + (numInstrs * sizeof(sfPresetHeader)));
	pdtaCk->AddChildChunk(phdrCk);

	//***********
	// pbag chunk
	//***********
	Chunk *pbagCk = new Chunk("pbag");
	const size_t ITEMS_IN_PGEN = 2;
	pbagCk->_size = (uint32) ((numInstrs + 1) * sizeof(sfPresetBag));
	pbagCk->_data = new uint8[pbagCk->_size];
	for (size_t i = 0; i < numInstrs; i++) {
		sfPresetBag presetBag;
		memset(&presetBag, 0, sizeof(sfPresetBag));
		presetBag.wGenNdx = (uint16) (i * ITEMS_IN_PGEN);
		presetBag.wModNdx = 0;

		presetBag.write(pbagCk->_data + (i * sizeof(sfPresetBag)));
	}
	//  add terminal sfPresetBag
	sfPresetBag presetBag;
	memset(&presetBag, 0, sizeof(sfPresetBag));
	presetBag.wGenNdx = (uint16) (numInstrs * ITEMS_IN_PGEN);
	presetBag.write(pbagCk->_data + (numInstrs * sizeof(sfPresetBag)));
	pdtaCk->AddChildChunk(pbagCk);

	//***********
	// pmod chunk
	//***********
	Chunk *pmodCk = new Chunk("pmod");
	//  create the terminal field
	sfModList modList;
	memset(&modList, 0, sizeof(sfModList));
	pmodCk->SetData(&modList, sizeof(sfModList));
	// modList.sfModSrcOper = cc1_Mod;
	// modList.sfModDestOper = startAddrsOffset;
	// modList.modAmount = 0;
	// modList.sfModAmtSrcOper = cc1_Mod;
	// modList.sfModTransOper = linear;
	pdtaCk->AddChildChunk(pmodCk);

	//***********
	// pgen chunk
	//***********
	Chunk *pgenCk = new Chunk("pgen");
	// pgenCk->size = (synthfile->vInstrs.size()+1) * sizeof(sfGenList);
	pgenCk->_size = (uint32) ((synthfile->_vInstrs.size() * sizeof(sfGenList) * ITEMS_IN_PGEN) +
							  sizeof(sfGenList));
	pgenCk->_data = new uint8[pgenCk->_size];
	uint32 dataPtr = 0;
	for (size_t i = 0; i < numInstrs; i++) {
		sfGenList genList;
		memset(&genList, 0, sizeof(sfGenList));

		// reverbEffectsSend
		genList.sfGenOper = reverbEffectsSend;
		genList.genAmount.setShAmount(250);
		genList.write(pgenCk->_data + dataPtr);
		dataPtr += sizeof(sfGenList);

		genList.sfGenOper = instrument;
		genList.genAmount.setwAmount((uint16) i);
		genList.write(pgenCk->_data + dataPtr);
		dataPtr += sizeof(sfGenList);
	}
	//  add terminal sfGenList
	sfGenList genList;
	memset(&genList, 0, sizeof(sfGenList));
	genList.write(pgenCk->_data + dataPtr);

	pdtaCk->AddChildChunk(pgenCk);

	//***********
	// inst chunk
	//***********
	Chunk *instCk = new Chunk("inst");
	instCk->_size = (uint32) ((synthfile->_vInstrs.size() + 1) * sizeof(sfInst));
	instCk->_data = new uint8[instCk->_size];
	size_t rgnCounter = 0;
	for (size_t i = 0; i < numInstrs; i++) {
		SynthInstr *instr = synthfile->_vInstrs[i];

		sfInst inst;
		memset(&inst, 0, sizeof(sfInst));
		memcpy(inst.achInstName, instr->_name.c_str(),
			   MIN((unsigned long) instr->_name.size(), (unsigned long) 20));
		inst.wInstBagNdx = (uint16) rgnCounter;
		rgnCounter += instr->_vRgns.size();

		inst.write(instCk->_data + (i * sizeof(sfInst)));
	}
	//  add terminal sfInst
	sfInst inst;
	memset(&inst, 0, sizeof(sfInst));
	inst.wInstBagNdx = (uint16) rgnCounter;
	inst.write(instCk->_data + (numInstrs * sizeof(sfInst)));
	pdtaCk->AddChildChunk(instCk);

	//***********
	// ibag chunk - stores all zones (regions) for instruments
	//***********
	Chunk *ibagCk = new Chunk("ibag");

	size_t totalNumRgns = 0;
	for (size_t i = 0; i < numInstrs; i++)
		totalNumRgns += synthfile->_vInstrs[i]->_vRgns.size();

	ibagCk->_size = (uint32) ((totalNumRgns + 1) * sizeof(sfInstBag));
	ibagCk->_data = new uint8[ibagCk->_size];

	rgnCounter = 0;
	int instGenCounter = 0;
	for (size_t i = 0; i < numInstrs; i++) {
		SynthInstr *instr = synthfile->_vInstrs[i];

		size_t numRgns = instr->_vRgns.size();
		for (size_t j = 0; j < numRgns; j++) {
			sfInstBag instBag;
			memset(&instBag, 0, sizeof(sfInstBag));
			instBag.wInstGenNdx = instGenCounter;
			instGenCounter += 11;
			instBag.wInstModNdx = 0;

			instBag.write(ibagCk->_data + (rgnCounter++ * sizeof(sfInstBag)));
		}
	}
	//  add terminal sfInstBag
	sfInstBag instBag;
	memset(&instBag, 0, sizeof(sfInstBag));
	instBag.wInstGenNdx = instGenCounter;
	instBag.wInstModNdx = 0;
	instBag.write(ibagCk->_data + (rgnCounter * sizeof(sfInstBag)));
	pdtaCk->AddChildChunk(ibagCk);

	//***********
	// imod chunk
	//***********
	Chunk *imodCk = new Chunk("imod");
	//  create the terminal field
	memset(&modList, 0, sizeof(sfModList));
	imodCk->SetData(&modList, sizeof(sfModList));
	pdtaCk->AddChildChunk(imodCk);

	//***********
	// igen chunk
	//***********
	Chunk *igenCk = new Chunk("igen");
	igenCk->_size = (uint32) ((totalNumRgns * sizeof(sfInstGenList) * 11) + sizeof(sfInstGenList));
	igenCk->_data = new uint8[igenCk->_size];
	dataPtr = 0;
	for (size_t i = 0; i < numInstrs; i++) {
		SynthInstr *instr = synthfile->_vInstrs[i];

		size_t numRgns = instr->_vRgns.size();
		for (size_t j = 0; j < numRgns; j++) {
			SynthRgn *rgn = instr->_vRgns[j];

			sfInstGenList instGenList;
			// Key range - (if exists) this must be the first chunk
			instGenList.sfGenOper = keyRange;
			instGenList.genAmount.setRangeLo((uint8) rgn->_usKeyLow);
			instGenList.genAmount.setRangeHi((uint8) rgn->_usKeyHigh);
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			if (rgn->_usVelHigh)  // 0 means 'not set', fixes TriAce instruments
			{
				// Velocity range (if exists) this must be the next chunk
				instGenList.sfGenOper = velRange;
				instGenList.genAmount.setRangeLo((uint8) rgn->_usVelLow);
				instGenList.genAmount.setRangeHi((uint8) rgn->_usVelHigh);
				instGenList.write(igenCk->_data + dataPtr);
				dataPtr += sizeof(sfInstGenList);
			}

			// initialAttenuation
			instGenList.sfGenOper = initialAttenuation;
			instGenList.genAmount.setShAmount((int16) (rgn->_sampinfo->_attenuation * 10));
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			// pan
			instGenList.sfGenOper = pan;
			instGenList.genAmount.setShAmount(
					(int16) ConvertPercentPanTo10thPercentUnits(rgn->_art->_pan));
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			// sampleModes
			instGenList.sfGenOper = sampleModes;
			instGenList.genAmount.setwAmount(rgn->_sampinfo->_cSampleLoops);
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			// overridingRootKey
			instGenList.sfGenOper = overridingRootKey;
			instGenList.genAmount.setwAmount(rgn->_sampinfo->_usUnityNote);
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			// attackVolEnv
			instGenList.sfGenOper = attackVolEnv;
			instGenList.genAmount.setShAmount(
					(rgn->_art->_attack_time == 0)
					? -32768
					: round(SecondsToTimecents(rgn->_art->_attack_time)));
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			// decayVolEnv
			instGenList.sfGenOper = decayVolEnv;
			instGenList.genAmount.setShAmount(
					(rgn->_art->_decay_time == 0) ? -32768
												  : round(SecondsToTimecents(rgn->_art->_decay_time)));
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			// sustainVolEnv
			instGenList.sfGenOper = sustainVolEnv;
			if (rgn->_art->_sustain_lev > 100.0)
				rgn->_art->_sustain_lev = 100.0;
			instGenList.genAmount.setShAmount((int16) (rgn->_art->_sustain_lev * 10));
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			// releaseVolEnv
			instGenList.sfGenOper = releaseVolEnv;
			instGenList.genAmount.setShAmount(
					(rgn->_art->_release_time == 0)
					? -32768
					: round(SecondsToTimecents(rgn->_art->_release_time)));
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			// reverbEffectsSend
			// instGenList.sfGenOper = reverbEffectsSend;
			// instGenList.genAmount.setShAmount(800);
			// memcpy(pgenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			// dataPtr += sizeof(sfInstGenList);

			// sampleID - this is the terminal chunk
			instGenList.sfGenOper = sampleID;
			instGenList.genAmount.setwAmount((uint16) (rgn->_tableIndex));
			instGenList.write(igenCk->_data + dataPtr);
			dataPtr += sizeof(sfInstGenList);

			// int numConnBlocks = rgn->art->vConnBlocks.size();
			// for (int k = 0; k < numConnBlocks; k++)
			//{
			//	SynthConnectionBlock* connBlock = rgn->art->vConnBlocks[k];
			//	connBlock->
			//}
		}
	}
	//  add terminal sfInstBag
	sfInstGenList instGenList;
	memset(&instGenList, 0, sizeof(sfInstGenList));
	instGenList.write(igenCk->_data + dataPtr);
	// memset(ibagCk->data + (totalNumRgns*sizeof(sfInstBag)), 0, sizeof(sfInstBag));
	// igenCk->SetData(&genList, sizeof(sfGenList));
	pdtaCk->AddChildChunk(igenCk);

	//***********
	// shdr chunk
	//***********
	Chunk *shdrCk = new Chunk("shdr");

	size_t numSamps = synthfile->_vWaves.size();
	shdrCk->_size = (uint32) ((numSamps + 1) * sizeof(sfSample));
	shdrCk->_data = new uint8[shdrCk->_size];

	uint32 sampOffset = 0;
	for (size_t i = 0; i < numSamps; i++) {
		SynthWave *wave = synthfile->_vWaves[i];

		sfSample samp;
		memset(&samp, 0, sizeof(sfSample));
		memcpy(samp.achSampleName, wave->_name.c_str(),
			   MIN((unsigned long) wave->_name.size(), (unsigned long) 20));
		samp.dwStart = sampOffset;
		samp.dwEnd = samp.dwStart + (wave->_dataSize / sizeof(uint16));
		sampOffset = samp.dwEnd + 46;  // plus the 46 padding samples required by sf2 spec

		// Search through all regions for an associated sampInfo structure with this sample
		SynthSampInfo *sampInfo = NULL;
		for (size_t j = 0; j < numInstrs; j++) {
			SynthInstr *instr = synthfile->_vInstrs[j];

			size_t numRgns = instr->_vRgns.size();
			for (size_t k = 0; k < numRgns; k++) {
				SynthRgn *rgn = instr->_vRgns[k];
				if (rgn->_tableIndex == i && rgn->_sampinfo != NULL) {
					sampInfo = rgn->_sampinfo;
					break;
				}
			}
			if (sampInfo != NULL)
				break;
		}
		//  If we didn't find a rgn association, then it should be in the SynthWave structure.
		if (sampInfo == NULL)
			sampInfo = wave->_sampinfo;
		assert(sampInfo != NULL);

		samp.dwStartloop = samp.dwStart + sampInfo->_ulLoopStart;
		samp.dwEndloop = samp.dwStartloop + sampInfo->_ulLoopLength;
		samp.dwSampleRate = wave->_dwSamplesPerSec;
		samp.byOriginalKey = (uint8) (sampInfo->_usUnityNote);
		samp.chCorrection = (char) (sampInfo->_sFineTune);
		samp.wSampleLink = 0;
		samp.sfSampleType = monoSample;

		samp.write(shdrCk->_data + (i * sizeof(sfSample)));
	}

	//  add terminal sfSample
	memset(shdrCk->_data + (numSamps * sizeof(sfSample)), 0, sizeof(sfSample));
	pdtaCk->AddChildChunk(shdrCk);

	this->AddChildChunk(pdtaCk);
}

SF2File::~SF2File() {}

const void *SF2File::SaveToMem() {
	uint8 *buf = new uint8[this->GetSize()];
	this->Write(buf);
	return buf;
}
