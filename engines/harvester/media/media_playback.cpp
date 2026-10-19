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

#include "harvester/media/media_manager.h"
#include "harvester/media/audio_decoder.h"

#include "audio/audiostream.h"
#include "common/algorithm.h"
#include "common/memstream.h"
#include "common/system.h"
#include "harvester/resources.h"
#include "harvester/script/script.h"

namespace Harvester {

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
