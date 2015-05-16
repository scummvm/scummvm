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

namespace Sherlock {

Sound::Sound(SherlockEngine *vm, Audio::Mixer *mixer): _vm(vm), _mixer(mixer) {
	_digitized = false;
	_music = false;
	_voices = 0;
	_playingEpilogue = false;
	_diskSoundPlaying = false;
	_soundPlaying = false;
	_soundIsOn = &_soundPlaying;
	_curPriority = 0;

	_soundOn = true;
	_musicOn = true;
	_speechOn = true;

	_vm->_res->addToCache("MUSIC.LIB");
	_vm->_res->addToCache("TITLE.SND");
	_vm->_res->addToCache("EPILOGUE.SND");
}

/**
 * Saves sound-related settings
 */
void Sound::syncSoundSettings() {
	_digitized = !ConfMan.getBool("mute");
	_music = !ConfMan.getBool("mute") && !ConfMan.getBool("music_mute");
	_voices = !ConfMan.getBool("mute") && !ConfMan.getBool("speech_mute") ? 1 : 0;
}

void Sound::loadSound(const Common::String &name, int priority) {
	// No implementation required in ScummVM
}

char Sound::decodeSample(char sample, byte& prediction, int& step) {
	char diff = ((sample & 0x07) << step);

	if (sample & 0x08) {
		if (prediction > diff)
			prediction = prediction - ((sample & 0x07) << step);
		else
			prediction = 0;
	} else {
		if (prediction < 0xff - diff)
			prediction = prediction + ((sample&0x07) << step);
		else
			prediction = 0xff;
	}


	if ((sample & 0x07) >= 5 && step < 3) {
		step ++;
	} else if ((sample & 0x07) == 0 && step > 0) {
		step --;
	}

	return prediction;
}

bool Sound::playSound(const Common::String &name, WaitType waitType, int priority) {
	stopSound();

	Common::String filename = name;
	if (!filename.contains('.'))
		filename += ".SND";
	
	Common::SeekableReadStream *stream = _vm->_res->load(filename);

	if (!stream)
		error("Unable to find sound file '%s'", filename.c_str());

	stream->skip(2);
	int size = stream->readUint32BE();
	int rate = stream->readUint16BE();
	byte *data = (byte *)malloc(size);
	byte *ptr = data;
	stream->read(ptr, size);

	byte *decoded = (byte *)malloc((size - 1) * 2);

	// +127 to eliminate the pop when the sound starts (signed vs unsigned PCM). Still does not help with the pop at the end
	byte prediction = (ptr[0] & 0x0f) + 127;
	int step = 0;
	int counter = 0;

	for(int i = 1; i < size; i++) {
		decoded[counter++] = decodeSample((ptr[i]>>4)&0x0f, prediction, step);
		decoded[counter++] = decodeSample((ptr[i]>>0)&0x0f, prediction, step);
	}

	free(data);

#if 0
	// Debug : used to dump files
	Common::DumpFile outFile;
	outFile.open(filename);
	outFile.write(decoded, (size - 2) * 2);
	outFile.flush();
	outFile.close();
#endif

	Audio::AudioStream *audioStream = Audio::makeRawStream(decoded, (size - 2) * 2, rate, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_effectsHandle, audioStream, -1,  Audio::Mixer::kMaxChannelVolume);
	_soundPlaying = true;
	_curPriority = priority;

	if (waitType == WAIT_RETURN_IMMEDIATELY) {
		_diskSoundPlaying = true;
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
	} while (!_vm->shouldQuit() && _mixer->isSoundHandleActive(_effectsHandle));
	
	_soundPlaying = false;
	_mixer->stopHandle(_effectsHandle);

	return retval;
}

void Sound::playLoadedSound(int bufNum, WaitType waitType) {
	if (_mixer->isSoundHandleActive(_effectsHandle) && (_curPriority > _vm->_scene->_sounds[bufNum]._priority))
		return;

	stopSound();
	playSound(_vm->_scene->_sounds[bufNum]._name, waitType, _vm->_scene->_sounds[bufNum]._priority);

	return;
}

void Sound::freeLoadedSounds() {
	// As sounds are played with DisposeAfterUse::YES, stopping the sounds also
	// frees them
	stopSound();
}

void Sound::stopSound() {
	_mixer->stopHandle(_effectsHandle);
}

void Sound::playMusic(const Common::String &name) {
	// TODO
	warning("Sound::playMusic %s", name.c_str());
	Common::SeekableReadStream *stream = _vm->_res->load(name, "MUSIC.LIB");

	byte *data = new byte[stream->size()];
	byte *ptr = data;
	stream->read(ptr, stream->size());
	Common::DumpFile outFile;
	outFile.open(name + ".RAW");
	outFile.write(data, stream->size());
	outFile.flush();
	outFile.close();
	delete[] data;

	stopMusic();
	startSong();
}

void Sound::stopMusic() {
	// TODO
	warning("TODO: Sound::stopMusic");
}

int Sound::loadSong(int songNumber) {
	// TODO
	warning("TODO: Sound::loadSong");
	return 0;
}

void Sound::startSong() {
	// TODO
	warning("TODO: Sound::startSong");
}

void Sound::freeSong() {
	// TODO
	warning("TODO: Sound::freeSong");
}

void Sound::stopSndFuncPtr(int v1, int v2) {
	// TODO
	warning("TODO: Sound::stopSndFuncPtr");
}

void Sound::waitTimerRoland(uint time) {
	// TODO
	warning("TODO: Sound::waitTimerRoland");
}

void Sound::freeDigiSound() {
	delete[] _digiBuf;
	_digiBuf = nullptr;
	_diskSoundPlaying = false;
	_soundPlaying = false;
}

} // End of namespace Sherlock
