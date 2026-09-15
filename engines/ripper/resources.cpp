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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/resources.h"

#include "common/archive.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/substream.h"
#include "graphics/surface.h"
#include "image/pcx.h"

#include "ripper/detection.h"
#include "ripper/iff.h"

namespace Ripper {

static const uint32 kModernLibraryMagic = 0x4c494232;
static const uint32 kCustomBitmapHeaderSize = 0x1c;
static const uint32 kCustomBitmapExtendedHeaderSize = 0xc;

static bool hasDirectorySeparator(const Common::String &path) {
	for (uint i = 0; i < path.size(); ++i) {
		if (path[i] == '/' || path[i] == '\\' || path[i] == ':')
			return true;
	}
	return false;
}

static bool readStream(Common::SeekableReadStream &stream, Common::Array<byte> &data) {
	if (stream.size() < 0 || (uint64)stream.size() > 0xffffffffULL)
		return false;

	data.resize((uint32)stream.size());
	return data.empty() || stream.read(data.data(), data.size()) == data.size();
}

static bool readPackedByte(const Common::Array<byte> &payload, uint32 &offset, byte shift,
		byte &value) {
	if (offset >= payload.size() || (shift != 0 && offset + 1 >= payload.size()))
		return false;

	if (shift == 0)
		value = payload[offset];
	else
		value = (payload[offset] >> 4) | (payload[offset + 1] << 4);
	++offset;
	return true;
}

static bool expandCustomBitmap(const Common::Array<byte> &payload, const byte *deltaTable,
		uint colorCount, uint32 rawTailSize, byte compressionMode, const byte *colorMap,
		Common::Array<byte> &pixels) {
	if (payload.empty() || rawTailSize > pixels.size())
		return false;

	uint32 sourceOffset = 0;
	uint32 outputOffset = 0;
	byte nibbleShift = 0;
	byte previousColor = payload[sourceOffset++];
	const uint32 compressedPixelCount = pixels.size() - rawTailSize;
	if (compressedPixelCount != 0)
		pixels[outputOffset++] = colorMap[previousColor];

	const bool repeatCodeEnabled = compressionMode == 2 || compressionMode == 6;
	while (outputOffset < compressedPixelCount) {
		if (sourceOffset >= payload.size())
			return false;
		const byte code = (payload[sourceOffset] >> nibbleShift) & 0xf;
		nibbleShift ^= 4;
		if (nibbleShift == 0)
			++sourceOffset;

		if (code == 0xf) {
			if (!readPackedByte(payload, sourceOffset, nibbleShift, previousColor))
				return false;
			pixels[outputOffset++] = colorMap[previousColor];
		} else if (code == 0xe && repeatCodeEnabled) {
			byte repeatByte = 0;
			if (!readPackedByte(payload, sourceOffset, nibbleShift, repeatByte))
				return false;
			uint32 repeatCount = repeatByte;
			if (repeatByte == 0xff) {
				byte high = 0;
				byte low = 0;
				if (!readPackedByte(payload, sourceOffset, nibbleShift, high) ||
					!readPackedByte(payload, sourceOffset, nibbleShift, low))
					return false;
				repeatCount = ((uint32)high << 8) | low;
			}
			repeatCount += 2;
			if (repeatCount > compressedPixelCount - outputOffset)
				return false;
			memset(pixels.data() + outputOffset, pixels[outputOffset - 1], repeatCount);
			outputOffset += repeatCount;
		} else {
			uint color = previousColor + deltaTable[code];
			if (color >= colorCount)
				color -= colorCount;
			previousColor = (byte)color;
			pixels[outputOffset++] = colorMap[previousColor];
		}
	}

	if (rawTailSize > payload.size() - sourceOffset)
		return false;
	for (uint32 i = 0; i < rawTailSize; ++i)
		pixels[outputOffset++] = colorMap[payload[sourceOffset++]];
	return outputOffset == pixels.size();
}

static bool decodeCustomBitmap(Common::SeekableReadStream &stream, BitmapAssetFrame &frame) {
	// DecodeCustomBitmapAsset at 0x53f60 reads this 0x1c-byte header and optional GCMS extension.
	byte header[kCustomBitmapHeaderSize];
	if (stream.read(header, sizeof(header)) != sizeof(header))
		return false;

	const byte flags = header[0];
	const uint colorCount = (uint)header[1] + 1;
	const uint paletteColorCount = (uint)header[2] + 1;
	const byte transparentColor = header[3];
	const byte compressionMode = header[4];
	uint32 rawTailSize = READ_LE_UINT16(header + 0x14);
	uint32 payloadSize = READ_LE_UINT16(header + 0x1a);
	if (compressionMode >= 4 && compressionMode <= 6) {
		byte extendedHeader[kCustomBitmapExtendedHeaderSize];
		if (stream.read(extendedHeader, sizeof(extendedHeader)) != sizeof(extendedHeader) ||
			READ_BE_UINT32(extendedHeader) != MKTAG('G', 'C', 'M', 'S'))
			return false;
		rawTailSize = READ_LE_UINT32(extendedHeader + 4);
		payloadSize = READ_LE_UINT32(extendedHeader + 8);
	} else if (compressionMode != 1 && compressionMode != 2) {
		return false;
	}

	// DecodeCustomBitmapAsset at 0x53f60 stores descriptor height first and width second.
	const uint16 height = READ_LE_UINT16(header + 0x16);
	const uint16 width = READ_LE_UINT16(header + 0x18);
	const uint32 pixelCount = (uint32)width * height;
	const int64 streamSize = stream.size();
	const int64 streamPosition = stream.pos();
	if (width == 0 || height == 0 || rawTailSize > pixelCount ||
			payloadSize == 0 || streamPosition < 0 || streamSize < streamPosition ||
			payloadSize > (uint64)(streamSize - streamPosition))
		return false;

	Common::Array<byte> payload;
	payload.resize(payloadSize);
	if (stream.read(payload.data(), payload.size()) != payload.size())
		return false;

	BitmapAssetFrame decodedFrame;
	decodedFrame.width = width;
	decodedFrame.height = height;
	if ((flags & 1) != 0) {
		const uint32 paletteSize = paletteColorCount * 3;
		decodedFrame.palette.resize(paletteSize);
		if (stream.read(decodedFrame.palette.data(), paletteSize) != paletteSize)
			return false;
		for (uint i = 0; i < decodedFrame.palette.size(); ++i)
			decodedFrame.palette[i] =
				(decodedFrame.palette[i] << 2) | (decodedFrame.palette[i] >> 4);
	}

	byte colorMap[256];
	if ((flags & 2) != 0) {
		// DecodeCustomBitmapAsset at 0x54106 reads only colorCount bytes into
		// its zero-initialized remap table. This matters for assets such as the
		// combat explosions, whose 0xff transparency key lies beyond that range
		// and therefore resolves to palette index 0.
		memset(colorMap, 0, sizeof(colorMap));
		if (stream.read(colorMap, colorCount) != colorCount)
			return false;
	} else {
		for (uint i = 0; i < ARRAYSIZE(colorMap); ++i)
			colorMap[i] = i;
	}

	decodedFrame.transparentColor = colorMap[transparentColor];
	decodedFrame.pixels.resize(pixelCount);
	// ExpandCustomBitmapCompressedPixels at 0x53de0 consumes low nibbles before high nibbles.
	if (!expandCustomBitmap(payload, header + 5, colorCount, rawTailSize, compressionMode,
			colorMap, decodedFrame.pixels))
		return false;
	frame = Common::move(decodedFrame);
	return true;
}

static bool copyIndexedSurface(const Graphics::Surface *surface,
		const Graphics::Palette &palette, byte transparentColor,
		BitmapAssetFrame &frame) {
	if (!surface || surface->format.bytesPerPixel != 1 || surface->w <= 0 || surface->h <= 0)
		return false;

	frame.width = surface->w;
	frame.height = surface->h;
	frame.transparentColor = transparentColor;
	frame.pixels.resize((uint32)frame.width * frame.height);
	for (uint y = 0; y < frame.height; ++y)
		memcpy(frame.pixels.data() + y * frame.width, surface->getBasePtr(0, y), frame.width);

	frame.palette.resize(palette.size() * 3);
	if (!frame.palette.empty())
		memcpy(frame.palette.data(), palette.data(), frame.palette.size());
	return true;
}

bool decodePcxAsset(Common::SeekableReadStream &stream, BitmapAssetFrame &frame) {
	Image::PCXDecoder decoder;
	return decoder.loadStream(stream) &&
		copyIndexedSurface(decoder.getSurface(), decoder.getPalette(), 0, frame);
}

static bool decodeIffBitmap(Common::SeekableReadStream &stream, BitmapAssetFrame &frame) {
	IFFDecoder decoder;
	if (!decoder.loadStream(stream))
		return false;

	// DecodeIffBitmapAssetToDescriptor at 0x6aca4 maps IFF height/width to the same descriptor order.
	if (!copyIndexedSurface(decoder.getSurface(), decoder.getPalette(),
			decoder.getHeader()->transparentColor & 0xff, frame))
		return false;
	debugC(3, kDebugResources,
		"Ripper: decoded IFF bitmap width=%u height=%u transparent=%u colors=%u",
		frame.width, frame.height, frame.transparentColor, decoder.getPalette().size());
	return true;
}

bool decodeBitmapAsset(Common::SeekableReadStream &stream, BitmapAssetFrame &frame) {
	byte magic[4];
	if (stream.read(magic, sizeof(magic)) != sizeof(magic))
		return false;
	stream.seek(0);
	if (READ_BE_UINT32(magic) == MKTAG('F', 'O', 'R', 'M'))
		return decodeIffBitmap(stream, frame);
	return decodeCustomBitmap(stream, frame);
}

bool decodeBitmapAssetSequence(Common::SeekableReadStream &stream,
		BitmapAssetSequence &sequence) {
	Common::Array<byte> data;
	if (!readStream(stream, data) || data.size() < 4)
		return false;

	sequence.frames.clear();
	if (READ_BE_UINT32(data.data()) == MKTAG('F', 'O', 'R', 'M')) {
		Common::MemoryReadStream bitmapStream(data.data(), data.size(), DisposeAfterUse::NO);
		BitmapAssetFrame frame;
		if (!decodeIffBitmap(bitmapStream, frame))
			return false;
		sequence.frames.push_back(Common::move(frame));
		return true;
	}

	if (data.size() < 6)
		return false;
	const uint entryCount = READ_LE_UINT16(data.data());
	const uint32 directoryOffset = READ_LE_UINT32(data.data() + 2);
	if (entryCount == 0 || directoryOffset < 6 ||
		(uint64)directoryOffset + (uint64)entryCount * 12 > data.size())
		return false;

	for (uint i = 0; i < entryCount; ++i) {
		const uint32 entryOffset = READ_LE_UINT32(data.data() + directoryOffset + i * 12);
		const uint32 endOffset = i + 1 < entryCount ?
			READ_LE_UINT32(data.data() + directoryOffset + (i + 1) * 12) : directoryOffset;
		if (entryOffset < 6 || endOffset <= entryOffset || endOffset > directoryOffset)
			return false;

		Common::MemoryReadStream bitmapStream(data.data() + entryOffset,
			endOffset - entryOffset, DisposeAfterUse::NO);
		BitmapAssetFrame frame;
		if (!decodeBitmapAsset(bitmapStream, frame))
			return false;
		if (frame.palette.empty() && !sequence.frames.empty())
			frame.palette = sequence.frames.front().palette;
		sequence.frames.push_back(Common::move(frame));
	}
	return true;
}

bool decodePresentationFrameRegionTable(Common::SeekableReadStream &stream,
		PresentationFrameRegionTable &table) {
	// LoadPresentationFrameRegionTable at 0x35615 skips the 0x64-byte tool
	// header, then separates frameCount * 6 state bytes from 9-byte regions.
	table.frames.clear();
	if (stream.size() < 0x68)
		return false;

	stream.seek(0x64);
	const uint32 frameCount = stream.readUint32LE();
	if (frameCount == 0 || frameCount > 10000 ||
			(uint64)0x68 + (uint64)frameCount * 6 > (uint64)stream.size())
		return false;

	Common::Array<byte> frameRecords;
	frameRecords.resize(frameCount * 6);
	if (stream.read(frameRecords.data(), frameRecords.size()) != frameRecords.size())
		return false;

	uint regionCount = 0;
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte *record = frameRecords.data() + frame * 6;
		const uint firstRegion = READ_LE_UINT16(record + 4);
		regionCount = MAX<uint>(regionCount, firstRegion + record[1]);
	}
	if ((uint64)stream.pos() + (uint64)regionCount * 9 > (uint64)stream.size())
		return false;

