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

#ifndef ACCESS_MARTIAN_MIDI_PARSER_BEMD_H
#define ACCESS_MARTIAN_MIDI_PARSER_BEMD_H

#include "audio/midiparser.h"

namespace Access {

class MidiParser_BEmd : public MidiParser {
public:
	MidiParser_BEmd();

	bool loadMusic(const byte *data, uint32 size) override;

protected:
	void parseNextEvent(EventInfo &info) override;
	bool processEvent(const EventInfo &info, bool fireEvents) override;
	void resetTracking() override;

private:
	const byte *_trackDataEnd;
	const byte *_tickData;
	const byte *_tickDataEnd;

};

} // end namespace Access

#endif // ACCESS_MARTIAN_MIDI_PARSER_BEMD_H
