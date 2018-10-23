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
#include "gargoyle/conf.h"
#include "gargoyle/events.h"
#include "gargoyle/gargoyle.h"
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

void WindowStream::putChar(unsigned char ch) {
	if (!_writable)
		return;
	++_writeCount;

	if (_window->_lineRequest || _window->_lineRequestUni) {
		if (g_conf->_safeClicks && g_vm->_events->_forceClick) {
			_window->cancelLineEvent(nullptr);
			g_vm->_events->_forceClick = false;
		} else {
			warning("putChar: window has pending line request");
		}
	}

	_window->putCharUni(ch);
	if (_window->_echoStream)
		_window->_echoStream->putChar(ch);
}

void WindowStream::putCharUni(uint32 ch) {
	if (!_writable)
		return;
	++_writeCount;

	if (_window->_lineRequest || _window->_lineRequestUni) {
		if (g_conf->_safeClicks && g_vm->_events->_forceClick) {
			_window->cancelLineEvent(nullptr);
			g_vm->_events->_forceClick = false;
		} else {
			warning("putCharUni: window has pending line request");
		}
	}

	_window->putCharUni(ch);
	if (_window->_echoStream)
		_window->_echoStream->putCharUni(ch);
}

void WindowStream::putBuffer(const char *buf, size_t len) {
	if (!_writable)
		return;
	_writeCount += len;

	if (_window->_lineRequest || _window->_lineRequestUni) {
		if (g_conf->_safeClicks && g_vm->_events->_forceClick) {
			_window->cancelLineEvent(nullptr);
			g_vm->_events->_forceClick = false;
		} else {
			warning("putBuffer: window has pending line request");
		}
	}

	for (size_t lx = 0; lx < len; lx++, buf++)
		_window->putCharUni(*buf);
	if (_window->_echoStream)
		_window->_echoStream->putBuffer(buf, len);

}

void WindowStream::putBufferUni(const uint32 *buf, size_t len) {
	if (!_writable)
		return;
	_writeCount += len;

	if (_window->_lineRequest || _window->_lineRequestUni) {
		if (g_conf->_safeClicks && g_vm->_events->_forceClick) {
			_window->cancelLineEvent(nullptr);
			g_vm->_events->_forceClick = false;
		} else {
			warning("putBuffer: window has pending line request");
		}
	}

	for (size_t lx = 0; lx < len; lx++, buf++)
		_window->putCharUni(*buf);
	if (_window->_echoStream)
		_window->_echoStream->putBufferUni(buf, len);
}

/*--------------------------------------------------------------------------*/

MemoryStream::MemoryStream(Streams *streams, void *buf, size_t buflen, FileMode mode, uint32 rock, bool unicode) :
		Stream(streams, mode != filemode_Write, mode != filemode_Read, rock, unicode),
		_buf(buf), _bufLen(buflen), _bufPtr(buf) {
	assert(_buf && _bufLen);
	assert(mode == filemode_Read || mode == filemode_Write || mode == filemode_ReadWrite);

	if (unicode)
		_bufEnd = (uint32 *)buf + buflen;
	else
		_bufEnd = (byte *)buf + buflen;
	_bufEof = mode == filemode_Write ? _buf : _bufEnd;
}

void MemoryStream::putChar(unsigned char ch) {
	if (!_writable)
		return;
	++_writeCount;

	if (_bufPtr < _bufEnd) {
		if (_unicode) {
			*((glui32 *)_bufPtr) = ch;
			_bufPtr = ((glui32 *)_bufPtr) + 1;
		} else {
			*((unsigned char *)_bufPtr) = ch;
			_bufPtr = ((unsigned char *)_bufPtr) + 1;
		}

		if (_bufPtr > _bufEof)
			_bufEof = _bufPtr;
	}
}

void MemoryStream::putCharUni(uint32 ch) {
	if (!_writable)
		return;
	++_writeCount;

	if (_bufPtr < _bufEnd) {
		if (_unicode) {
			*((glui32 *)_bufPtr) = ch;
			_bufPtr = ((glui32 *)_bufPtr) + 1;
		} else {
			*((unsigned char *)_bufPtr) = (unsigned char)ch;
			_bufPtr = ((unsigned char *)_bufPtr) + 1;
		}
		if (_bufPtr > _bufEof)
			_bufEof = _bufPtr;
	}
}

