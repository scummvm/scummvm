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

#include "common/formats/iso9660.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/substream.h"
#include "common/textconsole.h"

namespace Common {

// ISO 9660 constants
static constexpr uint32 kSectorSize    = 2048;
static constexpr uint32 kPVDSector     = 16;   // Primary Volume Descriptor
static constexpr uint32 kMaxSVDSectors = 32;   // How many sectors to scan for Joliet SVD

/** Byte offset for a given Logical Block Address. */
static inline int64 sectorOffset(uint32 lba) {
	return static_cast<int64>(lba) * kSectorSize;
}

// Volume Descriptor types
enum VDType {
	kVDTypePrimary       = 1,
	kVDTypeSupplementary = 2,
	kVDTypeSetTerminator = 255
};

// Directory record flag bits (byte 25 of a directory record)
enum DirFlags {
	kFlagHidden      = 0x01,
	kFlagDirectory   = 0x02
};

// Byte offsets within a Volume Descriptor sector (ISO 9660)
enum VDOffsets {
	kVDType     = 0,
	kVDIdent    = 1,   // 5 bytes: "CD001"
	kVDVersion  = 6,
	kVDEscape   = 88,  // 32 bytes; Joliet escape sequences start here
	kVDRootDR   = 156  // 34-byte embedded root directory record
};

// Byte offsets within a High Sierra Format Volume Descriptor
enum HSFVDOffsets {
	kHSFVDType    = 8,
	kHSFVDIdent   = 9,   // 5 bytes: "CDROM"
	kHSFVDVersion = 14,
	kHSFVDRootDR  = 180  // 34-byte embedded root directory record
};

// Byte offsets within a directory record (absolute from record start).
// ISO 9660 and HSF share the same layout except for the flags byte:
//   ISO 9660: date=7 bytes (18-24), flags at 25
//   HSF:      date=6 bytes (18-23), flags at 24, reserved at 25
enum DROffsets {
	kDRLength    = 0,
	kDRExtAttr   = 1,
	kDRExtLocLE  = 2,  // uint32 little-endian: LBA
	kDRExtLocBE  = 6,  // uint32 big-endian:    LBA
	kDRDataLenLE = 10, // uint32 little-endian: size
	kDRDataLenBE = 14, // uint32 big-endian:    size
	kDRDate      = 18, // 7 bytes (ISO) or 6 bytes (HSF)
	kDRFlagsHSF  = 24, // HSF: flags here
	kDRFlags     = 25, // ISO 9660: flags here
	kDRUnitSize  = 26,
	kDRIlvGap    = 27,
	kDRVolSeqLE  = 28, // uint16 LE
	kDRVolSeqBE  = 30, // uint16 BE
	kDRIdLen     = 32,
	kDRId        = 33  // variable length
};

// Joliet SVD escape sequence bytes (at VD offset 88)
static constexpr byte kJolietEsc1    = 0x25; // '%'
static constexpr byte kJolietEsc2    = 0x2F; // '/'
static constexpr byte kJolietLevel1  = 0x40; // '@' -- UCS-2 Level 1
static constexpr byte kJolietLevel2  = 0x43; // 'C' -- UCS-2 Level 2
static constexpr byte kJolietLevel3  = 0x45; // 'E' -- UCS-2 Level 3

ISO9660FileSystem::ISO9660FileSystem(SeekableReadStream *stream)
	: _stream(stream),
	  _rootLba(0), _rootSize(0),
	  _jolietRootLba(0), _jolietRootSize(0),
	  _nameEncoding(kNameISO9660), _hsf(false) {
	if (!stream) {
		return;
	}
	if (!parsePVD()) {
		_stream.reset();
		return;
	}
	if (!_hsf) {
		scanForJoliet();
	}
}

ISO9660FileSystem::~ISO9660FileSystem() {
}

bool ISO9660FileSystem::isOpen() const {
	return _stream.get() != nullptr;
}

bool ISO9660FileSystem::readSector(uint32 lba, byte *buf) const {
	if (!_stream->seek(sectorOffset(lba))) {
		return false;
	}
	return (_stream->read(buf, kSectorSize) == kSectorSize);
}

bool ISO9660FileSystem::parsePVD() {
	byte pvd[kSectorSize];
	if (!readSector(kPVDSector, pvd)) {
		warning("ISO9660: read PVD failed");
		return false;
	}

	// Try each known volume descriptor layout: ISO 9660 then High Sierra
	struct VDLayout {
		uint8       typeOfs;
		const char *ident;
		uint8       identOfs;
		uint8       versionOfs;
		uint16      rootDROfs;
		bool        hsf;
	};
	static const VDLayout layouts[] = {
		{ kVDType,    "CD001", kVDIdent,    kVDVersion,    kVDRootDR,    false },
		{ kHSFVDType, "CDROM", kHSFVDIdent, kHSFVDVersion, kHSFVDRootDR, true  },
	};

	for (const auto &l : layouts) {
		if (pvd[l.typeOfs]    == kVDTypePrimary &&
		    memcmp(pvd + l.identOfs, l.ident, 5) == 0 &&
		    pvd[l.versionOfs] == 1) {
			_hsf      = l.hsf;
			_rootLba  = READ_LE_UINT32(pvd + l.rootDROfs + kDRExtLocLE);
			_rootSize = READ_LE_UINT32(pvd + l.rootDROfs + kDRDataLenLE);
			debug(2, "ISO9660: %s PVD OK, root dir at LBA %u (%u bytes)",
			      l.hsf ? "High Sierra" : "ISO 9660", _rootLba, _rootSize);
			return (_rootLba > 0);
		}
	}

	warning("ISO9660: PVD signature invalid (not ISO 9660 or High Sierra)");
	return false;
}

bool ISO9660FileSystem::scanForJoliet() {
	byte svd[kSectorSize];

	for (uint32 sector = kPVDSector + 1; sector < kPVDSector + kMaxSVDSectors; sector++) {
		if (!readSector(sector, svd)) {
			break;
		}

		// All VDs share the same identifier
		if (memcmp(svd + kVDIdent, "CD001", 5) != 0) {
			break;
		}

		uint8 type = svd[kVDType];
		if (type == kVDTypeSetTerminator) {
			break;
		}

		if (type == kVDTypeSupplementary) {
			// Joliet escape sequences at VD offset 88:
			// 0x25 0x2F followed by 0x40 (@), 0x43 (C), or 0x45 (E)
			if (svd[kVDEscape+0] == kJolietEsc1 && svd[kVDEscape+1] == kJolietEsc2 &&
			    (svd[kVDEscape+2] == kJolietLevel1 ||
			     svd[kVDEscape+2] == kJolietLevel2 ||
			     svd[kVDEscape+2] == kJolietLevel3)) {
				_jolietRootLba  = READ_LE_UINT32(svd + kVDRootDR + kDRExtLocLE);
				_jolietRootSize = READ_LE_UINT32(svd + kVDRootDR + kDRDataLenLE);
				_nameEncoding = kNameJoliet;
				debug(2, "ISO9660: Joliet SVD found at sector %u (level 0x%02x), root at LBA %u",
				      sector, svd[kVDEscape + 2], _jolietRootLba);
				return true;
			}
		}
	}
	return false;
}

Array<ISO9660FileSystem::DirEntry> ISO9660FileSystem::readRootDirectory() const {
	return parseDir(getRootLba(), getRootSize(), _nameEncoding);
}

ISO9660FileSystem::DirEntry ISO9660FileSystem::getRootEntry() const {
	DirEntry entry;
	entry.isDir = true;
	entry.name  = "";
	entry.lba   = getRootLba();
	entry.size  = getRootSize();
	return entry;
}

Array<ISO9660FileSystem::DirEntry> ISO9660FileSystem::readDirectory(const DirEntry &dir) const {
	return parseDir(dir.lba, dir.size, _nameEncoding);
}

Array<ISO9660FileSystem::DirEntry> ISO9660FileSystem::readDirectory(uint32 lba, uint32 size) const {
	return parseDir(lba, size, _nameEncoding);
}

Array<ISO9660FileSystem::DirEntry> ISO9660FileSystem::parseDir(uint32 lba, uint32 size, NameEncoding encoding) const {
	Array<DirEntry> entries;
	if (!_stream || lba == 0 || size == 0) {
		return entries;
	}

	// Read all directory sectors into a contiguous buffer.
	// Directory records must not cross sector boundaries, so we read whole sectors.
	uint32 numSectors = (size + kSectorSize - 1) / kSectorSize;
	uint32 bufSize    = numSectors * kSectorSize;

	ScopedPtr<byte, ArrayDeleter<byte>> buf(new byte[bufSize]);
	_stream->seek(sectorOffset(lba));
	uint32 bytesRead = _stream->read(buf.get(), bufSize);
	if (bytesRead < size) {
		size = bytesRead; // truncate to what we actually got
	}

	uint32 pos = 0;
	while (pos < size) {
		uint8 recLen = buf.get()[pos];

		if (recLen == 0) {
			// Zero byte = padding to sector boundary
			uint32 nextSector = (pos / kSectorSize + 1) * kSectorSize;
			if (nextSector >= bufSize) {
				break;
			}
			pos = nextSector;
			continue;
		}

		if (recLen < kDRId || (pos + recLen) > bufSize) {
			break; // sanity check
		}

		const byte *rec = buf.get() + pos;
		uint8  idLen   = rec[kDRIdLen];

		// Minimum valid record: 33 fixed bytes + at least 1 byte identifier
		if (idLen == 0 || kDRId + idLen > recLen) {
			pos += recLen;
			continue;
		}

		// Skip the "." (0x00) and ".." (0x01) entries
		const byte *id = rec + kDRId;
		if (idLen == 1 && (id[0] == 0x00 || id[0] == 0x01)) {
			pos += recLen;
			continue;
		}

		DirEntry entry;
		const uint8 flagsOfs = _hsf ? kDRFlagsHSF : kDRFlags;
		entry.isDir = (rec[flagsOfs] & kFlagDirectory) != 0;
		entry.lba   = READ_LE_UINT32(rec + kDRExtLocLE);
		entry.size  = READ_LE_UINT32(rec + kDRDataLenLE);

		if (encoding == kNameJoliet) {
			entry.name = jolietToString(id, idLen);
		} else {
			entry.name = iso9660ToLower(id, idLen);
		}

		if (!entry.name.empty()) {
			entries.push_back(entry);
		}

		pos += recLen;
	}

	debug(5, "ISO9660: read %u entries from directory at LBA %u", entries.size(), lba);
	return entries;
}

// Joliet filenames are UTF-16 Big Endian.  We convert the Basic Multilingual
// Plane to lowercase ASCII / Latin-1.  Non-BMP characters become '?'.
String ISO9660FileSystem::jolietToString(const byte *data, uint8 len) {
	// Strip the ";1" version suffix (encoded as UTF-16BE 0x00 0x3B 0x00 0x31)
	uint8 effectiveLen = len;
	while (effectiveLen >= 4) {
		const byte *tail = data + effectiveLen - 4;
		if (tail[0] == 0x00 && tail[1] == 0x3B &&
		    tail[2] == 0x00 && tail[3] == 0x31) {
			effectiveLen -= 4;
		} else {
			break;
		}
	}
	// Also strip a trailing null word that some tools add to directory names
	if (effectiveLen >= 2 && data[effectiveLen-2] == 0x00 && data[effectiveLen-1] == 0x00) {
		effectiveLen -= 2;
	}

	String result;
	for (uint8 i = 0; i + 1 < effectiveLen; i += 2) {
		auto ch = static_cast<uint16>((data[i] << 8) | data[i + 1]);
		if (ch == 0) {
			break;
		}
		if (ch < 128) {
			result += static_cast<char>(ch);
		} else {
			result += '?'; // non-ASCII placeholder
		}
	}
	result.toLowercase();
	return result;
}

// Plain ISO 9660 filenames use uppercase ASCII d-characters or a-characters.
// Strip the ";N" version suffix and normalize to lowercase.
String ISO9660FileSystem::iso9660ToLower(const byte *data, uint8 len) {
	uint8 effectiveLen = len;
	for (uint8 i = 0; i < len; i++) {
		if (data[i] == ';') {
			effectiveLen = i;
			break;
		}
	}
	// Strip trailing dot (files without extension sometimes have a spurious dot)
	while (effectiveLen > 0 && data[effectiveLen - 1] == '.') {
		effectiveLen--;
	}

	String result(reinterpret_cast<const char *>(data), effectiveLen);
	result.toLowercase();
	return result;
}

SeekableReadStream *ISO9660FileSystem::openFile(const DirEntry &entry) const {
	if (!_stream || entry.isDir || entry.size == 0) {
		return nullptr;
	}

	const uint64 begin64 = static_cast<uint64>(entry.lba) * kSectorSize;
	const uint64 end64   = begin64 + entry.size;
	if (end64 > UINT32_MAX) {
		warning("ISO9660: file '%s' at LBA %u exceeds 4 GB substream limit",
		        entry.name.c_str(), entry.lba);
		return nullptr;
	}
	uint32 begin = static_cast<uint32>(begin64);
	uint32 end   = static_cast<uint32>(end64);
	debug(5, "ISO9660: opening file '%s' at LBA %u, %u bytes", entry.name.c_str(), entry.lba, entry.size);

	// SafeSeekableSubReadStream re-seeks the parent before every read, which
	// makes it safe to have multiple outstanding streams from the same ISO.
	return new SafeSeekableSubReadStream(
		_stream.get(), begin, end, DisposeAfterUse::NO);
}

} // namespace Common
