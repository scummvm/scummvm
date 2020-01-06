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

#include "kyra/resource/resource.h"
#include "kyra/resource/resource_intern.h"
#include "kyra/resource/resource_segacd.h"
#include "common/substream.h"

namespace Kyra {

SegaCDResource::SegaCDResource(Resource *res) : _res(res), _str(0), _offsetTable(0), _numResources(0) {	
}

SegaCDResource::~SegaCDResource() {
	unloadContainer();
}

bool SegaCDResource::loadContainer(const Common::String &filename) {
	unloadContainer();

	_str = _res->createEndianAwareReadStream(filename);
	if (!_str) {
		error("SegaCDResource: File '%s' not found.", filename);
		return false;
	}

	uint32 first = _str->readUint32();
	_numResources = first >> 2;
	_offsetTable = new uint32[_numResources + 1];
	_offsetTable[0] = first;
	for (int i = 1; i < _numResources; ++i) {
		_offsetTable[i] = _str->readUint32();
		if (_offsetTable[i] == 0)
			_numResources = i;
	}
	_offsetTable[_numResources] = _str->size();

	return true;
}

void SegaCDResource::unloadContainer() {
	delete[] _offsetTable;
	delete _str;
	_offsetTable = 0;
	_numResources = 0;
	_str = 0;
}

Common::SeekableReadStreamEndian *SegaCDResource::getEndianAwareResourceStream(int resID) {
	if (!_str || !_offsetTable || resID >= _numResources)
		return 0;

	Common::SeekableReadStream *str = getResourceStream(resID);
	if (!str)
		return 0;

	return new EndianAwareStreamWrapper(str, _str->isBE(), true);
}

Common::SeekableReadStream *SegaCDResource::getResourceStream(int resID) {
	if (!_str || !_offsetTable || resID >= _numResources)
		return 0;

	return new Common::SeekableSubReadStream(_str, _offsetTable[resID], _offsetTable[resID + 1], DisposeAfterUse::NO);
}

uint8 *SegaCDResource::fileData(int resID, uint32 *resLen) {
	if (!_str || !_offsetTable || resID >= _numResources)
		return 0;

	uint32 len = _offsetTable[resID + 1] - _offsetTable[resID];
	uint8 *res = new uint8[len];

	_str->seek(_offsetTable[resID], SEEK_SET);
	_str->read(res, len);

	if (resLen)
		*resLen = len;

	return res;
}

uint8 **SegaCDResource::loadAllResources(uint32 *numRes) {
	if (!_str || !_offsetTable)
		return 0;

	return 0;
}

} // End of namespace Kyra
