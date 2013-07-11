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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#ifndef ZVISION_UTILITY_H
#define ZVISION_UTILITY_H

#include "common/str.h"
#include "common/file.h"
#include "common/array.h"

namespace ZVision {

/**
 * Opens the sourceFile utilizing Common::File (aka SearchMan) and writes the
 * contents to destFile. destFile is created in the working directory
 *
 * @param sourceFile    The 'file' you want the contents of
 * @param destFile      The name of the file where the content will be written to
 */
void writeFileContentsToFile(const Common::String &sourceFile, const Common::String &destFile);

/**
 * Removes any line comments using '#' as a sequence start.
 * Then removes any trailing and leading 'whitespace' using String::trim()
 * Note: String::trim uses isspace() to determine what is whitespace and what is not.
 *
 * @param string    The string to modify. It is modified in place
 */
void trimCommentsAndWhiteSpace(Common::String *string);

/**
 * Searches through all the .scr files and dumps 'numberOfExamplesPerType' examples of each type of ResultAction
 * ZVision::initialize() must have been called before this function can be used.
 *
 * @param destFile    Where to write the examples
 */
void dumpEveryResultAction(const Common::String &destFile);


		}
	}
}

} // End of namespace ZVision

#endif
