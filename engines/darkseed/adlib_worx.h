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

#ifndef DARKSEED_ADLIB_WORX_H
#define DARKSEED_ADLIB_WORX_H

#include "audio/adlib_ms.h"

namespace Darkseed {

/**
 * Implementation of the AdLib code from the Worx Toolkit library,
 * based on version 2.1, as used by Dark Seed (CD version).
 * Implements the OPL channel allocation algorithm and note frequency
 * and volume calculation.
 *
 * TODO Implementation is incomplete, because Dark Seed does not use
 * some functionality of the library:
 * - OPL rhythm mode
 * - Transpose controllers 0x68 and 0x69 (seems to be buggy)
 */
class MidiDriver_Worx_AdLib : public MidiDriver_ADLIB_Multisource {
private:
	/**
	 * The OPL instrument bank of the Worx Toolkit.
	 * This was taken from the Dark Seed executable and might have been
	 * customized for the game.
	 */
	static AdLibIbkInstrumentDefinition WORX_INSTRUMENT_BANK[];
	/**
	 * The OPL frequency (F-num) for each octave note.
	 */
	static const uint16 OPL_NOTE_FREQUENCIES[];

public:
	MidiDriver_Worx_AdLib(OPL::Config::OplType oplType, int timerFrequency = OPL::OPL::kDefaultCallbackFrequency);
	~MidiDriver_Worx_AdLib();

protected:
	uint8 allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId) override;
	uint16 calculateFrequency(uint8 channel, uint8 source, uint8 note) override;
	uint8 calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, OplInstrumentDefinition &instrumentDef, uint8 operatorNum) override;
};

} // namespace Darkseed

#endif // DARKSEED_ADLIB_WORX_H
