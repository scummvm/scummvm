/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SIMON_MIDI_H
#define SIMON_MIDI_H

#include "sound/mididrv.h"
#include "sound/midiparser.h"

class File;
class OSystem;

class MidiPlayer : public MidiDriver {
protected:
	OSystem *_system;
	void *_mutex;
	MidiDriver *_driver;
	MidiParser *_parser;

	byte *_data;
	byte _volumeTable[16]; // 0-127
	byte _masterVolume;    // 0-255
	bool _paused;
	byte _currentTrack;

	byte _num_songs;
	byte *_songs[16];
	uint32 _song_sizes[16];

	static void onTimer (void *data);
	void clearConstructs();

public:
	bool _midi_sfx_toggle;

	 MidiPlayer (OSystem *system);
	~MidiPlayer();

	void playSMF (File *in);
	void playMultipleSMF (File *in);
	void playXMIDI (File *in);
	void jump (uint16 track, uint16 tick);
	void stop();
	void pause (bool b);

	int  get_volume() { return _masterVolume; }
	void set_volume (int volume);
	void set_driver (MidiDriver *md);

public:
	// MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);

	void metaEvent (byte type, byte *data, uint16 length);

	// Timing functions - MidiDriver now operates timers
	void setTimerCallback (void *timer_param, void (*timer_proc) (void *)) { }
	uint32 getBaseTempo (void) { return _driver ? _driver->getBaseTempo() : 0x4A0000; }

	// Channel allocation functions
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }
};

#endif
