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

#include "common/config-manager.h"
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

const char *SOUND_FILENAMES[] = {
	"NO_SOUND.SMP", // 0 - Silence/disabled
	"BUHO_ZZZ.SMP", // 1 - Owl
	"BIRD_1_1.SMP", // 2 - Bird variant 1
	"BIRD_1_2.SMP", // 3 - Bird variant 2
	"BIRD_1_3.SMP", // 4 - Bird variant 3
	"DESPERZZ.SMP", // 5 - Yawn/stretch
	"HORN_5ZZ.SMP", // 6 - Car horn 5
	"HORN_6ZZ.SMP", // 7 - Car horn 6
	"HORN_8ZZ.SMP", // 8 - Car horn 8
	"SUZIPASS.SMP", // 9 - Suzi passing
	"CAT_1ZZZ.SMP", // 10 - Cat
	"DOG_01ZZ.SMP", // 11 - Dog bark 1
	"DOG_02ZZ.SMP", // 12 - Dog bark 2
	"DOG_04ZZ.SMP", // 13 - Dog bark 4
	"DOG_05ZZ.SMP", // 14 - Dog bark 5
	"DOG_06ZZ.SMP", // 15 - Dog bark 6
	"DOG_07ZZ.SMP", // 16 - Dog bark 7
	"DOG_09ZZ.SMP", // 17 - Dog bark 9
	"ALARMZZZ.SMP", // 18 - Alarm
	"AMBULAN1.SMP", // 19 - Ambulance
	"FOUNTAIN.SMP", // 20 - Fountain
	"GRILLOSZ.SMP", // 21 - Crickets
	"HOJASZZZ.SMP", // 22 - Leaves rustling
	"FLASHZZZ.SMP", // 23 - Flash/camera
	"CUCHI1ZZ.SMP", // 24 - Knife 1
	"KNRRRRRZ.SMP", // 25 - Snoring
	"PHONE_02.SMP", // 26 - Phone ring 2
	"PHONE_03.SMP", // 27 - Phone ring 3
	"SSSHTZZZ.SMP", // 28 - Shush/quiet
	"BURGUER1.SMP", // 29 - Burger sizzle
	"FLIES_2Z.SMP", // 30 - Flies buzzing
	"PARRILLA.SMP", // 31 - Grill
	"WATER_2Z.SMP", // 32 - Water
	"XIQUETZZ.SMP", // 33 - Whistle
	"RONQUIZZ.SMP", // 34 - Snoring
	"MOCO1ZZZ.SMP", // 35 - Snot/mucus 1
	"MOCO2ZZZ.SMP", // 36 - Snot/mucus 2
	"SPRINGZZ.SMP", // 37 - Spring bounce
	"MARUJASZ.SMP", // 38 - Gossip/chatter
	"ELECTROZ.SMP", // 39 - Electric shock
	"GLASS1ZZ.SMP", // 40 - Glass clink
	"OPDOORZZ.SMP", // 41 - Door open
	"CLDOORZZ.SMP", // 42 - Door close
	"FXH2ZZZZ.SMP", // 43 - Effect 2
	"BOTEZZZZ.SMP", // 44 - Bottle
	"ELEC3ZZZ.SMP", // 45 - Electric 3
	"AJARLZZZ.SMP", // 46 - Ajar/creak
	"BELCHZZZ.SMP", // 47 - Belch/burp
	"64ZZZZZZ.SMP", // 48 - Sound effect 64
	"BIRDOWL2.SMP", // 49 - Bird/owl 2
	"BUBBLE2Z.SMP", // 50 - Bubbles
	"BURGUER1.SMP", // 51 - Burger (duplicate)
	"CACKLEZZ.SMP", // 52 - Cackle/laugh
	"CERAMIC1.SMP", // 53 - Ceramic break
	"CLANG5ZZ.SMP", // 54 - Metal clang
	"CUCHI2ZZ.SMP", // 55 - Knife 2
	"CUCHI3ZZ.SMP", // 56 - Knife 3
	"ELEC3ZZZ.SMP", // 57 - Electric 3 (duplicate)
	"HOJASZZZ.SMP", // 58 - Leaves (duplicate)
	"LIMA1ZZZ.SMP", // 59 - File/rasp
	"MOROSZZZ.SMP", // 60 - Moors/crowd
	"MOROZZZZ.SMP", // 61 - Moor/crowd
	"MUD1ZZZZ.SMP", // 62 - Mud squelch
	"PICOZZZZ.SMP", // 63 - Pickaxe
	"PICO1XZZ.SMP", // 64 - Pickaxe 1
	"PICO2XZZ.SMP", // 65 - Pickaxe 2
	"PICO3XZZ.SMP", // 66 - Pickaxe 3
	"RIMSHOTZ.SMP", // 67 - Rimshot drum
	"RONCOZZZ.SMP", // 68 - Snoring
	"SORBOZZZ.SMP", // 69 - Slurp/sip
	"VIENTO1Z.SMP", // 70 - Wind
	"2ZZZZZZZ.SMP", // 71 - Sound 2
	"20ZZZZZZ.SMP", // 72 - Sound 20
	"21ZZZZZZ.SMP", // 73 - Sound 21
	"23ZZZZZZ.SMP", // 74 - Sound 23
	"107ZZZZZ.SMP", // 75 - Sound 107
	"39ZZZZZZ.SMP", // 76 - Sound 39
	"81ZZZZZZ.SMP", // 77 - Sound 81
	"88ZZZZZZ.SMP", // 78 - Sound 88
	"92ZZZZZZ.SMP", // 79 - Sound 92
	"SAW_2ZZZ.SMP", // 80 - Saw
	"QUAKE2ZZ.SMP", // 81 - Earthquake
	"ROCKSZZZ.SMP", // 82 - Rocks falling
	"IN_FIREZ.SMP", // 83 - Fire
	"BEAMZZZZ.SMP", // 84 - Beam/ray
	"GLISSDWN.SMP", // 85 - Glissando down
	"REMATERL.SMP", // 86 - Rematerialize
	"FXH1ZZZZ.SMP", // 87 - Effect 1
	"FXH3ZZZZ.SMP", // 88 - Effect 3
	"FXH4ZZZZ.SMP", // 89 - Effect 4
	"MATCHZZZ.SMP", // 90 - Match strike
	"SURF_01Z.SMP", // 91 - Surf wave 1
	"SURF_02Z.SMP", // 92 - Surf wave 2
	"SURF_04Z.SMP", // 93 - Surf wave 4
	"TWANGZZZ.SMP", // 94 - Twang
	"LANDCRAS.SMP", // 95 - Crash landing
};

