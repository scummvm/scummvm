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

/**
 * @file

 */

#include "common/array.h"
#include "common/fs.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/str-array.h"

#ifndef COMMON_MACRESMAN_H
#define COMMON_MACRESMAN_H

namespace Common {

/**
 * @defgroup common_macresman Macintosh resource fork manager
 * @ingroup common
 *
 * @brief API for Macintosh resource fork manager.
 *
 * @details Used in engines:
 *          - director
 *          - groovie
 *          - kyra
 *          - mohawk
 *          - mtropolis
 *          - pegasus
 *          - sci
 *          - scumm
 * @{
 */

typedef Array<uint16> MacResIDArray;
typedef Array<uint32> MacResTagArray;

/**
 * Class containing the raw data bytes for a Macintosh Finder Info data block.
 */
struct MacFinderInfoData {
	byte data[16];
};

/**
 * Class containing the raw data bytes for a Macintosh Extended Finder Info data block.
 */
struct MacFinderExtendedInfoData {
	byte data[16];
};

/**
 * Class containing Macintosh Finder Info.
 */
struct MacFinderInfo {
	enum FinderFlags {
		kFinderFlagAlias = (1 << 15),
		kFinderFlagInvisible = (1 << 14),
		kFinderFlagBundle = (1 << 13),
		kFinderFlagNameLocked = (1 << 12),
		kFinderFlagStationery = (1 << 11),
		kFinderFlagCustomIcon = (1 << 10),
		kFinderFlagInited = (1 << 8),
		kFinderFlagNoInit = (1 << 7),
		kFinderFlagShared = (1 << 6),

		kFinderFlagColorBit2 = (1 << 3),
		kFinderFlagColorBit1 = (1 << 2),
		kFinderFlagColorBit0 = (1 << 1),
	};

	MacFinderInfo();
	explicit MacFinderInfo(const MacFinderInfoData &data);

	MacFinderInfoData toData() const;

	byte type[4];
	byte creator[4];
	uint16 flags;
	Common::Point position;
	int16 windowID;
};

/**
 * Class containing Macintosh Extended Finder Info.
 */
struct MacFinderExtendedInfo {
	static const uint kDataSize = 16;

	MacFinderExtendedInfo();
	explicit MacFinderExtendedInfo(const MacFinderExtendedInfoData &data);

	MacFinderExtendedInfoData toData() const;

	int16 iconID;
	int16 commentID;
	int32 homeDirectoryID;
};

/**
 * Class for handling Mac data and resource forks.
 * It can read from raw, MacBinary, and AppleDouble formats.
 */
class MacResManager {

#define MBI_INFOHDR 128

public:
	MacResManager();
	~MacResManager();

	/**
	 * Open a Mac data/resource fork pair.
	 *
	 * This uses SearchMan to find the data/resource forks. This should only be used
	 * from inside an engine.
	 *
	 * @param fileName The base file name of the file
	 * @note This will check for the raw resource fork, MacBinary, and AppleDouble formats.
	 * @return True on success
	 */
	bool open(const Path &fileName);

	/**
	 * Open a Mac data/resource fork pair from within the given archive.
	 *
	 * @param path The path that holds the forks
	 * @param fileName The base file name of the file
	 * @note This will check for the raw resource fork, MacBinary, and AppleDouble formats.
	 * @return True on success
	 */
	bool open(const Path &fileName, Archive &archive);

	/**
	 * Opens file named fileName or data fork extracted as macbin
	 * @return The stream if found, 0 otherwise
	 */
	static SeekableReadStream *openFileOrDataFork(const Path &fileName, Archive &archive);
	static SeekableReadStream *openFileOrDataFork(const Path &fileName);

	/**
	 * Open data fork of macbinary.
	 * @return The stream if found, 0 otherwise
	 */
	static SeekableReadStream *openDataForkFromMacBinary(SeekableReadStream *inStream, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::NO);

	/**
	 * See if a Mac data/resource fork pair exists.
	 * @param fileName The base file name of the file
	 * @return True if either a data fork or resource fork with this name exists
	 */
	static bool exists(const Path &fileName);

	/**
	 * Attempt to read the Mac Finder info metadata for a file path.
	 * @param fileName The base file name of the file
	 * @param archive The archive to search in
	 * @param outFinderInfo The loaded and parsed Finder info
	 * @param outFinderExtendedInfo The loaded and parsed Finder extended info
	 * @return True if finder info was available for a path, false if not
	 */
	static bool getFileFinderInfo(const Path &fileName, Archive &archive, MacFinderInfo &outFinderInfo);
	static bool getFileFinderInfo(const Path &fileName, Archive &archive, MacFinderInfo &outFinderInfo, MacFinderExtendedInfo &outFinderExtendedInfo);
	static bool getFileFinderInfo(const Path &fileName, MacFinderInfo &outFinderInfo);
	static bool getFileFinderInfo(const Path &fileName, MacFinderInfo &outFinderInfo, MacFinderExtendedInfo &outFinderExtendedInfo);

	/**
	 * List all filenames matching pattern for opening with open().
	 *
	 * @param files Array containing all matching filenames discovered. Only
	 *              adds to the list.
	 * @param pattern Pattern to match against. Taking String::matchPattern's
	 *                format.
	 */
	static void listFiles(Array<Path> &files, const Path &pattern);

	/**
	 * Close the Mac data/resource fork pair.
	 */
	void close();

	/**
	 * Query whether or not we have a data fork present.
	 * @return True if the resource fork is present
	 */
	bool hasResFork() const;

