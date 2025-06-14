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

#ifndef LASTEXPRESS_ARCHIVE_H
#define LASTEXPRESS_ARCHIVE_H

#include "lastexpress/lastexpress.h"

#include "common/file.h"

namespace LastExpress {

struct Seq;

/*
 *  HPF Archive Format
 *
 *  * uint32 {4}   Number of files
 *
 *  For each file:
 *  	* char {12}    Name (zero-terminated)
 *  	* uint32 {4}   Offset (expressed in sectors of 2048 bytes)
 *  	* uint16 {2}   Size (expressed in sectors of 2048 bytes)
 *  	* uint16 {2}   Current position (expressed in sectors of 2048 bytes)
 *  	* uint16 {2}   File status flags:
 *  				   - Bit 0: "Is on CD"
 *  				   - Bit 1: "Is loaded"
 */

typedef struct HPF {
	char name[12];
	uint32 offset;
	uint16 size;
	uint16 currentPos;
	uint16 status;

	// For Gold Edition only
	Common::ArchiveMemberPtr archiveRef;
	Common::String archiveName;

	HPF() {
		memset(name, 0, sizeof(name));
		offset = 0;
		size = 0;
		currentPos = 0;
		status = 0;

		archiveRef = nullptr;
		archiveName = "";
	};
} HPF;

enum HPFFlags {
	kHPFFileIsOnCD   = 1 << 0,
	kHPFFileIsLoaded = 1 << 1,
};


class ArchiveManager {
public:
	ArchiveManager(LastExpressEngine *engine);
	virtual ~ArchiveManager();

	HPF *search(const char *name, HPF *archive, int archiveSize);
	virtual bool lockCD(int32 index);
	virtual bool isCDAvailable(int cdNum, char *outPath, int pathSize);
	virtual bool lockCache(char *filename);
	virtual void initHPFS();
	virtual void shutDownHPFS();
	void unlockCD();
	virtual HPF *openHPF(const char *filename);
	void readHD(void *dstBuf, int offset, uint32 size);
	void readCD(void *dstBuf, int offset, uint32 size);
	virtual void readHPF(HPF *archive, void *dstBuf, uint32 size);
	void seekHPF(HPF *archive, uint32 position);
	void closeHPF(HPF *archive);

	virtual int loadBG(const char *filename);
	Seq *loadSeq(const char *filename, uint8 ticksToWaitUntilCycleRestart, int character);
	void loadMice();

protected:
	LastExpressEngine *_engine = nullptr;

	Common::File *_cdFilePointer = nullptr;
	int32 _cdFilePosition = 0;
	int32 _cdArchiveNumFiles = 0;

	Common::File *_hdFilePointer = nullptr;
	int32 _hdFilePosition = 0;
	int32 _hdArchiveNumFiles = 0;

	HPF *_cdArchive = nullptr;
	HPF *_hdArchive = nullptr;
	int32 _cdArchiveIndex = 0;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ARCHIVE_H
