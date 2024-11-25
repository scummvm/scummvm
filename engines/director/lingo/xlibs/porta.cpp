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

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/porta.h"

/*************************************
 *
 * XCMD: Porta
 *
 * USED IN:
 * the7colors: The Seven Colors: Legend of PSY・S City
 *
 * Used in Piano mini-game
 *
 *************************************/

namespace Director {

const char *const Porta::xlibName = "Porta";
const XlibFileDesc Porta::fileNames[] = {
	{ "Porta",			nullptr },
	{ "PortaXCMD.rsrc",	nullptr },
	{ nullptr,			nullptr },
};

static const BuiltinProto builtins[] = {
	{ "Porta",	Porta::b_porta, 1, 1, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void Porta::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
}

void Porta::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void Porta::b_porta(int nargs) {
	int mode = g_lingo->pop().asInt();

	debug(5, "LB::b_porta: mode: %d", mode);
}

} // End of namespace Director
