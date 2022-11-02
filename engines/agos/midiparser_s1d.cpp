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
		bool noDelta;
	} _loops[16];

	// Data for monophonic chords mode.
	// If this is activated, when multiple notes are played at the same time on
	// a melodic channel (0-5), only the highest note will be played.
	// This functionality is used by Elvira 2 (although there are no chords in
	// the MIDI data), Waxworks and the Simon 1 floppy demo.

	// The highest note played at _lastPlayedNoteTime for each channel.
	byte _highestNote[6];
	// The timestamp at which the last note was played at each channel.
	uint32 _lastPlayedNoteTime[6];

	// True if, for notes played at the same time, only the highest note should
	// be played. If false, all notes of a chord will be sent to the driver.
	bool _monophonicChords;

	uint32 readVLQ2(byte *&data);
protected:
	void parseNextEvent(EventInfo &info) override;
	bool processEvent(const EventInfo &info, bool fireEvents = true) override;
	void resetTracking() override;

	public:
	MidiParser_S1D(uint8 source = 0, bool monophonicChords = false) : MidiParser(source),
			_monophonicChords(monophonicChords), _data(nullptr), _noDelta(false) {
		Common::fill(_loops, _loops + ARRAYSIZE(_loops), Loop { 0, 0, 0, false });
		Common::fill(_highestNote, _highestNote + ARRAYSIZE(_highestNote), 0);
		Common::fill(_lastPlayedNoteTime, _lastPlayedNoteTime + ARRAYSIZE(_lastPlayedNoteTime), 0);
	}

	bool loadMusic(byte *data, uint32 size) override;
	int32 determineDataSize(Common::SeekableReadStream *stream) override;
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

void MidiParser_S1D::parseNextEvent(EventInfo &info) {
	info.start = _position._playPos;
	info.length = 0;
	info.delta = _noDelta ? 0 : readVLQ2(_position._playPos);
	info.noop = false;
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
				_loops[info.channel()].noDelta = _noDelta;
			} else {
				if (!_loops[info.channel()].timer) {
					if (_loops[info.channel()].start) {
						_loops[info.channel()].timer = uint16(loopIterations);
						_loops[info.channel()].end = _position._playPos;

						// Go to the start of the loop
						_position._playPos = _loops[info.channel()].start;
						_noDelta = _loops[info.channel()].noDelta;
						info.loop = true;
					}
				} else {
					if (_loops[info.channel()].timer) {
						_position._playPos = _loops[info.channel()].start;
						_noDelta = _loops[info.channel()].noDelta;
						info.loop = true;
					}
					--_loops[info.channel()].timer;
				}
			}
			// Event has been fully processed here.
			info.noop = true;
			} break;

		case 0xB: // auto stop marker(?)
			// In case the stop mode(?) is set to 0x80 this will stop the
			// track.

			// Event has been fully processed here.
			info.noop = true;
			break;

		case 0xC: // program change
			info.basic.param1 = *_position._playPos++;
			info.basic.param2 = 0;
			break;

		case 0xD: // jump to loop end
			if (_loops[info.channel()].end)
				_position._playPos = _loops[info.channel()].end;

			// Event has been fully processed here.
			info.noop = true;
			break;

		default:
			// The original called some other function from here, which seems
			// not to be MIDI related.
			warning("MidiParser_S1D: default case %d", info.channel());

			// Event has been fully processed here.
			info.noop = true;
			break;
		}
	}
}

bool MidiParser_S1D::processEvent(const EventInfo &info, bool fireEvents) {
	byte channel = info.channel();
	if (_monophonicChords && channel < 6 && info.command() == 0x9 && info.basic.param2 > 0) {
		// In monophonic chords mode, when multiple notes are played at the
		// same time on a melodic channel (0-5), only the highest note should
		// be played.
		if (_lastPlayedNoteTime[channel] == _position._playTick && _highestNote[channel] > info.basic.param1) {
			// This note is lower than a previously played note on the same
			// channel and with the same timestamp. Ignore it.
			return true;
		} else {
			// This note either has a different timestamp (i.e. it is not
			// played at the same time), or it is higher than the previously
			// played note.
			// Update the timestamp and note registry and play this note
			// (because the channel is monophonic, a previously played lower
			// note will be cut off).
			_lastPlayedNoteTime[channel] = _position._playTick;
			_highestNote[channel] = info.basic.param1;
		}
	}

	return MidiParser::processEvent(info, fireEvents);
}

bool MidiParser_S1D::loadMusic(byte *data, uint32 size) {
	unloadMusic();

	if (!size)
		return false;

	// The original actually just ignores the first two bytes.
	byte *pos = data + 2;
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

int32 MidiParser_S1D::determineDataSize(Common::SeekableReadStream* stream) {
	// Data size is stored in the first two bytes.
	return stream->readUint16LE() + 2;
}

void MidiParser_S1D::resetTracking() {
	MidiParser::resetTracking();
	// The first event never contains any delta.
	_noDelta = true;
	Common::fill(_loops, _loops + ARRAYSIZE(_loops), Loop { 0, 0, 0, false });
}

MidiParser *MidiParser_createS1D(uint8 source, bool monophonicChords) { return new MidiParser_S1D(source, monophonicChords); }

} // End of namespace AGOS
