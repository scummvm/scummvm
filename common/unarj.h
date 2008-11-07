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
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_UNARJ_H
#define COMMON_UNARJ_H

#include "common/file.h"
#include "common/hash-str.h"

namespace Common {

struct ArjHeader;
class ArjDecoder;

typedef HashMap<String, int, IgnoreCase_Hash, IgnoreCase_EqualTo> ArjFilesMap;

class ArjFile : public SeekableReadStream, public NonCopyable {
public:
	ArjFile();
	~ArjFile();

	void enableFallback(bool val) { _fallBack = val; }

	void registerArchive(const String &filename);

	bool open(const Common::String &filename);
	void close();

	uint32 read(void *dataPtr, uint32 dataSize);
	bool eos() const;
	int32 pos() const;
	int32 size() const;
	bool seek(int32 offset, int whence = SEEK_SET);
	bool isOpen() { return _uncompressed != 0; }

private:
	bool _fallBack;

	File _currArchive;
	Array<ArjHeader *> _headers;
	ArjFilesMap _fileMap;
	StringMap _archMap;

	SeekableReadStream *_uncompressed;

	ArjDecoder *_decoder;
};

} // End of namespace Common

#endif
