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
#include "chunck.h"
#include "chunck_type.h"

#include <assert.h>
#include <string.h> // for memcpy.h
#ifndef min
#define min(x, y) ((x) > (y) ? (y) : (x))
#endif

ImuseChannel::ImuseChannel(int track, int freq) : 
			_track(track), 
			_tbuffer(0), 
			_tbufferSize(0), 
			_sbuffer(0), 
			_sbufferSize(0), 
			_frequency(freq), 
			_dataSize(-1),
			_inData(false) {
}

ImuseChannel::~ImuseChannel() {
	if(_tbuffer) {
		delete []_tbuffer; 
	}
	if(_sbuffer) {
		warning("_sbuffer should be 0 !!!");
		delete []_sbuffer; 
	}
}

bool ImuseChannel::isTerminated() const {
	return (_dataSize <= 0 && _sbuffer == 0);
}

bool ImuseChannel::setParameters(int nbframes, int size, int unk1, int unk2) {
	return true;
}

bool ImuseChannel::checkParameters(int index, int nbframes, int size, int unk1, int unk2) {
	return true;
}

bool ImuseChannel::appendData(Chunck & b, int size) {
	if(_dataSize == -1) { // First call
		assert(size > 8);
		Chunck::type imus_type = b.getDword(); imus_type = TO_BE_32(imus_type);
		unsigned int imus_size = b.getDword(); imus_size = TO_BE_32(imus_size);
		if(imus_type != TYPE_iMUS) error("Invalid CHUNCK for imuse_channel");
		size -= 8;
		_tbufferSize = size;
		assert(_tbufferSize);
		_tbuffer = new unsigned char[_tbufferSize];
		if(!_tbuffer)  error("imuse_channel failed to allocate memory");
		b.read(_tbuffer, size);
		_dataSize = -2; // even if _in_data does not get set, this won't be called again
	} else {
		if(_tbuffer) { // remaining from last call
			unsigned char * old = _tbuffer;
			int new_size = size + _tbufferSize;
			_tbuffer = new unsigned char[new_size];
			if(!_tbuffer)  error("imuse_channel failed to allocate memory");
			memcpy(_tbuffer, old, _tbufferSize);
			delete []old;
			b.read(_tbuffer + _tbufferSize, size);
			_tbufferSize += size;
		} else {
			_tbufferSize = size;
			_tbuffer = new unsigned char[_tbufferSize];
			if(!_tbuffer)  error("imuse_channel failed to allocate memory");
			b.read(_tbuffer, size);
		}
	}
	return processBuffer();
}

bool ImuseChannel::handleFormat(Chunck & src) {
	if(src.getSize() != 20) error("invalid size for FRMT chunck");
	unsigned imuse_start = src.getDword();
	imuse_start = TO_BE_32(imuse_start);
	src.seek(4);
	_bitsize = src.getDword();
	_bitsize = TO_BE_32(_bitsize);
	_rate = src.getDword();
	_rate = TO_BE_32(_rate);
	_channels = src.getDword();
	_channels = TO_BE_32(_channels);
	assert(_channels == 1 || _channels == 2);
	return true;
}

bool ImuseChannel::handleText(Chunck & src) {
	return true;
}

bool ImuseChannel::handleRegion(Chunck & src) {
	if(src.getSize() != 8) error("invalid size for REGN chunck");
	return true;
}

bool ImuseChannel::handleStop(Chunck & src) {
	if(src.getSize() != 4) error("invalid size for STOP chunck");
	return true;
}

bool ImuseChannel::handleMap(Chunck & map) {
	while(!map.eof()) {
		Chunck * sub = map.subBlock();
		switch(sub->getType()) {
			case TYPE_FRMT:
				handleFormat(*sub);
				break;
			case TYPE_TEXT:
				handleText(*sub);
				break;
			case TYPE_REGN:
				handleRegion(*sub);
				break;
			case TYPE_STOP:
				handleStop(*sub);
				break;
			default:
				error("Unknown iMUS subchunck found : %s, %d", Chunck::ChunckString(sub->getType()), sub->getSize());
		}
		delete sub;
	}
	return true;
}

void ImuseChannel::decode() {
	int remaining_size = _sbufferSize % 3;
	if(remaining_size) {
		_srbufferSize -= remaining_size;
		assert(_inData);
		if(_tbuffer == 0) {
			_tbuffer = new unsigned char[remaining_size];
			memcpy(_tbuffer, _sbuffer + _sbufferSize - remaining_size, remaining_size);
			_tbufferSize = remaining_size;
			_sbufferSize -= remaining_size;
		} else {
			warning("impossible ! : %p, %d, %d, %p(%d), %p(%d, %d)",
				this, _dataSize, _inData, _tbuffer, _tbufferSize, _sbuffer, _sbufferSize, _srbufferSize);
			unsigned char * old = _tbuffer;
			int new_size = remaining_size + _tbufferSize;
			_tbuffer = new unsigned char[new_size];
			if(!_tbuffer)  error("imuse_channel failed to allocate memory");
			memcpy(_tbuffer, old, _tbufferSize);
			delete []old;
			memcpy(_tbuffer + _tbufferSize, _sbuffer + _sbufferSize - remaining_size, remaining_size);
			_tbufferSize += remaining_size;
		}
	}
	int loop_size = _sbufferSize / 3;
	int new_size = loop_size * 2;
	short * keep, * decoded;
	keep = decoded = new short[new_size];
	assert(keep);
	unsigned char * source = _sbuffer;
	while(loop_size--) {
		int v1 =  *source++;
		int v2 =  *source++;
		int v3 =  *source++;
		int value = (((v2 & 0x0f) << 12) | (v1 << 4)) - 0x8000;
		*decoded++ = (short)value;
		value = (((v2 & 0xf0) << 8) | (v3 << 4)) - 0x8000;		
		*decoded++ = (short)value;
	}
	delete []_sbuffer;
	_sbuffer = (unsigned char*)keep;
	_sbufferSize = new_size * sizeof(short);
}

