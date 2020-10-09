/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "common/endian.h"
#include "common/stream.h"

#include "engines/grim/update/packfile.h"

namespace Grim {

const uint32 PackFile::_knownOffsets[] = { 0x1c000, 0x21000, 0x23000, 0x24000, 0 };

PackFile::PackFile(Common::SeekableReadStream *data):
	_codeTable(nullptr), _orgStream(data), _offset(0), _kCodeTableSize(0x100) {

	uint32 magicContainer, magicCabinet, key;

	for (int i = 0; _knownOffsets[i] != 0; ++i) {
		if (_knownOffsets[i] > uint32(_orgStream->size()))
			continue;

		_orgStream->seek(_knownOffsets[i]);

		//Check for content signature
		magicContainer = _orgStream->readUint32BE();
		if (!err() && magicContainer == MKTAG('1','C','N','T')) {
			key = _orgStream->readUint32LE();
			createCodeTable(key);
			_offset = _orgStream->pos();

			//Check for cabinet signature
			magicCabinet = readUint32BE();
			if (!err() && magicCabinet == MKTAG('M','S','C','F'))
				break;
			else {
				delete[] _codeTable;
				_codeTable = nullptr;
				_offset = 0;
				continue;
			}
		}
	}

	_size = _orgStream->size() - _offset;
	_orgStream->seek(_offset);
}


PackFile::~PackFile() {
	delete[] _codeTable;

	delete _orgStream;
}

bool PackFile::err() const {
	return _orgStream->err();
}

void PackFile::clearErr() {
	_orgStream->clearErr();
}

void PackFile::createCodeTable(uint32 key) {
	const uint32 kRandA = 0x343FD;
	const uint32 kRandB = 0x269EC3;
	uint32 value = key;

	delete[] _codeTable;
	_codeTable = new uint16[_kCodeTableSize * 2];

	for (uint i = 0; i < _kCodeTableSize; i++) {
		value = kRandA * value + kRandB;
		_codeTable[i] = uint16((value >> 16) & 0x7FFF);
	}
}

void PackFile::decode(uint8 *data, uint32 dataSize, uint32 start_point) {
	for (uint32 i = 0; i < dataSize; i++) {
		data[i] ^= uint8(_codeTable[(i + start_point) % _kCodeTableSize]);
		data[i] -= uint8(_codeTable[(i + start_point) % _kCodeTableSize] >> 8);
	}
}

uint32 PackFile::read(void *dataPtr, uint32 dataSize) {
	uint32 start_point, count;

	start_point = uint32(pos());
	count = _orgStream->read(dataPtr, dataSize);

	if (err() || count != dataSize)
		return 0;

	if (_codeTable)
		decode((uint8*)dataPtr, count, start_point);

	return count;
}

bool PackFile::eos() const {
	return _orgStream->eos();
}

int32 PackFile::pos() const {
	return _orgStream->pos() - _offset;
}

int32 PackFile::size() const {
	return _size;
}

bool PackFile::seek(int32 offset, int whence) {
	if (_codeTable && whence == SEEK_SET)
		offset += _offset;
	return _orgStream->seek(offset, whence);
}

} // end of namespace Grim
