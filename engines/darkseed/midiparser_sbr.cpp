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

#include "darkseed/midiparser_sbr.h"

#include "audio/mididrv.h"

namespace Darkseed {

MidiParser_SBR::MidiParser_SBR(int8 source, bool sfx) : MidiParser_SMF(source), _sfx(sfx) {
	Common::fill(_trackDeltas, _trackDeltas + ARRAYSIZE(_trackDeltas), 0);
	Common::fill(_trackInstruments, _trackInstruments + ARRAYSIZE(_trackInstruments), 0xFF);
	Common::fill(_trackNoteActive, _trackNoteActive + ARRAYSIZE(_trackNoteActive), 0xFF);
	Common::fill(_trackLoopCounter, _trackLoopCounter + ARRAYSIZE(_trackLoopCounter), 0);

	// SBR uses a fixed tempo.
	_ppqn = 96;
	setTempo(1318214);
}

void MidiParser_SBR::parseNextEvent(EventInfo &info) {
	uint8 subtrack = info.subtrack;
	const byte *parsePos = _position._subtracks[subtrack]._playPos;
	const uint8 *start = parsePos;

	/**
	 * SBR uses 6 byte structures to represent events:
	 * - Event type / note
	 * - Note velocity
	 * - (unused)
	 * - Delta (word, little-endian)
	 * - Instrument
	 * Delta is ticks to the next event, not preceding this event like SMF.
	 *
	 * The following event types are used:
	 * - 0x00: End of track. This is the only byte in this event.
	 * - 0x01: Subtrack list. See loadMusic.
	 * - 0x02: Rest. Effectively a note off and a delta to the next event.
	 * - 0x03: Sample. Indicates the sample filename shoud be read from the 
	 *         corresponding DIG file entry. See Sound class.
	 * - 0x05: Restart playback from the beginning of the subtrack.
	 * - 0x06: Loop. Velocity specifies the number of events to jump back.
	 *         Delta specifies the number of times to repeat this section.
	 * - 0x24+: Note on.
	 */
	info.start = start;
	info.length = 0;
	info.noop = false;
	info.loop = false;
	info.delta = _trackDeltas[subtrack];
	_trackDeltas[subtrack] = 0;

	// Any event will turn off the active note on this subtrack.
	if (_trackNoteActive[subtrack] != 0xFF) {
		info.event = 0x80 | subtrack;
		info.basic.param1 = _trackNoteActive[subtrack];
		info.basic.param2 = 0x00;
		_trackNoteActive[subtrack] = 0xFF;
		return;
	}
	
	if (parsePos == nullptr || parsePos[0] == 0) {
		// Reached the end of the track. Generate an end-of-track event.
		info.event = 0xFF;
		info.ext.type = MidiDriver::MIDI_META_END_OF_TRACK;
		info.ext.data = parsePos;
		return;
	}

	// There are more MIDI events in this track.
	uint8 noteType = parsePos[0];
	if (noteType == 5) {
		// Subtrack needs to be restarted. Generate a custom meta event.
		info.event = 0xFF;
		info.ext.type = 5;
		info.loop = true;
		return;
	}

	if (noteType == 6) {
		// Subtrack needs to be looped. Generate a custom meta event.
		info.event = 0xFF;
		info.ext.type = 6;
		info.ext.data = parsePos;
		info.loop = true;
		// Delta needs to be one more than specified due to differences in the
		// way this event is processed compared to the original code.
		info.delta++;
		return;
	}

	// Check if the instrument specified in this event is different from the 
	// current instrument for this track. Typically, all events in a subtrack 
	// have the same instrument.
	uint8 instrument = parsePos[5];
	if (_trackInstruments[subtrack] != instrument) {
		if (_trackInstruments[subtrack] <= 0xF && (_trackInstruments[subtrack] / 3) != (instrument / 3)) {
			// WORKAROUND The current instrument for this subtrack is a rhythm
			// instrument, and the instrument specified in this event is for a
			// different rhythm instrument type. This occurs a few times in
			// the Dark Seed SBR files. The instrument for the offending events
			// is 0, probably by mistake. The original code will allocate a
			// subtrack to an OPL rhythm instrument based on the first event
			// and then never change it, even when an event with an instrument
			// with a different rhythm type is encountered. Instead, it will
			// write the new instrument definition to the originally allocated
			// OPL rhythm instrument, even if it has the wrong rhythm type.
			// The ScummVM code will change the allocation to the rhythm
			// instrument indicated by the instrument on the new event, which
			// causes incorrect playback, because typically there already is
			// a subtrack allocated to this instrument.
			// To fix this, the incorrect instrument on this event is simply
			// ignored.
		}
		else {
			// The instrument on this event is different from the current
			// instrument. Generate a program change event.
			_trackInstruments[subtrack] = instrument;
			info.event = 0xC0 | subtrack;
			info.basic.param1 = instrument;
			info.basic.param2 = 0;
			return;
		}
	}

	if (noteType >= 24) {
		// Note on.
		info.event = 0x90 | subtrack;
		info.basic.param1 = noteType;
		info.basic.param2 = parsePos[1];
		_trackNoteActive[subtrack] = noteType;
	}
	else {
		// For rest events, nothing needs to be done other than turning off
		// the current note (already done above) and process the delta.
		// Subtrack list and sample events are not processed here.
		info.noop = true;
	}
	if (noteType == 2 || noteType >= 24) {
		// Delta to the next event is only processed for
		// note on and rest events.
		_trackDeltas[subtrack] = READ_LE_UINT16(parsePos + 3);
	}

	// Set play position to the start of the next event.
	_position._subtracks[subtrack]._playPos += 6;
}

bool MidiParser_SBR::processEvent(const EventInfo &info, bool fireEvents) {
	// Handle custom meta events here.
	if (info.event == 0xFF) {
		uint8 subtrack = info.subtrack;
		if (info.ext.type == 5) {
			// Restart. Set play position to the beginning of the subtrack.
			_position._subtracks[subtrack]._playPos = _tracks[_activeTrack][subtrack];
			return true;
		}
		else if (info.ext.type == 6) {
			// Loop.
			bool loop = false;
			if (_trackLoopCounter[subtrack] > 0) {
				// A loop iteration has completed.
				_trackLoopCounter[subtrack]--;
				if (_trackLoopCounter[subtrack] > 0) {
					// There are more iterations remaining.
					loop = true;
				}
				else {
					// Loop has finished. Playback will resume at the event
					// after the loop event.
					_position._subtracks[subtrack]._playPos += 6;
				}
			}
			else {
				// Initialize the loop. Read number of iterations from the
				// event delta.
				_trackLoopCounter[subtrack] = READ_LE_UINT16(info.ext.data + 3);
				loop = true;
			}
			if (loop) {
				// Set the play position back by the number of events indicated
				// by the event velocity.
				_position._subtracks[subtrack]._playPos -= ((info.ext.data[1]) * 6);
			}
			return true;
		}
	}

	// All other events are handled like SMF events.
	return MidiParser::processEvent(info, fireEvents);
}

void MidiParser_SBR::onTrackStart(uint8 track) {
	Common::fill(_trackDeltas, _trackDeltas + ARRAYSIZE(_trackDeltas), 0);
	Common::fill(_trackInstruments, _trackInstruments + ARRAYSIZE(_trackInstruments), 0xFF);
	Common::fill(_trackNoteActive, _trackNoteActive + ARRAYSIZE(_trackNoteActive), 0xFF);
	Common::fill(_trackLoopCounter, _trackLoopCounter + ARRAYSIZE(_trackLoopCounter), 0);
}

bool MidiParser_SBR::loadMusic(const byte *data, uint32 size) {
	assert(size > 0);

	unloadMusic();

	// SBR files typically contain 120 tracks; some have 100.
	// Music files only use the first 10. SFX files use the remaining 110.
	uint8 startTrack = _sfx ? 10 : 0;
	uint8 endTrack = _sfx ? 120 : 10;

	// Read all the tracks. These consist of 0 or more 6 byte events,
	// terminated by a 00 byte.
	uint16 bytesRead = 0;
	for (int i = 0; i < endTrack; i++) {
		const byte *startOfTrack = data;
		uint16 trackSize = 0;

		bool foundEndOfTrack = false;
		while (bytesRead < size) {
			uint8 eventType = data[0];
			if (eventType == 0) {
				foundEndOfTrack = true;
				data++;
				trackSize++;
				bytesRead++;
				break;
			}
			else {
				data += 6;
				trackSize += 6;
				bytesRead += 6;
			}
		}

		if (!foundEndOfTrack) {
			// Some files have less than 120 tracks
			endTrack = i;
			break;
		}
		else if (i < startTrack) {
			_tracks[i][0] = nullptr;
		}
		else {
			_tracks[i][0] = startOfTrack;
		}
	}
	_numTracks = endTrack;

	// Look for tracks starting with a subtrack list event (type 01).
	// These are tracks consisting of multiple parallel subtracks.
	// Music files typically have a subtrack list in track 0. Some SFX use
	// multiple subtracks as well.
	for (int i = 0; i < _numTracks; i++) {
		if (_tracks[i][0] != nullptr && _tracks[i][0][0] == 0x01) {
			// Read the subtrack list. This is a variable-length list of
			// subtrack indices, terminated by a 00 byte.
			// (The track is padded with garbage and terminated by another
			// 00 byte to match the x * 6 byte event format used by all tracks.)
			const uint8 *tracklist = _tracks[i][0] + 1;
			uint8 subtrackIndex = 0;
			while (*tracklist != 0) {
				_tracks[i][subtrackIndex++] = _tracks[*tracklist][0];
				tracklist++;
			}
			_numSubtracks[i] = subtrackIndex;
		}
		else {
			// This is a track containing just a single subtrack.
			_numSubtracks[i] = 1;
		}
	}

	_disableAutoStartPlayback = true;
	resetTracking();

	setTrack(0);
	return true;
}

bool MidiParser_SBR::isSampleSfx(uint8 sfxId) {
	if (!_sfx || sfxId >= _numTracks)
		return false;
	return _tracks[sfxId][0] != nullptr && _tracks[sfxId][0][0] == 0x03;
}

} // End of namespace Darkseed
