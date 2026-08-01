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

#include "macs2/amiga_archive.h"

#include "common/compression/powerpacker.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/array.h"

#include <cstring>
#include "macs2/detection.h"

namespace Macs2 {

namespace {

static const uint32 kTagMXMF = MKTAG('M', 'X', 'M', 'F');
static const uint32 kTagMXDR = MKTAG('M', 'X', 'D', 'R');
static const uint32 kTagMXIN = MKTAG('M', 'X', 'I', 'N');
static const uint32 kTagMXOO = MKTAG('M', 'X', 'O', 'O');
static const uint32 kTagMXMM = MKTAG('M', 'X', 'M', 'M');
static const uint32 kTagPP20 = MKTAG('P', 'P', '2', '0');

static Common::Path volumePathForDisk(uint16 disk) {
	// Demo uses DataA only (disk 0). Full game may use DataB/DataC/DataD.
	char letter = (char)('A' + CLIP<int>(disk, 0, 25));
	return Common::Path(Common::String::format("Data%c", letter));
}

} // End of anonymous namespace

Macs2AmigaArchive::Macs2AmigaArchive() {
}

Macs2AmigaArchive::~Macs2AmigaArchive() {
	close();
}

AmigaResourceType Macs2AmigaArchive::typeFromTag(uint16 tag) {
	switch (tag) {
	case kAmigaResOO:
	case kAmigaResMM:
	case kAmigaResOS:
	case kAmigaResFF:
		return (AmigaResourceType)tag;
	default:
		return kAmigaResUnknown;
	}
}

const char *Macs2AmigaArchive::typeToString(AmigaResourceType type) {
	switch (type) {
	case kAmigaResOO:
		return "OO";
	case kAmigaResMM:
		return "MM";
	case kAmigaResOS:
		return "OS";
	case kAmigaResFF:
		return "FF";
	default:
		return "??";
	}
}

Common::String Macs2AmigaArchive::makeResourceName(AmigaResourceType type, uint16 id) {
	return Common::String::format("%s_%04u", typeToString(type), id);
}

bool Macs2AmigaArchive::parseResourceName(const Common::String &name, AmigaResourceType &type, uint16 &id) {
	if (name.equalsIgnoreCase("scene_table") || name.equalsIgnoreCase("Info"))
		return false;
	if (name.size() < 7 || name[2] != '_')
		return false;

	const uint16 tag = ((uint16)name[0] << 8) | (uint16)name[1];
	type = typeFromTag(tag);
	if (type == kAmigaResUnknown)
		return false;

	id = (uint16)atoi(name.c_str() + 3);
	return true;
}

bool Macs2AmigaArchive::open() {
	close();

	if (!loadMdir()) {
		close();
		return false;
	}

	loadInfo();

	debugC(1, kDebugFilePath, "Macs2 Amiga archive: %u resources, %u scenes, %u volumes indexed",
		   (uint)_entries.size(), _sceneCount, (uint)_volumes.size());
	return !_entries.empty();
}

void Macs2AmigaArchive::close() {
	Common::StackLock lock(_mutex);
	_entries.clear();
	_volumes.clear();
	_sceneCount = 0;
	_totalResources = 0;
	_firstBlockSize = 0;
	_info = AmigaInfoData();
}

bool Macs2AmigaArchive::openVolume(uint16 disk) {
	while (_volumes.size() <= disk)
		_volumes.push_back(Volume());

	Volume &volume = _volumes[disk];
	if (volume.file)
		return true;

	volume.path = volumePathForDisk(disk);
	volume.file.reset(new Common::File());
	if (!volume.file->open(volume.path)) {
		warning("Macs2 Amiga: cannot open volume %s", volume.path.toString().c_str());
		volume.file.reset();
		return false;
	}
	volume.size = (uint32)volume.file->size();
	return true;
}

bool Macs2AmigaArchive::loadMdir() {
	Common::File mdirFile;
	if (!mdirFile.open("Mdir")) {
		warning("Macs2 Amiga: Mdir not found");
		return false;
	}

	const int64 mdirSize = mdirFile.size();
	if (mdirSize < 18) {
		warning("Macs2 Amiga: Mdir too small");
		return false;
	}

	if (mdirFile.readUint32BE() != kTagMXDR) {
		warning("Macs2 Amiga: Mdir is not MXDR");
		return false;
	}

	mdirFile.readUint16BE(); // version
	mdirFile.readUint16BE(); // entry count field (often count+1)
	mdirFile.readUint16BE(); // type tag, usually "OO"
	_totalResources = mdirFile.readUint16BE();
	mdirFile.readUint32BE(); // reserved
	_sceneCount = mdirFile.readUint16BE();

	const uint16 dirEntryCount = (uint16)((mdirSize - 18) / 10);
	Common::Array<AmigaDirEntry> dirEntries;
	dirEntries.resize(dirEntryCount);

	for (uint16 i = 0; i < dirEntryCount; i++) {
		AmigaDirEntry &e = dirEntries[i];
		const uint16 typeTag = mdirFile.readUint16BE();
		e.type = typeFromTag(typeTag);
		e.id = mdirFile.readUint16BE();
		e.disk = mdirFile.readUint16BE();
		e.offset = mdirFile.readUint32BE();
	}

	// Open volumes referenced by the directory and read each entry header.
	for (const AmigaDirEntry &dir : dirEntries) {
		if (dir.type == kAmigaResUnknown)
			continue;
		if (!openVolume(dir.disk))
			return false;

		Volume &volume = _volumes[dir.disk];
		if (dir.offset + 8 > volume.size) {
			warning("Macs2 Amiga: entry %s_%04u offset past end of volume", typeToString(dir.type), dir.id);
			continue;
		}

		volume.file->seek(dir.offset, SEEK_SET);
		const uint16 fileTypeTag = volume.file->readUint16BE();
		const uint16 fileId = volume.file->readUint16BE();
		const uint32 payloadSize = volume.file->readUint32BE();

		if (typeFromTag(fileTypeTag) != dir.type || fileId != dir.id) {
			warning("Macs2 Amiga: Mdir/Data mismatch for %s_%04u", typeToString(dir.type), dir.id);
			continue;
		}
		if (payloadSize == 0 || dir.offset + 8 + payloadSize > volume.size) {
			warning("Macs2 Amiga: bad payload size for %s_%04u", typeToString(dir.type), dir.id);
			continue;
		}

		Entry entry;
		entry.dir = dir;
		entry.payloadSize = payloadSize;
		entry.name = makeResourceName(dir.type, dir.id);
		_entries[Common::Path(entry.name)] = entry;
	}

	// Index the first PP20 block (scene table) from DataA header.
	if (!openVolume(0))
		return false;

	Volume &dataA = _volumes[0];
	dataA.file->seek(0, SEEK_SET);
	if (dataA.file->readUint32BE() != kTagMXMF) {
		warning("Macs2 Amiga: DataA is not MXMF");
		return false;
	}
	dataA.file->readUint16BE(); // version
	dataA.file->readUint16BE(); // type
	const uint16 hdrResourceCount = dataA.file->readUint16BE();
	_firstBlockSize = dataA.file->readUint32BE();
	_firstBlockOffset = 14;
	if (hdrResourceCount != 0)
		_totalResources = hdrResourceCount;

	if (_firstBlockSize == 0 || _firstBlockOffset + _firstBlockSize > dataA.size) {
		warning("Macs2 Amiga: invalid first PP20 block size");
		return false;
	}

	Entry sceneTable;
	sceneTable.dir.type = kAmigaResOO;
	sceneTable.dir.id = 0xFFFF;
	sceneTable.dir.disk = 0;
	sceneTable.dir.offset = _firstBlockOffset;
	sceneTable.payloadSize = _firstBlockSize;
	sceneTable.name = "scene_table";
	_entries[Common::Path(sceneTable.name)] = sceneTable;

	return true;
}

bool Macs2AmigaArchive::loadInfo() {
	Common::File infoFile;
	if (!infoFile.open("Info"))
		return false;

	// Layout matches Ghidra load_info_mxin @ 00221494 on demo_unpacked.exe.0:
	//  MXIN(4) + ver(u16)=1 + pad(u16)=0 + startMmId(u32BE)
	//  + 5xu16 panel border color indices
	//  + 8xu16 panel darken color indices
	//  + 16xu16 UI RGB4 colors
	//  + flag bytes (g_bUsePanelPatternFill at +0x50)
	//  + mdirSize / volumeCount
	if (infoFile.size() < 0x5A || infoFile.readUint32BE() != kTagMXIN)
		return false;

	const uint16 version = infoFile.readUint16BE();
	const uint16 pad0 = infoFile.readUint16BE();
	if (version != 1 || pad0 != 0)
		return false;

	const uint32 startSceneResourceId = infoFile.readUint32BE();

	AmigaInfoData info;
	if (startSceneResourceId > 0 && startSceneResourceId <= 0xFFFF)
		info.startSceneResourceId = (uint16)startSceneResourceId;

	// Ghidra load_info_mxin: bytes 0x0C..0x11 are other MXIN fields; panel tables
	// start at file +0x12 (demo: six zero bytes after startMmId).
	byte gap[6] = {0};
	if (infoFile.read(gap, sizeof(gap)) != sizeof(gap))
		return false;

	// File offset 0x12: five border color indices (demo 18..22).
	for (uint i = 0; i < 5; i++)
		info.panelBorderColorIndices[i] = infoFile.readUint16BE();
	// Also used historically as cursor mode probes in the demo (OO ids 18..22).
	for (uint i = 0; i < 5; i++)
		info.cursorResourceIds[i] = info.panelBorderColorIndices[i];

	for (uint i = 0; i < 8; i++)
		info.panelDarkenColorIndices[i] = infoFile.readUint16BE();

	for (uint i = 0; i < 16; i++)
		info.uiPaletteAmiga[i] = infoFile.readUint16BE();

	// Flag block at file +0x4C (Ghidra after UI RGB copy).
	// g_bUsePanelPatternFill is the byte at file +0x50 (demo: 0 = darken remap).
	byte flags[10] = {0};
	if (infoFile.read(flags, sizeof(flags)) != sizeof(flags))
		return false;
	info.usePanelPatternFill = flags[4] != 0; // file +0x50 (pre-title only)

	if (infoFile.pos() + 4 <= infoFile.size()) {
		info.mdirSize = infoFile.readUint16BE();
		info.volumeCount = infoFile.readUint16BE();
	}
	// Demo Info ends with sceneCount-like word earlier in some dumps; keep 0 if absent.
	info.useInventoryCursorId = 0x17; // Walk/inventory fallback used by gameplay modes
	info.loaded = true;
	_info = info;

	loadPrefMenuMode();
	// Gameplay uses PREF MENUMODE (copied over Info flag after title @ 00234b42).
	_info.usePanelPatternFill = _info.prefMenuMode != 0;

	if (_info.sceneCount != 0)
		_sceneCount = _info.sceneCount;

	Entry infoEntry;
	infoEntry.name = "Info";
	infoEntry.dir.disk = 0xFFFF; // special: load from loose Info file
	infoEntry.payloadSize = (uint32)infoFile.size();
	_entries[Common::Path("Info")] = infoEntry;

	return true;
}

bool Macs2AmigaArchive::loadPrefMenuMode() {
	// parsePrefResourceSettings @ 00236e88: * PREF then colon-separated ASCII digits.
	// Order: SCREENMODE, MENUMODE, MUSIC, INTRO, PICSHADE, ...
	// initAndRunTitleScreen copies MENUMODE into g_bUsePanelPatternFill.
	Common::File prefFile;
	if (!prefFile.open("PREF"))
		return false;

	const int32 size = prefFile.size();
	if (size <= 0 || size > 0x10000) {
		prefFile.close();
		return false;
	}
	Common::Array<char> buf;
	buf.resize((uint)size + 1);
	if (prefFile.read(buf.data(), (uint32)size) != (uint32)size) {
		prefFile.close();
		return false;
	}
	prefFile.close();
	buf[size] = '\0';

	const char *p = strstr(buf.data(), "* PREF");
	if (!p)
		p = buf.data();

	uint values[16] = {0};
	uint count = 0;
	while (*p && count < 16) {
		while (*p && *p != '*')
			p++;
		if (!*p)
			break;
		p++;
		while (*p == ' ' || *p == '\t')
			p++;
		while (*p && *p != ':' && *p != '\n' && *p != '\r')
			p++;
		if (*p != ':')
			continue;
		p++;
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p < '0' || *p > '9')
			continue;
		uint v = 0;
		while (*p >= '0' && *p <= '9') {
			v = v * 10 + (uint)(*p - '0');
			p++;
		}
		values[count++] = v;
	}

