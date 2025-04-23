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

#ifndef LASTEXPRESS_HPF_H
#define LASTEXPRESS_HPF_H

/*
	HPF Archive Format

	* uint32 {4}   Number of files

	For each file:
		* char {12}    Name (zero-terminated)
		* uint32 {4}   Offset (expressed in sectors of 2048 bytes)
		* uint16 {2}   Size (expressed in sectors of 2048 bytes)
		* uint16 {2}   Current position (expressed in sectors of 2048 bytes)
		* uint16 {2}   File status flags:
					   - Bit 0: "Is on CD"
					   - Bit 1: "Is loaded"
*/

#include "common/archive.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/file.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

class HPFArchive : public Common::Archive {
public:
	HPFArchive(const Common::Path &path);

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	int count() { return _files.size(); }

private:
	static const unsigned int _archiveNameSize = 12;
	static const unsigned int _archiveSectorSize = 2048;

	// File entry
	struct HPFEntry {
		uint32 offset;          ///< Offset (in sectors of 2048 bytes)
		uint32 size;            ///< Size (in sectors of 2048 bytes)
		uint16 isOnHD;          ///< File location (1: on HD; 0: on CD)
	};

	typedef Common::HashMap<Common::String, HPFEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	FileMap _files;             ///< List of files
	Common::Path _filename;   ///< Filename of the archive
};

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

	HPF() {
		memset(name, 0, sizeof(name));
		offset = 0;
		size = 0;
		currentPos = 0;
		status = 0;
	};

} HPF;

enum HPFFlags {
	kHPFFileIsOnCD   = 1 << 0,
	kHPFFileIsLoaded = 1 << 1,
};


class ArchiveManager {
public:
	ArchiveManager(LastExpressEngine *engine);
	~ArchiveManager();

	HPF *search(const char *name, HPF *archive, int archiveSize);
	bool lockCD(int32 index);
	bool isCDAvailable(int cdNum, char *outPath, int pathSize);
	bool lockCache(char *filename);
	void initHPFS();
	void shutDownHPFS();
	void unlockCD();
	HPF *openHPF(const char *filename);
	void readHD(void *dstBuf, int offset, uint32 size);
	void readCD(void *dstBuf, int offset, uint32 size);
	void readHPF(HPF *archive, void *dstBuf, uint32 size);
	void seekHPF(HPF *archive, uint32 position);
	void closeHPF(HPF *archive);

	int loadBG(const char *filename);
	Seq *loadSeq(const char *filename, uint8 ticksToWaitUntilCycleRestart, int character);
	void loadMice();

private:
	LastExpressEngine *_engine = nullptr;

	Common::File *g_CDFilePointer = nullptr;
	int32 g_CDFilePosition = 0;
	int32 g_CDArchiveNumFiles = 0;

	Common::File *g_HDFilePointer = nullptr;
	int32 g_HDFilePosition = 0;
	int32 g_HDArchiveNumFiles = 0;

	HPF *g_CDArchive = nullptr;
	HPF *g_HDArchive = nullptr;
	int32 g_CDArchiveIndex = 0;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_HPF_H
