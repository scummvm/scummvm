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

#include <cstdlib>
#include <cstring>

#include "harvester/media_manager.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "common/algorithm.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/system.h"
#include "graphics/screen.h"
#include "harvester/art.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/script.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

enum class FcmpWarmupMode {
	kMusic,
	kSample
};

static const int8 kHarvesterImaIndexAdjustTable[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

static const uint16 kHarvesterImaStepTable[89] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int clampStartupOptionLevel(int level) {
	if (level < 0)
		return 0;
	if (level > 9)
		return 9;

	return level;
}

static int mapStartupVolumeLevelToMixerVolume(int level) {
	return (clampStartupOptionLevel(level) * Audio::Mixer::kMaxMixerVolume + 4) / 9;
}

static uint32 decodeHarvesterFcmp(byte *dest, const byte *src, uint32 srcSize, uint16 bitsPerSample) {
	const uint32 decodedBytesPerInputByte = bitsPerSample >> 2;
	const uint32 decodedSize = srcSize * decodedBytesPerInputByte;
	const uint32 sampleCount = (bitsPerSample == 16) ? (decodedSize >> 1) : decodedSize;
	int predictor = 0;
	int stepIndex = 0;
	int stepSize = 7;
	byte cachedByte = 0;
	uint32 srcPos = 0;

	for (uint32 sample = 0; sample < sampleCount; ++sample) {
		const byte nibble = ((sample & 1) == 0)
			? ((cachedByte = src[srcPos++]) & 0x0f)
			: ((cachedByte >> 4) & 0x0f);
		int delta = stepSize >> 3;
		if (nibble & 4)
			delta += stepSize;
		if (nibble & 2)
			delta += stepSize >> 1;
		if (nibble & 1)
			delta += stepSize >> 2;
		if (nibble & 8)
			delta = -delta;

		predictor += delta;
		if (predictor > 0x7fff)
			predictor = 0x7fff;
		else if (predictor < -0x8000)
			predictor = -0x8000;

		if (bitsPerSample == 16) {
			WRITE_LE_INT16(dest, predictor);
			dest += 2;
		} else {
			*dest++ = (byte)(((predictor >> 8) & 0xff) ^ 0x80);
		}

		stepIndex += kHarvesterImaIndexAdjustTable[nibble];
		if (stepIndex < 0)
			stepIndex = 0;
		else if (stepIndex > 88)
			stepIndex = 88;
		stepSize = kHarvesterImaStepTable[stepIndex];
	}

	return decodedSize;
}

static void applyStartupFcmpWarmup(byte *decodedPcm, uint32 &decodedSize, FcmpWarmupMode warmupMode) {
	if (!decodedPcm || decodedSize == 0)
		return;

	if (warmupMode == FcmpWarmupMode::kMusic) {
		// Native start_music_stream zeros the first 0x64 decoded bytes of the first FCMP chunk.
		memset(decodedPcm, 0, MIN<uint32>(decodedSize, 100));
		return;
	}

	// Native load_sound_sample/load_dialogue_voice_sample queue from decoded_pcm + 0x1f4.
	const uint32 trimBytes = MIN<uint32>(decodedSize, 500);
	if (trimBytes == decodedSize) {
		memset(decodedPcm, 0, decodedSize);
		return;
	}

	memmove(decodedPcm, decodedPcm + trimBytes, decodedSize - trimBytes);
	decodedSize -= trimBytes;
}

static Audio::SeekableAudioStream *decodeStartupAudioStream(Common::SeekableReadStream *stream,
		FcmpWarmupMode warmupMode) {
	if (!stream)
		return nullptr;

	if (stream->size() >= 14) {
		char magic[4];
		stream->read(magic, sizeof(magic));
		if (!memcmp(magic, "FCMP", sizeof(magic))) {
			const uint32 payloadSizeFromHeader = stream->readUint32LE();
			const uint32 sampleRate = stream->readUint32LE();
			const uint16 bitsPerSample = stream->readUint16LE();
			const uint32 availablePayloadSize = stream->size() - 14;
			const uint32 payloadSize = payloadSizeFromHeader != 0
				? MIN<uint32>(payloadSizeFromHeader, availablePayloadSize)
				: availablePayloadSize;

			if (sampleRate == 0 || (bitsPerSample != 8 && bitsPerSample != 16) || payloadSize == 0) {
				delete stream;
				return nullptr;
			}

			if (!stream->seek(14)) {
				delete stream;
				return nullptr;
			}

			byte *compressedPayload = (byte *)malloc(payloadSize);
			if (!compressedPayload) {
				delete stream;
				return nullptr;
			}
			if (stream->read(compressedPayload, payloadSize) != payloadSize) {
				free(compressedPayload);
				delete stream;
				return nullptr;
			}

			uint32 decodedSize = payloadSize * (bitsPerSample >> 2);
			byte *decodedPcm = (byte *)malloc(decodedSize);
			if (!decodedPcm) {
				free(compressedPayload);
				delete stream;
				return nullptr;
			}

			decodeHarvesterFcmp(decodedPcm, compressedPayload, payloadSize, bitsPerSample);
			applyStartupFcmpWarmup(decodedPcm, decodedSize, warmupMode);
			free(compressedPayload);
			delete stream;
			return Audio::makeRawStream(decodedPcm, decodedSize, sampleRate,
				Audio::FLAG_LITTLE_ENDIAN | ((bitsPerSample == 16) ? Audio::FLAG_16BITS : 0),
				DisposeAfterUse::YES);
		}
	}

	stream->seek(0);
	return Audio::makeWAVStream(stream, DisposeAfterUse::YES);
}

static Audio::SeekableAudioStream *openStartupAudioStream(ResourceManager &resources,
		const Common::String &path, FcmpWarmupMode warmupMode) {
	return decodeStartupAudioStream(resources.openFile(path), warmupMode);
}

} // End of anonymous namespace

