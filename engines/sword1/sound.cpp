/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "common/config-manager.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "sword1/sound.h"
#include "sword1/resman.h"
#include "sword1/logic.h"
#include "sword1/sword1.h"

#include "audio/audiostream.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/xa.h"
#include "audio/decoders/aiff.h"

namespace Sword1 {

#define SPEECH_FLAGS (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN)

Sound::Sound(Audio::Mixer *mixer, SwordEngine *vm, ResMan *pResMan)
	: _rnd("sword1sound") {
	_vm = vm;
	_mixer = mixer;
	_resMan = pResMan;
	_bigEndianSpeech = false;
	_cowHeader = nullptr;
	_cowMode = CowWave;
	_endOfQueue = 0;
	_currentCowFile = 0;

	_musicOutputStream[0] = Audio::makeQueuingAudioStream(DEFAULT_SAMPLE_RATE, false);
	_musicOutputStream[1] = Audio::makeQueuingAudioStream(DEFAULT_SAMPLE_RATE, false);
	getVolumes();

	for (uint i = 0; i < ARRAYSIZE(_fxQueue); i++) _fxQueue[i].reset();
}

Sound::~Sound() {
	// clean up fx queue
	_mixer->stopAll();
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++)
		if (_fxQueue[cnt].delay == 0)
			_resMan->resClose(getSampleId(_fxQueue[cnt].id));
	_endOfQueue = 0;
	closeCowSystem();
}

/*  Original volume tables from the Windows drivers:
 *  
 *  static const int32 speechVolTable[17] = {
 *      -10000,
 *      -5000, -3000, -2500, -2250,
 *      -2000, -1750, -1500, -1250,
 *      -1000, -750,  -500,  -350,
 *      -200,  -100,  -50,   0
 *  };
 *  
 *  static const int32 musicVolTable[17] = {
 *      -5000,
 *      -4000, -3500, -3000, -2750,
 *      -2500, -2250, -2000, -1800,
 *      -1550, -1300, -1100, -900, 
 *      -700,  -500,  -400,  -200  
 *  };
 *  
 *  static const int32 fxVolTable[17] = {
 *      -10000,
 *      -5000, -3000, -2500, -2250,
 *      -2000, -1750, -1500, -1250,
 *      -1000, -750,  -500,  -350,
 *      -200,  -100,  -50,   0
 *  };
 *  
 *  These values are expressed in hundredths of a decibel, therefore we pre-processed them in the following way:
 *  
 *  * Convert each dB value to a linear scale (from 0.0 to 1.0):
 *        pow(10, dB / 2000.0);
 *  * Normalize the obtained value to the 0-255 range, keeping in mind that DirectSound allowable
 *    values are between DSBVOLUME_MAX (no attenuation, 0) and DSBVOLUME_MIN (silence, -10000):
 *        normalizedValue = int(round((linearValue - DSBVOLUME_MIN) / (DSBVOLUME_MAX - DSBVOLUME_MIN) * 255.0))
 */

static const int32 speechVolTable[17] = {
	0,    // -100.00 dB
	1,    // -50.00 dB
	8,    // -30.00 dB
	14,   // -25.00 dB
	19,   // -22.50 dB
	25,   // -20.00 dB
	34,   // -17.50 dB
	45,   // -15.00 dB
	60,   // -12.50 dB
	81,   // -10.00 dB
	108,  // -7.50 dB
	143,  // -5.00 dB
	170,  // -3.50 dB
	203,  // -2.00 dB
	227,  // -1.00 dB
	241,  // -0.50 dB
	255   // 0 dB
};

static const int32 musicVolTable[17] = {
	1,    // -50.00 dB
	3,    // -40.00 dB
	5,    // -35.00 dB
	8,    // -30.00 dB
	11,   // -27.50 dB
	14,   // -25.00 dB
	19,   // -22.50 dB
	25,   // -20.00 dB
	32,   // -18.00 dB
	43,   // -15.50 dB
	57,   // -13.00 dB
	72,   // -11.00 dB
	90,   // -9.00 dB
	114,  // -7.00 dB
	143,  // -5.00 dB
	161,  // -4.00 dB
	203   // -2.00 dB
};

static const int32 fxVolTable[17] = {
	0,    // -100.00 dB
	1,    // -50.00 dB
	8,    // -30.00 dB
	14,   // -25.00 dB
	19,   // -22.50 dB
	25,   // -20.00 dB
	34,   // -17.50 dB
	45,   // -15.00 dB
	60,   // -12.50 dB
	81,   // -10.00 dB
	108,  // -7.50 dB
	143,  // -5.00 dB
	170,  // -3.50 dB
	203,  // -2.00 dB
	227,  // -1.00 dB
	241,  // -0.50 dB
	255   // 0 dB
};

static int32 getWindowsPanValue(int32 vol) {
	// DirectSound handles pan values in decibel, just like volume and accepts
	// values between -10000 (-100 dB, all to the left) and 10000 (100 dB, all to the right);
	// more specifically, a negative value lowers the current volume of the right
	// channel by that amount of decibels and viceversa for positive values.
	//
	// As we already know that any input value for this function is going to come
	// exclusively from any of the volume tables (therefore the conversion from log
	// to linear volumes is already performed), we can think of the range of possible
	// target pan values in two parts:
	// - The positive part (originally 0 to 10000 hundredths of dB) is the complementary part of the
	//   input volume divided by 2 and multiplied by -1, so it fits the (0,127) part of the pan range;
	// - The negative part (-10000 to 0 hundredths of dB) is the complementary part of the input volume 
	//   multiplied by -1 and divided by 2, so it fits the (-127,0) part of the pan range.

	if (vol > 0) {
		return -((vol - 255) / 2);
	} else if (vol < 0) {
		return ((vol - 255) / 2);
	}

	return 0;
}

uint32 Sound::getSampleId(int32 fxNo) {
	byte cluster = _fxList[fxNo].sampleId.cluster;
	byte id;
	if (SwordEngine::_systemVars.isDemo && SwordEngine::_systemVars.platform == Common::kPlatformWindows && !SwordEngine::_systemVars.isSpanishDemo) {
		id = _fxList[fxNo].sampleId.idWinDemo;
	} else {
		id = _fxList[fxNo].sampleId.idStd;
	}
	return (cluster << 24) | id;
}

