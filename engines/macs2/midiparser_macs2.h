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

#ifndef MACS2_MIDIPARSER_MACS2_H
#define MACS2_MIDIPARSER_MACS2_H

#include "audio/midiparser.h"

namespace Macs2 {

/**
 * MidiParser for the macs2 proprietary song format.
 *
 * The format is MIDI-like with:
 * - A file header containing instrument offset, data offset, timer frequency
 * - VLQ delta times (standard MIDI variable-length quantity)
 * - Running status
 * - Standard MIDI events: Note On/Off, Program Change, Control Change
 * - Custom CCs: 0x66 (loop count), 0x67 (percussion mode), 0x68/0x69 (pitch bend)
 * - Meta/system event (0xF0+) triggers loop back to start
 */
class MidiParser_Macs2 : public MidiParser {
public:
	MidiParser_Macs2();

	bool loadMusic(const byte *data, uint32 size) override;
	void parseNextEvent(EventInfo &info) override;

protected:
	void resetTracking() override;

private:
	const byte *_dataStart;
	uint16 _timerFrequency;
};

} // End of namespace Macs2

#endif