MediaManager::MediaManager(ResourceManager &resources) : _resources(resources) {
	_entityManager = new EntityManager(_resources);
}

MediaManager::~MediaManager() {
	stopMusic();
	stopSound();
	delete _text;
	delete _art;
	delete _screen;
	delete _entityManager;
}

float MediaManager::mapGammaLevelToBrightnessScale(int level) {
	return 1.0f + 0.1f * clampStartupOptionLevel(level);
}

void MediaManager::resetScreen(int width, int height) {
	delete _screen;
	_screen = new Graphics::Screen();
	_displayWidth = width;
	_displayHeight = height;
}

bool MediaManager::loadArt() {
	Art *art = new Art();
	if (!art->load(_resources)) {
		delete art;
		return false;
	}

	delete _art;
	_art = art;
	return true;
}

bool MediaManager::loadText() {
	Text *text = new Text();
	if (!text->load(_resources)) {
		delete text;
		return false;
	}

	delete _text;
	_text = text;
	return true;
}

bool MediaManager::loadQuickTipsResources() {
	return _art && _art->loadQuickTipsResources(_resources);
}

void MediaManager::drawWaitFrame() const {
	if (_art && _screen)
		_art->drawWaitFrame(*_screen);
}

void MediaManager::applyMixerLevels(int fxLevel, int musicLevel) {
	if (!g_system || !g_system->getMixer())
		return;

	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType,
		mapStartupVolumeLevelToMixerVolume(fxLevel));
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType,
		mapStartupVolumeLevelToMixerVolume(musicLevel));

	if (g_system->getMixer()->isSoundHandleActive(_musicHandle))
		g_system->getMixer()->pauseHandle(_musicHandle, musicLevel == 0);
}

bool MediaManager::isMusicPlaying() const {
	return g_system && g_system->getMixer() &&
		g_system->getMixer()->isSoundHandleActive(_musicHandle);
}

bool MediaManager::playMusic(const Common::String &path) {
	if (path.empty() || !g_system || !g_system->getMixer())
		return false;

	Common::String normalizedPath = _resources.normalizeResourcePath(path);
	if (_musicPath.equalsIgnoreCase(normalizedPath) &&
			g_system->getMixer()->isSoundHandleActive(_musicHandle)) {
		return true;
	}

	Audio::SeekableAudioStream *audioStream =
		openStartupAudioStream(_resources, path, FcmpWarmupMode::kMusic);
	if (!audioStream) {
		warning("Harvester: unable to decode startup music '%s'", path.c_str());
		return false;
	}

	stopMusic();
	g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle,
		Audio::makeLoopingAudioStream(audioStream, 0));
	_musicPath = Common::move(normalizedPath);
	return true;
}

void MediaManager::pauseMusic(bool paused) {
	if (g_system && g_system->getMixer())
		g_system->getMixer()->pauseHandle(_musicHandle, paused);
}

void MediaManager::stopMusic() {
	stopSoundHandle(_musicHandle);
	_musicPath.clear();
}

bool MediaManager::executeAudioCommand(const AudioCommand &command) {
	switch (command.type) {
	case kStartupAudioCommandStartWav:
		return playSound(command.path);
	case kStartupAudioCommandStartSingleWav:
		return playSingleSound(command.path);
	case kStartupAudioCommandLoadWav:
		return loadSound(command.slot, command.path);
	case kStartupAudioCommandPlayWav:
		return playLoadedSound(command.slot);
	case kStartupAudioCommandDeleteWav:
		return deleteLoadedSound(command.slot);
	default:
		return false;
	}
}