	/**
	 * Read resource from the MacBinary file
	 * @param typeID FourCC of the type
	 * @param resID Resource ID to fetch
	 * @return Pointer to a SeekableReadStream with loaded resource
	 */
	SeekableReadStream *getResource(uint32 typeID, uint16 resID);

	/**
	 * Read resource from the MacBinary file
	 * @note This will take the first resource that matches this name, regardless of type
	 * @param filename file name of the resource
	 * @return Pointer to a SeekableReadStream with loaded resource
	 */
	SeekableReadStream *getResource(const String &filename);

	/**
	 * Read resource from the MacBinary file
	 * @param typeID FourCC of the type
	 * @param filename file name of the resource
	 * @return Pointer to a SeekableReadStream with loaded resource
	 */
	SeekableReadStream *getResource(uint32 typeID, const String &filename);

	static int getDataForkOffset() { return MBI_INFOHDR; }

	/**
	 * Get the name of a given resource
	 * @param typeID FourCC of the type
	 * @param resID Resource ID to fetch
	 * @return The name of a given resource and an empty string if not present
	 */
	String getResName(uint32 typeID, uint16 resID) const;

	/**
	 * Get the length in bytes of a given resource
	 * @param typeID FourCC of the type
	 * @param resID Resource ID to fetch
	 * @return The length in bytes of a given resource
	 */
	uint32 getResLength(uint32 typeID, uint16 resID);

	/**
	 * Get the size of the data portion of the resource fork
	 * @return The size of the data portion of the resource fork
	 */
	uint32 getResForkDataSize() const;

	/**
	 * Calculate the MD5 checksum of the resource fork
	 * @param length The maximum length to compute for
	 * @param tail Calculate length from the tail
	 * @return The MD5 checksum of the resource fork
	 */
	String computeResForkMD5AsString(uint32 length = 0, bool tail = false) const;

	/**
	 * Get the base file name of the data/resource fork pair
	 * @return The base file name of the data/resource fork pair
	 */
	Path getBaseFileName() const { return _baseFileName; }

	void setBaseFileName(Common::Path str) { _baseFileName = str; }

	/**
	 * Return list of resource IDs with specified type ID
	 */
	MacResIDArray getResIDArray(uint32 typeID);

	/**
	 * Return list of resource tags
	 */
	MacResTagArray getResTagArray();

	/**
	 * Load from stream in MacBinary format
	 */
	bool loadFromMacBinary(SeekableReadStream *stream);

	/**
	 * Dump contents of the archive to ./dumps directory
	 */
	 void dumpRaw();

	/**
	 * Check if the given stream is in the MacBinary format.
	 * @param stream The stream we're checking
	 */
	static bool isMacBinary(SeekableReadStream &stream);

	struct MacVers {
		byte majorVer;
		byte minorVer;
		byte devStage;
		String devStr;
		byte preReleaseVer;
		uint16 region;
		String str;
		String msg;
	};
	static MacVers *parseVers(SeekableReadStream *vvers);

private:
	SeekableReadStream *_stream;
	Path _baseFileName;

	bool load(SeekableReadStream *stream);

	bool loadFromRawFork(SeekableReadStream *stream);
	bool loadFromAppleDouble(SeekableReadStream *stream);

	/**
	 * Get Finder info from a file in MacBinary format
	 */
	static bool getFinderInfoFromMacBinary(SeekableReadStream *stream, MacFinderInfo &outFinderInfo, MacFinderExtendedInfo &outFinderExtendedInfo);

	/**
	 * Get Finder info from a file in AppleDouble format
	 */
	static bool getFinderInfoFromAppleDouble(SeekableReadStream *stream, MacFinderInfo &outFinderInfo, MacFinderExtendedInfo &outFinderExtendedInfo);

	static bool readAndValidateMacBinaryHeader(SeekableReadStream &stream, byte (&outMacBinaryHeader)[MBI_INFOHDR]);

	static Path constructAppleDoubleName(const Path &name);
	static Path disassembleAppleDoubleName(const Path &name, bool *isAppleDouble);

	static SeekableReadStream *openAppleDoubleWithAppleOrOSXNaming(Archive& archive, const Path &fileName);

	/**
	 * Do a sanity check whether the given stream is a raw resource fork.
	 *
	 * @param stream Stream object to check. Will not preserve its position.
	 */
	static bool isRawFork(SeekableReadStream &stream);

	enum {
		kResForkNone = 0,
		kResForkRaw,
		kResForkMacBinary,
		kResForkAppleDouble
	} _mode;

	void readMap();

	struct ResMap {
		uint16 resAttr;
		uint16 typeOffset;
		uint16 nameOffset;
		uint16 numTypes;

		void reset() {
			resAttr = 0;
			typeOffset = 0;
			nameOffset = 0;
			numTypes = 0;
		}

		ResMap() { reset(); }
	};

	struct ResType {
		uint32 id;
		uint16 items;
		uint16 offset;
	};

	struct Resource {
		uint16 id;
		int16 nameOffset;
		byte attr;
		uint32 dataOffset;
		char *name;
	};

	typedef Resource *ResPtr;

	int32 _resForkOffset;
	uint32 _resForkSize;

	uint32 _dataOffset;
	uint32 _dataLength;
	uint32 _mapOffset;
	uint32 _mapLength;
	ResMap _resMap;
	ResType *_resTypes;
	ResPtr  *_resLists;
};

/** @} */

} // End of namespace Common

#endif
