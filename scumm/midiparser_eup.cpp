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

#include "stdafx.h"
#include "sound/midiparser.h"
#include "sound/mididrv.h"
#include "common/util.h"


namespace Scumm {

/**
 * The FM Towns Euphony version of MidiParser.
 */
class MidiParser_EUP : public MidiParser {
protected:
	byte _instruments[6][50]; // Two extra bytes for SysEx ID and channel #
	byte *_instr_to_channel;
	struct {
		byte *enable;
		int8 *channel;
		int8 *volume;
		int8 *transpose;
	} _presets;
	bool _loop;
	byte _presend;     // Tracks which startup implied events have been sent.
	uint32 _base_tick; // Events times are relative to this base.

protected:
	void parseNextEvent (EventInfo &info);
	void resetTracking();

public:
	bool loadMusic (byte *data, uint32 size);
};



//////////////////////////////////////////////////
//
// MidiParser_EUP implementation
//
//////////////////////////////////////////////////

void MidiParser_EUP::parseNextEvent (EventInfo &info) {
	byte *pos = _position._play_pos;

	// FIXME: The presend is for sending init events
	// that aren't actually in the stream. This would
	// be for, e.g., instrument setup. Right now, we
	// don't actually use the instruments specified
	// in the music header. We're sending fixed GM
	// program changes to get a reasonable "one-size-
	// fits-all" sound until we actually support the
	// FM synthesis capabilities of FM Towns.
	for (; _presend < 12; ++_presend) {
		if (_instr_to_channel[_presend>>1] >= 16)
			continue;
		info.start = pos;
		info.delta = 0;
		if (_presend & 1) {
			byte *data = &_instruments[_presend>>1][0];
			data[1] = _instr_to_channel[_presend>>1];
			info.event = 0xF0;
			info.ext.data = data;
			info.length = 48;
		} else {
			info.event = 0xB0 | (_presend >> 1);
			info.basic.param1 = 121;
			info.basic.param2 = 0;
		}
		++_presend;
		return;
	}

	while (true) {
		byte cmd = *pos;
		if ((cmd & 0xF0) == 0x90) {
			byte preset = pos[1];
			byte channel = _presets.channel[preset];
			if (channel >= 16)
				channel = cmd & 0x0F;
			uint16 tick = (pos[2] | ((uint16) pos[3] << 7)) + _base_tick;
			int note = (int) pos[4] + _presets.transpose[preset];
			int volume = (int) pos[5];
			// HACK: Loom-Towns distaff tracks seem to
			// contain zero-volume note events, so change
			// those to full volume.
			if (!volume)
				volume = 127;
			volume += _presets.volume[preset];
			if (volume > 127)
				volume = 127;
			else if (volume < 0)
				volume = 0;
			pos += 6;
			if (_presets.enable[preset]) {
				uint16 duration = pos[1] | (pos[2] << 4);
				info.start = pos;
				uint32 last = _position._last_event_tick;
				info.delta = (tick < last) ? 0 : (tick - last);
				info.event = 0x90 | channel;
				info.length = duration;
				info.basic.param1 = note;
				info.basic.param2 = volume;
				pos += 6;
				break;
			}
			pos += 6;
		} else if (cmd == 0xF2) {
			// This is a "measure marker" of sorts.
			// It advances the "base time", to which
			// all event times are relative.
			_base_tick += (pos[3] << 7) | pos[2];
			pos += 6;
		} else if (cmd == 0xF8) {
			// TODO: Implement this.
			pos += 6;
		} else if (cmd == 0xFD || cmd == 0xFE) {
			// End of track.
			if (_loop && false) {
				// TODO: Implement this.
			} else {
				info.start = pos;
				uint32 last = _position._last_event_tick;
				info.delta = (_base_tick < last) ? 0 : (_base_tick - last);
				info.event = 0xFF;
				info.length = 0;
				info.ext.type = 0x2F;
				info.ext.data = pos;
				break;
			}
		} else {
			printf ("Unknown Euphony music event 0x%02X\n", (int) cmd);
			memset (&info, 0, sizeof(info));
			pos = 0;
			break;
		}
	}
	_position._play_pos = pos;
}

bool MidiParser_EUP::loadMusic (byte *data, uint32 size) {
	unloadMusic();
	byte *pos = data;
	int i;

	if (memcmp (pos, "SO", 2)) {
		printf ("Warning: 'SO' header expected but found '%c%c' instead.\n", pos[0], pos[1]);
		return false;
	}

	byte numInstruments = pos[16];
	pos += 16 + 2;
	for (i = 0; i < numInstruments; ++i) {
		_instruments[i][0] = 0x7C;
		memcpy (&_instruments[i][2], pos, 48);
		pos += 48;
	}

	// Load the prest pointers
	_presets.enable = pos;
	pos += 32;
	_presets.channel = (int8 *) pos;
	pos += 32;
	_presets.volume = (int8 *) pos;
	pos += 32;
	_presets.transpose = (int8 *) pos;
	pos += 32;

	pos += 8; // Unknown bytes
	_instr_to_channel = pos; // Instrument-to-channel mapping
	pos += 6;
	pos += 4; // Skip the music size for now.
	pos++;    // Unknown byte
	byte tempo = *pos++;
	_loop = (*pos++ != 1);
	pos++;    // Unknown byte

	_num_tracks = 1;
	_ppqn = 120;
	_tracks[0] = pos;

	// Note that we assume the original data passed in
	// will persist beyond this call, i.e. we do NOT
	// copy the data to our own buffer. Take warning....
	resetTracking();
	setTempo (1000000 * 60 / tempo);
	setTrack (0);
	return true;
}

void MidiParser_EUP::resetTracking() {
	MidiParser::resetTracking();
	_presend = 0;
	_base_tick = 0;
}

MidiParser *MidiParser_createEUP() { return new MidiParser_EUP; }

} // End of namespace Scumm
