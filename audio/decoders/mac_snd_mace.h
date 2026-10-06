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

#ifndef AUDIO_MAC_SND_MACE_H
#define AUDIO_MAC_SND_MACE_H

#include "common/scummsys.h"
#include "common/types.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class SeekableAudioStream;

enum MacSndMACEType {
	kMacSndMACE3,
	kMacSndMACE6
};

SeekableAudioStream *makeMacSndMACEStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse,
	uint16 rate, uint32 packetFrameCount, uint32 channels,
	MacSndMACEType type);

} // End of namespace Audio

#endif
