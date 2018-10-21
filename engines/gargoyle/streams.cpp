/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gargoyle/streams.h"
#include "gargoyle/windows.h"

namespace Gargoyle {

Stream::Stream(Streams *streams, bool readable, bool writable, uint32 rock, bool unicode) :
		_streams(streams), _readable(readable), _writable(writable), _readCount(0),
		_writeCount(0), _prev(nullptr), _next(nullptr), _rock(0) {
}

Stream::~Stream() {
	_streams->removeStream(this);
}

Stream *Stream::getNext(uint32 *rock) const {
	Stream *stream = _next;
	if (rock)
		*rock = stream ? stream->_rock : 0;
	return stream;
}

void Stream::fillResult(StreamResult *result) {
	if (result) {
		result->_readCount = _readCount;
		result->_writeCount = _writeCount;
	}
}

void Stream::close(StreamResult *result) {
	// Get the read/write totals
	fillResult(result);

	// Remove the stream
	delete this;
}

/*--------------------------------------------------------------------------*/

void WindowStream::close(StreamResult *result) {
	warning("cannot close window stream");
}

void WindowStream::writeChar(unsigned char ch) {

}

void WindowStream::writeCharUni(uint32 ch) {

}

/*--------------------------------------------------------------------------*/

MemoryStream::MemoryStream(Streams *streams, void *buf, size_t buflen, FileMode mode, uint32 rock, bool unicode) :
		Stream(streams, mode != filemode_Write, mode != filemode_Read, rock, unicode),
		_buf(buf), _buflen(buflen), _bufptr(buf) {
	assert(_buf && _buflen);
	assert(mode == filemode_Read || mode == filemode_Write || mode == filemode_ReadWrite);

	if (unicode)
		_bufend = (uint32 *)buf + buflen;
	else
		_bufend = (byte *)buf + buflen;
	_bufeof = mode == filemode_Write ? _buf : _bufend;
}

void MemoryStream::writeChar(unsigned char ch) {

}

void MemoryStream::writeCharUni(uint32 ch) {

}

/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

Streams::Streams(GargoyleEngine *engine) : _engine(engine), _streamList(nullptr), _currentStream(nullptr) {
}

Streams::~Streams() {
	while (_streamList)
		deleteStream(_streamList);
}

WindowStream *Streams::addWindowStream(Window *window) {
	WindowStream *stream = new WindowStream(this, window);
	addStream(stream);
	return stream;
}

MemoryStream *Streams::addMemoryStream(void *buf, size_t buflen, FileMode mode, uint32 rock, bool unicode) {
	MemoryStream *stream = new MemoryStream(this, buf, buflen, mode, rock, unicode);
	addStream(stream);
	return stream;
}

void Streams::addStream(Stream *stream) {
	stream->_next = _streamList;
	_streamList = stream;
	if (stream->_next)
		stream->_next->_prev = stream;
}

void Streams::removeStream(Stream *stream) {
	Stream *prev = stream->_prev;
	Stream *next = stream->_next;

	if (prev)
		prev->_next = next;
	else
		_streamList = next;
	if (next)
		next->_prev = prev;
}

Stream *Streams::getFirst(uint32 *rock) {
	if (rock)
		*rock = _streamList ? _streamList->_rock : 0;
	return _streamList;
}

} // End of namespace Gargoyle
