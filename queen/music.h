/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef QUEENMUSIC_H
#define QUEENMUSIC_H

#include "common/util.h"
#include "sound/mididrv.h"

class MidiParser;

namespace Queen {

class QueenEngine;

class MusicPlayer : public MidiDriver {
public:
	MusicPlayer(MidiDriver *driver, byte *data, uint32 size);
	~MusicPlayer();
	void setVolume(int volume);
	int getVolume()			{ return _masterVolume; }

	void hasNativeMT32(bool b)	{ _nativeMT32 = b; }
	void playMusic();
	void stopMusic();
	void setLoop(bool loop)		{ _looping = loop; }
	void queueTuneList(int16 tuneList);
	bool queueSong(uint16 songNum);
	void queueClear();
	void setPassThrough(bool b)		{ _passThrough = b; }

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);

	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo(void)	{ return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

protected:

	enum {
		MUSIC_QUEUE_SIZE	=	14
	};

	void queueUpdatePos();
	uint8 randomQueuePos();
	static void onTimer(void *data);
	uint32 songOffset(uint16 songNum) const;
	uint32 songLength(uint16 songNum) const;

	MidiDriver *_driver;
	MidiParser *_parser;
	MidiChannel *_channel[16];
	byte _channelVolume[16];
	bool _nativeMT32;
	bool _passThrough;

	Common::RandomSource _rnd;

	bool _isPlaying;
	bool _looping;
	bool _randomLoop;
	byte _masterVolume;
	uint8 _queuePos;
	int16 _currentSong;
	int16 _lastSong;	//first song from previous queue
	int16 _songQueue[MUSIC_QUEUE_SIZE];

	uint16 _numSongs;
	byte *_musicData;
	uint16 *_buf;
	uint32 _musicDataSize;
};

class Music {
public:
	Music(MidiDriver *_driver, QueenEngine *vm);
	~Music();
	void hasNativeMT32(bool b)		{ _player->hasNativeMT32(b); }
	void playSong(uint16 songNum);
	void queueTuneList(int16 tuneList)	{ _player->queueTuneList(tuneList); }
	void playMusic()			{ _player->playMusic(); }
	void stopSong()				{ _player->stopMusic(); }
	void setPassThrough(bool b)		{ _player->setPassThrough(b); }

	void toggleVChange();
	void setVolume(int vol)			{ _player->setVolume(vol); }
	int volume()				{ return _player->getVolume(); }

protected:
	bool _vToggle;
	byte *_musicData;
	uint32 _musicDataSize;
	MusicPlayer *_player;
};

} // End of namespace Queen

#endif