bool ImuseChannel::handleSubTags(int & offset) {
	int available_size = _tbufferSize - offset;
	if(available_size >= 8) {
		Chunck::type type = READ_BE_UINT32(_tbuffer + offset);
		unsigned int size = READ_BE_UINT32(_tbuffer + offset + 4);
		switch(type) {
			case TYPE_MAP_: 
				_inData = false;
				if(available_size >= (size + 8)) {
					ContChunck c((char*)_tbuffer + offset);
					handleMap(c);
				}
				break;
			case TYPE_DATA: // Sound data !!!
				_inData = true;
				_dataSize = size;
				offset += 8;
				{
					int reqsize = 1;
					if(_channels == 2) reqsize *= 2;
					if(_bitsize == 16) reqsize *= 2;
					else if(_bitsize == 12) {
						if(reqsize > 1)
							reqsize = reqsize * 3 / 2;
						else reqsize = 3;
					}
					if((size % reqsize) != 0) {
						warning("Invalid iMUS sound data size : (%d %% %d) != 0, correcting...", size, reqsize);
						size += 3 - (size % reqsize);
					}
				}
				return false;
			default:
				error("unknown chunck in iMUS track : %s ", Chunck::ChunckString(type));
		}
		offset += size + 8;
		return true;
	}
	return false;
}

bool ImuseChannel::processBuffer() {
	// see comments in saud_channel::processBuffer for an explanation
	assert(_tbuffer != 0);
	assert(_tbufferSize != 0);
	assert(_sbuffer == 0);
	assert(_sbufferSize == 0);
	
	if(_inData) {
		if(_dataSize < _tbufferSize) {			
			int offset= _dataSize;
			while(handleSubTags(offset));
			_sbufferSize = _dataSize;
			_sbuffer = _tbuffer;
			if(offset < _tbufferSize) { // there is still some unprocessed data
				int new_size = _tbufferSize - offset;
				_tbuffer = new unsigned char[new_size];
				if(!_tbuffer)  error("imuse_channel failed to allocate memory");
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
			//~ unsigned char * old = _tbuffer;
			_sbufferSize = _tbufferSize - offset;
			assert(_sbufferSize);
			_sbuffer = new unsigned char[_sbufferSize];
			if(!_sbuffer)  error("imuse_channel failed to allocate memory");
			memcpy(_sbuffer, _tbuffer + offset, _sbufferSize);
			delete []_tbuffer;
			_tbuffer = 0;
			_tbufferSize = 0;
		} else {
			if(offset) { // maybe I should assert() this to avoid a lock...
				unsigned char * old = _tbuffer;
				int new_size = _tbufferSize - offset;
				_tbuffer = new unsigned char[new_size];
				if(!_tbuffer)  error("imuse_channel failed to allocate memory");
				memcpy(_tbuffer, old + offset, new_size);
				_tbufferSize = new_size;
				delete []old;
			}
		}
	}
	_srbufferSize = _sbufferSize;
	if(_sbuffer && _bitsize == 12) decode();
	return true;
}

int ImuseChannel::availableSoundData(void) const {
	int ret = _sbufferSize;
	if(_channels == 2) ret /= 2;
	if(_bitsize > 8) ret /= 2;
	return ret;
}

void ImuseChannel::getSoundData(short * snd, int size) {
	if(_dataSize <= 0 || _bitsize <= 8) error("invalid call to imuse_channel::read_sound_data()");
	if(_channels == 2) size *= 2;
	for(int i = 0; i < size; i++)
		snd[i] = READ_BE_UINT16(_sbuffer + 2 * i);
	delete []_sbuffer;
	assert(_sbufferSize == 2 * size);
	_sbuffer = 0;
	_sbufferSize = 0;
	_dataSize -= _srbufferSize;
}

void ImuseChannel::getSoundData(char * snd, int size) {
	if(_dataSize <= 0 || _bitsize > 8) error("invalid call to imuse_channel::read_sound_data()");
	if(_channels == 2) size *= 2;
	for(int i = 0; i < size; i++)
		snd[i] = _sbuffer[i];
	delete []_sbuffer;
	_sbuffer = 0;
	_sbufferSize = 0;
	_dataSize -= _srbufferSize;
}