	Common::Array<byte> regionRecords;
	regionRecords.resize(regionCount * 9);
	if (!regionRecords.empty() &&
			stream.read(regionRecords.data(), regionRecords.size()) != regionRecords.size())
		return false;

	table.frames.resize(frameCount);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte *record = frameRecords.data() + frame * 6;
		PresentationFrameRegion &destination = table.frames[frame];
		destination.state = record[0];
		destination.auxiliary = record[2];
		const uint firstRegion = READ_LE_UINT16(record + 4);
		for (uint regionIndex = 0; regionIndex < record[1]; ++regionIndex) {
			const byte *source =
				regionRecords.data() + (firstRegion + regionIndex) * 9;
			PresentationRegion region;
			region.type = source[0];
			region.coordinate1 = (int16)READ_LE_UINT16(source + 1);
			region.coordinate2 = (int16)READ_LE_UINT16(source + 3);
			region.extent1 = (int16)READ_LE_UINT16(source + 5);
			region.extent2 = (int16)READ_LE_UINT16(source + 7);
			destination.regions.push_back(region);
		}
	}
	return !stream.err();
}

bool decodePresentationFrameAudioMap(Common::SeekableReadStream &stream,
		uint frameCount, PresentationFrameAudioMap &map) {
	// LoadPresentationFrameAudioCueMap at 0x3574a reads the 16-bit rate and
	// optional 16-bit frame count, 60-byte source paths, then one
	// cue-index/volume pair for every presentation frame.
	map = PresentationFrameAudioMap();
	if (stream.size() < 0x6c)
		return false;

	stream.seek(0x64);
	map.frameRate = stream.readUint16LE();
	const uint16 storedFrameCount = stream.readUint16LE();
	const uint32 soundCount = stream.readUint32LE();
	if ((storedFrameCount != 0 && storedFrameCount != frameCount) ||
			soundCount > 256 ||
			(uint64)stream.pos() + (uint64)soundCount * 60 +
				(uint64)frameCount * 2 > (uint64)stream.size())
		return false;

	for (uint sound = 0; sound < soundCount; ++sound) {
		char path[60];
		if (stream.read(path, sizeof(path)) != sizeof(path))
			return false;
		uint length = 0;
		while (length < sizeof(path) && path[length] != '\0')
			++length;
		map.sounds.push_back(Common::String(path, path + length));
	}

	map.cues.resize(frameCount);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte soundIndex = stream.readByte();
		map.cues[frame].volume = stream.readByte();
		if (soundIndex != 0xff && soundIndex < soundCount)
			map.cues[frame].soundIndex = soundIndex;
	}
	return !stream.err();
}

