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
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "scumm/he/mixer_he.h"

namespace Scumm {

HEMixer::HEMixer(Audio::Mixer *mixer, ScummEngine_v60he *vm, bool useMilesSoundSystem) {
	_mixer = mixer;
	_vm = vm;
	_useMilesSoundSystem = useMilesSoundSystem;

	initSoftMixerSubSystem();
}

HEMixer::~HEMixer() {
	deinitSoftMixerSubSystem();
}

bool HEMixer::isMilesActive() {
	return _useMilesSoundSystem;
}

uint32 calculateDeflatedADPCMBlockSize(uint32 numBlocks, uint32 blockAlign, uint32 numChannels, uint32 bitsPerSample) {
	uint32 samplesPerBlock = (blockAlign - (4 * numChannels)) * (numChannels ^ 3) + 1;
	uint32 totalSize = numBlocks * samplesPerBlock * (bitsPerSample / 8);

	return totalSize;
}

bool HEMixer::initSoftMixerSubSystem() {
	if (!isMilesActive()) {
		return mixerInitMyMixerSubSystem();
	}

	return true;
}

void HEMixer::deinitSoftMixerSubSystem() {
	if (isMilesActive()) {
		milesStopAllSounds();
	} else {
		mixerStopAllSounds();
	}
}

bool HEMixer::stopChannel(int channel) {
	if (isMilesActive()) {
		return milesStopChannel(channel);
	} else {
		return mixerStopChannel(channel);
	}

	return true;
}

void HEMixer::stopAllChannels() {
	for (int i = 0; i < HSND_MAX_CHANNELS; i++) {
		stopChannel(i);
	}
}

bool HEMixer::pauseMixerSubSystem(bool paused) {
	if (isMilesActive()) {
		return milesPauseMixerSubSystem(paused);
	} else {
		return mixerPauseMixerSubSystem(paused);
	}
}

void HEMixer::feedMixer() {
	if (isMilesActive()) {
		milesFeedMixer();
	} else {
		mixerFeedMixer();
	}
}

bool HEMixer::changeChannelVolume(int channel, int volume, bool soft) {
	if (!isMilesActive()) {
		return mixerChangeChannelVolume(channel, volume, soft);
	}

	return true;
}

bool HEMixer::startChannelNew(
	int channel, int globType, int globNum, uint32 soundData, uint32 offset,
	int sampleLen, int frequency, int bitsPerSample, int sampleChannels,
	const HESoundModifiers &modifiers, int callbackID, int32 flags, ...) {

	va_list params;
	bool retValue;

	if (!isMilesActive()) {
		if (bitsPerSample != 8) {
			debug(5, "HEMixer::startChannelNew(): Glob(%d, %d) is %d bits per channel, must be 8 for software mixer", globType, globNum, bitsPerSample);
			return false;
		}

		if (flags & CHANNEL_CALLBACK_EARLY) {
			va_start(params, flags);

			retValue = mixerStartChannel(
				channel, globType, globNum, soundData + offset, sampleLen,
				frequency, modifiers.volume, callbackID, flags, va_arg(params, int));

			va_end(params);
			return retValue;
		} else {
			return mixerStartChannel(
				channel, globType, globNum, soundData + offset, sampleLen,
				frequency, modifiers.volume, callbackID, flags);
		}

	} else {
		flags &= ~CHANNEL_CALLBACK_EARLY;

		return milesStartChannel(
			channel, globType, globNum, soundData, offset, sampleLen,
			bitsPerSample, sampleChannels, frequency, modifiers, callbackID, flags);
	}

	return true;
}

bool HEMixer::startChannel(int channel, int globType, int globNum,
	uint32 sampleDataOffset, int sampleLen, int frequency, int volume, int callbackId, int32 flags, ...) {

	va_list params;
	bool retValue;

	if (flags & CHANNEL_CALLBACK_EARLY) {
		va_start(params, flags);

		retValue = mixerStartChannel(
			channel, globType, globNum, sampleDataOffset, sampleLen,
			frequency, volume, callbackId, flags, va_arg(params, int));

		va_end(params);
		return retValue;
	} else {
		return mixerStartChannel(
			channel, globType, globNum, sampleDataOffset, sampleLen,
			frequency, volume, callbackId, flags);
	}
}

bool HEMixer::startSpoolingChannel(int channel, int song, Common::File &sampleFileIOHandle,
	int sampleLen, int frequency, int volume, int callbackID, int32 flags) {

	if (!isMilesActive()) {
		return mixerStartSpoolingChannel(channel, song, sampleFileIOHandle, sampleLen,
			frequency, volume, callbackID, flags);
	}

	return false;
}

bool HEMixer::audioOverrideExists(int soundId, bool justGetInfo, int *duration, Audio::SeekableAudioStream **outStream) {
	if (!_vm->_enableAudioOverride) {
		return false;
	}

	const char *formats[] = {
#ifdef USE_FLAC
		"flac",
#endif
		"wav",
#ifdef USE_VORBIS
		"ogg",
#endif
#ifdef USE_MAD
		"mp3",
#endif
	};

	Audio::SeekableAudioStream *(*formatDecoders[])(Common::SeekableReadStream *, DisposeAfterUse::Flag) = {
#ifdef USE_FLAC
		Audio::makeFLACStream,
#endif
		Audio::makeWAVStream,
#ifdef USE_VORBIS
		Audio::makeVorbisStream,
#endif
#ifdef USE_MAD
		Audio::makeMP3Stream,
#endif
	};

	STATIC_ASSERT(
		ARRAYSIZE(formats) == ARRAYSIZE(formatDecoders),
		formats_formatDecoders_must_have_same_size);

	const char *type;
	if (soundId == HSND_TALKIE_SLOT) {
		// Speech audio doesn't have a unique ID,
		// so we use the file offset instead.
		// _heTalkOffset is set at playVoice.
		type = "speech";
		soundId = ((SoundHE *)(_vm->_sound))->getCurrentSpeechOffset();
	} else {
		// Music and sfx share the same prefix.
		type = "sound";
	}

	for (int i = 0; i < ARRAYSIZE(formats); i++) {
		Common::Path pathDir(Common::String::format("%s%d.%s", type, soundId, formats[i]));
		Common::Path pathSub(Common::String::format("%s/%d.%s", type, soundId, formats[i]));

		debug(5, "HEMixer::audioOverrideExists(): %s or %s", pathSub.toString().c_str(), pathDir.toString().c_str());

		// First check if the file exists before opening it to
		// reduce the amount of "opening %s failed" in the console.
		// Prefer files in subdirectory.
		Common::File soundFileOverride;
		bool foundFile = (soundFileOverride.exists(pathSub) && soundFileOverride.open(pathSub)) ||
						 (soundFileOverride.exists(pathDir) && soundFileOverride.open(pathDir));
		if (foundFile) {
			soundFileOverride.seek(0, SEEK_SET);
			Common::SeekableReadStream *oStr = soundFileOverride.readStream(soundFileOverride.size());
			soundFileOverride.close();

			Audio::SeekableAudioStream *seekStream = formatDecoders[i](oStr, DisposeAfterUse::YES);
			if (duration != nullptr) {
				*duration = seekStream->getLength().msecs();
			}

			if (justGetInfo) {
				delete seekStream;
				return true;
			}

			debug(5, "HEMixer::audioOverrideExists(): %s loaded from %s", formats[i], soundFileOverride.getName());

			*outStream = seekStream;
			return true;
		}
	}

	debug(5, "HEMixer::audioOverrideExists(): file not found");

	return false;
}

void HEMixer::setSpoolingSongsTable(HESpoolingMusicItem *heSpoolingMusicTable, int32 tableSize) {
	for (int i = 0; i < tableSize; i++) {
		_offsetsToSongId.setVal(heSpoolingMusicTable[i].offset, heSpoolingMusicTable[i].song);
	}
}

int32 HEMixer::matchOffsetToSongId(int32 offset) {
	return _offsetsToSongId.getValOrDefault(offset, 0);
}

/* --- SOFTWARE MIXER --- */

bool HEMixer::mixerInitMyMixerSubSystem() {
	for (int i = 0; i < MIXER_MAX_CHANNELS; i++) {
		_mixerChannels[i].stream = Audio::makeQueuingAudioStream(MIXER_DEFAULT_SAMPLE_RATE, false);
		_mixer->playStream(
			Audio::Mixer::kPlainSoundType,
			&_mixerChannels[i].handle,
			_mixerChannels[i].stream,
			-1,
			Audio::Mixer::kMaxChannelVolume);
	}

	for (int i = 0; i < MIXER_MAX_CHANNELS; i++) {
		mixerStartChannel(i, 0, 0, 0, 0, 0, 0, 0, CHANNEL_EMPTY_FLAGS);
	}

	return true;
}

bool HEMixer::mixerStopAllSounds() {
	for (int i = 0; i < MIXER_MAX_CHANNELS; i++) {
		mixerStopChannel(i);
	}

	return true;
}

void HEMixer::mixerFeedMixer() {
	if (_mixerPaused) {
		return;
	}

	// Unqueue any callback scripts, if available...
	if (!_vm->_insideCreateResource && _vm->_game.heversion >= 80) {
		((SoundHE *)_vm->_sound)->unqueueSoundCallbackScripts();
	}

	// Then check if any sound has ended; if so, issue a callback...
	for (int i = 0; i < MIXER_MAX_CHANNELS; i++) {
		if (!(_mixerChannels[i].flags & CHANNEL_LOOPING)) {

			// This is the check for audio overrides, it is sufficient to check if the stream is empty
			if ((_mixerChannels[i].flags & CHANNEL_ACTIVE) && _mixerChannels[i].isUsingStreamOverride) {
				if (_mixerChannels[i].stream->endOfData()) {
					_mixerChannels[i].flags &= ~CHANNEL_ACTIVE;
					_mixerChannels[i].flags |= CHANNEL_FINISHED;

					_mixerChannels[i].stream->finish();
					_mixerChannels[i].stream = nullptr;
					((SoundHE *)_vm->_sound)->digitalSoundCallback(HSND_SOUND_ENDED, _mixerChannels[i].callbackID, false);
				}

				continue;
			}

			if (!(_mixerChannels[i].flags & CHANNEL_SPOOLING)) {
				// Early callback! Play just one more audio frame from the residual data...
				if (_mixerChannels[i].flags & CHANNEL_CALLBACK_EARLY) {

					bool validEarlyCallback =
						(_mixerChannels[i].flags & CHANNEL_ACTIVE) &&
						!(_mixerChannels[i].flags & CHANNEL_LAST_CHUNK) &&
						_mixerChannels[i].stream->numQueuedStreams() == 1 &&
						_mixerChannels[i].residualData;

					if (validEarlyCallback) {
						_mixerChannels[i].flags |= CHANNEL_LAST_CHUNK;
					}
				}

				if (((_mixerChannels[i].flags & CHANNEL_ACTIVE) && _mixerChannels[i].stream->endOfData()) ||
					((_mixerChannels[i].flags & CHANNEL_ACTIVE) && (_mixerChannels[i].flags & CHANNEL_LAST_CHUNK))) {

					bool isEarlyCallback = (_mixerChannels[i].flags & CHANNEL_LAST_CHUNK);

					// For early callbacks...
					if (_mixerChannels[i].flags & CHANNEL_LAST_CHUNK)
						_mixerChannels[i].flags &= ~CHANNEL_LAST_CHUNK;


					_mixerChannels[i].flags |= CHANNEL_FINISHED;
					_mixerChannels[i].flags &= ~CHANNEL_ACTIVE;

					_mixerChannels[i].stream->finish();
					_mixerChannels[i].stream = nullptr;
					((SoundHE *)_vm->_sound)->digitalSoundCallback(HSND_SOUND_ENDED, _mixerChannels[i].callbackID, isEarlyCallback);
				}
			}

			if (_mixerChannels[i].flags & CHANNEL_SPOOLING) {
				// Callback, assuming the stream has finished playing the final chunk of the song...
				if (_mixerChannels[i].callbackOnNextFrame && _mixerChannels[i].stream->endOfData()) {
					_mixerChannels[i].callbackOnNextFrame = false;

					_mixerChannels[i].stream->finish();
					_mixerChannels[i].stream = nullptr;

					((SoundHE *)_vm->_sound)->digitalSoundCallback(HSND_SOUND_ENDED, _mixerChannels[i].callbackID);
				}

				// Last chunk fetched! Signal a callback for the next frame...
				if ((_mixerChannels[i].flags & CHANNEL_ACTIVE) && (_mixerChannels[i].flags & CHANNEL_LAST_CHUNK)) {
					_mixerChannels[i].flags &= ~CHANNEL_LAST_CHUNK;
					_mixerChannels[i].flags &= ~CHANNEL_ACTIVE;
					_mixerChannels[i].flags |= CHANNEL_FINISHED;

					_mixerChannels[i].callbackOnNextFrame = true;
				}
			}
		}
	}

	// Finally, feed the streams...
	for (int i = 0; i < MIXER_MAX_CHANNELS; i++) {
		// Do not feed the streams for audio overrides!
		if (_mixerChannels[i].isUsingStreamOverride)
			continue;

		if (_mixerChannels[i].flags & CHANNEL_ACTIVE) {
			if (_mixerChannels[i].flags & CHANNEL_SPOOLING) {
				bool looping = (_mixerChannels[i].flags & CHANNEL_LOOPING);

				if (_mixerChannels[i].stream->numQueuedStreams() > 1)
					continue;

				_mixerChannels[i].fileHandle->seek(_mixerChannels[i].initialSpoolingFileOffset, SEEK_SET);
				uint32 curOffset = _mixerChannels[i].lastReadPosition;

				_mixerChannels[i].fileHandle->seek(curOffset, SEEK_CUR);

				int length = MIXER_SPOOL_CHUNK_SIZE;
				if (_mixerChannels[i].lastReadPosition + MIXER_SPOOL_CHUNK_SIZE >= _mixerChannels[i].sampleLen) {
					length = _mixerChannels[i].sampleLen % MIXER_SPOOL_CHUNK_SIZE;
					_mixerChannels[i].flags |= CHANNEL_LAST_CHUNK;
				}

				// The file size is an exact multiple of SPOOL_CHUNK_SIZE, and we've already read the
				// last chunk, so signal it this is not a looping sample, otherwise rewind the file.
				// Should never happen, but... you know how it goes :-)
				if (length == 0) {
					if (looping) {
						curOffset = _mixerChannels[i].lastReadPosition = 0;
						_mixerChannels[i].fileHandle->seek(curOffset, SEEK_CUR);
					} else {
						_mixerChannels[i].flags |= CHANNEL_LAST_CHUNK;
						continue;
					}
				}

				byte *data = (byte *)malloc(length);
				if (data) {
					_mixerChannels[i].fileHandle->read(data, length);
					_mixerChannels[i].lastReadPosition += length;

					// If we've reached the end of the sample after a residual read,
					// and if we have to loop the sample, rewind the read position
					if (looping && _mixerChannels[i].lastReadPosition == _mixerChannels[i].sampleLen)
						_mixerChannels[i].lastReadPosition = 0;

					_mixerChannels[i].stream->queueBuffer(
						data,
						length,
						DisposeAfterUse::YES,
						mixerGetOutputFlags());
				}
			}
		}
	}
}

bool HEMixer::mixerIsMixerDisabled() {
	return false;
}

bool HEMixer::mixerStopChannel(int channel) {
	if ((channel >= 0) && (channel < MIXER_MAX_CHANNELS))
		return mixerStartChannel(channel, 0, 0, 0, 0, 0, 0, 0, CHANNEL_EMPTY_FLAGS);

	return false;
}

bool HEMixer::mixerChangeChannelVolume(int channel, int volume, bool soft) {
	byte newVolume = (byte)MAX<int>(0, MIN<int>(255, volume));
	_mixerChannels[channel].volume = newVolume;
	_mixer->setChannelVolume(_mixerChannels[channel].handle, newVolume);

	return true;
}

bool HEMixer::mixerPauseMixerSubSystem(bool paused) {
	_mixerPaused = paused;

	_mixer->pauseAll(_mixerPaused);

	return true;
}

bool HEMixer::mixerStartChannel(
	int channel, int globType, int globNum, uint32 sampleDataOffset,
	int sampleLen, int frequency, int volume, int callbackID, uint32 flags, ...) {

	va_list params;
	bool is3DOMusic = false;

	if ((channel < 0) || (channel >= MIXER_MAX_CHANNELS))
		return false;

	if (_mixerChannels[channel].flags != CHANNEL_EMPTY_FLAGS) {
		if (!(_mixerChannels[channel].flags & CHANNEL_FINISHED)) {

			_mixerChannels[channel].flags |= CHANNEL_FINISHED;
			_mixerChannels[channel].flags &= ~CHANNEL_ACTIVE;

			_mixer->stopHandle(_mixerChannels[channel].handle);
			_mixerChannels[channel].stream = nullptr;

			// Signal the sound engine that a sound has finished playing
			((SoundHE *)_vm->_sound)->digitalSoundCallback(HSND_SOUND_STOPPED, _mixerChannels[channel].callbackID);
		}
	}

	if (flags != CHANNEL_EMPTY_FLAGS) {
		if (sampleLen <= 0) {
			error("HEMixer::mixerStartChannel(): Sample invalid size %d", sampleLen);
		}
	}

	_mixerChannels[channel].flags = flags;
	_mixerChannels[channel].volume = MIN<int>(MAX<int>(0, volume), 255);
	_mixerChannels[channel].number = channel;
	_mixerChannels[channel].frequency = frequency;
	_mixerChannels[channel].callbackID = callbackID;
	_mixerChannels[channel].lastReadPosition = 0;
	_mixerChannels[channel].dataOffset = sampleDataOffset;
	_mixerChannels[channel].sampleLen = sampleLen;
	_mixerChannels[channel].globType = globType;
	_mixerChannels[channel].globNum = globNum;
	_mixerChannels[channel].residualData = nullptr;
	_mixerChannels[channel].isUsingStreamOverride = false;

	Audio::SeekableAudioStream *audioOverride = nullptr;
	if (audioOverrideExists(globNum, false, nullptr, &audioOverride)) {
		_mixerChannels[channel].isUsingStreamOverride = true;
		bool shouldLoop = (_mixerChannels[channel].flags & CHANNEL_LOOPING);

		_mixerChannels[channel].stream = Audio::makeQueuingAudioStream(
			audioOverride->getRate(),
			audioOverride->isStereo());

		_mixer->playStream(
			Audio::Mixer::kMusicSoundType,
			&_mixerChannels[channel].handle,
			_mixerChannels[channel].stream,
			-1,
			volume);

		_mixerChannels[channel].stream->queueAudioStream(
			Audio::makeLoopingAudioStream(audioOverride, shouldLoop ? 0 : 1),
			DisposeAfterUse::YES);

		return true;
	}

	bool hasCallbackData = false;
	if ((flags & CHANNEL_CALLBACK_EARLY) && !(flags & CHANNEL_LOOPING)) {
		va_start(params, flags);
		_mixerChannels[channel].endSampleAdjustment = va_arg(params, int);
		va_end(params);

		// The original has a very convoluted way of making sure that the sample
		// both callbacks earlier and stops earlier. We just set a shorter sample length
		// as this should ensure the same effect on both ends. Hopefully it's still accurate.
		if (_mixerChannels[channel].endSampleAdjustment != 0) {
			if ((int)_mixerChannels[channel].sampleLen >= _mixerChannels[channel].endSampleAdjustment) {
				_mixerChannels[channel].sampleLen -= _mixerChannels[channel].endSampleAdjustment;
				_mixerChannels[channel].residualData = (byte *)malloc(_mixerChannels[channel].endSampleAdjustment);
				hasCallbackData = _mixerChannels[channel].residualData != nullptr;
			} else {
				// Sometimes a game can give an end sample adjustment which is bigger than the sample itself
				// (looking at you, Backyard Baseball '97). In this case we should at least give one frame
				// for the sound to play for a while.
				_mixerChannels[channel].flags |= CHANNEL_LAST_CHUNK;
			}
		}

	} else {
		_mixerChannels[channel].endSampleAdjustment = 0;
	}

	if (flags != CHANNEL_EMPTY_FLAGS) {
		byte *ptr = _vm->getResourceAddress((ResType)globType, globNum);

		if (READ_BE_UINT32(ptr) == MKTAG('M', 'R', 'A', 'W')) {
			is3DOMusic = true;
		}

		if (READ_BE_UINT32(ptr) == MKTAG('W', 'S', 'O', 'U')) {
			ptr += 8;
		}

		byte *data = nullptr;
		ptr += sampleDataOffset;

		// Some looping sounds need the original resource address to
		// work properly (e.g. the ones created with createSound()),
		// so we only copy the data over and create a 64 samples fade-in
		// just for non-looping sounds. Also, remember that non-looping
		// sounds might have early callbacks, so we still have to copy
		// data over, instead of using the original buffer.
		if (!(_mixerChannels[channel].flags & CHANNEL_LOOPING)) {
			data = (byte *)malloc(_mixerChannels[channel].sampleLen);

			if (!data)
				return false;

			memcpy(data, ptr, _mixerChannels[channel].sampleLen);

			// Residual early callback data
			if (hasCallbackData) {
				memcpy(
					_mixerChannels[channel].residualData,
					&ptr[_mixerChannels[channel].sampleLen],
					_mixerChannels[channel].endSampleAdjustment);
			}

			// Fade-in to avoid possible sound popping...
			byte *dataTmp = data;
			int rampUpSampleCount = 64;
			if (!is3DOMusic) {
				for (int i = 0; i < rampUpSampleCount; i++) {
					*dataTmp = 128 + (((*dataTmp - 128) * i) / rampUpSampleCount);
					dataTmp++;
				}
			} else {
				// We can't just ramp volume as done above, we have to take
				// into account the fact that 3DO music is 8-bit -> signed <-
				rampUpSampleCount = 128;
				for (int i = 0; i < rampUpSampleCount; i++) {
					int8 signedSample = (int8)(*dataTmp);
					signedSample = (signedSample * i) / rampUpSampleCount;
					*dataTmp = (byte)signedSample;
					dataTmp++;
				}
			}
		}

		_mixerChannels[channel].stream = Audio::makeQueuingAudioStream(MIXER_DEFAULT_SAMPLE_RATE, false);

		Audio::Mixer::SoundType soundType =
			globNum == HSND_TALKIE_SLOT ?
			Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType;

		if (is3DOMusic)
			soundType = Audio::Mixer::kMusicSoundType;

		_mixer->playStream(
			soundType,
			&_mixerChannels[channel].handle,
			_mixerChannels[channel].stream,
			-1,
			volume);

		// Only HE60/61/62 games allowed for samples pitch shifting
		if (_vm->_game.heversion < 70)
			_mixer->setChannelRate(_mixerChannels[channel].handle, frequency);

		if (_mixerChannels[channel].flags & CHANNEL_LOOPING) {
			Audio::RewindableAudioStream *stream = Audio::makeRawStream(
				ptr,
				_mixerChannels[channel].sampleLen,
				MIXER_DEFAULT_SAMPLE_RATE,
				mixerGetOutputFlags(is3DOMusic),
				DisposeAfterUse::NO);

			_mixerChannels[channel].stream->queueAudioStream(Audio::makeLoopingAudioStream(stream, 0), DisposeAfterUse::YES);
		} else {
			// If we're here, data is surely a correctly allocated buffer...
			_mixerChannels[channel].stream->queueBuffer(
				data,
				_mixerChannels[channel].sampleLen,
				DisposeAfterUse::YES,
				mixerGetOutputFlags(is3DOMusic));
		}

		if (hasCallbackData && !(_mixerChannels[channel].flags & CHANNEL_LOOPING)) {
			_mixerChannels[channel].stream->queueBuffer(
				_mixerChannels[channel].residualData,
				_mixerChannels[channel].endSampleAdjustment,
				DisposeAfterUse::YES,
				mixerGetOutputFlags(is3DOMusic));
		}

	} else {
		_mixerChannels[channel].callbackOnNextFrame = false;
		_mixerChannels[channel].stream = nullptr;
	}

	return true;
}

bool HEMixer::mixerStartSpoolingChannel(
	int channel, int song, Common::File &sampleFileIOHandle, int sampleLen, int frequency,
	int volume, int callbackID, uint32 flags) {

	uint32 initialReadCount;

	if ((channel < 0) || (channel >= MIXER_MAX_CHANNELS))
		return false;

	if (_mixerChannels[channel].flags != CHANNEL_EMPTY_FLAGS) {
		if (!(_mixerChannels[channel].flags & CHANNEL_FINISHED)) {

			_mixerChannels[channel].flags |= CHANNEL_FINISHED;
			_mixerChannels[channel].flags &= ~CHANNEL_ACTIVE;

			_mixer->stopHandle(_mixerChannels[channel].handle);
			_mixerChannels[channel].stream = nullptr;

			// Signal the sound engine that a sound has finished playing
			((SoundHE *)_vm->_sound)->digitalSoundCallback(HSND_SOUND_STOPPED, callbackID);
		}
	}

	if (flags != CHANNEL_EMPTY_FLAGS) {
		if (sampleLen <= 0) {
			error("HEMixer::mixerStartSpoolingChannel(): Sample invalid size %d", sampleLen);
		}

		if ((flags & CHANNEL_LOOPING) && (sampleLen <= MIXER_PCM_CHUNK_SIZE)) {
			error("HEMixer::mixerStartSpoolingChannel(): Sample too small to loop (%d)", sampleLen);
		}
	}

	_mixerChannels[channel].flags = flags | CHANNEL_SPOOLING;
	_mixerChannels[channel].volume = MIN<int>(MAX<int>(0, volume), 255);
	_mixerChannels[channel].number = channel;
	_mixerChannels[channel].frequency = frequency;
	_mixerChannels[channel].callbackID = callbackID;
	_mixerChannels[channel].lastReadPosition = 0;
	_mixerChannels[channel].dataOffset = 0;
	_mixerChannels[channel].globType = rtSpoolBuffer;
	_mixerChannels[channel].globNum = channel + 1;
	_mixerChannels[channel].endSampleAdjustment = 0;
	_mixerChannels[channel].isUsingStreamOverride = false;

	Audio::SeekableAudioStream *audioOverride = nullptr;
	if (audioOverrideExists(song, false, nullptr, &audioOverride)) {
		_mixerChannels[channel].isUsingStreamOverride = true;
		bool shouldLoop = (_mixerChannels[channel].flags & CHANNEL_LOOPING);

		_mixerChannels[channel].stream = Audio::makeQueuingAudioStream(
			audioOverride->getRate(),
			audioOverride->isStereo());

		_mixer->playStream(
			Audio::Mixer::kMusicSoundType,
			&_mixerChannels[channel].handle,
			_mixerChannels[channel].stream,
			-1,
			volume);

		_mixerChannels[channel].stream->queueAudioStream(
			Audio::makeLoopingAudioStream(audioOverride, shouldLoop ? 0 : 1),
			DisposeAfterUse::YES);

		return true;
	}

	if (_vm->_res->createResource(
		(ResType)_mixerChannels[channel].globType,
			_mixerChannels[channel].globNum, MIXER_SPOOL_CHUNK_SIZE) == nullptr) {
		return false;
	}

	_mixerChannels[channel].fileHandle = &sampleFileIOHandle;
	_mixerChannels[channel].sampleLen = sampleLen;
	initialReadCount = MIN<int>(sampleLen, MIXER_SPOOL_CHUNK_SIZE);
	_mixerChannels[channel].initialSpoolingFileOffset = sampleFileIOHandle.pos();

	_mixerChannels[channel].stream = Audio::makeQueuingAudioStream(MIXER_DEFAULT_SAMPLE_RATE, false);

	_mixer->playStream(
		Audio::Mixer::kMusicSoundType,
		&_mixerChannels[channel].handle,
		_mixerChannels[channel].stream,
		-1,
		volume);

	byte *data = (byte *)malloc(initialReadCount);
	if (data) {
		sampleFileIOHandle.read(data, initialReadCount);

		_mixerChannels[channel].lastReadPosition += initialReadCount;

		// Freddi Fish 4 has some unhandled headers inside its spooled
		// music files which were leftovers from the development process.
		// These are 32 bytes blocks, so we can just skip them... (#13102)
		if (READ_BE_UINT32(data) == MKTAG('S', 'R', 'F', 'S')) {
			sampleFileIOHandle.seek(_mixerChannels[channel].initialSpoolingFileOffset, SEEK_SET);
			sampleFileIOHandle.seek(32, SEEK_CUR);
			sampleFileIOHandle.read(data, initialReadCount);
		}

		byte *dataTmp = data;
		int rampUpSampleCount = 64;
		for (int i = 0; i < rampUpSampleCount; i++) {
			*dataTmp = 128 + (((*dataTmp - 128) * i) / rampUpSampleCount);
			dataTmp++;
		}

		_mixerChannels[channel].stream->queueBuffer(
			data,
			initialReadCount,
			DisposeAfterUse::YES,
			mixerGetOutputFlags());
	}

	return true;
}

byte HEMixer::mixerGetOutputFlags(bool is3DOMusic) {
	// Just plain mono 8-bit sound...

	byte streamFlags = 0;

	if (!is3DOMusic)
		streamFlags |= Audio::FLAG_UNSIGNED;

#ifdef SCUMM_LITTLE_ENDIAN
	streamFlags |= Audio::FLAG_LITTLE_ENDIAN;
#endif

	return streamFlags;
}


/* --- MILES FUNCTIONS --- */

void HEMixer::milesServiceAllStreams() {
	for (int i = 0; i < MILES_MAX_CHANNELS; i++) {
		if (_milesChannels[i]._stream.streamObj != nullptr && !_milesChannels[i]._isUsingStreamOverride)
			_milesChannels[i].serviceStream();
	}
}

void HEMixer::milesStartSpoolingChannel(int channel, const char *filename, long offset, int flags, HESoundModifiers modifiers) {
	assert(channel >= 0 && channel < ARRAYSIZE(_milesChannels));

	if (_vm->_enableAudioOverride) {
		int32 songId = matchOffsetToSongId(offset);

		Audio::SeekableAudioStream *audioOverride = nullptr;
		if (songId != 0 && audioOverrideExists(songId, false, nullptr, &audioOverride)) {
			_milesChannels[channel]._playFlags = flags;
			_milesChannels[channel]._bitsPerSample = 16;
			_milesChannels[channel]._numChannels = audioOverride->isStereo() ? 2 : 1;
			_milesChannels[channel]._dataOffset = offset;
			_milesChannels[channel]._lastPlayPosition = 0;
			_milesChannels[channel]._globType = 0;
			_milesChannels[channel]._globNum = songId;
			_milesChannels[channel]._modifiers.frequencyShift = modifiers.frequencyShift;
			_milesChannels[channel]._modifiers.volume = modifiers.volume;
			_milesChannels[channel]._modifiers.pan = modifiers.pan;
			_milesChannels[channel]._baseFrequency = audioOverride->getRate();
			_milesChannels[channel]._audioHandleActive = true; // Treat it as a sound effect since we're not streaming it in chunks
			_milesChannels[channel]._isUsingStreamOverride = true;

			bool shouldLoop = (_milesChannels[channel]._playFlags & CHANNEL_LOOPING);

			if (shouldLoop) {
				// Looping sounds don't care for modifiers!
				_mixer->playStream(
					Audio::Mixer::kMusicSoundType,
					&_milesChannels[channel]._audioHandle,
					Audio::makeLoopingAudioStream(audioOverride, 0),
					channel,
					255,
					0,
					DisposeAfterUse::NO);

			} else {
				int scaledPan = (modifiers.pan != 64) ? 2 * modifiers.pan - 127 : 0;
				int newFrequency = (_milesChannels[channel]._baseFrequency * modifiers.frequencyShift) / HSND_SOUND_FREQ_BASE;

				_mixer->playStream(
					Audio::Mixer::kMusicSoundType,
					&_milesChannels[channel]._audioHandle,
					audioOverride,
					-1,
					modifiers.volume,
					scaledPan,
					DisposeAfterUse::NO);

				_mixer->setChannelRate(_milesChannels[channel]._audioHandle, newFrequency);
			}

			return;
		}
	}

	if (channel >= 0 && channel < ARRAYSIZE(_milesChannels))
		_milesChannels[channel].startSpoolingChannel(filename, offset, flags, modifiers, _mixer);
}

bool HEMixer::milesStartChannel(int channel, int globType, int globNum, uint32 soundData, uint32 offset,
	int sampleLen, int bitsPerSample, int sampleChannels, int frequency, HESoundModifiers modifiers, int callbackID, uint32 flags, ...) {

	// This function executes either a one-shot or a looping sfx/voice file
	// which will entirely fit in RAM (as opposed to spooling sounds)
	debug(5, "HEMixer::milesStartChannel(): Starting sound with resource %d in channel %d, modifiers v %d p %d f %d",
		globNum, channel, modifiers.volume, modifiers.pan, modifiers.frequencyShift);

	// Stop any running sound on the target channel; this
	// is also going to trigger the appropriate callbacks
	milesStopChannel(channel);

	// Are there any audio overrides? If so, use a different codepath...
	Audio::SeekableAudioStream *audioOverride = nullptr;
	if (audioOverrideExists(globNum, false, nullptr, &audioOverride)) {

		_milesChannels[channel]._playFlags = flags;
		_milesChannels[channel]._bitsPerSample = 16;
		_milesChannels[channel]._numChannels = audioOverride->isStereo() ? 2 : 1;
		_milesChannels[channel]._dataOffset = offset;
		_milesChannels[channel]._lastPlayPosition = 0;
		_milesChannels[channel]._globType = globType;
		_milesChannels[channel]._globNum = globNum;
		_milesChannels[channel]._modifiers.frequencyShift = modifiers.frequencyShift;
		_milesChannels[channel]._modifiers.volume = modifiers.volume;
		_milesChannels[channel]._modifiers.pan = modifiers.pan;
		_milesChannels[channel]._baseFrequency = audioOverride->getRate();
		_milesChannels[channel]._audioHandleActive = true;
		_mixerChannels[channel].isUsingStreamOverride = true;

		Audio::Mixer::SoundType soundType =
			globNum == HSND_TALKIE_SLOT ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType;

		bool shouldLoop = (_mixerChannels[channel].flags & CHANNEL_LOOPING);

		if (shouldLoop) {
			// Looping sounds don't care for modifiers!
			_mixer->playStream(
				soundType,
				&_milesChannels[channel]._audioHandle,
				Audio::makeLoopingAudioStream(audioOverride, 0),
				channel,
				255,
				0,
				DisposeAfterUse::NO);

		} else {
			int scaledPan = (modifiers.pan != 64) ? 2 * modifiers.pan - 127 : 0;
			int newFrequency = (_milesChannels[channel]._baseFrequency * modifiers.frequencyShift) / HSND_SOUND_FREQ_BASE;
			int msOffset = (offset * 1000) / newFrequency;
			audioOverride->seek(msOffset);

			_mixer->playStream(
				soundType,
				&_milesChannels[channel]._audioHandle,
				audioOverride,
				-1,
				modifiers.volume,
				scaledPan,
				DisposeAfterUse::NO);

			_mixer->setChannelRate(_milesChannels[channel]._audioHandle, newFrequency);
		}

		return true;
	}

	uint32 actualDataSize = 0;

	// Fetch the audio format for the target sound, and fill out
	// the format fields on our milesChannel
	byte *audioData = milesGetAudioDataFromResource(globType, globNum, soundData,
		_milesChannels[channel]._dataFormat, _milesChannels[channel]._blockAlign, actualDataSize);

	uint32 audioDataLen = 0;
	if (_milesChannels[channel]._dataFormat == WAVE_FORMAT_IMA_ADPCM)
		audioDataLen = actualDataSize;
	else
		audioDataLen = sampleLen * _milesChannels[channel]._blockAlign;

	_milesChannels[channel]._bitsPerSample = bitsPerSample;
	_milesChannels[channel]._numChannels = sampleChannels;

	if (audioData) {
		_vm->_res->lock((ResType)globType, globNum);

		// Fill out some other information about the sound
		_milesChannels[channel]._dataOffset = soundData + offset;
		_milesChannels[channel]._lastPlayPosition = 0;
		_milesChannels[channel]._globType = globType;
		_milesChannels[channel]._globNum = globNum;

		 // This flag signals that there's an active sound in our channel!
		_milesChannels[channel]._audioHandleActive = true;

		Audio::Mixer::SoundType soundType =
			globNum == HSND_TALKIE_SLOT ?
			Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType;

		// Play the sound, whether in one-shot fashion or as a loop
		if (flags & CHANNEL_LOOPING) {
			_milesChannels[channel]._playFlags = CHANNEL_LOOPING;

			// Looping sounds don't care for modifiers!
			Audio::RewindableAudioStream *stream = nullptr;
			if (_milesChannels[channel]._dataFormat == WAVE_FORMAT_PCM) {
				stream = Audio::makeRawStream(audioData, audioDataLen, frequency, _milesChannels[channel].getOutputFlags());

			} else if (_milesChannels[channel]._dataFormat == WAVE_FORMAT_IMA_ADPCM) {
				Common::MemoryReadStream memStream(audioData, audioDataLen);
				Audio::AudioStream *adpcmStream = Audio::makeADPCMStream(&memStream, DisposeAfterUse::NO, audioDataLen, Audio::kADPCMMSIma,
						frequency, _milesChannels[channel]._numChannels, _milesChannels[channel]._blockAlign);

				byte *adpcmData = (byte *)malloc(audioDataLen * 4);
				uint32 adpcmSize = adpcmStream->readBuffer((int16 *)(void *)adpcmData, audioDataLen * 2);
				delete adpcmStream;

				adpcmSize *= 2;
				stream = Audio::makeRawStream(adpcmData, adpcmSize, frequency, _milesChannels[channel].getOutputFlags());
			}

			if (stream) {
				_mixer->playStream(soundType, &_milesChannels[channel]._audioHandle,
						Audio::makeLoopingAudioStream(stream, 0), channel, 255, 0, DisposeAfterUse::NO);
			}

		} else {
			_milesChannels[channel]._playFlags = CHANNEL_EMPTY_FLAGS;

			// Fill out the modifiers
			int newFrequency = (frequency * modifiers.frequencyShift) / HSND_SOUND_FREQ_BASE;
			int msOffset = (offset * 1000) / newFrequency;

			_milesChannels[channel]._modifiers.frequencyShift = modifiers.frequencyShift;
			_milesChannels[channel]._modifiers.volume = modifiers.volume;
			_milesChannels[channel]._modifiers.pan = modifiers.pan;

			// Set the target sample rate for the playback
			_milesChannels[channel]._baseFrequency = frequency;

			// Transform the range of the pan value from [0, 127] to [-127, 127]
			int scaledPan = (_milesChannels[channel]._modifiers.pan != 64) ? 2 * _milesChannels[channel]._modifiers.pan - 127 : 0;

			// Play the one-shot sound!
			Audio::SeekableAudioStream *stream = nullptr;
			if (_milesChannels[channel]._dataFormat == WAVE_FORMAT_PCM) {
				stream = Audio::makeRawStream(audioData, audioDataLen, newFrequency, _milesChannels[channel].getOutputFlags());

			} else if (_milesChannels[channel]._dataFormat == WAVE_FORMAT_IMA_ADPCM) {
				Common::MemoryReadStream memStream(audioData, audioDataLen);
				Audio::AudioStream *adpcmStream = Audio::makeADPCMStream(&memStream, DisposeAfterUse::NO, audioDataLen, Audio::kADPCMMSIma,
						_milesChannels[channel]._baseFrequency, _milesChannels[channel]._numChannels, _milesChannels[channel]._blockAlign);

				byte *adpcmData = (byte *)malloc(audioDataLen * 4);
				uint32 adpcmSize = adpcmStream->readBuffer((int16 *)(void *)adpcmData, audioDataLen * 2);
				delete adpcmStream;

				adpcmSize *= 2;
				stream = Audio::makeRawStream(adpcmData, adpcmSize, newFrequency, _milesChannels[channel].getOutputFlags());
			}

			if (stream) {
				stream->seek(msOffset);
				_mixer->playStream(soundType, &_milesChannels[channel]._audioHandle,
								   stream, channel, _milesChannels[channel]._modifiers.volume, scaledPan, DisposeAfterUse::NO);
			}
		}
	}

	return true;
}

bool HEMixer::milesStopChannel(int channel) {
	milesStopAndCallback(channel, HSND_SOUND_STOPPED);

	return true;
}

void HEMixer::milesStopAllSounds() {
	for (int i = 0; i < MILES_MAX_CHANNELS; i++) {
		milesStopChannel(i);
	}
}

void HEMixer::milesModifySound(int channel, int offset, HESoundModifiers modifiers, int flags) {
	// This function usually serves as a parameter fade handler (e.g. fading out volume in Moonbase)
	debug(5, "HEMixer::milesModifySound(): modifying sound in channel %d, flags %d, vol %d, pan %d, freq %d",
		channel, flags, modifiers.volume, modifiers.pan, modifiers.frequencyShift);

	// Handling for non-streamed sound effects
	if (_milesChannels[channel]._audioHandleActive) {
		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_VOL)
			_milesChannels[channel]._modifiers.volume = modifiers.volume;

		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_PAN)
			_milesChannels[channel]._modifiers.pan = modifiers.pan;

