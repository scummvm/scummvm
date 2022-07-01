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

#ifndef AUDIO_MIDIPARSER_SMF_H
#define AUDIO_MIDIPARSER_SMF_H

#include "audio/midiparser.h"

/**
 * The Standard MIDI File version of MidiParser.
 */
class MidiParser_SMF : public MidiParser {
protected:
	byte *_buffer;
	int8 _noteChannelToTrack[16];

protected:
	/**
	 * Compresses the specified type 1 MIDI tracks to a single type 0 track.
	 * 
	 * @param tracks Pointer to an array of type 1 tracks.
	 * @param numTracks The number of type 1 tracks.
	 * @param buffer Buffer which will contain the compressed type 0 track.
	 * @param malformedPitchBends True if broken pitch bend events consisting
	 * of just the command byte should be ignored. This is only useful for MIDI
	 * data which has this specific problem.
	 * @return The size of the compressed type 0 track in bytes.
	 */
	uint32 compressToType0(byte *tracks[], byte numTracks, byte *buffer, bool malformedPitchBends = false);
	void parseNextEvent(EventInfo &info) override;

public:
	MidiParser_SMF(int8 source = -1);
	~MidiParser_SMF();

	bool loadMusic(byte *data, uint32 size) override;

	int32 determineDataSize(Common::SeekableReadStream *stream) override;
};

#endif