AssetLibrary::AssetLibrary() : _looseArchive(nullptr), _modernFormat(false) {
}

Common::String AssetLibrary::readFixedString(const char *data, uint length) {
	uint actualLength = 0;
	while (actualLength < length && data[actualLength] != '\0')
		++actualLength;
	return Common::String(data, data + actualLength);
}

Common::String AssetLibrary::normalizeMemberName(const Common::String &memberName, bool includeExtension) {
	uint baseStart = 0;
	for (uint i = 0; i < memberName.size(); ++i) {
		if (memberName[i] == '/' || memberName[i] == '\\' || memberName[i] == ':')
			baseStart = i + 1;
	}

	uint dot = memberName.size();
	for (uint i = baseStart; i < memberName.size(); ++i) {
		if (memberName[i] == '.' && memberName.size() - i <= 4)
			dot = i;
	}

	const uint baseEnd = dot < memberName.size() ? dot : memberName.size();
	Common::String key;
	for (uint i = baseStart; i < baseEnd && key.size() < 8; ++i)
		key += memberName[i];

	if (includeExtension && dot < memberName.size()) {
		key += '.';
		for (uint i = dot + 1; i < memberName.size() && i < dot + 4; ++i)
			key += memberName[i];
	}

	key.toLowercase();
	return key;
}

