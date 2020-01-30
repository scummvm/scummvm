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

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/files/utils.h"
#include "ultima/nuvie/nuvie.h"
#include "common/file.h"

namespace Ultima {
namespace Nuvie {

using Std::string;

/*
 *  Open a file for input,
 *  trying the original name (lower case), and the upper case version
 *  of the name.
 *
 *  Output: 0 if couldn't open.
 */

bool openFile(Common::ReadStream *&in, const char *fname) {
    Common::File *f = new Common::File();
    Common::String filename(fname);

    if (f->open(filename)) {
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

bool fileExists(const char *fname) {
	return Common::File::exists(fname);
}

} // End of namespace Nuvie
} // End of namespace Ultima
