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
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include "common/memstream.h"
#include "common/platform.h"
#include "common/str.h"
#include "common/stream.h"

#include "startrek/iwfile.h"

namespace Common {
class MacResManager;
}

namespace StarTrek {

struct ResourceIndex {
	uint32 indexOffset;
	bool foundData;
	uint16 fileCount;
	uint16 uncompressedSize;	// used in the demo
	Common::String fileName;

	ResourceIndex() {
		indexOffset = 0;
		foundData = 0;
		fileCount = 0;
		uncompressedSize = 0;
		fileName = "";
	}
};

class Resource {
public:
	Resource(Common::Platform platform, bool isDemo);
	virtual ~Resource();

	Common::List<ResourceIndex> searchIndex(Common::String filename);
	Common::MemoryReadStreamEndian *loadFile(Common::String filename, int fileIndex = 0, bool errorOnNotFound = true);
	Common::MemoryReadStreamEndian *loadBitmapFile(Common::String baseName);

	/**
	 * TODO: Figure out what the extra parameters are, and if they're important.
	 */
	Common::MemoryReadStreamEndian *loadFileWithParams(Common::String filename, bool unk1, bool unk2, bool unk3);

	/**
	 * ".txt" files are just lists of strings. This traverses the file to get a particular
	 * string index.
	 */
	Common::String getLoadedText(int textIndex);

	void setTxtFileName(Common::String txtFileName) {
		_txtFilename = txtFileName;
	}

	Common::List<ResourceIndex> _resources;

private:
	void readIndexFile();
	ResourceIndex getIndex(Common::String filename);
	ResourceIndex getIndexEntry(Common::SeekableReadStream *indexFile);
	Common::MemoryReadStreamEndian *loadSequentialFile(Common::String filename, int fileIndex = 0);
	uint32 getSequentialFileOffset(uint32 offset, int fileIndex);

	//IWFile *_iwFile;
	Common::MacResManager *_macResFork;
	Common::Platform _platform;
	bool _isDemo;
	Common::String _txtFilename;
	//Common::List<ResourceIndex> _resources;
};

} // End of namespace StarTrek

#endif
