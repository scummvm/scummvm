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

int IMuseDigital::waveOutInit(int sampleRate, waveOutParamsStruct *waveOutSettingsStruct) {
	_waveOutSampleRate = sampleRate;

	_waveOutBytesPerSample = 2;
	_waveOutNumChannels = 2;
	_waveOutZeroLevel = 0;
	_waveOutPreferredFeedSize = DIMUSE_FEEDSIZE;

	// Nine buffers (waveOutPreferredFeedSize * 4 bytes each), two will be used for the mixer
	_waveOutOutputBuffer = (uint8 *)malloc(_waveOutNumChannels * _waveOutBytesPerSample * _waveOutPreferredFeedSize * 9);
	_waveOutMixBuffer = _waveOutOutputBuffer + (_waveOutNumChannels * _waveOutBytesPerSample * _waveOutPreferredFeedSize * 7); // 8-th buffer

	// This information will be fed to the internal mixer during its initialization
	waveOutSettingsStruct->bytesPerSample = _waveOutBytesPerSample * 8;
	waveOutSettingsStruct->numChannels = _waveOutNumChannels;
	waveOutSettingsStruct->mixBufSize = (_waveOutBytesPerSample * _waveOutNumChannels) * _waveOutPreferredFeedSize;
	waveOutSettingsStruct->sizeSampleKB = 0;
	waveOutSettingsStruct->mixBuf = _waveOutMixBuffer;

	// Init the buffer filling it with zero volume samples
	memset(_waveOutOutputBuffer, _waveOutZeroLevel, _waveOutNumChannels * _waveOutBytesPerSample * _waveOutPreferredFeedSize * 9);

	_waveOutDisableWrite = 0;
	return 0;
}

void IMuseDigital::waveOutWrite(uint8 **audioData, int &feedSize, int &sampleRate) {
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
		curBufferBlock = &_waveOutOutputBuffer[_waveOutPreferredFeedSize * _waveOutWriteIndex * _waveOutBytesPerSample * _waveOutNumChannels];

		*audioData = curBufferBlock;

		sampleRate = _waveOutSampleRate;
		feedSize = _waveOutPreferredFeedSize;
		_waveOutWriteIndex = (_waveOutWriteIndex + 1) % 7;

		byte *ptr = (byte *)malloc(_outputFeedSize * _waveOutBytesPerSample * _waveOutNumChannels);
		memcpy(ptr, curBufferBlock, _outputFeedSize * _waveOutBytesPerSample * _waveOutNumChannels);

		_internalMixer->_stream->queueBuffer(ptr,
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
	Common::StackLock lock(_mutex);
	tracksCallback();
}

byte IMuseDigital::waveOutGetStreamFlags() {
	byte flags = Audio::FLAG_16BITS | Audio::FLAG_STEREO;
#ifdef SCUMM_LITTLE_ENDIAN
	flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
	return flags;
}

} // End of namespace Scumm
