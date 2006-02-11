/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "queen/music.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"

#include "sound/midiparser.h"

namespace Queen {

MusicPlayer::MusicPlayer(MidiDriver *driver, byte *data, uint32 size) : _driver(driver), _isPlaying(false), _looping(false), _randomLoop(false), _masterVolume(192), _queuePos(0), _musicData(data), _musicDataSize(size), _passThrough(false), _buf(0) {
	memset(_channel, 0, sizeof(_channel));
	queueClear();
	_lastSong = _currentSong = 0;
	_parser = MidiParser::createParser_SMF();
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());

	_numSongs = READ_LE_UINT16(_musicData);
	this->open();
}

MusicPlayer::~MusicPlayer() {
	_parser->unloadMusic();
	delete _parser;
	this->close();
	delete[] _buf;
}

void MusicPlayer::setVolume(int volume) {
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

bool MusicPlayer::queueSong(uint16 songNum) {
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
	if (/*isRoland && */ songNum == 88 || songNum == 89)
		songNum = 62;

	_songQueue[MUSIC_QUEUE_SIZE - emptySlots] = songNum;
	return true;
}

void MusicPlayer::queueClear() {
	_lastSong = _songQueue[0];
	_queuePos = 0;
	_looping = _randomLoop = false;
	memset(_songQueue, 0, sizeof(_songQueue));
}

int MusicPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;
	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MusicPlayer::close() {
	_driver->setTimerCallback(NULL, NULL);
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MusicPlayer::send(uint32 b) {
	if (_passThrough) {
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
	}
	else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (_channel[b & 0x0F])
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

void MusicPlayer::metaEvent(byte type, byte *data, uint16 length) {
	//Only thing we care about is End of Track.
	if (type != 0x2F)
		return;

	if (_looping || _songQueue[1])
		playMusic();
	else
		stopMusic();
}

void MusicPlayer::onTimer(void *refCon) {
	MusicPlayer *music = (MusicPlayer *)refCon;
	if (music->_isPlaying)
		music->_parser->onTimer();
}

void MusicPlayer::queueTuneList(int16 tuneList) {
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

	int mode = (_numSongs == 40) ? Sound::_tuneDemo[tuneList].mode : Sound::_tune[tuneList].mode;
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
	if (_numSongs == 40) {
		while (Sound::_tuneDemo[tuneList].tuneNum[i])
			queueSong(Sound::_tuneDemo[tuneList].tuneNum[i++] - 1);
	} else {
		while (Sound::_tune[tuneList].tuneNum[i])
			queueSong(Sound::_tune[tuneList].tuneNum[i++] - 1);
	}

	if (_randomLoop)
		_queuePos = randomQueuePos();
}

void MusicPlayer::playMusic() {
	if (!_songQueue[0]) {
		debug(5, "MusicPlayer::playMusic - Music queue is empty!");
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
	if (*prevSong == 0x43 || *prevSong == 0x63) {
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
	if (*musicPtr == 0x43 || *musicPtr == 0x63) {
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

		musicPtr = ((byte *)_buf) + ((*musicPtr == 0x63) ? 1 : 0);
		size = packedSize * 2;
	}

	_parser->loadMusic(musicPtr, size);
	_parser->setTrack(0);
	debug(8, "Playing song %d [queue position: %d]", songNum, _queuePos);
	_isPlaying = true;
	queueUpdatePos();
}

void MusicPlayer::queueUpdatePos() {
	if (_randomLoop) {
		_queuePos = randomQueuePos();
	} else {
		if (_queuePos < (MUSIC_QUEUE_SIZE - 1) && _songQueue[_queuePos + 1])
			_queuePos++;
		else if (_looping)
			_queuePos = 0;
	}
}

uint8 MusicPlayer::randomQueuePos() {
	int queueSize = 0;
	for (int i = 0; i < MUSIC_QUEUE_SIZE; i++)
		if (_songQueue[i])
			queueSize++;

	if (!queueSize)
		return 0;

	return (uint8) _rnd.getRandomNumber(queueSize - 1) & 0xFF;
}

void MusicPlayer::stopMusic() {
	_isPlaying = false;
	_parser->unloadMusic();
}

uint32 MusicPlayer::songOffset(uint16 songNum) const {
	uint16 offsLo = READ_LE_UINT16(_musicData + (songNum * 4) + 2);
	uint16 offsHi = READ_LE_UINT16(_musicData + (songNum * 4) + 4);
	return (offsHi << 4) | offsLo;
}

uint32 MusicPlayer::songLength(uint16 songNum) const {
	if (songNum < _numSongs)
		return (songOffset(songNum + 1) - songOffset(songNum));
	return (_musicDataSize - songOffset(songNum));
}

Music::Music(MidiDriver *driver, QueenEngine *vm) : _vToggle(false) {
	if (vm->resource()->isDemo()) {
		_musicData = vm->resource()->loadFile("AQ8.RL", 0, &_musicDataSize);
	} else {
		_musicData = vm->resource()->loadFile("AQ.RL", 0, &_musicDataSize);
	}
	_player = new MusicPlayer(driver, _musicData, _musicDataSize);
}

Music::~Music() {
	delete _player;
	delete[] _musicData;
}

void Music::playSong(uint16 songNum) {
	_player->queueClear();
	_player->queueSong(songNum);
	_player->playMusic();
}

void Music::toggleVChange() {
	setVolume(_vToggle ? (volume() * 2) : (volume() / 2));
	_vToggle ^= true;
}

} // End of namespace Queen
