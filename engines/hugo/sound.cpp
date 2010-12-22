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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

/* sound.c - sound effects and music support */

#include "common/system.h"
#include "common/config-manager.h"

#include "sound/decoders/raw.h"
#include "sound/audiostream.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/file.h"
#include "hugo/sound.h"

namespace Hugo {

MidiPlayer::MidiPlayer(MidiDriver *driver)
	: _driver(driver), _parser(0), _midiData(0), _isLooping(false), _isPlaying(false), _paused(false), _masterVolume(0) {
	assert(_driver);
	memset(_channelsTable, 0, sizeof(_channelsTable));
	for (int i = 0; i < NUM_CHANNELS; i++) {
		_channelsVolume[i] = 127;
	}
}

MidiPlayer::~MidiPlayer() {
	close();
}

void MidiPlayer::play(uint8 *stream, uint16 size) {
	debugC(3, kDebugMusic, "MidiPlayer::play");
	if (!stream) {
		stop();
		return;
	}

	_midiData = (uint8 *)malloc(size);
	if (_midiData) {
		memcpy(_midiData, stream, size);
		_mutex.lock();
		syncVolume();
		_parser->loadMusic(_midiData, size);
		_parser->setTrack(0);
		_isLooping = true;
		_isPlaying = true;
		_mutex.unlock();
	}
}

void MidiPlayer::stop() {
	debugC(3, kDebugMusic, "MidiPlayer::stop");
	_mutex.lock();
	if (_isPlaying) {
		_isPlaying = false;
		_parser->unloadMusic();
		free(_midiData);
		_midiData = 0;
	}
	_mutex.unlock();
}

void MidiPlayer::pause(bool p) {
	_paused = p;

	for (int i = 0; i < NUM_CHANNELS; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_paused ? 0 : _channelsVolume[i] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::updateTimer() {
	if (_paused) {
		return;
	}

	_mutex.lock();
	if (_isPlaying) {
		_parser->onTimer();
	}
	_mutex.unlock();
}

void MidiPlayer::adjustVolume(int diff) {
	debugC(3, kDebugMusic, "MidiPlayer::adjustVolume");
	setVolume(_masterVolume + diff);
}

void MidiPlayer::syncVolume() {
	int volume = ConfMan.getInt("music_volume");
	if (ConfMan.getBool("mute")) {
		volume = -1;
	}
	debugC(2, kDebugMusic, "Syncing music volume to %d", volume);
	setVolume(volume);
}

void MidiPlayer::setVolume(int volume) {
	debugC(3, kDebugMusic, "MidiPlayer::setVolume");
	_masterVolume = CLIP(volume, 0, 255);
	_mutex.lock();
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_channelsVolume[i] * _masterVolume / 255);
		}
	}
	_mutex.unlock();
}

void MidiPlayer::setChannelVolume(int channel) {
	int newVolume = _channelsVolume[channel] * _masterVolume / 255;
	debugC(3, kDebugMusic, "Music channel %d: volume %d->%d",
		channel, _channelsVolume[channel], newVolume);
	_channelsTable[channel]->volume(newVolume);
}

int MidiPlayer::open() {
	if (!_driver)
		return 255;
	int ret = _driver->open();
	if (ret)
		return ret;

    _driver->sendGMReset();

	_parser = MidiParser::createParser_SMF();
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_driver->setTimerCallback(this, &timerCallback);

	return 0;
}

void MidiPlayer::close() {
	stop();
	_mutex.lock();
	_driver->setTimerCallback(0, 0);
	_driver->close();
	delete _driver;
	_driver = 0;
	if (_parser)
		_parser->setMidiDriver(0);
	delete _parser;
	_mutex.unlock();
}

