/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
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

Debug::Debug(int level) :
	_stream(new MessageStream()) {
	_stream->level = level;
}

Debug::Debug(const Debug &other) {
	*this = other;
}

Debug::~Debug() {
	if (--_stream->ref == 0) {
		debug(_stream->level, "%s", _stream->msg.c_str());
		delete _stream;
	}
}

Debug &Debug::space() {
	if (!_stream->space) {
		_stream->msg += String(" ");
		_stream->space = true;
	}
	return *this;
}

Debug &Debug::nospace() {
	_stream->space = false;
	return *this;
}

Debug &Debug::maybeSpace() {
	if (_stream->space) {
		_stream->msg += " ";
	}
	return *this;
}

Debug &Debug::operator<<(const String &str) {
	_stream->msg += str;
	return maybeSpace();
}

Debug &Debug::operator<<(const char *str) {
	_stream->msg += str;
	return maybeSpace();
}

Debug &Debug::operator<<(char str) {
	_stream->msg += str;
	return maybeSpace();
}

Debug &Debug::operator<<(int num) {
	_stream->msg += String::format("%d", num);
	return maybeSpace();
}

Debug &Debug::operator<<(unsigned int num) {
	_stream->msg += String::format("%d", num);
	return maybeSpace();
}

Debug &Debug::operator<<(double num) {
	_stream->msg += String::format("%g", num);
	return maybeSpace();
}

Debug &Debug::operator<<(float num) {
	_stream->msg += String::format("%g", num);
	return maybeSpace();
}

Debug &Debug::operator<<(bool value) {
	_stream->msg += (value ? "true" : "false");
	return maybeSpace();
}

Debug &Debug::operator<<(void *p) {
	_stream->msg += String::format("%p", p);
	return maybeSpace();
}

Debug &Debug::operator=(const Debug &other) {
	_stream = other._stream;
	++_stream->ref;

	return *this;
}

}

Common::Debug streamDbg(int level) {
	return Common::Debug(level);
}