		if ((flags & ScummEngine_v70he::HESndFlags::HE_SND_VOL) || (flags & ScummEngine_v70he::HESndFlags::HE_SND_PAN)) {
			// Transform the range of the pan value from [0, 127] to [-127, 127]
			int scaledPan = (_milesChannels[channel]._modifiers.pan != 64) ? 2 * _milesChannels[channel]._modifiers.pan - 127 : 0;

			_mixer->setChannelVolume(_milesChannels[channel]._audioHandle, _milesChannels[channel]._modifiers.volume);
			_mixer->setChannelBalance(_milesChannels[channel]._audioHandle, scaledPan);
		}

		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_FREQUENCY) {
			_milesChannels[channel]._modifiers.frequencyShift = modifiers.frequencyShift;
			int newFrequency = (_milesChannels[channel]._baseFrequency * modifiers.frequencyShift) / HSND_SOUND_FREQ_BASE;
			if (newFrequency)
				_mixer->setChannelRate(_milesChannels[channel]._audioHandle, newFrequency);
		}
	}

	// Handling for streamed music
	if (_milesChannels[channel]._stream.streamObj) {
		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_VOL) {
			_milesChannels[channel]._modifiers.volume = modifiers.volume;
			_mixer->setChannelVolume(_milesChannels[channel]._stream.streamHandle, _milesChannels[channel]._modifiers.volume);
		}
		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_PAN) {
			_milesChannels[channel]._modifiers.pan = modifiers.pan;
			// Transform the range of the pan value from [0, 127] to [-127, 127]
			int scaledPan = (_milesChannels[channel]._modifiers.pan != 64) ? 2 * _milesChannels[channel]._modifiers.pan - 127 : 0;
			_mixer->setChannelBalance(_milesChannels[channel]._stream.streamHandle, scaledPan);
		}

		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_FREQUENCY) {
			_milesChannels[channel]._modifiers.frequencyShift = modifiers.frequencyShift;
			int newFrequency = (_milesChannels[channel]._baseFrequency * modifiers.frequencyShift) / HSND_SOUND_FREQ_BASE;
			if (newFrequency)
				_mixer->setChannelRate(_milesChannels[channel]._stream.streamHandle, newFrequency);
		}
	}
}

