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

#ifndef DARKSEED_ADLIB_DSF_H
#define DARKSEED_ADLIB_DSF_H

#include "audio/adlib_ms.h"

namespace Darkseed {

/**
 * Implementation of the AdLib code used by Dark Seed (floppy version).
 * The game uses a combination of the Creative FM-Music Functions library and
 * its own sound code.
 */
class MidiDriver_DarkSeedFloppy_AdLib : public MidiDriver_ADLIB_Multisource {
protected:
	static const uint16 OPL_NOTE_FREQUENCIES[12];

public:
	MidiDriver_DarkSeedFloppy_AdLib(OPL::Config::OplType oplType, int timerFrequency = OPL::OPL::kDefaultCallbackFrequency);
	~MidiDriver_DarkSeedFloppy_AdLib();

	int open() override;

	void deinitSource(uint8 source) override;

	void setSourcePriority(uint8 source, uint8 priority);

	void loadInstrumentBank(uint8 *instrumentBankData);

protected:
	uint8 allocateOplChannel(uint8 channel, uint8 source, InstrumentInfo &instrumentInfo) override;
	uint16 calculateFrequency(uint8 channel, uint8 source, uint8 note) override;
	uint8 calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, const OplInstrumentDefinition &instrumentDef, uint8 operatorNum) override;

	uint8 _sourcePriority[MAXIMUM_SOURCES];
	OplInstrumentDefinition *_dsfInstrumentBank;
};

} // namespace Darkseed

#endif // DARKSEED_ADLIB_DSF_H
