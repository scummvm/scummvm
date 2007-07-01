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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_PARSER_H
#define PARALLACTION_PARSER_H

#include "common/stream.h"

#include "parallaction/defs.h"

namespace Parallaction {

uint16 fillBuffers(Common::SeekableReadStream &stream, bool errorOnEOF = false);
char   *parseNextToken(char *s, char *tok, uint16 count, const char *brk);

extern char _tokens[][40];

class Script : public Common::SeekableReadStream {

	Common::SeekableReadStream *_input;
	bool	_disposeSource;

public:
	Script(Common::SeekableReadStream *, bool _disposeSource = false);
	~Script();

	uint32 read(void *dataPtr, uint32 dataSize);

	char *readLine(char *buf, size_t bufSize);

	bool eos() const;
	uint32 pos() const;
	uint32 size() const;

	void seek(int32 offset, int whence = SEEK_SET);
};

} // namespace Parallaction

#endif

