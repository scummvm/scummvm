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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "audio/mididrv.h"
#include "audio/midiparser.h"

namespace AGOS {

/**
 * Simon 1 Demo version of MidiParser.
 */
class MidiParser_S1D : public MidiParser {
private:
	byte *_data;
	bool _noDelta;

	struct Loop {
		uint16 timer;
		byte *start, *end;
	} _loops[16];

	uint32 readVLQ2(byte *&data);
	void chainEvent(EventInfo &info);
protected:
	void parseNextEvent(EventInfo &info) override;
	void resetTracking() override;

public:
	MidiParser_S1D() : _data(0), _noDelta(false) {}

	bool loadMusic(byte *data, uint32 size) override;
};

uint32 MidiParser_S1D::readVLQ2(byte *&data) {
	uint32 delta = 0;

	// LE format VLQ, which is 2 bytes long at max.
	delta = *data++;
	if (delta & 0x80) {
		delta &= 0x7F;
		delta |= *data++ << 7;
	}

	return delta;
}

void MidiParser_S1D::chainEvent(EventInfo &info) {
	// When we chain an event, we add up the old delta.
	uint32 delta = info.delta;
	parseNextEvent(info);
	info.delta += delta;
}

void MidiParser_S1D::parseNextEvent(EventInfo &info) {
	info.start = _position._playPos;
	info.length = 0;
	info.delta = _noDelta ? 0 : readVLQ2(_position._playPos);
	_noDelta = false;

	info.event = *_position._playPos++;
	if (!(info.event & 0x80)) {
		_noDelta = true;
		info.event |= 0x80;
	}

	if (info.event == 0xFC) {
		// This means End of Track.
		// Rewrite in SMF (MIDI transmission) form.
		info.event = 0xFF;
		info.ext.type = 0x2F;
	} else {
		switch (info.command()) {
		case 0x8: // note off
			info.basic.param1 = *_position._playPos++;
			info.basic.param2 = 0;
			break;

		case 0x9: // note on
			info.basic.param1 = *_position._playPos++;
			info.basic.param2 = *_position._playPos++;
			// Rewrite note on events with velocity 0 as note off events.
			// This is the actual meaning of this, but theoretically this
			// should not need to be rewritten, since all MIDI devices should
			// interpret it like that. On the other hand all our MidiParser
			// implementations do it and there seems to be code in MidiParser
			// which relies on this for tracking active notes.
			if (info.basic.param2 == 0) {
				info.event = info.channel() | 0x80;
			}
			break;

		case 0xA: { // loop control
			// In case the stop mode(?) is set to 0x80 this will stop the
			// track over here.

			const int16 loopIterations = int8(*_position._playPos++);
			if (!loopIterations) {
				_loops[info.channel()].start = _position._playPos;
			} else {
				if (!_loops[info.channel()].timer) {
					if (_loops[info.channel()].start) {
						_loops[info.channel()].timer = uint16(loopIterations);
						_loops[info.channel()].end = _position._playPos;

						// Go to the start of the loop
						_position._playPos = _loops[info.channel()].start;
					}
				} else {
					if (_loops[info.channel()].timer)
						_position._playPos = _loops[info.channel()].start;
					--_loops[info.channel()].timer;
				}
			}

			// We need to read the next midi event here. Since we can not
			// safely pass this event to the MIDI event processing.
			chainEvent(info);
			} break;

		case 0xB: // auto stop marker(?)
			// In case the stop mode(?) is set to 0x80 this will stop the
			// track.

			// We need to read the next midi event here. Since we can not
			// safely pass this event to the MIDI event processing.
			chainEvent(info);
			break;

		case 0xC: // program change
			info.basic.param1 = *_position._playPos++;
			info.basic.param2 = 0;
			break;

		case 0xD: // jump to loop end
			if (_loops[info.channel()].end)
				_position._playPos = _loops[info.channel()].end;

			// We need to read the next midi event here. Since we can not
			// safely pass this event to the MIDI event processing.
			chainEvent(info);
			break;

		default:
			// The original called some other function from here, which seems
			// not to be MIDI related.
			warning("MidiParser_S1D: default case %d", info.channel());

			// We need to read the next midi event here. Since we can not
			// safely pass this event to the MIDI event processing.
			chainEvent(info);
			break;
		}
	}
}

bool MidiParser_S1D::loadMusic(byte *data, uint32 size) {
	unloadMusic();

	if (!size)
		return false;

	// The original actually just ignores the first two bytes.
	byte *pos = data;
	if (*pos == 0xFC) {
		// SysEx found right at the start
		// this seems to happen since Elvira 2, we ignore it
		// 3rd byte after the SysEx seems to be saved into a global

		// We expect at least 4 bytes in total
		if (size < 4)
			return false;

		byte skipOffset = pos[2]; // get second byte after the SysEx
		// pos[1] seems to have been ignored
		// pos[3] is saved into a global inside the original interpreters

		// Waxworks + Simon 1 demo typical header is:
		//  0xFC 0x29 0x07 0x01 [0x00/0x01]
		// Elvira 2 typical header is:
		//  0xFC 0x04 0x06 0x06

		if (skipOffset >= 6) {
			// should be at least 6, so that we skip over the 2 size bytes and the
			// smallest SysEx possible
			skipOffset -= 2; // 2 size bytes were already read by previous code outside of this method

			if (size <= skipOffset) // Skip to the end of file? -> something is not correct
				return false;

			// Do skip over the bytes
			pos += skipOffset;
		} else {
			warning("MidiParser_S1D: unexpected skip offset in music file");
		}
	}

	// And now we're at the actual data. Only one track.
	_numTracks = 1;
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
	// The first event never contains any delta.
	_noDelta = true;
	memset(_loops, 0, sizeof(_loops));
}

MidiParser *MidiParser_createS1D() { return new MidiParser_S1D; }

} // End of namespace AGOS
