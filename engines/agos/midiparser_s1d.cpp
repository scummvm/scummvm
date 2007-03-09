/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/util.h"

#include "sound/mididrv.h"
#include "sound/midiparser.h"

namespace AGOS {

/**
 * Simon 1 Demo version of MidiParser.
 *
 * This parser is the result of eyeballing the one MUS file that's included
 * with simon1demo. So there might be some things missing. I've tried to notate
 * question-mark areas where they occur.
 */
class MidiParser_S1D : public MidiParser {
protected:
	byte *_data;
	bool _no_delta;

protected:
	void parseNextEvent(EventInfo &info);
	void resetTracking();
	uint32 readVLQ2(byte * &data);

public:
	MidiParser_S1D() : _data(0), _no_delta(false) {}

	bool loadMusic(byte *data, uint32 size);
};


// The VLQs for simon1demo seem to be
// in Little Endian format.
uint32 MidiParser_S1D::readVLQ2(byte * &data) {
	byte str;
	uint32 value = 0;
	int i;

	for (i = 0; i < 4; ++i) {
		str = data[0];
		++data;
		value |= (str & 0x7F) << (i * 7);
		if (!(str & 0x80))
			break;
	}
	return value;
}

void MidiParser_S1D::parseNextEvent(EventInfo &info) {
	info.start = _position._play_pos;
	info.delta = _no_delta ? 0 : readVLQ2(_position._play_pos);

	_no_delta = false;
	info.event = *(_position._play_pos++);
	if (info.command() < 0x8) {
		_no_delta = true;
		info.event += 0x80;
	}

	switch (info.command()) {
	case 0x8:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = 0;
		info.length = 0;
		break;

	case 0x9:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = *(_position._play_pos++); // I'm ASSUMING this byte is velocity!
		info.length = 0;
		break;

	case 0xA:
	case 0xB:
		// I'm not sure what these are meant to do, or what the
		// parameter is. Elvira 1 needs them, though, and who am I to
		// argue with her?
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = 0;
		break;

	case 0xC:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = 0;
		++_position._play_pos; // I have NO IDEA what the second byte is for.
		break;

	case 0xF:
		if (info.event == 0xFC) {
			// This means End of Track.
			// Rewrite in SMF (MIDI transmission) form.
			info.event = 0xFF;
			info.ext.type = 0x2F;
			info.length = 0;
			break;
		}

		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer?
			info.basic.param1 = *(_position._play_pos++);
			info.basic.param2 = *(_position._play_pos++);
			break;

		case 0x3: // Song Select?
			info.basic.param1 = *(_position._play_pos++);
			info.basic.param2 = 0;
			break;

		case 0x8:
			info.basic.param1 = info.basic.param2 = 0;
			break;

		case 0xF: // META event?
			info.ext.type = *(_position._play_pos++);
			info.length = readVLQ(_position._play_pos);
			info.ext.data = _position._play_pos;
			_position._play_pos += info.length;
			break;
		default:
			debug(10, "MidiParser_S1D: Unexpected type 0x%02X found", (int) info.event);
			break;
		}
		break;
	default:
		debug(10, "MidiParser_S1D: Unexpected event 0x%02X found", (int) info.command());
		break;
	}
}

bool MidiParser_S1D::loadMusic(byte *data, uint32 size) {
	unloadMusic();

	byte *pos = data;
	if (*(pos++) != 0xFC)
		debug(1, "Expected 0xFC header but found 0x%02X instead", (int) *pos);

	// The next 3 bytes MIGHT be tempo, but we skip them and use the default.
//	setTempo (*(pos++) | (*(pos++) << 8) | (*(pos++) << 16));
	pos += 3;

	// And now we're at the actual data. Only one track.
	_num_tracks = 1;
	_data = pos;
	_tracks[0] = pos;

	// Note that we assume the original data passed in
	// will persist beyond this call, i.e. we do NOT
	// copy the data to our own buffer. Take warning....
	resetTracking();
	setTempo(666667);
	setTrack(0);
	return true;
}

void MidiParser_S1D::resetTracking() {
	MidiParser::resetTracking();
	_no_delta = false;
}

MidiParser *MidiParser_createS1D() { return new MidiParser_S1D; }

} // End of namespace AGOS
