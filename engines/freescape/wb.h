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

#ifndef FREESCAPE_WB_H
#define FREESCAPE_WB_H

#include "audio/audiostream.h"
#include "common/types.h"

namespace Freescape {

namespace WBCommon {

/**
 * Decode order-list transpose command ($C1-$FE).
 * Formula used by Wally Beben engines: (cmd + $20) & $FF.
 */
int8 decodeOrderTranspose(byte cmd);

/**
 * Decode speed command ($F0-$FD): low nibble, with 0 coerced to 1.
 */
byte decodeTickSpeed(byte cmd);

/**
 * Decode duration command ($80-$BF): low 6 bits, with 0 coerced to 1.
 */
byte decodeDuration(byte cmd);

/**
 * Expand an arpeggio bitmask using the 8-byte interval lookup table.
 *
 * @param intervals   Source interval table (8 entries)
 * @param mask        Bitmask selecting entries from intervals
 * @param outTable    Output sequence buffer
 * @param maxLen      Capacity of output buffer
 * @param includeBase Whether to prepend 0 (base note)
 * @return Number of valid entries in outTable
 */
byte buildArpeggioTable(const byte intervals[8], byte mask, byte *outTable, byte maxLen, bool includeBase);

} // End of namespace WBCommon

/**
 * Create a music stream for the Wally Beben custom music engine
 * used in the Amiga version of Dark Side.
 *
 * @param data     Raw TEXT segment data from HDSMUSIC.AM (after 0x1C GEMDOS header)
 * @param dataSize Size of the TEXT segment (0xF4BC for Dark Side)
 * @param songNum  Song number to play (1 or 2)
 * @param rate     Output sample rate
 * @param stereo   Whether to produce stereo output
 * @return A new AudioStream, or nullptr on error
 */
Audio::AudioStream *makeWallyBebenStream(const byte *data, uint32 dataSize,
                                         int songNum = 1, int rate = 44100,
                                         bool stereo = true);

} // End of namespace Freescape

#endif
