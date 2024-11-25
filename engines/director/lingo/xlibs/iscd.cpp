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

/*************************************
 *
 * USED IN:
 * Secrets of the Pyramids (Mac)
 *
 *************************************/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/iscd.h"


namespace Director {

const char *const IsCD::xlibName = "isCD";
const XlibFileDesc IsCD::fileNames[] = {
	{ "isCD",	nullptr },
	{ nullptr,	nullptr },
};

static const BuiltinProto builtins[] = {
	{ "isCD", IsCD::m_iscd, 1, 1, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void IsCD::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
}

void IsCD::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void IsCD::m_iscd(int nargs) {
	// -1: running from hard drive
	//  0: running on CD
	//  1: no CD
	//  2: disk name with the label
	//  3: multiple CDs with the label
	Common::String disk_label = g_lingo->pop().asString();
	g_lingo->push(Datum(0));
}

} // End of namespace Director
