/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dgds/resource.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/platform.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/substream.h"
#include "dgds/decompress.h"
#include "dgds/includes.h"
#include "dgds/parser.h"

namespace Dgds {

ResourceManager::ResourceManager() {
	const char *indexFiles[] = {
	    "volume.vga", // early Dragon versions
	    "volume.rmf", // Beamish / HoC
	    "volume.map"  // Beamish CD
	};

	Common::File indexFile;
	for (int i = 0; i < ARRAYSIZE(indexFiles); i++) {
		if (Common::File::exists(indexFiles[i])) {
			indexFile.open(indexFiles[i]);
			break;
		}
	}

	indexFile.skip(4); // salt for file hash, TODO
	int volumes = indexFile.readUint16LE();

	for (int i = 0; i < volumes; i++) {
		Common::String volumeName;
		for (int j = 0; j < FILENAME_LENGTH; j++)
			volumeName += indexFile.readByte();
		volumeName.toLowercase();

		_volumes[i].open(volumeName);

		indexFile.skip(1); // unknown
		int entries = indexFile.readUint16LE();
		//debug("File %s has %d entries", volumeName.c_str(), entries);

		for (int j = 0; j < entries; j++) {
			Resource res;
			res.volume = i;
			res.checksum = indexFile.readUint32LE();
			res.pos = indexFile.readUint32LE();

			_volumes[i].seek(res.pos, SEEK_SET);
			res.pos += FILENAME_LENGTH + 1 + 4;

			Common::String fileName;
			for (int k = 0; k < FILENAME_LENGTH; k++)
				fileName += _volumes[i].readByte();
			fileName.toLowercase();

			_volumes[i].skip(1); // unknown
			res.size = _volumes[i].readUint32LE();
			_resources[fileName] = res;

			if (fileName == "" || res.size == 0)
				continue;

			//debug("  - %s at %d, size: %d", fileName.c_str(), res.pos, res.size);
		}
	}

	indexFile.close();
}

ResourceManager::~ResourceManager() {
	for (int i = 0; i < MAX_VOLUMES; i++)
		_volumes[i].close();
}

Common::SeekableReadStream *ResourceManager::getResource(Common::String name, bool ignorePatches) {
	name.toLowercase();

	// Load external patches
	if (!ignorePatches && Common::File::exists(name)) {
		Common::File *patch = new Common::File();
		patch->open(name);
		return patch;
	}

	if (!_resources.contains(name))
		return nullptr;

	Resource res = _resources[name];
	return new Common::SeekableSubReadStream(&_volumes[res.volume], res.pos, res.pos + res.size);
}

Resource ResourceManager::getResourceInfo(Common::String name) {
	name.toLowercase();

	if (!_resources.contains(name))
		return Resource();

	return _resources[name];
}

bool DgdsChunk::isSection(const Common::String &section) {
	return section.equals(_idStr);
}

bool DgdsChunk::isSection(DGDS_ID section) {
	return (section == _id);
}

bool DgdsChunk::isPacked(DGDS_EX ex) {
	bool packed = false;

	switch (ex) {
	case EX_ADS:
	case EX_ADL:
	case EX_ADH:
		packed = (_id == ID_SCR);
		break;
	case EX_BMP:
		packed = (_id == ID_BIN || _id == ID_VGA);
		break;
	case EX_GDS:
	case EX_SDS:
		packed = (_id == ID_SDS);
		break;
	case EX_SCR:
		packed = (_id == ID_BIN || _id == ID_VGA || _id == ID_MA8);
		break;
	case EX_SNG:
		packed = (_id == ID_SNG);
		break;
	case EX_TTM:
		packed = (_id == ID_TT3);
		break;
	case EX_TDS:
		packed = (_id == ID_THD);
		break;
	default:
		break;
	}

	switch (ex) {
	case EX_DDS:
		packed = !strcmp(_idStr, "DDS:");
		break;
	case EX_OVL:
		if (0) {
		} else if (strcmp(_idStr, "ADL:") == 0)
			packed = true;
		else if (strcmp(_idStr, "ADS:") == 0)
			packed = true;
		else if (strcmp(_idStr, "APA:") == 0)
			packed = true;
		else if (strcmp(_idStr, "ASB:") == 0)
			packed = true;
		else if (strcmp(_idStr, "GMD:") == 0)
			packed = true;
		else if (strcmp(_idStr, "M32:") == 0)
			packed = true;
		else if (strcmp(_idStr, "NLD:") == 0)
			packed = true;
		else if (strcmp(_idStr, "PRO:") == 0)
			packed = true;
		else if (strcmp(_idStr, "PS1:") == 0)
			packed = true;
		else if (strcmp(_idStr, "SBL:") == 0)
			packed = true;
		else if (strcmp(_idStr, "SBP:") == 0)
			packed = true;
		else if (strcmp(_idStr, "STD:") == 0)
			packed = true;
		else if (strcmp(_idStr, "TAN:") == 0)
			packed = true;
		else if (strcmp(_idStr, "T3V:") == 0)
			packed = true;
		else if (strcmp(_idStr, "001:") == 0)
			packed = true;
		else if (strcmp(_idStr, "003:") == 0)
			packed = true;
		else if (strcmp(_idStr, "004:") == 0)
			packed = true;
		else if (strcmp(_idStr, "101:") == 0)
			packed = true;
		else if (strcmp(_idStr, "VGA:") == 0)
			packed = true;
		break;
	case EX_TDS:
		if (0) {
		} else if (strcmp(_idStr, "TDS:") == 0)
			packed = true; /* ? */
		break;
	default:
		break;
	}
	return packed;
}

bool DgdsChunk::readHeader(DgdsParser &ctx) {
	memset(_idStr, 0, sizeof(_idStr));
	_id = 0;

	if (ctx._file.pos() >= ctx._file.size()) {
		return false;
	}

	ctx._file.read(_idStr, DGDS_TYPENAME_MAX);

	if (_idStr[DGDS_TYPENAME_MAX - 1] != ':') {
		debug("bad header in: %s", ctx._filename);
		return false;
	}
	_idStr[DGDS_TYPENAME_MAX] = '\0';
	_id = MKTAG24(uint32(_idStr[0]), uint32(_idStr[1]), uint32(_idStr[2]));

	_size = ctx._file.readUint32LE();
	//ctx._file.skip(2);
	if (_size & 0x80000000) {
		_size &= ~0x80000000;
		container = true;
	} else {
		container = false;
	}
	return true;
}

Common::SeekableReadStream *DgdsChunk::decodeStream(DgdsParser &ctx, Decompressor *decompressor) {
	byte compression;
	uint32 unpackSize;
	Common::SeekableReadStream *output = 0;

	compression = ctx._file.readByte();
	unpackSize = ctx._file.readUint32LE();
	_size -= (1 + 4);

	if (!container) {
		byte *dest = new byte[unpackSize];
		decompressor->decompress(compression, dest, unpackSize, &ctx._file, _size);
		output = new Common::MemoryReadStream(dest, unpackSize, DisposeAfterUse::YES);
		ctx.bytesRead += unpackSize;
	}

	/*debug("    %s %u %s %u%c",
		id, _size,
		compressionDescr[compression],
		unpackSize, (container ? '+' : ' '));*/
	return output;
}

Common::SeekableReadStream *DgdsChunk::readStream(DgdsParser &ctx) {
	Common::SeekableReadStream *output = 0;

	if (!container) {
		output = new Common::SeekableSubReadStream(&ctx._file, ctx._file.pos(), ctx._file.pos() + _size, DisposeAfterUse::NO);
		ctx.bytesRead += _size;
	}

	debug("    %s %u%c", _idStr, _size, (container ? '+' : ' '));
	return output;
}

bool isFlatfile(Common::Platform platform, DGDS_EX _ex) {
	bool flat = false;

	switch (_ex) {
	case EX_RST:
	case EX_VIN:
	case EX_DAT:
		flat = true;
		break;
	default:
		break;
	}

	switch (platform) {
	case Common::kPlatformAmiga:
		switch (_ex) {
		case EX_BMP:
		case EX_SCR:
		case EX_INS:
			//				case EX_SNG:
		case EX_AMG:
			flat = true;
			break;
		default:
			break;
		}
	default:
		break;
	}
	return flat;
}

#if 0
/*
  input:
    s - pointer to ASCIIZ filename string
    idx - pointer to an array of 4 bytes (hash indexes)
  return:
    hash - filename hash
*/
int32 dgdsHash(const char *s, byte *idx) {
	int32 i, c;
	int16 isum, ixor;
	isum = 0;
	ixor = 0;
	for (i = 0; s[i]; i++) {
		c = toupper(s[i]);
		isum += c;
		ixor ^= c;
	}
	/* both types here MUST be int16 */
	isum *= ixor;
	c = 0;
	for (ixor = 0; ixor < 4; ixor++) {
		c <<= 8;
		/* can use only existing characters
		   ("i" holds the string length now) */
		if (i > idx[ixor]) {
			c |= toupper(s[idx[ixor]]);
		}
	}
	c += isum;
	return c;
}
#endif

#if 0
// TAGS vs UNIQUE TAGS
// HASHTABLE?

uint32 lookupVolume(const char *rmfName, const char *filename, char *volname) {
	Common::File index;
	if (!index.open(rmfName))
		return 0xFFFFFFFF;

	byte salt[4];
	int32 filehash;

	index.read(salt, sizeof(salt));
	filehash = dgdsHash(filename, salt);

	uint16 nvolumes;
	nvolumes = index.readUint16LE();
	for (uint i = 0; i < nvolumes; i++) {
		index.read(volname, FILENAME_LENGTH + 1);
		volname[FILENAME_LENGTH] = '\0';

		uint16 nfiles;
		nfiles = index.readUint16LE();
		for (uint j = 0; j < nfiles; j++) {
			int32 hash = index.readSint32LE();
			uint32 offset = index.readUint32LE();
			if (hash == filehash) {
				index.close();
				return offset;
			}
		}
	}
	index.close();
	return 0xFFFFFFFF;
}
#endif

#if 0
Common::SeekableReadStream *createReadStream(const char *rmfName, const char *filename) {
	char volname[FILENAME_LENGTH + 1];
	uint32 offset;
	offset = lookupVolume(rmfName, filename, volname);
	if (offset == 0xFFFFFFFF)
		return 0;

	Common::File *volume = new Common::File;
	do {
		if (!volume->open(volname))
			break;
		volume->seek(offset);
		volume->read(volname, sizeof(volname));
		volname[FILENAME_LENGTH] = '\0';

		uint32 fileSize;
		fileSize = volume->readUint32LE();

		if (fileSize == 0xFFFFFFFF)
			break;
		if (scumm_stricmp(volname, filename))
			break;
		return new Common::SeekableSubReadStream(volume, volume->pos(), volume->pos() + fileSize, DisposeAfterUse::YES);
	} while (0);

	delete volume;
	return 0;
}
#endif

#if 0
void explode(Common::Platform platform, const char *indexName, const char *fileName, int resource) {
	if (fileName) {
		Common::File file;
		if (file.open(fileName)) {
			parseFile(platform, file, fileName, resource);
			file.close();
			return;
		}
	}

	Common::File index, volume;
	Common::SeekableSubReadStream *file;
	if (index.open(indexName)) {
		byte salt[4];
		uint16 nvolumes;

		index.read(salt, sizeof(salt));
		nvolumes = index.readUint16LE();

		if (!fileName)
			debug("(%u,%u,%u,%u) %u", salt[0], salt[1], salt[2], salt[3], nvolumes);

		for (uint i = 0; i < nvolumes; i++) {
			char name[FILENAME_LENGTH + 1];
			uint16 nfiles;

			index.read(name, sizeof(name));
			name[FILENAME_LENGTH] = '\0';

			nfiles = index.readUint16LE();

			debugN("--\n#%u %s, %u files", i, name, nfiles);

			if (!volume.open(name)) {
				debug(", failed to open");
				continue;
			}

			debug(", %d bytes", volume.size());

			for (uint j = 0; j < nfiles; j++) {
				int32 hash;
				uint32 offset, fileSize;

				hash = index.readSint32LE();
				offset = index.readUint32LE();

				volume.seek(offset);
				volume.read(name, sizeof(name));
				name[FILENAME_LENGTH] = '\0';
				fileSize = volume.readUint32LE();

				if (!fileName || scumm_stricmp(name, fileName) == 0)
					debug("  #%u %s 0x%X=0x%X %u %u\n  --", j, name, hash, dgdsHash(name, salt), offset, fileSize);

				if (fileSize == 0xFFFFFFFF) {
					continue;
				}

				if (fileName && scumm_stricmp(name, fileName)) {
					volume.skip(fileSize);
					continue;
				}

				file = new Common::SeekableSubReadStream(&volume, volume.pos(), volume.pos() + fileSize, DisposeAfterUse::NO);

				parseFile(platform, *file, name, resource);

				if (!fileName)
					debug("  #%u %s %d .", j, name, volume.pos());

				if (fileName) {
					volume.close();
					index.close();
					return;
				}
			}
			volume.close();
		}
		index.close();
	}
}
#endif

#if 0
int prev_id = -1;
int prev_bk = -1;
int prev_palette = -1;

void browseInit(Common::Platform _platform, const char *_rmfName, DgdsEngine* syst) {
	BMPs.clear();
	explode(_platform, _rmfName, 0, -1);
	bk = 0;
	_idStr = 0;
	sid = 1;
}

void browse(Common::Platform _platform, const char *_rmfName, DgdsEngine* syst) {
	if (prev_id != _idStr || prev_bk != bk) {
		explode(_platform, _rmfName, BMPs[_idStr].c_str(), bk);
		prev_id = _idStr;
		prev_bk = bk;

		Common::String txt = Common::String::format("%s: %ux%u (%u/%u)", BMPs[_idStr].c_str(), _tw, _th, bk+1, _tcount);
		g_system->displayMessageOnOSD(txt.c_str());
	}
	if (prev_palette != sid) {
		sid = sid&3;
		prev_palette = sid;

		switch (sid) {
		case 3:
			for (uint i=0; i<256; i++) {
				palette[i*3+0] = i;
				palette[i*3+1] = i;
				palette[i*3+2] = i;
			}
			break;
		case 2:
			for (uint i=0; i<256; i++) {
				palette[i*3+0] = 255-i;
				palette[i*3+1] = 255-i;
				palette[i*3+2] = 255-i;
			}
			break;
		case 1:
			explode(_platform, _rmfName, "DRAGON.PAL", 0);
			break;
		}
		g_system->getPaletteManager()->setPalette(palette, 0, 256);
	}

	if (bk != -1) {
		if (_bmpData.h != 0) {
			byte *_bmpData_;
			_bmpData_ = (byte *)_bmpData.getPixels();
			byte *vgaData_;
			byte *binData_;
			bw = _tw; bh = _th;
			vgaData_ = (byte *)_bmpData.getPixels();
			binData_ = (byte *)_binData.getPixels();

			for (int i=0; i<bw*bh; i+=2) {
				_bmpData_[i+0]  = ((vgaData_[i>>1] & 0xF0)     );
				_bmpData_[i+0] |= ((binData_[i>>1] & 0xF0) >> 4);
				_bmpData_[i+1]  = ((vgaData_[i>>1] & 0x0F) << 4);
				_bmpData_[i+1] |= ((binData_[i>>1] & 0x0F)     );
			}

			const int rows = sh;
			const int columns = sw;

			byte *src = _bmpData_;
			byte *ptr = (byte *)bottomBuffer.getBasePtr(0, 0);
			for (int i=0; i<rows; ++i) {
				for (int j=0; j<columns; ++j) {
					ptr[j] = src[j];
				}
				ptr += bottomBuffer.pitch;
				src += bw;
			}
		}

		resData.clear(0);
		Common::Rect bmpWin(0, 0, _tw, _th);
		Graphics::Surface bmpSub = bottomBuffer.getSubArea(bmpWin);
		resData.blitFrom(bmpSub, Common::Point(bmpWin.left, bmpWin.top));
		resData.frameRect(bmpWin, 1);

		Graphics::Surface *dst;
		dst = g_system->lockScreen();
		dst->copyRectToSurface(resData, 0, 0, rect);
		g_system->unlockScreen();
		g_system->updateScreen();
	}
}
#endif

} // End of namespace Dgds
