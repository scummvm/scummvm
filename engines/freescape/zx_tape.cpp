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

#include "freescape/zx_tape.h"

#include "common/endian.h"
#include "common/file.h"
#include "common/formats/spectrum_tape.h"
#include "common/fs.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"

namespace Freescape {

class ZxRecordingDecoder {
public:
	ZxRecordingDecoder(Common::SpectrumTapeBlocks &blocks) : _blocks(blocks) {}
	bool decode(const Common::Array<byte> &data);
	void finish();

private:
	void pulse(uint32 length);
	void endBlock();
	Common::SpectrumTapeBlocks &_blocks;
	Common::Array<byte> _tap;
	uint32 _length = 0;
	uint _pilot = 0, _bits = 0;
	int _level = -1, _halfBit = -1;
	byte _byte = 0, _checksum = 0;
	bool _sync = false, _reading = false;
};

void ZxRecordingDecoder::endBlock() {
	if (!_bits && !_checksum && _tap.size() >= 2 && (_tap[0] == 0 || _tap[0] == 255)) {
		Common::SpectrumTapeBlock block;
		block.id = 0x10;
		block.tap = _tap;
		_blocks.push_back(block);
	}
	_tap.clear();
	_bits = _byte = _checksum = 0;
	_halfBit = -1;
	_reading = false;
}

void ZxRecordingDecoder::pulse(uint32 length) {
	// ROM tape encoding: a pilot and two sync pulses, then two pulses per bit.
	if (_reading) {
		int bit = length >= 400 && length < 1300 ? 0 : length >= 1300 && length <= 2050 ? 1 : -1;
		if (bit < 0 || (_halfBit >= 0 && bit != _halfBit) || _tap.size() > 65536) {
			endBlock();
		} else {
			if (_halfBit < 0)
				_halfBit = bit;
			else {
				_halfBit = -1;
				_byte = (_byte << 1) | bit;
				if (++_bits == 8) {
					_tap.push_back(_byte);
					_checksum ^= _byte;
					_bits = _byte = 0;
				}
			}
			return;
		}
	}
	if (_sync) {
		_reading = length >= 400 && length <= 1100;
		_sync = false;
	} else if (length >= 1900 && length <= 2500) {
		++_pilot;
	} else {
		_sync = _pilot >= 256 && length >= 400 && length <= 1100;
		_pilot = 0;
	}
}

bool ZxRecordingDecoder::decode(const Common::Array<byte> &data) {
	if (data.size() < 9 || !READ_LE_UINT16(data.data()) || data[4] < 1 || data[4] > 8 ||
			READ_LE_UINT24(data.data() + 5) != data.size() - 8)
		return false;
	uint period = READ_LE_UINT16(data.data());
	for (uint i = 8; i < data.size(); ++i) {
		uint bits = i + 1 == data.size() ? data[4] : 8;
		for (uint bit = 0; bit < bits; ++bit) {
			int level = (data[i] >> (7 - bit)) & 1;
			if (level != _level) {
				if (_length)
					pulse(_length);
				_level = level;
				_length = 0;
			}
			_length = MIN<uint32>(10000, _length + period);
		}
	}
	if (READ_LE_UINT16(data.data() + 2))
		finish();
	return true;
}

void ZxRecordingDecoder::finish() {
	if (_length)
		pulse(_length);
	endBlock();
	_length = _pilot = 0;
	_level = -1;
	_sync = false;
}

bool unpackZxSpectrumBlock(Common::Array<byte> &data) {
	// Recognize the self-extracting LZ/RLE loader before reading its operands.
	const byte loader[] = { 0x01, 0x34, 0x00, 0x11, 0xc6, 0x5b, 0xd5, 0xed, 0xb0, 0xb7, 0xc9, 0x11 };
	const byte lzPrefix[] = { 0x2b, 0x7e, 0x1b, 0x12, 0xd6 };
	const byte lzSuffix[] = {
		0x20, 0xf8, 0x2b, 0xb6, 0x28, 0x14, 0x2b, 0xe5, 0x6e, 0x4f, 0xe6, 0x07, 0x67, 0x23, 0x19, 0xa9,
		0x0f, 0x0f, 0x0f, 0xc6, 0x03, 0x4f, 0xed, 0xb8, 0x13, 0xe1, 0xed, 0x52, 0x19, 0x20, 0xdb, 0xc3
	};
	if (data.size() < 66 || data[0] != 0x21 || memcmp(data.data() + 3, loader, sizeof(loader)) ||
			data[17] != 0x01 || READ_BE_UINT32(data.data() + 20) != 0xedb0eb11 ||
			memcmp(data.data() + 26, lzPrefix, sizeof(lzPrefix)) || memcmp(data.data() + 32, lzSuffix, sizeof(lzSuffix)))
		return true;

	int base = READ_LE_UINT16(data.data() + 15);
	uint16 packedSize = READ_LE_UINT16(data.data() + 18);
	int end = READ_LE_UINT16(data.data() + 24);
	int entry = READ_LE_UINT16(data.data() + 64);
	byte marker = data[31];
	if (READ_LE_UINT16(data.data() + 1) != base + 14 || !packedSize || packedSize + 66U != data.size() ||
			base + data.size() > 0x10000 || base + packedSize >= end)
		return false;

	Common::Array<byte> memory;
	memory.resize(0x10000);
	memcpy(memory.data() + base, data.data() + 66, packedSize);
	int src = base + packedSize;
	int dst = end;

	// The first stage expands backward until its input and output pointers meet.
	do {
		if (src <= base || src >= dst)
			return false;
		byte value = memory[--src];
		memory[--dst] = value;
		if (value != marker)
			continue;
		if (src <= base)
			return false;
		byte code = memory[--src];
		if (code) {
			if (src <= base)
				return false;
			int offset = ((code & 7) << 8) | memory[--src];
			int count = (code >> 3) + 3;
			int copy = dst + offset + 1;
			if (copy >= end || dst - count + 1 < src)
				return false;
			while (count--)
				memory[dst--] = memory[copy--];
			++dst;
		}
	} while (src != dst);

	if (entry < base || entry + 4 > end)
		return false;
	bool screen = memory[entry] == 0x21 && READ_LE_UINT16(memory.data() + entry + 1) == base && memory[entry + 3] == 0xe5;
	if (screen)
		entry += 4;

	const byte rleLoader[] = { 0x01, 0x12, 0x00, 0xd5, 0xed, 0xb0, 0x21 };
	const byte rleCode[] = { 0x7e, 0x07, 0xcb, 0x3f, 0x2b, 0xed, 0xa8, 0xe0, 0x3d, 0xcb, 0xbf, 0x28, 0xf3, 0x30, 0xf6, 0x23, 0x18, 0xf3 };
	if (entry + 40 > end || memory[entry] != 0x21 || READ_LE_UINT16(memory.data() + entry + 1) != entry + 22 ||
			memory[entry + 3] != 0x11 || memcmp(memory.data() + entry + 6, rleLoader, sizeof(rleLoader)) ||
			memory[entry + 15] != 0x11 || memory[entry + 18] != 0x01 || memory[entry + 21] != 0xc9 ||
			memcmp(memory.data() + entry + 22, rleCode, sizeof(rleCode)))
		return false;

	src = READ_LE_UINT16(memory.data() + entry + 13);
	dst = READ_LE_UINT16(memory.data() + entry + 16);
	int remaining = READ_LE_UINT16(memory.data() + entry + 19);
	int start = dst + 1 - remaining;
	if (!remaining || start < base || src < start || src >= entry || src > dst)
		return false;
	end = dst + 1;

	// The second stage uses the high bit for repeated runs; a zero count means 128.
	while (remaining) {
		if (src < start || src > dst)
			return false;
		byte control = memory[src--];
		int count = MIN<int>((control & 0x7f) ? (control & 0x7f) : 128, remaining);
		remaining -= count;
		while (count--) {
			if (src < start || src > dst)
				return false;
			memory[dst--] = memory[src];
			if (!(control & 0x80))
				--src;
		}
		if (control & 0x80)
			--src;
	}

	if (screen) {
		// Discard the screen-copy routine preceding the SCR image.
		if (start != base + 12 || end - start != 6912 || memory[base] != 0x21 ||
				READ_LE_UINT16(memory.data() + base + 1) != start || READ_BE_UINT32(memory.data() + base + 3) != 0x11004001 ||
				READ_LE_UINT16(memory.data() + base + 7) != 6912 || memory[base + 9] != 0xed ||
				memory[base + 10] != 0xb0 || memory[base + 11] != 0xc9)
			return false;
		base = start;
	}
	data.assign(memory.begin() + base, memory.begin() + end);
	return true;
}

Common::SeekableReadStream *openZxSpectrumFile(const Common::Path &name) {
	Common::File file;
	if (!file.open(name) || file.size() <= 0 || file.size() > 0x10000)
		return nullptr;
	Common::Array<byte> data;
	data.resize(file.size());
	if (file.read(data.data(), data.size()) != data.size() || !unpackZxSpectrumBlock(data))
		return nullptr;
	Common::MemoryReadStream stream(data.data(), data.size());
	return stream.readStream(stream.size());
}

bool extractZxSpectrumTapeFiles(Common::SeekableReadStream &stream, const char *prefix, ZxTapeFileList &files) {
	files.clear();

	Common::SpectrumTapeBlocks blocks;
	if (!Common::parseSpectrumTape(stream, blocks))
		return false;

	Common::SpectrumTapeBlocks decoded;
	ZxRecordingDecoder recording(decoded);
	for (const Common::SpectrumTapeBlock &block : blocks) {
		if (block.id == 0x15) {
			if (!recording.decode(block.data))
				return false;
		} else {
			recording.finish();
			decoded.push_back(block);
		}
	}
	recording.finish();

	Common::Array<byte> title;
	Common::Array<byte> border;
	Common::Array<byte> data;
	Common::Array<byte> kitData;
	Common::Array<byte> code;

	for (const Common::SpectrumTapeBlock &block : decoded) {
		if (block.tap.size() >= 2) {
			Common::Array<byte> body;
			body.assign(block.tap.begin() + 1, block.tap.end() - 1);
			if (!unpackZxSpectrumBlock(body))
				return false;
			if (body.size() >= 160 && READ_BE_UINT32(body.data()) == MKTAG('K', 'I', 'T', 'S') &&
					READ_LE_UINT16(body.data() + 4) == body.size()) {
				kitData = body;
			} else if (body.size() == 6912) {
				title = border;
				border = body;
			} else if (body.size() >= data.size()) {
				data = body;
			}
		}
	}
	if (!kitData.empty()) {
		code = data;
		data = kitData;
	}

	const struct {
		const char *suffix;
		const Common::Array<byte> &payload;
	} outputs[] = {
		{ "title", title },
		{ "border", border },
		{ "data", data },
		{ "code", code }
	};
	for (uint i = 0; i < ARRAYSIZE(outputs); ++i) {
		if (!outputs[i].payload.empty()) {
			ZxTapeFile file;
			file.name = Common::Path(Common::String::format("%s.zx.%s", prefix, outputs[i].suffix), Common::Path::kNoSeparator);
			file.data = outputs[i].payload;
			files.push_back(file);
		}
	}

	return !data.empty();
}

bool matchZxSpectrumTapeFiles(const ZxTapeFileList &files, const ADGameDescription &desc, uint md5Bytes) {
	bool matched = desc.platform == Common::kPlatformZX;

	for (const ADGameFileDescription *fileDesc = desc.filesDescriptions; matched && fileDesc->fileName; ++fileDesc) {
		bool fileMatched = false;
		Common::Path fileName(fileDesc->fileName, Common::Path::kNoSeparator);
		Common::String unprefixedName(fileDesc->fileName);
		if (unprefixedName.hasPrefix(Common::String(desc.gameId) + ".zx."))
			unprefixedName.erase(0, strlen(desc.gameId));
		for (uint i = 0; !fileMatched && i < files.size(); ++i) {
			fileMatched = (files[i].name.equalsIgnoreCase(fileName) ||
					files[i].name.equalsIgnoreCase(Common::Path(unprefixedName, Common::Path::kNoSeparator))) &&
				(fileDesc->fileSize == AD_NO_SIZE || fileDesc->fileSize == files[i].data.size());
			if (fileMatched && fileDesc->md5) {
				Common::MemoryReadStream stream(files[i].data.data(), files[i].data.size());
				fileMatched = Common::computeStreamMD5AsString(stream, md5Bytes) == fileDesc->md5;
			}
		}
		matched = fileMatched;
	}

	return matched;
}

Common::Archive *makeZxSpectrumTapeArchive(const ADGameDescription &desc, const Common::Path &gamePath) {
	Common::Archive *archive = nullptr;

	Common::FSList files;
	if (desc.platform == Common::kPlatformZX && Common::FSNode(gamePath).getChildren(files, Common::FSNode::kListFilesOnly)) {
		for (const Common::FSNode &node : files) {
			Common::File file;
			Common::String name = node.getName();
			if ((name.hasSuffixIgnoreCase(".tap") || name.hasSuffixIgnoreCase(".tzx")) && file.open(node)) {
				ZxTapeFileList tapeFiles;
				if (extractZxSpectrumTapeFiles(file, desc.gameId, tapeFiles) && matchZxSpectrumTapeFiles(tapeFiles, desc)) {
					Common::SpectrumTapeArchive *tapeArchive = new Common::SpectrumTapeArchive();
					for (const ZxTapeFile &tapeFile : tapeFiles)
						tapeArchive->addFile(tapeFile.name, tapeFile.data);
					archive = tapeArchive;
					break;
				}
			}
		}
	}

	return archive;
}

} // End of namespace Freescape
