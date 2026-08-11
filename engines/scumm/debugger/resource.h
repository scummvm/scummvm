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

#ifndef SCUMM_EDITOR_RESOURCE_H
#define SCUMM_EDITOR_RESOURCE_H

#include "common/array.h"
#include "common/noncopyable.h"
#include "common/path.h"

namespace Scumm {

namespace Editor {

class File;

struct Block {
	uint32 offset;
	uint32 tag;
	uint32 size;
	int parent;
	Common::Array<int> children;
};

class Resource : public Common::NonCopyable {
private:
	int _version;
	Common::Array<File *> _files;
	Common::Array<Common::Array<Block>> _blocks;

	void buildBlocks(File &file, uint32 startOffset, uint32 endOffset, int parentIndex, Common::Array<Block> &blocks);
	bool isContainerBlock(uint32 tag) const;

public:
	Resource(int version);
	~Resource();

	void addFile(const Common::Path &path, byte encByte);
	int getFileCount() const;
	File *getFile(int index);
	const Common::Array<Block> &getBlocks(int index) const;
};

} // End of namespace Editor

} // End of namespace Scumm

#endif