SoundManager::SoundManager(Audio::Mixer *mixer)
	: _mixer(mixer), _currentVolume(128) {
	// TODO: Initialize sound manager
	g_system->getAudioCDManager()->open();
}

SoundManager::~SoundManager() {
	stopAllSounds();
	stopMusic();
}

void SoundManager::playSound(byte index, int channel, int loopCount) {
	// debug("Playing sound index %d (%s)", index, SOUND_FILENAMES[index]);
	auto it = _soundMap.find(SOUND_FILENAMES[index]);
	if (it != _soundMap.end()) {
		playSound(it->_value, channel, loopCount);
	} else {
		debug("Sound file %s not found in sound map", SOUND_FILENAMES[index]);
	}
}

void SoundManager::playSound(const char *filename, int channel, int loopCount) {
	auto it = _soundMap.find(filename);
	if (it != _soundMap.end()) {
		playSound(it->_value, channel, loopCount);
	} else {
		debug("Sound file %s not found in sound map", filename);
	}
}

void SoundManager::playSound(SonidoFile sound, int channel, int loopCount) {
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
	uint32 sampleRate = getSampleRate(data, format);
	Audio::SeekableAudioStream *stream = nullptr;

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
				debug("Stopped active sound on channel %d to play new sound %s", channel, sound.filename.c_str());
			}
		}
		Audio::AudioStream *finalStream = loopCount != -1 ? stream : Audio::makeLoopingAudioStream(stream, 0);

		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[channel], finalStream, -1, _currentVolume, 0, DisposeAfterUse::YES);
	}
}

void SoundManager::playSound(byte *soundData, uint32 size, int channel) {
	Audio::AudioStream *stream = Audio::makeRawStream(soundData, size, 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	if (stream) {
		if (_mixer->isSoundHandleActive(_sfxHandles[channel])) {
				_mixer->stopHandle(_sfxHandles[channel]);
		}
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[channel], stream, -1, _currentVolume, 0, DisposeAfterUse::YES);
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

void SoundManager::setVolumeSfx(int volume) {
	ConfMan.setInt("sfx_volume", volume);
	g_engine->syncSoundSettings();
}

void SoundManager::setVolumeMusic(int volume) {
	ConfMan.setInt("music_volume", volume);
	g_engine->syncSoundSettings();
}

void SoundManager::setVolumeMaster(int volume) {
	ConfMan.setInt("sfx_volume", volume);
	ConfMan.setInt("music_volume", volume);
	ConfMan.setInt("speech_volume", volume);
	g_engine->syncSoundSettings();
}

int SoundManager::getVolumeSfx() const {
	return ConfMan.getInt("sfx_volume");
}

int SoundManager::getVolumeMusic() const {
	return ConfMan.getInt("music_volume");
}

int SoundManager::getVolumeMaster() const {
	// Master is the minimum of the channel volumes
	return MIN(ConfMan.getInt("sfx_volume"), ConfMan.getInt("music_volume"));
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
	_isPaused = false;
	g_system->getAudioCDManager()->stop();
}

void SoundManager::pauseMusic() {
	uint32 elapsed = g_system->getMillis() - _cdPlayStartTime;
	uint32 elapsedFrames = elapsed * 75 / 1000;
	_cdTrackStart += elapsedFrames; // advance the start offset
	if (_cdTrackDuration > 0)
    	_cdTrackDuration -= elapsedFrames; // shrink remaining duration
	g_system->getAudioCDManager()->stop();
	_isPaused = true;
}

bool SoundManager::isMusicPlaying() {
	return g_system->getAudioCDManager()->isPlaying();
}

void SoundManager::playMusicTrack(int trackNumber, bool loop) {
	if (!_isPaused && _currentMusicTrack == trackNumber && isMusicPlaying()) {
		// Already playing this track
		debug("Track %d is already playing", trackNumber);
		return;
	}
	_currentMusicTrack = trackNumber;
	debug("Playing music track %d, loop=%d", trackNumber, loop);

	if(!_isPaused) {
		_cdTrackStart = 0;
		_cdTrackDuration = 0;
		_cdPlayStartTime = g_system->getMillis();
	}
	g_system->getAudioCDManager()->stop();
	g_system->getAudioCDManager()->play(trackNumber, loop ? -1 : 1, _cdTrackStart, _cdTrackDuration);
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

	for (uint32 i = 0; i < fileCount; i++) {
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
