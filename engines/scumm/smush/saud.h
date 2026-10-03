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
	int64 scriptOffset = -1;
	uint32 scriptSize = 0;
};

// Read a SAUD container spanning the rest of the stream, leaving it at its PCM.
// The sample rate defaults to 11025 Hz when no rate metadata is present.
// allowTruncated stops at the first SDAT, accepting the available PCM prefix
// without reading trailing metadata. This supports Rebel SFX and auxiliary buffers.
// The header is only updated on success.
bool readSaudHeader(Common::SeekableReadStream &stream, SaudHeader &header, bool allowTruncated = false);

enum SaudOpcode {
	SAUD_OP_INIT = 1,
	SAUD_OP_UPDATE_HEADER = 2,
	SAUD_OP_SET_PARAM = 3,
	SAUD_OP_INCR_PARAM = 4,
	SAUD_OP_STOP = 5,
	SAUD_OP_SET_OFFSET = 6,
	SAUD_OP_SET_LENGTH = 7,
	SAUD_OP_COMPARE_GT = 8,
	SAUD_OP_COMPARE_LT = 9,
	SAUD_OP_COMPARE_EQ = 10,
	SAUD_OP_COMPARE_NE = 11
};

enum SaudValueId {
	SAUD_VALUEID_ALL_VOLS = 0xff,
	SAUD_VALUEID_TRK_VOL = 0xfe,
	SAUD_VALUEID_TRK_PAN = 0xfd
};

struct SaudSpan {
	uint32 offset = 0;
	uint32 length = 0;
	uint32 rate = 0;
};

// STRK interpreter shared by the standalone INSANE sound players.
// Script storage and the 256 global variables belong to the caller.
class SaudScript {
public:
	SaudScript();
	void reset(const byte *data, uint32 size, uint32 defaultRate);
	bool nextSpan(SaudSpan &span, byte *variables, int &masterVolume, byte &volume, byte &pan);

private:
	bool fail(const char *message);
	bool branch(int16 displacement);

	const byte *_data;
	uint32 _size, _position, _defaultRate;
	uint32 _remaining, _elapsed;
};

} // End of namespace Scumm

#endif