bool AssetLibrary::open(const Common::Path &filename) {
	Common::File file;
	_entries.clear();
	_entryIndices.clear();
	_archiveData.clear();
	_filename = filename;
	_looseArchive = nullptr;
	_modernFormat = false;

	if (filename.empty()) {
		warning("Ripper: cannot open an asset library with an empty filename");
		return false;
	}
	if (!file.open(filename)) {
		warning("Ripper: could not open asset library '%s'", filename.toString().c_str());
		return false;
	}

	const int64 fileSize64 = file.size();
	if (fileSize64 < 6 || fileSize64 > 0xffffffffLL) {
		warning("Ripper: invalid asset library size %lld for '%s'", fileSize64, filename.toString().c_str());
		return false;
	}
	return loadDirectory(file, (uint32)fileSize64);
}

bool AssetLibrary::open(Common::SeekableReadStream &stream, const Common::Path &sourceName) {
	_entries.clear();
	_entryIndices.clear();
	_archiveData.clear();
	_filename = sourceName;
	_looseArchive = nullptr;
	_modernFormat = false;
	if (!readStream(stream, _archiveData) || _archiveData.size() < 6)
		return false;

	Common::MemoryReadStream memory(_archiveData.data(), _archiveData.size(), DisposeAfterUse::NO);
	return loadDirectory(memory, _archiveData.size());
}

bool AssetLibrary::openLooseFiles(const Common::Archive &archive) {
	_entries.clear();
	_entryIndices.clear();
	_archiveData.clear();
	_filename = Common::Path();
	_looseArchive = &archive;
	_modernFormat = false;
	return true;
}

