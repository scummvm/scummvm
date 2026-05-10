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

#include "scumm/debugger/file.h"
#include "scumm/debugger/resource.h"

namespace Scumm {

namespace Editor {

static const int headerSize = 8;

Resource::Resource(int version)
	: _version(version) {
	if (_version < 5)
		warning("Editor: SCUMM v%d resources not supported", _version);
}

Resource::~Resource() {
	for (uint i = 0; i < _files.size(); ++i)
		delete _files[i];
}

void Resource::buildBlocks(File &file, uint32 startOffset, uint32 endOffset, int parentIndex, Common::Array<Block> &blocks) {
	// Parse provided range
	uint32 offset = startOffset;
	while (offset + headerSize <= endOffset) {
		file.seek(offset);
		uint32 tag = file.readUint32BE();
		uint32 size = file.readUint32BE();

		// Break if data is exhausted
		if (size < headerSize || offset + size > endOffset)
			break;

		// Push block
		int blockIndex = blocks.size();
		Block block;
		block.offset = offset;
		block.tag = tag;
		block.size = size;
		block.parent = parentIndex;
		blocks.push_back(block);

		// Add block to parent
		if (parentIndex >= 0)
			blocks[parentIndex].children.push_back(blockIndex);

		// Build children blocks
		if (isContainerBlock(tag)) {
			buildBlocks(file, offset + headerSize, offset + size, blockIndex, blocks);

			// Warn if children sizes don't match parent payload
			uint32 childrenSize = 0;
			for (uint i = 0; i < blocks[blockIndex].children.size(); ++i)
				childrenSize += blocks[blocks[blockIndex].children[i]].size;
			if (childrenSize != size - headerSize)
				warning("Editor: %s at 0x%08X children size mismatch (%u vs %u)", tag2str(tag), offset, childrenSize, size - headerSize);
		}

		offset += size;
	}
}

bool Resource::isContainerBlock(uint32 tag) const {
	// Handle v5+ blocks
	if (_version >= 5)
		switch (tag) {
		case MKTAG('L','E','C','F'):
		case MKTAG('L','F','L','F'):
		case MKTAG('O','B','C','D'):
		case MKTAG('O','B','I','M'):
		case MKTAG('R','M','I','M'):
		case MKTAG('R','O','O','M'):
		case MKTAG('W','R','A','P'):
			return true;
		default:
			break;
		}

	// Handle v6+ blocks
	if (_version >= 6)
		switch (tag) {
		case MKTAG('P','A','L','S'):
			return true;
		default:
			break;
		}

	// Handle v5-v7 IMxx blocks
	if (_version >= 5 && _version <= 7) {
		byte b0 = (tag >> 24) & 0xFF;
		byte b1 = (tag >> 16) & 0xFF;
		byte b2 = (tag >> 8) & 0xFF;
		byte b3 = tag & 0xFF;
		if (b0 == 'I' && b1 == 'M' && Common::isXDigit(b2) && Common::isXDigit(b3))
			return true;
	}

	// Handle v7+ blocks
	if (_version >= 7)
		switch (tag) {
		case MKTAG('A','K','O','S'):
			return true;
		default:
			break;
		}

	// Handle v8 blocks
	if (_version == 8)
		switch (tag) {
		case MKTAG('I','M','A','G'):
		case MKTAG('R','M','S','C'):
		case MKTAG('S','M','A','P'):
		case MKTAG('Z','P','L','N'):
			return true;
		default:
			break;
		}

	return false;
}

void Resource::addFile(const Common::Path &path, byte encByte) {
	// Open resource file
	File *file = new File();
	if (!file->open(path, encByte)) {
		delete file;
		return;
	}
	_files.push_back(file);

	// Build block tree
	Common::Array<Block> blocks;
	buildBlocks(*file, 0, (uint32)file->size(), -1, blocks);
	_blocks.push_back(blocks);
}

int Resource::getFileCount() const {
	return _files.size();
}

File *Resource::getFile(int index) {
	return _files[index];
}

const Common::Array<Block> &Resource::getBlocks(int index) const {
	return _blocks[index];
}

} // End of namespace Editor

} // End of namespace Scumm
