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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/m4/midi.h $
 * $Id: midi.h 47705 2010-01-30 09:21:07Z dreammaster $
 *
 */

// Music class

#ifndef PRISONER_MIDI_H
#define PRISONER_MIDI_H

#include "sound/mididrv.h"
#include "sound/midiparser.h"

#include "common/mutex.h"

#include "prisoner/resource.h"

namespace Prisoner {

class MidiPlayer : public MidiDriver {
public:
	MidiPlayer(PrisonerEngine *vm, MidiDriver *driver);
	~MidiPlayer();

	bool isPlaying() { return _isPlaying; }

	void setVolume(int volume);
	int getVolume() { return _masterVolume; }

	void setNativeMT32(bool b) { _nativeMT32 = b; }
	bool hasNativeMT32() { return _nativeMT32; }
	void playMusic(MidiResource *midiResource, int32 vol, bool loop);
	void stopMusic();
	void setPassThrough(bool b) { _passThrough = b; }

	void setGM(bool isGM) { _isGM = isGM; }

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);

	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo()	{ return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

protected:
	static void onTimer(void *data);

	PrisonerEngine *_vm;

	MidiChannel *_channel[16];
	MidiDriver *_driver;
	MidiParser *_parser;
	byte _channelVolume[16];
	bool _nativeMT32;
	bool _isGM;
	bool _passThrough;

	bool _isPlaying;
	bool _randomLoop;
	byte _masterVolume;

	Common::Mutex _mutex;

};

} // End of namespace Prisoner

#endif

