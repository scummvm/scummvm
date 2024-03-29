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

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/files/utils.h"
#include "ultima/nuvie/nuvie.h"
#include "common/file.h"

namespace Ultima {
namespace Nuvie {

/*
 *  Open a file for input,
 *  trying the original name (lower case), and the upper case version
 *  of the name.
 *
 *  Output: 0 if couldn't open.
 */

bool openFile(Common::ReadStream *&in, const Common::Path &fname) {
	Common::File *f = new Common::File();
	if (f->open(fname)) {
	    in = f;
	    return true;
	} else {
	    delete f;
	    return false;
	}
}

/*
 *  See if a file exists.
 */

bool fileExists(const Common::Path &fname) {
	return Common::File::exists(fname);
}

} // End of namespace Nuvie
} // End of namespace Ultima
