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

#ifndef AUDIO_ADLIB_HMISOS_H
#define AUDIO_ADLIB_HMISOS_H

#include "audio/adlib_ms.h"

#include "common/stream.h"

/**
 * General MIDI AdLib driver based on HMI Sound Operating System version dated 1994-11-27
 * (as used by The Riddle Of Master Lu).
 *
 * Currently, only instrument bank loading is implemented.
 * TODO:
 * - Frequency calculation
 * - Level calculation
 * - Channel allocation
 * - Controller 0x66 (pitch bend range)
 */
class MidiDriver_ADLIB_HMISOS : public MidiDriver_ADLIB_Multisource {
public:
	static const byte INSTRUMENT_BANK_SIGNATURE[8];

	MidiDriver_ADLIB_HMISOS(OPL::Config::OplType oplType, int timerFrequency = OPL::OPL::kDefaultCallbackFrequency);
	~MidiDriver_ADLIB_HMISOS() {};

	int open() override;
	bool loadInstrumentBanks(Common::SeekableReadStream *instrumentBankStream, Common::SeekableReadStream *rhythmBankStream = nullptr);

protected:
	int loadInstrumentBank(Common::SeekableReadStream *instrumentBankStream, bool rhythmBank);

	OplInstrumentDefinition _sosInstrumentBank[128];
	OplInstrumentDefinition _sosRhythmBank[128];
};

#endif
