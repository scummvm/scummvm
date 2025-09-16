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


//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::getDirectoryName(const Common::String &path) {
	Common::String newPath = unifySeparators(path);

	size_t pos = newPath.findLastOf('/');

	if (pos == AnsiString::npos) {
		return "";
	} else {
		return newPath.substr(0, pos + 1);
	}
}

//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::getFileName(const Common::String &path) {
	Common::String newPath = unifySeparators(path);

	size_t pos = newPath.findLastOf('/');

	if (pos == Common::String::npos) {
		return path;
	} else {
		return newPath.substr(pos + 1);
	}
}

//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::getFileNameWithoutExtension(const Common::String &path) {
	Common::String fileName = getFileName(path);

	size_t pos = fileName.findLastOf('.');

	if (pos == Common::String::npos) {
		return fileName;
	} else {
		return fileName.substr(0, pos);
	}
}


//////////////////////////////////////////////////////////////////////////
Common::String PathUtil::getExtension(const Common::String &path) {
	Common::String fileName = getFileName(path);

	size_t pos = fileName.findLastOf('.');

	if (pos == Common::String::npos) {
		return "";
	} else {
		return fileName.substr(pos);
	}
}


} // End of namespace Wintermute
