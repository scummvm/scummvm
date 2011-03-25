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

#ifndef SOUND_MIDIPLAYER_H
#define SOUND_MIDIPLAYER_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "audio/mididrv.h"

class MidiParser;

namespace Audio {

/**
 * Simple MIDI playback class.
 *
 * @note Currently incomplete, as it lacks play() methods. This is just a
 * start of the real thing, which tries to include code replicated between
 * several of our engines.
 *
 * Eventually, this should offer ways to start playback of SMF and XMIDI
 * data (and possibly make it easy to add further playback methods),
 * should be able to automatically instantiate _driver as needed,
 * should perform memory management on the MidiParser object(s) being
 * used, and possibly more.
 *
 * Also, care should be taken to ensure that mutex locking is done right;
 * currently, it isn't: That is, many engines have (and already had before
 * this class was introduced) various potential deadlocks hiding in their
 * MIDI code, caused by a thread trying to lock a mutex twice -- this may
 * work on some systems, but on others is a sure way towards a deadlock.
 *
 * @todo Document origin of this class. It is based on code shared by
 * several engines (e.g. DRACI says it copied it from MADE, which took
 * it from SAGE).
 */
class MidiPlayer : public MidiDriver_BASE {
public:
	MidiPlayer();
	~MidiPlayer();

	// TODO: Implement ways to actually play stuff
	//virtual void play(TODO);
	// TODO: Document these
	virtual void stop();
	virtual void pause();
	virtual void resume();

	bool isPlaying() const { return _isPlaying; }

	int getVolume() const { return _masterVolume; }
	virtual void setVolume(int volume);	// FIXME: Overloaded by Tinsel
	void syncVolume();

	bool hasNativeMT32() const { return _nativeMT32; }

	// MidiDriver_BASE interface
	virtual void send(uint32 b);
	virtual void metaEvent(byte type, byte *data, uint16 length);

	/**
	 * This method is invoked by send() after suitably filtering
	 * the message b.
	 */
	virtual void sendToChannel(byte ch, uint32 b);

	/**
	 * This method is invoked by metaEvent when an end-of-track
	 * event arrives. By default, this tells the parser
	 * to jump to the start (if looping is enabled) resp.
	 * invokes stope():
	 * Overload this to customize behavior.
	 */
	virtual void endOfTrack();

protected:

#if 0
	// TODO: Start to make use of these, once we figured
	// out the right way :)
	static void onTimer(void *data);
	virtual void onTimer();
#endif

	enum {
		kNumChannels = 16
	};

	Common::Mutex _mutex;
	MidiDriver *_driver;
	MidiParser *_parser;

	MidiChannel *_channelsTable[kNumChannels];
	uint8 _channelsVolume[kNumChannels];

	bool _isLooping;
	bool _isPlaying;
	int _masterVolume;	// FIXME: byte or int ?

	bool _nativeMT32;
};


} // End of namespace Audio

#endif
