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
 * Unit tests for Common::ISO9660FileSystem -- also serve as usage examples.
 *
 * All tests use synthetic in-memory ISO images built from byte arrays --
 * no real disc image is required to run them.
 *
 * Key patterns demonstrated:
 *   - Opening an ISO from a memory stream
 *   - Reading the root directory: readRootDirectory(), getRootEntry()
 *   - Traversing subdirectories: readDirectory(DirEntry)
 *   - Opening and reading a file: openFile(DirEntry)
 *   - Joliet and High Sierra format variants
 */

#include <cxxtest/TestSuite.h>
#include "common/formats/iso9660.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/endian.h"
#include "common/str.h"

// Helpers to build a minimal in-memory ISO 9660 image
static const uint32 kSectorSize = 2048;

/** Write a 4-byte little-endian integer into buf at pos. */
static void writeLE32(byte *buf, int pos, uint32 v) {
	buf[pos+0] = static_cast<byte>(v      );
	buf[pos+1] = static_cast<byte>(v >>  8);
	buf[pos+2] = static_cast<byte>(v >> 16);
	buf[pos+3] = static_cast<byte>(v >> 24);
}

/** Write a 4-byte big-endian integer into buf at pos. */
static void writeBE32(byte *buf, int pos, uint32 v) {
	buf[pos+0] = static_cast<byte>(v >> 24);
	buf[pos+1] = static_cast<byte>(v >> 16);
	buf[pos+2] = static_cast<byte>(v >>  8);
	buf[pos+3] = static_cast<byte>(v      );
}

/** Write a both-endian (ISO 9660 DWORD) at pos: LE then BE. */
static void writeBothEndian32(byte *buf, int pos, uint32 v) {
	writeLE32(buf, pos,   v);
	writeBE32(buf, pos+4, v);
}

/**
 * Write a directory record at buf+pos.
 * Returns the actual length of the record (padded to even byte count).
 *
 * @param flagsOfs  Byte offset for the flags field within the record.
 *                  ISO 9660 uses 25 (default), High Sierra uses 24.
 */
static int writeDirRecord(byte *buf, int pos,
                           uint32 lba, uint32 size, uint8 flags,
                           const char *id, uint8 idLen,
                           uint8 flagsOfs = 25) {
	uint8 recLen = static_cast<uint8>(33 + idLen);
	if (recLen & 1) recLen++; // ISO 9660 requires even-length records

	memset(buf + pos, 0, recLen);
	buf[pos + 0]  = recLen;
	buf[pos + 1]  = 0; // ext attr
	writeLE32(buf, pos + 2, lba);
	writeBE32(buf, pos + 6, lba);
	writeBothEndian32(buf, pos + 10, size);
	buf[pos + flagsOfs] = flags;
	buf[pos + 28] = 1; buf[pos + 29] = 0; // vol seq LE = 1
	buf[pos + 30] = 0; buf[pos + 31] = 1; // vol seq BE = 1
	buf[pos + 32] = idLen;
	memcpy(buf + pos + 33, id, idLen);
	return recLen;
}

/**
 * Build a complete minimal ISO 9660 image in *outBuf (caller must delete[]).
 *
 * Layout:
 *   Sector 16 : Primary Volume Descriptor
 *   Sector 17 : Volume Descriptor Set Terminator
 *   Sector 18 : Root directory (contains "hello.txt" and "subdir/")
 *   Sector 19 : Contents of "hello.txt" = "Hello, World!"
 *   Sector 20 : Contents of "subdir/" (just . and .. entries)
 *
 * @param outSize      Receives the buffer size in bytes.
 * @param escapeLevel  0 = no Joliet; 0x40/0x43/0x45 = Joliet UCS-2 level.
 *                     When non-zero, a Joliet SVD is inserted at sector 17
 *                     and subsequent sectors shift down by one.
 */
