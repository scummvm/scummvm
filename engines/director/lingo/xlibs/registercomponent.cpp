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
#include "director/lingo/xlibs/registercomponent.h"

/*************************************
 *
 * USED IN:
 *
 * Star Trek: The Next Generation Interactive Technical Manual
 *************************************/

namespace Director {

const char *RegisterComponent::xlibName = "RegisterComponent";
const char *RegisterComponent::fileNames[] = {
	"RegisterComponent",
	nullptr
};

static BuiltinProto builtins[] = {
	{ "RegisterComponent",	RegisterComponent::b_RegisterComponent, 1, 2, 400, FBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void RegisterComponent::open(int type) {
	g_lingo->initBuiltIns(builtins);
}

void RegisterComponent::close(int type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void RegisterComponent::b_RegisterComponent(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

} // End of namespace Director
