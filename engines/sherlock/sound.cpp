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

#include "sherlock/sherlock.h"
#include "sherlock/sound.h"
#include "common/config-manager.h"
#include "audio/audiostream.h"
#include "common/algorithm.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/wave.h"

namespace Sherlock {

static const int8 creativeADPCM_ScaleMap[64] = {
	0,  1,  2,  3,  4,  5,  6,  7,  0,  -1,  -2,  -3,  -4,  -5,  -6,  -7,
	1,  3,  5,  7,  9, 11, 13, 15, -1,  -3,  -5,  -7,  -9, -11, -13, -15,
	2,  6, 10, 14, 18, 22, 26, 30, -2,  -6, -10, -14, -18, -22, -26, -30,
	4, 12, 20, 28, 36, 44, 52, 60, -4, -12, -20, -28, -36, -44, -52, -60
};

static const uint8 creativeADPCM_AdjustMap[64] = {
	0, 0, 0, 0, 0, 16, 16, 16,
	0, 0, 0, 0, 0, 16, 16, 16,
	240, 0, 0, 0, 0, 16, 16, 16,
	240, 0, 0, 0, 0, 16, 16, 16,
	240, 0, 0, 0, 0, 16, 16, 16,
	240, 0, 0, 0, 0, 16, 16, 16,
	240, 0, 0, 0, 0,  0,  0,  0,
	240, 0, 0, 0, 0,  0,  0,  0
};

/*----------------------------------------------------------------*/

Sound::Sound(SherlockEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer) {
	_digitized = false;
	_voices = 0;
	_soundPlaying = false;
	_speechPlaying = false;
	_curPriority = 0;
	_soundVolume = ConfMan.hasKey("sfx_volume") ? ConfMan.getInt("sfx_volume") : 255;
	_soundOn = ConfMan.hasKey("mute") ? !ConfMan.getBool("mute") : true;
	_speechOn = ConfMan.hasKey("speech_mute") ? !ConfMan.getBool("speech_mute") : true;

	if (IS_3DO) {
		// 3DO: we don't need to prepare anything for sound
		return;
	}

	_vm->_res->addToCache("MUSIC.LIB");
	if (!_vm->_interactiveFl)
		_vm->_res->addToCache("TITLE.SND");
	else {
		_vm->_res->addToCache("MUSIC.LIB");

		if (IS_ROSE_TATTOO) {
			_vm->_res->addToCache("SOUND.LIB");
		} else {
			_vm->_res->addToCache("SND.SND");

			if (!_vm->isDemo()) {
				_vm->_res->addToCache("TITLE.SND");
				_vm->_res->addToCache("EPILOGUE.SND");
			}
		}
	}
}

void Sound::syncSoundSettings() {
	_digitized = !ConfMan.getBool("mute");
	_speechOn = !ConfMan.getBool("mute") && !ConfMan.getBool("speech_mute");
	_voices = _digitized ? 1 : 0;
}

void Sound::loadSound(const Common::String &name, int priority) {
	// No implementation required in ScummVM
	//warning("loadSound");
}

byte Sound::decodeSample(byte sample, byte &reference, int16 &scale) {
	int16 samp = sample + scale;
	int16 ref = 0;

	// clip bad ADPCM-4 sample
	samp = CLIP<int16>(samp, 0, 63);

	ref = reference + creativeADPCM_ScaleMap[samp];
	if (ref > 0xff) {
		reference = 0xff;
	} else {
		if (ref < 0x00) {
			reference = 0;
		} else {
			reference = (uint8)(ref & 0xff);
		}
	}

	scale = (scale + creativeADPCM_AdjustMap[samp]) & 0xff;
	return reference;
}

bool Sound::playSound(const Common::String &name, WaitType waitType, int priority, const char *libraryFilename) {
	// Scalpel has only a single sound handle, so it must be stopped before starting a new sound
	if (IS_SERRATED_SCALPEL)
		stopSound();

	Common::String filename = formFilename(name);

	Audio::SoundHandle &soundHandle = (IS_SERRATED_SCALPEL) ? _scalpelEffectsHandle : getFreeSoundHandle();
	if (!playSoundResource(filename, libraryFilename, Audio::Mixer::kSFXSoundType, soundHandle))
		error("Could not find sound resource - %s", filename.c_str());

	_soundPlaying = true;
	_curPriority = priority;

	if (waitType == WAIT_RETURN_IMMEDIATELY) {
		return true;
	}

	bool retval = true;
	do {
		_vm->_events->pollEvents();
		g_system->delayMillis(10);
		if ((waitType == WAIT_KBD_OR_FINISH) && _vm->_events->kbHit()) {
			retval = false;
			break;
		}
	} while (!_vm->shouldQuit() && _mixer->isSoundHandleActive(soundHandle));

	_soundPlaying = false;
	_mixer->stopHandle(soundHandle);

	return retval;
}

Common::String Sound::formFilename(const Common::String &name) {
	Common::String filename = name;

	if (!filename.contains('.')) {
		if (!IS_3DO) {
			if (IS_SERRATED_SCALPEL) {
				filename += ".SND";
			} else {
				filename += ".WAV";
			}
		} else {
			// 3DO uses .aiff extension
			filename += ".AIFF";
			if (!filename.contains('/')) {
				// if no directory was given, use the room sounds directory
				filename = "rooms/sounds/" + filename;
			}
		}
	}

	return filename;
}

void Sound::playAiff(const Common::String &name, int volume, bool loop) {
	Common::File *file = new Common::File();
	if (!file->open(name)) {
		delete file;
		return;
	}
	Audio::AudioStream *stream;
	Audio::RewindableAudioStream *audioStream = Audio::makeAIFFStream(file, DisposeAfterUse::YES);
	if (loop) {
		Audio::AudioStream *loopingStream = Audio::makeLoopingAudioStream(audioStream, 0);
		stream = loopingStream;
	} else {
		stream = audioStream;
	}
	stopAiff();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_aiffHandle, stream, -1, volume);
}

