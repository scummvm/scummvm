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

#include "lastexpress/lastexpress.h"

#include "audio/decoders/raw.h"

namespace LastExpress {

void SoundManager::soundDriverInit() {
	_stream = Audio::makeQueuingAudioStream(22050, _engine->_mixer->getOutputStereo());
	memset(_soundMixBuffer, 0, 1470 * sizeof(int16));
	_mixer->playStream(
		Audio::Mixer::kPlainSoundType,
		&_channelHandle,
		_stream,
		-1,
		Audio::Mixer::kMaxChannelVolume
	);

	// The following is a way to adapt the amount of buffers to the current device capabilities.
	// Re-adapted from my own code found within the Digital iMUSE subsystem from the SCUMM engine.
	_maxQueuedStreams = (uint32)ceil((_mixer->getOutputBufSize() / 1470) / ((float)_mixer->getOutputRate() / 44100));

	if (_mixer->getOutputRate() % 44100) {
		_maxQueuedStreams++;
	}

	_maxQueuedStreams = MAX<uint32>(4, _maxQueuedStreams);
}

void SoundManager::soundDriverCopyBuffersToDevice() {
	_sound30HzCounter++;

	while (_stream->numQueuedStreams() < _maxQueuedStreams) {
		byte *ptr = (byte *)malloc(1470 * sizeof(int16));
		assert(ptr);

		mixEngine();
		memcpy(ptr, _soundMixBuffer, 1470 * sizeof(int16));

		byte flags = Audio::FLAG_16BITS;

		if (_mixer->getOutputStereo()) {
			flags |= Audio::FLAG_STEREO;
		}

#ifdef SCUMM_LITTLE_ENDIAN
		flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif

		_stream->queueBuffer(ptr, 1470 * sizeof(int16), DisposeAfterUse::YES, flags);
	}
}

int SoundManager::soundDriverGetVolume() {
	// I know, this thing could have been a look-up table :-)
	// Still, I want this to be completely clear and transparent.

	int scummVMVolume = _mixer->getChannelVolume(_channelHandle);

	// Convert driver volume (0-255) to DirectSound format (-10000 to 0)
	double dsVolume;
	if (scummVMVolume == 0) {
		dsVolume = -10000; // Silence
	} else {
		// Convert from linear scale to dB
		dsVolume = (2000 * log10((double)scummVMVolume / 255.0));
	}

	// Convert DirectSound scale to game scale (0-7)
	int engineVolume;
	if (dsVolume <= -3000) {
		engineVolume = 0;
	} else {
		engineVolume = (int)round((7 * (dsVolume + 3000)) / 3000);
	}

	return engineVolume;
}

void SoundManager::soundDriverSetVolume(int volume) {
	// Convert from game scale (0-7) to DirectSound scale (-10000 to 0)
	int32 dsVolume;
	if (volume == 0) {
		dsVolume = -10000; // Silence
	} else {
		dsVolume = 3000 * volume / 7 - 3000;
	}

	// Convert DirectSound volume to ScummVM volume (0-255)
	int scummVMVolume = (int)round(pow(10, dsVolume / 2000.0) * 255.0);
	assert(scummVMVolume >= 0 && scummVMVolume < 256);

	_mixer->setChannelVolume(_channelHandle, (byte)scummVMVolume);
}

int32 SoundManager::getSoundDriverTicks() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _soundDriverTicks;
}

void SoundManager::setSoundDriverTicks(int32 value) {
	Common::StackLock lock(*_engine->_soundMutex);
	_soundDriverTicks = value;
}

int32 SoundManager::getSoundDriver30HzCounter() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _sound30HzCounter;
}

int32 SoundManager::getSoundDriverFlags() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _soundDriverFlags;
}

void SoundManager::addSoundDriverFlags(int32 flags) {
	Common::StackLock lock(*_engine->_soundMutex);
	_soundDriverFlags |= flags;
}

void SoundManager::removeSoundDriverFlags(int32 flags) {
	Common::StackLock lock(*_engine->_soundMutex);
	_soundDriverFlags &= ~flags;
}

bool SoundManager::isCopyingDataToSoundDriver() {
	Common::StackLock lock(*_engine->_soundMutex);
	return _copyingDataToSoundDriver;
}

} // End of namespace LastExpress
