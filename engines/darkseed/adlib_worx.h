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

class MidiDriver_Worx_AdLib : public MidiDriver_ADLIB_Multisource {
private:
	static AdLibIbkInstrumentDefinition WORX_INSTRUMENT_BANK[];

public:
	MidiDriver_Worx_AdLib(OPL::Config::OplType oplType, int timerFrequency);
	~MidiDriver_Worx_AdLib();

protected:
	uint8 allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId) override;
};

} // namespace Darkseed

#endif // DARKSEED_ADLIB_WORX_H