void HEMixer::milesStopAndCallback(int channel, int messageId) {
	// In here we check if we can actually stop the target channel,
	// and if so, we call the script callback.

	if (!_milesChannels[channel]._audioHandleActive && !_milesChannels[channel]._stream.streamObj) {
		return;
	}

	if (_milesChannels[channel]._audioHandleActive) { // Non streamed sounds
		// Stop the sound, and then unload it...
		_mixer->stopHandle(_milesChannels[channel]._audioHandle);

		int globType = _milesChannels[channel]._globType;
		int globNum = _milesChannels[channel]._globNum;

		if (!_milesChannels[channel]._isUsingStreamOverride && !_vm->_res->isOffHeap((ResType)globType, globNum)) {
			_vm->_res->unlock((ResType)globType, globNum);

			if (globType == rtSound && globNum == HSND_TALKIE_SLOT) {
				// Voice files have to be manually purged
				_vm->_res->nukeResource((ResType)globType, globNum);
			}
		}
	} else { // Streamed music
		_milesChannels[channel]._stream.streamObj->finish();
		_mixer->stopHandle(_milesChannels[channel]._stream.streamHandle);

		if (_milesChannels[channel]._stream.fileHandle)
			_milesChannels[channel]._stream.fileHandle->close();
	}

	// Clean up the channel
	_milesChannels[channel].clearChannelData();

	// Signal the sound engine that we've stopped a sound
	((SoundHE *)_vm->_sound)->digitalSoundCallback(messageId, channel);
}

