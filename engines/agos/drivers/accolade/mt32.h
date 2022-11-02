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

#ifndef AGOS_DRIVERS_ACCOLADE_MT32_H
#define AGOS_DRIVERS_ACCOLADE_MT32_H

#include "audio/mt32gm.h"

namespace AGOS {

class MidiDriver_Accolade_MT32 : public MidiDriver_MT32GM {
protected:
	static const uint8 SFX_PROGRAM_BASE = 0x75;
	static const uint8 SYSEX_INSTRUMENT_ASSIGNMENT[7];

public:
	MidiDriver_Accolade_MT32();

	int open(MidiDriver *driver, bool nativeMT32) override;
	using MidiDriver_MT32GM::send;
	void send(int8 source, uint32 b) override;

	int8 mapSourceChannel(uint8 source, uint8 dataChannel) override;
	void deinitSource(uint8 source) override;

	// Read the specified data from INSTR.DAT or MUSIC.DRV.
	void readDriverData(byte *driverData, uint16 driverDataSize, bool isMusicDrv);

	// Loads the specified instrument for the specified instrument source.
	void loadSfxInstrument(uint8 source, byte *instrumentData);
	// Changes the channel assigned to the specified SFX source to the SFX
	// program number.
	void changeSfxInstrument(uint8 source);

protected:
	// simple mapping between MIDI channel and MT32 channel
	byte _channelRemapping[16];
	// simple mapping between MIDI instruments and MT32 instruments
	byte _instrumentRemapping[128];

	// Indicates if a MIDI channel is locked by an SFX source and unavailable
	// for music.
	bool _channelLocks[MIDI_CHANNEL_COUNT];
};

} // End of namespace AGOS

#endif
