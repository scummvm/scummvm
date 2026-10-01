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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCUMM_SMUSH_SAUD_H
#define SCUMM_SMUSH_SAUD_H

#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Scumm {

struct SaudHeader {
	uint32 dataSize = 0;
	uint32 sampleRate = 11025;
};

// Read a SAUD container spanning the rest of the stream, leaving it at its PCM.
// The sample rate defaults to 11025 Hz when no rate metadata is present.
// allowTruncated stops at the first SDAT, accepting the available PCM prefix
// without reading trailing metadata. This supports Rebel SFX and auxiliary buffers.
// The header is only updated on success.
bool readSaudHeader(Common::SeekableReadStream &stream, SaudHeader &header, bool allowTruncated = false);

} // End of namespace Scumm

#endif
