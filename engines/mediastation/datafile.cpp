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

#include "common/file.h"

#include "mediastation/mediastation.h"
#include "mediastation/datafile.h"

namespace MediaStation {

Datafile::Datafile(const Common::Path &path) {
	openFile(path);
}

Datafile::~Datafile() {
	close();
}

bool Datafile::openFile(const Common::Path &path) {
	Common::File *file = new Common::File();
	if (path.empty() || !file->open(path)) {
		error("Datafile::openFile(): Error opening file %s", path.toString(Common::Path::kNativeSeparator).c_str());
		delete file;
		return false;
	}

	_path = path;
	_stream = file;
	return true;
}

void Datafile::close() {
	delete _stream;
	_stream = nullptr;
}

} // End of namespace MediaStation