void Sound::stopAiff() {
	if (_mixer->isSoundHandleActive(_aiffHandle)) {
		_mixer->stopHandle(_aiffHandle);
	}
}

void Sound::playLoadedSound(int bufNum, WaitType waitType) {
	if (IS_SERRATED_SCALPEL) {
		if (_mixer->isSoundHandleActive(_scalpelEffectsHandle) && (_curPriority > _vm->_scene->_sounds[bufNum]._priority))
			return;

		stopSound();
	}

	playSound(_vm->_scene->_sounds[bufNum]._name, waitType, _vm->_scene->_sounds[bufNum]._priority);
}

void Sound::freeLoadedSounds() {
	// As sounds are played with DisposeAfterUse::YES, stopping the sounds also
	// frees them
	stopSound();
}

void Sound::stopSound() {
	if (IS_SERRATED_SCALPEL) {
		_mixer->stopHandle(_scalpelEffectsHandle);
	} else {
		for (int i = 0; i < MAX_MIXER_CHANNELS; i++)
			_mixer->stopHandle(_tattooEffectsHandle[i]);
	}
}

void Sound::freeDigiSound() {
	_soundPlaying = false;
}

Audio::SoundHandle &Sound::getFreeSoundHandle() {
	for (int i = 0; i < MAX_MIXER_CHANNELS; i++) {
		if (!_mixer->isSoundHandleActive(_tattooEffectsHandle[i]))
			return _tattooEffectsHandle[i];
	}

	error("getFreeSoundHandle: No sound handle found");
}

void Sound::setVolume(int volume) {
	_soundVolume = volume;
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, volume);
}

void Sound::playSpeech(const Common::String &name) {
	Resources &res = *_vm->_res;
	Scene &scene = *_vm->_scene;

	// Stop any previously playing speech
	stopSpeech();

	if (IS_SERRATED_SCALPEL) {
		Common::String filename = formFilename(name);
		if (playSoundResource(filename, Common::String(), Audio::Mixer::kSFXSoundType, _speechHandle))
			_speechPlaying = true;
	} else {
		// Figure out which speech library to use
		Common::String libraryName = Common::String::format("speech%02d.lib", scene._currentScene);
		if ((!scumm_strnicmp(name.c_str(), "SLVE12S", 7)) || (!scumm_strnicmp(name.c_str(), "WATS12X", 7))
				|| (!scumm_strnicmp(name.c_str(), "HOLM12X", 7)))
			libraryName = "SPEECH12.LIB";

		// If the speech library file doesn't even exist, then we can't play anything
		Common::File f;
		if (!f.exists(libraryName))
			return;

		// Ensure the given library is in the cache
		res.addToCache(libraryName);

		if (playSoundResource(name, libraryName, Audio::Mixer::kSpeechSoundType, _speechHandle))
			_speechPlaying = true;
	}
}

void Sound::stopSpeech() {
	_mixer->stopHandle(_speechHandle);
	_speechPlaying = false;
}

bool Sound::isSpeechPlaying() {
	_speechPlaying = _mixer->isSoundHandleActive(_speechHandle);
	return _speechPlaying;
}

bool Sound::playSoundResource(const Common::String &name, const Common::String &libFilename,
		Audio::Mixer::SoundType soundType, Audio::SoundHandle &handle) {
	Resources &res = *_vm->_res;
	Common::SeekableReadStream *stream = libFilename.empty() ? res.load(name) : res.load(name, libFilename, true);
	if (!stream)
		return false;

	Audio::AudioStream *audioStream;
	if (IS_ROSE_TATTOO && soundType == Audio::Mixer::kSpeechSoundType) {
		audioStream = Audio::makeRawStream(stream, 11025, Audio::FLAG_UNSIGNED);
	} else if (IS_3DO) {
		// 3DO: AIFF file
		audioStream = Audio::makeAIFFStream(stream, DisposeAfterUse::YES);
	} else if (IS_SERRATED_SCALPEL) {
		stream->skip(2);
		int size = stream->readUint32BE();
		int rate = stream->readUint16BE();
		byte *data = (byte *)malloc(size);
		byte *ptr = data;
		stream->read(ptr, size);
		delete stream;

		assert(size > 2);

		byte *decoded = (byte *)malloc((size - 1) * 2);

		// Holmes uses Creative ADPCM 4-bit data
		int counter = 0;
		byte reference = ptr[0];
		int16 scale = 0;

		for (int i = 1; i < size; i++) {
			decoded[counter++] = decodeSample((ptr[i] >> 4) & 0x0f, reference, scale);
			decoded[counter++] = decodeSample((ptr[i] >> 0) & 0x0f, reference, scale);
		}

		free(data);

		audioStream = Audio::makeRawStream(decoded, (size - 2) * 2, rate, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	} else {
		audioStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	}

	if (!audioStream)
		return false;

	_mixer->playStream(soundType, &handle, audioStream, -1, Audio::Mixer::kMaxChannelVolume);
	return true;
}

} // End of namespace Sherlock

