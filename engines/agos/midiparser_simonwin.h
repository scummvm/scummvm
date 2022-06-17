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

#ifndef AGOS_MIDIPARSER_SIMONWIN_H
#define AGOS_MIDIPARSER_SIMONWIN_H

#include "audio/midiparser_smf.h"

#include "common/stream.h"

namespace AGOS {

/**
 * Parser for the MIDI data of the Windows versions of Simon The Sorcerer 1
 * and 2. This consists of 1 or more type 1 SMF files, preceded by a byte
 * indicating the number of files. The MIDI data for Simon 1 also has broken
 * pitch bend events, as well as incorrect tempos, which are corrected by this
 * parser.
 */
class MidiParser_SimonWin : public MidiParser_SMF {
protected:
	static const uint8 MAXIMUM_TRACKS = 16;

public:
	int32 determineDataSize(Common::SeekableReadStream *stream) override;

	MidiParser_SimonWin(int8 source = -1, bool useDosTempos = false);
	~MidiParser_SimonWin();

	void setTempo(uint32 tempo) override;

	bool loadMusic(byte *data, uint32 size) override;
	void unloadMusic() override;

protected:
	void parseNextEvent(EventInfo &info) override;

	byte *_trackData[MAXIMUM_TRACKS];

	bool _useDosTempos;
};

} // End of namespace AGOS

#endif
