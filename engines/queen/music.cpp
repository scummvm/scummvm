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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/EventRecorder.h"

#include "queen/music.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"

#include "sound/midiparser.h"

namespace Queen {

extern MidiDriver *C_Player_CreateAdlibMidiDriver(Audio::Mixer *);

MidiMusic::MidiMusic(QueenEngine *vm)
	: _isPlaying(false), _looping(false), _randomLoop(false), _masterVolume(192), _buf(0) {

	memset(_channel, 0, sizeof(_channel));
	_queuePos = _lastSong = _currentSong = 0;
	queueClear();

	MidiDriverType midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	_adlib = (midiDriver == MD_ADLIB);
	_nativeMT32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));

	const char *musicDataFile;
	if (vm->resource()->isDemo()) {
		_tune = Sound::_tuneDemo;
		musicDataFile = "AQ8.RL";
	} else {
		_tune = Sound::_tune;
		musicDataFile = "AQ.RL";
	}
	if (_adlib) {
		musicDataFile = "AQBANK.MUS";
	}
	_musicData = vm->resource()->loadFile(musicDataFile, 0, &_musicDataSize);
	_numSongs = READ_LE_UINT16(_musicData);

	_tune = vm->resource()->isDemo() ? Sound::_tuneDemo : Sound::_tune;

	if (_adlib) {
//		int infoOffset = _numSongs * 4 + 2;
//		if (READ_LE_UINT16(_musicData + 2) != infoOffset) {
//			defaultAdlibVolume = _musicData[infoOffset];
//		}
		_driver = C_Player_CreateAdlibMidiDriver(vm->_mixer);
	} else {
		_driver = MidiDriver::createMidi(midiDriver);
		if (_nativeMT32) {
			_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
		}
	}

	_driver->open();
	_driver->setTimerCallback(this, &timerCallback);

	_parser = MidiParser::createParser_SMF();
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());

	g_eventRec.registerRandomSource(_rnd, "queenMusic");
}

MidiMusic::~MidiMusic() {
	_driver->setTimerCallback(0, 0);
	_parser->unloadMusic();
	_driver->close();
	delete _parser;
	delete _driver;
	delete[] _buf;
	delete[] _musicData;
}

void MidiMusic::setVolume(int volume) {
	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	for (int i = 0; i < 16; ++i) {
		if (_channel[i])
			_channel[i]->volume(_channelVolume[i] * _masterVolume / 255);
	}
}

void MidiMusic::playSong(uint16 songNum) {
	queueClear();
	queueSong(songNum);
	playMusic();
}

bool MidiMusic::queueSong(uint16 songNum) {
	if (songNum >= _numSongs && songNum < 1000) {
		// this happens at the end of the car chase, where we try to play song 176,
		// see Sound::_tune[], entry 39
		debug(3, "Trying to queue an invalid song number %d, max %d", songNum, _numSongs);
		return false;
	}
	uint8 emptySlots = 0;
	for (int i = 0; i < MUSIC_QUEUE_SIZE; i++)
		if (!_songQueue[i])
			emptySlots++;

	if (!emptySlots)
		return false;

	// Work around bug in Roland music, note that these numbers are 'one-off'
	// from the original code
	if (!_adlib && (songNum == 88 || songNum == 89))
		songNum = 62;

	_songQueue[MUSIC_QUEUE_SIZE - emptySlots] = songNum;
	return true;
}

void MidiMusic::queueClear() {
	_lastSong = _songQueue[0];
	_queuePos = 0;
	_looping = _randomLoop = false;
	memset(_songQueue, 0, sizeof(_songQueue));
}

void MidiMusic::send(uint32 b) {
	if (_adlib) {
		_driver->send(b);
		return;
	}

	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	} else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (_channel[channel])
			return;
	}

	//Work around annoying loud notes in certain Roland Floda tunes
	if (channel == 3 && _currentSong == 90)
		return;
	if (channel == 4 && _currentSong == 27)
		return;
	if (channel == 5 && _currentSong == 38)
		return;

	if (!_channel[channel])
		_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();

	if (_channel[channel])
		_channel[channel]->send(b);
}

void MidiMusic::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F: // End of Track
		if (_looping || _songQueue[1]) {
			playMusic();
		} else {
			stopMusic();
		}
		break;
	case 0x7F: // Specific
		if (_adlib) {
			_driver->metaEvent(type, data, length);
		}
		break;
	default:
//		warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MidiMusic::onTimer() {
	_mutex.lock();
	if (_isPlaying)
		_parser->onTimer();
	_mutex.unlock();
}

