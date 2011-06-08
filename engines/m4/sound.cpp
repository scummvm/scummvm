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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "m4/m4.h"
#include "m4/sound.h"
#include "m4/compression.h"

#include "common/stream.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

namespace M4 {

Sound::Sound(MadsM4Engine *vm, Audio::Mixer *mixer, int volume) :
	_vm(vm), _mixer(mixer) {

	for (int i = 0; i < SOUND_HANDLES; i++)
		_handles[i].type = kFreeHandle;

	_dsrFileLoaded = false;

	setVolume(volume);
}

Sound::~Sound() {
	unloadDSRFile();
}

SndHandle *Sound::getHandle() {
	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kFreeHandle)
			return &_handles[i];

		if (!_mixer->isSoundHandleActive(_handles[i].handle)) {
			_handles[i].type = kFreeHandle;
			return &_handles[i];
		}
	}

	error("Sound::getHandle(): Too many sound handles");
	return NULL;	// for compilers that don't support NORETURN
}

bool Sound::isHandleActive(SndHandle *handle) {
	return (_mixer->isSoundHandleActive(handle->handle));
}

void Sound::playSound(const char *soundName, int volume, bool loop, int channel) {
	Common::SeekableReadStream *soundStream = _vm->res()->get(soundName);
	SndHandle *handle;
	if (channel < 0) {
		handle = getHandle();
	} else {
		if (_handles[channel].type == kFreeHandle) {
			handle = &_handles[channel];
		} else {
			warning("Attempted to play a sound on a channel that isn't free");
			return;
		}
	}

	int bufferSize = soundStream->size();
	byte *buffer = new byte[bufferSize];
	soundStream->read(buffer, bufferSize);
	_vm->res()->toss(soundName);

	handle->type = kEffectHandle;

	_vm->res()->toss(soundName);

	// Sound format is 8bit mono, unsigned, 11025kHz
	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
				Audio::makeRawStream(buffer, bufferSize, 11025, Audio::FLAG_UNSIGNED),
				loop ? 0 : 1);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle->handle, stream, -1, volume);
}

void Sound::playSound(int soundNum) {
	warning("TODO: playSound(%d)", soundNum);
}

void Sound::pauseSound() {
	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kEffectHandle)
			_mixer->pauseHandle(_handles[i].handle, true);
	}
}

void Sound::resumeSound() {
	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kEffectHandle)
			_mixer->pauseHandle(_handles[i].handle, false);
	}
}

void Sound::stopSound(int channel) {
	if (channel >= 0) {
		if (_handles[channel].type == kEffectHandle) {
			_mixer->stopHandle(_handles[channel].handle);
			_handles[channel].type = kFreeHandle;
			return;
		} else {
			warning("Attempted to stop a sound on a channel that is already free");
			return;
		}
	}

	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kEffectHandle) {
			_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
		}
	}
}

void Sound::playVoice(const char *soundName, int volume) {
	Common::SeekableReadStream *soundStream = _vm->res()->get(soundName);
	SndHandle *handle = getHandle();
	byte *buffer;

	buffer = (byte *)malloc(soundStream->size());
	soundStream->read(buffer, soundStream->size());

	handle->type = kEffectHandle;

	_vm->res()->toss(soundName);

	// Voice format is 8bit mono, unsigned, 11025kHz
	Audio::AudioStream *stream = Audio::makeRawStream(buffer, soundStream->size(), 11025, Audio::FLAG_UNSIGNED);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle->handle, stream, -1, volume);
}

void Sound::pauseVoice() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kVoiceHandle)
			_mixer->pauseHandle(_handles[i].handle, true);
}

void Sound::resumeVoice() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kVoiceHandle)
			_mixer->pauseHandle(_handles[i].handle, false);
}

void Sound::stopVoice() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kVoiceHandle) {
			_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
		}
}

void Sound::stopAll() {
	stopVoice();
	stopSound();
}

void Sound::setVolume(int volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume);
}

void Sound::loadDSRFile(const char *fileName) {
	if (_dsrFileLoaded)
		unloadDSRFile();

	Common::SeekableReadStream *fileStream = _vm->res()->get(fileName);

	sprintf(_dsrFile.fileName, "%s", fileName);

	// Read header
	_dsrFile.entryCount = fileStream->readUint16LE();
	//warning(kDebugSound, "DSR has %i entries\n", _dsrFile.entryCount);

	for (int i = 0; i < _dsrFile.entryCount; i++) {
		DSREntry newEntry;
		newEntry.frequency = fileStream->readUint16LE();
		newEntry.channels = fileStream->readUint32LE();
		newEntry.compSize = fileStream->readUint32LE();
		newEntry.uncompSize = fileStream->readUint32LE();
		newEntry.offset = fileStream->readUint32LE();
		_dsrFile.dsrEntries.push_back(newEntry);

		/*
		warning(kDebugSound, "%i: ", i);
		warning(kDebugSound, "frequency: %i ", newEntry->frequency);
		warning(kDebugSound, "channels: %i ", newEntry->channels);
		warning(kDebugSound, "comp: %i ", newEntry.compSize);
		warning(kDebugSound, "uncomp: %i ", newEntry.uncompSize);
		warning(kDebugSound, "offset: %i ", newEntry->offset);
		warning(kDebugSound, "\n");
		*/
	}

	_vm->res()->toss(fileName);

	_dsrFileLoaded = true;
}

void Sound::unloadDSRFile() {
	if (!_dsrFileLoaded)
		return;

	_dsrFile.dsrEntries.clear();

	_dsrFile.entryCount = 0;
	strcpy(_dsrFile.fileName, "");
	_dsrFileLoaded = false;
}

void Sound::playDSRSound(int soundIndex, int volume, bool loop) {
	if (!_dsrFileLoaded) {
		warning("DSR file not loaded, not playing sound");
		return;
	}

	if (soundIndex < 0 || soundIndex > _dsrFile.entryCount - 1) {
		warning("Invalid sound index: %i, not playing sound", soundIndex);
		return;
	}

	SndHandle *handle = getHandle();

	handle->type = kEffectHandle;

	// Get sound data
	FabDecompressor fab;
	byte *compData = new byte[_dsrFile.dsrEntries[soundIndex].compSize];
	byte *buffer = new byte[_dsrFile.dsrEntries[soundIndex].uncompSize];
	Common::SeekableReadStream *fileStream = _vm->res()->get(_dsrFile.fileName);
	fileStream->seek(_dsrFile.dsrEntries[soundIndex].offset, SEEK_SET);
	fileStream->read(compData, _dsrFile.dsrEntries[soundIndex].compSize);
	_vm->res()->toss(_dsrFile.fileName);

	fab.decompress(compData, _dsrFile.dsrEntries[soundIndex].compSize,
				   buffer, _dsrFile.dsrEntries[soundIndex].uncompSize);

	// Play sound
	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
				Audio::makeRawStream(buffer,
					_dsrFile.dsrEntries[soundIndex].uncompSize,
					_dsrFile.dsrEntries[soundIndex].frequency, Audio::FLAG_UNSIGNED),
				loop ? 0 : 1);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle->handle, stream, -1, volume);

	/*
	// Dump the sound file
	FILE *destFile = fopen("sound.raw", "wb");
	fwrite(_dsrFile.dsrEntries[soundIndex]->data, _dsrFile.dsrEntries[soundIndex].uncompSize, 1, destFile);
	fclose(destFile);
	*/
}

} // End of namespace M4
