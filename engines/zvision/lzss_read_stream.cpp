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

#include "common/scummsys.h"

#include "zvision/lzss_read_stream.h"

namespace ZVision {

LzssReadStream::LzssReadStream(Common::SeekableReadStream *source, bool stream, uint32 decompressedSize)
		: _source(source),
		  // It's convention to set the starting cursor position to blockSize - 16
		  _windowCursor(0x0FEE),
		  _readCursor(0) {
	// Clear the window to null
	memset(_window, 0, blockSize);

	// Reserve space in the destination buffer
	// TODO: Make a better guess
	if (decompressedSize == npos) {
		decompressedSize = source->size();
	}
	_destination.reserve(decompressedSize);

	if (stream)
		decompressBytes(blockSize);
	else
		decompressAll();
}

void LzssReadStream::decompressBytes(uint32 numberOfBytes) {
	uint32 bytesRead = 0;

	while (!_source->eos() && bytesRead <= numberOfBytes) {
		byte flagbyte = _source->readByte();
		byte mask = 1;

		for (uint32 i = 0; i < 8; i++) {
			if (!_source->eos()) {
				if ((flagbyte & mask) == mask)
				{
					byte data = _source->readByte();
					bytesRead++;
					if (_source->eos())
						break;

					_window[_windowCursor] = data;
					_destination.push_back(data);

					// Increment and wrap the window cursor
					_windowCursor = (_windowCursor + 1) & 0xFFF;
				}
				else
				{
					byte low = _source->readByte();
					bytesRead++;
					if (_source->eos())
						break;

					byte high = _source->readByte();
					bytesRead++;
					if (_source->eos())
						break;

					uint16 length = (high & 0xF) + 2;
					uint16 offset = low | ((high & 0xF0)<<4);

					for(byte j = 0; j <= length; j++)
					{
						byte temp = _window[(offset + j) & 0xFFF];
						_window[_windowCursor] = temp;
						_destination.push_back(temp);
						_windowCursor = (_windowCursor + 1) & 0xFFF;
					}
				};

				mask = mask << 1;
			}
		}
	}
}

void LzssReadStream::decompressAll() {
	decompressBytes(_source->size());
}

bool LzssReadStream::eos() const {
	// Make sure that we've read all of the source
	return _readCursor >= _destination.size() && _source->eos();
}

uint32 LzssReadStream::read(void *dataPtr, uint32 dataSize) {
	// Check if there are enough bytes available
	// If not, keep decompressing until we have enough bytes or until we reach EOS
	while (dataSize > _destination.size() - _readCursor) {
		// Check if we can read any more data from source
		if (_source->eos()) {
			dataSize = _destination.size() - _readCursor;
			break;
		}

		decompressBytes(blockSize);
	}

	memcpy(dataPtr, _destination.begin() + _readCursor, dataSize);
	_readCursor += dataSize;

	return dataSize;
}

uint32 LzssReadStream::currentSize() const {
	return _destination.size();
}

} // End of namespace ZVision