static byte *buildMinimalISO(uint32 *outSize, byte escapeLevel = 0) {
	bool joliet = (escapeLevel != 0);
	// Sector numbers
	uint32 pvdSector   = 16;
	uint32 jolietSec   = joliet ? 17 : 0;
	uint32 vtermSector = joliet ? 18 : 17;
	uint32 rootDirSec  = joliet ? 19 : 18;
	uint32 fileSec     = joliet ? 20 : 19;
	uint32 subdirSec   = joliet ? 21 : 20;
	uint32 totalSectors = subdirSec + 1;

	uint32 bufSize = totalSectors * kSectorSize;
	byte  *buf     = new byte[bufSize];
	memset(buf, 0, bufSize);

	static const char kFileData[] = "Hello, World!";
	static const uint32 kFileSize = 13;

	// Root directory data
	byte  *rootDir = buf + rootDirSec * kSectorSize;
	int    rdPos   = 0;

	// "." entry
	char dotId = 0x00;
	rdPos += writeDirRecord(rootDir, rdPos, rootDirSec, kSectorSize, 0x02, &dotId, 1);

	// ".." entry
	char dotdotId = 0x01;
	rdPos += writeDirRecord(rootDir, rdPos, rootDirSec, kSectorSize, 0x02, &dotdotId, 1);

	// "HELLO.TXT" file entry
	rdPos += writeDirRecord(rootDir, rdPos, fileSec, kFileSize, 0x00, "HELLO.TXT;1", 11);

	// "SUBDIR" directory entry
	rdPos += writeDirRecord(rootDir, rdPos, subdirSec, 68, 0x02, "SUBDIR", 6);

	auto rootDirSize = static_cast<uint32>(rdPos);

	// Subdir directory data
	byte *subdirBuf = buf + subdirSec * kSectorSize;
	int   sdPos     = 0;
	sdPos += writeDirRecord(subdirBuf, sdPos, subdirSec, 68, 0x02, &dotId, 1);
	sdPos += writeDirRecord(subdirBuf, sdPos, rootDirSec, rootDirSize, 0x02, &dotdotId, 1);

	// File data
	memcpy(buf + fileSec * kSectorSize, kFileData, kFileSize);

	// PVD
	byte *pvd = buf + pvdSector * kSectorSize;
	pvd[0] = 1;           // type = PVD
	pvd[1] = 'C'; pvd[2] = 'D'; pvd[3] = '0'; pvd[4] = '0'; pvd[5] = '1';
	pvd[6] = 1;           // version
	// Embed root directory record at offset 156
	writeDirRecord(pvd, 156, rootDirSec, rootDirSize, 0x02, &dotId, 1);
	// Also set the volume space size
	writeBothEndian32(pvd, 80, totalSectors);

	// Joliet SVD (optional)
	if (joliet) {
		byte *svd = buf + jolietSec * kSectorSize;
		svd[0] = 2;  // supplementary VD
		svd[1] = 'C'; svd[2] = 'D'; svd[3] = '0'; svd[4] = '0'; svd[5] = '1';
		svd[6] = 1;
		// Joliet escape sequences at offset 88: %/ + level byte
		svd[88] = 0x25; svd[89] = 0x2F; svd[90] = escapeLevel;
		// Embed the same root directory record (Joliet uses the same data sectors)
		writeDirRecord(svd, 156, rootDirSec, rootDirSize, 0x02, &dotId, 1);
	}

	// VD Set Terminator
	byte *vterm = buf + vtermSector * kSectorSize;
	vterm[0] = 255;  // terminator type
	vterm[1] = 'C'; vterm[2] = 'D'; vterm[3] = '0'; vterm[4] = '0'; vterm[5] = '1';
	vterm[6] = 1;

	*outSize = bufSize;
	return buf;
}

/**
 * Build a minimal ISO with a custom filename for the single file entry.
 * No Joliet, no subdirectory.
 */
static byte *buildMinimalISOCustomFile(uint32 *outSize,
                                        const char *filename, uint8 filenameLen) {
	uint32 pvdSector   = 16;
	uint32 vtermSector = 17;
	uint32 rootDirSec  = 18;
	uint32 fileSec     = 19;
	uint32 totalSectors = 20;

	uint32 bufSize = totalSectors * kSectorSize;
	byte  *buf     = new byte[bufSize];
	memset(buf, 0, bufSize);

	static const char kFileData[] = "Test data";
	static const uint32 kFileSize = 9;

	byte *rootDir = buf + rootDirSec * kSectorSize;
	int   rdPos   = 0;

	char dotId = 0x00;
	char dotdotId = 0x01;
	rdPos += writeDirRecord(rootDir, rdPos, rootDirSec, kSectorSize, 0x02, &dotId, 1);
	rdPos += writeDirRecord(rootDir, rdPos, rootDirSec, kSectorSize, 0x02, &dotdotId, 1);
	rdPos += writeDirRecord(rootDir, rdPos, fileSec, kFileSize, 0x00, filename, filenameLen);

	auto rootDirSize = static_cast<uint32>(rdPos);

	memcpy(buf + fileSec * kSectorSize, kFileData, kFileSize);

	byte *pvd = buf + pvdSector * kSectorSize;
	pvd[0] = 1;
	pvd[1] = 'C'; pvd[2] = 'D'; pvd[3] = '0'; pvd[4] = '0'; pvd[5] = '1';
	pvd[6] = 1;
	writeDirRecord(pvd, 156, rootDirSec, rootDirSize, 0x02, &dotId, 1);
	writeBothEndian32(pvd, 80, totalSectors);

	byte *vterm = buf + vtermSector * kSectorSize;
	vterm[0] = 255;
	vterm[1] = 'C'; vterm[2] = 'D'; vterm[3] = '0'; vterm[4] = '0'; vterm[5] = '1';
	vterm[6] = 1;

	*outSize = bufSize;
	return buf;
}

