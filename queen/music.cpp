/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "queen/music.h"
#include "queen/queen.h"
#include "queen/resource.h"

#include "sound/midiparser.h"

namespace Queen {

	MusicPlayer::MusicPlayer(MidiDriver *driver, byte *data, uint32 size) : _driver(driver), _isPlaying(false), _looping(false), _volume(255), _queuePos(0), _musicData(data), _musicDataSize(size) {
		queueClear();
		_lastSong = 0;
		_parser = MidiParser::createParser_SMF();
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		
		_numSongs = READ_LE_UINT16(_musicData);
		this->open();
	}
	
	MusicPlayer::~MusicPlayer() {
		_driver->setTimerCallback(NULL, NULL);
		_parser->unloadMusic();
		this->close();
		delete _parser;
	}
	
	bool MusicPlayer::queueSong(uint16 songNum) {
		uint8 emptySlots = 0;
		for (int i = 0; i < MUSIC_QUEUE_SIZE; i++)
			if (!_songQueue[i])
				emptySlots++;
		
		if (!emptySlots)
			return false;
			
		_songQueue[MUSIC_QUEUE_SIZE - emptySlots] = songNum;
		return true;
	}
	
	void MusicPlayer::queueClear() {
		_lastSong = _songQueue[0];
		_queuePos = 0;
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
		stopMusic();
		if (_driver)
			_driver->close();
		_driver = 0;
	}
	
	void MusicPlayer::send(uint32 b) {
		byte channel = (byte)(b & 0x0F);
		if ((b & 0xFFF0) == 0x07B0) {
			// Adjust volume changes by master volume
			byte volume = (byte)((b >> 16) & 0x7F);
			_channelVolume[channel] = volume;
			//volume = volume * _masterVolume / 255;
			b = (b & 0xFF00FFFF) | (volume << 16);
		} 
		else if ((b & 0xFFF0) == 0x007BB0) {
			//Only respond to All Notes Off if this channel
			//has currently been allocated
			if (_channel[b & 0x0F])
				return;
		}
		
		
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
	
	void MusicPlayer::playMusic() {
		if (!_queuePos && !_songQueue[_queuePos]) {
			debug(5, "MusicPlayer::playMusic - Music queue is empty!");
			return;
		}
		
		uint16 songNum = _songQueue[_queuePos];

		//Special type
		//2000: (songNum + 1) - repeat music from previous queue
		if (songNum == 1999) {
			songNum = _lastSong;
			queueClear();
			queueSong(songNum);
		}
		
		_parser->loadMusic(_musicData + songOffset(songNum), songLength(songNum));
		_parser->setTrack(0);	
		//debug(0, "Playing song %d [queue position: %d]", songNum, _queuePos);
		_isPlaying = true;
		queueUpdatePos();
	}
	
	void MusicPlayer::queueUpdatePos() {
		if (_queuePos < (MUSIC_QUEUE_SIZE - 1) && _songQueue[_queuePos + 1])
			_queuePos++;
		else
			_queuePos = 0;
	}
	
	void MusicPlayer::stopMusic() {
		_isPlaying = false;
		_parser->unloadMusic();
	}

	uint32 MusicPlayer::songOffset(uint16 songNum) {
		uint16 offsLo = READ_LE_UINT16(_musicData + (songNum * 4) + 2);
		uint16 offsHi = READ_LE_UINT16(_musicData + (songNum * 4) + 4);
		return (offsHi << 4) | offsLo;
	}

	uint32 MusicPlayer::songLength(uint16 songNum) {
		if (songNum < _numSongs)
			return (songOffset(songNum + 1) - songOffset(songNum));
		return (_musicDataSize - songOffset(songNum));
	}

	Music::Music(MidiDriver *driver, QueenEngine *vm) {
		if (vm->resource()->isDemo()) {
			_musicData = vm->resource()->loadFile("AQ8.RL", 0, NULL);
			_musicDataSize = vm->resource()->fileSize("AQ8.RL");
		} else {
			_musicData = vm->resource()->loadFile("AQ.RL", 0, NULL);
			_musicDataSize = vm->resource()->fileSize("AQ.RL");
		}
		
		_player = new MusicPlayer(driver, _musicData, _musicDataSize);
	}

	Music::~Music() {
		delete _player;
		delete[] _musicData;	
	}

	bool Music::queueSong(uint16 songNum) {
		// Work around bug in Roland music, note that these numbers are 'one-off'
		// from the original code
		if (/*isRoland && */ songNum == 88 || songNum == 89)
			songNum = 62;
			
		return _player->queueSong(songNum);
	}
	
	void Music::playSong(uint16 songNum) {
		_player->queueClear();
		_player->queueSong(songNum);
		_player->playMusic();				
	}

	void Music::stopSong() {
		return _player->stopMusic();
	}
	
} // End of namespace Queen
