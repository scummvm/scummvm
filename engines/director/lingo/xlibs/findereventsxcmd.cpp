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

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/findereventsxcmd.h"

/**************************************************
 *
 * USED IN:
 * puppetmotel
 *
 **************************************************/

namespace Director {

const char *const FinderEventsXCMD::xlibName = "FinderEvents";
const XlibFileDesc FinderEventsXCMD::fileNames[] = {
	{ "FinderEvents",	nullptr },
	{ nullptr,			nullptr },
};

static const BuiltinProto builtins[] = {
	{ "FinderEvents", FinderEventsXCMD::m_FinderEvents, -1, 0, 400, CBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void FinderEventsXCMD::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
}

void FinderEventsXCMD::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

XOBJSTUB(FinderEventsXCMD::m_FinderEvents, 0)

}
