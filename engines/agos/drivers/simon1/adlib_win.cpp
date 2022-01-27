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

#include "agos/drivers/simon1/adlib_win.h"

namespace AGOS {

MidiDriver_Simon1_AdLib_Windows::MidiDriver_Simon1_AdLib_Windows() : MidiDriver_ADLIB_Multisource(OPL::Config::kOpl3) { }

void MidiDriver_Simon1_AdLib_Windows::programChange(uint8 channel, uint8 program, uint8 source) {
	// WORKAROUND The Windows version of Simon The Sorcerer uses the MT-32 MIDI
	// data of the DOS versions, but plays this using Windows' General MIDI
	// system. As a result, the music is played using different instruments
	// than intended. This is fixed here by mapping the MT-32 instruments to
	// GM instruments using MidiDriver's standard mapping.
	uint8 gmInstrument = _mt32ToGm[program];
	MidiDriver_ADLIB_Multisource::programChange(channel, gmInstrument, source);
}

} // End of namespace AGOS
