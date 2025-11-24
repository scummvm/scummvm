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

#include "common/debug.h"
#include "common/stream.h"

#include "engines/private/savegame.h"

namespace Private {

static const uint32 kSavegameHeader = MKTAG('P','E','Y','E');

void writeSavegameMetadata(Common::WriteStream *stream, const SavegameMetadata &meta) {
	stream->writeUint32BE(kSavegameHeader);
	stream->writeUint16LE(meta.version);
	stream->writeSByte(meta.language);
	stream->writeSByte(meta.platform);
}

bool readSavegameMetadata(Common::SeekableReadStream *stream, SavegameMetadata &meta) {
	byte buffer[8];
	stream->read(buffer, 8);
	if (stream->eos() || stream->err()) {
		return false;
	}

	uint32 header = READ_BE_UINT32(buffer);
	if (header != kSavegameHeader) {
		debugN(1, "Save does not have metadata header");
		return false;
	}

	meta.version = READ_LE_UINT16(buffer + 4);
	if (meta.version < kMinimumSavegameVersion) {
		debugN("Save version %d lower than minimum %d", meta.version, kMinimumSavegameVersion);
		return false;
	}
	if (meta.version > kCurrentSavegameVersion) {
		debugN("Save version %d newer than current %d", meta.version, kCurrentSavegameVersion);
		return false;
	}

	meta.language = (Common::Language)buffer[6];
	meta.platform = (Common::Platform)buffer[7];

	return true;
}

} // End of namespace Private
