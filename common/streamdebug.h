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

#ifndef COMMON_STREAMDEBUG_H
#define COMMON_STREAMDEBUG_H

namespace Common {

class String;
class MessageStream;

class Debug {
public:
	Debug(int level);
	Debug(const Debug &other);
	~Debug();

	Debug &space();
	Debug &nospace();

	Debug &operator<<(const String &str);
	Debug &operator<<(const char *str);
	Debug &operator<<(char str);
	Debug &operator<<(int number);
	Debug &operator<<(unsigned int number);
	Debug &operator<<(double number);
	Debug &operator<<(float number);
	Debug &operator<<(bool value);
	Debug &operator<<(void *p);

	Debug &operator=(const Debug &other);

private:
	Debug &maybeSpace();

	MessageStream *_stream;
};

}

Common::Debug streamDbg(int level = -1);

#endif
