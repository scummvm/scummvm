/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <stdafx.h>
#include "channel.h"
#include "chunk.h"
#include "chunk_type.h"

#include <assert.h>
#include <string.h> // for memcpy.h
#ifndef min
#define min(x, y) ((x) > (y) ? (y) : (x))
#endif

void SaudChannel::handleStrk(Chunk & b) {
	int size = b.getSize();
	if(size != 14 && size != 10) {
		error("STRK has a invalid size : %d", size);
	}
}

void SaudChannel::handleSmrk(Chunk & b) {
	_markReached = true;
}

void SaudChannel::handleShdr(Chunk & b) {
	int size = b.getSize();
	if(size != 4) warning("SMRK has a invalid size : %d", size);
}

bool SaudChannel::handleSubTags(int & offset) {
	if(_tbufferSize - offset >= 8) {
		Chunk::type type = READ_BE_UINT32(_tbuffer + offset);
		unsigned int size = READ_BE_UINT32(_tbuffer + offset + 4);
		unsigned int available_size = _tbufferSize - offset;

		switch(type) {
			case TYPE_STRK:
				_inData = false;
				if(available_size >= (size + 8)) {
					ContChunk c((char*)_tbuffer + offset);
					handleStrk(c);
				}
				else
					return false;
				break;
			case TYPE_SMRK:
				_inData = false;
				if(available_size >= (size + 8)) {
					ContChunk c((char*)_tbuffer + offset);
					handleSmrk(c);
				}
				else
					return false;
				break;
			case TYPE_SHDR:
				_inData = false;
				if(available_size >= (size + 8)) {
					ContChunk c((char*)_tbuffer + offset);
					handleShdr(c);
				}
				else
					return false;
				break;
			case TYPE_SDAT: 
				_inData = true;
				_dataSize = size;
				offset += 8;
				return false;
			default:
				error("unknown Chunk in SAUD track : %s ", Chunk::ChunkString(type));
		}
		offset += size + 8;
		return true;
	}
	return false;
}

bool SaudChannel::processBuffer() {
	// At the start of this function, we have _tbuffer[0.._tbuffersize] containing possible data...
	// and _sbuffer is 0
	// At the end we have :
	// if(sound data) _sbuffer[0.._sbuffer_size] contains the sound data
	// the unprocessed data is kept in _tbuffer[0.._tbuffersize] (which may have changed)
	// if no unprocessed data, then _tbuffer is 0
	assert(_tbuffer != 0);
	assert(_tbufferSize != 0);
	assert(_sbuffer == 0);
	assert(_sbufferSize == 0);
	
	if(_inData) {
		if(_dataSize < _tbufferSize) {
			// I can't assume that the channel is finished after data is received... (this assumption failed in realride.san)
			int offset= _dataSize;
			while(handleSubTags(offset));
			_sbufferSize = _dataSize;
			_sbuffer = _tbuffer;
			if(offset < _tbufferSize) { // there is still some unprocessed data
				int new_size = _tbufferSize - offset;
				_tbuffer = new unsigned char[new_size];
				if(!_tbuffer)  error("SaudChannel failed to allocate memory");
				memcpy(_tbuffer, _sbuffer + offset, new_size);
				_tbufferSize = new_size;
			} else {
				_tbuffer = 0;
				_tbufferSize = 0;
			}
			if(_sbufferSize == 0) {
				// this never happened yet, but who knows
				delete []_sbuffer; 
				_sbuffer = 0;
			}
		} else {
			// easy, swap the buffer
			_sbufferSize = _tbufferSize;
			_sbuffer = _tbuffer;
			_tbufferSize = 0;
			_tbuffer = 0;
		}
	} else {
		int offset = 0;
		while(handleSubTags(offset));
		if(_inData) {
			_sbufferSize = _tbufferSize - offset;
			assert(_sbufferSize);
			_sbuffer = new unsigned char[_sbufferSize];
			if(!_sbuffer)  error("saud_channel failed to allocate memory");
			memcpy(_sbuffer, _tbuffer + offset, _sbufferSize);
			delete []_tbuffer;
			_tbuffer = 0;
			_tbufferSize = 0;
		} else {
			if(offset) { // maybe I should assert() this to avoid a lock...
				unsigned char * old = _tbuffer;
				int new_size = _tbufferSize - offset;
				_tbuffer = new unsigned char[new_size];
				if(!_tbuffer)  error("SaudChannel failed to allocate memory");
				memcpy(_tbuffer, old + offset, new_size);
				_tbufferSize = new_size;
				delete []old;
			}
		}
	}
	return true;
}