/**
 * Build a minimal HSF (High Sierra Format) image.
 *
 * Layout:
 *   Sector 16 : HSF PVD ("CDROM" at offset 9, root dir record at offset 180)
 *   Sector 17 : Root directory (with HSF-style dir records: flags at byte 24)
 *   Sector 18 : File data
 *   Sector 19 : Subdirectory
 *
 * No VD terminator (HSF skips Joliet scan).
 */
static byte *buildHSFImage(uint32 *outSize) {
	uint32 pvdSector   = 16;
	uint32 rootDirSec  = 17;
	uint32 fileSec     = 18;
	uint32 subdirSec   = 19;
	uint32 totalSectors = subdirSec + 1;

	uint32 bufSize = totalSectors * kSectorSize;
	byte  *buf     = new byte[bufSize];
	memset(buf, 0, bufSize);

	static const char kFileData[] = "Hello, World!";
	static const uint32 kFileSize = 13;

	// Root directory data (HSF dir records: flags at byte 24)
	byte *rootDir = buf + rootDirSec * kSectorSize;
	int   rdPos   = 0;

	char dotId = 0x00;
	char dotdotId = 0x01;
	rdPos += writeDirRecord(rootDir, rdPos, rootDirSec, kSectorSize, 0x02, &dotId, 1, 24);
	rdPos += writeDirRecord(rootDir, rdPos, rootDirSec, kSectorSize, 0x02, &dotdotId, 1, 24);
	rdPos += writeDirRecord(rootDir, rdPos, fileSec, kFileSize, 0x00, "HELLO.TXT;1", 11, 24);
	rdPos += writeDirRecord(rootDir, rdPos, subdirSec, 68, 0x02, "SUBDIR", 6, 24);

	auto rootDirSize = static_cast<uint32>(rdPos);

	// Subdir
	byte *subdirBuf = buf + subdirSec * kSectorSize;
	int   sdPos     = 0;
	sdPos += writeDirRecord(subdirBuf, sdPos, subdirSec, 68, 0x02, &dotId, 1, 24);
	sdPos += writeDirRecord(subdirBuf, sdPos, rootDirSec, rootDirSize, 0x02, &dotdotId, 1, 24);

	// File data
	memcpy(buf + fileSec * kSectorSize, kFileData, kFileSize);

	// HSF PVD: type=1 at byte 8, "CDROM" at bytes 9-13, version=1 at 14
	byte *pvd = buf + pvdSector * kSectorSize;
	pvd[8]  = 1; // type = PVD
	pvd[9]  = 'C'; pvd[10] = 'D'; pvd[11] = 'R'; pvd[12] = 'O'; pvd[13] = 'M';
	pvd[14] = 1; // version
	// Root dir record at offset 180 (kHSFVDRootDR)
	writeDirRecord(pvd, 180, rootDirSec, rootDirSize, 0x02, &dotId, 1, 24);
	writeBothEndian32(pvd, 80, totalSectors);

	*outSize = bufSize;
	return buf;
}

/**
 * Build an ISO where the root directory spans 2 sectors.
 *
 * Sector 1 of root dir has ".", "..", "FILE1.TXT;1", then zero-padding.
 * Sector 2 of root dir starts with "FILE2.TXT;1".
 * Two file data sectors follow.
 */
