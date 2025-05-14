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

#ifndef ATARI_FILESYSTEM_H
#define ATARI_FILESYSTEM_H

#include "backends/fs/posix/posix-fs.h"

/**
 * Implementation of the ScummVM file system API based on POSIX with some Atari specific features.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class AtariFilesystemNode final : public POSIXFilesystemNode {
protected:
	AbstractFSNode *makeNode(const Common::String &path) const override {
		return new AtariFilesystemNode(path);
	}

public:
	AtariFilesystemNode(const Common::String &path)
		: POSIXFilesystemNode(path) {
	}
};

#endif
