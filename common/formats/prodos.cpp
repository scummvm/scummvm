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

#include "common/formats/prodos.h"

namespace Common {

// --- ProDOSFile methods ---

ProDOSFile::ProDOSFile(char name[16], uint8 type, uint16 tBlk, uint32 eof, uint16 bPtr, Common::File *disk)
	: _type(type)
	, _totalBlocks(tBlk)
	, _eof(eof)
	, _blockPtr(bPtr)
	, _disk(disk) {
	strncpy(_name, name, 16);
}

/* For debugging purposes, this prints the meta data of a file */

void ProDOSFile::printInfo() {
	debug("File: %s", _name);
	debug("Type: %02X", _type);
	debug("data: %d", _blockPtr);
	debug("Blocks: %d", _totalBlocks);
	debug("Size: %u\n", _eof);
}

/* For Common::Archive, this method just returns a string of the name */

Common::String ProDOSFile::getName() const {
	return Common::String(_name);
}

Common::String ProDOSFile::getFileName() const {
	return Common::String(_name);
}

Common::Path ProDOSFile::getPathInArchive() const {
	return Common::Path(_name);
}

/* This method is used to get a single block of data from the disk,
 * but is not strictly 512 bytes. This is so that it can get only what
 * it needs when in the final block. It then adds it into the allocated
 * memory starting at memOffset
 */

void ProDOSFile::getDataBlock(byte *memOffset, int offset, int size) const {

	// All this method needs to do is read (size) of data at (offset) into (memOffset)
	_disk->seek(offset);
	_disk->read(memOffset, size);
}

/* To put together a sapling file, you need to loop through the index
 * block, adding to the file data one block at a time. This method also
 * returns the size of data it got, just to make it a little simpler to
 * determine the new position within the byte data.
 */

int ProDOSFile::parseIndexBlock(byte *memOffset, int blockNum, int rem) const {
	int dataSize;               // For most of the blocks, this will be kBlockSize, but the last one will be the calculated remainder
	int readSize = 0;           // This keeps track of the new pointer position to read data to, by updating the size of data read last
	int dataOffset;             // Where in the disk to read from
	int diskPos;                // Current position of cursor

	for (int i = 0; i < blockNum; i++) {
		dataSize   = (i == (blockNum - 1)) ? rem : ProDOSDisk::kBlockSize;
		dataOffset = _disk->readByte();         // Low byte is first

		/* The cursor needs to know where to get the next pointer from in the index block,
		 * but it also needs to jump to the offset of data to read it, so we need to preserve
		 * the position in the index block it was in before.
		 */
		diskPos = _disk->pos();

		_disk->skip(255);                       // The high bytes are stored at the end of the block
		dataOffset = (dataOffset + (_disk->readByte() << 8)) * ProDOSDisk::kBlockSize;    // High byte is second

		getDataBlock(memOffset + readSize, dataOffset, dataSize);
		readSize += dataSize;

		// And now we resume the position before this call
		_disk->seek(diskPos);
	}
	return readSize;
}

/* Extracting file data is a little tricky, as the blocks are spread out in the disk. There are 3 types
 * of regular files. Seed, Sapling, and Tree. A Seed file only needs a single data block, while a
 * Sapling needs an index block to manage up to 256 data blocks, and a Tree file needs an index block
 * to manage up to 128 (only uses half the block) index blocks. This is also an Archive method as it
 * returns a read stream of the file contents.
 */

Common::SeekableReadStream *ProDOSFile::createReadStream() const {

	// We know the total byte size of the data, so we can allocate the full amount right away
	byte *finalData = (byte *)malloc(_eof);

	/* For a seed, this is a direct pointer to data. For a sapling it is an index file,
	 * and for a tree it is a master index file.
	 */
	int indexBlock = _blockPtr * ProDOSDisk::kBlockSize;

	/* For a sapling or tree, the size needs to be calculated, as they are made from multiple blocks.
	 * _totalBlocks *includes* the index block, so the blocks before the oef block are _totalBlocks-2
	 */
	int remainder = _eof - ((_totalBlocks - 2) * ProDOSDisk::kBlockSize);

	// For a seed file, the end of file value is also the size in the block, because it's just the one block
	if (_type == kFileTypeSeed) {
		getDataBlock(finalData, indexBlock, _eof);

	} else if (_type == kFileTypeSapling) {
		_disk->seek(indexBlock);
		parseIndexBlock(finalData, _totalBlocks - 1, remainder);

	} else {
		// If it's not a seed and not a sapling, it's a tree.
		_disk->seek(indexBlock);

		/* A sapling can have an index block of up to 256, so if it is a tree,
		 * that means it has more than 256 blocks
		 */
		int indexNum  = (_totalBlocks - 1) / 256;
		int indexNumR = (_totalBlocks - 1) % 256;

		/* However, to know how many index blocks there are, we need to know the remainder
		 * so we can figure out if it's ex. 2 index blocks, or 2 and some portion of a 3rd
		 */
		indexNum += indexNumR;
		int blockNum;
		int indexOffset;
		int readSize = 0;

		// Now we can loop through the master index file, parsing the individual index files similar to a sapling
		for (int i = 0; i < indexNum; i++) {
			blockNum = (i == indexNum - 1) ? indexNumR : 256;

			indexOffset = _disk->readByte();
			int diskPos = _disk->pos();

			_disk->skip(255);
			indexOffset = (indexOffset + (_disk->readByte() << 8)) * ProDOSDisk::kBlockSize;

			_disk->seek(indexOffset);
			readSize += parseIndexBlock(finalData + readSize, blockNum, remainder);

			_disk->seek(diskPos);
		}
	}
	return new Common::MemoryReadStream(finalData, _eof, DisposeAfterUse::YES);
}

Common::SeekableReadStream *ProDOSFile::createReadStreamForAltStream(Common::AltStreamType altStreamType) const {
	return nullptr;
}

// --- ProDOSDisk methods ---

/* The time and date are compressed into 16bit words, so to make them useable
 * we have to decompress them by masking the other bits and then shifting
 * to the lowest bit so that they can be stored in 8 bits each.
 */

void ProDOSDisk::getDate(Date *d, uint16 date) {
	d->_day   =  date & 0x001f;
	d->_month = (date & 0x01e0) >> 5;
	d->_year  = (date & 0xfe00) >> 9;
}

void ProDOSDisk::getTime(Time *t, uint16 time) {
	t->_minute =  time & 0x003f;
	t->_hour   = (time & 0x1f00) >> 8;
}

/* Adds most of the header data to a directory header struct */

void ProDOSDisk::getHeader(DirHeader *h) {

	/* The type and nameLen fields are stored in the same byte,
	 * so we need to split the byte, and shift the high bits to
	 * make it readable as an int
	 */
	uint8 tempByte = _disk.readByte();
	h->_nameLen = tempByte & 0xf;
	h->_type = (tempByte & 0xf0) >> 4;

	/* The name field is stored in 15 bytes with no null character (unused chars default to 0).
	 * To make it easier to use the name, we will add a terminator regardless.
	 */
	_disk.read(h->_name, 15);
	h->_name[15] = 0;
	_disk.read(h->_reserved, 8);

	// The time and date can be decompressed into structs right away
	getDate(&(h->_date), _disk.readUint16LE());
	getTime(&(h->_time), _disk.readUint16LE());

	h->_ver = _disk.readByte();
	h->_minVer = _disk.readByte();
	h->_access = _disk.readByte();
	h->_entryLen = _disk.readByte();
	h->_entriesPerBlock = _disk.readByte();
	h->_fileCount = _disk.readUint16LE();
}

/* Since a subdirectory header is mostly the same a volume header, we will reuse the code where we can */

void ProDOSDisk::getDirectoryHeader(DirHeader *h) {
	getHeader(h);
	h->_parentBlockPtr   = _disk.readUint16LE();
	h->_parentEntryIndex = _disk.readByte();
	h->_parentEntryLen   = _disk.readUint16LE();
}

/* This is a little sneaky, but since the bulk of the header is the same, we're just going to pretend the volume header
 * is a directory header for the purose of filling it out with the same code
 */

void ProDOSDisk::getVolumeHeader(VolHeader *h) {
	getHeader((DirHeader *)h);
	h->_bitmapPtr = _disk.readUint16LE();
	h->_volBlocks = _disk.readUint16LE();
	_volBlocks = h->_volBlocks;
}

/* Getting a file entry header is very similar to getting a header, but with different data. */

void ProDOSDisk::getFileEntry(FileEntry *f) {
	uint8 tempByte = _disk.readByte();
	f->_nameLen = tempByte & 0xf;
	f->_type = (tempByte & 0xf0) >> 4;

	_disk.read(f->_name, 15);
	f->_name[15] = 0;
	f->_ext = _disk.readByte();
	f->_blockPtr = _disk.readUint16LE();
	f->_totalBlocks = _disk.readUint16LE();

	// The file size in bytes is stored as a long (3 bytes), lowest to highest
	f->_eof = _disk.readByte() + (_disk.readByte() << 8) + (_disk.readByte() << 16);

	getDate(&(f->_date), _disk.readUint16LE());
	getTime(&(f->_time), _disk.readUint16LE());

	f->_ver = _disk.readByte();
	f->_minVer = _disk.readByte();
	f->_access = _disk.readByte();
	f->_varUse = _disk.readUint16LE();

	getDate(&(f->_modDate), _disk.readUint16LE());
	getTime(&(f->_modTime), _disk.readUint16LE());

	f->_dirHeadPtr = _disk.readUint16LE();
}

/* This is basically a loop based on the number of total files indicated by the header (including deleted file entries),
 * which parses the file entry, and if it is a regular file (ie. active and not a pascal area) then it will create a file object.
 * If it is instead a subdirectory file entry, it will use this same function to search in that directory creating files
 * and continue like that until all directories have been explored. Along the way it puts together the current file path,
 * which is stored with the file object so that the engine can search by path name.
 */

void ProDOSDisk::searchDirectory(DirHeader *h, uint16 p, uint16 n, Common::String path) {
	// NB: p for previous set, but not currently used. This debug message silences any set-but-unused compiler warnings
	debug(10, "searchDirectory(h:%p prev: %d next:%d, path:%s", (void *)h, p, n, path.c_str());
	int currPos;
	int parsedFiles = 0;

	for (int i = 0; i < h->_fileCount; i++) {
		// When we have read all the files for a given block (_entriesPerBlock), we need to change to the next block of the directory
		if (parsedFiles == h->_entriesPerBlock) {
			parsedFiles = 0;
			_disk.seek(n * kBlockSize);
			p = _disk.readUint16LE();
			n = _disk.readUint16LE();
		}

		FileEntry fileEntry;
		getFileEntry(&fileEntry);
		parsedFiles++;
		currPos = _disk.pos();

		// It is a regular file if (dead < file type < pascal) and the file has a size
		if ((kFileTypeDead < fileEntry._type) && (fileEntry._type < kFileTypePascal) && (fileEntry._eof > 0)) {
			Common::String fileName = path + fileEntry._name;
			ProDOSFile *currFile = new ProDOSFile(fileEntry._name, fileEntry._type, fileEntry._totalBlocks, fileEntry._eof, fileEntry._blockPtr, &_disk);

			_files.setVal(fileName, Common::SharedPtr<ProDOSFile>(currFile));
			_disk.seek(currPos);

			// Otherwise, if it is a subdirectory, we want to explore that subdirectory
		} else if (fileEntry._type == kFileTypeSubDir) {
			_disk.seek(fileEntry._blockPtr * kBlockSize);

			uint16 subP = _disk.readUint16LE();
			uint16 subN = _disk.readUint16LE();
			DirHeader subHead;
			getDirectoryHeader(&subHead);

			// Give it a temporary new path name by sticking the name of the subdirectory on to the end of the current path
			Common::String subPath = Common::String(path + subHead._name + '/');
			searchDirectory(&subHead, subP, subN, path);

			_disk.seek(currPos);
		}
	}
}

/* The volume bitmap is a bitmap spanning as many blocks as is required to store 1 bit for every
 * block on the disk. There are 8 bits per byte and 512 bytes per block, so it needs
 * ((total_blocks / 4096) + 1 (if remainder)) * 512 bytes.
 */

void ProDOSDisk::getVolumeBitmap(VolHeader *h) {
	int currPos = _disk.pos();
	int bitmapSize;

	bitmapSize = _volBlocks / 4096;
	if ((_volBlocks % 4096) > 0) {
		bitmapSize++;
	}

	_volBitmap = (byte *)malloc(bitmapSize * kBlockSize);
	_disk.seek(h->_bitmapPtr * kBlockSize);
	_disk.read(_volBitmap, bitmapSize);

	_disk.seek(currPos);
}

/* Gets the volume information and parses the filesystem, adding file objects to a map as it goes */

bool ProDOSDisk::open(const Common::Path &filename) {
	_disk.open(filename);
	_disk.read(_loader1, kBlockSize);
	_disk.read(_loader2, kBlockSize);

	uint16 prev = _disk.readUint16LE();             // This is always going to be 0 for the volume header, but there's also no reason to skip it
	uint16 next = _disk.readUint16LE();

	VolHeader header;
	getVolumeHeader(&header);
	getVolumeBitmap(&header);

	Common::String pathName;                        // This is so that the path name starts blank, and then for every directory searched it adds the directory name to the path
	searchDirectory((DirHeader *)&header, prev, next, pathName);

	return true;                                    // When I get to error checking on this, the bool will be useful
}

/* Constructor simply calls open(), and if it is successful it prints a statement */

ProDOSDisk::ProDOSDisk(const Common::Path &filename) {
	if (open(filename)) {
		//debug("%s has been loaded", filename.c_str());
	}
}

/* Destructor closes the disk and clears the map of files */

ProDOSDisk::~ProDOSDisk() {
	_disk.close();
	_files.clear();
	free(_volBitmap);                               // Should this be free() or delete?
}

// --- Common::Archive methods ---

// Very simple, just checks if the dictionary contains the path name
bool ProDOSDisk::hasFile(const Common::Path &path) const {
	Common::String name = path.toString();
	return _files.contains(name);
}

/* To create a list of files in the Archive, we define an iterator for the object type
 * used by the Archive member, and then loop through the hashmap, adding the object
 * pointer returned as the value from the given path. This also returns the size.
 */

int ProDOSDisk::listMembers(Common::ArchiveMemberList &list) const {
	int f = 0;
	Common::HashMap<Common::String, Common::SharedPtr<ProDOSFile>>::const_iterator it;
	for (it = _files.begin(); it != _files.end(); ++it) {
		list.push_back(Common::ArchiveMemberList::value_type(it->_value));
		++f;
	}
	return f;
}

// If the dictionary contains the path name (could probably call hasFile() instead), get the object
const Common::ArchiveMemberPtr ProDOSDisk::getMember(const Common::Path &path) const {
	Common::String name = path.toString();
	if (!_files.contains(name)) {
		return Common::ArchiveMemberPtr();
	}
	return _files.getValOrDefault(name);
}

/* This method is called on Archive members as it searches for the correct one,
 * so if this member is not the correct one, we return a null pointer.
 */

Common::SeekableReadStream *ProDOSDisk::createReadStreamForMember(const Common::Path &path) const {
	Common::String name = path.toString();
	if (!_files.contains(name)) {
		return nullptr;
	}
	Common::SharedPtr<ProDOSFile> f = _files.getValOrDefault(name);
	return f->createReadStream();
}

} // Namespace Common
