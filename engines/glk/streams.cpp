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

#include "glk/streams.h"
#include "glk/conf.h"
#include "glk/events.h"
#include "glk/glk.h"
#include "glk/windows.h"
#include "gui/saveload.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/translation.h"

namespace Glk {

Stream::Stream(Streams *streams, bool readable, bool writable, uint rock, bool unicode) :
		_streams(streams), _readable(readable), _writable(writable), _rock(0), _unicode(unicode),
		_readCount(0), _writeCount(0), _prev(nullptr), _next(nullptr) {
	if (g_vm->gli_register_obj)
		_dispRock = (*g_vm->gli_register_obj)(this, gidisp_Class_Stream);
}

Stream::~Stream() {
	_streams->removeStream(this);

	if (g_vm->gli_unregister_obj)
		(*g_vm->gli_unregister_obj)(this, gidisp_Class_Stream, _dispRock);
}

Stream *Stream::getNext(uint *rock) const {
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

void Stream::setZColors(uint fg, uint bg) {
	if (_writable && g_conf->_styleHint)
		Windows::_forceRedraw = true;
}

void Stream::setReverseVideo(bool reverse) {
	if (_writable && g_conf->_styleHint)
		Windows::_forceRedraw = true;
}

/*--------------------------------------------------------------------------*/

WindowStream::~WindowStream() {
	_window->_stream = nullptr;
}

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

void WindowStream::unputBuffer(const char *buf, size_t len) {
	uint lx;
	const char *cx;

	if (!_writable)
		return;

	if (_window->_lineRequest || _window->_lineRequestUni) {
		if (g_conf->_safeClicks && g_vm->_events->_forceClick) {
			_window->cancelLineEvent(nullptr);
			g_vm->_events->_forceClick = false;
		} else {
			warning("unput_buffer: window has pending line request");
			return;
		}
	}

	for (lx = 0, cx = buf + len - 1; lx < len; lx++, cx--) {
		if (!_window->unputCharUni(*cx))
			break;
		_writeCount--;
	}
	if (_window->_echoStream)
		_window->_echoStream->unputBuffer(buf, len);
}

void WindowStream::unputBufferUni(const uint32 *buf, size_t len) {
	uint lx;
	const uint32 *cx;

	if (!_writable)
		return;

	if (_window->_lineRequest || _window->_lineRequestUni) {
		if (g_conf->_safeClicks && g_vm->_events->_forceClick) {
			_window->cancelLineEvent(nullptr);
			g_vm->_events->_forceClick = false;
		} else {
			warning("unput_buffer: window has pending line request");
			return;
		}
	}

	for (lx = 0, cx = buf + len - 1; lx < len; lx++, cx--) {
		if (!_window->unputCharUni(*cx))
			break;
		_writeCount--;
	}

	if (_window->_echoStream)
		_window->_echoStream->unputBufferUni(buf, len);
}

void WindowStream::setStyle(uint val) {
	if (!_writable)
		return;

	if (val >= style_NUMSTYLES)
		val = 0;

	_window->_attr.style = val;
	if (_window->_echoStream)
		_window->_echoStream->setStyle(val);
}

void WindowStream::setHyperlink(uint linkVal) {
	if (_writable)
		_window->_attr.hyper = linkVal;
}

void WindowStream::setZColors(uint fg, uint bg) {
	if (!_writable || !g_conf->_styleHint)
		return;

	uint fore = fg, back = bg;

	if (fg != zcolor_Transparent && fg != zcolor_Cursor) {
		PropFontInfo *info = &g_conf->_propInfo;

		if (fg == zcolor_Default) {
			_window->_attr.fgset = false;
			_window->_attr.fgcolor = 0;
			Windows::_overrideFgSet = false;
			Windows::_overrideFgVal = 0;

			info->_moreColor = info->_moreSave;
			info->_caretColor = info->_caretSave;
			info->_linkColor = info->_linkSave;
		} else if (fg != zcolor_Current) {
			_window->_attr.fgset = true;
			_window->_attr.fgcolor = fg;
			Windows::_overrideFgSet = true;
			Windows::_overrideFgVal = fg;

			info->_moreColor = fore;
			info->_caretColor = fore;
			info->_linkColor = fore;
		}
	}

	if (/*bg != zcolor_Transparent &&*/ bg != zcolor_Cursor) {
		if (bg == zcolor_Default) {
			_window->_attr.bgset = false;
			_window->_attr.bgcolor = 0;
			Windows::_overrideBgSet = false;
			Windows::_overrideBgVal = 0;

			g_conf->_windowColor = g_conf->_windowSave;
			g_conf->_borderColor = g_conf->_borderSave;
		} else if (bg != zcolor_Current) {
			_window->_attr.bgset = true;
			_window->_attr.bgcolor = bg;
			Windows::_overrideBgSet = true;
			Windows::_overrideBgVal = bg;

			g_conf->_windowColor = back;
			g_conf->_borderColor = back;
		}
	}

	Windows::_overrideReverse = !(fg == zcolor_Default && bg == zcolor_Default);
	Windows::_forceRedraw = true;

	if (_window->_echoStream)
		_window->_echoStream->setZColors(fg, bg);
}

void WindowStream::setReverseVideo(bool reverse) {
	if (!_writable || !g_conf->_styleHint)
		return;

	_window->_attr.reverse = reverse;
	if (_window->_echoStream)
		_window->_echoStream->setReverseVideo(reverse);

	Windows::_forceRedraw = true;
}

/*--------------------------------------------------------------------------*/

MemoryStream::MemoryStream(Streams *streams, void *buf, size_t buflen, FileMode mode, uint rock, bool unicode) :
	Stream(streams, mode != filemode_Write, mode != filemode_Read, rock, unicode),
	_buf(buf), _bufLen(buflen), _bufPtr(buf) {
	assert(_buf || !_bufLen);
	assert(mode == filemode_Read || mode == filemode_Write || mode == filemode_ReadWrite);

	if (unicode)
		_bufEnd = (uint32 *)buf + buflen;
	else
		_bufEnd = (byte *)buf + buflen;
	_bufEof = mode == filemode_Write ? _buf : _bufEnd;

	if (g_vm->gli_register_arr)
		_arrayRock = (*g_vm->gli_register_arr)(buf, buflen, unicode ?  "&+#!Iu" : "&+#!Cn");
}

MemoryStream::~MemoryStream() {
	if (g_vm->gli_unregister_arr) {
		const char *typedesc = _unicode ? "&+#!Iu" : "&+#!Cn";
		(*g_vm->gli_unregister_arr)(_buf, _bufLen, typedesc, _arrayRock);
	}
}

void MemoryStream::putChar(unsigned char ch) {
	if (!_writable)
		return;
	++_writeCount;

	if (_bufPtr < _bufEnd) {
		if (_unicode) {
			*((uint32 *)_bufPtr) = ch;
			_bufPtr = ((uint32 *)_bufPtr) + 1;
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
			*((uint32 *)_bufPtr) = ch;
			_bufPtr = ((uint32 *)_bufPtr) + 1;
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
			if (bp + len > (unsigned char *)_bufEnd) {
				lx = (bp + len) - (unsigned char *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				memmove(bp, buf, len);
				bp += len;
				if (bp > (unsigned char *)_bufEof)
					_bufEof = bp;
			}
			_bufPtr = bp;
		} else {
			uint32 *bp = (uint32 *)_bufPtr;
			if (bp + len > (uint32 *)_bufEnd) {
				lx = (bp + len) - (uint32 *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				uint i;
				for (i = 0; i < len; i++)
					bp[i] = buf[i];
				bp += len;
				if (bp > (uint32 *)_bufEof)
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
				uint i;
				for (i = 0; i < len; i++) {
					uint32 ch = buf[i];
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
			uint32 *bp = (uint32 *)_bufPtr;
			if (bp + len > (uint32 *)_bufEnd) {
				lx = (bp + len) - (uint32 *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				memmove(bp, buf, len * 4);
				bp += len;
				if (bp > (uint32 *)_bufEof)
					_bufEof = bp;
			}
			_bufPtr = bp;
		}
	}
}

uint MemoryStream::getPosition() const {
	if (_unicode)
		return ((uint32 *)_bufPtr - (uint32 *)_buf);
	else
		return ((unsigned char *)_bufPtr - (unsigned char *)_buf);
}

void MemoryStream::setPosition(int pos, uint seekMode) {
	if (!_unicode) {
		if (seekMode == seekmode_Current)
			pos = ((unsigned char *)_bufPtr - (unsigned char *)_buf) + pos;
		else if (seekMode == seekmode_End)
			pos = ((unsigned char *)_bufEof - (unsigned char *)_buf) + pos;
		else {
			// pos = pos
		}

		if (pos < 0)
			pos = 0;
		if (pos > ((unsigned char *)_bufEof - (unsigned char *)_buf))
			pos = ((unsigned char *)_bufEof - (unsigned char *)_buf);
		_bufPtr = (unsigned char *)_buf + pos;
	} else {
		if (seekMode == seekmode_Current)
			pos = ((uint32 *)_bufPtr - (uint32 *)_buf) + pos;
		else if (seekMode == seekmode_End)
			pos = ((uint32 *)_bufEof - (uint32 *)_buf) + pos;

		if (pos < 0)
			pos = 0;
		if (pos > ((uint32 *)_bufEof - (uint32 *)_buf))
			pos = ((uint32 *)_bufEof - (uint32 *)_buf);
		_bufPtr = (uint32 *)_buf + pos;
	}
}

int MemoryStream::getChar() {
	if (!_readable)
		return -1;

	if (_bufPtr < _bufEnd) {
		if (!_unicode) {
			unsigned char ch;
			ch = *((unsigned char *)_bufPtr);
			_bufPtr = ((unsigned char *)_bufPtr) + 1;
			_readCount++;
			return ch;
		} else {
			uint32 ch;
			ch = *((uint32 *)_bufPtr);
			_bufPtr = ((uint32 *)_bufPtr) + 1;
			_readCount++;
			if (ch > 0xff)
				ch = '?';
			return ch;
		}
	} else {
		return -1;
	}
}

int MemoryStream::getCharUni() {
	if (!_readable)
		return -1;

	if (_bufPtr < _bufEnd) {
		if (!_unicode) {
			unsigned char ch;
			ch = *((unsigned char *)_bufPtr);
			_bufPtr = ((unsigned char *)_bufPtr) + 1;
			_readCount++;
			return ch;
		} else {
			uint32 ch;
			ch = *((uint32 *)_bufPtr);
			_bufPtr = ((uint32 *)_bufPtr) + 1;
			_readCount++;
			return ch;
		}
	} else {
		return -1;
	}
}

uint MemoryStream::getBuffer(char *buf, uint len) {
	if (!_readable)
		return 0;

	if (_bufPtr >= _bufEnd) {
		len = 0;
	} else {
		if (!_unicode) {
			unsigned char *bp = (unsigned char *)_bufPtr;
			if (bp + len > (unsigned char *)_bufEnd) {
				uint lx;
				lx = (bp + len) - (unsigned char *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}

			if (len) {
				memcpy(buf, bp, len);
				bp += len;
				if (bp > (unsigned char *)_bufEof)
					_bufEof = bp;
			}

			_readCount += len;
			_bufPtr = bp;
		} else {
			uint32 *bp = (uint32 *)_bufPtr;
			if (bp + len > (uint32 *)_bufEnd) {
				uint lx;
				lx = (bp + len) - (uint32 *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				uint i;
				for (i = 0; i < len; i++) {
					uint32 ch = *bp++;
					if (ch > 0xff)
						ch = '?';
					*buf++ = (char)ch;
				}
				if (bp > (uint32 *)_bufEof)
					_bufEof = bp;
			}

			_readCount += len;
			_bufPtr = bp;
		}
	}

	return len;
}

uint MemoryStream::getBufferUni(uint32 *buf, uint len) {
	if (!_readable)
		return 0;

	if (_bufPtr >= _bufEnd) {
		len = 0;
	} else {
		if (!_unicode) {
			unsigned char *bp = (unsigned char *)_bufPtr;
			if (bp + len > (unsigned char *)_bufEnd) {
				uint lx;
				lx = (bp + len) - (unsigned char *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				uint i;
				for (i = 0; i < len; i++)
					buf[i] = bp[i];
				bp += len;
				if (bp > (unsigned char *)_bufEof)
					_bufEof = bp;
			}
			_readCount += len;
			_bufPtr = bp;
		} else {
			uint32 *bp = (uint32 *)_bufPtr;
			if (bp + len > (uint32 *)_bufEnd) {
				uint lx;
				lx = (bp + len) - (uint32 *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
			if (len) {
				memcpy(buf, bp, len * 4);
				bp += len;
				if (bp > (uint32 *)_bufEof)
					_bufEof = bp;
			}
			_readCount += len;
			_bufPtr = bp;
		}
	}

	return len;
}

uint MemoryStream::getLine(char *buf, uint len) {
	uint lx;
	bool gotNewline;

	if (len == 0)
		return 0;

	len -= 1; // for the terminal null
	if (!_unicode) {
		if (_bufPtr >= _bufEnd) {
			len = 0;
		} else {
			if ((char *)_bufPtr + len > (char *)_bufEnd) {
				lx = ((char *)_bufPtr + len) - (char *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
		}

		gotNewline = false;
		for (lx = 0; lx < len && !gotNewline; lx++) {
			buf[lx] = ((char *)_bufPtr)[lx];
			gotNewline = (buf[lx] == '\n');
		}

		buf[lx] = '\0';
		_bufPtr = ((char *)_bufPtr) + lx;
	} else {
		if (_bufPtr >= _bufEnd) {
			len = 0;
		} else {
			if ((char *)_bufPtr + len > (char *)_bufEnd) {
				lx = ((char *)_bufPtr + len) - (char *)_bufEnd;
				if (lx < len)
					len -= lx;
				else
					len = 0;
			}
		}

		gotNewline = false;
		for (lx = 0; lx < len && !gotNewline; lx++) {
			uint32 ch;
			ch = ((uint32 *)_bufPtr)[lx];
			if (ch >= 0x100)
				ch = '?';
			buf[lx] = (char)ch;
			gotNewline = (ch == '\n');
		}

		buf[lx] = '\0';
		_bufPtr = ((uint32 *)_bufPtr) + lx;
	}

	_readCount += lx;
	return lx;
}

uint MemoryStream::getLineUni(uint32 *ubuf, uint len) {
	bool gotNewline;
	int lx;

	if (!_readable || len == 0)
		return 0;

	len -= 1; // for the terminal null
	if (!_unicode) {
		if (_bufPtr >= _bufEnd) {
			len = 0;
		} else {
			if ((char *)_bufPtr + len > (char *)_bufEnd) {
				lx = ((char *)_bufPtr + len) - (char *)_bufEnd;
				if (lx < (int)len)
					len -= lx;
				else
					len = 0;
			}
		}
		gotNewline = false;
		for (lx = 0; lx < (int)len && !gotNewline; lx++) {
			ubuf[lx] = ((unsigned char *)_bufPtr)[lx];
			gotNewline = (ubuf[lx] == '\n');
		}
		ubuf[lx] = '\0';
		_bufPtr = ((unsigned char *)_bufPtr) + lx;
	} else {
		if (_bufPtr >= _bufEnd) {
			len = 0;
		} else {
			if ((uint32 *)_bufPtr + len > (uint32 *)_bufEnd) {
				lx = ((uint32 *)_bufPtr + len) - (uint32 *)_bufEnd;
				if (lx < (int)len)
					len -= lx;
				else
					len = 0;
			}
		}
		gotNewline = false;
		for (lx = 0; lx < (int)len && !gotNewline; lx++) {
			uint32 ch;
			ch = ((uint32 *)_bufPtr)[lx];
			ubuf[lx] = ch;
			gotNewline = (ch == '\n');
		}
		ubuf[lx] = '\0';
		_bufPtr = ((uint32 *)_bufPtr) + lx;
	}

	_readCount += lx;
	return lx;
}

/*--------------------------------------------------------------------------*/

void IOStream::ensureOp(FileMode mode) {
	// No implementation
}

void IOStream::putChar(unsigned char ch) {
	if (!_writable)
		return;
	++_writeCount;

	ensureOp(filemode_Write);
	if (!_unicode) {
		_outStream->writeByte(ch);
	} else if (_textFile) {
		putCharUtf8((uint)ch);
	} else {
		_outStream->writeUint32BE(ch);
	}

	_outStream->flush();
}

void IOStream::putCharUni(uint32 ch) {
	if (!_writable)
		return;
	++_writeCount;

	ensureOp(filemode_Write);
	if (!_unicode) {
		if (ch >= 0x100)
			ch = '?';
		_outStream->writeByte(ch);
	} else if (_textFile) {
		putCharUtf8(ch);
	} else {
		_outStream->writeUint32BE(ch);
	}

	_outStream->flush();
}

void IOStream::putBuffer(const char *buf, size_t len) {
	if (!_writable)
		return;
	_writeCount += len;

	ensureOp(filemode_Write);
	for (size_t lx = 0; lx < len; lx++) {
		unsigned char ch = ((const unsigned char *)buf)[lx];
		if (!_unicode) {
			_outStream->writeByte(ch);
		} else if (_textFile) {
			putCharUtf8((uint)ch);
		} else {
			_outStream->writeUint32BE(ch);
		}
	}

	_outStream->flush();
}

void IOStream::putBufferUni(const uint32 *buf, size_t len) {
	if (!_writable)
		return;
	_writeCount += len;


	ensureOp(filemode_Write);
	for (size_t lx = 0; lx < len; lx++) {
		uint32 ch = buf[lx];
		if (!_unicode) {
			if (ch >= 0x100)
				ch = '?';
			_outStream->writeByte(ch);
		} else if (_textFile) {
			putCharUtf8(ch);
		} else {
			_outStream->writeUint32BE(ch);
		}
	}

	_outStream->flush();
}

void IOStream::putCharUtf8(uint val) {
	if (val < 0x80) {
		_outStream->writeByte(val);
	} else if (val < 0x800) {
		_outStream->writeByte((0xC0 | ((val & 0x7C0) >> 6)));
		_outStream->writeByte((0x80 | (val & 0x03F)));
	} else if (val < 0x10000) {
		_outStream->writeByte((0xE0 | ((val & 0xF000) >> 12)));
		_outStream->writeByte((0x80 | ((val & 0x0FC0) >> 6)));
		_outStream->writeByte((0x80 | (val & 0x003F)));
	} else if (val < 0x200000) {
		_outStream->writeByte((0xF0 | ((val & 0x1C0000) >> 18)));
		_outStream->writeByte((0x80 | ((val & 0x03F000) >> 12)));
		_outStream->writeByte((0x80 | ((val & 0x000FC0) >> 6)));
		_outStream->writeByte((0x80 | (val & 0x00003F)));
	} else {
		_outStream->writeByte('?');
	}
}

int IOStream::getCharUtf8() {
	uint res;
	uint val0, val1, val2, val3;

	if (_inStream->eos())
		return -1;
	val0 = _inStream->readByte();
	if (val0 < 0x80) {
		res = val0;
		return res;
	}

	if ((val0 & 0xe0) == 0xc0) {
		if (_inStream->eos()) {
			warning("incomplete two-byte character");
			return -1;
		}

		val1 = _inStream->readByte();
		if ((val1 & 0xc0) != 0x80) {
			warning("malformed two-byte character");
			return '?';
		}

		res = (val0 & 0x1f) << 6;
		res |= (val1 & 0x3f);
		return res;
	}

	if ((val0 & 0xf0) == 0xe0) {
		val1 = _inStream->readByte();
		val2 = _inStream->readByte();
		if (_inStream->eos()) {
			warning("incomplete three-byte character");
			return -1;
		}
		if ((val1 & 0xc0) != 0x80) {
			warning("malformed three-byte character");
			return '?';
		}
		if ((val2 & 0xc0) != 0x80) {
			warning("malformed three-byte character");
			return '?';
		}

		res = (((val0 & 0xf) << 12) & 0x0000f000);
		res |= (((val1 & 0x3f) << 6) & 0x00000fc0);
		res |= (((val2 & 0x3f)) & 0x0000003f);
		return res;
	}

	if ((val0 & 0xf0) == 0xf0) {
		if ((val0 & 0xf8) != 0xf0) {
			warning("malformed four-byte character");
			return '?';
		}

		val1 = _inStream->readByte();
		val2 = _inStream->readByte();
		val3 = _inStream->readByte();
		if (_inStream->eos()) {
			warning("incomplete four-byte character");
			return -1;
		}
		if ((val1 & 0xc0) != 0x80) {
			warning("malformed four-byte character");
			return '?';
		}
		if ((val2 & 0xc0) != 0x80) {
			warning("malformed four-byte character");
			return '?';
		}
		if ((val3 & 0xc0) != 0x80) {
			warning("malformed four-byte character");
			return '?';
		}

		res = (((val0 & 0x7) << 18) & 0x1c0000);
		res |= (((val1 & 0x3f) << 12) & 0x03f000);
		res |= (((val2 & 0x3f) << 6) & 0x000fc0);
		res |= (((val3 & 0x3f)) & 0x00003f);
		return res;
	}

	warning("malformed character");
	return '?';
}

uint IOStream::getPosition() const {
	return _outStream ? _outStream->pos() : _inStream->pos();
}

void IOStream::setPosition(int pos, uint seekMode) {
	_lastOp = 0;
	if (_unicode)
		pos *= 4;

	if (_inStream) {
		_inStream->seek(pos, SEEK_SET);
	} else {
		Common::SeekableWriteStream *ws =
			dynamic_cast<Common::SeekableWriteStream *>(_outStream);

		if (ws)
			ws->seek(pos, SEEK_SET);
		else
			error("seek not supported for writing files");
	}
}

int IOStream::getChar() {
	if (!_readable)
		return -1;

	ensureOp(filemode_Read);
	int res;
	if (!_unicode) {
		res = _inStream->eos() ? -1 : _inStream->readByte();
	} else if (_textFile) {
		res = getCharUtf8();
	} else {
		uint32 ch;
		res = _inStream->readByte();
		if (_inStream->eos())
			return -1;
		ch = (res & 0xFF);
		res = _inStream->readByte();
		if (_inStream->eos())
			return -1;
		ch = (ch << 8) | (res & 0xFF);
		res = _inStream->readByte();
		if (_inStream->eos())
			return -1;
		ch = (ch << 8) | (res & 0xFF);
		res = _inStream->readByte();
		if (_inStream->eos())
			return -1;
		ch = (ch << 8) | (res & 0xFF);
		res = ch;
	}
	if (res != -1) {
		_readCount++;
		if (res >= 0x100)
			return '?';
		return (int)res;
	} else {
		return -1;
	}
}

int IOStream::getCharUni() {
	if (!_readable)
		return -1;

	ensureOp(filemode_Read);
	int res;
	if (!_unicode) {
		res = _inStream->readByte();
	} else if (_textFile) {
		res = getCharUtf8();
	} else {
		uint32 ch;
		res = _inStream->readByte();
		if (res == -1)
			return -1;
		ch = (res & 0xFF);
		res = _inStream->readByte();
		if (res == -1)
			return -1;
		ch = (ch << 8) | (res & 0xFF);
		res = _inStream->readByte();
		if (res == -1)
			return -1;
		ch = (ch << 8) | (res & 0xFF);
		res = _inStream->readByte();
		if (res == -1)
			return -1;
		ch = (ch << 8) | (res & 0xFF);
		res = ch;
	}
	if (res != -1) {
		_readCount++;
		return (int)res;
	} else {
		return -1;
	}
}

uint IOStream::getBuffer(char *buf, uint len) {
	ensureOp(filemode_Read);
	if (!_unicode) {
		uint res;
		res = _inStream->read(buf, len);
		_readCount += res;
		return res;
	} else if (_textFile) {
		uint lx;
		for (lx = 0; lx < len; lx++) {
			uint32 ch;
			ch = getCharUtf8();
			if (ch == (uint)-1)
				break;
			_readCount++;
			if (ch >= 0x100)
				ch = '?';
			buf[lx] = (char)ch;
		}
		return lx;
	} else {
		uint lx;
		for (lx = 0; lx < len; lx++) {
			int res;
			uint32 ch;
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			_readCount++;
			if (ch >= 0x100)
				ch = '?';
			buf[lx] = (char)ch;
		}
		return lx;
	}
}

uint IOStream::getBufferUni(uint32 *buf, uint len) {
	if (!_readable)
		return 0;

	ensureOp(filemode_Read);
	if (!_unicode) {
		uint lx;
		for (lx = 0; lx < len; lx++) {
			int res;
			uint32 ch;
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (res & 0xFF);
			_readCount++;
			buf[lx] = ch;
		}
		return lx;
	} else if (_textFile) {
		uint lx;
		for (lx = 0; lx < len; lx++) {
			uint32 ch;
			ch = getCharUtf8();
			if (ch == (uint)-1)
				break;
			_readCount++;
			buf[lx] = ch;
		}
		return lx;
	} else {
		uint lx;
		for (lx = 0; lx < len; lx++) {
			int res;
			uint32 ch;
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			_readCount++;
			buf[lx] = ch;
		}
		return lx;
	}
}

uint IOStream::getLine(char *buf, uint len) {
	uint lx;
	bool gotNewline;

	if (len == 0)
		return 0;

	ensureOp(filemode_Read);
	if (!_unicode) {
		char *res = buf;
		for (; len > 0; ++res, --len) {
			*res = _inStream->readByte();
			if (*res == '\n')
				break;
		}
		*res = '\0';

		lx = strlen(buf);
		_readCount += lx;
		return lx;
	} else if (_textFile) {
		len -= 1; // for the terminal null
		gotNewline = false;
		for (lx = 0; lx < len && !gotNewline; lx++) {
			uint32 ch;
			ch = getCharUtf8();
			if (ch == (uint)-1)
				break;
			_readCount++;
			if (ch >= 0x100)
				ch = '?';
			buf[lx] = (char)ch;
			gotNewline = (ch == '\n');
		}
		buf[lx] = '\0';
		return lx;
	} else {
		len -= 1; // for the terminal null
		gotNewline = false;
		for (lx = 0; lx < len && !gotNewline; lx++) {
			int res;
			uint32 ch;
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			_readCount++;
			if (ch >= 0x100)
				ch = '?';
			buf[lx] = (char)ch;
			gotNewline = (ch == '\n');
		}

		buf[lx] = '\0';
		return lx;
	}
}

uint IOStream::getLineUni(uint32 *ubuf, uint len) {
	bool gotNewline;
	int lx;

	if (!_readable || len == 0)
		return 0;

	ensureOp(filemode_Read);
	if (!_unicode) {
		len -= 1; // for the terminal null
		gotNewline = false;
		for (lx = 0; lx < (int)len && !gotNewline; lx++) {
			int res;
			uint32 ch;
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (res & 0xFF);
			_readCount++;
			ubuf[lx] = ch;
			gotNewline = (ch == '\n');
		}
		ubuf[lx] = '\0';
		return lx;
	} else if (_textFile) {
		len -= 1; // for the terminal null
		gotNewline = false;
		for (lx = 0; lx < (int)len && !gotNewline; lx++) {
			uint32 ch;
			ch = getCharUtf8();
			if (ch == (uint)-1)
				break;
			_readCount++;
			ubuf[lx] = ch;
			gotNewline = (ch == '\n');
		}
		ubuf[lx] = '\0';
		return lx;
	} else {
		len -= 1; // for the terminal null
		gotNewline = false;
		for (lx = 0; lx < (int)len && !gotNewline; lx++) {
			int res;
			uint32 ch;
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			res = _inStream->readByte();
			if (res == -1)
				break;
			ch = (ch << 8) | (res & 0xFF);
			_readCount++;
			ubuf[lx] = ch;
			gotNewline = (ch == '\n');
		}
		ubuf[lx] = '\0';
		return lx;
	}
}

/*--------------------------------------------------------------------------*/

FileStream::FileStream(Streams *streams, frefid_t fref, uint fmode, uint rock, bool unicode) :
		IOStream(streams, fmode == filemode_Read, fmode != filemode_Read, rock, unicode),
		_inSave(nullptr), _outSave(nullptr) {
	
	_textFile = fref->_textMode;
	Common::String fname = fref->_slotNumber == -1 ? fref->_filename : fref->getSaveName();

	if (fmode == filemode_Write || fmode == filemode_ReadWrite || fmode == filemode_WriteAppend) {
		_outSave = g_system->getSavefileManager()->openForSaving(fname, false);
		if (!_outSave)
			error("Could open file for writing - %s", fname.c_str());
		setStream(_outSave);

	} else if (fmode == filemode_Read) {
		if (_file.open(fname)) {
			setStream(&_file);
		} else {
			_inSave = g_system->getSavefileManager()->openForLoading(fname);
			setStream(_inSave);

			if (!_inSave)
				error("Could not open for reading - %s", fname.c_str());
		}
	}
}

FileStream::~FileStream() {
	_file.close();
	delete _inSave;
	if (_outSave) {
		_outSave->finalize();
		delete _outSave;
	}
}

/*--------------------------------------------------------------------------*/

Streams::Streams() : _streamList(nullptr), _currentStream(nullptr) {
}

Streams::~Streams() {
	for (Stream *currStream = _streamList, *nextStream; currStream; currStream = nextStream) {
		nextStream = currStream->_next;
		delete currStream;
	}
}

FileStream *Streams::openFileStream(frefid_t fref, uint fmode, uint rock, bool unicode) {
	FileStream *stream = new FileStream(this, fref, fmode, rock, unicode);
	addStream(stream);
	return stream;
}

IOStream *Streams::openStream(Common::SeekableReadStream *rs, uint rock) {
	IOStream *stream = new IOStream(this, rs, rock);
	addStream(stream);
	return stream;
}

IOStream *Streams::openStream(Common::WriteStream *ws, uint rock) {
	IOStream *stream = new IOStream(this, ws, rock);
	addStream(stream);
	return stream;
}

WindowStream *Streams::openWindowStream(Window *window) {
	WindowStream *stream = new WindowStream(this, window);
	addStream(stream);
	return stream;
}

MemoryStream *Streams::openMemoryStream(void *buf, size_t buflen, FileMode mode, uint rock, bool unicode) {
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

	if (_currentStream == stream)
		_currentStream = nullptr; 
}

Stream *Streams::getFirst(uint *rock) {
	if (rock)
		*rock = _streamList ? _streamList->_rock : 0;
	return _streamList;
}


frefid_t Streams::createByPrompt(uint usage, FileMode fmode, uint rock) {
	switch (usage & fileusage_TypeMask) {
	case fileusage_SavedGame: {
		if (fmode == filemode_Write) {
			// Select a savegame slot
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

			int slot = dialog->runModalWithCurrentTarget();
			if (slot >= 0) {
				Common::String desc = dialog->getResultString();
				return createRef(slot, desc, usage, rock);
			}
		} else if (fmode == filemode_Read) {
			// Load a savegame slot
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);

			int slot = dialog->runModalWithCurrentTarget();
			if (slot >= 0) {
				return createRef(slot, "", usage, rock);
			}
		} else {
			error("Unsupport file mode");
		}
		break;
	}

	case fileusage_Transcript:
		return createRef("transcript.txt", fmode, rock);

	default:
		error("Unsupport file mode");
		break;
	}

	return nullptr;
}

frefid_t Streams::createRef(int slot, const Common::String &desc, uint usage, uint rock) {
	frefid_t fref = new FileReference();
	fref->_slotNumber = slot;
	fref->_description = desc;
	fref->_textMode = ((usage & fileusage_TextMode) != 0);
	fref->_fileType = (FileUsage)(usage & fileusage_TypeMask);

	_fileReferences.push_back(FileRefArray::value_type(fref));
	return fref;
}

frefid_t Streams::createRef(const Common::String &filename, uint usage, uint rock) {
	frefid_t fref = new FileReference();
	fref->_filename = filename;
	fref->_textMode = ((usage & fileusage_TextMode) != 0);
	fref->_fileType = (FileUsage)(usage & fileusage_TypeMask);

	_fileReferences.push_back(FileRefArray::value_type(fref));
	return fref;
}

frefid_t Streams::createTemp(uint usage, uint rock) {
	return createRef(Common::String::format("%s.tmp", g_vm->getTargetName().c_str()),
	                 usage, rock);
}

frefid_t Streams::createFromRef(frefid_t fref, uint usage, uint rock) {
	return createRef(fref->_filename, usage, rock);
}

void Streams::deleteRef(frefid_t fref) {
	for (uint idx = 0; idx < _fileReferences.size(); ++idx) {
		if (_fileReferences[idx].get() == fref) {
			_fileReferences.remove_at(idx);
			return;
		}
	}
}

frefid_t Streams::iterate(frefid_t fref, uint *rock) {
	// Find reference following the specified one
	int index = -1;
	for (uint idx = 0; idx < _fileReferences.size(); ++idx) {
		if (fref == nullptr || _fileReferences[idx].get() == fref) {
			if (idx < (_fileReferences.size() - 1))
				index = idx + 1;
			break;
		}
	}

	if (index != -1) {
		if (rock)
			*rock = _fileReferences[index].get()->_rock;
		return _fileReferences[index].get();
	}

	if (rock)
		*rock = 0;
	return nullptr;
}

/*--------------------------------------------------------------------------*/

FileReference::FileReference() : _rock(0), _slotNumber(-1), _fileType(fileusage_Data), _textMode(false) {
	if (g_vm->gli_register_obj)
		_dispRock = (*g_vm->gli_register_obj)(this, gidisp_Class_Fileref);
}

FileReference::FileReference(int slot, const Common::String &desc, uint usage, uint rock) :
		_rock(rock), _slotNumber(slot), _description(desc),
		_fileType((FileUsage)(usage & fileusage_TypeMask)), _textMode(usage & fileusage_TextMode) {
	if (g_vm->gli_register_obj)
		_dispRock = (*g_vm->gli_register_obj)(this, gidisp_Class_Fileref);
}

FileReference::~FileReference() {
	if (g_vm->gli_unregister_obj)
		(*g_vm->gli_unregister_obj)(this, gidisp_Class_Fileref, _dispRock);
}

const Common::String FileReference::getSaveName() const {
	assert(_slotNumber != -1);
	return g_vm->getSaveName(_slotNumber);
}

bool FileReference::exists() const {
	Common::String filename;

	if (_slotNumber == -1) {
		if (Common::File::exists(_filename))
			return true;
		filename = _filename;
	} else {
		filename = getSaveName();
	}

	// Check for a savegame (or other file in the save folder) with that name
	Common::InSaveFile *inSave = g_system->getSavefileManager()->openForLoading(filename);
	bool result = inSave != nullptr;
	delete inSave;
	return result;
}

void FileReference::deleteFile() {
	Common::String filename = (_slotNumber == -1) ? _filename : getSaveName();
	g_system->getSavefileManager()->removeSavefile(filename);
}

} // End of namespace Glk
