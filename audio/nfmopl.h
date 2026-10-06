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

/*
    OPL interface using nFM library(https://framagit.org/nokturnal/nfm)
    for NokturnFM2 / 3, CE OPL2 Audio board, CE OPL3 Duo!, Serdaco OPL2LPT / OPL3LPT, ST Bus ISA / VME SoundBlaster and NatFeats
    (c) 2023-26 Paweł Góralski
 */

#ifndef AUDIO_SYNTH_NFM_OPL_H
#define AUDIO_SYNTH_NFM_OPL_H

#include "audio/fmopl.h"

namespace OPL {
namespace NfmOPL {
enum OplDevice : int16_t {
	dtNokturnFM2 = 0,
	dtNokturnFM3,
	dtOPL2LPT,
	dtOPL3LPT,
	dtOPL2AudioBoard,
	dtOPL3Duo,
	dtStBusIsaVmeSb,
	dtNatfeatsOpl,
	dtNumDevices
};

namespace RealChip {
OPL *create(Config::OplType type, OplDevice device);
} // End of namespace RealChip

} // End of namespace NfmOPL
} // End of namespace OPL

#endif