bool AssetLibrary::loadDirectory(Common::SeekableReadStream &file, uint32 fileSize) {
	file.seek(0);
	const uint16 entryCount = file.readUint16LE();
	const uint32 directoryOffset = file.readUint32LE();
	uint32 payloadStart = 6;

	if (fileSize >= 10) {
		const uint32 magic = file.readUint32LE();
		if (magic == kModernLibraryMagic) {
			_modernFormat = true;
			payloadStart = 10;
		}
	}

	const uint32 recordSize = _modernFormat ? 20 : 12;
	const uint64 tableSize = (uint64)entryCount * recordSize;
	if (directoryOffset < payloadStart || (uint64)directoryOffset + tableSize > fileSize) {
		warning("Ripper: invalid directory range offset=%u count=%u recordSize=%u fileSize=%u in '%s'",
			directoryOffset, entryCount, recordSize, fileSize, _filename.toString().c_str());
		return false;
	}

	file.seek(directoryOffset);
	for (uint i = 0; i < entryCount; ++i) {
		Entry entry;
		entry.offset = file.readUint32LE();
		char baseName[8];
		if (file.read(baseName, sizeof(baseName)) != sizeof(baseName)) {
			warning("Ripper: truncated directory entry %u in '%s'", i, _filename.toString().c_str());
			return false;
		}

		Common::String name = readFixedString(baseName, sizeof(baseName));
		entry.timestamp = 0;
		if (_modernFormat) {
			char extension[4];
			if (file.read(extension, sizeof(extension)) != sizeof(extension))
				return false;
			const Common::String ext = readFixedString(extension, sizeof(extension));
			if (!ext.empty())
				name += "." + ext;
			entry.timestamp = file.readUint32LE();
		}

		entry.key = normalizeMemberName(name, _modernFormat);
		entry.size = 0;
		if (entry.key.empty() || entry.offset < payloadStart || entry.offset > directoryOffset) {
			warning("Ripper: invalid directory entry %u key='%s' offset=%u in '%s'",
				i, entry.key.c_str(), entry.offset, _filename.toString().c_str());
			return false;
		}
		if (!_entries.empty() && entry.offset < _entries.back().offset) {
			warning("Ripper: non-monotonic directory entry %u in '%s'", i, _filename.toString().c_str());
			return false;
		}
		if (_entryIndices.contains(entry.key)) {
			warning("Ripper: duplicate member '%s' in '%s'", entry.key.c_str(), _filename.toString().c_str());
			return false;
		}

		_entries.push_back(entry);
		_entryIndices[entry.key] = _entries.size() - 1;
	}

	for (uint i = 0; i < _entries.size(); ++i) {
		const uint32 end = i + 1 < _entries.size() ? _entries[i + 1].offset : directoryOffset;
		if (end < _entries[i].offset) {
			warning("Ripper: invalid member range for '%s' in '%s'",
				_entries[i].key.c_str(), _filename.toString().c_str());
			return false;
		}
		_entries[i].size = end - _entries[i].offset;
		debugC(3, kDebugResources,
			"Ripper: archive '%s' member=%u key='%s' offset=%u size=%u timestamp=0x%08x",
			_filename.toString().c_str(), i, _entries[i].key.c_str(), _entries[i].offset,
			_entries[i].size, _entries[i].timestamp);
	}

	debugC(1, kDebugResources, "Ripper: opened %s asset library '%s' with %u entries",
		_modernFormat ? "2BIL" : "legacy", _filename.toString().c_str(), _entries.size());
	return true;
}

const AssetLibrary::Entry *AssetLibrary::findEntry(const Common::String &memberName) const {
	const Common::String key = normalizeMemberName(memberName, _modernFormat);
	Common::HashMap<Common::String, uint>::const_iterator it = _entryIndices.find(key);
	return it != _entryIndices.end() ? &_entries[it->_value] : nullptr;
}

bool AssetLibrary::hasMember(const Common::String &memberName) const {
	if (_looseArchive)
		return !memberName.empty() && _looseArchive->hasFile(Common::Path(memberName));
	return findEntry(memberName) != nullptr;
}

Common::SeekableReadStream *AssetLibrary::createReadStreamForMember(const Common::String &memberName) const {
	if (_looseArchive) {
		if (memberName.empty()) {
			warning("Ripper: cannot open an empty loose resource name");
			return nullptr;
		}

		Common::SeekableReadStream *stream =
			_looseArchive->createReadStreamForMember(Common::Path(memberName));
		if (!stream) {
			warning("Ripper: loose resource '%s' was not found", memberName.c_str());
			return nullptr;
		}
		debugC(2, kDebugResources, "Ripper: opening loose resource '%s'",
			memberName.c_str());
		return stream;
	}

	const Entry *entry = findEntry(memberName);
	if (!entry) {
		warning("Ripper: member '%s' was not found in '%s'", memberName.c_str(), _filename.toString().c_str());
		return nullptr;
	}

	if (!_archiveData.empty()) {
		debugC(2, kDebugResources,
			"Ripper: opening member '%s' from nested '%s' offset=%u size=%u",
			memberName.c_str(), _filename.toString().c_str(), entry->offset, entry->size);
		return new Common::MemoryReadStream(_archiveData.data() + entry->offset,
			entry->size, DisposeAfterUse::NO);
	}

	Common::ScopedPtr<Common::File> file(new Common::File());
	if (!file->open(_filename)) {
		warning("Ripper: could not reopen asset library '%s' for member '%s'",
			_filename.toString().c_str(), memberName.c_str());
		return nullptr;
	}

	debugC(2, kDebugResources, "Ripper: opening member '%s' from '%s' offset=%u size=%u",
		memberName.c_str(), _filename.toString().c_str(), entry->offset, entry->size);
	return new Common::SeekableSubReadStream(file.release(), entry->offset,
		entry->offset + entry->size,
		DisposeAfterUse::YES);
}

