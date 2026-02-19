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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/scummsys.h"

#include "backends/audiocd/audiocd.h"

#include "pelrock/pelrock.h"
#include "pelrock/sound.h"
#include "sound.h"

namespace Pelrock {

SoundManager::SoundManager(Audio::Mixer *mixer)
	: _mixer(mixer), _currentVolume(128), _musicFile(nullptr) {
	// TODO: Initialize sound manager
	g_system->getAudioCDManager()->open();
}

SoundManager::~SoundManager() {
	stopAllSounds();
	stopMusic();
}

void SoundManager::playSound(byte index, int channel) {
	// debug("Playing sound index %d (%s)", index, SOUND_FILENAMES[index]);
	auto it = _soundMap.find(SOUND_FILENAMES[index]);
	if (it != _soundMap.end()) {
		playSound(it->_value, channel);
	} else {
		debug("Sound file %s not found in sound map", SOUND_FILENAMES[index]);
	}
}

void SoundManager::playSound(const char *filename, int channel) {
	auto it = _soundMap.find(filename);
	if (it != _soundMap.end()) {
		playSound(it->_value, channel);
	} else {
		debug("Sound file %s not found in sound map", filename);
	}
}

void SoundManager::playSound(SonidoFile sound, int channel) {
	Common::File sonidosFile;
	if (!sonidosFile.open(Common::Path("SONIDOS.DAT"))) {
		debug("Failed to open SONIDOS.DAT");
		return;
	}

	sonidosFile.seek(sound.offset, SEEK_SET);
	byte *data = new byte[sound.size];
	sonidosFile.read(data, sound.size);
	sonidosFile.close();

	SoundFormat format = detectFormat(data, sound.size);
	uint32_t sampleRate = getSampleRate(data, format);
	Audio::AudioStream *stream = nullptr;

	if (format == SOUND_FORMAT_RIFF) {
		// For WAV/RIFF files, use the wave decoder
		Common::MemoryReadStream *memStream = new Common::MemoryReadStream(data, sound.size, DisposeAfterUse::YES);
		stream = Audio::makeWAVStream(memStream, DisposeAfterUse::YES);
	} else if (format == SOUND_FORMAT_RAWPCM || format == SOUND_FORMAT_MILES || format == SOUND_FORMAT_MILES2) {
		// Determine the offset to skip the header
		uint32 headerSize = 0;
		if (format == SOUND_FORMAT_MILES || format == SOUND_FORMAT_MILES2) {
			headerSize = 80;
		}

		uint32 pcmSize = sound.size - headerSize;
		byte *pcmData = (byte *)malloc(pcmSize);
		memcpy(pcmData, data + headerSize, pcmSize);
		delete[] data;

		// Create raw audio stream (8-bit unsigned mono is common for old games)
		stream = Audio::makeRawStream(pcmData, pcmSize, sampleRate, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	} else {
		debug("Unknown sound format on sound with name %s at offset %d, with size %d", sound.filename.c_str(), sound.offset, sound.size);
		delete[] data;
		return;
	}

	if (stream) {
		if (channel == -1) {
			// Find a free channel
			channel = findFreeChannel();
		} else {
			if (_mixer->isSoundHandleActive(_sfxHandles[channel])) {
				_mixer->stopHandle(_sfxHandles[channel]);
			}
		}
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[channel], stream, -1, 255U, 0, DisposeAfterUse::YES);
	}
}

void SoundManager::playSound(byte *soundData, uint32 size) {
	Audio::AudioStream *stream = Audio::makeRawStream(soundData, size, 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	if (stream) {
		int channel = findFreeChannel();
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[channel], stream, -1, 255U, 0, DisposeAfterUse::YES);
	}
}

SoundFormat SoundManager::detectFormat(byte *data, uint32 size) {

	if (size < 16) {
		return SOUND_FORMAT_INVALID;
	}
	byte byte0 = data[0];
	byte byte1 = data[1];

	if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F') {
		return SOUND_FORMAT_RIFF;
	}
	if (byte0 == 0x01 && byte1 == 0x2e) {
		return SOUND_FORMAT_MILES;
	}
	if (byte0 == 0x01 && (byte1 >= 0x40 && byte1 <= 0x7f)) {
		return SOUND_FORMAT_MILES2;
	}
	if (size <= 100) {
		return SOUND_FORMAT_INVALID;
	}
	return SOUND_FORMAT_RAWPCM;
}

