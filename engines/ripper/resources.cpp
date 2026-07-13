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

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/substream.h"
#include "graphics/surface.h"
#include "image/iff.h"

#include "ripper/detection.h"

namespace Ripper {

static const uint32 kModernLibraryMagic = 0x4c494232;
static const uint32 kCustomBitmapHeaderSize = 0x1c;
static const uint32 kCustomBitmapExtendedHeaderSize = 0xc;

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
	frame.height = READ_LE_UINT16(header + 0x16);
	frame.width = READ_LE_UINT16(header + 0x18);
	const uint32 pixelCount = (uint32)frame.width * frame.height;
	if (frame.width == 0 || frame.height == 0 || rawTailSize > pixelCount ||
		payloadSize == 0 || payloadSize > (uint64)stream.size() - stream.pos())
		return false;

	Common::Array<byte> payload;
	payload.resize(payloadSize);
	if (stream.read(payload.data(), payload.size()) != payload.size())
		return false;

	frame.palette.clear();
	if ((flags & 1) != 0) {
		const uint32 paletteSize = paletteColorCount * 3;
		frame.palette.resize(paletteSize);
		if (stream.read(frame.palette.data(), paletteSize) != paletteSize)
			return false;
		for (uint i = 0; i < frame.palette.size(); ++i)
			frame.palette[i] = (frame.palette[i] << 2) | (frame.palette[i] >> 4);
	}

	byte colorMap[256];
	for (uint i = 0; i < ARRAYSIZE(colorMap); ++i)
		colorMap[i] = i;
	if ((flags & 2) != 0 && stream.read(colorMap, colorCount) != colorCount)
		return false;

	frame.transparentColor = colorMap[transparentColor];
	frame.pixels.resize(pixelCount);
	// ExpandCustomBitmapCompressedPixels at 0x53de0 consumes low nibbles before high nibbles.
	return expandCustomBitmap(payload, header + 5, colorCount, rawTailSize, compressionMode,
		colorMap, frame.pixels);
}

static bool decodeIffBitmap(Common::SeekableReadStream &stream, BitmapAssetFrame &frame) {
	Image::IFFDecoder decoder;
	if (!decoder.loadStream(stream))
		return false;

	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || surface->format.bytesPerPixel != 1 || surface->w <= 0 || surface->h <= 0)
		return false;

	// DecodeIffBitmapAssetToDescriptor at 0x6aca4 maps IFF height/width to the same descriptor order.
	frame.width = surface->w;
	frame.height = surface->h;
	frame.transparentColor = decoder.getHeader()->transparentColor & 0xff;
	frame.pixels.resize((uint32)frame.width * frame.height);
	for (uint y = 0; y < frame.height; ++y)
		memcpy(frame.pixels.data() + y * frame.width, surface->getBasePtr(0, y), frame.width);

	const Graphics::Palette &palette = decoder.getPalette();
	frame.palette.resize(palette.size() * 3);
	if (!frame.palette.empty())
		memcpy(frame.palette.data(), palette.data(), frame.palette.size());
	return true;
}

static bool decodeBitmap(Common::SeekableReadStream &stream, BitmapAssetFrame &frame) {
	byte magic[4];
	if (stream.read(magic, sizeof(magic)) != sizeof(magic))
		return false;
	stream.seek(0);
	if (READ_BE_UINT32(magic) == MKTAG('F', 'O', 'R', 'M'))
		return decodeIffBitmap(stream, frame);
	return decodeCustomBitmap(stream, frame);
}

static bool decodeBitmapSequence(Common::SeekableReadStream &stream,
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
		if (!decodeBitmap(bitmapStream, frame))
			return false;
		if (frame.palette.empty() && !sequence.frames.empty())
			frame.palette = sequence.frames.front().palette;
		sequence.frames.push_back(Common::move(frame));
	}
	return true;
}

