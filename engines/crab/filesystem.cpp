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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "common/file.h"
#include "crab/filesystem.h"

namespace Crab {

Common::String cleansePath(const Common::String &path) {
	if (path.hasPrefix("res/"))
		return Common::String(&path.c_str()[4]);

	return path;
}

bool fileOpen(const Common::Path &path, char *&data) {
	if (path.empty())
		return false;

	if (data != nullptr)
		delete[] data;

	Common::File file;
	if (!file.open(cleansePath(path.toString()))) {
		warning("Unable to open file %s", path.toString().c_str());
		data = nullptr;
		return false;
	}

	// allocate data
	int64 len = file.size();
	data = new char[len + 1];
	data[len] = '\0';

	// read the file into data
	file.read(data, len);
	file.close();

	return true;
}

bool fileOpen(const Common::Path &path, Common::File *file) {
	if (path.empty())
		return false;

	if (file->isOpen())
		file->close();

	if (!file->open(cleansePath(path.toString()))) {
		warning("Unable to open file %s", path.toString().c_str());
		return false;
	}

	return true;
}

} // End of namespace Crab