void MidiMusic::queueTuneList(int16 tuneList) {
	queueClear();

	//Jungle is the only part of the game that uses multiple tunelists.
	//For the sake of code simplification we just hardcode the extended list ourselves
	if ((tuneList + 1) == 3) {
		_randomLoop = true;
		int i = 0;
		while (Sound::_jungleList[i])
			queueSong(Sound::_jungleList[i++] - 1);
		return;
	}

	int mode = _tune[tuneList].mode;
	switch (mode) {
	case 0: // random loop
		_randomLoop = true;
		setLoop(false);
		break;
	case 1: // sequential loop
		setLoop(_songQueue[1] == 0);
		break;
	case 2: // play once
	default:
		setLoop(false);
		break;
	}

	int i = 0;
	while (_tune[tuneList].tuneNum[i])
		queueSong(_tune[tuneList].tuneNum[i++] - 1);

	if (_randomLoop)
		_queuePos = randomQueuePos();
}

void MidiMusic::playMusic() {
	if (!_songQueue[0]) {
		debug(5, "MidiMusic::playMusic - Music queue is empty");
		return;
	}

	uint16 songNum = _songQueue[_queuePos];

	//Special type
	// > 1000 && < 2000 -> queue different tunelist
	// 2000 -> repeat music from previous queue
	if (songNum > 999) {
		if ((songNum + 1) == 2000) {
			songNum = _lastSong;
			queueClear();
			queueSong(songNum);
		} else {
			queueTuneList(songNum - 1000);
			_queuePos = _randomLoop ? randomQueuePos() : 0;
			songNum = _songQueue[_queuePos];
		}
	}

	byte *prevSong = _musicData + songOffset(_currentSong);
	if (*prevSong == 'C' || *prevSong == 'c') {
		if (_buf) {
			delete[] _buf;
			_buf = 0;
		}
	}

	_currentSong = songNum;
	if (!songNum) {
		stopMusic();
		return;
	}

	byte *musicPtr = _musicData + songOffset(songNum);
	uint32 size = songLength(songNum);
	if (*musicPtr == 'C' || *musicPtr == 'c') {
		uint32 packedSize = songLength(songNum) - 0x200;
		_buf = new uint16[packedSize];

		uint16 *data = (uint16 *)(musicPtr + 1);
		byte *idx  = ((byte *)data) + 0x200;

		for (uint i = 0; i < packedSize; i++)
#if defined(SCUMM_NEED_ALIGNMENT)
			memcpy(&_buf[i], (byte*)((byte*)data + *(idx + i) * sizeof(uint16)), sizeof(uint16));
#else
			_buf[i] = data[*(idx + i)];
#endif

		musicPtr = ((byte *)_buf) + ((*musicPtr == 'c') ? 1 : 0);
		size = packedSize * 2;
	}

	_mutex.lock();
	_parser->loadMusic(musicPtr, size);
	_parser->setTrack(0);
	_isPlaying = true;
	_mutex.unlock();

	debug(8, "Playing song %d [queue position: %d]", songNum, _queuePos);
	queueUpdatePos();
}

void MidiMusic::queueUpdatePos() {
	if (_randomLoop) {
		_queuePos = randomQueuePos();
	} else {
		if (_queuePos < (MUSIC_QUEUE_SIZE - 1) && _songQueue[_queuePos + 1])
			_queuePos++;
		else if (_looping)
			_queuePos = 0;
	}
}

uint8 MidiMusic::randomQueuePos() {
	int queueSize = 0;
	for (int i = 0; i < MUSIC_QUEUE_SIZE; i++)
		if (_songQueue[i])
			queueSize++;

	if (!queueSize)
		return 0;

	return (uint8) _rnd.getRandomNumber(queueSize - 1) & 0xFF;
}

void MidiMusic::stopMusic() {
	_mutex.lock();
	_isPlaying = false;
	_parser->unloadMusic();
	_mutex.unlock();
}

uint32 MidiMusic::songOffset(uint16 songNum) const {
	uint16 offsLo = READ_LE_UINT16(_musicData + (songNum * 4) + 2);
	uint16 offsHi = READ_LE_UINT16(_musicData + (songNum * 4) + 4);
	return (offsHi << 4) | offsLo;
}

uint32 MidiMusic::songLength(uint16 songNum) const {
	if (songNum < _numSongs)
		return (songOffset(songNum + 1) - songOffset(songNum));
	return (_musicDataSize - songOffset(songNum));
}

void MidiMusic::toggleVChange() {
	setVolume(_vToggle ? (getVolume() * 2) : (getVolume() / 2));
	_vToggle = !_vToggle;
}

} // End of namespace Queen
