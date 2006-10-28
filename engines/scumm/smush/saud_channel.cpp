/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "scumm/util.h"
#include "scumm/smush/channel.h"
#include "scumm/smush/chunk.h"
#include "scumm/smush/chunk_type.h"

namespace Scumm {

void SaudChannel::handleStrk(Chunk &b) {
	int32 size = b.getSize();
	if (size != 14 && size != 10) {
		error("STRK has an invalid size : %d", size);
	}
}

void SaudChannel::handleSmrk(Chunk &b) {
	_markReached = true;
}

void SaudChannel::handleShdr(Chunk &b) {
	int32 size = b.getSize();
	if (size != 4)
		error("SHDR has an invalid size : %d", size);
}

bool SaudChannel::handleSubTags(int32 &offset) {
	if (_tbufferSize - offset >= 8) {
		Chunk::type type = READ_BE_UINT32(_tbuffer + offset);
		uint32 size = READ_BE_UINT32(_tbuffer + offset + 4);
		uint32 available_size = _tbufferSize - offset;

		switch (type) {
		case TYPE_STRK:
			_inData = false;
			if (available_size >= (size + 8)) {
				MemoryChunk c((byte *)_tbuffer + offset);
				handleStrk(c);
			} else
				return false;
			break;
		case TYPE_SMRK:
			_inData = false;
			if (available_size >= (size + 8)) {
				MemoryChunk c((byte *)_tbuffer + offset);
				handleSmrk(c);
			} else
				return false;
			break;
		case TYPE_SHDR:
			_inData = false;
			if (available_size >= (size + 8)) {
				MemoryChunk c((byte *)_tbuffer + offset);
				handleShdr(c);
			} else
				return false;
			break;
		case TYPE_SDAT:
			_inData = true;
			_dataSize = size;
			offset += 8;
			return false;
		default:
			error("unknown Chunk in SAUD track : %s ", tag2str(type));
		}
		offset += size + 8;
		return true;
	}
	return false;
}

SaudChannel::SaudChannel(int32 track) : SmushChannel(track),
	_nbframes(0),
	_markReached(false),
	_index(0),
	_keepSize(false) {
}

SaudChannel::~SaudChannel() {
	_dataSize = 0;
	_tbufferSize = 0;
	_sbufferSize = 0;
	_markReached = true;
	_sbuffer = 0;
}

bool SaudChannel::isTerminated() const {
	return (_markReached && _dataSize == 0 && _sbuffer == 0);
}

bool SaudChannel::setParameters(int32 nb, int32 flags, int32 volume, int32 pan, int32 index) {
	_nbframes = nb;
	_flags = flags; // bit 7 == IS_VOICE, bit 6 == IS_BACKGROUND_MUSIC, other ??
	_volume = volume;
	_pan = pan;
	_index = index;
	if (index != 0) {
		_dataSize = -2;
		_keepSize = true;
		_inData = true;
	}
	return true;
}

bool SaudChannel::checkParameters(int32 index, int32 nb, int32 flags, int32 volume, int32 pan) {
	if (++_index != index)
		error("invalid index in SaudChannel::checkParameters()");
	if (_nbframes != nb)
		error("invalid duration in SaudChannel::checkParameters()");
	if (_flags != flags)
		error("invalid flags in SaudChannel::checkParameters()");
	if (_volume != volume || _pan != pan) {
		_volume = volume;
		_pan = pan;
	}
	return true;
}

bool SaudChannel::appendData(Chunk &b, int32 size) {
	if (_dataSize == -1) {
		assert(size > 8);
		Chunk::type saud_type = b.getDword();
		saud_type = SWAP_BYTES_32(saud_type);
		uint32 saud_size = b.getDword();
		saud_size = SWAP_BYTES_32(saud_size);
		if (saud_type != TYPE_SAUD)
			error("Invalid Chunk for SaudChannel : %X", saud_type);
		size -= 8;
		_dataSize = -2;
	}
	if (_tbuffer) {
		byte *old = _tbuffer;
		_tbuffer = new byte[_tbufferSize + size];
		if (!_tbuffer)
			error("saud_channel failed to allocate memory");
		memcpy(_tbuffer, old, _tbufferSize);
		delete []old;
		b.read(_tbuffer + _tbufferSize, size);
		_tbufferSize += size;
	} else {
		_tbufferSize = size;
		_tbuffer = new byte[_tbufferSize];
		if (!_tbuffer)
			error("saud_channel failed to allocate memory");
		b.read(_tbuffer, _tbufferSize);
	}

	if (_keepSize) {
		_sbufferSize = _tbufferSize;
		_sbuffer = _tbuffer;
		_tbufferSize = 0;
		_tbuffer = 0;
	} else {
		processBuffer();
	}

	return true;
}

int32 SaudChannel::getAvailableSoundDataSize(void) const {
	return _sbufferSize;
}

void SaudChannel::getSoundData(int8 *snd, int32 size) {
	memcpy(snd, _sbuffer, size);
	if (!_keepSize)
		_dataSize -= size;
	delete []_sbuffer;
	_sbuffer = 0;
	_sbufferSize = 0;
}

} // End of namespace Scumm
