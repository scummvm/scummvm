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

#include "scumm/imuse_digi/dimuse_engine.h"

namespace Scumm {

int IMuseDigital::waveOutInit(waveOutParamsStruct *waveOutSettingsStruct) {
	_waveOutBytesPerSample = 2;
	_waveOutNumChannels = _mixer->getOutputStereo() ? 2 : 1;
	_waveOutZeroLevel = 0;

	_waveOutSampleRate = _internalSampleRate;
	_waveOutPreferredFeedSize = _internalFeedSize;

	_waveOutOutputBuffer = nullptr;
	_waveOutMixBuffer = nullptr;
	_waveOutLowLatencyOutputBuffer = nullptr;

	if (!_lowLatencyMode || _isEarlyDiMUSE) {
		// Nine buffers (waveOutPreferredFeedSize * 4 bytes each), two will be used for the mixer
		_waveOutOutputBuffer = (uint8 *)malloc(_waveOutNumChannels * _waveOutBytesPerSample * _waveOutPreferredFeedSize * 9);
		_waveOutMixBuffer = _waveOutOutputBuffer + (_waveOutNumChannels * _waveOutBytesPerSample * _waveOutPreferredFeedSize * 7); // 8-th buffer
	}

	// Replicate another set of buffers for the low latency mode, we will use the previous ones for cutscenes if the mode is active
	if (_lowLatencyMode) {
		_waveOutLowLatencyOutputBuffer = (uint8 *)malloc(_waveOutNumChannels * _waveOutBytesPerSample * _waveOutPreferredFeedSize * 9);
	}

	// This information will be fed to the internal mixer during its initialization
	waveOutSettingsStruct->bytesPerSample = _waveOutBytesPerSample * 8;
	waveOutSettingsStruct->numChannels = _waveOutNumChannels;
	waveOutSettingsStruct->mixBufSize = (_waveOutBytesPerSample * _waveOutNumChannels) * _waveOutPreferredFeedSize;
	waveOutSettingsStruct->sizeSampleKB = 0;
	waveOutSettingsStruct->mixBuf = _waveOutMixBuffer; // Note: in low latency mode this initialization is a dummy

	// Init the buffers filling them with zero volume samples
	if (!_lowLatencyMode || _isEarlyDiMUSE) {
		memset(_waveOutOutputBuffer, _waveOutZeroLevel, _waveOutNumChannels * _waveOutBytesPerSample * _waveOutPreferredFeedSize * 9);
	}

	if (_lowLatencyMode) {
		memset(_waveOutLowLatencyOutputBuffer, _waveOutZeroLevel, _waveOutNumChannels * _waveOutBytesPerSample * _waveOutPreferredFeedSize * 9);
	}

	_waveOutDisableWrite = 0;
	return 0;
}

void IMuseDigital::waveOutWrite(uint8 **audioData, int &feedSize, int &sampleRate) {
	uint8 *curBufferBlock;
	if (_waveOutDisableWrite)
		return;

	feedSize = 0;

	// A bit of context for what follows:
	//
	// In COMI, when entering certain rooms (e.g. barber shop in Plunder Island), the game
	// sends a State music event followed immediately by a Sequence music event.
	// If the iMUSE callback happens to run between these two commands, it can cause a brief
	// audio glitch where a few milliseconds of the intermediate state music is heard
	// before being replaced by the sequence music, with the fade in between canceled.
	//
	// To prevent this, we skip the very first audio callback after any music change
	// by setting _waveOutXorTrigger to 1 in playComiMusic(). This gives the scripts
	// enough time to send any follow-up sequence events before iMUSE processes
	// the audio transition, ensuring the absence of glitches and most importantly
	// no permanent audio delay on other audio feeds.
	//
	// This is very strongly inspired by what the disassembly does:
	//
	// The original DirectSound implementation uses the XOR flag combined with play
	// cursor position checks that naturally skip audio processing cycles when the
	// play cursor gets too close to the write cursor. In addition to this, the main
	// portion of tracksCallback() is executed in loop until feedSide is finally 0.
	// This creates the exact timing conditions where rapid music events have sufficient
	// time to accumulate before iMUSE processes any audio transitions.
	//
	// We're not using DirectSound, so recreate this I could either simulate the
	// play cursor handling shenanigans, or I could just do exactly what I have done :-)
	if (_vm->_game.id == GID_CMI && _waveOutXorTrigger != 0) {
		_waveOutXorTrigger = 0;
		return;
	}

	if (!_isEarlyDiMUSE && _vm->_game.id == GID_DIG) {
		_waveOutXorTrigger ^= 1;
		if (!_waveOutXorTrigger)
			return;
	}

	if (_mixer->isReady()) {
		curBufferBlock = &_waveOutOutputBuffer[_waveOutPreferredFeedSize * _waveOutWriteIndex * _waveOutBytesPerSample * _waveOutNumChannels];

		*audioData = curBufferBlock;

		sampleRate = _waveOutSampleRate;
		feedSize = _waveOutPreferredFeedSize;
		_waveOutWriteIndex = (_waveOutWriteIndex + 1) % 7;

		byte *ptr = (byte *)malloc(_outputFeedSize * _waveOutBytesPerSample * _waveOutNumChannels);
		memcpy(ptr, curBufferBlock, _outputFeedSize * _waveOutBytesPerSample * _waveOutNumChannels);

		_internalMixer->getStream(-1)->queueBuffer(ptr,
			_outputFeedSize * _waveOutBytesPerSample * _waveOutNumChannels,
			DisposeAfterUse::YES,
			waveOutGetStreamFlags());

	}
}

int IMuseDigital::waveOutDeinit() {
	_waveOutDisableWrite = 1;
	return 0;
}

void IMuseDigital::waveOutCallback() {
	Common::StackLock lock(*_mutex);
	if (_lowLatencyMode) {
		tracksLowLatencyCallback();
	} else {
		tracksCallback();
	}
}

byte IMuseDigital::waveOutGetStreamFlags() {
	byte flags = Audio::FLAG_16BITS;

	if (_mixer->getOutputStereo()) {
		flags |= Audio::FLAG_STEREO;
	}

#ifdef SCUMM_LITTLE_ENDIAN
	flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
	return flags;
}

void IMuseDigital::waveOutLowLatencyWrite(uint8 **audioData, int &feedSize, int &sampleRate, int idx) {
	uint8 *curBufferBlock;
	if (_waveOutDisableWrite)
		return;

	if (!_isEarlyDiMUSE && _vm->_game.id == GID_DIG) {
		_waveOutXorTrigger ^= 1;
		if (!_waveOutXorTrigger)
			return;
	}

	feedSize = 0;
	if (_mixer->isReady()) {
		curBufferBlock = &_waveOutLowLatencyOutputBuffer[_waveOutPreferredFeedSize * idx * _waveOutBytesPerSample * _waveOutNumChannels];

		*audioData = curBufferBlock;

		sampleRate = _waveOutSampleRate;
		feedSize = _waveOutPreferredFeedSize;

		byte *ptr = (byte *)malloc(_outputFeedSize * _waveOutBytesPerSample * _waveOutNumChannels);
		memcpy(ptr, curBufferBlock, _outputFeedSize * _waveOutBytesPerSample * _waveOutNumChannels);

		_internalMixer->getStream(idx)->queueBuffer(ptr,
			 _outputFeedSize * _waveOutBytesPerSample * _waveOutNumChannels,
			 DisposeAfterUse::YES,
			 waveOutGetStreamFlags());
	}
}

void IMuseDigital::waveOutEmptyBuffer(int idx) {
	// This is necessary in low latency mode to clean-up the buffers of stale/finished sounds
	int bufferSize = _waveOutNumChannels * _waveOutBytesPerSample * _waveOutPreferredFeedSize;
	memset(&_waveOutLowLatencyOutputBuffer[bufferSize * idx], _waveOutZeroLevel, bufferSize);
}

} // End of namespace Scumm
