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
	// Reads a delta between events.
	virtual uint32 readDelta(const byte *&data);

	void parseNextEvent(EventInfo &info) override;

public:
	MidiParser_SMF(int8 source = -1);

	bool loadMusic(const byte *data, uint32 size) override;

	int32 determineDataSize(Common::SeekableReadStream *stream) override;
};

#endif
