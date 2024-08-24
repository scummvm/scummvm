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
#include "agi/words.h"

#include "common/config-manager.h"
#include "common/formats/disk_image.h"
#include "common/fs.h"
#include "common/memstream.h"

namespace Agi {

// AgiLoader_A2 reads Apple II floppy disk images.
//
// Floppy disks have two sides; each side is a disk with its own image file.
// All disk sides are 140k with 35 tracks and 16 sectors per track.
//
// Multiple disk image formats are supported; see Common::DiskImage. The file
// extension determines the format. For example: .do, .dsk, .nib, .woz.
//
// The disks do not use a standard file system. Instead, file locations are
// stored in an INITDIR structure at a fixed location. KQ2 and BC don't have
// INITDIR, so we use the known locations of their files.
//
// Almost every AGI game was released on Apple II. Due to the small disk size,
// games can have many image files. KQ4 and Gold Rush each have eight physical
// floppies, for a total of sixteen disks. Disk one contains the disk count and
// a volume map with the location of each volume on each disk. Disks can contain
// multiple volumes. Volumes can appear on multiple disks in any location.
// Later games have so many volumes that Sierra had to change the DIR format.
//
// File detection is done a little differently. Instead of requiring hard-coded
// names for the image files, we scan the game directory for the first usable
// image of disk one, and then continue scanning until all disks are found.
// The volume map from disk one is used to identify each disk by its content.
// The only naming requirement is that the images have a known file extension.
//
// AgiMetaEngineDetection also scans for usable disk images. It finds the LOGDIR
// file inside disk one, hashes LOGDIR, and matches against the detection table.

typedef Common::HashMap<Common::Path, Common::FSNode, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FileMap;

AgiLoader_A2::~AgiLoader_A2() {
	for (uint d = 0; d < _disks.size(); d++) {
		delete _disks[d];
	}
}

void AgiLoader_A2::init() {
	// get all files in game directory
	Common::FSList allFiles;
	Common::FSNode dir(ConfMan.getPath("path"));
	if (!dir.getChildren(allFiles, Common::FSNode::kListFilesOnly)) {
		warning("AgiLoader_A2: invalid game path: %s", dir.getPath().toString(Common::Path::kNativeSeparator).c_str());
		return;
	}

	// build array of files with a2 disk image extensions
	Common::Array<Common::Path> imageFiles;
	FileMap fileMap;
	for (const Common::FSNode &file : allFiles) {
		for (int i = 0; i < ARRAYSIZE(a2DiskImageExtensions); i++) {
			if (file.getName().hasSuffixIgnoreCase(a2DiskImageExtensions[i])) {
				Common::Path path = file.getPath();
				imageFiles.push_back(path);
				fileMap[path] = file;
				break;
			}
		}
	}

	// sort potential image files by name
	Common::sort(imageFiles.begin(), imageFiles.end());

	// find disk one by reading potential images until successful
	int diskCount = 0;
	Common::Array<uint32> volumeMap;
	uint diskOneIndex;
	for (diskOneIndex = 0; diskOneIndex < imageFiles.size(); diskOneIndex++) {
		const Common::Path &imageFile = imageFiles[diskOneIndex];
		Common::SeekableReadStream *stream = openA2DiskImage(imageFile, fileMap[imageFile]);
		if (stream == nullptr) {
			warning("AgiLoader_A2: unable to open disk image: %s", imageFile.baseName().c_str());
			continue;
		}

		// read image as disk one
		diskCount = readDiskOne(*stream, volumeMap);
		if (diskCount > 0) {
			debugC(3, "AgiLoader_A2: disk one found: %s", imageFile.baseName().c_str());
			_disks.resize(diskCount);
			_disks[0] = stream;
			break;
		} else {
			delete stream;
		}
	}

	// if disk one wasn't found, we're done
	if (diskCount <= 0) {
		warning("AgiLoader_A2: disk one not found");
		return;
	}

	// find all other disks by comparing their contents to the volume map.
	// if every volume that's supposed to be on a disk has a valid header
	// at that location, then it's a match. continue until all disks are found.
	// since the potential image file list is sorted, begin with the file after
	// disk one and try until successful.
	int volumeCount = volumeMap.size() / diskCount;
	int disksFound = 1;
	for (uint i = 1; i < imageFiles.size() && disksFound < diskCount; i++) {
		uint imageFileIndex = (diskOneIndex + i) % imageFiles.size();
		Common::Path &imageFile = imageFiles[imageFileIndex];

		Common::SeekableReadStream *stream = openA2DiskImage(imageFile, fileMap[imageFile]);
		if (stream == nullptr) {
			continue;
		}

		// check each disk
		bool diskFound = false;
		for (int d = 1; d < diskCount; d++) {
			// has disk already been found?
			if (_disks[d] != nullptr) {
				continue;
			}

			bool match = false;
			for (int v = 0; v < volumeCount; v++) {
				uint32 offset = volumeMap[(v * diskCount) + d];
				if (offset == _EMPTY) {
					continue;
				}

				// test for expected resource header
				stream->seek(offset);
				uint16 magic = stream->readUint16BE();
				byte volume = stream->readByte();
				uint16 size = stream->readUint16LE();
				if (magic == 0x1234 && volume == v && stream->pos() + size <= stream->size()) {
					match = true;
				} else {
					match = false;
					break;
				}
			}

			if (match) {
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

	// populate _volumes with the locations of the ones we will use.
	// for each volume, select the one on the first available disk.
	_volumes.resize(volumeCount);
	for (uint32 i = 0; i < volumeMap.size(); i++) {
		int volume = i / diskCount;
		int disk = i % diskCount;
		if (volumeMap[i] != _EMPTY) {
			// use this disk's copy of the volume
			_volumes[volume].disk = disk;
			_volumes[volume].offset = volumeMap[i];

			// skip to next volume
			i = ((volume + 1) * diskCount) - 1;
		}
	}
}

// returns disk count on success, 0 on failure
int AgiLoader_A2::readDiskOne(Common::SeekableReadStream &stream, Common::Array<uint32> &volumeMap) {
	// INITDIR is located at track 1, sector 3, for games that have it.
	int diskCount;
	bool success = true;
	if (_vm->getGameID() == GID_KQ2) {
		// KQ2 doesn't have INITDIR. Use known locations.
		diskCount = A2_KQ2_DISK_COUNT;
		success &= readDir(stream, A2_KQ2_LOGDIR_POSITION,   _logDir);
		success &= readDir(stream, A2_KQ2_PICDIR_POSITION,   _picDir);
		success &= readDir(stream, A2_KQ2_VIEWDIR_POSITION,  _viewDir);
		success &= readDir(stream, A2_KQ2_SOUNDDIR_POSITION, _soundDir);
		success &= readDir(stream, A2_KQ2_OBJECTS_POSITION,  _objects);
		success &= readDir(stream, A2_KQ2_WORDS_POSITION,    _words);
		// KQ2 doesn't have a volume map, probably because all the
		// volumes on the data disks start at the first sector.
		// Create one with known values so that it can also be
		// used for disk detection.
		volumeMap.clear();
		volumeMap.resize(A2_KQ2_DISK_COUNT * (A2_KQ2_DISK_COUNT + 1), _EMPTY);
		volumeMap[0 * diskCount + 0] = A2_KQ2_VOL0_POSITION;
		volumeMap[1 * diskCount + 0] = A2_KQ2_VOL1_POSITION;
		volumeMap[2 * diskCount + 1] = 0;
		volumeMap[3 * diskCount + 2] = 0;
		volumeMap[4 * diskCount + 3] = 0;
		volumeMap[5 * diskCount + 4] = 0;
	} else if (_vm->getGameID() == GID_BC) {
		// BC doesn't have INITDIR. Use known locations.
		diskCount = A2_BC_DISK_COUNT;
		success &= readDir(stream, A2_BC_LOGDIR_POSITION,   _logDir);
		success &= readDir(stream, A2_BC_PICDIR_POSITION,   _picDir);
		success &= readDir(stream, A2_BC_VIEWDIR_POSITION,  _viewDir);
		success &= readDir(stream, A2_BC_SOUNDDIR_POSITION, _soundDir);
		success &= readDir(stream, A2_BC_OBJECTS_POSITION,  _objects);
		success &= readDir(stream, A2_BC_WORDS_POSITION,    _words);
		// BC has a volume map even though it doesn't have INITDIR.
		// The uint16 in front of it might be volume count.
		int volumeMapBufferSize = A2_BC_DISK_COUNT * A2_BC_VOLUME_COUNT * 2;
		success &= readVolumeMap(stream, A2_BC_VOLUME_MAP_POSITION, volumeMapBufferSize, volumeMap);
	} else {
		stream.seek(A2_INITDIR_POSITION);
		uint16 magic = stream.readUint16BE();
		byte volume = stream.readByte();
		uint16 size = stream.readUint16LE();
		if (!(magic == 0x1234 && volume == 0)) {
			return 0;
		}

		diskCount = stream.readByte(); // first byte of INITDIR
		success &= readInitDir(stream, A2_INITDIR_LOGDIR_INDEX,   _logDir);
		success &= readInitDir(stream, A2_INITDIR_PICDIR_INDEX,   _picDir);
		success &= readInitDir(stream, A2_INITDIR_VIEWDIR_INDEX,  _viewDir);
		success &= readInitDir(stream, A2_INITDIR_SOUNDDIR_INDEX, _soundDir);
		success &= readInitDir(stream, A2_INITDIR_OBJECTS_INDEX,  _objects);
		success &= readInitDir(stream, A2_INITDIR_WORDS_INDEX,    _words);
		// volume map begins at byte 33 of INITDIR and runs until the end.
		int volumeMapBufferSize = size - 33;
		success &= readVolumeMap(stream, A2_INITDIR_VOLUME_MAP_POSITION, volumeMapBufferSize, volumeMap);
	}

	return success ? diskCount : 0;
}

bool AgiLoader_A2::readInitDir(Common::SeekableReadStream &stream, byte index, AgiDir &agid) {
	// read INITDIR entry
	stream.seek(A2_INITDIR_POSITION + 5 + 1 + (index * A2_INITDIR_ENTRY_SIZE));
	byte volume = stream.readByte();
	byte track = stream.readByte();
	byte sector = stream.readByte();
	byte offset = stream.readByte();
	if (stream.eos() || stream.err()) {
		return false;
	}

	// resource must be on disk one
	if (!(volume == 0 || volume == 1)) {
		return false;
	}

	int position = A2_DISK_POSITION(track, sector, offset);
	return readDir(stream, position, agid);
}

bool AgiLoader_A2::readDir(Common::SeekableReadStream &stream, int position, AgiDir &agid) {
	// resource begins with a 5-byte header
	stream.seek(position);
	uint16 magic = stream.readUint16BE();
	byte volume = stream.readByte();
	uint16 size = stream.readUint16LE();
	if (!(magic == 0x1234 && (volume == 0 || volume == 1))) {
		return false;
	}
	if (!(stream.pos() + size <= stream.size())) {
		return false;
	}

	// resource found
	agid.volume = volume;
	agid.offset = stream.pos();
	agid.len = size;
	agid.clen = size;
	return true;
}

bool AgiLoader_A2::readVolumeMap(
	Common::SeekableReadStream &stream,
	uint32 position,
	uint32 bufferLength,
	Common::Array<uint32> &volumeMap) {

	// Volume map contains the location of every volume on every disk.
	// Each entry is the location in sectors. Volumes can appear on
	// multiple disks.
	// ## ##   location of VOL.0 on disk 1. FF FF if empty.
	// ## ##   location of VOL.0 on disk 2. FF FF if empty.
	// ...
	// ## ##   location of VOL.1 on disk 1. FF FF if empty.
	stream.seek(position);
	uint32 entryCount = bufferLength / 2;
	volumeMap.clear();
	volumeMap.resize(entryCount, _EMPTY);
	for (uint32 i = 0; i < entryCount; i++) {
		uint16 sectors = stream.readUint16LE();
		if (sectors != 0xffff) {
			volumeMap[i] = A2_DISK_POSITION(0, sectors, 0);
		}
	}
	return !stream.eos() && !stream.err();
}


int AgiLoader_A2::loadDirs() {
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

	// all dirs are on disk one
	Common::SeekableReadStream &disk = *_disks[0];

	// detect dir format
	A2DirVersion dirVersion = detectDirVersion(disk);

	// load each directory
	bool success = true;
	success &= loadDir(_vm->_game.dirLogic, disk, _logDir.offset,   _logDir.len,   dirVersion);
	success &= loadDir(_vm->_game.dirPic,   disk, _picDir.offset,   _picDir.len,   dirVersion);
	success &= loadDir(_vm->_game.dirView,  disk, _viewDir.offset,  _viewDir.len,  dirVersion);
	success &= loadDir(_vm->_game.dirSound, disk, _soundDir.offset, _soundDir.len, dirVersion);
	return success ? errOK : errBadResource;
}

A2DirVersion AgiLoader_A2::detectDirVersion(Common::SeekableReadStream &stream) {
	// A2 DIR format:
	//         old      new
	// volume  4 bits   5 bits
	// track   8 bits   7 bits
	// sector  4 bits   4 bits
	// offset  8 bits   8 bits
	//
	// This can be detected by scanning all dirs for entry 08 00 00.
	// It must exist in the new format, but can't exist in the old.
	// In the new format it's the first resource in volume 1.
	// In the old format it would be track 128, which is invalid.
	AgiDir *dirs[4] = { &_logDir, &_picDir, &_viewDir, &_soundDir };
	for (int d = 0; d < 4; d++) {
		stream.seek(dirs[d]->offset);
		uint16 dirEntryCount = MIN<uint32>(dirs[d]->len / 3, MAX_DIRECTORY_ENTRIES);
		for (uint16 i = 0; i < dirEntryCount; i++) {
			byte b0 = stream.readByte();
			byte b1 = stream.readByte();
			byte b2 = stream.readByte();
			if (b0 == 0x08 && b1 == 0x00 && b2 == 0x00) {
				return A2DirVersionNew;
			}
		}
	}
	return A2DirVersionOld;
}

bool AgiLoader_A2::loadDir(AgiDir *dir, Common::SeekableReadStream &disk, uint32 dirOffset, uint32 dirLength, A2DirVersion dirVersion) {
	// seek to directory on disk
	disk.seek(dirOffset);

	// re-validate length from initdir
	if (!(disk.pos() + dirLength <= disk.size())) {
		return false;
	}

	// read directory entries
	uint16 dirEntryCount = MIN<uint32>(dirLength / 3, MAX_DIRECTORY_ENTRIES);
	for (uint16 i = 0; i < dirEntryCount; i++) {
		byte b0 = disk.readByte();
		byte b1 = disk.readByte();
		byte b2 = disk.readByte();
		if (b0 == 0xff && b1 == 0xff && b2 == 0xff) {
			continue;
		}

		// A2 DIR format:
		//         old      new
		// volume  4 bits   5 bits
		// track   8 bits   7 bits
		// sector  4 bits   4 bits
		// offset  8 bits   8 bits
		// position is relative to the start of volume
		byte track;
		if (dirVersion == A2DirVersionOld) {
			dir[i].volume = b0 >> 4;
			track = ((b0 & 0x0f) << 4) | (b1 >> 4);
		} else {
			dir[i].volume = b0 >> 3;
			track = ((b0 & 0x07) << 4) | (b1 >> 4);
		}
		byte sector = b1 & 0x0f;
		byte offset = b2;
		dir[i].offset = A2_DISK_POSITION(track, sector, offset);
	}

	return true;
}

uint8 *AgiLoader_A2::loadVolumeResource(AgiDir *agid) {
	if (agid->volume >= _volumes.size()) {
		warning("AgiLoader_A2: invalid volume: %d", agid->volume);
		return nullptr;
	}
	if (_volumes[agid->volume].disk == _EMPTY) {
		warning("AgiLoader_A2: volume not found: %d", agid->volume);
		return nullptr;
	}

	int diskIndex = _volumes[agid->volume].disk;
	Common::SeekableReadStream &disk = *_disks[diskIndex];

	// seek to resource and validate header
	int offset = _volumes[agid->volume].offset + agid->offset;
	disk.seek(offset);
	uint16 magic = disk.readUint16BE();
	if (magic != 0x1234) {
		warning("AgiLoader_A2: no resource at volume %d offset %d", agid->volume, agid->offset);
		return nullptr;
	}
	disk.skip(1); // volume
	agid->len = disk.readUint16LE();

	uint8 *data = (uint8 *)calloc(1, agid->len + 32); // why the extra 32 bytes?
	if (disk.read(data, agid->len) != agid->len) {
		warning("AgiLoader_A2: error reading %d bytes at volume %d offset %d", agid->len, agid->volume, agid->offset);
		free(data);
		return nullptr;
	}

	return data;
}

int AgiLoader_A2::loadObjects() {
	if (_disks.empty()) {
		return errFilesNotFound;
	}

	Common::SeekableReadStream &disk = *_disks[0];
	disk.seek(_objects.offset);
	return _vm->loadObjects(disk, _objects.len);
}

int AgiLoader_A2::loadWords() {
	if (_disks.empty()) {
		return errFilesNotFound;
	}

	// TODO: pass length and validate in parser
	Common::SeekableReadStream &disk = *_disks[0];
	disk.seek(_words.offset);
	if (_vm->getVersion() < 0x2000) {
		return _vm->_words->loadDictionary_v1(disk);
	} else {
		return _vm->_words->loadDictionary(disk);
	}
}

} // End of namespace Agi