void AssetLibrary::listMembersWithPrefix(const Common::String &prefix,
		Common::Array<Common::String> &members) const {
	members.clear();
	const Common::String keyPrefix = normalizeMemberName(prefix, false);
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].key.hasPrefix(keyPrefix))
			members.push_back(_entries[i].key);
	}
}

ResourceManager::ResourceManager() : _gameTextLoaded(false) {
}

bool ResourceManager::initialize(bool loadNestedOptions) {
	_fontCache.clear();
	_gameTextCache.clear();
	_gameTextLoaded = false;
	_searchDirectories.clear();

	Common::File iniFile;
	Common::INIFile ini;
	if (!iniFile.open("ripper.ini") || !ini.loadFromStream(iniFile)) {
		warning("Ripper: could not load RIPPER.INI");
		return false;
	}

	Common::String scriptLibrary;
	Common::String interfaceLibrary;
	Common::String puzzleLibrary;
	Common::String soundLibrary;
	if (!ini.getKey("script", "files", scriptLibrary) ||
		!ini.getKey("interface", "files", interfaceLibrary) ||
		!ini.getKey("sound", "files", soundLibrary)) {
		warning("Ripper: RIPPER.INI is missing FILES script, interface, or sound entries");
		return false;
	}
	ini.getKey("puzzle", "files", puzzleLibrary);

	debugC(2, kDebugResources,
		"Ripper: RIPPER.INI script='%s' interface='%s' puzzle='%s' sound='%s'",
		scriptLibrary.c_str(), interfaceLibrary.c_str(), puzzleLibrary.c_str(),
		soundLibrary.c_str());
	Common::String puzzleDirectory;
	ini.getKey("puzzle", "paths", puzzleDirectory);
	puzzleDirectory.trim();
	static const char *const searchDirectoryKeys[] = {
		"scene",
		"puzzle",
		"combat",
		"cyber"
	};
	for (uint i = 0; i < ARRAYSIZE(searchDirectoryKeys); ++i) {
		Common::String directory;
		if (ini.getKey(searchDirectoryKeys[i], "paths", directory)) {
			directory.trim();
			if (!directory.empty())
				_searchDirectories.push_back(directory);
		}
	}
	debugC(2, kDebugResources,
		"Ripper: configured explicit resource fallback directories=%u",
		_searchDirectories.size());
	const bool scriptSourceReady = scriptLibrary.empty() ?
		_scripts.openLooseFiles(SearchMan) :
		_scripts.open(Common::Path(scriptLibrary));
	if (scriptLibrary.empty())
		debugC(1, kDebugResources, "Ripper: using loose script resources from the game directory");
	Common::Path puzzleLibraryPath(puzzleLibrary);
	if (!puzzleLibrary.empty() && !puzzleDirectory.empty())
		puzzleLibraryPath = Common::Path(puzzleDirectory).appendComponent(puzzleLibrary);
	const bool puzzleSourceReady = puzzleLibrary.empty() ||
		_puzzle.open(puzzleLibraryPath);
	if (puzzleLibrary.empty())
		debugC(1, kDebugResources,
			"Ripper: using loose puzzle resources from configured search paths");
	if (!scriptSourceReady ||
		!_interface.open(Common::Path(interfaceLibrary)) ||
		!puzzleSourceReady ||
		!_sound.open(Common::Path(soundLibrary)))
		return false;
	if (!loadNestedOptions) {
		debugC(1, kDebugResources,
			"Ripper: skipped retail nested OPTIONS.PL for loose-script distribution");
		return true;
	}
	Common::ScopedPtr<Common::SeekableReadStream> optionsStream(
		_interface.createReadStreamForMember("options.pl"));
	if (!optionsStream || !_options.open(*optionsStream, Common::Path("options.pl"))) {
		warning("Ripper: could not open nested OPTIONS.PL asset library");
		return false;
	}
	return true;
}

Common::SeekableReadStream *ResourceManager::createReadStreamForPath(
		const Common::String &memberName) const {
	const Common::Path directPath(memberName);
	if (SearchMan.hasFile(directPath))
		return SearchMan.createReadStreamForMember(directPath);
	if (hasDirectorySeparator(memberName))
		return nullptr;

	for (uint i = 0; i < _searchDirectories.size(); ++i) {
		const Common::Path qualifiedPath =
			Common::Path(_searchDirectories[i]).appendComponent(memberName);
		if (!SearchMan.hasFile(qualifiedPath))
			continue;
		Common::SeekableReadStream *stream =
			SearchMan.createReadStreamForMember(qualifiedPath);
		if (stream) {
			debugC(2, kDebugResources,
				"Ripper: resolved resource '%s' through configured path '%s'",
				memberName.c_str(), qualifiedPath.toString().c_str());
			return stream;
		}
	}
	return nullptr;
}