	if (count >= 2) {
		_info.prefMenuMode = (uint8)(values[1] & 0xFF);
		debugC(1, kDebugFilePath, "Amiga PREF MENUMODE=%u (0=SHADE/1=GFX/2=SOLID/3=POINTS)",
			   _info.prefMenuMode);
		return true;
	}
	return false;
}

byte *Macs2AmigaArchive::decompressPP20(const byte *src, uint32 srcLen, uint32 &outLen) {
	outLen = 0;
	if (!src || srcLen < 12 || READ_BE_UINT32(src) != kTagPP20)
		return nullptr;

	// PowerPackerStream::unpackBuffer allocates with new[]; MemoryReadStream frees with free().
	byte *unpacked = Common::PowerPackerStream::unpackBuffer(src, srcLen, outLen);
	if (!unpacked || outLen == 0) {
		delete[] unpacked;
		outLen = 0;
		return nullptr;
	}

	byte *copy = (byte *)malloc(outLen);
	if (!copy) {
		delete[] unpacked;
		outLen = 0;
		return nullptr;
	}
	memcpy(copy, unpacked, outLen);
	delete[] unpacked;
	return copy;
}

byte *Macs2AmigaArchive::decompressPayload(const byte *payload, uint32 payloadLen, uint32 &outLen) {
	outLen = 0;
	if (!payload || payloadLen < 4)
		return nullptr;

	const uint32 magic = READ_BE_UINT32(payload);
	if (magic == kTagPP20)
		return decompressPP20(payload, payloadLen, outLen);

	// MXMM is a multi-chunk scene package (chunk0 = planar BG). Return the raw
	// container so callers can decompress/parse individual chunks.
	if (magic == kTagMXMM) {
		byte *copy = (byte *)malloc(payloadLen);
		if (!copy)
			return nullptr;
		memcpy(copy, payload, payloadLen);
		outLen = payloadLen;
		return copy;
	}

	// Uncompressed MXOO / raw payload: return a malloc'd copy for MemoryReadStream.
	byte *copy = (byte *)malloc(payloadLen);
	if (!copy)
		return nullptr;
	memcpy(copy, payload, payloadLen);
	outLen = payloadLen;
	return copy;
}

