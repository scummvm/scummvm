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

#include "common/scummsys.h"

#include "zvision/file/lzss_read_stream.h"

namespace ZVision {

LzssReadStream::LzssReadStream(Common::SeekableReadStream *source)
	: _source(source),
	  // It's convention to set the starting cursor position to blockSize - 16
	  _windowCursor(0x0FEE),
	  _eosFlag(false) {
	// All values up to _windowCursor inits by 0x20
	memset(_window, 0x20, _windowCursor);
	memset(_window + _windowCursor, 0, BLOCK_SIZE - _windowCursor);
}

uint32 LzssReadStream::decompressBytes(byte *destination, uint32 numberOfBytes) {
	uint32 destinationCursor = 0;

	while (destinationCursor < numberOfBytes) {
		byte flagbyte = _source->readByte();
		if (_source->eos())
			break;
		uint mask = 1;

		for (int i = 0; i < 8; ++i) {
			if ((flagbyte & mask) == mask) {
				byte data = _source->readByte();
				if (_source->eos()) {
					return destinationCursor;
				}

				_window[_windowCursor] = data;
				destination[destinationCursor++] = data;

				// Increment and wrap the window cursor
				_windowCursor = (_windowCursor + 1) & 0xFFF;
			} else {
				byte low = _source->readByte();
				if (_source->eos()) {
					return destinationCursor;
				}

				byte high = _source->readByte();
				if (_source->eos()) {
					return destinationCursor;
				}

				uint16 length = (high & 0xF) + 2;
				uint16 offset = low | ((high & 0xF0) << 4);

				for (int j = 0; j <= length; ++j) {
					byte temp = _window[(offset + j) & 0xFFF];
					_window[_windowCursor] = temp;
					destination[destinationCursor++] = temp;
					_windowCursor = (_windowCursor + 1) & 0xFFF;
				}
			}

			mask = mask << 1;
		}
	}

	return destinationCursor;
}

bool LzssReadStream::eos() const {
	return _eosFlag;
}

uint32 LzssReadStream::read(void *dataPtr, uint32 dataSize) {
	uint32 bytesRead = decompressBytes(static_cast<byte *>(dataPtr), dataSize);
	if (bytesRead < dataSize) {
		// Flag that we're at EOS
		_eosFlag = true;
	}

	return dataSize;
}

} // End of namespace ZVision