static bool loadBitmapSequenceFromLibrary(const AssetLibrary &library,
		const char *libraryName, const Common::String &memberName,
		BitmapAssetSequence &sequence) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		library.createReadStreamForMember(memberName));
	if (!stream || !decodeBitmapAssetSequence(*stream, sequence)) {
		warning("Ripper: could not decode %s bitmap sequence '%s'",
			libraryName, memberName.c_str());
		return false;
	}

	debugC(2, kDebugResources, "Ripper: decoded %s bitmap sequence '%s' frames=%u",
		libraryName, memberName.c_str(), sequence.frames.size());
	return true;
}

static bool loadPcxFromLibrary(const AssetLibrary &library,
		const char *libraryName, const Common::String &memberName,
		BitmapAssetFrame &frame);

bool ResourceManager::loadInterfaceBitmapSequence(const Common::String &memberName,
		BitmapAssetSequence &sequence) const {
	return loadBitmapSequenceFromLibrary(_interface, "interface", memberName, sequence);
}

bool ResourceManager::loadOptionsBitmapSequence(const Common::String &memberName,
		BitmapAssetSequence &sequence) const {
	return loadBitmapSequenceFromLibrary(_options, "options", memberName, sequence);
}

bool ResourceManager::loadBitmap(const Common::String &memberName,
		BitmapAssetFrame &frame) const {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		SearchMan.createReadStreamForMember(Common::Path(memberName)));
	if (!stream || !decodeBitmapAsset(*stream, frame)) {
		warning("Ripper: could not decode bitmap '%s'", memberName.c_str());
		return false;
	}

	debugC(2, kDebugResources,
		"Ripper: decoded bitmap '%s' size=%ux%u transparent=%u colors=%u",
		memberName.c_str(), frame.width, frame.height, frame.transparentColor,
		frame.palette.size() / 3);
	return true;
}

bool ResourceManager::loadBitmapSequence(const Common::String &memberName,
		BitmapAssetSequence &sequence) const {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		SearchMan.createReadStreamForMember(Common::Path(memberName)));
	if (!stream || !decodeBitmapAssetSequence(*stream, sequence)) {
		warning("Ripper: could not decode bitmap sequence '%s'", memberName.c_str());
		return false;
	}

	debugC(2, kDebugResources, "Ripper: decoded bitmap sequence '%s' frames=%u",
		memberName.c_str(), sequence.frames.size());
	return true;
}

bool ResourceManager::loadBitmapLibrary(const Common::String &libraryName,
		Common::Array<BitmapAssetFrame> &frames) const {
	AssetLibrary library;
	if (!library.open(Common::Path(libraryName)))
		return false;

	Common::Array<Common::String> members;
	library.listMembersWithPrefix("", members);
	frames.clear();
	for (uint member = 0; member < members.size(); ++member) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(
			library.createReadStreamForMember(members[member]));
		BitmapAssetFrame frame;
		if (!stream || !decodeBitmapAsset(*stream, frame)) {
			warning("Ripper: could not decode bitmap library '%s' member '%s'",
				libraryName.c_str(), members[member].c_str());
			frames.clear();
			return false;
		}
		frames.push_back(Common::move(frame));
	}
	debugC(2, kDebugResources,
		"Ripper: decoded bitmap library '%s' members=%u frames=%u",
		libraryName.c_str(), members.size(), frames.size());
	return !frames.empty();
}

bool ResourceManager::loadInterfaceBitmapSet(const Common::String &prefix,
		Common::Array<BitmapAssetFrame> &frames) const {
	Common::Array<Common::String> members;
	_interface.listMembersWithPrefix(prefix, members);
	frames.clear();
	for (uint i = 0; i < members.size(); ++i) {
		BitmapAssetSequence sequence;
		if (!loadInterfaceBitmapSequence(members[i], sequence))
			return false;
		for (uint frame = 0; frame < sequence.frames.size(); ++frame)
			frames.push_back(Common::move(sequence.frames[frame]));
	}
	debugC(2, kDebugResources,
		"Ripper: decoded interface bitmap set prefix='%s' members=%u frames=%u",
		prefix.c_str(), members.size(), frames.size());
	return !frames.empty();
}

bool ResourceManager::loadInterfacePcx(const Common::String &memberName,
		BitmapAssetFrame &frame) const {
	return loadPcxFromLibrary(_interface, "interface", memberName, frame);
}

bool ResourceManager::loadOptionsPcx(const Common::String &memberName,
		BitmapAssetFrame &frame) const {
	return loadPcxFromLibrary(_options, "options", memberName, frame);
}