Common::SeekableReadStream *Macs2AmigaArchive::readAndDecompressEntry(const Entry &entry) const {
	// Special case: loose Info file
	if (entry.name.equalsIgnoreCase("Info")) {
		Common::File infoFile;
		if (!infoFile.open("Info"))
			return nullptr;
		const uint32 size = (uint32)infoFile.size();
		byte *data = (byte *)malloc(size);
		if (!data)
			return nullptr;
		if (infoFile.read(data, size) != size) {
			free(data);
			return nullptr;
		}
		return new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
	}

	if (entry.dir.disk == 0xFFFF || entry.dir.disk >= _volumes.size() || !_volumes[entry.dir.disk].file)
		return nullptr;

	Common::StackLock lock(_mutex);
	Volume &volume = const_cast<Volume &>(_volumes[entry.dir.disk]);

	uint32 readOffset = entry.dir.offset;
	uint32 readSize = entry.payloadSize;

	// scene_table points at the raw PP20 block (no 8-byte type header).
	// Regular resources point at the 8-byte header; payload follows.
	if (!entry.name.equalsIgnoreCase("scene_table")) {
		readOffset += 8;
	}

	if (readOffset + readSize > volume.size)
		return nullptr;

	Common::Array<byte> payload;
	payload.resize(readSize);
	volume.file->seek(readOffset, SEEK_SET);
	if (volume.file->read(payload.data(), readSize) != readSize)
		return nullptr;

	uint32 outLen = 0;
	byte *decoded = decompressPayload(payload.data(), readSize, outLen);
	if (!decoded)
		return nullptr;

	return new Common::MemoryReadStream(decoded, outLen, DisposeAfterUse::YES);
}

