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

#include "agi/agi.h"
#include "agi/disk_image.h"
#include "agi/loader.h"
#include "agi/words.h"

#include "common/fs.h"

namespace Agi {

// GalLoader reads KQ1 PC Booter floppy disk images.
//
// All disks are 360k. The only supported image format is "raw". There are no
// headers, footers, or metadata. Each image file must be exactly 368,640 bytes.
//
// All KQ1 PC booter versions are only one disk.
//
// The disks do not use a standard file system. Instead, file locations are
// stored in a directory structure at known locations.
//
// File detection is done a little differently. Instead of requiring hard-coded
// names for the image files, we scan the game directory for the first usable
// disk image file. The only naming requirement is that the image has a known
// file extension.
//
// AgiMetaEngineDetection also scans for usable disk images. It finds and hashes
// the logic directory inside the disk, and matches against the detection table.

/**
 * Locates the disk image and the disk offset of the resource directory
 */
void GalLoader::init() {
	// build sorted array of files with image extensions
	Common::Array<Common::Path> imageFiles;
	FileMap fileMap;
	getPotentialDiskImages(pcDiskImageExtensions, ARRAYSIZE(pcDiskImageExtensions), imageFiles, fileMap);

	// find the disk by reading potential images until successful
	for (uint i = 0; i < imageFiles.size(); i++) {
		const Common::Path &imageFile = imageFiles[i];
		Common::SeekableReadStream *stream = openPCDiskImage(imageFile, fileMap[imageFile]);
		if (stream == nullptr) {
			continue;
		}

		// look for the directory in both locations
		if (isDirectory(*stream, GAL_DIR_POSITION_PCJR)) {
			_imageFile = imageFile.baseName();
			_dirOffset = GAL_DIR_POSITION_PCJR;
		} else if (isDirectory(*stream, GAL_DIR_POSITION_PC)) {
			_imageFile = imageFile.baseName();
			_dirOffset = GAL_DIR_POSITION_PC;
		}

		delete stream;
		if (!_imageFile.empty()) {
			break;
		}
	}

	if (_imageFile.empty()) {
		warning("GalLoader: disk not found");
	}
}

/**
 * Identifies the directory by validating the first few logic entries
 */
bool GalLoader::isDirectory(Common::SeekableReadStream &stream, uint32 dirOffset) {
	for (int i = 0; i < 10; i++) {
		stream.seek(dirOffset + (i * 4));

		uint32 sectorCount;
		uint32 logicOffset = readDirectoryEntry(stream, &sectorCount);

		stream.seek(logicOffset);
		uint32 logicSize = 8;
		for (int j = 0; j < 4; j++) {
			logicSize += stream.readUint16LE();
		}

		if (stream.eos()) {
			return false;
		}

		stream.seek(logicOffset + logicSize - 1);
		byte logicTerminator = stream.readByte();
		if (stream.eos() || logicTerminator != 0xff) {
			return false;
		}
	}
	return true;
}

/**
 * Reads a directory entry.
 *
 * Returns the disk offset and the resource size in sectors.
 */
uint32 GalLoader::readDirectoryEntry(Common::SeekableReadStream &stream, uint32 *sectorCount) {
	//  9 bit offset (last bit is MSB)
	//  6 bit zero
	//  5 bit sector count
	//  2 bit zero
	// 10 bit sector
	byte b0 = stream.readByte();
	byte b1 = stream.readByte();
	byte b2 = stream.readByte();
	byte b3 = stream.readByte();

	uint16 offset = ((b1 & 0x80) << 1) | b0;
	uint16 sector = ((b2 & 0x03) << 8) | b3;

	*sectorCount = ((b1 & 0x01) << 4) | (b2 >> 4);
	return (sector * 512) + offset;
}

int GalLoader::loadDirs() {
	// if init didn't find disk then fail
	if (_imageFile.empty()) {
		return errFilesNotFound;
	}

	// open disk
	Common::File disk;
	if (!disk.open(Common::Path(_imageFile))) {
		return errBadFileOpen;
	}

	// load logic and picture directory entries.
	// pictures do not have directory entries. each picture immediately follows
	// its logic. if there is no real picture then it is just the FF terminator.
	uint32 sectorCount;
	for (int i = 0; i < 84; i++) {
		disk.seek(_dirOffset + (i * 4));
		uint32 logicOffset = readDirectoryEntry(disk, &sectorCount);

		// seek to logic and calculate length from header
		disk.seek(logicOffset);
		uint32 logicLength = 8;
		for (int j = 0; j < 4; j++) {
			logicLength += disk.readUint16LE();
		}
		if (disk.eos()) {
			return errBadResource;
		}

		// scan for picture terminator after logic
		uint32 pictureOffset = logicOffset + logicLength;
		disk.seek(pictureOffset);
		uint32 pictureLength = 0;
		while (true) {
			byte terminator = disk.readByte();
			if (disk.eos()) {
				return errBadResource;
			}
			if (terminator == 0xff) {
				pictureLength = disk.pos() - pictureOffset;
				break;
			}
		}

		_vm->_game.dirLogic[i].offset = logicOffset;
		_vm->_game.dirLogic[i].len    = logicLength;
		_vm->_game.dirPic[i].offset   = pictureOffset;
		_vm->_game.dirPic[i].len      = pictureLength;
	}

	// load sound directory entries
	for (int i = 0; i < 10; i++) {
		disk.seek(_dirOffset + ((90 + i) * 4));
		uint32 soundOffset = readDirectoryEntry(disk, &sectorCount);

		// seek to sound and calculate length from header
		disk.seek(soundOffset);
		uint32 soundLength = 8;
		for (int j = 0; j < 4; j++) {
			soundLength += disk.readUint16LE();
		}
		if (disk.eos()) {
			return errBadResource;
		}

		_vm->_game.dirSound[i].offset = soundOffset;
		_vm->_game.dirSound[i].len    = soundLength;
	}

	// load view directory entries
	for (int i = 0; i < 110; i++) {
		disk.seek(_dirOffset + ((128 + i) * 4));
		uint32 viewOffset = readDirectoryEntry(disk, &sectorCount);

		// seek to view and calculate length from header
		disk.seek(viewOffset);
		uint32 viewLength = 2 + disk.readUint16LE();
		if (disk.eos()) {
			return errBadResource;
		}

		_vm->_game.dirView[i].offset = viewOffset;
		_vm->_game.dirView[i].len    = viewLength;
	}

	return errOK;
}

uint8 *GalLoader::loadVolumeResource(AgiDir *agid) {
	Common::File disk;
	if (!disk.open(Common::Path(_imageFile))) {
		warning("GalLoader: unable to open disk image: %s", _imageFile.c_str());
		return nullptr;
	}

	// read resource
	uint8 *data = (uint8 *)calloc(1, agid->len);
	disk.seek(agid->offset);
	if (disk.read(data, agid->len) != agid->len) {
		warning("GalLoader: error reading %d bytes at offset %d", agid->len, agid->offset);
		free(data);
		return nullptr;
	}

	return data;
}

// TODO
int GalLoader::loadObjects() {
	return errOK;
}

// TODO
int GalLoader::loadWords() {
	return errOK;
}

} // End of namespace Agi
