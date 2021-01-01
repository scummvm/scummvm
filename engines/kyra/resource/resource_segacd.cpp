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

#ifdef ENABLE_EOB

#include "kyra/resource/resource.h"
#include "kyra/resource/resource_intern.h"
#include "kyra/resource/resource_segacd.h"
#include "common/substream.h"

namespace Kyra {

SegaCDResource::SegaCDResource(Resource *res) : _res(res), _str(0), _resTable(0), _numResources(0), _curOffset(0), _curSize(0) {
}

SegaCDResource::~SegaCDResource() {
	unloadContainer();
}

bool SegaCDResource::loadContainer(const Common::String &filename, uint32 offset, uint32 size) {
	if (_curFile.equals(filename) && _curOffset == offset && _curSize == size)
		return true;

	unloadContainer();

	_str = _res->createEndianAwareReadStream(filename);
	if (!_str) {
		error("SegaCDResource: File '%s' not found.", filename.c_str());
		return false;
	}

	_str->seek(offset, SEEK_SET);
	uint32 first = _str->readUint32();
	_numResources = first >> 2;

	if (_numResources & 0xFFFF0000) {
		_curFile.clear();
		_numResources = 0;
		return false;
	}

	for (int i = 1; i < _numResources; ++i) {
		uint32 next = _str->readUint32();
		if (next == 0) {
			_numResources = i;
		} else if (next < first) {
			first = next;
			_numResources = first >> 2;
		}
	}

	_str->seek(offset, SEEK_SET);
	_resTable = new TableEntry[_numResources];
	for (int i = 0; i < _numResources; ++i)
		_resTable[i]._offset = offset + _str->readUint32();

	if (size)
		assert(offset + size <= (uint32)_str->size());

	for (int i = 0; i < _numResources; ++i) {
		uint32 next = size ? offset + size : _str->size();
		for (int ii = 0; ii < _numResources; ++ii) {
			if (_resTable[ii]._offset <= _resTable[i]._offset)
				continue;
			next = MIN<uint32>(_resTable[ii]._offset, next);
		}
		_resTable[i]._len = next - _resTable[i]._offset;
	}

	_curFile = filename;
	_curOffset = offset;
	_curSize = size;

	return true;
}

void SegaCDResource::unloadContainer() {
	delete[] _resTable;
	delete _str;
	_resTable = 0;
	_numResources = 0;
	_str = 0;
}

Common::SeekableReadStreamEndian *SegaCDResource::resStreamEndian(int resID) {
	if (!_str || !_resTable || resID >= _numResources)
		return 0;

	Common::SeekableReadStream *str = resStream(resID);
	if (!str)
		return 0;

	return new EndianAwareStreamWrapper(str, _str->isBE(), true);
}

Common::SeekableReadStream *SegaCDResource::resStream(int resID) {
	if (!_str || !_resTable || resID >= _numResources)
		return 0;

	return new Common::SeekableSubReadStream(_str, _resTable[resID]._offset, _resTable[resID]._offset + _resTable[resID]._len, DisposeAfterUse::NO);
}

uint8 *SegaCDResource::resData(int resID, uint32 *resLen) {
	if (!_str || !_resTable || resID >= _numResources)
		return 0;

	uint8 *res = new uint8[_resTable[resID]._len];

	_str->seek(_resTable[resID]._offset, SEEK_SET);
	_str->read(res, _resTable[resID]._len);

	if (resLen)
		*resLen = _resTable[resID]._len;

	return res;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
