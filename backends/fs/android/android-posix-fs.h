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

#ifndef ANDROID_FILESYSTEM_ROOT_H
#define ANDROID_FILESYSTEM_ROOT_H

#include "backends/fs/posix-drives/posix-drives-fs.h"

#include "backends/fs/android/android-fs.h"

class AndroidPOSIXFilesystemNode : public DrivePOSIXFilesystemNode, public AndroidFSNode {
	// To let the factory redefine the displayed name
	friend class AndroidFilesystemFactory;
protected:
	AbstractFSNode *makeNode() const override;
	AbstractFSNode *makeNode(const Common::String &path) const override;

	bool remove() override;

public:
	AndroidPOSIXFilesystemNode(const Common::String &path, const Config &config)
		: DrivePOSIXFilesystemNode(path, config) { }
	AndroidPOSIXFilesystemNode(const Config &config)
		: DrivePOSIXFilesystemNode(config) { _path = "/"; }
};

#endif
