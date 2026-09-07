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
