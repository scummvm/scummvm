/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "sound/midiparser.h"
#include "sound/mididrv.h"
#include "common/util.h"

namespace Scumm {

//////////////////////////////////////////////////
//
// The Standard MIDI File version of MidiParser
//
//////////////////////////////////////////////////

class MidiParser_RO : public MidiParser {
protected:
	int _markerCount;     // Number of markers encountered in stream so far
	int _lastMarkerCount; // Cache markers until parsed event is actually consumed

protected:
	void compressToType0();
	void parseNextEvent (EventInfo &info);

public:
	bool loadMusic (byte *data, uint32 size);
	uint32 getTick() { return (uint32) _markerCount * _ppqn / 2; }
};



//////////////////////////////////////////////////
//
// MidiParser_RO implementation
//
//////////////////////////////////////////////////

void MidiParser_RO::parseNextEvent (EventInfo &info) {
	_markerCount += _lastMarkerCount;
	_lastMarkerCount = 0;

	info.delta = 0;
	do {
		info.start = _position._play_pos;
		info.event = *(_position._play_pos++);
		if (info.command() == 0xA) {
			++_lastMarkerCount;
			info.event = 0xF0;
		} else if (info.event == 0xF0) {
			byte delay = *(_position._play_pos++);
			info.delta += delay;
			continue;
		}
		break;
	} while (true);

	// Seems to indicate EOT
	if (info.event == 0) {
		info.event = 0xFF;
		info.ext.type = 0x2F;
		info.length = 0;
		info.ext.data = 0;
		return;
	}

	if (info.event < 0x80)
		return;

	_position._running_status = info.event;
	switch (info.command()) {
	case 0xC:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = 0;
		break;

	case 0x8: case 0x9: case 0xB:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = *(_position._play_pos++);
		if (info.command() == 0x9 && info.basic.param2 == 0)
			info.event = info.channel() | 0x80;
		info.length = 0;
		break;

	case 0xF: // Marker and EOT messages
		info.length = 0;
		info.ext.data = 0;
		if (info.event == 0xFF) {
			_autoLoop = true;
			info.ext.type = 0x2F;
		} else {
			info.ext.type = 0x7F; // Bogus META
		}
		info.event = 0xFF;
		break;
	}
}

bool MidiParser_RO::loadMusic (byte *data, uint32 size) {
	unloadMusic();
	byte *pos = data;

	if (memcmp (pos, "RO", 2)) {
		error("'RO' header expected but found '%c%c' instead", pos[0], pos[1]);
		return false;
	}

	_num_tracks = 1;
	_ppqn = 120;
	_tracks[0] = pos + 2;
	_markerCount = _lastMarkerCount = 0;

	// Note that we assume the original data passed in
	// will persist beyond this call, i.e. we do NOT
	// copy the data to our own buffer. Take warning....
	resetTracking();
	setTempo (500000);
	setTrack (0);
	return true;
}

MidiParser *MidiParser_createRO() { return new MidiParser_RO; }

} // End of namespace Scumm
