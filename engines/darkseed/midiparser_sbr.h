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

#ifndef DARKSEED_MIDIPARSER_SBR_H
#define DARKSEED_MIDIPARSER_SBR_H

#include "audio/midiparser_smf.h"

namespace Darkseed {

/**
 * MIDI parser for the SBR format used by Dark Seed floppy version.
 */
class MidiParser_SBR : public MidiParser_SMF {
public:
	MidiParser_SBR(int8 source = -1, bool sfx = false);

	bool loadMusic(const byte *data, uint32 size) override;
	bool isSampleSfx(uint8 sfxId);

protected:
	void parseNextEvent(EventInfo &info) override;
	bool processEvent(const EventInfo &info, bool fireEvents = true) override;
	void onTrackStart(uint8 track) override;

	bool _sfx = false;
	uint8 _trackInstruments[10];
	uint16 _trackDeltas[10];
	uint8 _trackNoteActive[10];
	uint16 _trackLoopCounter[10];
};

} // End of namespace Darkseed

#endif
