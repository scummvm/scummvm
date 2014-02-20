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

#ifndef ZVISION_UTILITY_H
#define ZVISION_UTILITY_H

#include "common/array.h"


namespace Common {
class String;
}

namespace ZVision {

class ZVision;

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

/**
 * Removes all duplicate entries from container. Relative order will be preserved.
 *
 * @param container    The Array to remove duplicate entries from
 */
template<class T>
void removeDuplicateEntries(Common::Array<T> &container) {
	// Length of modified array
	uint newLength = 1;
	uint j;

	for(uint i = 1; i < container.size(); i++) {
		for(j = 0; j < newLength; j++) {
			if (container[i] == container[j]) {
				break;
			}
		}

		// If none of the values in index[0..j] of container are the same as array[i],
		// then copy the current value to corresponding new position in array
		if (j == newLength) {
			container[newLength++] = container[i];
		}
	}

	// Actually remove the unneeded space
	while (container.size() < newLength) {
		container.pop_back();
	}
}

/**
 * Gets the name of the file (including extension). Forward or back slashes
 * are interpreted as directory changes
 *
 * @param fullPath    A full or partial path to the file. Ex: folderOne/folderTwo/file.txt
 * @return            The name of the file without any preceding directories. Ex: file.txt
 */
Common::String getFileName(const Common::String &fullPath);

/**
 * Converts a ZVision .RAW file to a .WAV
 * The .WAV will be created in the working directory and will overwrite any existing file
 *
 * @param inputFile     The path to the input .RAW file
 * @param outputFile    The name of the output .WAV file
 */
void convertRawToWav(const Common::String &inputFile, ZVision *engine, const Common::String &outputFile);

} // End of namespace ZVision

#endif
