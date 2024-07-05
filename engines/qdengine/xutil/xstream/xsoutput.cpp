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

#include "qdengine/qd_precomp.h"


namespace QDEngine {

XStream& XStream::operator> (char& v) {
	read(&v, (unsigned)sizeof(char));
	return *this;
}

XStream& XStream::operator> (unsigned char& v) {
	read(&v, (unsigned)sizeof(char));
	return *this;
}

XStream& XStream::operator> (short& v) {
	read(&v, (unsigned)sizeof(short));
	return *this;
}

XStream& XStream::operator> (unsigned short& v) {
	read(&v, (unsigned)sizeof(unsigned short));
	return *this;
}

XStream& XStream::operator> (int& v) {
	read(&v, (unsigned)sizeof(int));
	return *this;
}

XStream& XStream::operator> (unsigned& v) {
	read(&v, (unsigned)sizeof(unsigned));
	return *this;
}

XStream& XStream::operator> (long& v) {
	read(&v, (unsigned)sizeof(long));
	return *this;
}

XStream& XStream::operator> (unsigned long& v) {
	read(&v, (unsigned)sizeof(unsigned long));
	return *this;
}

XStream& XStream::operator> (double& v) {
	read(&v, (unsigned)sizeof(double));
	return *this;
}

XStream& XStream::operator> (float& v) {
	read(&v, (unsigned)sizeof(float));
	return *this;
}

XStream& XStream::operator> (long double& v) {
	read(&v, (unsigned)sizeof(long double));
	return *this;
}
} // namespace QDEngine