void HEMixer::milesFeedMixer() {
	if (_mixerPaused) {
		return;
	}

	// Feed the audio streams
	milesServiceAllStreams();

	// Check for any sound which has finished playing and call the milesStopAndCallback function
	for (int i = 0; i < MILES_MAX_CHANNELS; i++) {
		bool soundDone = false;

		if (_milesChannels[i]._audioHandleActive) {
			soundDone = !_mixer->isSoundHandleActive(_milesChannels[i]._audioHandle);
		}

		if (_milesChannels[i]._stream.streamObj && !_milesChannels[i]._isUsingStreamOverride) {
			soundDone |= _milesChannels[i]._stream.streamObj->endOfStream();
			soundDone |= !_mixer->isSoundHandleActive(_milesChannels[i]._stream.streamHandle);
		}

		if (soundDone) {
			milesStopAndCallback(i, HSND_SOUND_ENDED);
		}
	}

	// Finally, check the callback queue and execute any pending callback script
	if (!_vm->_insideCreateResource) {
		((SoundHE *)_vm->_sound)->unqueueSoundCallbackScripts();
	}
}

bool HEMixer::milesPauseMixerSubSystem(bool paused) {
	_mixerPaused = paused;

	_mixer->pauseAll(_mixerPaused);

	return true;
}

