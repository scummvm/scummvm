/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"
#include "common/error.h"
#include "common/macresman.h"
#include "common/stream.h"

#include "pegasus/MMShell/Utilities/MMResourceFile.h"

namespace Pegasus {

MMResourceFile::MMResourceFile() {
	_resFork = 0;
}

MMResourceFile::~MMResourceFile() {
	if (IsFileOpen())
		CloseResourceFile();
}

Common::Error MMResourceFile::UsePathname(const Common::String &str) {
	if (IsFileOpen())
		return Common::kUnknownError;

	_fileName = str;
	return Common::kNoError;
}

Common::Error MMResourceFile::OpenResourceFile() {
	if (IsFileOpen() || _fileName.empty())
		return Common::kUnknownError;

	_resFork = new Common::MacResManager();

	if (!_resFork->open(_fileName)) {
		delete _resFork; _resFork = 0;
		return Common::kReadingFailed;
	}

	return Common::kNoError;
}

Common::Error MMResourceFile::CloseResourceFile() {
	if (!IsFileOpen())
		return Common::kUnknownError;

	delete _resFork;
	_resFork = 0;
	return Common::kNoError;
}

Common::Error MMResourceFile::GetResource(const uint32 type, const tResIDType id, Common::SeekableReadStream *&h) {
	if (!IsFileOpen())
		return Common::kUnknownError;

	// Look how well our API fits in!
	h = _resFork->getResource(type, id);
	return Common::kNoError;
}

Common::Error MMResourceFile::GetResource(const uint32 type, const Common::String &name, Common::SeekableReadStream *&h) {
	if (!IsFileOpen())
		return Common::kUnknownError;

	// Look how well our API fits in!
	h = _resFork->getResource(type, name);
	return Common::kNoError;
}

Common::Error MMResourceFile::GetCIcon(const tResIDType id, Common::SeekableReadStream *&icon) {
	Common::Error err = GetResource(MKTAG('c', 'i', 'c', 'n'), id, icon);

	if (err.getCode() != Common::kNoError)
		return err;

	return (icon == 0) ? Common::kUnknownError : Common::kNoError;
}

bool MMResourceFile::IsFileOpen() const {
	return _resFork != 0;
}

} // End of namespace Pegasus