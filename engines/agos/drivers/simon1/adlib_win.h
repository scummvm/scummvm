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

#ifndef AGOS_SIMON1_ADLIB_WIN_H
#define AGOS_SIMON1_ADLIB_WIN_H

#include "audio/adlib_ms.h"

namespace AGOS {

/**
 * AdLib MIDI driver for the Windows version of Simon The Sorcerer.
 * This driver contains a workaround for converting the MT-32 instruments to
 * the General MIDI instruments used by this driver.
 */
class MidiDriver_Simon1_AdLib_Windows : public MidiDriver_ADLIB_Multisource {
public:
	MidiDriver_Simon1_AdLib_Windows();

protected:
	void programChange(uint8 channel, uint8 program, uint8 source) override;
};

} // End of namespace AGOS

#endif