byte *HEMixer::milesGetAudioDataFromResource(int globType, int globNum, uint32 dataOffset, uint16 &compType, uint16 &blockAlign, uint32 &dataSize) {
	// This function is used for non streamed sound effects and voice files,
	// and fetches metadata for the target sound resource

	byte *globPtr = _vm->getResourceAddress((ResType)globType, globNum);

	if (globPtr == nullptr) {
		error("HEMixer::milesGetAudioDataFromResource(): Glob(%d,%d) missing from heap", globType, globNum);
	}

	uint32 globId = READ_BE_UINT32(globPtr);

	if (globId != MKTAG('W', 'S', 'O', 'U')) {
		debug(5, "HEMixer::milesGetAudioDataFromResource(): Glob(%d,%d) - type '%s' - is not a WSOU (wrapped .wav) file", globType, globNum, tag2str(globId));
		return nullptr;
	}

	// WSOU tag found, skip to the RIFF header...
	globPtr += 8;
	globId = READ_BE_UINT32(globPtr);

	if (globId != MKTAG('R', 'I', 'F', 'F')) {
		debug(5, "HEMixer::milesGetAudioDataFromResource(): Glob(%d,%d) - '%s' - is not a .wav file", globType, globNum, tag2str(globId));
		return nullptr;
	}

	compType = READ_LE_UINT16(globPtr + 20); // Format type from the 'fmt ' block
	blockAlign = READ_LE_UINT16(globPtr + 32); // Block align field

	if (compType != WAVE_FORMAT_PCM && compType != WAVE_FORMAT_IMA_ADPCM) {
		debug("HEMixer::milesGetAudioDataFromResource(): .wav files must be PCM or IMA ADPCM. Unsupported .wav sound type %d.", compType);
		return nullptr;
	}

	// Check for the 'data' chunk
	if (READ_BE_UINT32(globPtr + (dataOffset - 8)) != MKTAG('d', 'a', 't', 'a')) {
		debug("HEMixer::milesGetAudioDataFromResource(): Did not find 'data' chunk in .wav file");
		return nullptr;
	}

	// The 'data' chunk size is immediately after the 'data' chunk ID
	dataSize = READ_LE_UINT32(globPtr + (dataOffset - 4));

	return globPtr + dataOffset;
}

