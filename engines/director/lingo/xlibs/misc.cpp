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
 * Around the World With Willy Wabbit
 * Willy Wabbit Math Adventure
 *
 *************************************/

#include "common/formats/ini-file.h"
#include "director/director.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/misc.h"

namespace Director {

const char *const Misc::xlibName = "misc";
const XlibFileDesc Misc::fileNames[] = {
    { "misc",   nullptr },
    { nullptr,  nullptr },
};

static const MethodProto xlibMethods[] = {
    { "getProfileString",    Misc::m_getProfileString,   3, 3,  400 },
    { "isFilePresent",       Misc::m_isFilePresent,      1, 1,  400 },
    { nullptr, nullptr, 0, 0, 0 }
};

void Misc::open(ObjectType type, const Common::Path &path) {
    if (type == kXObj) {
        MiscObject::initMethods(xlibMethods);
        MiscObject *xobj = new MiscObject(kXObj);
        g_lingo->exposeXObject(xlibName, xobj);
    }
}

void Misc::close(ObjectType type) {
    if (type == kXObj) {
        MiscObject::cleanupMethods();
        g_lingo->_globalvars[xlibName] = Datum();
    }
}

MiscObject::MiscObject(ObjectType ObjectType) :Object<MiscObject>("Misc") {
    _objType = ObjectType;
}

void Misc::m_getProfileString(int nargs) {
    Common::String file = g_lingo->pop().asString();
    Common::String entry = g_lingo->pop().asString();
    Common::String section = g_lingo->pop().asString();

    Common::Path filePath = findPath(file);
    if (filePath.empty()) {
        warning("Unable to locate config file %s", file.c_str());
        g_lingo->push(Datum(""));
        return;
    }

    Common::INIFile config;
    config.loadFromFile(filePath);

    Common::String value;
    if (config.getKey(entry, section, value)) {
        g_lingo->push(Datum(value));
    } else {
        warning("Unable to fetch %s:%s; returning default", section.c_str(), entry.c_str());
        g_lingo->push(Datum(""));
    }
}

void Misc::m_isFilePresent(int nargs) {
    Common::String filename = g_lingo->pop().asString();
    Common::Path filePath = findMoviePath(filename);
    if (filePath.empty()) {
        g_lingo->push(Datum(0));
    } else {
        g_lingo->push(Datum(1));
    }
}

} // End of namespace Director
