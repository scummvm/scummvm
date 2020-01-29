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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/fs/stdiostream.cpp $
 * $Id: stdiostream.cpp 53961 2010-10-30 21:27:42Z fingolfin $
 *
 */
//#include <assert.h>
#include "decoder/wave/stdiostream.h"

StdioStream::StdioStream(NuvieIOFileRead *handle, DisposeAfterUse::Flag disposeMemory) : _handle(handle), _disposeMemory(disposeMemory) {
	assert(handle);
}

StdioStream::~StdioStream() {
	_handle->close();
	if (_disposeMemory == DisposeAfterUse::YES)
		delete _handle;
}

bool StdioStream::err() const {
	return false; //FIXME what should we return here?
}

void StdioStream::clearErr() {
	//clearerr((FILE *)_handle);
}

bool StdioStream::eos() const {
	return _handle->is_end();
}

sint32 StdioStream::pos() const {
	return _handle->position();
}

sint32 StdioStream::size() const {
	return _handle->get_size();
}

bool StdioStream::seek(sint32 offs, int whence) {

	if (whence == SEEK_SET) {
		_handle->seek(offs);
	} else if (whence == SEEK_CUR) {
		uint32 pos = _handle->position();
		_handle->seek(pos + offs);
	}
	return true;
}

uint32 StdioStream::read(void *ptr, uint32 len) {
	return _handle->readToBuf((unsigned char *)ptr, len);
}

uint32 StdioStream::write(const void *ptr, uint32 len) {
	return false;
}

bool StdioStream::flush() {
	return true;
}

StdioStream *StdioStream::makeFromPath(const Std::string &path) {
	NuvieIOFileRead *niofr = new NuvieIOFileRead();

	if (niofr == NULL || niofr->open(path) == false)
		return NULL;

	return new StdioStream(niofr, DisposeAfterUse::YES);
}
