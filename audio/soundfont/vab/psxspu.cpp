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
#include "psxspu.h"

// A lot of games use a simple linear amplitude decay/release for their envelope.
// In other words, the envelope level drops at a constant rate (say from
// 0xFFFF to 0 (cps2) ), and to get the attenuation we multiply by this
// percent value (env_level / 0xFFFF).  This means the attenuation will be
// -20*log10( env_level / 0xFFFF ) decibels.  Wonderful, but SF2 and DLS have
// the a linear decay in decibels - not amplitude - for their decay/release slopes.
// So if you were to graph it, the SF2/DLS attenuation over time graph would be
// a simple line.

// (Note these are obviously crude ASCII drawings and in no way accurate!)
// 100db
// |                  /
// |               /
// |            /
// |         /
// |      /
// 10db /                   -  half volume
// |/
// |--------------------TIME

// But games using linear amplitude have a convex curve
// 100db
// |                  -
// |                  -
// |                 -
// |                -
// |             -
// 10db       x             -  half volume
// |-   -
// |-------------------TIME

// Now keep in mind that 10db of attenuation is half volume to the human ear.
// What this mean is that SF2/DLS are going to sound like they have much shorter
// decay/release rates if we simply plug in a time value from 0 atten to full atten
// from a linear amplitude game.

// My approach at the moment is to calculate the time it takes to get to half volume
// and then use that value accordingly with the SF2/DLS decay time.  In other words
// Take the second graph, find where y = 10db, and the draw a line from the origin
// through it to get your DLS/SF2 decay/release line
// (the actual output value is time where y = 100db for sf2 or 96db for DLS, SynthFile class uses
// 100db).

// This next function converts seconds to full attenuation in a linear amplitude decay scale
// and approximates the time to full attenuation in a linear DB decay scale.
double LinAmpDecayTimeToLinDBDecayTime(double secondsToFullAtten, int linearVolumeRange) {
	double expMinDecibel = -100.0;
	double linearMinDecibel = log10(1.0 / linearVolumeRange) * 20.0;
	double linearToExpScale = log(linearMinDecibel - expMinDecibel) / log(2.0);
	return secondsToFullAtten * linearToExpScale;
}

/*
 * PSX's PSU analysis was done by Neill Corlett.
 * Thanks to Antires for his ADPCM decompression routine.
 */

PSXSampColl::PSXSampColl(VGMInstrSet *instrset, uint32 offset,
						 uint32 length, const Common::Array<SizeOffsetPair> &vagLocations)
		: VGMSampColl(instrset->GetRawFile(), instrset, offset, length),
		  _vagLocations(vagLocations) {}

bool PSXSampColl::GetSampleInfo() {
	if (_vagLocations.empty()) {
		/*
		 * We scan through the sample section, and determine the offsets and size of each sample
		 * We do this by searching for series of 16 0x00 value bytes.  These indicate the beginning
		 * of a sample, and they will never be found at any other point within the adpcm sample
		 * data.
		 */
		uint32 nEndOffset = _dwOffset + _unLength;
		if (_unLength == 0) {
			nEndOffset = GetEndOffset();
		}

		uint32 i = _dwOffset;
		while (i + 32 <= nEndOffset) {
			bool isSample = false;

			if (GetWord(i) == 0 && GetWord(i + 4) == 0 && GetWord(i + 8) == 0 &&
				GetWord(i + 12) == 0) {
				// most of samples starts with 0s
				isSample = true;
			} else {
				// some sample blocks may not start with 0.
				// so here is a dirty hack for it.
				// (Dragon Quest VII, for example)
				int countOfContinue = 0;
				uint8 continueByte = 0xff;
				bool badBlock = false;
				while (i + (countOfContinue * 16) + 16 <= nEndOffset) {
					uint8 keyFlagByte = GetByte(i + (countOfContinue * 16) + 1);

					if ((keyFlagByte & 0xF8) != 0) {
						badBlock = true;
						break;
					}

					if (continueByte == 0xff) {
						if (keyFlagByte == 0 || keyFlagByte == 2) {
							continueByte = keyFlagByte;
						}
					}

					if (keyFlagByte != continueByte) {
						if (keyFlagByte == 0 || keyFlagByte == 2) {
							badBlock = true;
						}
						break;
					}
					countOfContinue++;
				}
				if (!badBlock && ((continueByte == 0 && countOfContinue >= 16) ||
								  (continueByte == 2 && countOfContinue >= 3))) {
					isSample = true;
				}
			}

			if (isSample) {
				uint32 extraGunkLength = 0;
				//uint8 filterRangeByte = GetByte(i + 16);
				uint8 keyFlagByte = GetByte(i + 16 + 1);
				if ((keyFlagByte & 0xF8) != 0)
					break;

				// if (filterRangeByte == 0 && keyFlagByte == 0)	// Breaking on FFXII 309 -
				// Eruyt Village at 61D50 of the WD
				if (GetWord(i + 16) == 0 && GetWord(i + 20) == 0 && GetWord(i + 24) == 0 &&
					GetWord(i + 28) == 0)
					break;

				uint32 beginOffset = i;
				i += 16;

				// skip through until we reach the chunk with the end flag set
				bool loopEnd = false;
				while (i + 16 <= nEndOffset && !loopEnd) {
					loopEnd = ((GetByte(i + 1) & 1) != 0);
					i += 16;
				}

				// deal with exceptional cases where we see 00 07 77 77 77 77 77 etc.
				while (i + 16 <= nEndOffset) {
					loopEnd = ((GetByte(i + 1) & 1) != 0);
					if (!loopEnd) {
						break;
					}
					extraGunkLength += 16;
					i += 16;
				}

				PSXSamp *samp = new PSXSamp(this, beginOffset, i - beginOffset, beginOffset,
											i - beginOffset - extraGunkLength, 1, 16, 44100,
											Common::String::format("Sample %d", _samples.size()));
				_samples.push_back(samp);
			} else {
				break;
			}
		}
		_unLength = i - _dwOffset;
	} else {
		uint32 sampleIndex = 0;
		for (Common::Array<SizeOffsetPair>::iterator it = _vagLocations.begin();
			 it != _vagLocations.end(); ++it) {
			uint32 offSampStart = _dwOffset + it->offset;
			uint32 offDataEnd = offSampStart + it->size;
			uint32 offSampEnd = offSampStart;

			// detect loop end and ignore garbages like 00 07 77 77 77 77 77 etc.
			bool lastBlock;
			do {
				if (offSampEnd + 16 > offDataEnd) {
					offSampEnd = offDataEnd;
					break;
				}

				lastBlock = ((GetByte(offSampEnd + 1) & 1) != 0);
				offSampEnd += 16;
			} while (!lastBlock);

			PSXSamp *samp = new PSXSamp(this, _dwOffset + it->offset, it->size,
										_dwOffset + it->offset, offSampEnd - offSampStart, 1, 16,
										44100, Common::String::format("Sample %d", sampleIndex));
			_samples.push_back(samp);
			sampleIndex++;
		}
	}
	return true;
}