bool MediaManager::playSound(const Common::String &path) {
	if (path.empty() || !g_system || !g_system->getMixer())
		return false;

	Common::String normalizedPath = _resources.normalizeResourcePath(path);
	for (int i = 0; i < ARRAYSIZE(_soundPaths); ++i) {
		if (_soundPaths[i].equalsIgnoreCase(normalizedPath)) {
			stopSoundHandle(_soundHandles[i]);
			_soundPaths[i].clear();
		}
	}

	_soundSlotIndex = (_soundSlotIndex + 1) % ARRAYSIZE(_soundHandles);
	stopSoundHandle(_soundHandles[_soundSlotIndex]);

	Audio::SeekableAudioStream *audioStream =
		openStartupAudioStream(_resources, path, FcmpWarmupMode::kSample);
	if (!audioStream) {
		warning("Harvester: unable to decode startup sound '%s'", path.c_str());
		return false;
	}

	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType,
		&_soundHandles[_soundSlotIndex], audioStream);
	_soundPaths[_soundSlotIndex] = Common::move(normalizedPath);
	return true;
}

bool MediaManager::playSingleSound(const Common::String &path) {
	if (path.empty() || !g_system || !g_system->getMixer())
		return false;

	stopSoundHandle(_singleSoundHandle);
	Audio::SeekableAudioStream *audioStream =
		openStartupAudioStream(_resources, path, FcmpWarmupMode::kSample);
	if (!audioStream) {
		warning("Harvester: unable to decode startup sound '%s'", path.c_str());
		return false;
	}

	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_singleSoundHandle, audioStream);
	return true;
}

void MediaManager::stopSingleSound() {
	stopSoundHandle(_singleSoundHandle);
}

bool MediaManager::isSingleSoundPlaying() const {
	return g_system && g_system->getMixer() &&
		g_system->getMixer()->isSoundHandleActive(_singleSoundHandle);
}

bool MediaManager::playSpeech(const Common::String &path) {
	if (path.empty() || !g_system || !g_system->getMixer())
		return false;

	stopSoundHandle(_speechHandle);
	Audio::SeekableAudioStream *audioStream =
		openStartupAudioStream(_resources, path, FcmpWarmupMode::kSample);
	if (!audioStream) {
		warning("Harvester: unable to decode startup speech '%s'", path.c_str());
		return false;
	}

	g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, audioStream);
	return true;
}

void MediaManager::stopSpeech() {
	stopSoundHandle(_speechHandle);
}

bool MediaManager::isSpeechPlaying() const {
	return g_system && g_system->getMixer() &&
		g_system->getMixer()->isSoundHandleActive(_speechHandle);
}

bool MediaManager::loadSound(int slot, const Common::String &path) {
	if (!validateLoadedSoundSlot(slot) || path.empty())
		return false;

	stopSoundHandle(_loadedSoundHandles[slot]);
	_loadedSoundPaths[slot].clear();
	_loadedSoundData[slot].clear();

	if (!_resources.loadFile(path, _loadedSoundData[slot])) {
		warning("Harvester: unable to load startup sound '%s' into slot %d", path.c_str(), slot);
		return false;
	}

	_loadedSoundPaths[slot] = _resources.normalizeResourcePath(path);
	return true;
}

bool MediaManager::playLoadedSound(int slot) {
	if (!validateLoadedSoundSlot(slot) || !g_system || !g_system->getMixer())
		return false;
	if (_loadedSoundData[slot].empty())
		return false;

	stopSoundHandle(_loadedSoundHandles[slot]);

	Common::SeekableReadStream *stream = new Common::MemoryReadStream(
		_loadedSoundData[slot].data(), _loadedSoundData[slot].size(), DisposeAfterUse::NO);
	Audio::SeekableAudioStream *audioStream = decodeStartupAudioStream(stream, FcmpWarmupMode::kSample);
	if (!audioStream) {
		warning("Harvester: unable to decode startup sound slot %d ('%s')",
			slot, _loadedSoundPaths[slot].c_str());
		return false;
	}

	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_loadedSoundHandles[slot], audioStream);
	return true;
}

bool MediaManager::deleteLoadedSound(int slot) {
	if (!validateLoadedSoundSlot(slot))
		return false;

	stopSoundHandle(_loadedSoundHandles[slot]);
	_loadedSoundPaths[slot].clear();
	_loadedSoundData[slot].clear();
	return true;
}

void MediaManager::stopSound() {
	stopSingleSound();
	stopSpeech();
	for (int i = 0; i < ARRAYSIZE(_soundHandles); ++i) {
		stopSoundHandle(_soundHandles[i]);
		_soundPaths[i].clear();
	}
	for (int i = 0; i < ARRAYSIZE(_loadedSoundHandles); ++i) {
		stopSoundHandle(_loadedSoundHandles[i]);
		_loadedSoundPaths[i].clear();
		_loadedSoundData[i].clear();
	}
	_soundSlotIndex = -1;
}

void MediaManager::stopSoundHandle(Audio::SoundHandle &handle) {
	if (g_system && g_system->getMixer())
		g_system->getMixer()->stopHandle(handle);
}

bool MediaManager::validateLoadedSoundSlot(int slot) const {
	return slot >= 0 && slot < ARRAYSIZE(_loadedSoundHandles);
}

} // End of namespace Harvester
