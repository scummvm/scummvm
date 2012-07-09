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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BDynBuffer.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBDynBuffer::CBDynBuffer(CBGame *inGame, uint32 initSize, uint32 growBy): CBBase(inGame) {
	_buffer = NULL;
	_size = 0;
	_realSize = 0;

	_offset = 0;
	_initSize = initSize;
	_growBy = growBy;

	_initialized = false;
}


//////////////////////////////////////////////////////////////////////////
CBDynBuffer::~CBDynBuffer() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::cleanup() {
	if (_buffer) free(_buffer);
	_buffer = NULL;
	_size = 0;
	_realSize = 0;
	_offset = 0;
	_initialized = false;
}


//////////////////////////////////////////////////////////////////////////
uint32 CBDynBuffer::getSize() {
	return _size;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBDynBuffer::init(uint32 initSize) {
	cleanup();

	if (initSize == 0) initSize = _initSize;

	_buffer = (byte *)malloc(initSize);
	if (!_buffer) {
		Game->LOG(0, "CBDynBuffer::Init - Error allocating %d bytes", initSize);
		return STATUS_FAILED;
	}

	_realSize = initSize;
	_initialized = true;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBDynBuffer::putBytes(byte *buffer, uint32 size) {
	if (!_initialized) init();

	while (_offset + size > _realSize) {
		_realSize += _growBy;
		_buffer = (byte *)realloc(_buffer, _realSize);
		if (!_buffer) {
			Game->LOG(0, "CBDynBuffer::PutBytes - Error reallocating buffer to %d bytes", _realSize);
			return STATUS_FAILED;
		}
	}

	memcpy(_buffer + _offset, buffer, size);
	_offset += size;
	_size += size;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBDynBuffer::getBytes(byte *buffer, uint32 size) {
	if (!_initialized) init();

	if (_offset + size > _size) {
		Game->LOG(0, "CBDynBuffer::GetBytes - Buffer underflow");
		return STATUS_FAILED;
	}

	memcpy(buffer, _buffer + _offset, size);
	_offset += size;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::putDWORD(uint32 val) {
	putBytes((byte *)&val, sizeof(uint32));
}


//////////////////////////////////////////////////////////////////////////
uint32 CBDynBuffer::getDWORD() {
	uint32 ret;
	getBytes((byte *)&ret, sizeof(uint32));
	return ret;
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::putString(const char *val) {
	if (!val) putString("(null)");
	else {
		putDWORD(strlen(val) + 1);
		putBytes((byte *)val, strlen(val) + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBDynBuffer::getString() {
	uint32 len = getDWORD();
	char *ret = (char *)(_buffer + _offset);
	_offset += len;

	if (!strcmp(ret, "(null)")) return NULL;
	else return ret;
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::putText(const char *fmt, ...) {
	va_list va;

	va_start(va, fmt);
	putTextForm(fmt, va);
	va_end(va);

}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::putTextIndent(int indent, const char *fmt, ...) {
	va_list va;

	putText("%*s", indent, "");

	va_start(va, fmt);
	putTextForm(fmt, va);
	va_end(va);
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::putTextForm(const char *format, va_list argptr) {
	char buff[32768];
	vsprintf(buff, format, argptr);
	putBytes((byte *)buff, strlen(buff));
}

} // end of namespace WinterMute