void MidiPlayer::send(uint32 b) {
	byte volume, ch = (byte)(b & 0xF);
	debugC(9, kDebugMusic, "MidiPlayer::send, channel %d (volume is %d)", ch, _channelsVolume[ch]);
	switch (b & 0xFFF0) {
	case 0x07B0:                                    // volume change
		volume = (byte)((b >> 16) & 0x7F);
		_channelsVolume[ch] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
        debugC(8, kDebugMusic, "Volume change, channel %d volume %d", ch, volume);
		break;
	case 0x7BB0:                                    // all notes off
        debugC(8, kDebugMusic, "All notes off, channel %d", ch);
		if (!_channelsTable[ch]) {                  // channel not yet allocated, no need to send the event
			return;
		}
		break;
	}

	if (!_channelsTable[ch]) {
		_channelsTable[ch] = (ch == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		if (_channelsTable[ch])
			setChannelVolume(ch);
	}
	if (_channelsTable[ch]) {
		_channelsTable[ch]->send(b);
	}
}

void MidiPlayer::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F:                                      // end of Track
		if (_isLooping) {
			_parser->jumpToTick(0);
		} else {
			stop();
		}
		break;
	default:
//		warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MidiPlayer::timerCallback(void *p) {
	MidiPlayer *player = (MidiPlayer *)p;

	player->updateTimer();
}

SoundHandler::SoundHandler(HugoEngine *vm) : _vm(vm) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	MidiDriver *driver = MidiDriver::createMidi(dev);

	_midiPlayer = new MidiPlayer(driver);
}

SoundHandler::~SoundHandler() {
	delete _midiPlayer;
}

/**
* Set the FM music volume from config.mvolume (0..100%)
*/
void SoundHandler::setMusicVolume() {
	_midiPlayer->setVolume(_config.musicVolume * 255 / 100);
}

/**
* Stop any sound that might be playing
*/
void SoundHandler::stopSound() {
	_vm->_mixer->stopAll();
}

/**
* Stop any tune that might be playing
*/
void SoundHandler::stopMusic() {
	_midiPlayer->stop();
}

/**
* Turn music on and off
*/
void SoundHandler::toggleMusic() {
	_config.musicFl = !_config.musicFl;

	_midiPlayer->pause(!_config.musicFl);
}

/**
* Turn digitized sound on and off
*/
void SoundHandler::toggleSound() {
	_config.soundFl = !_config.soundFl;
}

void SoundHandler::playMIDI(sound_pt seq_p, uint16 size) {
	_midiPlayer->play(seq_p, size);
}

/**
* Read a tune sequence from the sound database and start playing it
*/
void SoundHandler::playMusic(int16 tune) {
	sound_pt seqPtr;                                // Sequence data from file
	uint16 size;                                    // Size of sequence data

	if (_config.musicFl) {
		_vm->getGameStatus().song = tune;
		seqPtr = _vm->_file->getSound(tune, &size);
		playMIDI(seqPtr, size);
		free(seqPtr);
	}
}

/**
* Produce various sound effects on supplied stereo channel(s)
* Override currently playing sound only if lower or same priority
*/
void SoundHandler::playSound(int16 sound, stereo_t channel, byte priority) {
	// uint32 dwVolume;                             // Left, right volume of sound
	sound_pt sound_p;                               // Sound data
	uint16 size;                                    // Size of data
	static byte curPriority = 0;                    // Priority of currently playing sound
	//
	/* Sound disabled */
	if (!_config.soundFl || !_vm->_mixer->isReady())
		return;
	//
	// // See if last wave still playing - if so, check priority
	// if (waveOutUnprepareHeader(hwav, lphdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
	//  if (priority < curPriority)                 // Don't override unless priority >= current
	//      return;
	//  else
	//      Stop_sound();
	curPriority = priority;
	//
	/* Get sound data */
	if ((sound_p = _vm->_file->getSound(sound, &size)) == 0)
		return;

	Audio::AudioStream *stream = Audio::makeRawStream(sound_p, size, 11025, Audio::FLAG_UNSIGNED);
	_vm->_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, stream);

}

/**
* Initialize for MCI sound and midi
*/
void SoundHandler::initSound() {
	_midiPlayer->open();
}

} // End of namespace Hugo