int SoundManager::getSampleRate(byte *data, SoundFormat format) {

	uint32 sampleRate = 11025; // Default sample rate
	if (format == SOUND_FORMAT_RIFF) {
		sampleRate = READ_LE_UINT32(data + 0x18);
	} else if (format == SOUND_FORMAT_MILES) {
		sampleRate = READ_LE_UINT32(data + 0x1C);
	} else if (format == SOUND_FORMAT_MILES2) {
		sampleRate = READ_LE_UINT32(data + 0x10);
	}
	return sampleRate;
}

int SoundManager::findFreeChannel() {
	// Reserve first 3 channels for one-off sounds
	for (int i = 3; i < kMaxChannels; i++) {
		if (!_mixer->isSoundHandleActive(_sfxHandles[i])) {
			return i;
		}
	}
	return 0;
}

void SoundManager::stopAllSounds() {
	for (int i = 0; i < kMaxChannels; i++) {
		_mixer->stopHandle(_sfxHandles[i]);
	}
}

void SoundManager::stopSound(int channel) {
	if (channel >= 0 && channel < kMaxChannels) {
		_mixer->stopHandle(_sfxHandles[channel]);
	}
}

void SoundManager::setVolume(int volume) {
	// TODO: Set sound volume
}

bool SoundManager::isPlaying() const {
	for (int i = 0; i < 8; i++) {
		if (_mixer->isSoundHandleActive(_sfxHandles[i])) {
			return true;
		}
	}
	return false;
}

bool SoundManager::isPlaying(int channel) const {
	if (channel >= 0 && channel < kMaxChannels) {
		return _mixer->isSoundHandleActive(_sfxHandles[channel]);
	}
	return false;
}

void SoundManager::stopMusic() {
	g_system->getAudioCDManager()->stop();
}

bool SoundManager::isMusicPlaying() {
	return g_system->getAudioCDManager()->isPlaying();
}

void SoundManager::playMusicTrack(int trackNumber, bool loop) {
	if (_currentMusicTrack == trackNumber && isMusicPlaying()) {
		// Already playing this track
		return;
	}
	_currentMusicTrack = trackNumber;
	g_system->getAudioCDManager()->stop();
	g_system->getAudioCDManager()->play(trackNumber, loop ? -1 : 0, 0, 0);
}

void SoundManager::loadSoundIndex() {

	Common::File sonidosFile;
	if (!sonidosFile.open(Common::Path("SONIDOS.DAT"))) {
		debug("Failed to open SONIDOS.DAT");
		return;
	}
	// Read header
	char magic[4];
	sonidosFile.read(magic, 4);
	if (strncmp(magic, "PACK", 4) != 0) {
		debug("SONIDOS.DAT has invalid magic");
		return;
	}
	byte fileCount = sonidosFile.readByte();
	debug("SONIDOS.DAT contains %u files", fileCount);
	sonidosFile.skip(3); // Padding bytes

	for (uint32_t i = 0; i < fileCount; i++) {
		SonidoFile sonido;
		sonido.filename = sonidosFile.readString('\0', 12);
		sonidosFile.skip(1);
		sonido.offset = sonidosFile.readUint32LE();
		sonido.size = sonidosFile.readUint32LE();
		_soundMap[sonido.filename] = sonido;
	}
	sonidosFile.close();
}

static const uint kAmbientCounterMask = 0x1F; // Trigger when (counter & mask) == mask

int SoundManager::tickAmbientSound(uint32 frameCount) {
	// Counter gate: only trigger every 32 frames when (counter & 0x1F) == 0x1F
	if ((frameCount & kAmbientCounterMask) != kAmbientCounterMask) {
		return -1;
	}

	// 50% probability gate using ScummVM's random source
	if (g_engine->getRandomNumber(1) == 0) {
		return -1;
	}

	// Pick random ambient slot 0-3 (corresponds to room sound indices 4-7)
	int ambientSlotOffset = g_engine->getRandomNumber(3);

	return ambientSlotOffset; // Caller adds 4 to get room sound index
}

} // End of namespace Pelrock
