/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/p4.h"
#include "engines/icb/sound/direct_sound.h"
#include "engines/icb/sound/sound_common.h"

#include "common/textconsole.h"
#include "common/stream.h"

namespace ICB {

// Common code between the three sound-managers
bool8 openWav(Common::SeekableReadStream *stream, _wavHeader &header, uint32 &length, uint32 &byteOffsetInCluster, int32 &lengthInCycles) {

	int32 pos = stream->pos();
	int32 bytesRead;
	char buff[1024];
	bool bMore = true;

	// Read the wave header
	if (stream->read(&header, sizeof(_wavHeader)) != sizeof(_wavHeader))
		return FALSE8;

	if (FROM_LE_16(header.formatTag) != 1) {
		warning("Only supports PCM uncompressed wav files");
		return FALSE8;
	}

	// Find the start of the data
	do {
		int32 i = 0;
		bytesRead = stream->read(buff, 256);
		for (i = 0; i < (bytesRead - 3) && bMore; i++)
			bMore = READ_LE_U32(buff + i) != MKTAG('a', 't', 'a', 'd');

		if (bMore)
			stream->seek(-3, SEEK_CUR);
		else
			stream->seek(i - bytesRead + 3, SEEK_CUR);

	} while (bMore && bytesRead == 256);

	if (bMore) // Can't find 'data' !
		return FALSE8;

	// Find length of data
	length = stream->readUint32LE();
	if (stream->err())
		return FALSE8;

	// Adjust this byte counter to represent the first data byte of the wav
	byteOffsetInCluster = (uint32)stream->pos();

	// Expected number of cycles this sample reguires to completely playback
	lengthInCycles = (int32)ceil(((double)length / (double)FROM_LE_32(header.avgBytesPerSec)) * 12.0f * 1.215f);

	stream->seek(pos);

	return TRUE8;
}

} // End of namespace ICB
