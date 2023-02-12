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

#ifndef COMMON_PRODOS_H
#define COMMON_PRODOS_H

#include "common/memstream.h"
#include "common/file.h"
#include "common/debug.h"
#include "common/error.h"

/* Quick note about ProDOS:
 * This disk code handles inactive, seedling, sapling, tree, and subdirectory files.
 * It does *not* handle sparse files at the moment. If a sparse file exists, it may not
 * be read correctly. It also does not do anything with Pascal files, but those should not
 * matter for game engines anyway.
 */

namespace Common {

// These values define for ProDOS how to read the file entry, and also whether it's a keyblock (if it is a directory header, it's the keyblock of that directory)
enum FileType : char {
	kFileTypeDead    = 0,
	kFileTypeSeed    = 1,
	kFileTypeSapling = 2,
	kFileTypeTree    = 3,
	kFileTypePascal  = 4,
	kFileTypeSubDir  = 0x0D,
	kFileTypeSubHead = 0x0E,
	kFileTypeVolHead = 0x0F
};

/* File extensions for all the ProDOS supported file types
 * NOTE: ProDOS user defined files are F1-F8. If they are ever required,
 * they can be added to this enum.
 */
enum FileExt {
	kFileExtNull     = 0,
	kFileExtBad      = 1,
	kFileExtTxt      = 4,
	kFileExtBin      = 6,
	kFileExtGfx      = 8,
	kFileExtDir      = 0xF,
	kFileExtDB       = 0x19,
	kFileExtWord     = 0x1A,
	kFileExtSpread   = 0x1B,
	kFileExtSTART    = 0xB3,
	kFileExtPascal   = 0xEF,
	kFileExtPDCI     = 0xF0,
	kFileExtPDRes    = 0xF9,
	kFileExtIBProg   = 0xFA,
	kFileExtIBVar    = 0xFB,
	kFileExtAPSProg  = 0xFC,
	kFileExtAPSVar   = 0xFD,
	kFileExtEDASM    = 0xFE,
	kFileExtSYS      = 0xFF
};

/* A ProDOS file simply contains meta data about the file and the ability to
 * find and put together the data blocks that make up the file contents.
 * This implements Common::ArchiveMember so that it can be used directly in
 * the Archive methods in ProDOSDisk.
 */

class ProDOSFile : public Common::ArchiveMember {
public:
	ProDOSFile(char name[16], uint8 type, uint16 tBlk, uint32 eof, uint16 bPtr, Common::File *disk);
	~ProDOSFile() {};                                            // File does not need a destructor, because the file it reads from is a pointer to Disk, and Disk has a destructor

	// -- These are the Common::ArchiveMember related functions --
	Common::String getName() const override;                              // Returns _name
	Common::SeekableReadStream *createReadStream() const override;        // This is what the archive needs to create a file
	void getDataBlock(byte *memOffset, int offset, int size) const;       // Gets data up to the size of a single data block (512 bytes)
	int parseIndexBlock(byte *memOffset, int blockNum, int cSize) const;  // Uses getDataBlock() on every pointer in the index file, adding them to byte * memory block

	// For debugging purposes, just prints the metadata
	void printInfo();

private:
	char   _name[16];
	uint8  _type;                           // As defined by enum FileType
	uint16 _blockPtr;                       // Block index in volume of index block or data
	uint16 _totalBlocks;
	uint32 _eof;                            // End Of File, used generally as size (exception being sparse files)
	Common::File *_disk;                    // This is a pointer because it is the same _disk as in ProDosDisk, passed to the file object
};

/* This class defines the entire disk volume. Upon using the open() method,
 * it will parse the volume and add them to a hashmap where the file path
 * returns a file object. This implements Common::Archive to allow regular
 * file operations to work with it.
 */

class ProDOSDisk : public Common::Archive {
public:
	static const int kBlockSize = 512;		// A ProDOS block is always 512 bytes (should this be an enum?)

	ProDOSDisk(const Common::String filename);
	~ProDOSDisk();							// Frees the memory used in the dictionary and the volume bitmap

