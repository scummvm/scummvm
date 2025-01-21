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

// GalLoader_A2 reads KQ1 Apple II floppy disk images.
//
// Floppy disks have two sides; each side is a disk with its own image file.
// All disk sides are 140k with 35 tracks and 16 sectors per track.
//
// KQ1 has three disk sides (labeled A, B, C) on two physical disks.
//
// Multiple disk image formats are supported; see Common::DiskImage. The file
// extension determines the format. For example: .do, .dsk, .nib, .woz.
//
// The disks do not use a standard file system. Instead, file locations are
// stored in directory structures at known locations.
//
// File detection is done a little differently. Instead of requiring hard-coded
// names for the image files, we scan the game directory for the first usable
// image of disk one, and then continue scanning until all disks are found.
// The directory from disk one is used to identify each disk by its content.
// The only naming requirement is that the images have a known file extension.
//
// AgiMetaEngineDetection also scans for usable disk images. It finds and hashes
// the logic directory inside disk one, and matches against the detection table.

GalLoader_A2::~GalLoader_A2() {
	for (uint d = 0; d < _disks.size(); d++) {
		delete _disks[d];
	}
}

/**
 * Locates the three disk images.
 */
void GalLoader_A2::init() {
	// build sorted array of files with image extensions
	Common::Array<Common::Path> imageFiles;
	FileMap fileMap;
	getPotentialDiskImages(a2DiskImageExtensions, ARRAYSIZE(a2DiskImageExtensions), imageFiles, fileMap);

	// find disk one by reading potential images until successful
	_disks.clear();
	AgiDir logicDir[GAL_A2_LOGIC_COUNT];
	uint diskOneIndex;
	for (diskOneIndex = 0; diskOneIndex < imageFiles.size(); diskOneIndex++) {
		const Common::Path &imageFile = imageFiles[diskOneIndex];
		Common::SeekableReadStream *stream = openA2DiskImage(imageFile, fileMap[imageFile]);
		if (stream == nullptr) {
			warning("GalLoader_A2: unable to open disk image: %s", imageFile.baseName().c_str());
			continue;
		}

		// read image as disk one
		if (readDiskOne(*stream, logicDir)) {
			debugC(3, "GalLoader_A2: disk one found: %s", imageFile.baseName().c_str());
			_disks.resize(GAL_A2_DISK_COUNT);
			_disks[0] = stream;
			break;
		} else {
			delete stream;
		}
	}

	// if disk one wasn't found, we're done
	if (_disks.empty()) {
		warning("GalLoader_A2: disk one not found");
		return;
	}

	// find all other disks by comparing their contents to the logic directory.
	int disksFound = 1;
	for (uint i = 1; i < imageFiles.size() && disksFound < GAL_A2_DISK_COUNT; i++) {
		uint imageFileIndex = (diskOneIndex + i) % imageFiles.size();
		Common::Path &imageFile = imageFiles[imageFileIndex];

		Common::SeekableReadStream *stream = openA2DiskImage(imageFile, fileMap[imageFile]);
		if (stream == nullptr) {
			continue;
		}

		// check each disk
		bool diskFound = false;
		for (int d = 1; d < GAL_A2_DISK_COUNT; d++) {
			// has disk already been found?
			if (_disks[d] != nullptr) {
				continue;
			}

			if (validateDisk(*stream, d, logicDir)) {
				_disks[d] = stream;
				disksFound++;
				diskFound = true;
				break;
			}
		}

		if (!diskFound) {
			delete stream;
		}
	}
}

/**
 * Reads a disk image as disk one by attempting to parse the logic directory
 * and then validating that all the expected logic resources exist.
 */
bool GalLoader_A2::readDiskOne(Common::SeekableReadStream &disk, AgiDir *logicDir) {
	disk.seek(GAL_A2_LOGDIR_POSITION);

	// attempt to read logic directory
	for (int i = 0; i < GAL_A2_LOGIC_COUNT; i++) {
		if (!readDirectoryEntry(disk, logicDir[i])) {
			return false;
		}
	}

	// validate that all disk one logics exist
	return validateDisk(disk, 0, logicDir);
}

/**
 * Reads a directory entry.
 */
