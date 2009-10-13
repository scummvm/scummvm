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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/draci/barchive.cpp $
 * $Id: barchive.cpp 44493 2009-09-30 16:04:21Z fingolfin $
 *
 */

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/str.h"
#include "common/stream.h"

#include "draci/sound.h"
#include "draci/draci.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Draci {

void SoundArchive::openArchive(const Common::String &path) {
	// Close previously opened archive (if any)
	closeArchive();

	debugCN(2, kDraciArchiverDebugLevel, "Loading samples %s: ", path.c_str());

	_f = new Common::File();
	_f->open(path);
	if (_f->isOpen()) {
		debugC(2, kDraciArchiverDebugLevel, "Success");
	} else {
		debugC(2, kDraciArchiverDebugLevel, "Error");
		return;
	}

	// Save path for reading in files later on
	_path = path;

	// Read archive header
	debugC(2, kDraciArchiverDebugLevel, "Loading header");

	uint totalLength = _f->readUint32LE();
	const uint kMaxSamples = 4095;	// The no-sound file is exactly 16K bytes long, so don't fail on short reads
	uint sampleStarts[kMaxSamples];
	for (uint i = 0; i < kMaxSamples; ++i) {
		sampleStarts[i] = _f->readUint32LE();
	}

	// Fill the sample table
	for (_sampleCount = 0; _sampleCount < kMaxSamples - 1; ++_sampleCount) {
		int length = sampleStarts[_sampleCount + 1] - sampleStarts[_sampleCount];
		if (length <= 0 && sampleStarts[_sampleCount] >= totalLength)	// heuristics to detect the last sample
			break;
	}
	if (_sampleCount > 0) {
		debugC(2, kDraciArchiverDebugLevel, "Archive info: %d samples, %d total length",
			_sampleCount, totalLength);
		_samples = new SoundSample[_sampleCount];
		for (uint i = 0; i < _sampleCount; ++i) {
			_samples[i]._offset = sampleStarts[i];
			_samples[i]._length = sampleStarts[i+1] - sampleStarts[i];
			_samples[i]._frequency = 0;	// set in getSample()
			_samples[i]._data = NULL;
		}
		if (_samples[_sampleCount-1]._offset + _samples[_sampleCount-1]._length != totalLength &&
		    _samples[_sampleCount-1]._offset + _samples[_sampleCount-1]._length - _samples[0]._offset != totalLength) {
			// WORKAROUND: the stored length is stored with the header for sounds and without the hader for dubbing.  Crazy.
			debugC(2, kDraciArchiverDebugLevel, "Broken sound archive: %d != %d",
				_samples[_sampleCount-1]._offset + _samples[_sampleCount-1]._length,
				totalLength);
			closeArchive();
			return;
		}
	} else {
		debugC(2, kDraciArchiverDebugLevel, "Archive info: empty");
	}

	// Indicate that the archive has been successfully opened
	_opened = true;
}

/**
 * @brief SoundArchive close method
 *
 * Closes the currently opened archive. It can be called explicitly to
 * free up memory.
 */
void SoundArchive::closeArchive() {
	clearCache();
	delete _f;
	_f = NULL;
	delete[] _samples;
	_samples = NULL;
	_sampleCount = 0;
	_path = "";
	_opened = false;
}

/**
 * Clears the cache of the open files inside the archive without closing it.
 * If the files are subsequently accessed, they are read from the disk.
 */
void SoundArchive::clearCache() {
	// Delete all cached data
	for (uint i = 0; i < _sampleCount; ++i) {
		_samples[i].close();
	}
}

/**
 * @brief On-demand sound sample loader
 * @param i Index of file inside an archive
 * @return Pointer to a SoundSample coresponding to the opened file or NULL (on failure)
 *
 * Loads individual samples from an archive to memory on demand.
 */
SoundSample *SoundArchive::getSample(int i, uint freq) {
	// Check whether requested file exists
	if (i < 0 || i >= (int) _sampleCount) {
		return NULL;
	}

	debugCN(2, kDraciArchiverDebugLevel, "Accessing sample %d from archive %s... ",
		i, _path.c_str());

	// Check if file has already been opened and return that
	if (_samples[i]._data) {
		debugC(2, kDraciArchiverDebugLevel, "Success");
	} else {
		// Read in the file (without the file header)
		_f->seek(_samples[i]._offset);
		_samples[i]._data = new byte[_samples[i]._length];
		_f->read(_samples[i]._data, _samples[i]._length);

		debugC(3, kDraciArchiverDebugLevel, "Cached sample %d from archive %s",
			i, _path.c_str());
	}
	_samples[i]._frequency = freq ? freq : _defaultFreq;

	return _samples + i;
}

Sound::Sound(Audio::Mixer *mixer) : _mixer(mixer) {

	for (int i = 0; i < SOUND_HANDLES; i++)
		_handles[i].type = kFreeHandle;

	setVolume();
}

SndHandle *Sound::getHandle() {
	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type != kFreeHandle && !_mixer->isSoundHandleActive(_handles[i].handle)) {
			debugC(5, kDraciSoundDebugLevel, "Handle %d has finished playing", i);
			_handles[i].type = kFreeHandle;
		}
	}

	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kFreeHandle)
			return &_handles[i];
	}

	error("Sound::getHandle(): Too many sound handles");

	return NULL;	// for compilers that don't support NORETURN
}

void Sound::playSoundBuffer(Audio::SoundHandle *handle, const SoundSample &buffer, int volume,
				sndHandleType handleType, bool loop) {

	// Don't use FLAG_AUTOFREE, because our caching system deletes samples by itself.
	byte flags = Audio::Mixer::FLAG_UNSIGNED;

	if (loop)
		flags |= Audio::Mixer::FLAG_LOOP;

	const Audio::Mixer::SoundType soundType = (handleType == kVoiceHandle) ? 
				Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType;

	_mixer->playRaw(soundType, handle, buffer._data,
			buffer._length, buffer._frequency, flags, -1, volume);
}

void Sound::playSound(const SoundSample *buffer, int volume, bool loop) {
	if (!buffer)
		return;
	SndHandle *handle = getHandle();

	handle->type = kEffectHandle;
	playSoundBuffer(&handle->handle, *buffer, 2 * volume, handle->type, loop);
}

void Sound::pauseSound() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kEffectHandle)
			_mixer->pauseHandle(_handles[i].handle, true);
}

void Sound::resumeSound() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kEffectHandle)
			_mixer->pauseHandle(_handles[i].handle, false);
}

void Sound::stopSound() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kEffectHandle) {
			_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
		}
}

void Sound::playVoice(const SoundSample *buffer) {
	if (!buffer)
		return;
	SndHandle *handle = getHandle();

	handle->type = kVoiceHandle;
	playSoundBuffer(&handle->handle, *buffer, Audio::Mixer::kMaxChannelVolume, handle->type, false);
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

void Sound::setVolume() {
	const int soundVolume = ConfMan.getInt("sfx_volume");
	const int speechVolume = ConfMan.getInt("speech_volume");
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, speechVolume);
	// TODO: make sure this sound settings works
}

} // End of namespace Draci