	// Called from the constructor, it parses the volume and fills the hashmap with files
	bool open(const Common::String filename);

	// These are the Common::Archive related methods
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	byte _loader1[kBlockSize];				// There's not much reason for these to be needed, but I included them just in case
	byte _loader2[kBlockSize];
	Common::String _name;					// Name of volume
	Common::File _disk;						// The volume file itself
	int _volBlocks;							// Total blocks in volume
	byte *_volBitmap;						// This can determine if the volume is corrupt as it contains a bit for every block, where 0 = unused, 1 = used
	Common::HashMap<Common::String, Common::SharedPtr<ProDOSFile>> _files; // Hashmap of files in the volume, where key=Path, Value=ProDOSFile

	struct Date {
		uint8 _day;
		uint8 _month;
		uint8 _year;
	};

	struct Time {
		uint8 _hour;
		uint8 _minute;
	};

	struct VolHeader {
		uint8 _type;						// Not really important for a volume header, as this will always be F
		uint8 _nameLen;
		char _name[16];
		byte _reserved[8];					// Extra space reserved for possible future uses, not important
		Date _date;
		Time _time;
		uint8 _ver;
		uint8 _minVer;						// Should pretty much always be 0 as far as I know
		uint8 _access;						// If this ends up useful, there should be an enum for the access values
		uint8 _entryLen;					// Always 27 in ProDOS 1.0
		uint8 _entriesPerBlock;				// Always 0D in ProDOS 1.0
		uint16 _fileCount;                  // Number of files across all data blocks in this directory
		uint16 _bitmapPtr;                  // Block pointer to the keyblock of the bitmap for the entire volume
		uint16 _volBlocks;                  // Blocks in entire volume
	};

	struct DirHeader {
		uint8 _type;
		uint8 _nameLen;
		char _name[16];
		byte _reserved[8];
		Date _date;
		Time _time;
		uint8 _ver;
		uint8 _minVer;
		uint8 _access;
		uint8 _entryLen;
		uint8 _entriesPerBlock;
		uint16 _fileCount;
		uint16 _parentBlockPtr;             // These values allow ProDOS to navigate back out of a directory, but they aren't really needed by the class to navigate
		uint8 _parentEntryIndex;			// Index in the current directory
		uint8 _parentEntryLen;				// This is always 27 in ProDOS 1.0
	};

	struct FileEntry {
		uint8 _type;						// 0 = inactive, 1-3 = file, 4 = pascal area, 14 = subdirectory, 15 = volume directory
		uint8 _nameLen;
		char _name[16];
		uint8 _ext;							// File extension, uses the enum FileExt
		uint16 _blockPtr;                   // Block pointer to data for seedling, index block for sapling, or master block for tree
		uint16 _totalBlocks;                // Really important to remember this is the total *including* the index block
		uint32 _eof;                        // This is a long (3 bytes, read low to high) value representing the total readable data in a file (unless it's a sparse file, be careful!)
		Date _date;
		Time _time;
		uint8 _ver;
		uint8 _minVer;
		uint8 _access;
		uint16 _varUse;
		Date _modDate;
		Time _modTime;
		uint16 _dirHeadPtr;                 // Pointer to the key block of the directory that contains this file entry
	};

	void getDate(Date *d, uint16 date);     // Decompresses the date into a struct
	void getTime(Time *t, uint16 time);     // Decompresses the time into a struct
	void getHeader(DirHeader *h);           // Adds the main header values to the struct
	void getDirectoryHeader(DirHeader *h);  // Uses getHeader and then fills in the values for the parent directory
	void getVolumeHeader(VolHeader *dir);   // Uses getHeader and then fills in the volume related information (there is no parent directory to this one)
	void getFileEntry(FileEntry *f);        // Adds all of the file entry information to the struct
	void searchDirectory(DirHeader *h, uint16 p, uint16 n, Common::String path);  // Recursively searches all files within a directory, by calling itself for subdirectories
	void getVolumeBitmap(VolHeader *h);     // Puts together the volume bitmap
};


} // Namespace Common

#endif
