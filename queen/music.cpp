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
#include "queen/sound.h"

#include "sound/midiparser.h"

namespace Queen {

// Instrument mapping for MT32 tracks emulated under GM.
static const byte mt32_to_gm[128] = {
//    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};

	MusicPlayer::MusicPlayer(MidiDriver *driver, byte *data, uint32 size) : _driver(driver), _isPlaying(false), _looping(false), _randomLoop(false), _volume(255), _queuePos(0), _musicData(data), _musicDataSize(size) {
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
		} else if ((b & 0xF0) == 0xC0 && !_nativeMT32) {
			b = (b & 0xFFFF00FF) | mt32_to_gm[(b >> 8) & 0xFF] << 8;
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
			while(Sound::_jungleList[i])
				queueSong(Sound::_jungleList[i++] - 1);
			return;
		}
		
		int mode = (_numSongs == 40) ? Sound::_tuneDemo[tuneList].mode : Sound::_tune[tuneList].mode;
		switch (mode) {
			//Random loop
			case  0:
				_randomLoop = true;
				setLoop(false);
				break;
			//Sequential loop
			case  1:
				setLoop(_songQueue[1] == 0);
				break;
			//Play once
			case  2:
			default:
				setLoop(false);
				break;
		}
		
		int i = 0;
		if (_numSongs == 40) {
			while(Sound::_tuneDemo[tuneList].tuneNum[i])
				queueSong(Sound::_tuneDemo[tuneList].tuneNum[i++] - 1);
		} else {
			while(Sound::_tune[tuneList].tuneNum[i])
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
				_buf[i] = data[*(idx + i)];

			musicPtr = ((byte *)_buf) + ((*musicPtr == 0x63) ? 1 : 0);
			size = packedSize * 2;
		}
		
		_parser->loadMusic(musicPtr, size);
		_parser->setTrack(0);	
		//debug(0, "Playing song %d [queue position: %d]", songNum, _queuePos);
		_isPlaying = true;
		queueUpdatePos();
	}
	
	void MusicPlayer::queueUpdatePos() {
		if (_randomLoop)
			_queuePos = randomQueuePos();
		else {
			if (_queuePos < (MUSIC_QUEUE_SIZE - 1) && _songQueue[_queuePos + 1])
				_queuePos++;
			else
				if (_looping)
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
	
	void Music::playSong(uint16 songNum) {
		_player->queueClear();
		_player->queueSong(songNum);
		_player->playMusic();				
	}
	
} // End of namespace Queen
