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
	: _mixer(mixer), _currentVolume(255), _musicFile(nullptr) {
	// TODO: Initialize sound manager
	g_system->getAudioCDManager()->open();
}

SoundManager::~SoundManager() {
	stopAllSounds();
	stopMusic();
}

void SoundManager::playSound(byte index, int volume) {
	// debug("Playing sound index %d (%s)", index, SOUND_FILENAMES[index]);
	auto it = _soundMap.find(SOUND_FILENAMES[index]);
	if (it != _soundMap.end()) {
		playSound(it->_value, volume);
	} else {
		debug("Sound file %s not found in sound map", SOUND_FILENAMES[index]);
	}
}

void SoundManager::playSound(SonidoFile sound, int volume) {
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
		stream = Audio::makeRawStream(pcmData, pcmSize, sampleRate,
									  Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	} else {
		debug("Unknown sound format");
		delete[] data;
		return;
	}

	// if (stream) {
	// 	int channel = findFreeChannel();
	// 	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[channel], stream, -1, volume, 0, DisposeAfterUse::YES);
	// }
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
	for (int i = 0; i < kMaxChannels; i++) {
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

void SoundManager::playMusicTrack(int trackNumber) {
	if (_currentMusicTrack == trackNumber && isMusicPlaying()) {
		// Already playing this track
		return;
	}
	_currentMusicTrack = trackNumber;
	g_system->getAudioCDManager()->stop();
	g_system->getAudioCDManager()->play(trackNumber, -1, 0, 0);
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

int RANDOM_THRESHOLD = 0x4000;

int SoundManager::tick(uint32 frameCount) {

	uint16 rand1 = _rng.nextRandom();
	// uint32 random = g_engine->getRandomNumber(1);
	if (rand1 <= RANDOM_THRESHOLD) {
		// debug("No SFX this tick due to 50% random");
		return -1;
	}

	if ((frameCount & COUNTER_MASK) != COUNTER_MASK) {
		// debug("No SFX this tick due to counter mask (counter = %d)", soundFrameCounter);
		return -1;
	}

	uint16 rand2 = _rng.nextRandom();
	int slot = rand2 & 3;
	// debug("Slot = %d (rand2 = %u)", slot, rand2);
	// uint32 slot = g_engine->getRandomNumber(4);
	return slot + 1;
}

} // End of namespace Pelrock
