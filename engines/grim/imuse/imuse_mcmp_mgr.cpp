/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/file.h"

#include "engines/grim/resource.h"

#include "engines/grim/imuse/imuse_mcmp_mgr.h"
#include "engines/grim/movie/codecs/vima.h"

namespace Grim {

uint16 imuseDestTable[5786];

McmpMgr::McmpMgr() {
	_compTable = nullptr;
	_numCompItems = 0;
	_curSample = -1;
	_compInput = nullptr;
	_outputSize = 0;
	_file = nullptr;
	_lastBlock = -1;
}

McmpMgr::~McmpMgr() {
	delete[] _compTable;
	delete[] _compInput;
}

bool McmpMgr::openSound(const char *filename, Common::SeekableReadStream *data, int &offsetData) {
	_file = data;

	uint32 tag = _file->readUint32BE();
	if (tag != 'MCMP') {
		error("McmpMgr::openSound() Expected MCMP tag");
		return false;
	}

	_numCompItems = _file->readSint16BE();
	assert(_numCompItems > 0);

	int32 offset = _file->pos() + (_numCompItems * 9) + 2;
	_numCompItems--;
	_compTable = new CompTable[_numCompItems];
	_file->seek(5, SEEK_CUR);
	int32 headerSize = _compTable[0].decompSize = _file->readSint32BE();
	int32 maxSize = headerSize;
	offset += headerSize;

	int i;
	for (i = 0; i < _numCompItems; i++) {
		_compTable[i].codec = _file->readByte();
		_compTable[i].decompSize = _file->readSint32BE();
		_compTable[i].compSize = _file->readSint32BE();
		_compTable[i].offset = offset;
		offset += _compTable[i].compSize;
		if (_compTable[i].compSize > maxSize)
			maxSize = _compTable[i].compSize;
	}
	int16 sizeCodecs = _file->readSint16BE();
	for (i = 0; i < _numCompItems; i++) {
		_compTable[i].offset += sizeCodecs;
	}
	_file->seek(sizeCodecs, SEEK_CUR);
	// hack: two more bytes at the end of input buffer
	_compInput = new byte[maxSize + 2];
	offsetData = headerSize;

	return true;
}

int32 McmpMgr::decompressSample(int32 offset, int32 size, byte **comp_final) {
	int32 i, final_size, output_size;
	int skip, first_block, last_block;

	if (!_file) {
		error("McmpMgr::decompressSampleByName() File is not open!");
		return 0;
	}

	first_block = offset / 0x2000;
	last_block = (offset + size - 1) / 0x2000;
	skip = offset % 0x2000;

	// Clip last_block by the total number of blocks (= "comp items")
	if ((last_block >= _numCompItems) && (_numCompItems > 0))
		last_block = _numCompItems - 1;

	int32 blocks_final_size = 0x2000 * (1 + last_block - first_block);
	*comp_final = static_cast<byte *>(malloc(blocks_final_size));
	final_size = 0;

	for (i = first_block; i <= last_block; i++) {
		if (_lastBlock != i) {
			// hack: two more zero bytes at the end of input buffer
			_compInput[_compTable[i].compSize] = 0;
			_compInput[_compTable[i].compSize + 1] = 0;
			_file->seek(_compTable[i].offset, SEEK_SET);
			_file->read(_compInput, _compTable[i].compSize);
			decompressVima(_compInput, (int16 *)_compOutput, _compTable[i].decompSize, imuseDestTable);
			_outputSize = _compTable[i].decompSize;
			if (_outputSize > 0x2000) {
				error("McmpMgr::decompressSample() _outputSize: %d", _outputSize);
			}
			_lastBlock = i;
		}

		output_size = _outputSize - skip;

		if ((output_size + skip) > 0x2000) // workaround
			output_size -= (output_size + skip) - 0x2000;

		if (output_size > size)
			output_size = size;

		assert(final_size + output_size <= blocks_final_size);

		memcpy(*comp_final + final_size, _compOutput + skip, output_size);
		final_size += output_size;

		size -= output_size;
		assert(size >= 0);
		if (size == 0)
			break;

		skip = 0;
	}

	return final_size;
}

} // end of namespace Grim
