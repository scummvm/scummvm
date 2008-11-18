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

#ifndef GROOVIE_MUSIC_H
#define GROOVIE_MUSIC_H

#include "groovie/groovie.h"

#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/mutex.h"

namespace Groovie {

class MusicPlayer : public MidiDriver {
public:
	MusicPlayer(GroovieEngine *vm);
	~MusicPlayer();
	void playSong(uint16 fileref);
	void setBackgroundSong(uint16 fileref);

	// Volume
	void setUserVolume(uint16 volume);
	void setGameVolume(uint16 volume, uint16 time);
private:
	uint16 _userVolume;
	uint16 _gameVolume;
	byte _chanVolumes[0x10];
	void updateChanVolume(byte channel);

public:
	// MidiDriver interface
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length);
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc);
	uint32 getBaseTempo(void);
	MidiChannel *allocateChannel();
	MidiChannel *getPercussionChannel();

private:
	GroovieEngine *_vm;
	Common::Mutex _mutex;
	byte *_data;
	MidiParser *_midiParser;
	MidiDriver *_driver;

	uint16 _backgroundFileRef;

	static void onTimer(void *data);

	bool play(uint16 fileref, bool loop);
	bool load(uint16 fileref);
	void unload();
};

} // End of Groovie namespace

#endif // GROOVIE_MUSIC_H