void HEMilesChannel::startSpoolingChannel(const char *filename, long offset, int flags, HESoundModifiers modifiers, Audio::Mixer *mixer) {
	// This function sets up a stream for the target spooling music file.
	// applies modifiers, and begins playing said stream

	// We use this auxiliary struct representing a RIFF header for two things:
	// - Tidier code;
	// - File read consistency checks
	//
	// I certainly could have done without it, but I feel the code is
	// more readable and comprehensible this way...
	struct WaveHeader {
		uint32 riffTag;
		uint32 riffSize;
		uint32 waveTag;
		uint32 fmtTag;
		uint32 fmtSize;

		// Format subchunk
		uint16 wFormatTag;
		uint16 wChannels;
		uint32 dwSamplesPerSec;
		uint32 dwAvgBytesPerSec;
		uint16 wBlockAlign;
		uint16 wBitsPerSample;
	};

	WaveHeader waveHeader;

	// Open the music bundle file and then seek to the target sound
	_stream.fileHandle = new Common::File();

	if (!_stream.fileHandle->open(filename)) {
		debug(5, "HEMilesChannel::startSpoolingChannel(): Couldn't open spooling file '%s'", filename);
		return;
	}

	_stream.fileHandle->seek(offset, SEEK_CUR);
	if (_stream.fileHandle->pos() != offset) {
		debug(5, "HEMilesChannel::startSpoolingChannel(): Couldn't seek file %s to offset %ld", filename, offset);
		_stream.fileHandle->close();
		return;
	}

	// Extract the usual metadata information from the header, for later usage
	int beginningPos = _stream.fileHandle->pos();

	waveHeader.riffTag = _stream.fileHandle->readUint32BE();
	waveHeader.riffSize = _stream.fileHandle->readUint32LE();
	waveHeader.waveTag = _stream.fileHandle->readUint32BE();
	waveHeader.fmtTag = _stream.fileHandle->readUint32BE();
	waveHeader.fmtSize = _stream.fileHandle->readUint32LE();

	int fmtPos = _stream.fileHandle->pos();

	waveHeader.wFormatTag = _stream.fileHandle->readUint16LE();
	waveHeader.wChannels = _stream.fileHandle->readUint16LE();
	waveHeader.dwSamplesPerSec = _stream.fileHandle->readUint32LE();
	waveHeader.dwAvgBytesPerSec = _stream.fileHandle->readUint32LE();
	waveHeader.wBlockAlign = _stream.fileHandle->readUint16LE();
	waveHeader.wBitsPerSample = _stream.fileHandle->readUint16LE();

	// Some safety checks...
	if (_stream.fileHandle->pos() - beginningPos != sizeof(waveHeader)) {
		debug(5, "HEMilesChannel::startSpoolingChannel(): Couldn't read RIFF header correctly");
		_stream.fileHandle->close();
		return;
	}

	if (waveHeader.riffTag != MKTAG('R', 'I', 'F', 'F')) {
		debug(5, "HEMilesChannel::startSpoolingChannel(): Expected RIFF tag, found %s instead", tag2str(waveHeader.riffTag));
		return;
	}

	if (waveHeader.waveTag != MKTAG('W', 'A', 'V', 'E')) {
		debug(5, "HEMilesChannel::startSpoolingChannel(): Expected WAVE tag, found %s instead", tag2str(waveHeader.waveTag));
		return;
	}

	if (waveHeader.fmtTag != MKTAG('f', 'm', 't', ' ')) {
		debug(5, "HEMilesChannel::startSpoolingChannel(): Expected fmt tag, found %s instead", tag2str(waveHeader.fmtTag));
		return;
	}

	// Fill out the relevant metadata for our channel
	_modifiers.volume = modifiers.volume;
	_modifiers.pan = modifiers.pan;
	_modifiers.frequencyShift = modifiers.frequencyShift;

	_dataFormat = waveHeader.wFormatTag;
	_blockAlign = waveHeader.wBlockAlign;
	_numChannels = waveHeader.wChannels;
	_bitsPerSample = waveHeader.wBitsPerSample;
	_baseFrequency = waveHeader.dwSamplesPerSec;

	// Transform the range of the pan value from [0, 127] to [-127, 127]
	int scaledPan = (_modifiers.pan != 64) ? 2 * _modifiers.pan - 127 : 0;
	int newFrequency = (_baseFrequency * modifiers.frequencyShift) / HSND_SOUND_FREQ_BASE;

	// Create our stream and start it
	_stream.streamObj = Audio::makeQueuingAudioStream(_baseFrequency, (waveHeader.wChannels > 1));
	mixer->playStream(Audio::Mixer::kMusicSoundType, &_stream.streamHandle, _stream.streamObj, -1, 255, 0, DisposeAfterUse::NO);

	if (_dataFormat == WAVE_FORMAT_PCM) {
		// Apply the modifiers and the loop flag, if available
		mixer->setChannelVolume(_stream.streamHandle, _modifiers.volume);
		mixer->setChannelBalance(_stream.streamHandle, scaledPan);
		if (newFrequency)
			mixer->setChannelRate(_stream.streamHandle, newFrequency);

		_stream.loopFlag = flags & ScummEngine_v70he::HESndFlags::HE_SND_LOOP;

		// And now we help out the stream by feeding the first bytes of data to it
		_stream.fileHandle->readUint32BE(); // Skip 'data' tag

		_stream.dataLength = _stream.fileHandle->readUint32LE();
		_stream.curDataPos = 0;
		_stream.dataOffset = _stream.fileHandle->pos();
	} else if (_dataFormat == WAVE_FORMAT_IMA_ADPCM) {
		// IMA ADPCM might have a longer header, so use the previously obtained
		// information to jump through blocks and find the 'data' tag
		_stream.fileHandle->seek(fmtPos, SEEK_SET);
		_stream.fileHandle->seek(waveHeader.fmtSize, SEEK_CUR);

		uint32 curTag = 0;
		while ((curTag = _stream.fileHandle->readUint32BE()) != MKTAG('d', 'a', 't', 'a')) {
			uint32 blockSize = _stream.fileHandle->readUint32LE();
			_stream.fileHandle->seek(blockSize, SEEK_CUR);
			debug(5, "HEMixer::milesStartChannel(): APDCM spooling sound, searching for 'data' tag, now on '%s' tag...",
				tag2str(curTag));

			if (_stream.fileHandle->eos()) {
				debug(5, "HEMixer::milesStartChannel(): APDCM spooling sound, couldn't find 'data' block, bailing out...");
				return;
			}
		}

		_stream.dataLength = _stream.fileHandle->readUint32LE();
		_stream.curDataPos = 0;
		_stream.dataOffset = _stream.fileHandle->pos();
	} else {
		debug(5, "HEMixer::milesStartChannel(): Unexpected sound format %d in sound file '%s' at offset %ld",
			  _dataFormat, filename, offset);
	}

	// Saturate the stream queue with the beginning of the audio data
	for (int i = 0; i < MILES_MAX_QUEUED_STREAMS; i++)
		serviceStream();
}

