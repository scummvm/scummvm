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

#ifndef QUEEN_MUSIC_H
#define QUEEN_MUSIC_H

#include "common/util.h"
#include "common/mutex.h"
#include "sound/mididrv.h"

class MidiParser;

namespace Queen {

struct TuneData;

class QueenEngine;

class MidiMusic : public MidiDriver {
public:
	MidiMusic(QueenEngine *vm);
	~MidiMusic();
	void setVolume(int volume);
	int getVolume()	const { return _masterVolume; }

	void playSong(uint16 songNum);
	void stopSong() { stopMusic(); }
	void playMusic();
	void stopMusic();
	void setLoop(bool loop)		{ _looping = loop; }
	void queueTuneList(int16 tuneList);
	bool queueSong(uint16 songNum);
	void queueClear();
	void toggleVChange();

	//MidiDriver interface implementation
	int open() { return 0; }
	void close() {}
	void send(uint32 b);

	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo()	{ return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

protected:

	enum {
		MUSIC_QUEUE_SIZE	=	14
	};

	void queueUpdatePos();
	uint8 randomQueuePos();
	void onTimer();
	uint32 songOffset(uint16 songNum) const;
	uint32 songLength(uint16 songNum) const;

	static void timerCallback(void *refCon) { ((MidiMusic *)refCon)->onTimer(); }

	MidiDriver *_driver;
	MidiParser *_parser;
	MidiChannel *_channel[16];
	byte _channelVolume[16];
	bool _adlib;
	bool _nativeMT32;
	Common::Mutex _mutex;
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
	uint16 *_buf;
	uint32 _musicDataSize;
	bool _vToggle;
	byte *_musicData;
	const TuneData *_tune;
};

} // End of namespace Queen

#endif
