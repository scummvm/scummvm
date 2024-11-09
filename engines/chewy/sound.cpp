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
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "audio/mods/protracker.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/system.h"
#include "chewy/resource.h"
#include "chewy/sound.h"
#include "chewy/types.h"
#include "chewy/globals.h"
#include "chewy/audio/chewy_voc.h"
#include "chewy/audio/tmf_stream.h"

namespace Chewy {

Sound::Sound(Audio::Mixer *mixer) {
	_mixer = mixer;
	_speechRes = new SoundResource("speech.tvp");
	_soundRes = new SoundResource("details.tap");
}

Sound::~Sound() {
	delete _soundRes;
	delete _speechRes;
}

void Sound::playSound(int num, uint channel, uint16 loops, uint16 volume, uint16 balance) {
	if (num < 0)
		return;

	SoundChunk *sound = _soundRes->getSound(num);
	uint8 *data = (uint8 *)MALLOC(sound->size);
	memcpy(data, sound->data, sound->size);

	playSound(data, sound->size, channel, loops, volume, balance);

	delete[] sound->data;
	delete sound;
}

void Sound::playSound(uint8 *data, uint32 size, uint channel, uint16 loops, uint16 volume, uint16 balance, DisposeAfterUse::Flag dispose) {
	stopSound(channel);

	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
		new ChewyVocStream(
			new Common::MemorySeekableReadWriteStream(data, size, dispose),
			dispose),
		loops);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle[channel], stream, -1,
		convertVolume(volume), convertBalance(balance));
}

void Sound::pauseSound(uint channel) {
	assert(channel < MAX_SOUND_EFFECTS);
	_mixer->pauseHandle(_soundHandle[channel], true);
}

void Sound::resumeSound(uint channel) {
	assert(channel < MAX_SOUND_EFFECTS);
	_mixer->pauseHandle(_soundHandle[channel], false);
}

void Sound::stopSound(uint channel) {
	assert(channel < MAX_SOUND_EFFECTS);
	_mixer->stopHandle(_soundHandle[channel]);
}

void Sound::stopAllSounds() {
	for (int i = 0; i < 14; i++)
		stopSound(i);
}

bool Sound::isSoundActive(uint channel) const {
	assert(channel < MAX_SOUND_EFFECTS);
	return _mixer->isSoundHandleActive(_soundHandle[channel]);
}

void Sound::setUserSoundVolume(uint volume) {
	_userSoundVolume = volume;
	if (soundEnabled())
		ConfMan.setInt("sfx_volume", volume);
}

int Sound::getUserSoundVolume() const {
	return _userSoundVolume;
}

void Sound::setSoundChannelVolume(uint channel, uint volume) {
	assert(channel < MAX_SOUND_EFFECTS);
	_mixer->setChannelVolume(_soundHandle[channel], convertVolume(volume));
}

void Sound::setSoundChannelBalance(uint channel, int8 balance) {
	assert(channel < MAX_SOUND_EFFECTS);
	_mixer->setChannelBalance(_soundHandle[channel], convertBalance(balance));
}

void Sound::playMusic(int16 num, bool loop) {
	uint32 musicNum = _soundRes->getChunkCount() - 1 - num;
	Chunk *chunk = _soundRes->getChunk(musicNum);
	uint8 *data = _soundRes->getChunkData(musicNum);
	_curMusic = num;

	playMusic(data, chunk->size);

	delete[] data;
}

void Sound::playMusic(uint8 *data, uint32 size, uint8 volume) {
	TMFStream *stream = new TMFStream(new Common::MemoryReadStream(data, size), 0);
	_curMusic = -1;

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, stream, -1, convertVolume(volume));
}

void Sound::pauseMusic() {
	_mixer->pauseHandle(_musicHandle, true);
}

void Sound::resumeMusic() {
	_mixer->pauseHandle(_musicHandle, false);
}

void Sound::stopMusic() {
	_curMusic = -1;
	_mixer->stopHandle(_musicHandle);
}

bool Sound::isMusicActive() const {
	return _mixer->isSoundHandleActive(_musicHandle);
}

void Sound::setUserMusicVolume(uint volume) {
	_userMusicVolume = volume;
	if (musicEnabled())
		ConfMan.setInt("music_volume", volume);
}

int Sound::getUserMusicVolume() const {
	return _userMusicVolume;
}

void Sound::setActiveMusicVolume(uint8 volume) {
	if (isMusicActive())
		_mixer->setChannelVolume(_musicHandle, convertVolume(volume));
}

void Sound::playSpeech(int num, bool waitForFinish, uint16 balance) {
	if (isSpeechActive())
		stopSpeech();

	// Get the speech data
	SoundChunk *sound = _speechRes->getSound(num);
	size_t size = sound->size;
	uint8 *data = (uint8 *)MALLOC(size);
	memcpy(data, sound->data, size);

	delete[] sound->data;
	delete sound;

	// Play it
	Audio::AudioStream *stream = new ChewyVocStream(
		new Common::MemorySeekableReadWriteStream(data, size, DisposeAfterUse::YES),
		DisposeAfterUse::YES);

	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream,
		-1, Audio::Mixer::kMaxChannelVolume, convertBalance(balance));

	if (waitForFinish) {
		// Wait for speech to finish
		while (isSpeechActive() && !SHOULD_QUIT) {
			setupScreen(DO_SETUP);
		}
	}
}

void Sound::pauseSpeech() {
	_mixer->pauseHandle(_speechHandle, true);
}

