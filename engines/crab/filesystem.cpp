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

#include "crab/filesystem.h"
#include "common/file.h"

namespace Crab {

bool FileOpen(const Common::Path &path, char *&data) {
	if (data != NULL)
		delete[] data;

	Common::File file;
	if (!file.open(path)) {
		warning("Unable to open file %s", path.toString().c_str());
		data = NULL;
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

#if 0
bool PathCompare(const path &p1, const path &p2)
{
	return last_write_time(p1) > last_write_time(p2);
}
#endif

} // End of namespace Crab
