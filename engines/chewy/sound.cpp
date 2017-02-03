/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "common/system.h"

#include "chewy/resource.h"
#include "chewy/sound.h"

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

void Sound::playSound(int num, bool loop, uint channel) {
	SoundChunk *sound = _soundRes->getSound(num);
	byte *data = (byte *)malloc(sound->size);
	memcpy(data, sound->data, sound->size);

	playSound(data, sound->size, loop, channel);

	delete[] sound->data;
	delete sound;
}

void Sound::playSound(byte *data, uint32 size, bool loop, uint channel, DisposeAfterUse::Flag dispose) {
	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
		Audio::makeRawStream(data,
		size, 22050, Audio::FLAG_UNSIGNED,
		dispose),
		loop ? 0 : 1);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle[channel], stream);
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

bool Sound::isSoundActive(uint channel) {
	assert(channel < MAX_SOUND_EFFECTS);
	return _mixer->isSoundHandleActive(_soundHandle[channel]);
}

void Sound::setSoundVolume(uint volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
}

void Sound::setSoundChannelVolume(uint channel, uint volume) {
	assert(channel < MAX_SOUND_EFFECTS);
	_mixer->setChannelVolume(_soundHandle[channel], volume);
}

void Sound::setSoundChannelBalance(uint channel, uint balance) {
	assert(channel < MAX_SOUND_EFFECTS);
	_mixer->setChannelBalance(_soundHandle[channel], balance);
}

void Sound::playMusic(int num, bool loop) {
	uint32 musicNum = _soundRes->getChunkCount() - 1 - num;
	Chunk *chunk = _soundRes->getChunk(musicNum);
	byte *data = _soundRes->getChunkData(musicNum);

	playMusic(data, chunk->size, loop);

	delete[] data;
	delete chunk;
}

void Sound::playMusic(byte *data, uint32 size, bool loop, DisposeAfterUse::Flag dispose) {
	byte *modData = nullptr;
	uint32 modSize;

	// TODO: TMF music files are similar to MOD files. With the following
	// incorrect implementation, the PCM parts of these files can be played
	warning("The current music playing implementation is wrong");
	modSize = size;
	modData = (byte *)malloc(modSize);
	memcpy(modData, data, size);
	//convertTMFToMod(data, size, modData, modSize);

	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
		Audio::makeRawStream(modData,
		modSize, 22050, Audio::FLAG_UNSIGNED,
		dispose),
		loop ? 0 : 1);

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, stream);
}

void Sound::pauseMusic() {
	_mixer->pauseHandle(_musicHandle, true);
}

void Sound::resumeMusic() {
	_mixer->pauseHandle(_musicHandle, false);
}

void Sound::stopMusic() {
	_mixer->stopHandle(_musicHandle);
}

bool Sound::isMusicActive() {
	return _mixer->isSoundHandleActive(_musicHandle);
}

void Sound::setMusicVolume(uint volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
}

void Sound::playSpeech(int num) {
	SoundChunk *sound = _speechRes->getSound(num);
	byte *data = (byte *)malloc(sound->size);
	memcpy(data, sound->data, sound->size);

	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
		Audio::makeRawStream(data,
		sound->size, 22050, Audio::FLAG_UNSIGNED,
		DisposeAfterUse::YES),
		1);

	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream);

	delete[] sound->data;
	delete sound;
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

bool Sound::isSpeechActive() {
	return _mixer->isSoundHandleActive(_speechHandle);
}

void Sound::setSpeechVolume(uint volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume);
}

void Sound::stopAll() {
	_mixer->stopAll();
}

void Sound::convertTMFToMod(byte *tmfData, uint32 tmfSize, byte *modData, uint32 &modSize) {
	// Convert the TMF stream back to a regular Protracker MOD stream
	const int maxInstruments = 31;
	// Extra bytes needed: 20 bytes song name, 31 * 22 instrument names, 4 magic bytes "M.K."
	modSize = tmfSize + 20 + maxInstruments * 22 + 4;
	modData = (byte *)malloc(modSize);
	byte *tmfPtr = tmfData;
	byte *modPtr = modData;

	const byte songName[20] = {
		'S', 'C', 'U', 'M', 'M',
		'V', 'M', ' ', 'M', 'O',
		'D', 'U', 'L', 'E', '\0',
		'\0', '\0', '\0', '\0', '\0'
	};
	const byte instrumentName[22] = {
		'S', 'C', 'U', 'M', 'M',
		'V', 'M', ' ', 'I', 'N',
		'S', 'T', 'R', 'U', 'M',
		'E', 'N', 'T', '\0', '\0',
		'\0', '\0'
	};

	if (READ_BE_UINT32(tmfPtr) != MKTAG('T', 'M', 'F', '\0'))
		error("Corrupt TMF resource");
	tmfPtr += 4;

	memcpy(modPtr, songName, 20);
	modPtr += 20;

	byte fineTune, instVolume;
	uint16 repeatPoint, repeatLength, sampleLength;

	for (int i = 0; i < maxInstruments; i++) {
		fineTune = *tmfPtr++;
		instVolume = *tmfPtr++;
		repeatPoint = READ_BE_UINT16(tmfPtr);	tmfPtr += 2;
		repeatLength = READ_BE_UINT16(tmfPtr);	tmfPtr += 2;
		sampleLength = READ_BE_UINT16(tmfPtr);	tmfPtr += 2;

		// Instrument name
		memcpy(modPtr, instrumentName, 18);	modPtr += 18;
		*modPtr++ = ' ';
		*modPtr++ = i / 10;
		*modPtr++ = i % 10;
		*modPtr++ = '\0';

		WRITE_BE_UINT16(modPtr, sampleLength / 2);	modPtr += 2;
		*modPtr++ = fineTune;
		*modPtr++ = instVolume;
		WRITE_BE_UINT16(modPtr, repeatPoint / 2);	modPtr += 2;
		WRITE_BE_UINT16(modPtr, repeatLength / 2);	modPtr += 2;
	}

	*modPtr++ = *tmfPtr++;	// song length
	*modPtr++ = *tmfPtr++;	// undef
	memcpy(modPtr, tmfPtr, 128);
	modPtr += 128;
	tmfPtr += 128;
	WRITE_BE_UINT32(modPtr, MKTAG('M', '.', 'K', '.'));	modPtr += 4;
	// TODO: 31 bytes instrument positions

	// TODO: notes

	free(modData);
}

} // End of namespace Chewy
