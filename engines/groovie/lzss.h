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

#include "common/stream.h"

namespace Groovie {

class LzssReadStream : public Common::ReadStream {
private:
	uint8 *_outLzssBufData;
	uint32 _size;
	uint32 _pos;

	uint32 decodeLZSS(Common::ReadStream *in, uint8 lengthmask, uint8 lengthbits);

public:
	LzssReadStream(Common::ReadStream *indata, uint8 lengthmask, uint8 lengthbits);
	~LzssReadStream() override;

	bool eos() const override;
	uint32 read(void *buf, uint32 size) override;
};

} // End of Groovie namespace
