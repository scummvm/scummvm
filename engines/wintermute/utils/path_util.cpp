/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "common/file.h"
#include "engines/wintermute/utils/path_util.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::unifySeparators(const Common::String &path) {
	Common::String newPath = path;

	for (uint32 i = 0; i < newPath.size(); i++) {
		if (newPath[i] == '\\') {
			newPath.setChar('/', i);
		}
	}

	return newPath;
}

//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::normalizeFileName(const Common::String &path) {
	Common::String newPath = unifySeparators(path);
	newPath.toLowercase();
	return newPath;
}

//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::combine(const Common::String &path1, const Common::String &path2) {
	Common::String newPath1 = unifySeparators(path1);
	Common::String newPath2 = unifySeparators(path2);

	if (!newPath1.hasSuffix("/") && !newPath2.hasPrefix("/")) {
		newPath1 += "/";
	}

	return newPath1 + newPath2;
}

bool PathUtil::hasTrailingSlash(const Common::String &path) {
	return (path.size() > 0 && path[path.size() - 1 ] == '/');
}

//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::getDirectoryName(const Common::String &path) {
	Common::String newPath = unifySeparators(path);
	Common::String filename = getFileName(path);
	if (hasTrailingSlash(newPath)) {
		return path;
	} else {
		return Common::String(path.c_str(), path.size() - filename.size());
	}
}

//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::getFileName(const Common::String &path) {
	Common::String newPath = unifySeparators(path);
	Common::String lastPart = Common::lastPathComponent(newPath, '/');
	if (hasTrailingSlash(newPath)) {
		return Common::String("");
	} else {
		return lastPart;
	}
}

//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::getFileNameWithoutExtension(const Common::String &path) {
	Common::String fileName = getFileName(path);
	// TODO: Prettify this.
	Common::String extension = Common::lastPathComponent(fileName, '.');
	for (uint32 i = 0; i < extension.size() + 1; i++) {
		fileName.deleteLastChar();
	}
	return fileName;
}

//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::getExtension(const Common::String &path) {
	Common::String fileName = getFileName(path);
	return Common::lastPathComponent(path, '.');
}

} // End of namespace Wintermute