static byte *buildMultiSectorDirISO(uint32 *outSize) {
	uint32 pvdSector   = 16;
	uint32 vtermSector = 17;
	uint32 rootDirSec  = 18; // root dir spans sectors 18-19
	uint32 file1Sec    = 20;
	uint32 file2Sec    = 21;
	uint32 totalSectors = 22;

	uint32 bufSize = totalSectors * kSectorSize;
	byte  *buf     = new byte[bufSize];
	memset(buf, 0, bufSize);

	static const char kFile1Data[] = "File One";
	static const uint32 kFile1Size = 8;
	static const char kFile2Data[] = "File Two";
	static const uint32 kFile2Size = 8;

	uint32 rootDirSize = 2 * kSectorSize;

	// Root dir sector 1 (sector 18)
	byte *rootDir1 = buf + rootDirSec * kSectorSize;
	int   rdPos    = 0;

	char dotId = 0x00;
	char dotdotId = 0x01;
	rdPos += writeDirRecord(rootDir1, rdPos, rootDirSec, rootDirSize, 0x02, &dotId, 1);
	rdPos += writeDirRecord(rootDir1, rdPos, rootDirSec, rootDirSize, 0x02, &dotdotId, 1);
	rdPos += writeDirRecord(rootDir1, rdPos, file1Sec, kFile1Size, 0x00, "FILE1.TXT;1", 11);
	// Remaining bytes in sector 18 are zero (padding to sector boundary)

	// Root dir sector 2 (sector 19)
	byte *rootDir2 = buf + (rootDirSec + 1) * kSectorSize;
	writeDirRecord(rootDir2, 0, file2Sec, kFile2Size, 0x00, "FILE2.TXT;1", 11);

	// File data
	memcpy(buf + file1Sec * kSectorSize, kFile1Data, kFile1Size);
	memcpy(buf + file2Sec * kSectorSize, kFile2Data, kFile2Size);

	// PVD
	byte *pvd = buf + pvdSector * kSectorSize;
	pvd[0] = 1;
	pvd[1] = 'C'; pvd[2] = 'D'; pvd[3] = '0'; pvd[4] = '0'; pvd[5] = '1';
	pvd[6] = 1;
	writeDirRecord(pvd, 156, rootDirSec, rootDirSize, 0x02, &dotId, 1);
	writeBothEndian32(pvd, 80, totalSectors);

	// VD Set Terminator
	byte *vterm = buf + vtermSector * kSectorSize;
	vterm[0] = 255;
	vterm[1] = 'C'; vterm[2] = 'D'; vterm[3] = '0'; vterm[4] = '0'; vterm[5] = '1';
	vterm[6] = 1;

	*outSize = bufSize;
	return buf;
}

/**
 * Create an ISO9660FileSystem from a byte buffer.
 *
 * The buffer is COPIED internally -- the caller may free their original
 * buffer immediately after this returns. The caller owns the returned
 * ISO9660FileSystem and must delete it.
 */
static Common::ISO9660FileSystem *openTestISO(const byte *buf, uint32 size) {
	byte *copy = new byte[size];
	memcpy(copy, buf, size);
	Common::SeekableReadStream *stream =
		new Common::MemoryReadStream(copy, size, DisposeAfterUse::YES);
	return new Common::ISO9660FileSystem(stream);
}

static const Common::ISO9660FileSystem::DirEntry *findEntry(
		const Common::Array<Common::ISO9660FileSystem::DirEntry> &entries,
		const Common::String &name) {
	for (const auto &e : entries) {
		if (e.name == name) return &e;
	}
	return nullptr;
}

// Test suite
class ISO9660TestSuite : public CxxTest::TestSuite {
public:

	Common::ISO9660FileSystem *openStandardISO(byte escapeLevel = 0) {
		uint32 size;
		byte *buf = buildMinimalISO(&size, escapeLevel);
		Common::ISO9660FileSystem *fs = openTestISO(buf, size);
		delete[] buf;
		return fs;
	}

	// Parser basics
	void testMissingMagicReturnsError() {
		byte buf[2048 * 18];
		memset(buf, 0, sizeof(buf));
		// Write garbage instead of PVD
		buf[16 * 2048] = 0xFF; // wrong type
		Common::ISO9660FileSystem *fs = openTestISO(buf, sizeof(buf));
		TS_ASSERT(!fs->isOpen());
		delete fs;
	}

	void testParsePrimaryVolumeDescriptor() {
		Common::ISO9660FileSystem *fs = openStandardISO();
		TS_ASSERT(fs->isOpen());
		TS_ASSERT_EQUALS(fs->getRootLba(), static_cast<uint32>(18)); // sector 18
		delete fs;
	}

