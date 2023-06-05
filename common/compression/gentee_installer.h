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

#ifndef COMMON_GENTEE_INSTALLER_H
#define COMMON_GENTEE_INSTALLER_H

#include "common/archive.h"

namespace Common {

// NOTE for future implementation:
// A Gentee Installer setup executable can be deployed with a package file (pak1.pak, etc.) or the
// data can be embedded in the setup executable.  If it's deployed with the executable, then you
// must read 12 bytes from offset 1008 in the executable file.  Of that, the first 4 bytes is the
// position of the embedded ginstall.dll compressed data, the next 4 are the compressed size of
// ginstall.dll, and the next 4 are the decompressed size of ginstall.dll.
//
// From that, compute the end position of the DLL data from the DLL position + the compressed size.
// If that position is less than the end of the file, then the package is embedded in the setup
// executable starting at that location.

/**
 * Loads a Gentee Installer package.
 *
 * @param stream          Data stream to load
 * @param prefixToRemove  Specifies the prefix of extract directives to include, and removes the prefix
 * @param threadSafe      If true, all read operations will be wrapped in a mutex-guarded substream
 *
 * @return The number of members added to list.
 */
Common::Archive *createGenteeInstallerArchive(Common::SeekableReadStream *stream, const char *prefixToRemove, bool threadSafe);

} // End of namespace Common

#endif
