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

#include "common/stream.h"

#include "audio/effects/hmi/hmifxlib.h"

#include "engines/grim/resource.h"
#include "engines/grim/imuse/imuse_engine.h"

namespace Grim {

int IMuse::setupHMI(int heapSize) {
	if (_hmiFxInitialized)
		return 1;

	memset(&_hmiInitData, 0, sizeof(_hmiInitData));
	_hmiInitData.heapSize = heapSize;
	if (_hmiFxLib->hmiFXInitSystem(&_hmiInitData))
		return 0;

	Common::SeekableReadStream *presetFile = g_resourceloader->openNewStreamFile("imuse.psl");
	int loadResult = presetFile ? _hmiFxLib->hmiFXPresetLoadLibrary(&_presetCollection, *presetFile) : 1;
	delete presetFile;

	if (loadResult) {
		_hmiFxLib->hmiFXUnInitSystem(&_hmiInitData);
		return 0;
	}

	_hmiInputFmt.fmtBeginSection = 1;
	_hmiInputFmt.numChannels = 1;
	_hmiInputFmt.sampleRate = 22050;
	_hmiInputFmt.bitsPerSample = 16;
	_hmiInputFmt.blockAlign = 2;
	_hmiInputFmt.byteRate = 44100;
	_hmiInputFmt.fmtEndSection = 0;

	_hmiOutputFmt.fmtBeginSection = 1;
	_hmiOutputFmt.numChannels = 1;
	_hmiOutputFmt.sampleRate = 22050;
	_hmiOutputFmt.bitsPerSample = 16;
	_hmiOutputFmt.blockAlign = 2;
	_hmiOutputFmt.byteRate = 44100;
	_hmiOutputFmt.fmtEndSection = 0;

	// The original never sets this flag, so it never releases the library
	_hmiFxInitialized = true;
	return 1;
}

int IMuse::registerHMIEffect(const char *effectName) {
	if (_loadedPreset)
		_hmiFxLib->hmiFXPresetRelease(_loadedPreset);

	_hmiRequest.flags = HMI_PROCESS_LOOKUP_ONLY;
	_hmiRequest.effectName = (char *)effectName;
	if (_hmiFxLib->hmiFXPresetProcess(_presetCollection, &_hmiRequest)) {
		_loadedPreset = nullptr;
		_curHMIEffectIdInternal = _hmiOutputFmt.numChannels;
		return 0;
	}

	_loadedPreset = _hmiRequest.presetPtr;

	// The original reads an uninitialized value if the preset has no effects
	uint32 channels = 1;
	_hmiFxLib->hmiFXPresetGetRouting(_loadedPreset, &channels);

	_curHMIEffectIdInternal = channels;
	_hmiOutputFmt.blockAlign = 2 * channels;
	_hmiOutputFmt.numChannels = channels;
	_hmiOutputFmt.byteRate = 44100 * channels;
	_hmiFxLib->hmiFXPresetLock(_loadedPreset);
	return _curHMIEffectIdInternal;
}

int IMuse::processSound(uint8 *srcBuf, uint32 soundLength, uint8 *destBuf, uint8 **outBuf, int32 *outLength, bool isLastChunk) {
	if (_loadedPreset) {
		_hmiRequest.flags = HMI_PROCESS_USE_PRESET | HMI_PROCESS_OVERRIDE_INPUT_FORMAT | HMI_PROCESS_OVERRIDE_OUTPUT_FORMAT;
		if (isLastChunk)
			_hmiRequest.flags |= HMI_PROCESS_ALLOCATE_OUTPUT | HMI_PROCESS_INCLUDE_TAIL;

		_hmiRequest.presetPtr = _loadedPreset;
		_hmiRequest.inputFmt = &_hmiInputFmt;
		_hmiRequest.outputFmt = &_hmiOutputFmt;
		_hmiRequest.srcBuf = srcBuf;
		_hmiRequest.srcLen = soundLength;
		if (!isLastChunk)
			_hmiRequest.dstBuf = destBuf;

		int processResult = _hmiFxLib->hmiFXPresetProcess(_presetCollection, &_hmiRequest);
		if (isLastChunk) {
			// The original would hand back the stale buffer of the previous call on failure
			*outBuf = processResult ? nullptr : _hmiRequest.dstBuf;
			*outLength = processResult ? 0 : _hmiRequest.dstLen;
		}

		return _curHMIEffectIdInternal;
	}

	uint8 *dest;
	if (isLastChunk) {
		*outLength = _hmiOutputFmt.numChannels * soundLength;
		*outBuf = (uint8 *)malloc(*outLength);
		if (!*outBuf)
			return _curHMIEffectIdInternal;

		dest = *outBuf;
	} else {
		dest = destBuf;
	}

	if (!dest)
		return _curHMIEffectIdInternal;

	// Without a preset the samples are just copied, duplicated if the output is stereo
	for (uint32 i = 0; i < soundLength >> 1; i++) {
		memcpy(dest, srcBuf, 2);
		dest += 2;

		if (_hmiOutputFmt.numChannels == 2) {
			memcpy(dest, srcBuf, 2);
			dest += 2;
		}

		srcBuf += 2;
	}

	return _curHMIEffectIdInternal;
}

void IMuse::soundsTerminateProcessing() {
	if (!_hmiFxInitialized)
		return;

	_hmiFxInitialized = false;

	if (_loadedPreset)
		_hmiFxLib->hmiFXPresetRelease(_loadedPreset);

	if (_presetCollection)
		_hmiFxLib->hmiFXPresetFreeLibrary(_presetCollection);

	_hmiFxLib->hmiFXUnInitSystem(&_hmiInitData);
}

} // End of namespace Grim
