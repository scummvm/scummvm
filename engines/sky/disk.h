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

#ifndef SKY_DISK_H
#define SKY_DISK_H


#include "common/scummsys.h"
#include "common/compression/rnc_deco.h"

#define MAX_FILES_IN_LIST 60

#define MAX_FRAMES 16 // Maximum number of frames per animation in ibass

namespace Common {
class File;
}

namespace Graphics {
class Surface;
class PixelFormat;
}

struct FileEntry {
	uint32 _fileNum;
	uint32 _offset;
	uint32 _size;
	uint32 _compressedSize;
	FileEntry() : _fileNum(0), _offset(0), _size(0), _compressedSize(0) {}
};

namespace Sky {

struct Animation {
	Graphics::Surface **_frames;
	int _numFrames;
	int _width;
	int _height;
};

class Disk {
public:
	Disk();
	~Disk();

	uint8 *loadFile(uint16 fileNr);
	uint16 *loadScriptFile(uint16 fileNr);
	bool fileExists(uint16 fileNr);

	uint32 determineGameVersion();

	uint32 _lastLoadedFileSize;

	void fnMiniLoad(uint16 fileNum);
	void fnCacheFast(uint16 *fList);
	void fnCacheChip(uint16 *fList);
	void fnCacheFiles();
	void fnFlushBuffers();
	uint32 *giveLoadedFilesList() { return _loadedFilesList; }
	void refreshFilesList(uint32 *list);
	Animation *loadAnim(const char *filename, const Graphics::PixelFormat &targetFormat);

protected:
	uint8 *getFileInfo(uint16 fileNr);
	void dumpFile(uint16 fileNr);

	uint32 _numFiles;
	FileEntry *_entry;

	uint32 _dinnerTableEntries;
	uint8 *_dinnerTableArea;
	Common::File *_dataDiskHandle;
	Common::RncDecoder _rncDecoder;

	uint32 getFileSize(uint32 filenum);
	FileEntry *getEntry(uint32 filenum);

	uint16 _buildList[MAX_FILES_IN_LIST];
	uint32 _loadedFilesList[MAX_FILES_IN_LIST];
};

} // End of namespace Sky

#endif