SaudChannel::SaudChannel(int track, int freq) : 
			_track(track), 
			_nbframes(0),
			_dataSize(-1),
			_frequency(freq),
			_inData(false),
			_markReached(false),
			_tbuffer(0),
			_tbufferSize(0),
			_sbuffer(0),
			_sbufferSize(0)
{
}

SaudChannel::~SaudChannel() {
	if(_tbuffer) delete []_tbuffer;
	if(_sbuffer) {
		warning("this should never happen !!!! (_sbuffer not NULL here)");
		delete []_sbuffer;
	}
}

bool SaudChannel::isTerminated() const {
	return (_markReached && _dataSize == 0 && _sbuffer == 0);
}

void SaudChannel::recalcVolumeTable() {
	const int MAX_BALANCE = 100;
	int volume_left, volume_right;
	if(_balance < -MAX_BALANCE || _balance > MAX_BALANCE) {
		error("balance is out of range ! : %d", _balance);
	}
	int left_multiplier = MAX_BALANCE - _balance;
	int right_multiplier = MAX_BALANCE + _balance;
	volume_left = _volume * left_multiplier / (MAX_BALANCE * 2);
	volume_right = _volume * right_multiplier / (MAX_BALANCE * 2);
	if(volume_left < 0) volume_left = 0;
	if(volume_left > 128) volume_left = 128;
	if(volume_right < 0) volume_right = 0;
	if(volume_right > 128) volume_right = 128;
	for(int i = 0; i < 256; i++) {
		int value = volume_left * (signed char)i;
		_voltable[0][i] = TO_BE_16(value);
		value = volume_right * (signed char)i;
		_voltable[1][i] = TO_BE_16(value);
	}
}

bool SaudChannel::setParameters(int nb, int flags, int volume, int balance) {
	_nbframes = nb;
	_flags = flags; // bit 7 == IS_VOICE, bit 6 == IS_BACKGROUND_MUSIC, other ??
	_volume = volume;
	_balance = balance;
	_index = 0;
	recalcVolumeTable();
	return true;
}

bool SaudChannel::checkParameters(int index, int nb, int flags, int volume, int balance) {
	if(++_index != index) error("invalid index in SaudChannel::checkParameters()");
	if(_nbframes != nb) error("invalid duration in SaudChannel::checkParameters()");
	if(_flags != flags) error("invalid flags in SaudChannel::checkParameters()");
	if(_volume != volume || _balance != balance) {
		_volume = volume;
		_balance = balance;
		recalcVolumeTable();
	}
	return true;
}

bool SaudChannel::appendData(Chunk & b, int size) {
	if(_dataSize == -1) { // First call
		assert(size > 8);
		Chunk::type saud_type = b.getDword(); saud_type = TO_BE_32(saud_type);
		unsigned int saud_size = b.getDword(); saud_size = TO_BE_32(saud_size);
		if(saud_type != TYPE_SAUD) error("Invalid Chunk for SaudChannel : %X", saud_type);
		size -= 8;
		_dataSize = -2; // We don't get here again...
	}
	if(_tbuffer) {
		unsigned char * old = _tbuffer;
		_tbuffer = new unsigned char[_tbufferSize + size];
		if(!_tbuffer)  error("saud_channel failed to allocate memory");
		memcpy(_tbuffer, old, _tbufferSize);
		delete []old;
		b.read(_tbuffer + _tbufferSize, size);
		_tbufferSize += size;
	} else {
		_tbufferSize = size;
		_tbuffer = new unsigned char[_tbufferSize];
		if(!_tbuffer)  error("saud_channel failed to allocate memory");
		b.read(_tbuffer, _tbufferSize);
	}
	return processBuffer();
}

int SaudChannel::availableSoundData(void) const {
	return _sbufferSize;
}

void SaudChannel::getSoundData(short * snd, int size) {
	for(int i = 0; i < size; i++) {
		snd[2 * i] = _voltable[0][_sbuffer[i] ^ 0x80];
		snd[2 * i + 1] = _voltable[1][_sbuffer[i] ^ 0x80];
	}
	_dataSize -= size;
	delete []_sbuffer;
	_sbuffer = 0;
	_sbufferSize = 0;
}