void Sound::checkSpeechFileEndianness() {
	// Some mac versions (not all of them) use big endian wav, although
	// the wav header doesn't indicate it.
	// Use heuristic to determine endianness of speech.
	// The heuristic consist in computing the sum of the absolute difference for
	// every two consecutive samples. This is done both with a big endian and a
	// little endian assumption. The one with the smallest sum should be the
	// correct one (the sound wave is supposed to be relatively smooth).
	// It needs at least 1000 samples to get stable result (the code below is
	// using the first 2000 samples of the wav sound).

	// Init speech file if not already done.
	if (!_currentCowFile) {
		// Open one of the speech files. It uses SwordEngine::_systemVars.currentCD
		// to decide which file to open, therefore if it is currently set to zero
		// we have to set it to either 1 or 2 (I decided to set it to 1 as this is
		// more likely to be the first file that will be needed).
		bool no_current_cd = false;
		if (SwordEngine::_systemVars.currentCD == 0) {
			SwordEngine::_systemVars.currentCD = 1;
			no_current_cd = true;
		}
		initCowSystem();
		if (no_current_cd) {
			// In case it fails with CD1 retry with CD2
			if (!_currentCowFile) {
				SwordEngine::_systemVars.currentCD = 2;
				initCowSystem();
			}
			// Reset currentCD flag
			SwordEngine::_systemVars.currentCD = 0;
		}
	}

	// Testing for endianness makes sense only if using the uncompressed files.
	if (_cowHeader == nullptr || (_cowMode != CowWave && _cowMode != CowDemo))
		return;

	// I picked the sample to use randomly (I just made sure it is long enough so that there is
	// a fair chance of the heuristic to have a stable result and work for every language).
	int roomNo = _currentCowFile == 1 ? 1 : 129;
	int localNo = _currentCowFile == 1 ? 2 : 933;
	// Get the speech data and apply the heuristic
	uint32 locIndex = _cowHeader[roomNo] >> 2;
	uint32 sampleSize = _cowHeader[locIndex + (localNo * 2)];
	uint32 index = _cowHeader[locIndex + (localNo * 2) - 1];
	if (sampleSize) {
		uint32 size = 0;
		bool leOk = false, beOk = false;
		// Compute average of difference between two consecutive samples for both BE and LE

		_bigEndianSpeech = false;
		byte *compSample = (byte *)malloc(sampleSize);
		_cowFile.seek(index + _cowHeaderSize, SEEK_SET);
		_cowFile.read(compSample, sampleSize);
		byte *data = (byte *)malloc(getSpeechSize(compSample, sampleSize));
		expandSpeech(compSample, data, sampleSize, &leOk, &size);
		uint32 maxSamples = size > 2000 ? 2000 : size;
		double le_diff = endiannessHeuristicValue((int16 *)data, size, maxSamples);
		free(data);
		data = nullptr;

		_bigEndianSpeech = true;
		_cowFile.seek(index + _cowHeaderSize, SEEK_SET);
		_cowFile.read(compSample, sampleSize);
		data = (byte *)malloc(getSpeechSize(compSample, sampleSize));
		expandSpeech(compSample, data, sampleSize, &beOk, &size);
		double be_diff = endiannessHeuristicValue((int16 *)data, size, maxSamples);
		free(data);

		free(compSample);

		// Set the big endian flag
		if (leOk && !beOk)
			_bigEndianSpeech = false;
		else if (beOk && !leOk)
			_bigEndianSpeech = true;
		else
			_bigEndianSpeech = (be_diff < le_diff);

		if (_bigEndianSpeech)
			debug(6, "Mac version: using big endian speech file");
		else
			debug(6, "Mac version: using little endian speech file");

		debug(8, "Speech decompression memory check: big endian = %s, little endian = %s", beOk ? "good" : "bad", leOk ? "good" : "bad");
		debug(8, "Speech endianness heuristic: average = %f for BE and %f for LE (%d samples)", be_diff, le_diff, maxSamples);
	}
}

double Sound::endiannessHeuristicValue(int16* data, uint32 dataSize, uint32 &maxSamples) {
	if (!data)
		return 50000.; // the heuristic value for the wrong endianess is about 21000 (1/3rd of the 16 bits range)

	double diff_sum = 0.;
	uint32 cpt = 0;
	int16 prev_value = (int16)FROM_LE_16(*((uint16 *)(data)));
	for (uint32 i = 1; i < dataSize && cpt < maxSamples; ++i) {
		int16 value = (int16)FROM_LE_16(*((uint16 *)(data + i)));
		if (value != prev_value) {
			diff_sum += fabs((double)(value - prev_value));
			++cpt;
			prev_value = value;
		}
	}
	if (cpt == 0)
		return 50000.;
	maxSamples = cpt;
	return diff_sum / cpt;
}

int Sound::addToQueue(uint32 fxNo) {
	bool alreadyInQueue = false;
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++) {
		if (_fxQueue[cnt].id == fxNo) {
			alreadyInQueue = true;
			break;
		}
	}

	if (alreadyInQueue) {
		debug(5, "Sound::addToQueue(): Sound %d is already in the queue, ignoring...", fxNo);
		return 0;
	} else {
		if (_endOfQueue == MAX_FXQ_LENGTH) {
			warning("Sound::addToQueue(): Sound queue overflow");
			return 0;
		}

		uint32 sampleId = getSampleId(fxNo);
		if ((sampleId & 0xFF) != 0xFF) {
			_resMan->resOpen(sampleId);
			_fxQueue[_endOfQueue].id = fxNo;

			if (_fxList[fxNo].type == FX_SPOT) {
				_fxQueue[_endOfQueue].delay = _fxList[fxNo].delay + 1;
			} else {
				_fxQueue[_endOfQueue].delay = 1;
			}

			_endOfQueue++;
			return 1; // Success!
		}

		return 0;
	}
}

void Sound::removeFromQueue(uint32 fxNo) {
	bool alreadyInQueue = false;
	int cnt = 0;

	for (cnt = 0; cnt < _endOfQueue; cnt++) {
		if (_fxQueue[cnt].id == fxNo) {
			alreadyInQueue = true;
			break;
		}
	}

	if (alreadyInQueue) {
		_resMan->resClose(getSampleId(_fxQueue[cnt].id));

		for (int j = 0; j < _endOfQueue; j++) {
			if (_fxQueue[j].id == fxNo) {
				// Move all the others down one to fill this space...
				for (int i = j; i < (_endOfQueue - 1); i++) {
					_fxQueue[i].id = _fxQueue[i + 1].id;
					_fxQueue[i].delay = _fxQueue[i + 1].delay;
				}

				debug(5, "Sound::addToQueue(): Sound fxNo %d removed from _fxQueue[%d] (_endOfQueue = %d)", fxNo, j, _endOfQueue - 1);
				_endOfQueue--;

				break;
			}
		}
	}
}

void Sound::engine() {
	// Update music streaming...
	updateMusicStreaming();

	// Add any random sfx to the queue...
	for (uint16 cnt = 0; cnt < TOTAL_FX_PER_ROOM; cnt++) {
		uint16 fxNo = _roomsFixedFx[Logic::_scriptVars[SCREEN]][cnt];
		if (fxNo) {
			if (_fxList[fxNo].type == FX_RANDOM) {
				if (_rnd.getRandomNumber(_fxList[fxNo].delay) == 0)
					addToQueue(fxNo);
			}
		} else
			break;
	}

	// Now process the queue...
	int32 fxNo = 0;
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++) {
		fxNo = _fxQueue[cnt].id;
		if (_fxQueue[cnt].delay > 0) {
			_fxQueue[cnt].delay--;
			if (_fxQueue[cnt].delay == 0)
				playSample(fxNo);
		} else if (checkSampleStatus(fxNo) == S_STATUS_FINISHED) {
			// Delay countdown was already zero, so the sample has
			// already been played, so check if it's finished...
			removeFromQueue(fxNo);
		}
	}
}

bool Sound::amISpeaking() {
	int16 *offset;
	int16 count;
	int32 readPos;

	if (!_speechSampleBusy)
		return false;

	if (_mixer->isSoundHandleActive(_hSampleSpeech)) {
		_speechLipsyncCounter += 1;

		readPos = _speechLipsyncCounter * 919 * 2;

		// Ensure that we don't read beyond the buffer...
		if (readPos + (int32)(150 * sizeof(int16)) > _speechSize)
			return false;

		offset = (int16 *)&_speechSample[readPos];
		count = 0;
		for (int i = 0; i < 150; i++) {
			if ((offset[i] < NEG_MOUTH_THRESHOLD) || (offset[i] > POS_MOUTH_THRESHOLD)) {
				count += 1;
				if (count == 50) {
					return true;
				}
			}
		}
	}

	return false;
}

void Sound::newScreen(uint32 screen) {
	if (_currentCowFile != SwordEngine::_systemVars.currentCD) {
		if (_cowFile.isOpen())
			closeCowSystem();
		initCowSystem();
	}

	// Start the room's looping sounds.
	for (uint16 cnt = 0; cnt < TOTAL_FX_PER_ROOM; cnt++) {
		uint16 fxNo = _roomsFixedFx[screen][cnt];
		if (fxNo) {
			if (_fxList[fxNo].type == FX_LOOP)
				addToQueue(fxNo);
		} else
			break;
	}
}

