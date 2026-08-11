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

#include "agi/agi.h"
#include "agi/loader.h"

#include "common/config-manager.h"
#include "common/fs.h"

namespace Agi {

void AgiLoader::getPotentialDiskImages(
	const char * const *imageExtensions,
	size_t imageExtensionCount,
	Common::Array<Common::Path> &imageFiles,
	FileMap &fileMap) {

	// get all files in game directory
	Common::FSList allFiles;
	Common::FSNode dir(ConfMan.getPath("path"));
	if (!dir.getChildren(allFiles, Common::FSNode::kListFilesOnly)) {
		warning("invalid game path: %s", dir.getPath().toString(Common::Path::kNativeSeparator).c_str());
		return;
	}

	// build array of files with provided disk image extensions
	for (const Common::FSNode &file : allFiles) {
		for (size_t i = 0; i < imageExtensionCount; i++) {
			if (file.getName().hasSuffixIgnoreCase(imageExtensions[i])) {
				Common::Path path = file.getPath();
				imageFiles.push_back(path);
				fileMap[path] = file;
				break;
			}
		}
	}

	// sort potential image files by name.
	// this is an important step for consistent results,
	// and because the first disk is likely to be first.
	Common::sort(imageFiles.begin(), imageFiles.end());
}

} // End of namespace Agi
