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
 * Yearn2Learn: The Flintstones Coloring Book
 *
 *************************************/

/*
 * © 1989,1990 Apple Computer, Inc., v.1.1, by Anup Murarka
 * FileExists(pathname «, “noDialog”:errGlobal»)
 */

#include "common/file.h"
#include "common/savefile.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/fileexists.h"


namespace Director {

const char *const FileExists::xlibName = "FileExists";
const XlibFileDesc FileExists::fileNames[] = {
	{ "FileExists",	nullptr },
	{ nullptr,		nullptr },
};

static const BuiltinProto builtins[] = {
	{ "FileExists", FileExists::m_fileexists, 1, 1, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void FileExists::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
}

void FileExists::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void FileExists::m_fileexists(int nargs) {
	// This is mostly copied from FileIO::m_new
	Common::SaveFileManager *saves = g_system->getSavefileManager();
	Common::String path = g_lingo->pop().asString();
	Common::String origpath = path;

	if (!path.hasSuffixIgnoreCase(".txt")) {
		path += ".txt";
	}
	Common::String filename = lastPathComponent(path, g_director->_dirSeparator);
	if (!(saves->exists(filename))) {
		Common::File file;
		Common::Path location = findPath(origpath);
		if (location.empty() || !file.open(location)) {
			g_lingo->push(Datum(false));
			return;
		}
		g_lingo->push(Datum(true));
	}
}

} // End of namespace Director
