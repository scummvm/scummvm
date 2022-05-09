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

#ifndef AGOS_MIDIPARSER_GMF_H
#define AGOS_MIDIPARSER_GMF_H

#include "audio/midiparser_smf.h"

namespace AGOS {

/**
 * MIDI parser for the GMF format used by the DOS (and Acorn CD) versions of
 * Simon The Sorcerer. GMF is basically SMF but with the following differences:
 * - Different header
 * - Tempo is determined by a header field
 * - Just a single track with no track header
 * - No end of track or other meta events
 */
class MidiParser_GMF : public MidiParser_SMF {
public:
	MidiParser_GMF(int8 source = -1, bool useDosTempos = false);

	bool loadMusic(byte *data, uint32 size) override;

protected:
	void parseNextEvent(EventInfo &info) override;

	// The end position of each track, exclusive
	// (i.e. 1 byte past the end of the data).
	byte *_tracksEndPos[MAXIMUM_TRACKS];

	// True if the music tempos from the DOS version should be used; false if
	// the tempos from the Windows version should be used.
	bool _useDosTempos;
};

} // End of namespace AGOS

#endif
