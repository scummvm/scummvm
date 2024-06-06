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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/xlibs/xutil/xglobal.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

XConsole& XConsole::operator< (const char* v) {
	UpdateBuffer(v);
	return *this;
}

XConsole& XConsole::operator< (char v) {
	_ConvertBuffer[0] = v;
	_ConvertBuffer[1] = 0;
	UpdateBuffer(&_ConvertBuffer[0]);
	return *this;
}

XConsole& XConsole::operator< (unsigned char v) {
	_ConvertBuffer[0] = v;
	_ConvertBuffer[1] = 0;
	UpdateBuffer(&_ConvertBuffer[0]);
	return *this;
}

XConsole& XConsole::operator<= (short var) {
	Common::String s = Common::String::format("%hd", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (unsigned short var) {
	Common::String s = Common::String::format("%u", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (int var) {
	Common::String s = Common::String::format("%d", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (unsigned var) {
	Common::String s = Common::String::format("%u", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (long var) {
	Common::String s = Common::String::format("%ld", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (unsigned long var) {
	Common::String s = Common::String::format("%lu", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (float var) {
	char* s = gcvt(var, digits, _ConvertBuffer);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (double var) {
	char* s = gcvt(var, digits, _ConvertBuffer);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (long double var) {
	char* s = gcvt(var, digits, _ConvertBuffer);
	UpdateBuffer(s);
	return *this;
}
} // namespace QDEngine
