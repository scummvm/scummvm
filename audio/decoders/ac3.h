/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AUDIO_DECODERS_AC3_H
#define AUDIO_DECODERS_AC3_H

#include "common/scummsys.h"

#ifdef USE_A52

namespace Common {
class SeekableReadStream;
} // End of namespace Common

namespace Audio {

class PacketizedAudioStream;

/**
 * Create a PacketizedAudioStream that decodes AC-3 sound
 *
 * @param firstPacket  The stream containing the first packet of data
 * @return             A new PacketizedAudioStream, or NULL on error
 */
PacketizedAudioStream *makeAC3Stream(Common::SeekableReadStream &firstPacket, double decibel = 0.0);

} // End of namespace Audio

#endif

#endif