//  *******
//  PSXSamp
//  *******

PSXSamp::PSXSamp(VGMSampColl *sampColl, uint32 offset, uint32 length, uint32 dataOffset,
				 uint32 dataLen, uint8 nChannels, uint16 theBPS, uint32 theRate,
				 Common::String name, bool bSetloopOnConversion)
		: VGMSamp(sampColl, offset, length, dataOffset, dataLen, nChannels, theBPS, theRate, name),
		  _setLoopOnConversion(bSetloopOnConversion) {
	_bPSXLoopInfoPrioritizing = true;
}

double PSXSamp::GetCompressionRatio() {
	return ((28.0 / 16.0) * 2);  // aka 3.5;
}

void PSXSamp::ConvertToStdWave(uint8 *buf) {
	int16 *uncompBuf = (int16 *) buf;
	VAGBlk theBlock;
	f32 prev1 = 0;
	f32 prev2 = 0;

	if (this->_setLoopOnConversion)
		SetLoopStatus(0);  // loopStatus is initiated to -1.  We should default it now to not loop

	bool addrOutOfVirtFile = false;
	for (uint32 k = 0; k < _dataLength; k += 0x10)  // for every adpcm chunk
	{
		if (_dwOffset + k + 16 > _vgmfile->GetEndOffset()) {
			debug("Unexpected EOF (%s)", _name.c_str());
			break;
		} else if (!addrOutOfVirtFile && k + 16 > _unLength) {
			debug("Unexpected end of PSXSamp (%s)", _name.c_str());
			addrOutOfVirtFile = true;
		}

		theBlock.range = GetByte(_dwOffset + k) & 0xF;
		theBlock.filter = (GetByte(_dwOffset + k) & 0xF0) >> 4;
		theBlock.flag.end = GetByte(_dwOffset + k + 1) & 1;
		theBlock.flag.looping = (GetByte(_dwOffset + k + 1) & 2) > 0;

		// this can be the loop point, but in wd, this info is stored in the instrset
		theBlock.flag.loop = (GetByte(_dwOffset + k + 1) & 4) > 0;
		if (this->_setLoopOnConversion) {
			if (theBlock.flag.loop) {
				this->SetLoopOffset(k);
				this->SetLoopLength(_dataLength - k);
			}
			if (theBlock.flag.end && theBlock.flag.looping) {
				SetLoopStatus(1);
			}
		}

		GetRawFile()->GetBytes(_dwOffset + k + 2, 14, theBlock.brr);

		// each decompressed pcm block is 52 bytes   EDIT: (wait, isn't it 56 bytes? or is it 28?)
		DecompVAGBlk(uncompBuf + ((k * 28) / 16), &theBlock, &prev1, &prev2);
	}
}

// This next function is taken from Antires's work
void PSXSamp::DecompVAGBlk(int16 *pSmp, VAGBlk *pVBlk, f32 *prev1, f32 *prev2) {
	uint32 i, shift;  // Shift amount for compressed samples
	f32 t;         // Temporary sample
	f32 f1, f2;
	f32 p1, p2;
	static const f32 Coeff[5][2] = {{0.0,          0.0},
									{60.0 / 64.0,  0.0},
									{115.0 / 64.0, 52.0 / 64.0},
									{98.0 / 64.0,  55.0 / 64.0},
									{122.0 / 64.0, 60.0 / 64.0}};

	// Expand samples ---------------------------
	shift = pVBlk->range + 16;

	for (i = 0; i < 14; i++) {
		pSmp[i * 2] = ((int32) pVBlk->brr[i] << 28) >> shift;
		pSmp[i * 2 + 1] = ((int32) (pVBlk->brr[i] & 0xF0) << 24) >> shift;
	}

	// Apply ADPCM decompression ----------------
	i = pVBlk->filter;

	if (i) {
		f1 = Coeff[i][0];
		f2 = Coeff[i][1];
		p1 = *prev1;
		p2 = *prev2;

		for (i = 0; i < 28; i++) {
			t = pSmp[i] + (p1 * f1) - (p2 * f2);
			pSmp[i] = (int16) t;
			p2 = p1;
			p1 = t;
		}

		*prev1 = p1;
		*prev2 = p2;
	} else {
		*prev2 = pSmp[26];
		*prev1 = pSmp[27];
	}
}