static bool loadPcxFromLibrary(const AssetLibrary &library,
		const char *libraryName, const Common::String &memberName,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		library.createReadStreamForMember(memberName));
	if (!stream || !decodePcxAsset(*stream, frame)) {
		warning("Ripper: could not decode %s PCX '%s'", libraryName, memberName.c_str());
		return false;
	}

	debugC(2, kDebugResources,
		"Ripper: decoded %s PCX '%s' width=%u height=%u colors=%u",
		libraryName, memberName.c_str(), frame.width, frame.height,
		frame.palette.size() / 3);
	return true;
}

bool ResourceManager::loadInterfaceBitmapFont(const Common::String &memberName,
		BitmapFontAsset &font) const {
	Common::String cacheKey = memberName;
	cacheKey.toLowercase();
	Common::HashMap<Common::String, BitmapFontAsset>::const_iterator cached =
		_fontCache.find(cacheKey);
	if (cached != _fontCache.end()) {
		font = cached->_value;
		debugC(3, kDebugResources, "Ripper: reused cached NF2T font '%s'", memberName.c_str());
		return true;
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_interface.createReadStreamForMember(memberName));
	Common::Array<byte> data;
	if (!stream || !readStream(*stream, data) || data.size() < 12 ||
		READ_BE_UINT32(data.data()) != MKTAG('N', 'F', '2', 'T')) {
		warning("Ripper: could not read NF2T font '%s'", memberName.c_str());
		return false;
	}

	// LoadNF2TDescriptorAndBitmap at 0x5c290 reads the 12-byte descriptor,
	// its eight-byte glyph records, and the trailing custom bitmap.
	const uint glyphCount = data[5];
	const uint32 bitmapOffset = 12 + glyphCount * 8;
	if (glyphCount == 0 || bitmapOffset >= data.size())
		return false;

	BitmapAssetFrame bitmap;
	Common::MemoryReadStream bitmapStream(data.data() + bitmapOffset,
		data.size() - bitmapOffset, DisposeAfterUse::NO);
	if (!decodeCustomBitmap(bitmapStream, bitmap))
		return false;

	font.firstCharacter = data[4];
	font.lineHeight = data[7];
	font.characterSpacing = data[9];
	font.spaceWidth = data[11];
	font.transparentColor = bitmap.transparentColor;
	font.pixels = Common::move(bitmap.pixels);
	font.glyphs.clear();
	font.glyphs.resize(glyphCount);
	for (uint i = 0; i < glyphCount; ++i) {
		const byte *record = data.data() + 12 + i * 8;
		BitmapFontGlyph &glyph = font.glyphs[i];
		glyph.pixelOffset = READ_LE_UINT32(record);
		glyph.width = record[4];
		glyph.height = record[5];
		glyph.xOffset = (int8)record[6];
		glyph.yOffset = (int8)record[7];
		if ((uint64)glyph.pixelOffset + (uint32)glyph.width * glyph.height > font.pixels.size()) {
			warning("Ripper: NF2T font '%s' has invalid glyph %u", memberName.c_str(), i);
			return false;
		}
	}

	debugC(2, kDebugResources,
		"Ripper: decoded NF2T font '%s' first=%u glyphs=%u lineHeight=%u pixels=%u",
		memberName.c_str(), font.firstCharacter, font.glyphs.size(), font.lineHeight,
		font.pixels.size());
	_fontCache[cacheKey] = font;
	return true;
}

bool ResourceManager::loadGameText(Common::Array<Common::String> &strings) const {
	if (_gameTextLoaded) {
		strings = _gameTextCache;
		debugC(3, kDebugResources, "Ripper: reused cached GAMETEXT.TF strings=%u", strings.size());
		return true;
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_scripts.createReadStreamForMember("gametext.tf"));
	Common::Array<byte> data;
	if (!stream || !readStream(*stream, data) || data.size() < 4)
		return false;

	// ResolveStartupResourceString at 0x1f7a2 indexes this one-based offset table.
	const uint32 stringCount = READ_LE_UINT32(data.data());
	if (stringCount == 0 || (uint64)4 + (uint64)stringCount * 4 > data.size())
		return false;

	strings.clear();
	strings.resize(stringCount);
	for (uint i = 0; i < stringCount; ++i) {
		const uint32 offset = READ_LE_UINT32(data.data() + 4 + i * 4);
		const uint32 end = i + 1 < stringCount ?
			READ_LE_UINT32(data.data() + 8 + i * 4) : data.size();
		if (offset >= end || end > data.size() || data[end - 1] != 0)
			return false;
		strings[i] = Common::String((const char *)data.data() + offset,
			(const char *)data.data() + end - 1);
	}

	debugC(2, kDebugResources, "Ripper: decoded GAMETEXT.TF strings=%u", strings.size());
	_gameTextCache = strings;
	_gameTextLoaded = true;
	return true;
}

} // End of namespace Ripper
