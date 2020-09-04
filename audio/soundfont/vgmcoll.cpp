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
#include "vgmcoll.h"
#include "vgminstrset.h"
#include "vgmsamp.h"

using namespace std;

double ConvertLogScaleValToAtten(double percent) {
	if (percent == 0)
		return 100.0;  // assume 0 is -100.0db attenuation
	double atten = 20 * log10(percent) * 2;
	return MIN<double>(-atten, 100.0);
}

// Convert a percent of volume value to it's attenuation in decibels.
//  ex: ConvertPercentVolToAttenDB_SF2(0.5) returns -(-6.02db) = half perceived loudness
double ConvertPercentAmplitudeToAttenDB_SF2(double percent) {
	if (percent == 0)
		return 100.0;  // assume 0 is -100.0db attenuation
	double atten = 20 * log10(percent);
	return MIN<double>(-atten, 100.0);
}

void VGMColl::UnpackSampColl(SynthFile &synthfile, VGMSampColl *sampColl,
							 Common::Array<VGMSamp *> &finalSamps) {
	assert(sampColl != nullptr);

	size_t nSamples = sampColl->_samples.size();
	for (size_t i = 0; i < nSamples; i++) {
		VGMSamp *samp = sampColl->_samples[i];

		uint32 bufSize;
		if (samp->_ulUncompressedSize)
			bufSize = samp->_ulUncompressedSize;
		else
			bufSize = (uint32) ceil((double) samp->_dataLength * samp->GetCompressionRatio());

		uint8 *uncompSampBuf =
				new uint8[bufSize];  // create a new memory space for the uncompressed wave
		samp->ConvertToStdWave(uncompSampBuf);  // and uncompress into that space

		uint16 blockAlign = samp->_bps / 8 * samp->_channels;
		SynthWave *wave =
				synthfile.AddWave(1, samp->_channels, samp->_rate, samp->_rate * blockAlign, blockAlign,
								  samp->_bps, bufSize, uncompSampBuf, (samp->_name));
		finalSamps.push_back(samp);

		// If we don't have any loop information, then don't create a sampInfo structure for the
		// Wave
		if (samp->_loop.loopStatus == -1) {
			debug("No loop information for %s - some parameters might be incorrect",
				  samp->_sampName.c_str());
			return;
		}

		SynthSampInfo *sampInfo = wave->AddSampInfo();
		if (samp->_bPSXLoopInfoPrioritizing) {
			if (samp->_loop.loopStart != 0 || samp->_loop.loopLength != 0)
				sampInfo->SetLoopInfo(samp->_loop, samp);
		} else
			sampInfo->SetLoopInfo(samp->_loop, samp);

		double attenuation = (samp->_volume != -1) ? ConvertLogScaleValToAtten(samp->_volume) : 0;
		uint8 unityKey = (samp->_unityKey != -1) ? samp->_unityKey : 0x3C;
		short fineTune = samp->_fineTune;
		sampInfo->SetPitchInfo(unityKey, fineTune, attenuation);
	}
}

SF2File *VGMColl::CreateSF2File(VGMInstrSet *theInstrSet) {
	SynthFile *synthfile = CreateSynthFile(theInstrSet);
	if (!synthfile) {
		debug("SF2 conversion aborted");
		return nullptr;
	}

	SF2File *sf2file = new SF2File(synthfile);
	delete synthfile;
	return sf2file;
}

