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

#ifndef CRYOMNI3D_DATSTREAM_H
#define CRYOMNI3D_DATSTREAM_H

#include "common/language.h"
#include "common/platform.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/substream.h"

namespace CryOmni3D {

class DATSeekableStream : public Common::SeekableSubReadStream {
public:

	/* Parent stream must not be used after this call and will be disposed if fetch succeeded */
	static DATSeekableStream *getGame(Common::SeekableReadStream *stream,
	                                  uint32 gameId, uint16 version, Common::Language lang, Common::Platform platform);

	Common::String readString16();
	void readString16Array16(Common::StringArray &array);

private:
	DATSeekableStream(SeekableReadStream *parentStream, uint32 start, uint32 end) :
		SeekableSubReadStream(parentStream, start, end, DisposeAfterUse::YES) { }

	static uint16 translateLanguage(Common::Language lang);
	static uint32 translatePlatform(Common::Platform platform);

	/* This is the version of the global file format
	 * Each game has then a version specific for its data */
	static const uint16 kFileVersion = 1;
};

} // End of namespace CryOmni3D

#endif
