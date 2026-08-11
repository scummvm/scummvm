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

#ifndef FREESCAPE_DRILLER_MUSICDATA_H
#define FREESCAPE_DRILLER_MUSICDATA_H

#include "common/scummsys.h"

namespace Freescape {

extern const uint8_t frq_lo[96];
extern const uint8_t frq_hi[96];

extern const uint8_t instrumentDataA0[];
extern const uint8_t instrumentDataA1[];
extern const int NUM_INSTRUMENTS;

extern const uint8_t arpeggio_data[];

extern const uint8_t *const pattern_addresses[];
extern const int NUM_PATTERNS;

extern const uint8_t tune_tempo_data[];
extern const uint8_t *const tune_track_data[][3];
extern const int NUM_TUNES;

extern const uint8_t initialSomethingData[][3];
extern const uint8_t initialInstrumentIndex[];
extern const uint8_t initialSomethingElseData[][3];
extern const uint8_t initialCtrl0[];
extern const uint8_t initialPwDirection[];
extern const uint8_t initialStuffData[][7];
extern const uint8_t initialThingsData[][7];
extern const uint8_t initialTwoCtr[];

} // namespace Freescape

#endif
