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

#ifndef NUVIE_SOUND_MIDIDRV_M_MT32_H
#define NUVIE_SOUND_MIDIDRV_M_MT32_H

#include "audio/mt32gm.h"

namespace Ultima {
namespace Nuvie {

// An assignment of a MIDI instrument to a MIDI output channel. M data channels
// using this instrument will be mapped to the specified MIDI channel.
struct MInstrumentAssignment {
	int8 midiChannel;
	uint8 midiInstrument;
};

/**
 * M driver for the Roland MT-32.
 * The M format is focused on OPL2 and conversion to MIDI is rudimentary. Only
 * note on/off, channel volume (using note velocity) and program change are
 * implemented by the original driver.
 * A mapping of M instruments to MIDI instruments must be set using
 * setInstrumentAssignments before starting playback of a track.
 */
class MidiDriver_M_MT32 : public MidiDriver_MT32GM {
protected:
	/**
	 * Playback status information for an M channel.
	 * Note that although this data applies to an M data channel, the values
	 * are MIDI note and velocity values.
	 */
	struct MChannelData {
		// The MIDI note currently played on this channel.
		int8 activeNote = -1;
		// The MIDI note velocity currently used on this channel.
		uint8 velocity = 0;
		// The MIDI note queued for playback on this channel.
		int8 queuedNote = -1;
	};

	// Converts M note values to MIDI notes.
	static const uint8 MIDI_NOTE_MAP[24];

public:
	MidiDriver_M_MT32();
	~MidiDriver_M_MT32();

	using MidiDriver_MT32GM::open;
	int open(MidiDriver *driver, bool nativeMT32) override;

	using MidiDriver_MT32GM::send;
	void send(int8 source, uint32 b) override;
	void metaEvent(int8 source, byte type, byte *data, uint16 length) override;

	/**
	 * Sets the assignments of the 16 M instruments to the MIDI instruments and
	 * MIDI output channels they should use.
	 * 
	 * @param assignments An instrument assignment array of length 16
	 */
	void setInstrumentAssignments(const MInstrumentAssignment *assignments);

	void stopAllNotes(bool stopSustainedNotes) override;

protected:
	void onTimer() override;

	MChannelData _mChannelData[9];
	// Mapping of M instrument numbers to MIDI output channels
	int8 _mInstrumentMidiChannels[16];
	// Mapping of M instrument numbers to MIDI instrument numbers
	uint8 _mInstrumentMapping[16];
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