void Sound::startSpeech(uint16 roomNo, uint16 localNo) {
	if (_cowHeader == nullptr) {
		warning("Sound::startSpeech: COW file isn't open");
		return;
	}

	uint32 locIndex = 0xFFFFFFFF;
	uint32 sampleFileSize = 0;
	uint32 index = 0;

	if (_cowMode == CowPSX) {
		Common::File file;
		uint16 i;

		if (!file.open("speech.lis")) {
			warning("Could not open speech.lis");
			return;
		}

		for (i = 0; !file.eos() && !file.err(); i++)
			if (file.readUint16LE() == roomNo) {
				locIndex = i;
				break;
			}
		file.close();

		if (locIndex == 0xFFFFFFFF) {
			warning("Could not find room %d in speech.lis", roomNo);
			return;
		}

		if (!file.open("speech.inf")) {
			warning("Could not open speech.inf");
			return;
		}

		uint16 numRooms = file.readUint16LE();

		file.seek(locIndex * 4 + 2); // 4 bytes per room, skip first 2 bytes

		uint16 numLines = file.readUint16LE();
		uint16 roomOffset = file.readUint16LE();

		// The offset is in terms of uint16's, so multiply by 2. Skip the room indexes too...
		file.seek(2 + numRooms * 4 + roomOffset * 2);

		locIndex = 0xFFFFFFFF;

		for (i = 0; i < numLines; i++)
			if (file.readUint16LE() == localNo) {
				locIndex = i;
				break;
			}

		if (locIndex == 0xFFFFFFFF) {
			warning("Could not find local number %d in room %d in speech.inf", roomNo, localNo);
			return;
		}

		file.close();

		index = _cowHeader[(roomOffset + locIndex) * 2];
		sampleFileSize = _cowHeader[(roomOffset + locIndex) * 2 + 1];
	} else {
		locIndex = _cowHeader[roomNo] >> 2;
		sampleFileSize = _cowHeader[locIndex + (localNo * 2)];
		index = _cowHeader[locIndex + (localNo * 2) - 1];
	}

	debug(6, "Sound::startSpeech(%d, %d): locIndex %d, sampleFileSize %d, index %d", roomNo, localNo, locIndex, sampleFileSize, index);

	if (sampleFileSize) {
		byte *compSample = (byte *)malloc(sampleFileSize);
		if (compSample) {
			if ((_cowMode == CowWave) || (_cowMode == CowDemo)) {
				_cowFile.seek(index + _cowHeaderSize, SEEK_SET);
			} else if (_cowMode == CowPSX && sampleFileSize != 0xffffffff) {
				_cowFile.seek(index * 2048);
			} else if (_cowMode == CowFLAC || _cowMode == CowVorbis || _cowMode == CowMP3) {
				_cowFile.seek(index);
			}

			_cowFile.read(compSample, sampleFileSize);
			_speechSize = getSpeechSize(compSample, sampleFileSize);

			// Force alignment on sample size (bug #15094)...
			if ((_speechSize % 2) != 0)
				_speechSize += 1;

			_speechSample = (byte *)malloc(_speechSize);

			if (_speechSample) {
				// Sometimes, the decompressed speech size is slightly longer than the actual
				// decompressed sample data we are about to receive; this is normal, but to
				// fully emulate what the original does, let's zero out the sample buffer.
				memset(_speechSample, 0, _speechSize);

				SwordEngine::_systemVars.speechRunning = expandSpeech(compSample, _speechSample, sampleFileSize);
				free(compSample);
			}
		}
	}
}

bool Sound::expandSpeech(byte *src, byte *dst, uint32 dstSize, bool *endiannessCheck, uint32 *sizeForEndiannessCheck) {

	if (_cowMode == CowPSX) {
		Audio::RewindableAudioStream *stream = Audio::makeXAStream(new Common::MemoryReadStream(src, dstSize), 11025);
		stream->readBuffer((int16 *)_speechSample, _speechSize);
		return true;
	} else if (_cowMode != CowWave && _cowMode != CowDemo) {
		Audio::RewindableAudioStream *stream = nullptr;
#ifdef USE_FLAC
		if (_cowMode == CowFLAC) {
			stream = Audio::makeFLACStream(new Common::MemoryReadStream(src, dstSize), DisposeAfterUse::YES);
		}
#endif
#ifdef USE_VORBIS
		if (_cowMode == CowVorbis) {
			stream = Audio::makeVorbisStream(new Common::MemoryReadStream(src, dstSize), DisposeAfterUse::YES);
		}
#endif
#ifdef USE_MAD
		if (_cowMode == CowMP3) {
			stream = Audio::makeMP3Stream(new Common::MemoryReadStream(src, dstSize), DisposeAfterUse::YES);
		}
#endif
		if (stream) {
			stream->readBuffer((int16 *)_speechSample, _speechSize);
			return true;
		} else {
			return false;
		}
	}

	uint8 *fBuf = src;
	uint32 headerPos = 0;

	while ((READ_BE_UINT32(fBuf + headerPos) != 'data') && (headerPos < 100))
		headerPos++;

	if (headerPos < 100) {
		if (endiannessCheck)
			*endiannessCheck = true;
		int32 resSize;
		int16 *srcData;
		uint32 srcPos;
		int16 length;
		dstSize /= 2;
		headerPos += 4; // skip 'data' tag
		if (_cowMode != CowDemo) {
			resSize = READ_LE_UINT32(fBuf + headerPos) >> 1;
			headerPos += 4;
		} else {
			// the demo speech files have the uncompressed size
			// embedded in the compressed stream *sigh*
			//
			// But not always, apparently. See bug #4002. Is
			// there any way to figure out the size other than
			// decoding the sound in that case?

			if (fBuf[headerPos + 1] == 0) {
				if (READ_LE_UINT16(fBuf + headerPos) == 1) {
					resSize = READ_LE_UINT16(fBuf + headerPos + 2);
					resSize |= READ_LE_UINT16(fBuf + headerPos + 6) << 16;
				} else
					resSize = READ_LE_UINT32(fBuf + headerPos + 2);
				resSize >>= 1;
			} else {
				resSize = 0;
				srcData = (int16 *)fBuf;
				srcPos = headerPos >> 1;
				while (srcPos < dstSize) {
					length = (int16)READ_LE_UINT16(srcData + srcPos);
					srcPos++;
					if (length < 0) {
						resSize -= length;
						srcPos++;
					} else {
						resSize += length;
						srcPos += length;
					}
				}
			}
		}
		assert(!(headerPos & 1));
		srcData = (int16 *)fBuf;
		srcPos = headerPos >> 1;
		uint32 dstPos = 0;
		int16 *dstData = (int16 *)dst;

		int32 samplesLeft = resSize;
		while (srcPos < dstSize && samplesLeft > 0) {
			length = (int16)(_bigEndianSpeech ? READ_BE_UINT16(srcData + srcPos) : READ_LE_UINT16(srcData + srcPos));
			srcPos++;
			if (length < 0) {
				length = -length;
				if (length > samplesLeft) {
					length = samplesLeft;
					if (endiannessCheck)
						*endiannessCheck = false;
				}
				int16 value;
				if (_bigEndianSpeech) {
					value = (int16)SWAP_BYTES_16(*((uint16 *)(srcData + srcPos)));
				} else {
					value = srcData[srcPos];
				}
				for (uint16 cnt = 0; cnt < (uint16)length; cnt++)
					dstData[dstPos++] = value;
				srcPos++;
			} else {
				if (length > samplesLeft) {
					length = samplesLeft;
					if (endiannessCheck)
						*endiannessCheck = false;
				}
				if (_bigEndianSpeech) {
					for (uint16 cnt = 0; cnt < (uint16)length; cnt++)
						dstData[dstPos++] = (int16)SWAP_BYTES_16(*((uint16 *)(srcData + (srcPos++))));
				} else {
					memcpy(dstData + dstPos, srcData + srcPos, length * 2);
					dstPos += length;
					srcPos += length;
				}
			}
			samplesLeft -= length;
		}
		if (samplesLeft > 0) {
			memset(dstData + dstPos, 0, samplesLeft * 2);
			if (endiannessCheck)
				*endiannessCheck = false;
		}
		if (_cowMode == CowDemo) // demo has wave output size embedded in the compressed data
			*(uint32 *)dstData = 0;
		return true;
	} else {
		warning("Sound::expandSpeech(): DATA tag not found in wave header");
		return false;
	}
}