AssetLibrary::AssetLibrary() : _modernFormat(false) {
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
	_filename = filename;
	_modernFormat = false;

	if (!file.open(filename)) {
		warning("Ripper: could not open asset library '%s'", filename.toString().c_str());
		return false;
	}

	const int64 fileSize64 = file.size();
	if (fileSize64 < 6 || fileSize64 > 0xffffffffLL) {
		warning("Ripper: invalid asset library size %lld for '%s'", fileSize64, filename.toString().c_str());
		return false;
	}
	const uint32 fileSize = (uint32)fileSize64;
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
			directoryOffset, entryCount, recordSize, fileSize, filename.toString().c_str());
		return false;
	}

	file.seek(directoryOffset);
	for (uint i = 0; i < entryCount; ++i) {
		Entry entry;
		entry.offset = file.readUint32LE();
		char baseName[8];
		if (file.read(baseName, sizeof(baseName)) != sizeof(baseName)) {
			warning("Ripper: truncated directory entry %u in '%s'", i, filename.toString().c_str());
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
				i, entry.key.c_str(), entry.offset, filename.toString().c_str());
			return false;
		}
		if (!_entries.empty() && entry.offset < _entries.back().offset) {
			warning("Ripper: non-monotonic directory entry %u in '%s'", i, filename.toString().c_str());
			return false;
		}
		for (uint existing = 0; existing < _entries.size(); ++existing) {
			if (_entries[existing].key == entry.key) {
				warning("Ripper: duplicate member '%s' in '%s'", entry.key.c_str(), filename.toString().c_str());
				return false;
			}
		}

		_entries.push_back(entry);
	}

	for (uint i = 0; i < _entries.size(); ++i) {
		const uint32 end = i + 1 < _entries.size() ? _entries[i + 1].offset : directoryOffset;
		if (end < _entries[i].offset) {
			warning("Ripper: invalid member range for '%s' in '%s'",
				_entries[i].key.c_str(), filename.toString().c_str());
			return false;
		}
		_entries[i].size = end - _entries[i].offset;
		debugC(3, kDebugResources,
			"Ripper: archive '%s' member=%u key='%s' offset=%u size=%u timestamp=0x%08x",
			filename.toString().c_str(), i, _entries[i].key.c_str(), _entries[i].offset,
			_entries[i].size, _entries[i].timestamp);
	}

	debugC(1, kDebugResources, "Ripper: opened %s asset library '%s' with %u entries",
		_modernFormat ? "2BIL" : "legacy", filename.toString().c_str(), _entries.size());
	return true;
}

const AssetLibrary::Entry *AssetLibrary::findEntry(const Common::String &memberName) const {
	const Common::String key = normalizeMemberName(memberName, _modernFormat);
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].key == key)
			return &_entries[i];
	}
	return nullptr;
}

bool AssetLibrary::hasMember(const Common::String &memberName) const {
	return findEntry(memberName) != nullptr;
}

Common::SeekableReadStream *AssetLibrary::createReadStreamForMember(const Common::String &memberName) const {
	const Entry *entry = findEntry(memberName);
	if (!entry) {
		warning("Ripper: member '%s' was not found in '%s'", memberName.c_str(), _filename.toString().c_str());
		return nullptr;
	}

	Common::File *file = new Common::File();
	if (!file->open(_filename)) {
		warning("Ripper: could not reopen asset library '%s' for member '%s'",
			_filename.toString().c_str(), memberName.c_str());
		delete file;
		return nullptr;
	}

	debugC(2, kDebugResources, "Ripper: opening member '%s' from '%s' offset=%u size=%u",
		memberName.c_str(), _filename.toString().c_str(), entry->offset, entry->size);
	return new Common::SeekableSubReadStream(file, entry->offset, entry->offset + entry->size,
		DisposeAfterUse::YES);
}

bool ResourceManager::initialize() {
	Common::File iniFile;
	Common::INIFile ini;
	if (!iniFile.open("ripper.ini") || !ini.loadFromStream(iniFile)) {
		warning("Ripper: could not load RIPPER.INI");
		return false;
	}

	Common::String scriptLibrary;
	Common::String interfaceLibrary;
	Common::String soundLibrary;
	if (!ini.getKey("script", "files", scriptLibrary) ||
		!ini.getKey("interface", "files", interfaceLibrary) ||
		!ini.getKey("sound", "files", soundLibrary)) {
		warning("Ripper: RIPPER.INI is missing FILES script, interface, or sound entries");
		return false;
	}

	debugC(2, kDebugResources, "Ripper: RIPPER.INI script='%s' interface='%s' sound='%s'",
		scriptLibrary.c_str(), interfaceLibrary.c_str(), soundLibrary.c_str());
	return _scripts.open(Common::Path(scriptLibrary)) &&
		_interface.open(Common::Path(interfaceLibrary)) &&
		_sound.open(Common::Path(soundLibrary));
}

bool ResourceManager::loadInterfaceBitmapSequence(const Common::String &memberName,
		BitmapAssetSequence &sequence) const {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_interface.createReadStreamForMember(memberName));
	if (!stream || !decodeBitmapSequence(*stream, sequence)) {
		warning("Ripper: could not decode interface bitmap sequence '%s'", memberName.c_str());
		return false;
	}

	debugC(2, kDebugResources, "Ripper: decoded interface bitmap sequence '%s' frames=%u",
		memberName.c_str(), sequence.frames.size());
	return true;
}

} // End of namespace Ripper
