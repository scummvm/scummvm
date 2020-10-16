/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/streamdebug.h"
#include "common/debug.h"
#include "common/str.h"

namespace Common {

class MessageStream {
public:
	MessageStream() :
		ref(1), space(true) {

	}

	String msg;
	int ref;
	bool space;
	int level;
};

StreamDebug::StreamDebug(int level) :
	_stream(new MessageStream()) {
	_stream->level = level;
}

StreamDebug::StreamDebug(const StreamDebug &other) {
	*this = other;
}

StreamDebug::~StreamDebug() {
	if (--_stream->ref == 0) {
		debug(_stream->level, "%s", _stream->msg.c_str());
		delete _stream;
	}
}

StreamDebug &StreamDebug::space() {
	if (!_stream->space) {
		_stream->msg += String(" ");
		_stream->space = true;
	}
	return *this;
}

StreamDebug &StreamDebug::nospace() {
	_stream->space = false;
	return *this;
}

StreamDebug &StreamDebug::maybeSpace() {
	if (_stream->space) {
		_stream->msg += " ";
	}
	return *this;
}

StreamDebug &StreamDebug::operator<<(const String &str) {
	_stream->msg += str;
	return maybeSpace();
}

StreamDebug &StreamDebug::operator<<(const char *str) {
	_stream->msg += str;
	return maybeSpace();
}

StreamDebug &StreamDebug::operator<<(char str) {
	_stream->msg += str;
	return maybeSpace();
}

StreamDebug &StreamDebug::operator<<(int num) {
	_stream->msg += String::format("%d", num);
	return maybeSpace();
}

StreamDebug &StreamDebug::operator<<(unsigned int num) {
	_stream->msg += String::format("%d", num);
	return maybeSpace();
}

#if !defined(__DC__) && !defined(__DS__)
StreamDebug &StreamDebug::operator<<(double num) {
	_stream->msg += String::format("%g", num);
	return maybeSpace();
}
#endif

StreamDebug &StreamDebug::operator<<(float num) {
	_stream->msg += String::format("%g", num);
	return maybeSpace();
}

StreamDebug &StreamDebug::operator<<(bool value) {
	_stream->msg += (value ? "true" : "false");
	return maybeSpace();
}

StreamDebug &StreamDebug::operator<<(void *p) {
	_stream->msg += String::format("%p", p);
	return maybeSpace();
}

StreamDebug &StreamDebug::operator=(const StreamDebug &other) {
	_stream = other._stream;
	++_stream->ref;

	return *this;
}

}

Common::StreamDebug streamDbg(int level) {
	return Common::StreamDebug(level);
}