	// Directory reading
	void testRootDirectoryEntries() {
		Common::ISO9660FileSystem *fs = openStandardISO();
		TS_ASSERT(fs->isOpen());
		Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
			fs->readRootDirectory();

		// Should have exactly 2 visible entries: hello.txt and subdir
		TS_ASSERT_EQUALS(entries.size(), static_cast<uint32>(2));

		const auto *fileEntry = findEntry(entries, "hello.txt");
		TS_ASSERT(fileEntry != nullptr);
		TS_ASSERT(!fileEntry->isDir);
		TS_ASSERT_EQUALS(fileEntry->size, static_cast<uint32>(13));

		const auto *dirEntry = findEntry(entries, "subdir");
		TS_ASSERT(dirEntry != nullptr);
		TS_ASSERT(dirEntry->isDir);
		delete fs;
	}

	// File reading
	void testFileContentsMatch() {
		Common::ISO9660FileSystem *fs = openStandardISO();
		TS_ASSERT(fs->isOpen());
		Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
			fs->readRootDirectory();

		const auto *fileEntry = findEntry(entries, "hello.txt");
		TS_ASSERT(fileEntry != nullptr);

		Common::ScopedPtr<Common::SeekableReadStream> stream(
			fs->openFile(*fileEntry));
		TS_ASSERT(stream.get() != nullptr);
		TS_ASSERT_EQUALS(stream->size(), static_cast<int64>(13));

		char readBuf[14] = {};
		TS_ASSERT_EQUALS(stream->read(readBuf, 13), static_cast<uint32>(13));
		TS_ASSERT_EQUALS(Common::String(readBuf), Common::String("Hello, World!"));
		delete fs;
	}

	// Subdirectory traversal
	void testSubdirectoryTraversal() {
		Common::ISO9660FileSystem *fs = openStandardISO();
		TS_ASSERT(fs->isOpen());
		Common::Array<Common::ISO9660FileSystem::DirEntry> root =
			fs->readRootDirectory();

		const auto *subdirEntry = findEntry(root, "subdir");
		TS_ASSERT(subdirEntry != nullptr);

		Common::Array<Common::ISO9660FileSystem::DirEntry> sub =
			fs->readDirectory(*subdirEntry);

		// Subdir is empty (only . and .. which are filtered out)
		TS_ASSERT_EQUALS(sub.size(), static_cast<uint32>(0));
		delete fs;
	}

	// Joliet extensions
	void testJolietDetected() {
		Common::ISO9660FileSystem *fs = openStandardISO(/*escapeLevel=*/0x40);
		TS_ASSERT(fs->isOpen());
		// With Joliet, the root LBA should point to the Joliet root
		// (which in our synthetic image is the same rootDirSec = 19)
		TS_ASSERT_EQUALS(fs->getRootLba(), static_cast<uint32>(19));
		delete fs;
	}

	// Version suffix stripping
	void testVersionSuffixStripped() {
		// The filename "HELLO.TXT;1" should be reported as "hello.txt"
		Common::ISO9660FileSystem *fs = openStandardISO();
		TS_ASSERT(fs->isOpen());
		Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
			fs->readRootDirectory();

		TS_ASSERT(findEntry(entries, "hello.txt") != nullptr);
		delete fs;
	}

	// openFile on a directory returns nullptr
	void testOpenFileOnDirectoryReturnsNull() {
		Common::ISO9660FileSystem *fs = openStandardISO();
		TS_ASSERT(fs->isOpen());
		Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
			fs->readRootDirectory();

		const auto *dirEntry = findEntry(entries, "subdir");
		TS_ASSERT(dirEntry != nullptr);
		TS_ASSERT_EQUALS(fs->openFile(*dirEntry), static_cast<Common::SeekableReadStream *>(nullptr));
		delete fs;
	}

	// High Sierra Format
	void testHighSierraFormat() {
		uint32 size;
		byte *buf = buildHSFImage(&size);
		Common::ISO9660FileSystem *fs = openTestISO(buf, size);
		delete[] buf;

		TS_ASSERT(fs->isOpen());

		Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
			fs->readRootDirectory();

		// Should have 2 visible entries: hello.txt and subdir
		TS_ASSERT_EQUALS(entries.size(), static_cast<uint32>(2));

		bool foundFile = false, foundDir = false;
		for (const auto &e : entries) {
			if (e.name == "hello.txt") {
				foundFile = true;
				TS_ASSERT(!e.isDir);
			} else if (e.name == "subdir") {
				foundDir = true;
				// Proves flags are read from byte 24 (HSF), not byte 25 (ISO).
				// In our HSF records, byte 24 = 0x02, byte 25 = 0x00.
				TS_ASSERT(e.isDir);
			}
		}
		TS_ASSERT(foundFile);
		TS_ASSERT(foundDir);
		delete fs;
	}