void Sound::resumeSpeech() {
	_mixer->pauseHandle(_speechHandle, false);
}

void Sound::stopSpeech() {
	_mixer->stopHandle(_speechHandle);
}

bool Sound::isSpeechActive() const {
	return _mixer->isSoundHandleActive(_speechHandle);
}

void Sound::stopAll() {
	_mixer->stopAll();
}

void Sound::waitForSpeechToFinish() {
	if (speechEnabled()) {
		while (isSpeechActive() && !SHOULD_QUIT) {
			setupScreen(DO_SETUP);
		}
	}
}

void Sound::setSpeechBalance(uint16 balance) {
	if (isSpeechActive()) {
		_mixer->setChannelBalance(_speechHandle, convertBalance(balance));
	}
}

bool Sound::soundEnabled() const {
	return ConfMan.getInt("sfx_volume") > 0;
}

void Sound::toggleSound(bool enable) {
	ConfMan.setInt("sfx_volume", enable ? _userSoundVolume : 0);
}

bool Sound::musicEnabled() const {
	return ConfMan.getInt("music_volume") > 0;
}

void Sound::toggleMusic(bool enable) {
	ConfMan.setInt("music_volume", enable ? _userSoundVolume : 0);
}

bool Sound::speechEnabled() const {
	return !ConfMan.getBool("speech_mute");
}

void Sound::toggleSpeech(bool enable) {
	ConfMan.setBool("speech_mute", !enable);
}

bool Sound::subtitlesEnabled() const {
	return ConfMan.getBool("subtitles");
}

void Sound::toggleSubtitles(bool enable) {
	ConfMan.setBool("subtitles", enable);
}

void Sound::syncSoundSettings() {
	int confSoundVolume = ConfMan.getInt("sfx_volume");
	int confMusicVolume = ConfMan.getInt("music_volume");

	if (confSoundVolume == 0) {
		// Sound is muted.
		if (_userSoundVolume == 0)
			// Set a default value.
			_userSoundVolume = 192;
		// Otherwise leave _soundVolume at the last value set by the user.
	} else {
		_userSoundVolume = confSoundVolume;
	}
	if (confMusicVolume == 0) {
		// Music is muted.
		if (_userMusicVolume == 0)
			// Set a default value.
			_userMusicVolume = 192;
		// Otherwise leave _musicVolume at the last value set by the user.
	} else {
		_userMusicVolume = confMusicVolume;
	}
}

struct RoomMusic {
	int16 room;
	int16 music;
};

const RoomMusic roomMusic[] = {
	{   0, 13 }, {   1, 17 }, {  18, 17 }, {  90, 17 }, {   2, 43 },
	{  88, 43 }, {   3,  0 }, {   4,  0 }, {   5, 14 }, {   8, 14 },
	{  12, 14 }, {  86, 14 }, {   6,  1 }, {   7, 18 }, {  97, 18 },
	{   9, 20 }, {  10, 20 }, {  47, 20 }, {  87, 20 }, {  11, 19 },
	{  14, 19 }, {  15, 16 }, {  16, 16 }, {  19, 16 }, {  96, 16 },
	{  21,  2 }, {  22, 48 }, {  25, 11 }, {  26, 11 }, {  27, 33 },
	{  30, 33 }, {  54, 33 }, {  63, 33 }, {  28, 47 }, {  29, 47 },
	{  31,  9 }, {  35,  9 }, {  32, 38 }, {  40, 38 }, {  71, 38 },
	{  89, 38 }, {  92, 38 }, {  33, 35 }, {  37,  8 }, {  39,  9 },
	{  42, 41 }, {  45, 44 }, {  46, 21 }, {  50, 21 }, {  73, 21 },
	{  74, 21 }, {  48, 22 }, {  49,  3 }, {  51, 27 }, {  52, 27 },
	{  53, 26 }, {  55, 23 }, {  57, 23 }, {  56,  7 }, {  62, 25 },
	{  64, 51 }, {  66, 34 }, {  68, 34 }, {  67, 28 }, {  69, 28 },
	{  70, 28 }, {  75, 28 }, {  72, 31 }, {  76, 46 }, {  79,  6 },
	{  80, 29 }, {  81, 45 }, {  82, 50 }, {  84, 24 }, {  85, 32 },
	{  91, 36 }, {  94, 40 }, {  95, 40 }, {  98,  4 }, { 255,  5 },
	{ 256, 10 }, { 257, 52 }, { 258, 53 }, { 259, 54 }, { 260, 24 },
	{  -1, -1 }
};

void Sound::playRoomMusic(int16 roomNum) {
	int16 musicIndex = -1;
	if (!musicEnabled())
		return;

	for (const RoomMusic *cur = roomMusic; cur->room >= 0; ++cur) {
		if (cur->room == roomNum) {
			musicIndex = cur->music;
			break;
		}
	}
	
	// Room 56 (harbor) music - first vs second visit
	if (roomNum == 56 && _G(gameState).flags32_10 && !_G(gameState).flags33_80)
		musicIndex = 52;

	if (musicIndex != _curMusic) {
		stopMusic();
		if (musicIndex >= 0)
			playMusic(musicIndex, true);
	}
}

uint8 Sound::convertVolume(uint16 volume) {
	assert(volume >= 0 && volume < 64);
	return volume * Audio::Mixer::kMaxChannelVolume / 63;
}

int8 Sound::convertBalance(uint16 balance) {
	assert(balance >= 0 && balance < 128);
	return MIN(127, (balance - 63) * 2);
}

} // namespace Chewy