SynthFile *VGMColl::CreateSynthFile(VGMInstrSet *theInstrSet) {
	Common::Array<VGMInstrSet *> instrsets;
	instrsets.push_back(theInstrSet);
	if (instrsets.empty()) {
		debug("No instruments found.");
		return nullptr;
	}

	/* FIXME: shared_ptr eventually */
	SynthFile *synthfile = new SynthFile("SynthFile");

	Common::Array<VGMSamp *> finalSamps;
	Common::Array<VGMSampColl *> finalSampColls;

	for (uint32 i = 0; i < instrsets.size(); i++) {
		VGMSampColl *instrset_sampcoll = instrsets[i]->_sampColl;
		if (instrset_sampcoll) {
			finalSampColls.push_back(instrset_sampcoll);
			UnpackSampColl(*synthfile, instrset_sampcoll, finalSamps);
		}
	}

	if (finalSamps.empty()) {
		debug("No sample collection present");
		delete synthfile;
		return nullptr;
	}

	for (size_t inst = 0; inst < instrsets.size(); inst++) {
		VGMInstrSet *set = instrsets[inst];
		size_t nInstrs = set->_aInstrs.size();
		for (size_t i = 0; i < nInstrs; i++) {
			VGMInstr *vgminstr = set->_aInstrs[i];
			size_t nRgns = vgminstr->_aRgns.size();
			if (nRgns == 0)  // do not write an instrument if it has no regions
				continue;
			SynthInstr *newInstr = synthfile->AddInstr(vgminstr->_bank, vgminstr->_instrNum);
			for (uint32 j = 0; j < nRgns; j++) {
				VGMRgn *rgn = vgminstr->_aRgns[j];
				//				if (rgn->sampNum+1 > sampColl->samples.size())
				////does thereferenced sample exist? 					continue;

				// Determine the SampColl associated with this rgn.  If there's an explicit pointer
				// to it, use that.
				VGMSampColl *sampColl = rgn->_sampCollPtr;
				if (!sampColl) {
					// If rgn is of an InstrSet with an embedded SampColl, use that SampColl.
					if (((VGMInstrSet *) rgn->_vgmfile)->_sampColl)
						sampColl = ((VGMInstrSet *) rgn->_vgmfile)->_sampColl;

						// If that does not exist, assume the first SampColl
					else
						sampColl = finalSampColls[0];
				}

				// Determine the sample number within the rgn's associated SampColl
				size_t realSampNum = rgn->_sampNum;

				// Determine the sampCollNum (index into our finalSampColls vector)
				size_t sampCollNum = finalSampColls.size();
				for (size_t k = 0; k < finalSampColls.size(); k++) {
					if (finalSampColls[k] == sampColl)
						sampCollNum = k;
				}
				if (sampCollNum == finalSampColls.size()) {
					debug("SampColl does not exist");
					delete synthfile;
					return nullptr;
				}
				//   now we add the number of samples from the preceding SampColls to the value to
				//   get the real sampNum in the final DLS file.
				for (uint32 k = 0; k < sampCollNum; k++)
					realSampNum += finalSampColls[k]->_samples.size();

				SynthRgn *newRgn = newInstr->AddRgn();
				newRgn->SetRanges(rgn->_keyLow, rgn->_keyHigh, rgn->_velLow, rgn->_velHigh);
				newRgn->SetWaveLinkInfo(0, 0, 1, (uint32) realSampNum);

				if (realSampNum >= finalSamps.size()) {
					debug("Sample %lu does not exist", realSampNum);
					realSampNum = finalSamps.size() - 1;
				}

				VGMSamp *samp = finalSamps[realSampNum];  // sampColl->samples[rgn->sampNum];
				SynthSampInfo *sampInfo = newRgn->AddSampInfo();

				// This is a really loopy way of determining the loop information, pardon the pun.
				// However, it works. There might be a way to simplify this, but I don't want to
				// test out whether another method breaks anything just yet Use the sample's
				// loopStatus to determine if a loop occurs.  If it does, see if the sample provides
				// loop info (gathered during ADPCM > PCM conversion.  If the sample doesn't provide
				// loop offset info, then use the region's loop info.
				if (samp->_bPSXLoopInfoPrioritizing) {
					if (samp->_loop.loopStatus != -1) {
						if (samp->_loop.loopStart != 0 || samp->_loop.loopLength != 0)
							sampInfo->SetLoopInfo(samp->_loop, samp);
						else {
							rgn->_loop.loopStatus = samp->_loop.loopStatus;
							sampInfo->SetLoopInfo(rgn->_loop, samp);
						}
					} else {
						error("_bPSXLoopInfoPrioritizing: Invalid sample loop status");
					}
				}
					// The normal method: First, we check if the rgn has loop info defined.
					// If it doesn't, then use the sample's loop info.
				else if (rgn->_loop.loopStatus == -1) {
					if (samp->_loop.loopStatus != -1)
						sampInfo->SetLoopInfo(samp->_loop, samp);
					else {
						error("Invalid sample loop status");
					}
				} else
					sampInfo->SetLoopInfo(rgn->_loop, samp);

				int8 realUnityKey = -1;
				if (rgn->_unityKey == -1)
					realUnityKey = samp->_unityKey;
				else
					realUnityKey = rgn->_unityKey;
				if (realUnityKey == -1)
					realUnityKey = 0x3C;

				short realFineTune;
				if (rgn->_fineTune == 0)
					realFineTune = samp->_fineTune;
				else
					realFineTune = rgn->_fineTune;

				double attenuation;
				if (rgn->_volume != -1)
					attenuation = ConvertLogScaleValToAtten(rgn->_volume);
				else if (samp->_volume != -1)
					attenuation = ConvertLogScaleValToAtten(samp->_volume);
				else
					attenuation = 0;

				double sustainLevAttenDb;
				if (rgn->_sustain_level == -1)
					sustainLevAttenDb = 0.0;
				else
					sustainLevAttenDb = ConvertPercentAmplitudeToAttenDB_SF2(rgn->_sustain_level);

				SynthArt *newArt = newRgn->AddArt();
				newArt->AddPan(rgn->_pan);
				newArt->AddADSR(rgn->_attack_time, (Transform) rgn->_attack_transform, rgn->_decay_time,
								sustainLevAttenDb, rgn->_sustain_time, rgn->_release_time,
								(Transform) rgn->_release_transform);

				sampInfo->SetPitchInfo(realUnityKey, realFineTune, attenuation);
			}
		}
	}
	return synthfile;
}
