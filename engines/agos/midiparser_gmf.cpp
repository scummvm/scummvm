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

#include "agos/midiparser_gmf.h"

#include "audio/mididrv.h"

namespace AGOS {

MidiParser_GMF::MidiParser_GMF(int8 source, bool useDosTempos) : MidiParser_SMF(source), _useDosTempos(useDosTempos) {
	memset(_tracksEndPos, 0, sizeof(_tracksEndPos));
}

void MidiParser_GMF::parseNextEvent(EventInfo &info) {
	byte *parsePos = _position._playPos;
	uint8 *start = parsePos;
	uint32 delta = readVLQ(parsePos);

	// GMF does not use end of track events, so we have to use the size of the
	// MIDI data to determine if we're at the end of the track.

	// Simon 1 CD version data has several zero bytes at the end.
	// Check for these and ignore them.
	bool containsMoreData = true;
	if (parsePos > _tracksEndPos[_activeTrack] - 5) {
		containsMoreData = false;
		byte *checkPos = parsePos;
		while (checkPos < _tracksEndPos[_activeTrack]) {
			if (*checkPos != 0) {
				containsMoreData = true;
				break;
			}
			checkPos++;
		}
		// If we're already past the end of the track, the while loop will not
		// execute and containsMoreData remains false.
	}

	if (!containsMoreData) {
		// Reached the end of the track. Generate an end-of-track event.
		info.start = start;
		info.delta = delta;
		info.event = 0xFF;
		info.ext.type = MidiDriver::MIDI_META_END_OF_TRACK;
		info.length = 0;
		info.ext.data = parsePos;
		info.noop = false;

		_position._playPos = parsePos;
		return;
	}

	// There are more MIDI events in this track.
	uint8 event = *(parsePos++);

	// Pitch bend events in the Simon 1 data are broken. They contain just the
	// command byte; no data bytes follow. We generate an empty event and set
	// the noop flag to have MidiParser process only the delta and otherwise
	// ignore the event.
	if ((event & 0xF0) == MidiDriver::MIDI_COMMAND_PITCH_BEND) {
		info.start = start;
		info.delta = delta;
		info.event = event;
		info.basic.param1 = 0;
		info.basic.param2 = 0;
		info.length = 0;
		info.noop = true;

		_position._playPos = parsePos;
	} else {
		// Processing of the other events is the same as the SMF format.
		info.noop = false;
		MidiParser_SMF::parseNextEvent(info);
	}
}

bool MidiParser_GMF::loadMusic(byte *data, uint32 size) {
	assert(size > 7);

	unloadMusic();

	// Determine start and end of the MIDI track(s) in the data, as well as
	// tempo and loop flag.
	uint8 headerTempo;
	bool headerLoop;

	// Simon 1 uses two GMF variants: a single track music file and a multiple
	// track SFX file. These are processed as a MIDI type 0 and type 2 track,
	// respectively.
	if (!memcmp(data, "GMF", 3)) {
		// Single track file.
		_numTracks = 1;

		// GMF header is GMF<majorVersion><minorVersion><tempo><loop>.
		// Version is always 1.0 for Simon 1.
		headerTempo = data[5];
		headerLoop = data[6] == 1;

		// MIDI track data starts immediately after the GMF header.
		_tracks[0] = data + 7;
		_tracksEndPos[0] = data + size;
	} else {
		// Assume multi-track file.

		// Each track has its own GMF header, but tempo and loop flags are all
		// the same for Simon 1.
		headerTempo = 2;
		headerLoop = false;

		// A multi-track file starts with a list of 2-byte offsets which
		// identify the starting position of each track, as well as the end of
		// the last track.
		byte *pos = data;
		// Read the start offset of the first track.
		byte *trackStart = data + READ_LE_UINT16(pos);
		pos += 2;
		// The number of offsets before the first track indicates the number of
		// tracks plus 1 because the end offset of the last track is included
		// as well.
		_numTracks = (*trackStart / 2) - 1;

		if (_numTracks > ARRAYSIZE(_tracks)) {
			warning("MidiParser_GMF::loadMusic - Can only handle %d tracks but was handed %d", (int)ARRAYSIZE(_tracks), (int)_numTracks);
			return false;
		}

		// Read all the track start offsets.
		int tracksRead = 0;
		while (tracksRead < _numTracks) {
			_tracks[tracksRead] = trackStart + 7; // Skip 7-byte GMF header
			trackStart = data + READ_LE_UINT16(pos);
			pos += 2;
			// Start of the next track is the end of this track.
			_tracksEndPos[tracksRead] = trackStart;

			tracksRead++;
		}
	}

	// Note that we assume the original data passed in
	// will persist beyond this call, i.e. we do NOT
	// copy the data to our own buffer. Take warning....
	_disableAutoStartPlayback = true;
	resetTracking();
	_autoLoop = headerLoop;
	_ppqn = 192;

	uint32 tempo;
	if (_useDosTempos) {
		// These translations from the GMF header tempo (2-8) to the SMF tempo have
		// been determined by measuring the tempos generated by the DOS version of
		// Simon 1 in DOSBox.
		if (headerTempo < 6) {
			tempo = 330000 + ((headerTempo - 2) * 105000);
		} else {
			tempo = 750000 + ((headerTempo - 6) * 125000);
		}
	} else {
		// These are the tempos as specified in the Windows version SMF data.
		tempo = headerTempo * 125000;
	}
	setTempo(tempo);

	setTrack(0);
	return true;
}

} // End of namespace AGOS