bool GalLoader_A2::readDirectoryEntry(Common::SeekableReadStream &stream, AgiDir &dirEntry) {
	// GAL A2 DIR format:
	// track   8 bits
	// disk    4 bits (0 for all disks, else 1-3)
	// sector  4 bits
	// offset  8 bits
	byte b0 = stream.readByte();
	byte b1 = stream.readByte();
	byte b2 = stream.readByte();

	byte disk = b1 >> 4;
	byte sector = b1 & 0x0f;
	uint32 position = A2_DISK_POSITION(b0, sector, b2);

	// use the first disk for resources that are on all disks
	if (disk > 0) {
		disk--;
	}

	// validate entry
	if (!(disk <= 2 && position < A2_DISK_SIZE)) {
		return false;
	}

	dirEntry.volume = disk;
	dirEntry.offset = position;
	return true;
}

/**
 * Tests if a disk contains all of the expected logic resources.
 */
bool GalLoader_A2::validateDisk(Common::SeekableReadStream &disk, byte diskIndex, AgiDir *logicDir) {
	for (int i = 0; i < GAL_A2_LOGIC_COUNT; i++) {
		// Only validate logics on this disk
		if (logicDir[i].volume != diskIndex) {
			continue;
		}

		// Do not use logic 64 to validate a disk. Its logic header contains
		// an incorrect length that is one byte too large. This would fail our
		// validation method below of comparing the resource length in the A2
		// header to the length in the logic header.
		if (i == 64) {
			continue;
		}

		// A2 resources begin with a header consisting of the resource length.
		// Logic resources begin with a header consisting of four lengths; one
		// for each section of the logic. If a logic exists at this location,
		// then the A2 length will equal the calculated logic length.
		disk.seek(logicDir[i].offset);
		uint16 resourceLength = disk.readUint16LE();
		uint32 logicLength = 8;
		for (int j = 0; j < 4; j++) {
			logicLength += disk.readUint16LE();
		}
		if (disk.eos() ||
			resourceLength != logicLength ||
			!(logicDir[i].offset + 2 + resourceLength <= A2_DISK_SIZE)) {
			return false;
		}
	}
	return true;
}

/**
 * Load logic, pic, and view directories. KQ1-A2 has no sound resources.
 */
int GalLoader_A2::loadDirs() {
	// if init didn't find disks then fail
	if (_disks.empty()) {
		return errFilesNotFound;
	}
	for (uint d = 0; d < _disks.size(); d++) {
		if (_disks[d] == nullptr) {
			warning("AgiLoader_A2: disk %d not found", d);
			return errFilesNotFound;
		}
	}

	// directories are on disk one
	Common::SeekableReadStream &disk = *_disks[0];

	bool success = true;
	success &= loadDir(_vm->_game.dirLogic, disk, GAL_A2_LOGDIR_POSITION,  GAL_A2_LOGIC_COUNT);
	success &= loadDir(_vm->_game.dirPic,   disk, GAL_A2_PICDIR_POSITION,  GAL_A2_PICTURE_COUNT);
	success &= loadDir(_vm->_game.dirView,  disk, GAL_A2_VIEWDIR_POSITION, GAL_A2_VIEW_COUNT);
	return success ? errOK : errBadResource;
}

/**
 * Loads a resource directory.
 */
bool GalLoader_A2::loadDir(AgiDir *dir, Common::SeekableReadStream &disk, uint32 dirOffset, uint32 dirCount) {
	disk.seek(dirOffset);
	for (uint32 i = 0; i < dirCount; i++) {
		// Skip pictures 0 and 81. These pictures do not exist, but the entries
		// contain junk bytes. This did not matter in the original because they
		// never loaded, but if we read them then they will fail validation.
		if ((i == 0 || i == 81) && dirOffset == GAL_A2_PICDIR_POSITION) {
			disk.skip(3);
			continue;
		}

		if (!readDirectoryEntry(disk, dir[i])) {
			return false;
		}
	}
	return true;
}

uint8 *GalLoader_A2::loadVolumeResource(AgiDir *agid) {
	if (agid->volume >= _disks.size()) {
		warning("GalLoader_A2: invalid volume: %d", agid->volume);
		return nullptr;
	}

	Common::SeekableReadStream &disk = *_disks[agid->volume];

	// seek to resource and read header (resource length)
	disk.seek(agid->offset);
	agid->len = disk.readUint16LE();

	uint8 *data = (uint8 *)calloc(1, agid->len);
	if (disk.read(data, agid->len) != agid->len) {
		warning("GalLoader_A2: error reading %d bytes at volume %d offset %d", agid->len, agid->volume, agid->offset);
		free(data);
		return nullptr;
	}

	return data;
}

// TODO
int GalLoader_A2::loadObjects() {
	return errOK;
}

// TODO
int GalLoader_A2::loadWords() {
	// words location: GAL_A2_WORDS_POSITION
	// two byte header with resource length.
	return errOK;
}

} // End of namespace Agi
