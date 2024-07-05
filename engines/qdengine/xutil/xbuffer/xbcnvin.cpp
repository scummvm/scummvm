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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/str.h"

#include "qdengine/qd_precomp.h"

namespace QDEngine {

XBuffer& XBuffer::operator<= (const char var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (unsigned char var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (short var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (unsigned short var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (int var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (unsigned var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (long var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (unsigned long var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (float var) {
	char* s = gcvt(var, digits_, ConvertBuffer_);
	write(s, strlen(s), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (double var) {
	char* s = gcvt(var, digits_, ConvertBuffer_);
	write(s, strlen(s), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (long double var) {
	char* s = gcvt(var, digits_, ConvertBuffer_);
	write(s, strlen(s), 0);
	return *this;
}
} // namespace QDEngine