void MemoryStream::putBuffer(const char *buf, size_t len) {
	size_t lx;

	if (!_writable)
		return;
	_writeCount += len;

	if (_bufPtr >= _bufEnd) {
		len = 0;
	} else {
		if (!_unicode) {
			unsigned char *bp = (unsigned char *)_bufPtr;
			if (bp + len > (unsigned char *)_bufEnd)
			{
				lx = (bp + len) - (unsigned char *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				memmove(bp, buf, len);
				bp += len;
				if (bp >(unsigned char *)_bufEof)
					_bufEof = bp;
			}
			_bufPtr = bp;
		} else {
			glui32 *bp = (glui32 *)_bufPtr;
			if (bp + len > (glui32 *)_bufEnd) {
				lx = (bp + len) - (glui32 *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				glui32 i;
				for (i = 0; i < len; i++)
					bp[i] = buf[i];
				bp += len;
				if (bp >(glui32 *)_bufEof)
					_bufEof = bp;
			}
			_bufPtr = bp;
		}
	}
}

void MemoryStream::putBufferUni(const uint32 *buf, size_t len) {
	size_t lx;

	if (!_writable)
		return;
	_writeCount += len;

	if (_bufPtr >= _bufEnd) {
		len = 0;
	} else {
		if (!_unicode) {
			unsigned char *bp = (unsigned char *)_bufPtr;
			if (bp + len > (unsigned char *)_bufEnd) {
				lx = (bp + len) - (unsigned char *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				glui32 i;
				for (i = 0; i < len; i++) {
					glui32 ch = buf[i];
					if (ch > 0xff)
						ch = '?';
					bp[i] = (unsigned char)ch;
				}
				bp += len;
				if (bp > (unsigned char *)_bufEof)
					_bufEof = bp;
			}
			_bufPtr = bp;
		} else {
			glui32 *bp = (glui32 *)_bufPtr;
			if (bp + len > (glui32 *)_bufEnd) {
				lx = (bp + len) - (glui32 *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				memmove(bp, buf, len * 4);
				bp += len;
				if (bp >(glui32 *)_bufEof)
					_bufEof = bp;
			}
			_bufPtr = bp;
		}
	}
}

/*--------------------------------------------------------------------------*/

FileStream::FileStream(Streams *streams, uint32 rock, bool unicode) :
	Stream(streams, true, false, rock, unicode), _lastOp(0), _textFile(false) {
	// TODO: Set up files
	_outFile = nullptr;
	_inFile = nullptr;
}

void FileStream::ensureOp(FileMode mode) {
	// No implementation
}

void FileStream::putChar(unsigned char ch) {
	if (!_writable)
		return;
	++_writeCount;

	ensureOp(filemode_Write);
	if (!_unicode) {
		_outFile->writeByte(ch);
	} else if (_textFile) {
		putCharUtf8((glui32)ch);
	} else {
		_outFile->writeUint32BE(ch);
	}

	_outFile->flush();
}

void FileStream::putCharUni(uint32 ch) {
	if (!_writable)
		return;
	++_writeCount;

	ensureOp(filemode_Write);
	if (!_unicode) {
		if (ch >= 0x100)
			ch = '?';
		_outFile->writeByte(ch);
	} else if (_textFile) {
		putCharUtf8(ch);
	} else {
		_outFile->writeUint32BE(ch);
	}

	_outFile->flush();
}

void FileStream::putBuffer(const char *buf, size_t len) {
	if (!_writable)
		return;
	_writeCount += len;

	ensureOp(filemode_Write);
	for (size_t lx = 0; lx < len; lx++) {
		unsigned char ch = ((unsigned char *)buf)[lx];
		if (!_unicode) {
			_outFile->writeByte(ch);
		} else if (_textFile) {
			putCharUtf8((glui32)ch);
		} else {
			_outFile->writeUint32BE(ch);
		}
	}

	_outFile->flush();
}

void FileStream::putBufferUni(const uint32 *buf, size_t len) {
	if (!_writable)
		return;
	_writeCount += len;


	ensureOp(filemode_Write);
	for (size_t lx = 0; lx<len; lx++) {
		glui32 ch = buf[lx];
		if (!_unicode) {
			if (ch >= 0x100)
				ch = '?';
			_outFile->writeByte(ch);
		} else if (_textFile) {
			putCharUtf8(ch);
		} else {
			_outFile->writeUint32BE(ch);
		}
	}

	_outFile->flush();
}

void FileStream::putCharUtf8(glui32 val) {
	if (val < 0x80) {
		_outFile->writeByte(val);
	} else if (val < 0x800) {
		_outFile->writeByte((0xC0 | ((val & 0x7C0) >> 6)));
		_outFile->writeByte((0x80 | (val & 0x03F)));
	} else if (val < 0x10000) {
		_outFile->writeByte((0xE0 | ((val & 0xF000) >> 12)));
		_outFile->writeByte((0x80 | ((val & 0x0FC0) >> 6)));
		_outFile->writeByte((0x80 | (val & 0x003F)));
	} else if (val < 0x200000) {
		_outFile->writeByte((0xF0 | ((val & 0x1C0000) >> 18)));
		_outFile->writeByte((0x80 | ((val & 0x03F000) >> 12)));
		_outFile->writeByte((0x80 | ((val & 0x000FC0) >> 6)));
		_outFile->writeByte((0x80 | (val & 0x00003F)));
	} else {
		_outFile->writeByte('?');
	}
}

/*--------------------------------------------------------------------------*/

Streams::Streams() : _streamList(nullptr), _currentStream(nullptr) {
}

Streams::~Streams() {
	while (_streamList)
		delete _streamList;
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

	// Remove the stream as the echo stream of any window
	for (Windows::iterator i = g_vm->_windows->begin(); i != g_vm->_windows->end(); ++i) {
		if ((*i)->_echoStream == stream)
			(*i)->_echoStream = nullptr;
	}
}

Stream *Streams::getFirst(uint32 *rock) {
	if (rock)
		*rock = _streamList ? _streamList->_rock : 0;
	return _streamList;
}

} // End of namespace Gargoyle