void Sound::initCowSystem() {
	if (SwordEngine::_systemVars.currentCD == 0)
		return;

	char cowName[25];
	/* look for speech1/2.clu in the data dir
	   and speech/speech.clu (running from cd or using cd layout)
	*/
#ifdef USE_FLAC
	if (!_cowFile.isOpen()) {
		Common::sprintf_s(cowName, "SPEECH%d.CLF", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (_cowFile.isOpen()) {
			debug(1, "Using FLAC compressed Speech Cluster");
			_cowMode = CowFLAC;
		}
	}
#endif
#ifdef USE_VORBIS
	if (!_cowFile.isOpen()) {
		Common::sprintf_s(cowName, "SPEECH%d.CLV", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (_cowFile.isOpen()) {
			debug(1, "Using Vorbis compressed Speech Cluster");
			_cowMode = CowVorbis;
		}
	}
#endif
#ifdef USE_MAD
	if (!_cowFile.isOpen()) {
		Common::sprintf_s(cowName, "SPEECH%d.CL3", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (_cowFile.isOpen()) {
			debug(1, "Using MP3 compressed Speech Cluster");
			_cowMode = CowMP3;
		}
	}
#endif
	if (!_cowFile.isOpen()) {
		Common::sprintf_s(cowName, "SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (!_cowFile.isOpen()) {
			_cowFile.open("speech.clu");
		}
		debug(1, "Using uncompressed Speech Cluster");
		_cowMode = CowWave;
	}

	if (SwordEngine::isPsx()) {
		// There's only one file on the PSX, so set it to the current disc.
		_currentCowFile = SwordEngine::_systemVars.currentCD;
		if (!_cowFile.isOpen()) {
			if (!_cowFile.open("speech.dat"))
				error("Could not open speech.dat");
			_cowMode = CowPSX;
		}
	}

	if (!_cowFile.isOpen())
		_cowFile.open("speech.clu");

	if (!_cowFile.isOpen()) {
		_cowFile.open("cows.mad");
		if (_cowFile.isOpen())
			_cowMode = CowDemo;
	}

	if (_cowFile.isOpen()) {
		if (SwordEngine::isPsx()) {
			// Get data from the external table file
			Common::File tableFile;
			if (!tableFile.open("speech.tab"))
				error("Could not open speech.tab");
			_cowHeaderSize = tableFile.size();
			_cowHeader = (uint32 *)malloc(_cowHeaderSize);
			if (_cowHeaderSize & 3)
				error("Unexpected cow header size %d", _cowHeaderSize);
			for (uint32 cnt = 0; cnt < _cowHeaderSize / 4; cnt++)
				_cowHeader[cnt] = tableFile.readUint32LE();
		} else {
			_cowHeaderSize = _cowFile.readUint32LE();
			_cowHeader = (uint32 *)malloc(_cowHeaderSize);
			if (_cowHeaderSize & 3)
				error("Unexpected cow header size %d", _cowHeaderSize);
			for (uint32 cnt = 0; cnt < (_cowHeaderSize / 4) - 1; cnt++)
				_cowHeader[cnt] = _cowFile.readUint32LE();
			_currentCowFile = SwordEngine::_systemVars.currentCD;
		}
	} else
		warning("Sound::initCowSystem: Can't open SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);
}

void Sound::closeCowSystem() {
	_cowFile.close();
	free(_cowHeader);
	_cowHeader = nullptr;
	_currentCowFile = 0;
}

int32 Sound::checkSampleStatus(int32 id) {
	Common::StackLock lock(_soundMutex);

	for (int i = 0; i < MAX_FX; i++) {
		if (_fxSampleId[i] == id) {
			if (!_mixer->isSoundHandleActive(_hSampleFX[i]) && (_fxSampleBusy[i]) && (!_fxPaused[i])) {
				_fxSampleBusy[i] = false;
				return S_STATUS_FINISHED;
			} else {
				return S_STATUS_RUNNING;
			}
		}
	}

	return S_STATUS_RUNNING;
}

int32 Sound::checkSpeechStatus() {
	Common::StackLock lock(_soundMutex);

	if (!_speechSampleBusy || _speechPaused)
		return S_STATUS_FINISHED;

	if (!_mixer->isSoundHandleActive(_hSampleSpeech)) {
		_speechSampleBusy = 0;
		restoreMusicVolume();
		return S_STATUS_FINISHED;
	}

	return S_STATUS_RUNNING;
}

void Sound::playSpeech() {
	Common::StackLock lock(_soundMutex);

	_speechLipsyncCounter = 0;

	if (_speechSampleBusy)
		stopSpeech();

	_speechSampleBusy = true;

	Audio::SeekableAudioStream *stream = Audio::makeRawStream(
		(byte *)_speechSample, _speechSize, 11025, SPEECH_FLAGS, DisposeAfterUse::NO);
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_hSampleSpeech, stream);

	if (SwordEngine::_systemVars.useWindowsAudioMode) {
		int32 vol = 0;
		int32 pan = 0;

		if (_volSpeech[0] < _volSpeech[1]) {		
			vol = speechVolTable[_volSpeech[1]];
			pan = getWindowsPanValue(-speechVolTable[(16 * _volSpeech[0]) / _volSpeech[1]]);
		} else if (_volSpeech[0] > _volSpeech[1]) {
			vol = speechVolTable[_volSpeech[0]];
			pan = getWindowsPanValue(speechVolTable[(16 * _volSpeech[1]) / _volSpeech[0]]);
		} else {
			vol = speechVolTable[_volSpeech[1]];
			pan = 0;
		}

		_mixer->setChannelVolume(_hSampleSpeech, vol);
		_mixer->setChannelBalance(_hSampleSpeech, pan);
	} else {
		byte speechVolume = clampVolume(2 * (4 * (_volSpeech[0] + _volSpeech[1])));
		_mixer->setChannelVolume(_hSampleSpeech, speechVolume);

		int pan = 64 + (4 * ((int32)_volSpeech[1] - (int32)_volSpeech[0]));
		_mixer->setChannelBalance(_hSampleSpeech, scalePan(pan));

		reduceMusicVolume();
	}
}

void Sound::stopSpeech() {
	Common::StackLock lock(_soundMutex);

	if (_mixer->isSoundHandleActive(_hSampleSpeech)) {
		_mixer->stopHandle(_hSampleSpeech);
		_speechSampleBusy = false;

		if (!SwordEngine::_systemVars.useWindowsAudioMode)
			restoreMusicVolume();
	}
}

static void soundCallback(void *refCon) {
	Sound *snd = (Sound *)refCon;
	Common::StackLock lock(snd->_soundMutex);

	// Originally the code here had handling of fading flags for
	// the master volume and the music volume (the latter only
	// for fade-ups). They are omitted here as they are not used
	// anywhere...

	if (snd->_fxFadingFlag) {
		snd->_fxCount += 1;
		if (snd->_fxCount > 128 / snd->_fxFadingRate) {
			snd->_fxFadingFlag = 0;
		} else {
			if (snd->_fxFadingFlag == 1) {
				// Fade the volume up...
				snd->_fxFadeVolume[0] = 8 * snd->_volFX[0] * snd->_fxCount * snd->_fxFadingRate / 128;
				snd->_fxFadeVolume[1] = 8 * snd->_volFX[1] * snd->_fxCount * snd->_fxFadingRate / 128;
			} else {
				// Fade the volume down...
				snd->_fxFadeVolume[0] = 8 * snd->_volFX[0] - (8 * snd->_volFX[0] * snd->_fxCount * snd->_fxFadingRate / 128);
				snd->_fxFadeVolume[1] = 8 * snd->_volFX[1] - (8 * snd->_volFX[1] * snd->_fxFadingRate * snd->_fxCount / 128);
			}

			for (int i = 0; i < MAX_FX; i++) {
				if (snd->_fxSampleBusy[i]) {
					int32 targetVolume = (snd->_fxFadeVolume[0] + snd->_fxFadeVolume[1]) / 2;

					// Multiplying by 2 because Miles Sound System uses 0-127 and we use 0-255
					snd->setFXVolume(snd->clampVolume(targetVolume * 2), i);
				}
			}
		}
	}
}

void Sound::installFadeTimer() {
	if (!SwordEngine::_systemVars.useWindowsAudioMode)
		_vm->getTimerManager()->installTimerProc(&soundCallback, 1000000 / TIMER_RATE, this, "AILFadeTimer");
}

void Sound::uninstallFadeTimer() {
	if (!SwordEngine::_systemVars.useWindowsAudioMode)
		_vm->getTimerManager()->removeTimerProc(&soundCallback);
}

void Sound::setFXVolume(byte targetVolume, int handleIdx) {
	_mixer->setChannelVolume(_hSampleFX[handleIdx], targetVolume);
}

void Sound::playSample(int32 fxNo) {
	uint8 *samplePtr;
	uint32 vol[2] = { 0, 0 };
	int32 screen = Logic::_scriptVars[SCREEN];

	samplePtr = (uint8 *)_resMan->fetchRes(getSampleId(fxNo));

	for (int i = 0; i < MAX_ROOMS_PER_FX; i++) {
		if (_fxList[fxNo].roomVolList[i].roomNo != 0) {
			if ((_fxList[fxNo].roomVolList[i].roomNo == screen) ||
				(_fxList[fxNo].roomVolList[i].roomNo == -1)) { // -1 indicates 'all rooms'
				vol[0] = (_fxList[fxNo].roomVolList[i].leftVol);
				vol[1] = (_fxList[fxNo].roomVolList[i].rightVol);

				debug(5, "Sound::playSample(): fxNo=%d, vol[0]=%d, vol[1]=%d)",fxNo,vol[0],vol[1]);
				playFX(fxNo, _fxList[fxNo].type, samplePtr, vol);
				break;
			}
		} else {
			break;
		}
	}
}

void Sound::stopSample(int32 fxNo) {
	stopFX(fxNo);
}

bool Sound::prepareMusicStreaming(const Common::Path &filename, int newHandleId, int32 tuneId, uint32 volume, int8 pan, MusCompMode assignedMode) {
	int sampleRate = DEFAULT_SAMPLE_RATE;
	WaveHeader wavHead;
	bool isStereo = false;

	if (filename.empty())
		return false;

	if (!_musicFile[newHandleId].open(filename)) {
		debug(5, "Sound::streamMusicFile(): couldn't find file %s, bailing out...", filename.toString().c_str());
		return false;
	}

	delete _compressedMusicStream[newHandleId];

	if (assignedMode == MusWav) {
		if (_musicFile[newHandleId].read(&wavHead, sizeof(WaveHeader)) != sizeof(WaveHeader)) {
			debug(5, "Sound::streamMusicFile(): couldn't read from file %s, bailing out...", filename.toString().c_str());
			_musicFile[newHandleId].close();
			return false;
		}

		sampleRate = wavHead.dwSamplesPerSec;
	}
#ifdef USE_FLAC
	else if (assignedMode == MusFLAC) {
		_compressedMusicStream[newHandleId] = Audio::makeFLACStream(&_musicFile[newHandleId], DisposeAfterUse::NO);
		sampleRate = _compressedMusicStream[newHandleId]->getRate();
	}
#endif
#ifdef USE_VORBIS
	else if (assignedMode == MusVorbis) {
		_compressedMusicStream[newHandleId] = Audio::makeVorbisStream(&_musicFile[newHandleId], DisposeAfterUse::NO);
		sampleRate = _compressedMusicStream[newHandleId]->getRate();
	}
#endif
#ifdef USE_MAD
	else if (assignedMode == MusMP3) {
		_compressedMusicStream[newHandleId] = Audio::makeMP3Stream(&_musicFile[newHandleId], DisposeAfterUse::NO);
		sampleRate = _compressedMusicStream[newHandleId]->getRate();
	}
#endif
	else if (assignedMode == MusAif) {
		_compressedMusicStream[newHandleId] = Audio::makeAIFFStream(&_musicFile[newHandleId], DisposeAfterUse::NO);
		sampleRate = _compressedMusicStream[newHandleId]->getRate();
	} else if (assignedMode == MusPSX) {
		Common::File tableFile;
		if (!tableFile.open("tunes.tab")) {
			debug(5, "Sound::streamMusicFile(): couldn't open the tunes.tab file, bailing out...");
			return false;
		}

		// The PSX demo has a broken/truncated tunes.tab. So we check here
		// that the offset is not beyond the end of the file.
		int32 tableOffset = (tuneId - 1) * 8;
		if (tableOffset >= tableFile.size())
			return false;
		tableFile.seek(tableOffset, SEEK_SET);
		uint32 offset = tableFile.readUint32LE() * 0x800;
		uint32 size = tableFile.readUint32LE();

		tableFile.close();

		// Because of broken tunes.dat/tab in psx demo,
		// also check that tune offset is not over file size
		if ((size != 0) && (size != 0xffffffff) && ((int32)(offset + size) <= _musicFile[newHandleId].size())) {
			_musicFile[newHandleId].seek(offset, SEEK_SET);
			_compressedMusicStream[newHandleId] = Audio::makeXAStream(_musicFile[newHandleId].readStream(size), DEFAULT_SAMPLE_RATE);
		}
	}

	if (assignedMode != MusWav && !_compressedMusicStream[newHandleId]) {
		debug(5, "Sound::streamMusicFile(): couldn't process compressed file %s, bailing out...", filename.toString().c_str());
		_musicFile[newHandleId].close();
		return false;
	}

	_musicOutputStream[newHandleId] = Audio::makeQueuingAudioStream(sampleRate, isStereo);
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_hSampleMusic[newHandleId], _musicOutputStream[newHandleId]);

	_mixer->setChannelRate(_hSampleMusic[newHandleId], sampleRate);
	_mixer->setChannelVolume(_hSampleMusic[newHandleId], clampVolume((int32)volume));
	_mixer->setChannelBalance(_hSampleMusic[newHandleId], pan);

	_musicStreamPlaying[newHandleId] = true;
	_musicStreamFormat[newHandleId] = assignedMode;

	return true;
}

void Sound::streamMusicFile(int32 tuneId, int32 looped) {
	int32 oldHandleId, newHandleId;

	Common::File tmp;
	Common::String filename(_tuneList[tuneId]);

	MusCompMode assignedMode = MusWav;
	if (tmp.exists(Common::Path(filename + ".wav"))) {
		filename = filename + ".wav";
		assignedMode = MusWav;
	} else if (SwordEngine::isPsx() && tmp.exists("tunes.dat") && tmp.exists("tunes.tab")) {
		filename = "tunes.dat";
		assignedMode = MusPSX;
	} else if (tmp.exists(Common::Path(filename + ".fla"))) {
		filename = filename + ".fla";
		assignedMode = MusFLAC;
	} else if (tmp.exists(Common::Path(filename + ".ogg"))) {
		filename = filename + ".ogg";
		assignedMode = MusVorbis;
	} else if (tmp.exists(Common::Path(filename + ".mp3"))) {
		filename = filename + ".mp3";
		assignedMode = MusMP3;
	} else if (tmp.exists(Common::Path(filename + ".flac"))) {
		filename = filename + ".flac";
		assignedMode = MusFLAC;
	} else if (tmp.exists(Common::Path(filename + ".aif"))) {
		filename = filename + ".aif";
		assignedMode = MusAif;
	} else {
		filename = "";
	}

	newHandleId = 0;

	if ((_musicStreamPlaying[0]) || (_musicStreamPlaying[1])) {
		// In this case at least one of the music streams is already
		// busy so we still attempt to find a free one...
		if (_musicStreamPlaying[0])
			newHandleId = 1;
		oldHandleId = 1 - newHandleId;

		_streamLoopingFlag[newHandleId] = looped;

		_musicStreamFading[oldHandleId] = SwordEngine::_systemVars.useWindowsAudioMode ? -16 : -12;
		_musicStreamFading[newHandleId] = SwordEngine::_systemVars.useWindowsAudioMode ? 0 : 1;

		if (_musicStreamPlaying[newHandleId]) {
			// Whoops, they are BOTH busy! Let's kill the older one...
			_mixer->stopHandle(_hSampleMusic[newHandleId]);
			_musicFile[newHandleId].close();

			int32 vol = 2 * (2 * (_volMusic[0] + _volMusic[1]));
			int32 pan = scalePan(64 + (4 * (_volMusic[1] - _volMusic[0])));

			if (SwordEngine::_systemVars.useWindowsAudioMode) {
				if (_volMusic[0] < _volMusic[1]) {
					vol = musicVolTable[_volMusic[1]];
					pan = getWindowsPanValue(-musicVolTable[16 * _volMusic[0] / _volMusic[1]]);
				} else if (_volMusic[0] > _volMusic[1]) {
					vol = musicVolTable[_volMusic[0]];
					pan = getWindowsPanValue(musicVolTable[16 * _volMusic[1] / _volMusic[0]]);
				} else {
					vol = musicVolTable[_volMusic[1]];
					pan = 0;
				} 
			}

			bool success = prepareMusicStreaming(Common::Path(filename), newHandleId, tuneId, vol, pan, assignedMode);

			if (success)
				debug(5, "Sound::streamMusicFile(): interrupting sound in handle %d to play %s", newHandleId, filename.c_str());

			return;
		} else {
			// All good! We got the non-busy one :-)
			int32 vol = 0;
			int32 pan = scalePan(64 + (4 * (_volMusic[1] - _volMusic[0])));

			if (SwordEngine::_systemVars.useWindowsAudioMode) {
				if (_volMusic[0] < _volMusic[1]) {
					vol = musicVolTable[_volMusic[1]];
				pan = getWindowsPanValue(-musicVolTable[16 * _volMusic[0] / _volMusic[1]]);
				} else if (_volMusic[0] > _volMusic[1]) {
					vol = musicVolTable[_volMusic[0]];
					pan = getWindowsPanValue(musicVolTable[16 * _volMusic[1] / _volMusic[0]]);
				} else {
					vol = musicVolTable[_volMusic[1]];
					pan = 0;
				} 
			}

			bool success = prepareMusicStreaming(Common::Path(filename), newHandleId, tuneId, vol, pan, assignedMode);

			if (success)
				debug(5, "Sound::streamMusicFile(): playing sound %s in handle %d with other handle busy", filename.c_str(), newHandleId);
		}
	} else {
		// No streams are busy, let's go!
		int32 vol = 2 * (3 * (_volMusic[0] + _volMusic[1]));
		int32 pan = scalePan(64 + (4 * (_volMusic[1] - _volMusic[0])));

		if (SwordEngine::_systemVars.useWindowsAudioMode) {
			if (_volMusic[0] < _volMusic[1]) {
				vol = musicVolTable[_volMusic[1]];
				pan = getWindowsPanValue(-musicVolTable[16 * _volMusic[0] / _volMusic[1]]);
			} else if (_volMusic[0] > _volMusic[1]) {
				vol = musicVolTable[_volMusic[0]];
				pan = getWindowsPanValue(musicVolTable[16 * _volMusic[1] / _volMusic[0]]);
			} else {
				vol = musicVolTable[_volMusic[1]];
				pan = 0;
			} 
		}

		bool success = prepareMusicStreaming(Common::Path(filename), newHandleId, tuneId, vol, pan, assignedMode);

		if (success)
			debug(5, "Sound::streamMusicFile(): playing sound %s in handle %d", filename.c_str(), newHandleId);
	}

	_streamLoopingFlag[newHandleId] = looped;
}

void Sound::updateMusicStreaming() {
	Common::StackLock lock(_soundMutex);

	// Within this function we make sure to fade music streams and
	// stop them whenever they are either finished or at zero volume
	for (int i = 0; i < MAX_MUSIC; i++) {
		if ((_musicStreamPlaying[i]) && (!_musicPaused[i])) {
			if (_musicStreamFading[i]) {
				if (_crossFadeIncrement) {
					_crossFadeIncrement = false;

					if (_musicStreamFading[i] < 0) {
						if (!SwordEngine::_systemVars.useWindowsAudioMode) {
							debug(5, "Sound::updateMusicStreaming(): Fading %s to %d", _musicFile[i].getName(),
								2 * (((0 - _musicStreamFading[i]) * 3 * (_volMusic[0] + _volMusic[1])) / 16));
							_mixer->setChannelVolume(_hSampleMusic[i],
								clampVolume(2 * (((0 - _musicStreamFading[i]) * 3 * (_volMusic[0] + _volMusic[1])) / 16)));

							_musicStreamFading[i] += 1;
						} else {
							_musicStreamFading[i] += 1;

							int32 vol, pan;
							int32 volL = ((-_musicStreamFading[i]) * _volMusic[0]) >> 4;
							int32 volR = ((-_musicStreamFading[i]) * _volMusic[1]) >> 4;

							if (volL > volR) {
								vol = musicVolTable[volL];
								pan = musicVolTable[16 * volR / volL];
							} else if (volR > volL) {
								vol = musicVolTable[volR];
								pan = -musicVolTable[16 * volL / volR];
							} else {
								vol = musicVolTable[volR];
								pan = 0;
							}

							debug(5, "Sound::updateMusicStreaming(): Fading %s to %d (pan %d)", _musicFile[i].getName(), vol, getWindowsPanValue(pan));
							_mixer->setChannelVolume(_hSampleMusic[i], vol);
							_mixer->setChannelBalance(_hSampleMusic[i], getWindowsPanValue(pan));
						}
						
						if (_musicStreamFading[i] == 0) {
							_mixer->setChannelVolume(_hSampleMusic[i], 0);
							_musicOutputStream[i]->finish();
							_musicOutputStream[i] = nullptr;

							_mixer->stopHandle(_hSampleMusic[i]);
							_musicFile[i].close();

							_musicStreamPlaying[i] = false;
						}
					} else if (!SwordEngine::_systemVars.useWindowsAudioMode) { // The Windows driver doesn't fade-in
						debug(5, "Sound::updateMusicStreaming(): Fading %s to %d", _musicFile[i].getName(),
							2 * ((_musicStreamFading[i] * 3 * (_volMusic[0] + _volMusic[1])) / 16));
						_mixer->setChannelVolume(_hSampleMusic[i],
							clampVolume(2 * ((_musicStreamFading[i] * 3 * (_volMusic[0] + _volMusic[1])) / 16)));

						_musicStreamFading[i] += 1;
						if (_musicStreamFading[i] == 17) {
							_musicStreamFading[i] = 0;
						}
					}
				}
			}

			if (_musicFile[i].isOpen())
				serveSample(&_musicFile[i], i);

			if (!_mixer->isSoundHandleActive(_hSampleMusic[i]) ||
				(_musicOutputStream[i] && _musicOutputStream[i]->endOfData())) {
				_musicStreamPlaying[i] = false;

				if (_musicFile[i].isOpen())
					_musicFile[i].close();

				if (_musicOutputStream[i]) {
					_musicOutputStream[i]->finish();
					_musicOutputStream[i] = nullptr;
				}
			}
		}
	}
}

void Sound::serveSample(Common::File *file, int32 i) {
	int32 len;
	int32 nominalSize = MUSIC_BUFFER_SIZE;
	byte *buf = nullptr;

	if (!_musicPaused[i]) {
		if (_musicOutputStream[i]->numQueuedStreams() < 4) {
			if (_musicStreamFormat[i] == MusWav) {
				buf = (byte *)malloc(nominalSize);
				if (!buf) {
					warning("Sound::serveSample(): Couldn't allocate memory for streaming file %s", file->getName());
					return;
				}

				len = file->read(buf, nominalSize);
				if (len < nominalSize) {
					if (_streamLoopingFlag[i]) {
						file->seek(44, SEEK_SET);
						file->read(buf + len, nominalSize - len);
						len = nominalSize;
						debug(5, "Sound::serveSample(): Looping music file %s", file->getName());
					}
				}
			} else {
				buf = (byte *)malloc(nominalSize * 2);
				if (!buf) {
					warning("Sound::serveSample(): Couldn't allocate memory for streaming file %s", file->getName());
					return;
				}

				len = _compressedMusicStream[i]->readBuffer((int16 *)buf, nominalSize);
				if (len < nominalSize) {
					if (_streamLoopingFlag[i]) {
						_compressedMusicStream[i]->rewind();
						 _compressedMusicStream[i]->readBuffer((int16 *)buf + len, nominalSize - len);
						len = nominalSize;
						debug(5, "Sound::serveSample(): Looping music file %s", file->getName());
					} else {
						Common::String fname(file->getName());
						if (!fname.empty())
							debug(5, "Sound::serveSample(): Finished feeding music file %s", file->getName());
					}
				}
			}

			if (_musicStreamFormat[i] != MusWav)
				len *= 2;

			_musicOutputStream[i]->queueBuffer(buf, len, DisposeAfterUse::YES, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
		}
	}
}

void Sound::playFX(int32 fxID, int32 type, uint8 *wavData, uint32 vol[2]) {
	Common::StackLock lock(_soundMutex);

	int32 v0, v1;
	// Search through the FX sample handles for a free slot...
	for (int i = 0; i < MAX_FX; i++) {
		if (!_fxSampleBusy[i]) {
			// Found the handle! Now setup and play the sound...
			_fxSampleBusy[i] = true;
			_fxSampleId[i] = fxID;

			Audio::AudioStream *stream = nullptr;

			if (SwordEngine::isPsx()) {
				uint32 size = READ_LE_UINT32(wavData);
				stream = Audio::makeLoopingAudioStream(
					Audio::makeXAStream(new Common::MemoryReadStream(wavData + 4, size - 4), 11025),
					(type == FX_LOOP) ? 0 : 1);
			} else {
				uint32 size = READ_LE_UINT32(wavData + 0x28);
				uint8 flags;
				if (READ_LE_UINT16(wavData + 0x22) == 16)
					flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
				else
					flags = Audio::FLAG_UNSIGNED;
				if (READ_LE_UINT16(wavData + 0x16) == 2)
					flags |= Audio::FLAG_STEREO;

				stream = Audio::makeLoopingAudioStream(
					Audio::makeRawStream(wavData + 0x2C, size, 11025, flags, DisposeAfterUse::NO),
					(type == FX_LOOP) ? 0 : 1);
			}

			if (stream) {
				_mixer->playStream(Audio::Mixer::kPlainSoundType, &_hSampleFX[i], stream, -1, 0);

				if (SwordEngine::_systemVars.useWindowsAudioMode) {
					int32 leftVol  = (vol[0] * _volFX[0]) >> 4;
					int32 rightVol = (vol[1] * _volFX[1]) >> 4;
					int32 volume = 0;
					int32 pan = 0;

					if (leftVol > rightVol) {
						volume = fxVolTable[leftVol];
						pan = -fxVolTable[16 * rightVol / leftVol];
					} else if (leftVol < rightVol) {
						volume = fxVolTable[rightVol];
						pan = fxVolTable[16 * leftVol / rightVol];
					} else {
						volume = fxVolTable[leftVol];
						pan = 0;
					}

					_mixer->setChannelVolume(_hSampleFX[i], volume);
					_mixer->setChannelBalance(_hSampleFX[i], getWindowsPanValue(pan));
				} else {
					v0 = _volFX[0] * vol[0];
					v1 = _volFX[1] * vol[1];
		
					_mixer->setChannelVolume(_hSampleFX[i], clampVolume(2 * ((v0 + v1) / 8)));
					_mixer->setChannelBalance(_hSampleFX[i], scalePan(64 + ((v1 - v0) / 4)));
				}
			}

			return;
		}
	}
}

void Sound::stopFX(int32 fxID) {
	Common::StackLock lock(_soundMutex);
	for (int i = 0; i < MAX_FX; i++) {
		if (_fxSampleId[i] == fxID) {
			if (_mixer->isSoundHandleActive(_hSampleFX[i])) {
				_mixer->stopHandle(_hSampleFX[i]);
				_fxSampleBusy[i] = false;
			}
		}
	}
}

void Sound::clearAllFx() {
	// Remove them starting from the end...
	for (int j = _endOfQueue - 1; j >= 0; j--) {
		// Check if the sample has finished playing and
		// if not, stop it manually...
		if (checkSampleStatus(_fxQueue[j].id) == S_STATUS_RUNNING)
			stopFX(_fxQueue[j].id);

		removeFromQueue(_fxQueue[j].id);
	}

	_endOfQueue = 0;
}

void Sound::fadeMusicDown(int32 rate) {
	Common::StackLock lock(_soundMutex);
	_musicStreamFading[1 - _musicStreamPlaying[0]] = SwordEngine::_systemVars.useWindowsAudioMode ? -16 : -12;
}

void Sound::fadeFxDown(int32 rate) {
	Common::StackLock lock(_soundMutex);
	_fxFadingFlag = -1;
	_fxFadingRate = 2 * rate;
	_fxCount = 0;
}

void Sound::fadeFxUp(int32 rate) {
	Common::StackLock lock(_soundMutex);
	_fxFadingFlag = 1;
	_fxFadingRate = 2 * rate;
	_fxCount = 0;
}

int32 Sound::getSpeechSize(byte *compData, uint32 compSize) {
	if ((_cowMode == CowWave) || (_cowMode == CowDemo)) {
		WaveHeader *waveHeader = (WaveHeader *)compData;

		return (FROM_LE_32(waveHeader->riffSize) + 8) - sizeof(WaveHeader);
	} else {
		Common::MemoryReadStream memStream(compData, compSize);
		Audio::RewindableAudioStream *stream = nullptr;

		if (_cowMode == CowPSX) {
			stream = Audio::makeXAStream(&memStream, 11025);
		}
#ifdef USE_FLAC
		else if (_cowMode == CowFLAC) {
			stream = Audio::makeFLACStream(&memStream, DisposeAfterUse::YES);
		}
#endif
#ifdef USE_VORBIS
		else if (_cowMode == CowVorbis) {
			stream = Audio::makeVorbisStream(&memStream, DisposeAfterUse::YES);
		}
#endif
#ifdef USE_MAD
		else if (_cowMode == CowMP3) {
			stream = Audio::makeMP3Stream(&memStream, DisposeAfterUse::YES);
		}
#endif

		if (stream) {
			byte tmpBuffer[1000 * 2];
			int32 finalSize = 0;
			while (!stream->endOfData())
				finalSize += stream->readBuffer((int16 *)tmpBuffer, 1000) * 2;

			return finalSize;
		}
	}

	return 0;
}

void Sound::reduceMusicVolume() {
	Common::StackLock lock(_soundMutex);
	_musicFadeVolume[0] = _volMusic[0] * MUSIC_UNDERSCORE / 100;
	_musicFadeVolume[1] = _volMusic[0] * MUSIC_UNDERSCORE / 100; // We are explicitly accessing _volMusic[0] again

	// Multiplying by 2 because Miles Sound System uses 0-127 and we use 0-255
	_mixer->setChannelVolume(_hSampleMusic[0], clampVolume(2 * ((_musicFadeVolume[0] + _musicFadeVolume[1]) * 3)));
}

void Sound::restoreMusicVolume() {
	Common::StackLock lock(_soundMutex);

	// Multiplying by 2 because Miles Sound System uses 0-127 and we use 0-255
	_mixer->setChannelVolume(_hSampleMusic[0], clampVolume(2 * ((_volMusic[0] + _volMusic[1]) * 3)));
}

void Sound::setCrossFadeIncrement() {
	_crossFadeIncrement = true;
}

void Sound::pauseSpeech() {
	if ((_speechSampleBusy) && (!_speechPaused)) {
		_speechPaused = true;
		_mixer->pauseHandle(_hSampleSpeech, true);
	}
}

void Sound::unpauseSpeech() {
	if ((_speechSampleBusy) && (_speechPaused)) {
		_speechPaused = false;
		_mixer->pauseHandle(_hSampleSpeech, false);
	}
}

void Sound::pauseMusic() {
	Common::StackLock lock(_soundMutex);
	for (int i = 0; i < MAX_MUSIC; i++) {
		if (_musicStreamPlaying[i]) {
			_musicPaused[i] = true;
			_mixer->pauseHandle(_hSampleMusic[i], true);
		}
	}
}

void Sound::unpauseMusic() {
	Common::StackLock lock(_soundMutex);
	for (int i = 0; i < MAX_MUSIC; i++) {
		if (_musicPaused[i]) {
			_mixer->pauseHandle(_hSampleMusic[i], false);
			_musicPaused[i] = false;
		}
	}
}

void Sound::pauseFx() {
	Common::StackLock lock(_soundMutex);
	for (int i = 0; i < MAX_FX; i++) {
		if (_fxSampleBusy[i]) {
			_mixer->pauseHandle(_hSampleFX[i], true);
			_fxPaused[i] = true;
		}
	}
}

void Sound::unpauseFx() {
	Common::StackLock lock(_soundMutex);
	for (int i = 0; i < MAX_FX; i++) {
		if (_fxPaused[i]) {
			_mixer->pauseHandle(_hSampleFX[i], false);
			_fxPaused[i] = false;
		}
	}
}

static void getConfigVolumes(uint32 volL, uint32 volR, int &balance, int &volume) {
	// Calculate the balance
	if (volL + volR == 0) {
		balance = 50;
	} else {
		balance = (int)(100.0f * volL / (volL + volR) + 0.5f);
	}

	// Calculate and scale the volume to the 0-255 range
	volume = (int)(((volL + volR) * 255.0f / 32) + 0.5f);
	volume =  CLIP<int>(((volL + volR) * 255 / 32), 0, 255);
}

static void getGameVolumes(int balance, int volume, uint32 &volL, uint32 &volR) {
	volume = CLIP<int>(volume, 0, 255);

	int totalVolume = (int)(volume * 32.0f / 255 + 0.5f);

	if (balance == 50) {
		volL = totalVolume / 2;
		volR = volL;
		return;
	}

	volL = (uint32)(totalVolume * (balance / 100.0f) + 0.5f);
	volR = totalVolume - volL;
}

void Sound::getVolumes() {
	int musicVol = ConfMan.getInt("music_volume");
	int sfxVol = ConfMan.getInt("sfx_volume");
	int speechVol = ConfMan.getInt("speech_volume");

	int musicBal = 50;
	if (ConfMan.hasKey("music_balance")) {
		musicBal = CLIP(ConfMan.getInt("music_balance"), 0, 100);
	}

	int speechBal = 50;
	if (ConfMan.hasKey("speech_balance")) {
		speechBal = CLIP(ConfMan.getInt("speech_balance"), 0, 100);
	}

	int sfxBal = 50;
	if (ConfMan.hasKey("sfx_balance")) {
		sfxBal = CLIP(ConfMan.getInt("sfx_balance"), 0, 100);
	}

	getGameVolumes(musicBal,  musicVol,  _volMusic[0],  _volMusic[1]);
	getGameVolumes(speechBal, speechVol, _volSpeech[0], _volSpeech[1]);
	getGameVolumes(sfxBal,    sfxVol,    _volFX[0],     _volFX[1]);

	if (ConfMan.getBool("mute")) {
		_volSpeech[0] = 0;
		_volSpeech[1] = 0;
	}

	SwordEngine::_systemVars.showText = ConfMan.getBool("subtitles");

	if (_volSpeech[0] + _volSpeech[1] == 0) {
		SwordEngine::_systemVars.showText = true;
		SwordEngine::_systemVars.playSpeech = false;
	} else {
		SwordEngine::_systemVars.playSpeech = true;
	}
}

void Sound::setVolumes() {
	int volume = 0;
	int balance = 0;

	getConfigVolumes(_volMusic[0], _volMusic[1], balance, volume);
	if (volume != ConfMan.getInt("music_volume"))
		ConfMan.setInt("music_volume", volume);
	if (balance != ConfMan.getInt("music_balance"))
		ConfMan.setInt("music_balance", balance);

	getConfigVolumes(_volSpeech[0], _volSpeech[1], balance, volume);
	if (volume != ConfMan.getInt("speech_volume"))
		ConfMan.setInt("speech_volume", volume);
	if (balance != ConfMan.getInt("speech_balance"))
		ConfMan.setInt("speech_balance", balance);

	getConfigVolumes(_volFX[0], _volFX[1], balance, volume);
	if (volume != ConfMan.getInt("sfx_volume"))
		ConfMan.setInt("sfx_volume", volume);
	if (balance != ConfMan.getInt("sfx_balance"))
		ConfMan.setInt("sfx_balance", balance);

	if (SwordEngine::_systemVars.showText != ConfMan.getBool("subtitles"))
		ConfMan.setBool("subtitles", SwordEngine::_systemVars.showText);
	ConfMan.flushToDisk();

	if (_volSpeech[0] + _volSpeech[1] == 0) {
		SwordEngine::_systemVars.showText = true;
		SwordEngine::_systemVars.playSpeech = false;
	} else {
		SwordEngine::_systemVars.playSpeech = true;
	}
}

byte Sound::clampVolume(int32 volume) {
	return (byte)CLIP<int32>(volume, 0, 255);
}

int8 Sound::scalePan(int pan) {
	return (pan != 64) ? (int8)(2 * pan - 127) : 0;
}

} // End of namespace Sword1