void HEMilesChannel::clearChannelData() {
	_audioHandleActive = false;
	_lastPlayPosition = 0;
	_playFlags = 0;
	_dataOffset = 0;
	_globType = 0;
	_globNum = 0;

	_baseFrequency = 0;
	_modifiers.volume = HSND_MAX_VOLUME;
	_modifiers.pan = HSND_SOUND_PAN_CENTER;
	_modifiers.frequencyShift = HSND_BASE_FREQ_FACTOR;

	closeFileHandle();
	_stream.fileHandle = nullptr;
	_stream.streamObj = nullptr;
	_stream.loopFlag = false;
	_stream.dataLength = 0;
	_stream.curDataPos = 0;
	_stream.dataOffset = 0;

	_bitsPerSample = 8;
	_numChannels = 1;
	_dataFormat = WAVE_FORMAT_PCM;

	_isUsingStreamOverride = false;
}

void HEMilesChannel::closeFileHandle() {
	if (_stream.fileHandle && _stream.fileHandle->isOpen())
		_stream.fileHandle->close();
}

byte HEMilesChannel::getOutputFlags() {
	byte streamFlags = 0;
	if (_bitsPerSample == 8) // 8 bit data is unsigned
		streamFlags |= Audio::FLAG_UNSIGNED;
	else if (_bitsPerSample == 16) // 16 bit data is signed little endian
		streamFlags |= (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
	else if (_bitsPerSample == 24) // 24 bit data is signed little endian
		streamFlags |= (Audio::FLAG_24BITS | Audio::FLAG_LITTLE_ENDIAN);
	else if (_bitsPerSample == 4 && (_dataFormat == WAVE_FORMAT_IMA_ADPCM))
		streamFlags |= Audio::FLAG_16BITS;

	if (_numChannels == 2)
		streamFlags |= Audio::FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
	if (_dataFormat == WAVE_FORMAT_IMA_ADPCM)
		streamFlags |= Audio::FLAG_LITTLE_ENDIAN;
#endif

	return streamFlags;
}

void HEMilesChannel::serviceStream() {
	bool reachedTheEnd = false;
	uint32 sizeToRead = 0;

	// This is called at each frame, to ensure that the target stream doesn't starve
	if (_stream.streamObj->numQueuedStreams() < MILES_MAX_QUEUED_STREAMS) {
		if (_dataFormat == WAVE_FORMAT_PCM) {
			sizeToRead = MIN<uint32>(MILES_PCM_CHUNK_SIZE * _blockAlign, _stream.dataLength - _stream.curDataPos);
			reachedTheEnd = sizeToRead < MILES_PCM_CHUNK_SIZE * _blockAlign;


			if (sizeToRead > 0) {
				byte *buffer = (byte *)malloc(sizeToRead);
				if (buffer != nullptr) {
					int readBytes = _stream.fileHandle->read(buffer, sizeToRead);
					_stream.curDataPos += readBytes;
					_stream.streamObj->queueBuffer(buffer, readBytes, DisposeAfterUse::YES, getOutputFlags());
				}
			}

		} else if (_dataFormat == WAVE_FORMAT_IMA_ADPCM) {
			// Look, I know: it's some of the ugliest code you've ever seen. Sorry.
			// Unfortunately when it comes to streaming ADPCM audio from a file this is as
			// clean as I can possibly make it (instead of loading and keeping the whole
			// thing in memory, that is).

			sizeToRead = MIN<uint32>(MILES_IMA_ADPCM_PER_FRAME_CHUNKS_NUM * _blockAlign, _stream.dataLength - _stream.curDataPos);
			reachedTheEnd = sizeToRead < MILES_IMA_ADPCM_PER_FRAME_CHUNKS_NUM * _blockAlign;

			// We allocate a buffer which is going to be filled with
			// (MILES_IMA_ADPCM_PER_FRAME_CHUNKS_NUM) compressed blocks or less
			if (sizeToRead > 0) {
				byte *compressedBuffer = (byte *)malloc(sizeToRead);
				if (compressedBuffer != nullptr){
					int readBytes = _stream.fileHandle->read(compressedBuffer, sizeToRead);
					_stream.curDataPos += readBytes;

					// Now, the ugly trick: use a MemoryReadStream containing our compressed data,
					// to feed an ADPCM stream, and then use the latter to read uncompressed data,
					// and then queue the latter in the output stream.
					// Hey, it IS ugly! ...and it works :-)
					Common::MemoryReadStream memStream(compressedBuffer, readBytes);
					Audio::AudioStream *adpcmStream = Audio::makeADPCMStream(&memStream, DisposeAfterUse::NO,
																			 readBytes, Audio::kADPCMMSIma, _baseFrequency, _numChannels, _blockAlign);

					uint32 uncompSize =
						calculateDeflatedADPCMBlockSize(MILES_IMA_ADPCM_PER_FRAME_CHUNKS_NUM, _blockAlign, _numChannels, 16);

					byte *adpcmData = (byte *)malloc(uncompSize);
					uint32 adpcmSize = adpcmStream->readBuffer((int16 *)(void *)adpcmData, uncompSize * 2);

					adpcmSize *= 2;
					_stream.streamObj->queueBuffer(adpcmData, adpcmSize, DisposeAfterUse::YES, getOutputFlags());

					delete adpcmStream;
					free(compressedBuffer);
				}
			}
		}

		if (reachedTheEnd) {
			if (_stream.loopFlag) {
				// Rewind the stream...
				_stream.curDataPos = 0;
				_stream.fileHandle->seek(_stream.dataOffset, SEEK_SET);
			} else {
				// Mark the stream as finished...
				_stream.streamObj->finish();
			}
		}
	}
}

} // End of namespace Scumm
