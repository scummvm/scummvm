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

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "access/access.h"
#include "access/data.h"

namespace Access {

SpriteResource::SpriteResource(AccessEngine *vm, const byte *data, uint32 size,
		DisposeAfterUse::Flag disposeMemory) {
	Common::MemoryReadStream stream(data, size);
	Common::Array<uint32> offsets;
	int count = stream.readUint16LE();

	for (int i = 0; i < count; i++)
		offsets.push_back(stream.readUint32LE());
	offsets.push_back(size);	// For easier calculations of Noctropolis sizes

	// Build up the frames
	for (int i = 0; i < count; ++i) {
		stream.seek(offsets[i]);

		SpriteFrame *frame = new SpriteFrame();
		frame->_width = stream.readUint16LE();
		frame->_height = stream.readUint16LE();
		frame->_size = (vm->getGameID() == GType_MeanStreets) ? stream.readUint16LE() :
			offsets[i + 1] - offsets[i];

		frame->_data = new byte[frame->_size];
		stream.read(frame->_data, frame->_size);

		_frames.push_back(frame);
	}

	if (disposeMemory == DisposeAfterUse::YES)
		delete[] data;
}

SpriteResource::~SpriteResource() {
	for (uint i = 0; i < _frames.size(); ++i)
		delete _frames[i];
}

SpriteFrame::~SpriteFrame() {
	delete[] _data;
}

} // End of namespace Access
