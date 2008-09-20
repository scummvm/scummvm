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

// Music class

#ifndef TINSEL_MUSIC_H
#define TINSEL_MUSIC_H

#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/mutex.h"

namespace Tinsel {

bool PlayMidiSequence(		// Plays the specified MIDI sequence through the sound driver
	uint32 dwFileOffset,		// handle of MIDI sequence data
	bool bLoop);			// Whether to loop the sequence

bool MidiPlaying(void);		// Returns TRUE if a Midi tune is currently playing

bool StopMidi(void);		// Stops any currently playing midi

void SetMidiVolume(		// Sets the volume of the MIDI music. Returns the old volume
	int vol);		// new volume - 0..MAXMIDIVOL

int GetMidiVolume();

void OpenMidiFiles();
void DeleteMidiBuffer();

void CurrentMidiFacts(SCNHANDLE	*pMidi, bool *pLoop);
void RestoreMidiFacts(SCNHANDLE	Midi, bool Loop);

int GetTrackNumber(SCNHANDLE hMidi);
SCNHANDLE GetTrackOffset(int trackNumber);

void dumpMusic();


class MusicPlayer : public MidiDriver {
public:
	MusicPlayer(MidiDriver *driver);
	~MusicPlayer();

	bool isPlaying() { return _isPlaying; }
	void setPlaying(bool playing) { _isPlaying = playing; }

	void setVolume(int volume);
	int getVolume() { return _masterVolume; }

	void playXMIDI(byte *midiData, uint32 size, bool loop);
	void stop();
	void pause();
	void resume();
	void setLoop(bool loop) { _looping = loop; }

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);

	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }

	// The original sets the "sequence timing" to 109 Hz, whatever that
	// means. The default is 120.

	uint32 getBaseTempo(void)	{ return _driver ? (109 * _driver->getBaseTempo()) / 120 : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	MidiParser *_parser;
	Common::Mutex _mutex;

protected:

	static void onTimer(void *data);

	MidiChannel *_channel[16];
	MidiDriver *_driver;
	MidiParser *_xmidiParser;
	byte _channelVolume[16];

	bool _isPlaying;
	bool _looping;
	byte _masterVolume;
};

} // End of namespace Made

#endif
