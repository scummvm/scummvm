/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NUVIE_SOUND_MIDIPARSER_M_H
#define NUVIE_SOUND_MIDIPARSER_M_H

#include "audio/midiparser.h"

#include "common/stack.h"
#include "common/util.h"

namespace Ultima {
namespace Nuvie {

/**
 * A parser for the music format M, used by Times Of Lore, Bad Blood and
 * Ultima 6.
 * This format is not really a MIDI format; it targets the OPL2 chip. However,
 * it has several things in common with MIDI: it is a stream of events, it has
 * note on and note off events and events similar to MIDI controllers, the high
 * nibble of the first event byte is the command while the low nibble is
 * usually the channel.
 * The commands are different. M does not use the status byte / data byte
 * convention and delta times are specified using a wait command. It uses
 * channels 0-8, corresponding to the 9 OPL2 channels. OPL rhythm mode is not
 * used.
 */
class MidiParser_M : public MidiParser {
protected:
	struct LoopData {
		byte numLoops;
		byte *startPos;
		byte *returnPos;
	};

public:
	MidiParser_M(int8 source = -1);
	~MidiParser_M();

	bool loadMusic(byte *data, uint32 size) override;
	void unloadMusic() override;
	void onTimer() override;

protected:
	bool processEvent(const EventInfo &info, bool fireEvents = true) override;
	void parseNextEvent(EventInfo &info) override;

	void allNotesOff() override;

	uint32 _trackLength;

	// The point in the MIDI data where the global loop (not using the stack)
	// has started and will return.
	byte *_loopPoint;

	// A stack of nested loops, similar to a call stack. A call command will
	// specify an offset where the parser should jump to (startPus), plus a
	// number of times the data from this offset up to the return command should
	// be repeated (numLoops). Then the parser resumes with the command after
	// the call command (returnPos). A maximum depth of 16 levels is supported.
	Common::FixedStack<LoopData, 16> *_loopStack;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
