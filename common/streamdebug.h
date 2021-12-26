/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_STREAMDEBUG_H
#define COMMON_STREAMDEBUG_H

namespace Common {

class String;
class MessageStream;

class StreamDebug {
public:
	StreamDebug(int level);
	StreamDebug(const StreamDebug &other);
	~StreamDebug();

	StreamDebug &space();
	StreamDebug &nospace();

	StreamDebug &operator<<(const String &str);
	StreamDebug &operator<<(const char *str);
	StreamDebug &operator<<(char str);
	StreamDebug &operator<<(int number);
	StreamDebug &operator<<(unsigned int number);

	// DC and DS has float and double equal
#if !defined(__DC__) && !defined(__DS__)
	StreamDebug &operator<<(double number);
#endif
	StreamDebug &operator<<(float number);
	StreamDebug &operator<<(bool value);
	StreamDebug &operator<<(void *p);

	StreamDebug &operator=(const StreamDebug &other);

private:
	StreamDebug &maybeSpace();

	MessageStream *_stream;
};

}

Common::StreamDebug streamDbg(int level = -1);

#endif
