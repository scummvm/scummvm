/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */

#include <math.h>
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
	sfVersionTag versionTag;  // soundfont version 2.01
	versionTag.wMajor = 2;
	versionTag.wMinor = 1;
	ifilCk->SetData(&versionTag, sizeof(versionTag));
	AddChildChunk(ifilCk);
	AddChildChunk(new SF2StringChunk("isng", "EMU8000"));
	AddChildChunk(new SF2StringChunk("INAM", name));
	AddChildChunk(new SF2StringChunk("ISFT", Common::String("ScummVM")));
}

//  *******
//  SF2File
//  *******

SF2File::SF2File(SynthFile *synthfile) : RiffFile(synthfile->name, "sfbk") {
	//***********
	// INFO chunk
	//***********
	AddChildChunk(new SF2InfoListChunk(name));

	// sdta chunk and its child smpl chunk containing all samples
	LISTChunk *sdtaCk = new LISTChunk("sdta");
	Chunk *smplCk = new Chunk("smpl");

	// Concatanate all of the samples together and add the result to the smpl chunk data
	size_t numWaves = synthfile->vWaves.size();
	smplCk->size = 0;
	for (size_t i = 0; i < numWaves; i++) {
		SynthWave *wave = synthfile->vWaves[i];
		wave->ConvertTo16bitSigned();
		smplCk->size +=
				wave->dataSize + (46 * 2);  // plus the 46 padding samples required by sf2 spec
	}
	smplCk->data = new uint8[smplCk->size];
	uint32 bufPtr = 0;
	for (size_t i = 0; i < numWaves; i++) {
		SynthWave *wave = synthfile->vWaves[i];

		memcpy(smplCk->data + bufPtr, wave->data, wave->dataSize);
		memset(smplCk->data + bufPtr + wave->dataSize, 0, 46 * 2);
		bufPtr += wave->dataSize + (46 * 2);  // plus the 46 padding samples required by sf2 spec
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
	size_t numInstrs = synthfile->vInstrs.size();
	phdrCk->size = (uint32) ((numInstrs + 1) * sizeof(sfPresetHeader));
	phdrCk->data = new uint8[phdrCk->size];

	for (size_t i = 0; i < numInstrs; i++) {
		SynthInstr *instr = synthfile->vInstrs[i];

		sfPresetHeader presetHdr;
		memset(&presetHdr, 0, sizeof(sfPresetHeader));
		memcpy(presetHdr.achPresetName, instr->name.c_str(),
			   MIN((unsigned long) instr->name.size(), (unsigned long) 20));
		presetHdr.wPreset = (uint16) instr->ulInstrument;

		// Despite being a 16-bit value, SF2 only supports banks up to 127. Since
		// it's pretty common to have either MSB or LSB be 0, we'll use whatever
		// one is not zero, with preference for MSB.
		uint16 bank16 = (uint16) instr->ulBank;

		if ((bank16 & 0xFF00) == 0) {
			presetHdr.wBank = bank16 & 0x7F;
		} else {
			presetHdr.wBank = (bank16 >> 8) & 0x7F;
		}
		presetHdr.wPresetBagNdx = (uint16) i;
		presetHdr.dwLibrary = 0;
		presetHdr.dwGenre = 0;
		presetHdr.dwMorphology = 0;

		memcpy(phdrCk->data + (i * sizeof(sfPresetHeader)), &presetHdr, sizeof(sfPresetHeader));
	}
	//  add terminal sfPresetBag
	sfPresetHeader presetHdr;
	memset(&presetHdr, 0, sizeof(sfPresetHeader));
	presetHdr.wPresetBagNdx = (uint16) numInstrs;
	memcpy(phdrCk->data + (numInstrs * sizeof(sfPresetHeader)), &presetHdr, sizeof(sfPresetHeader));
	pdtaCk->AddChildChunk(phdrCk);

	//***********
	// pbag chunk
	//***********
	Chunk *pbagCk = new Chunk("pbag");
	const size_t ITEMS_IN_PGEN = 2;
	pbagCk->size = (uint32) ((numInstrs + 1) * sizeof(sfPresetBag));
	pbagCk->data = new uint8[pbagCk->size];
	for (size_t i = 0; i < numInstrs; i++) {
		sfPresetBag presetBag;
		memset(&presetBag, 0, sizeof(sfPresetBag));
		presetBag.wGenNdx = (uint16) (i * ITEMS_IN_PGEN);
		presetBag.wModNdx = 0;

		memcpy(pbagCk->data + (i * sizeof(sfPresetBag)), &presetBag, sizeof(sfPresetBag));
	}
	//  add terminal sfPresetBag
	sfPresetBag presetBag;
	memset(&presetBag, 0, sizeof(sfPresetBag));
	presetBag.wGenNdx = (uint16) (numInstrs * ITEMS_IN_PGEN);
	memcpy(pbagCk->data + (numInstrs * sizeof(sfPresetBag)), &presetBag, sizeof(sfPresetBag));
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
	pgenCk->size = (uint32) ((synthfile->vInstrs.size() * sizeof(sfGenList) * ITEMS_IN_PGEN) +
							 sizeof(sfGenList));
	pgenCk->data = new uint8[pgenCk->size];
	uint32 dataPtr = 0;
	for (size_t i = 0; i < numInstrs; i++) {
		sfGenList genList;
		memset(&genList, 0, sizeof(sfGenList));

		// reverbEffectsSend
		genList.sfGenOper = reverbEffectsSend;
		genList.genAmount.shAmount = 250;
		memcpy(pgenCk->data + dataPtr, &genList, sizeof(sfGenList));
		dataPtr += sizeof(sfGenList);

		genList.sfGenOper = instrument;
		genList.genAmount.wAmount = (uint16) i;
		memcpy(pgenCk->data + dataPtr, &genList, sizeof(sfGenList));
		dataPtr += sizeof(sfGenList);
	}
	//  add terminal sfGenList
	sfGenList genList;
	memset(&genList, 0, sizeof(sfGenList));
	memcpy(pgenCk->data + dataPtr, &genList, sizeof(sfGenList));

	pdtaCk->AddChildChunk(pgenCk);

	//***********
	// inst chunk
	//***********
	Chunk *instCk = new Chunk("inst");
	instCk->size = (uint32) ((synthfile->vInstrs.size() + 1) * sizeof(sfInst));
	instCk->data = new uint8[instCk->size];
	size_t rgnCounter = 0;
	for (size_t i = 0; i < numInstrs; i++) {
		SynthInstr *instr = synthfile->vInstrs[i];

		sfInst inst;
		memset(&inst, 0, sizeof(sfInst));
		memcpy(inst.achInstName, instr->name.c_str(),
			   MIN((unsigned long) instr->name.size(), (unsigned long) 20));
		inst.wInstBagNdx = (uint16) rgnCounter;
		rgnCounter += instr->vRgns.size();

		memcpy(instCk->data + (i * sizeof(sfInst)), &inst, sizeof(sfInst));
	}
	//  add terminal sfInst
	sfInst inst;
	memset(&inst, 0, sizeof(sfInst));
	inst.wInstBagNdx = (uint16) rgnCounter;
	memcpy(instCk->data + (numInstrs * sizeof(sfInst)), &inst, sizeof(sfInst));
	pdtaCk->AddChildChunk(instCk);

	//***********
	// ibag chunk - stores all zones (regions) for instruments
	//***********
	Chunk *ibagCk = new Chunk("ibag");

	size_t totalNumRgns = 0;
	for (size_t i = 0; i < numInstrs; i++)
		totalNumRgns += synthfile->vInstrs[i]->vRgns.size();

	ibagCk->size = (uint32) ((totalNumRgns + 1) * sizeof(sfInstBag));
	ibagCk->data = new uint8[ibagCk->size];

	rgnCounter = 0;
	int instGenCounter = 0;
	for (size_t i = 0; i < numInstrs; i++) {
		SynthInstr *instr = synthfile->vInstrs[i];

		size_t numRgns = instr->vRgns.size();
		for (size_t j = 0; j < numRgns; j++) {
			sfInstBag instBag;
			memset(&instBag, 0, sizeof(sfInstBag));
			instBag.wInstGenNdx = instGenCounter;
			instGenCounter += 11;
			instBag.wInstModNdx = 0;

			memcpy(ibagCk->data + (rgnCounter++ * sizeof(sfInstBag)), &instBag, sizeof(sfInstBag));
		}
	}
	//  add terminal sfInstBag
	sfInstBag instBag;
	memset(&instBag, 0, sizeof(sfInstBag));
	instBag.wInstGenNdx = instGenCounter;
	instBag.wInstModNdx = 0;
	memcpy(ibagCk->data + (rgnCounter * sizeof(sfInstBag)), &instBag, sizeof(sfInstBag));
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
	igenCk->size = (uint32) ((totalNumRgns * sizeof(sfInstGenList) * 11) + sizeof(sfInstGenList));
	igenCk->data = new uint8[igenCk->size];
	dataPtr = 0;
	for (size_t i = 0; i < numInstrs; i++) {
		SynthInstr *instr = synthfile->vInstrs[i];

		size_t numRgns = instr->vRgns.size();
		for (size_t j = 0; j < numRgns; j++) {
			SynthRgn *rgn = instr->vRgns[j];

			sfInstGenList instGenList;
			// Key range - (if exists) this must be the first chunk
			instGenList.sfGenOper = keyRange;
			instGenList.genAmount.ranges.byLo = (uint8) rgn->usKeyLow;
			instGenList.genAmount.ranges.byHi = (uint8) rgn->usKeyHigh;
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			dataPtr += sizeof(sfInstGenList);

			if (rgn->usVelHigh)  // 0 means 'not set', fixes TriAce instruments
			{
				// Velocity range (if exists) this must be the next chunk
				instGenList.sfGenOper = velRange;
				instGenList.genAmount.ranges.byLo = (uint8) rgn->usVelLow;
				instGenList.genAmount.ranges.byHi = (uint8) rgn->usVelHigh;
				memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
				dataPtr += sizeof(sfInstGenList);
			}

			// initialAttenuation
			instGenList.sfGenOper = initialAttenuation;
			instGenList.genAmount.shAmount = (int16_t) (rgn->sampinfo->attenuation * 10);
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			dataPtr += sizeof(sfInstGenList);

			// pan
			instGenList.sfGenOper = pan;
			instGenList.genAmount.shAmount =
					(int16_t) ConvertPercentPanTo10thPercentUnits(rgn->art->pan);
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			dataPtr += sizeof(sfInstGenList);

			// sampleModes
			instGenList.sfGenOper = sampleModes;
			instGenList.genAmount.wAmount = rgn->sampinfo->cSampleLoops;
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			dataPtr += sizeof(sfInstGenList);

			// overridingRootKey
			instGenList.sfGenOper = overridingRootKey;
			instGenList.genAmount.wAmount = rgn->sampinfo->usUnityNote;
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			dataPtr += sizeof(sfInstGenList);

			// attackVolEnv
			instGenList.sfGenOper = attackVolEnv;
			instGenList.genAmount.shAmount =
					(rgn->art->attack_time == 0)
					? -32768
					: round(SecondsToTimecents(rgn->art->attack_time));
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			dataPtr += sizeof(sfInstGenList);

			// decayVolEnv
			instGenList.sfGenOper = decayVolEnv;
			instGenList.genAmount.shAmount =
					(rgn->art->decay_time == 0) ? -32768
												: round(SecondsToTimecents(rgn->art->decay_time));
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			dataPtr += sizeof(sfInstGenList);

			// sustainVolEnv
			instGenList.sfGenOper = sustainVolEnv;
			if (rgn->art->sustain_lev > 100.0)
				rgn->art->sustain_lev = 100.0;
			instGenList.genAmount.shAmount = (int16_t) (rgn->art->sustain_lev * 10);
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			dataPtr += sizeof(sfInstGenList);

			// releaseVolEnv
			instGenList.sfGenOper = releaseVolEnv;
			instGenList.genAmount.shAmount =
					(rgn->art->release_time == 0)
					? -32768
					: round(SecondsToTimecents(rgn->art->release_time));
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			dataPtr += sizeof(sfInstGenList);

			// reverbEffectsSend
			// instGenList.sfGenOper = reverbEffectsSend;
			// instGenList.genAmount.shAmount = 800;
			// memcpy(pgenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
			// dataPtr += sizeof(sfInstGenList);

			// sampleID - this is the terminal chunk
			instGenList.sfGenOper = sampleID;
			instGenList.genAmount.wAmount = (uint16) (rgn->tableIndex);
			memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
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
	memcpy(igenCk->data + dataPtr, &instGenList, sizeof(sfInstGenList));
	// memset(ibagCk->data + (totalNumRgns*sizeof(sfInstBag)), 0, sizeof(sfInstBag));
	// igenCk->SetData(&genList, sizeof(sfGenList));
	pdtaCk->AddChildChunk(igenCk);

	//***********
	// shdr chunk
	//***********
	Chunk *shdrCk = new Chunk("shdr");

	size_t numSamps = synthfile->vWaves.size();
	shdrCk->size = (uint32) ((numSamps + 1) * sizeof(sfSample));
	shdrCk->data = new uint8[shdrCk->size];

	uint32 sampOffset = 0;
	for (size_t i = 0; i < numSamps; i++) {
		SynthWave *wave = synthfile->vWaves[i];

		sfSample samp;
		memset(&samp, 0, sizeof(sfSample));
		memcpy(samp.achSampleName, wave->name.c_str(),
			   MIN((unsigned long) wave->name.size(), (unsigned long) 20));
		samp.dwStart = sampOffset;
		samp.dwEnd = samp.dwStart + (wave->dataSize / sizeof(uint16));
		sampOffset = samp.dwEnd + 46;  // plus the 46 padding samples required by sf2 spec

		// Search through all regions for an associated sampInfo structure with this sample
		SynthSampInfo *sampInfo = NULL;
		for (size_t j = 0; j < numInstrs; j++) {
			SynthInstr *instr = synthfile->vInstrs[j];

			size_t numRgns = instr->vRgns.size();
			for (size_t k = 0; k < numRgns; k++) {
				SynthRgn *rgn = instr->vRgns[k];
				if (rgn->tableIndex == i && rgn->sampinfo != NULL) {
					sampInfo = rgn->sampinfo;
					break;
				}
			}
			if (sampInfo != NULL)
				break;
		}
		//  If we didn't find a rgn association, then it should be in the SynthWave structure.
		if (sampInfo == NULL)
			sampInfo = wave->sampinfo;
		assert(sampInfo != NULL);

		samp.dwStartloop = samp.dwStart + sampInfo->ulLoopStart;
		samp.dwEndloop = samp.dwStartloop + sampInfo->ulLoopLength;
		samp.dwSampleRate = wave->dwSamplesPerSec;
		samp.byOriginalKey = (uint8) (sampInfo->usUnityNote);
		samp.chCorrection = (char) (sampInfo->sFineTune);
		samp.wSampleLink = 0;
		samp.sfSampleType = monoSample;

		memcpy(shdrCk->data + (i * sizeof(sfSample)), &samp, sizeof(sfSample));
	}

	//  add terminal sfSample
	memset(shdrCk->data + (numSamps * sizeof(sfSample)), 0, sizeof(sfSample));
	pdtaCk->AddChildChunk(shdrCk);

	this->AddChildChunk(pdtaCk);
}

SF2File::~SF2File() {}

const void *SF2File::SaveToMem() {
	uint8 *buf = new uint8[this->GetSize()];
	this->Write(buf);
	return buf;
}
