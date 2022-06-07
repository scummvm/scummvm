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

#include "mm/mm1/messages.h"

namespace MM {
namespace MM1 {

InfoMessage::InfoMessage() : Message(), _callbackFn(nullptr) {}

InfoMessage::InfoMessage(const Common::String &str) :
		Message(), _callbackFn(nullptr) {
	_lines.push_back(str);
}

InfoMessage::InfoMessage(int x, int y, const Common::String &str,
		InfoKeyCallback callbackFn) :
		Message(), _callbackFn(callbackFn) {
	_lines.push_back(Line(x, y, str));
}

InfoMessage::InfoMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2,
		InfoKeyCallback callbackFn) :
		Message(), _callbackFn(callbackFn) {
	_lines.push_back(Line(x1, y1, str1));
	_lines.push_back(Line(x2, y2, str2));
}

} // namespace MM1
} // namespace MM
