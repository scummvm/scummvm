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
#include "common/fs.h"

namespace Agi {

// AgiLoader_v1 reads PC Booter floppy disk images.
//
// - King's Quest II           V1      2 disks
// - The Black Cauldron        V1      2 disks
// - Donald Duck's Playground  V2.001  1 disk
//
// All disks are 360k. The only supported image format is "raw". There are no
// headers, footers, or metadata. Each image file must be exactly 368,640 bytes.
//
// The disks do not use a standard file system. Instead, file locations are
// stored in an INITDIR structure at a fixed location. The interpreter version
// determines the location and format of INITDIR.
//
// File detection is done a little differently. Instead of requiring hard-coded
// names for the image files, we scan the game directory for the first usable
// image of disk one, and then scan for disk two. The only naming requirement is
// that the images have a known file extension.
//
// AgiMetaEngineDetection also scans for usable disk images. It finds the LOGDIR
// file inside disk one, hashes LOGDIR, and matches against the detection table.

typedef Common::HashMap<Common::Path, Common::FSNode, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FileMap;

void AgiLoader_v1::init() {
	// get all files in game directory
	Common::FSList allFiles;
	Common::FSNode dir(ConfMan.getPath("path"));
	if (!dir.getChildren(allFiles, Common::FSNode::kListFilesOnly)) {
		warning("AgiLoader_v1: invalid game path: %s", dir.getPath().toString(Common::Path::kNativeSeparator).c_str());
		return;
	}

	// build array of files with pc disk image extensions
	Common::Array<Common::Path> imageFiles;
	FileMap fileMap;
	for (const Common::FSNode &file : allFiles) {
		for (int i = 0; i < ARRAYSIZE(pcDiskImageExtensions); i++) {
			if (file.getName().hasSuffixIgnoreCase(pcDiskImageExtensions[i])) {
				Common::Path path = file.getPath();
				imageFiles.push_back(path);
				fileMap[path] = file;
			}
		}
	}

	// sort potential image files by name
	Common::sort(imageFiles.begin(), imageFiles.end());

	// find disk one by reading potential images until successful
	uint diskOneIndex;
	for (diskOneIndex = 0; diskOneIndex < imageFiles.size(); diskOneIndex++) {
		const Common::Path &imageFile = imageFiles[diskOneIndex];
		Common::SeekableReadStream *stream = openPCDiskImage(imageFile, fileMap[imageFile]);
		if (stream == nullptr) {
			continue;
		}

		// read image as disk one
		bool success;
		int vol0Offset = 0;
		if (_vm->getVersion() < 0x2001) {
			success = readDiskOneV1(*stream);
		} else {
			success = readDiskOneV2001(*stream, vol0Offset);
		}
		delete stream;

		if (success) {
			debugC(3, "AgiLoader_v1: disk one found: %s", imageFile.baseName().c_str());
			_imageFiles.push_back(imageFile.baseName());
			if (_vm->getVersion() < 0x2001) {
				// the first disk contains volumes 0 and 1.
				// there is no volume offset, resource
				// directories use absolute disk positions.
				_volumes.push_back(AgiDiskVolume(0, 0));
				_volumes.push_back(AgiDiskVolume(0, 0));
			} else {
				// the first disk contains volume 0.
				// resource offsets are relative to its location.
				_volumes.push_back(AgiDiskVolume(0, vol0Offset));
			}
			break;
		}
	}

	// if disk one wasn't found, we're done
	if (_imageFiles.empty()) {
		warning("AgiLoader_v1: disk one not found");
		return;
	}

	// two games have a second disk
	if (!(_vm->getGameID() == GID_KQ2 || _vm->getGameID() == GID_BC)) {
		return;
	}

	// find disk two by locating the next image file that begins with a resource
	// header with a volume number set to two. since the potential image file list
	// is sorted, begin with the file after disk one and try until successful.
	for (uint i = 1; i < imageFiles.size(); i++) {
		uint diskTwoIndex = (diskOneIndex + i) % imageFiles.size();
		Common::Path &imageFile = imageFiles[diskTwoIndex];

		Common::SeekableReadStream *stream = openPCDiskImage(imageFile, fileMap[imageFile]);
		if (stream == nullptr) {
			continue;
		}

		// read resource header
		uint16 magic = stream->readUint16BE();
		byte volume = stream->readByte();
		delete stream;

		if (magic == 0x1234 && volume == 2) {
			debugC(3, "AgiLoader_v1: disk two found: %s", imageFile.baseName().c_str());
			_imageFiles.push_back(imageFile.baseName());
			_volumes.push_back(AgiDiskVolume(_imageFiles.size() - 1, 0));
			break;
		}
	}

	if (imageFiles.size() < 2) {
		warning("AviLoader_v1: disk two not found");
	}
}

bool AgiLoader_v1::readDiskOneV1(Common::SeekableReadStream &stream) {
	// INITDIR V1 is located at the 9th sector after the 5-byte resource header.
	// Each entry is 10 bytes and there are always 8.
	stream.seek(PC_INITDIR_POSITION_V1);
	uint16 magic = stream.readUint16BE();
	byte volume = stream.readByte();
	uint16 size = stream.readUint16LE();
	if (!(magic == 0x1234 && volume == 1 && size == PC_INITDIR_SIZE_V1)) {
		return false;
	}

	bool success = true;
	success &= readInitDirV1(stream, PC_INITDIR_LOGDIR_INDEX_V1,   _logDir);
	success &= readInitDirV1(stream, PC_INITDIR_PICDIR_INDEX_V1,   _picDir);
	success &= readInitDirV1(stream, PC_INITDIR_VIEWDIR_INDEX_V1,  _viewDir);
	success &= readInitDirV1(stream, PC_INITDIR_SOUNDDIR_INDEX_V1, _soundDir);
	success &= readInitDirV1(stream, PC_INITDIR_OBJECTS_INDEX_V1,  _objects);
	success &= readInitDirV1(stream, PC_INITDIR_WORDS_INDEX_V1,    _words);
	return success;
}

bool AgiLoader_v1::readInitDirV1(Common::SeekableReadStream &stream, byte index, AgiDir &agid) {
	// read INITDIR entry
	stream.seek(PC_INITDIR_POSITION_V1 + 5 + (index * PC_INITDIR_ENTRY_SIZE_V1));
	byte volume = stream.readByte();
	byte head = stream.readByte();
	uint16 track = stream.readUint16LE();
	uint16 sector = stream.readUint16LE();
	uint16 offset = stream.readUint16LE();
	if (stream.eos() || stream.err()) {
		return false;
	}

	// resource must be on disk one
	if (!(volume == 0 || volume == 1)) {
		return false;
	}

	// resource begins with a 5-byte header
	uint32 position = PC_DISK_POSITION(head, track, sector, offset);
	stream.seek(position);
	uint16 magic = stream.readUint16BE();
	volume = stream.readByte();
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

bool AgiLoader_v1::readDiskOneV2001(Common::SeekableReadStream &stream, int &vol0Offset) {
	// INITDIR V2001 is located at the 2nd sector with no resource header.
	// Each entry is 3 bytes. The number of entries is technically variable,
	// because the list ends in an entry for each volume followed by FF FF FF.
	// But since there was only one V2001 game (Donald Duck's Playground),
	// and it only has one disk, there is really only ever one volume.

	bool success = true;
	success &= readInitDirV2001(stream, PC_INITDIR_LOGDIR_INDEX_V2001,   _logDir);
	success &= readInitDirV2001(stream, PC_INITDIR_PICDIR_INDEX_V2001,   _picDir);
	success &= readInitDirV2001(stream, PC_INITDIR_VIEWDIR_INDEX_V2001,  _viewDir);
	success &= readInitDirV2001(stream, PC_INITDIR_SOUNDDIR_INDEX_V2001, _soundDir);
	success &= readInitDirV2001(stream, PC_INITDIR_OBJECTS_INDEX_V2001,  _objects);
	success &= readInitDirV2001(stream, PC_INITDIR_WORDS_INDEX_V2001,    _words);

	// V2001 directories (LOGDIR, etc) contain resource offsets relative to
	// the start of their volume on disk. All volumes start at the beginning
	// of the disk, except for volume 0.
	AgiDir vol0;
	success &= readInitDirV2001(stream, PC_INITDIR_VOL0_INDEX_V2001, vol0);
	vol0Offset = vol0.offset - 5;

	return success;
}

bool AgiLoader_v1::readInitDirV2001(Common::SeekableReadStream &stream, byte index, AgiDir &agid) {
	// read INITDIR entry
	stream.seek(PC_INITDIR_POSITION_V2001 + (index * PC_INITDIR_ENTRY_SIZE_V2001));
	byte b0 = stream.readByte();
	byte b1 = stream.readByte();

	// volume      4 bits
	// position   12 bits  (in half-sectors)
	byte volume = b0 >> 4;
	uint32 position = (((b0 & 0x0f) << 8) + b1) * 256;

	// resource must be on disk one (because the only V2001 game is one disk)
	if (!(volume == 0 || volume == 1)) {
		return false;
	}

	// resource begins with a 5-byte header
	stream.seek(position);
	uint16 magic = stream.readUint16BE();
	volume = stream.readByte();
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

int AgiLoader_v1::loadDirs() {
	// if init didn't find disks then fail
	if (_imageFiles.empty()) {
		return errFilesNotFound;
	}

	// open disk one
	Common::File disk;
	if (!disk.open(Common::Path(_imageFiles[0]))) {
		return errBadFileOpen;
	}

	// load each directory
	bool success = true;
	success &= loadDir(_vm->_game.dirLogic, disk, _logDir.offset,   _logDir.len);
	success &= loadDir(_vm->_game.dirPic,   disk, _picDir.offset,   _picDir.len);
	success &= loadDir(_vm->_game.dirView,  disk, _viewDir.offset,  _viewDir.len);
	success &= loadDir(_vm->_game.dirSound, disk, _soundDir.offset, _soundDir.len);
	return success ? errOK : errBadResource;
}

bool AgiLoader_v1::loadDir(AgiDir *dir, Common::File &disk, uint32 dirOffset, uint32 dirLength) {
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

		if (_vm->getVersion() < 0x2001) {
			// volume   2 bits
			// track    6 bits
			// sector   6 bits (one based)
			// head     1 bit
			// offset   9 bits
			dir[i].volume = b0 >> 6;
			byte track = b0 & 0x3f;
			byte sector = b1 >> 2;
			byte head = (b1 >> 1) & 1;
			uint16 offset = ((b1 & 1) << 8) | b2;
			dir[i].offset = PC_DISK_POSITION(head, track, sector, offset);
		} else {
			// volume   4 bits
			// sector  11 bits (zero based)
			// offset   9 bits
			// position is relative to the start of volume
			dir[i].volume = b0 >> 4;
			uint16 sector = ((b0 & 0x0f) << 7) | (b1 >> 1);
			uint16 offset = ((b1 & 0x01) << 8) | b2;
			dir[i].offset = PC_DISK_POSITION(0, 0, sector + 1, offset);
		}
	}

	return true;
}

uint8 *AgiLoader_v1::loadVolumeResource(AgiDir *agid) {
	if (agid->volume >= _volumes.size()) {
		warning("AgiLoader_v1: invalid volume: %d", agid->volume);
		return nullptr;
	}

	Common::File disk;
	int diskIndex = _volumes[agid->volume].disk;
	if (!disk.open(Common::Path(_imageFiles[diskIndex]))) {
		warning("AgiLoader_v1: unable to open disk image: %s", _imageFiles[diskIndex].c_str());
		return nullptr;
	}

	// seek to resource and validate header
	int offset = _volumes[agid->volume].offset + agid->offset;
	disk.seek(offset);
	uint16 magic = disk.readUint16BE();
	if (magic != 0x1234) {
		warning("AgiLoader_v1: no resource at volume %d offset %d", agid->volume, agid->offset);
		return nullptr;
	}
	disk.skip(1); // volume
	agid->len = disk.readUint16LE();

	uint8 *data = (uint8 *)calloc(1, agid->len + 32); // why the extra 32 bytes?
	if (disk.read(data, agid->len) != agid->len) {
		warning("AgiLoader_v1: error reading %d bytes at volume %d offset %d", agid->len, agid->volume, agid->offset);
		free(data);
		return nullptr;
	}

	return data;
}

int AgiLoader_v1::loadObjects() {
	// DDP has an empty-ish objects resource but doesn't use it
	if (_vm->getGameID() == GID_DDP) {
		return errOK;
	}

	Common::File disk;
	if (!disk.open(Common::Path(_imageFiles[0]))) {
		return errBadFileOpen;
	}

	disk.seek(_objects.offset);
	return _vm->loadObjects(disk, _objects.len);
}

int AgiLoader_v1::loadWords() {
	// DDP has an empty-ish words resource but doesn't use it
	if (_vm->getGameID() == GID_DDP) {
		return errOK;
	}

	Common::File disk;
	if (!disk.open(Common::Path(_imageFiles[0]))) {
		return errBadFileOpen;
	}

	// TODO: pass length and validate in parser
	disk.seek(_words.offset);
	return _vm->_words->loadDictionary_v1(disk);
}

} // End of namespace Agi