bool Macs2AmigaArchive::hasResource(AmigaResourceType type, uint16 id) const {
	return hasFile(Common::Path(makeResourceName(type, id)));
}

Common::SeekableReadStream *Macs2AmigaArchive::createReadStreamForResource(AmigaResourceType type, uint16 id) const {
	return createReadStreamForMember(Common::Path(makeResourceName(type, id)));
}

Common::SeekableReadStream *Macs2AmigaArchive::createSceneTableStream() const {
	return createReadStreamForMember(Common::Path("scene_table"));
}

bool Macs2AmigaArchive::hasFile(const Common::Path &path) const {
	return _entries.contains(path);
}

int Macs2AmigaArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (const auto &entry : _entries)
		list.push_back(getMember(entry._key));
	return _entries.size();
}

const Common::ArchiveMemberPtr Macs2AmigaArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *Macs2AmigaArchive::createReadStreamForMember(const Common::Path &path) const {
	if (!_entries.contains(path))
		return nullptr;
	return readAndDecompressEntry(_entries[path]);
}

bool Macs2AmigaArchive::decodePlanarSprite(const byte *mxoo, uint32 mxooSize, uint16 &width, uint16 &height, Common::Array<byte> &pixels) {
	width = height = 0;
	pixels.clear();

	if (!mxoo || mxooSize < 32 || READ_BE_UINT32(mxoo) != kTagMXOO)
		return false;

	const uint32 scriptOff = READ_BE_UINT32(mxoo + 4);
	if (scriptOff <= 12 || scriptOff > mxooSize)
		return false;

	const byte *body = mxoo + 12;
	const uint32 bodyLen = scriptOff - 12;
	if (bodyLen < 20 || body[10] != 1 || body[11] != 1)
		return false;

	const uint16 frames = READ_BE_UINT16(body + 14);
	width = READ_BE_UINT16(body + 16);
	height = READ_BE_UINT16(body + 18);
	if (frames == 0 || width == 0 || width > 320 || height == 0 || height > 200)
		return false;

	const uint32 rowBytes = (width + 7) / 8;
	const uint32 planeSize = rowBytes * height;
	const uint32 pixelBytes = bodyLen - 20;
	if (planeSize == 0 || pixelBytes < planeSize * 5)
		return false;

	const uint32 numPlanes = pixelBytes / planeSize;
	if (numPlanes < 5 || numPlanes > 6 || pixelBytes != planeSize * numPlanes)
		return false;

	pixels.resize((uint)width * height);
	Common::fill(pixels.begin(), pixels.end(), 0);

	const byte *pixData = body + 20;
	// Planes 0..4 are color; plane 5 (if present) is mask/transparency.
	for (uint32 plane = 0; plane < 5 && plane < numPlanes; plane++) {
		const uint32 pOff = plane * planeSize;
		for (uint16 y = 0; y < height; y++) {
			for (uint32 bx = 0; bx < rowBytes; bx++) {
				const byte b = pixData[pOff + y * rowBytes + bx];
				for (int bit = 0; bit < 8; bit++) {
					const uint16 x = (uint16)(bx * 8 + bit);
					if (x < width && (b & (0x80 >> bit)))
						pixels[y * width + x] |= (byte)(1 << plane);
				}
			}
		}
	}

	if (numPlanes >= 6) {
		// Mask plane: clear transparent pixels (bit clear in mask).
		const uint32 pOff = 5 * planeSize;
		for (uint16 y = 0; y < height; y++) {
			for (uint32 bx = 0; bx < rowBytes; bx++) {
				const byte b = pixData[pOff + y * rowBytes + bx];
				for (int bit = 0; bit < 8; bit++) {
					const uint16 x = (uint16)(bx * 8 + bit);
					if (x < width && (b & (0x80 >> bit)) == 0)
						pixels[y * width + x] = 0;
				}
			}
		}
	}

	return true;
}

} // End of namespace Macs2
