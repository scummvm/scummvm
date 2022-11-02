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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TWINE_RESOURCES_LZSS_H
#define TWINE_RESOURCES_LZSS_H

#include "common/stream.h"

namespace TwinE {

class LzssReadStream : public Common::SeekableReadStream {
private:
	uint8 *_outLzssBufData;
	uint32 _size;
	uint32 _pos;
	bool _err = false;

	void decodeLZSS(Common::ReadStream *indata, uint32 mode, uint32 length);

public:
	LzssReadStream(Common::ReadStream *indata, uint32 mode, uint32 realsize);
	virtual ~LzssReadStream();

	void clearErr() override { _err = false; }
	bool err() const override { return _err; }
	int64 pos() const override { return _pos; }
	int64 size() const override { return _size; }
	bool seek(int64 offset, int whence = SEEK_SET) override;

	bool eos() const override;
	uint32 read(void *buf, uint32 size) override;
};

} // namespace TwinE

#endif
