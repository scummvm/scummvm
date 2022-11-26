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

#include "common/concatstream.h"

namespace Common {

ConcatReadStream::ConcatReadStream(ParentStreamArray parentStreams) : _parentStreams(parentStreams), _totalSize(0), _linearPos(0), _volume(0), _volumePos(0), _err(false), _eos(false) {
	_sizes.resize(parentStreams.size());
	_startOffsets.resize(parentStreams.size());
	for (uint i = 0; i < parentStreams.size(); i++) {
		_sizes[i] = parentStreams[i]->size();
		_totalSize += _sizes[i];
	}
	_startOffsets[0] = 0;
	for (uint i = 1; i < parentStreams.size(); i++)
		_startOffsets[i] = _startOffsets[i - 1] + _sizes[i - 1];
}

bool ConcatReadStream::seek(int64 offset, int whence) {
	int64 target = 0;
	switch (whence) {
	case SEEK_SET:
		target = offset;
		break;
	case SEEK_CUR:
		target = _linearPos + offset;
		break;
	case SEEK_END:
		target = _totalSize + offset;
		break;
	default:
		return false;
	}

	if (target < 0 || target > _totalSize) {
		return false;
	}

	_linearPos = target;
	_eos = false;
	_err = false;

	// Special case: seek'ing to the EOF.
	if (target == _totalSize) {
		if (_parentStreams.empty()) {
			_volume = 0;
			_volumePos = 0;
			return true;
		}
		_volume = _parentStreams.size() - 1;
		_volumePos = _sizes[_parentStreams.size() - 1];
		return true;
	}

	// Find volume
	for (unsigned vol = 0; vol < _parentStreams.size(); vol++) {
		if (_startOffsets[vol] <= _linearPos &&
		    _linearPos < _startOffsets[vol] + _sizes[vol]) {
			_volume = vol;
			_volumePos = _linearPos - _startOffsets[vol];
			return true;
		}
	}

	// Should never be reached.
	_err = true;
	return false;
}

bool ConcatReadStream::seekToVolume(int volume, int64 offset) {
	if (volume < 0 || (uint) volume >= _parentStreams.size()
	    || offset < 0 || offset >= _sizes[volume]) {
		_err = true;
		return false;
	}

	_err = false;
	_eos = false;
	_volume = volume;
	_volumePos = offset;
	_linearPos = _startOffsets[volume] + offset;
	return true;
}

uint32 ConcatReadStream::read(void *dataPtr, uint32 dataSize) {
	uint32 rem = dataSize;
	uint32 alreadyRead = 0;
	byte *curPtr = (byte*) dataPtr;
	while (rem > 0) {
		int64 avail = _startOffsets[_volume] + _sizes[_volume];
		while (avail == 0) {
			if (_volume + 1 >= _startOffsets.size()) {
				_eos = true;
				return alreadyRead;
			}
			_volume++;
			_volumePos = 0;
			avail = _startOffsets[_volume] + _sizes[_volume];
		}

		uint32 toRead = MIN((int64) rem, avail);
		_parentStreams[_volume]->seek(_volumePos); // Also clears error and EOS.
		uint32 actuallyRead = _parentStreams[_volume]->read(curPtr, toRead);
		alreadyRead += actuallyRead;
		rem -= actuallyRead;
		curPtr += actuallyRead;
		_volumePos += actuallyRead;
		_linearPos += actuallyRead;
		if (_volumePos == _sizes[_volume] && _volume + 1 < _startOffsets.size()) {
			_volume++;
			_volumePos = 0;			
		}
		if (_parentStreams[_volume]->err()) {
			_err = true;
			return alreadyRead;
		}
	}

	return alreadyRead;
}
} // End of namespace Common
