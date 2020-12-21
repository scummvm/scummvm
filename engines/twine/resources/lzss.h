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
 */

#include "common/stream.h"

namespace TwinE {

class LzssReadStream : public Common::SeekableReadStream {
private:
	uint8 *_outLzssBufData;
	uint32 _size;
	uint32 _pos;

	void decodeLZSS(Common::ReadStream *indata, uint32 mode, uint32 length);

public:
	LzssReadStream(Common::ReadStream *indata, uint32 mode, uint32 realsize);
	~LzssReadStream();

	virtual int32 pos() const { return _pos; }
	virtual int32 size() const { return _size; }
	virtual bool seek(int32 offset, int whence = SEEK_SET);

	bool eos() const;
	uint32 read(void *buf, uint32 size);
};

} // namespace TwinE