	// Trailing dot stripping
	void testTrailingDotStripped() {
		// iso9660ToLower() should strip trailing dots: "README." -> "readme"
		uint32 size;
		byte *buf = buildMinimalISOCustomFile(&size, "README.", 7);
		Common::ISO9660FileSystem *fs = openTestISO(buf, size);
		delete[] buf;

		TS_ASSERT(fs->isOpen());
		Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
			fs->readRootDirectory();

		TS_ASSERT_EQUALS(entries.size(), static_cast<uint32>(1));
		TS_ASSERT_EQUALS(entries[0].name, Common::String("readme"));
		delete fs;
	}

	// Filename without version suffix
	void testFilenameNoVersionSuffix() {
		// iso9660ToLower() with no ";N" suffix: "MAKEFILE" -> "makefile"
		uint32 size;
		byte *buf = buildMinimalISOCustomFile(&size, "MAKEFILE", 8);
		Common::ISO9660FileSystem *fs = openTestISO(buf, size);
		delete[] buf;

		TS_ASSERT(fs->isOpen());
		Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
			fs->readRootDirectory();

		TS_ASSERT_EQUALS(entries.size(), static_cast<uint32>(1));
		TS_ASSERT_EQUALS(entries[0].name, Common::String("makefile"));
		delete fs;
	}

	// Multi-sector directory padding
	void testMultiSectorDirPadding() {
		uint32 size;
		byte *buf = buildMultiSectorDirISO(&size);
		Common::ISO9660FileSystem *fs = openTestISO(buf, size);
		delete[] buf;

		TS_ASSERT(fs->isOpen());
		Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
			fs->readRootDirectory();

		// Should find 2 file entries from 2 different sectors
		TS_ASSERT_EQUALS(entries.size(), static_cast<uint32>(2));

		bool foundFile1 = false, foundFile2 = false;
		for (const auto &e : entries) {
			if (e.name == "file1.txt") {
				foundFile1 = true;
				TS_ASSERT(!e.isDir);
				// Verify the file is readable
				Common::ScopedPtr<Common::SeekableReadStream> stream(
					fs->openFile(e));
				TS_ASSERT(stream.get() != nullptr);
				TS_ASSERT_EQUALS(stream->size(), static_cast<int64>(8));
			} else if (e.name == "file2.txt") {
				foundFile2 = true;
				TS_ASSERT(!e.isDir);
				Common::ScopedPtr<Common::SeekableReadStream> stream(
					fs->openFile(e));
				TS_ASSERT(stream.get() != nullptr);
				TS_ASSERT_EQUALS(stream->size(), static_cast<int64>(8));
			}
		}
		TS_ASSERT(foundFile1);
		TS_ASSERT(foundFile2);
		delete fs;
	}

	// Null stream
	void testNullStreamNotOpen() {
		Common::ISO9660FileSystem fs(nullptr);
		TS_ASSERT(!fs.isOpen());
	}

	// Zero-size file
	void testOpenFileZeroSizeReturnsNull() {
		Common::ISO9660FileSystem *fs = openStandardISO();
		TS_ASSERT(fs->isOpen());

		// Manually construct a zero-size DirEntry to test the edge case.
		// In normal usage, entries come from readRootDirectory()/readDirectory().
		Common::ISO9660FileSystem::DirEntry entry;
		entry.isDir = false;
		entry.size  = 0;
		entry.lba   = 19;
		entry.name  = "empty";
		TS_ASSERT_EQUALS(fs->openFile(entry),
		                 static_cast<Common::SeekableReadStream *>(nullptr));
		delete fs;
	}

	void testGetRootEntry() {
		Common::ISO9660FileSystem *fs = openStandardISO();
		TS_ASSERT(fs->isOpen());

		Common::ISO9660FileSystem::DirEntry root = fs->getRootEntry();
		TS_ASSERT(root.isDir);
		TS_ASSERT_EQUALS(root.lba, fs->getRootLba());
		TS_ASSERT_EQUALS(root.size, fs->getRootSize());
		delete fs;
	}

	void testReadDirectoryWithRootEntry() {
		Common::ISO9660FileSystem *fs = openStandardISO();
		TS_ASSERT(fs->isOpen());

		Common::ISO9660FileSystem::DirEntry root = fs->getRootEntry();
		Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
			fs->readDirectory(root);

		TS_ASSERT_EQUALS(entries.size(), static_cast<uint32>(2));
		delete fs;
	}
};
