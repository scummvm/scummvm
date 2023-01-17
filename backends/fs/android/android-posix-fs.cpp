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

#if defined(__ANDROID__)

#include "backends/fs/android/android-fs-factory.h"
#include "backends/fs/android/android-posix-fs.h"
#include "backends/fs/android/android-saf-fs.h"

AbstractFSNode *AndroidPOSIXFilesystemNode::makeNode() const {
	return new AndroidPOSIXFilesystemNode(_config);
}

AbstractFSNode *AndroidPOSIXFilesystemNode::makeNode(const Common::String &path) const {
	// If SAF works, it was a SAF URL
	AbstractFSNode *node = AndroidSAFFilesystemNode::makeFromPath(path);
	if (node) {
		return node;
	}

	return new AndroidPOSIXFilesystemNode(path, _config);
}

#endif
