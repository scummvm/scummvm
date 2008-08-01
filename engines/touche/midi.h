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

#ifndef TOUCHE_MIDI_H
#define TOUCHE_MIDI_H

#include "common/util.h"
#include "common/mutex.h"

#include "sound/mididrv.h"

class MidiParser;

namespace Common {
	class ReadStream;
}

namespace Touche {

class MidiPlayer : public MidiDriver {
public:

	enum {
		NUM_CHANNELS = 16
	};

	MidiPlayer();
	~MidiPlayer();

	void play(Common::ReadStream &stream, int size, bool loop = false);
	void stop();
	void updateTimer();
	void adjustVolume(int diff);
	void setVolume(int volume);
	int getVolume() const { return _masterVolume; }
	void setLooping(bool loop) { _isLooping = loop; }

	// MidiDriver interface
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length);
	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo() { return _driver ? _driver->getBaseTempo() : 0; }
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

private:

	static void timerCallback(void *p);

	MidiDriver *_driver;
	MidiParser *_parser;
	uint8 *_midiData;
	bool _isLooping;
	bool _isPlaying;
	int _masterVolume;
	bool _nativeMT32;
	MidiChannel *_channelsTable[NUM_CHANNELS];
	uint8 _channelsVolume[NUM_CHANNELS];
	Common::Mutex _mutex;

	static const uint8 _gmToRol[];
};

} // namespace Touche

#endif
