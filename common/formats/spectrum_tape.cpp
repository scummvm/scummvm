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

#include "common/formats/spectrum_tape.h"

#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/util.h"

namespace Common {

struct SpectrumTapeHeader {
	bool valid = false;
	byte type = 0;
	String name;
	uint16 size = 0;
	uint16 param1 = 0;
	uint16 param2 = 0;
};

enum TzxBlockId {
	kTzxBlockStandardSpeedData = 0x10,
	kTzxBlockTurboSpeedData = 0x11,
	kTzxBlockPureTone = 0x12,
	kTzxBlockPulseSequence = 0x13,
	kTzxBlockPureData = 0x14,
	kTzxBlockDirectRecording = 0x15,
	kTzxBlockC64RomData = 0x16,
	kTzxBlockC64TurboTapeData = 0x17,
	kTzxBlockCswRecording = 0x18,
	kTzxBlockGeneralizedData = 0x19,
	kTzxBlockPause = 0x20,
	kTzxBlockGroupStart = 0x21,
	kTzxBlockGroupEnd = 0x22,
	kTzxBlockJumpTo = 0x23,
	kTzxBlockLoopStart = 0x24,
	kTzxBlockLoopEnd = 0x25,
	kTzxBlockCallSequence = 0x26,
	kTzxBlockReturnFromSequence = 0x27,
	kTzxBlockSelect = 0x28,
	kTzxBlockStopTape48k = 0x2a,
	kTzxBlockSetSignalLevel = 0x2b,
	kTzxBlockTextDescription = 0x30,
	kTzxBlockMessage = 0x31,
	kTzxBlockArchiveInfo = 0x32,
	kTzxBlockHardwareType = 0x33,
	kTzxBlockEmulationInfo = 0x34,
	kTzxBlockCustomInfo = 0x35,
	kTzxBlockSnapshot = 0x40,
	kTzxBlockKansasCityStandard = 0x4b,
	kTzxBlockGlue = 0x5a
};

static bool readBytes(SeekableReadStream &stream, Array<byte> &out, uint32 len) {
	out.resize(len);
	return len == 0 || stream.read(out.data(), len) == len;
}

static bool readPayload(SeekableReadStream &stream, Array<byte> &out, uint32 len) {
	if (stream.pos() + len > stream.size())
		return false;

	return readBytes(stream, out, len);
}

static uint32 readUint24LE(const byte *data) {
	return READ_LE_UINT16(data) | (data[2] << 16);
}

static bool appendBlock(SpectrumTapeBlocks &blocks, byte id, const Array<byte> &data, uint32 tapOffset = 0xffffffff) {
	SpectrumTapeBlock block;
	block.id = id;
	block.data = data;

	if (tapOffset != 0xffffffff) {
		if (tapOffset > data.size())
			return false;
		block.tap.assign(data.begin() + tapOffset, data.end());
	}

	blocks.push_back(block);
	return true;
}

static bool getTapBody(const SpectrumTapeBlock &block, Array<byte> &body, byte *flag = nullptr) {
	if (block.tap.size() < 2)
		return false;

	if (flag)
		*flag = block.tap[0];

	body.assign(block.tap.begin() + 1, block.tap.end() - 1);
	return true;
}

static SpectrumTapeHeader parseTapHeader(const SpectrumTapeBlock &block) {
	SpectrumTapeHeader header;
	Array<byte> body;
	byte flag = 0xff;
	if (!getTapBody(block, body, &flag) || flag != 0x00 || body.size() != 17)
		return header;

	header.valid = true;
	header.type = body[0];
	header.name = String(reinterpret_cast<const char *>(body.data() + 1), 10);
	header.name.trim();
	header.size = READ_LE_UINT16(body.data() + 11);
	header.param1 = READ_LE_UINT16(body.data() + 13);
	header.param2 = READ_LE_UINT16(body.data() + 15);
	return header;
}

static String sanitizeTapeName(const String &name) {
	String result;
	for (uint i = 0; i < name.size(); ++i) {
		char c = name[i];
		if (isAlnum(c))
			result += c;
		else if (c == '_' || c == '-' || c == '.')
			result += c;
		else if (!result.empty() && result.lastChar() != '_')
			result += '_';
	}

	while (!result.empty() && (result.lastChar() == '_' || result.lastChar() == '.'))
		result.deleteLastChar();

	return result;
}

static String getTapeFileName(const SpectrumTapeHeader &header, const Array<byte> &body, uint index) {
	String base = sanitizeTapeName(header.name);
	if (base.empty())
		base = body.size() == 6912 ? "screen" : "data";

	const char *extension = "bin";
	if (body.size() == 6912)
		extension = "scr";
	else if (header.valid) {
		switch (header.type) {
		case 0:
			extension = "bas";
			break;
		case 1:
			extension = "num";
			break;
		case 2:
			extension = "chr";
			break;
		case 3:
			extension = "bin";
			break;
		default:
			extension = "dat";
			break;
		}
	}

	return String::format("%03u-%s.%s", index, base.c_str(), extension);
}

static bool parseTap(SeekableReadStream &stream, SpectrumTapeBlocks &blocks) {
	while (stream.pos() < stream.size()) {
		if (stream.pos() + 2 > stream.size())
			return false;

		uint16 len = stream.readUint16LE();
		if (stream.err() || stream.pos() + len > stream.size())
			return false;

		SpectrumTapeBlock block;
		block.id = 0x10;
		block.data.resize(4);
		WRITE_LE_UINT16(block.data.data(), 1000);
		WRITE_LE_UINT16(block.data.data() + 2, len);
		if (!readBytes(stream, block.tap, len))
			return false;

		blocks.push_back(block);
	}

	return true;
}

static bool readTzxDataBlock(SeekableReadStream &stream, SpectrumTapeBlocks &blocks, byte id, uint32 headerSize, uint32 lenOffset) {
	Array<byte> data;
	if (!readPayload(stream, data, headerSize))
		return false;

	uint32 len = readUint24LE(data.data() + lenOffset);
	if (stream.pos() + len > stream.size())
		return false;

	uint32 offset = data.size();
	data.resize(offset + len);
	if (len != 0 && stream.read(data.data() + offset, len) != len)
		return false;

	return appendBlock(blocks, id, data, headerSize);
}

static bool readSizedBlock(SeekableReadStream &stream, SpectrumTapeBlocks &blocks, byte id, uint32 size) {
	Array<byte> data;
	if (!readPayload(stream, data, size))
		return false;

	return appendBlock(blocks, id, data);
}

static bool readLengthPrefixedBlock(SeekableReadStream &stream, SpectrumTapeBlocks &blocks, byte id, uint32 lengthSize, uint32 extraSize = 0) {
	Array<byte> data;
	if (!readPayload(stream, data, lengthSize))
		return false;

	uint32 len;
	if (lengthSize == 1)
		len = data[0];
	else if (lengthSize == 2)
		len = READ_LE_UINT16(data.data());
	else
		len = READ_LE_UINT32(data.data());

	if (len < extraSize)
		return false;

	uint32 offset = data.size();
	data.resize(offset + len - extraSize);
	if (data.size() != offset && stream.read(data.data() + offset, data.size() - offset) != data.size() - offset)
		return false;

	return appendBlock(blocks, id, data);
}

static bool parseTzx(SeekableReadStream &stream, SpectrumTapeBlocks &blocks) {
	if (!stream.seek(10))
		return false;

	while (stream.pos() < stream.size()) {
		byte id = stream.readByte();
		if (stream.err())
			return false;

		switch (id) {
		case kTzxBlockStandardSpeedData: { // Standard Speed Data Block
			Array<byte> data;
			if (!readPayload(stream, data, 4))
				return false;
			uint16 len = READ_LE_UINT16(data.data() + 2);
			uint32 offset = data.size();
			data.resize(offset + len);
			if (len != 0 && stream.read(data.data() + offset, len) != len)
				return false;
			if (!appendBlock(blocks, id, data, 4))
				return false;
			break;
		}
		case kTzxBlockTurboSpeedData: // Turbo Speed Data Block
			if (!readTzxDataBlock(stream, blocks, id, 0x12, 0x0f))
				return false;
			break;
		case kTzxBlockPureTone: // Pure Tone
			if (!readSizedBlock(stream, blocks, id, 4))
				return false;
			break;
		case kTzxBlockPulseSequence: { // Pulse Sequence
			byte count = stream.readByte();
			if (stream.err())
				return false;
			Array<byte> data;
			data.resize(1 + count * 2);
			data[0] = count;
			if (count != 0 && (int)stream.read(data.data() + 1, count * 2) != count * 2)
				return false;
			if (!appendBlock(blocks, id, data))
				return false;
			break;
		}
		case kTzxBlockPureData: // Pure Data Block
			if (!readTzxDataBlock(stream, blocks, id, 0x0a, 0x07))
				return false;
			break;
		case kTzxBlockDirectRecording: { // Direct Recording
			Array<byte> data;
			if (!readPayload(stream, data, 8))
				return false;
			uint32 len = readUint24LE(data.data() + 5);
			uint32 offset = data.size();
			data.resize(offset + len);
			if (len != 0 && stream.read(data.data() + offset, len) != len)
				return false;
			if (!appendBlock(blocks, id, data))
				return false;
			break;
		}
		case kTzxBlockC64RomData: // C64 ROM type data
		case kTzxBlockC64TurboTapeData: // C64 turbo tape data
			if (!readLengthPrefixedBlock(stream, blocks, id, 4, 4))
				return false;
			break;
		case kTzxBlockCswRecording: // CSW recording
		case kTzxBlockGeneralizedData: // Generalized data
			if (!readLengthPrefixedBlock(stream, blocks, id, 4))
				return false;
			break;
		case kTzxBlockPause: // Pause
		case kTzxBlockJumpTo: // Jump to
		case kTzxBlockLoopStart: // Loop start
			if (!readSizedBlock(stream, blocks, id, 2))
				return false;
			break;
		case kTzxBlockGroupStart: // Group start
		case kTzxBlockTextDescription: // Text description
			if (!readLengthPrefixedBlock(stream, blocks, id, 1))
				return false;
			break;
		case kTzxBlockGroupEnd: // Group end
		case kTzxBlockLoopEnd: // Loop end
		case kTzxBlockReturnFromSequence: // Return from sequence
			if (!appendBlock(blocks, id, Array<byte>()))
				return false;
			break;
		case kTzxBlockCallSequence: { // Call sequence
			uint16 count = stream.readUint16LE();
			if (stream.err())
				return false;
			Array<byte> data;
			data.resize(2 + count * 2);
			WRITE_LE_UINT16(data.data(), count);
			if (count != 0 && (int)stream.read(data.data() + 2, count * 2) != count * 2)
				return false;
			if (!appendBlock(blocks, id, data))
				return false;
			break;
		}
		case kTzxBlockSelect: // Select
		case kTzxBlockArchiveInfo: // Archive info
			if (!readLengthPrefixedBlock(stream, blocks, id, 2))
				return false;
			break;
		case kTzxBlockStopTape48k: // Stop the tape (48k)
			if (!readSizedBlock(stream, blocks, id, 4))
				return false;
			break;
		case kTzxBlockSetSignalLevel: // Set signal level
			if (!readSizedBlock(stream, blocks, id, 5))
				return false;
			break;
		case kTzxBlockMessage: { // Message
			Array<byte> data;
			if (!readPayload(stream, data, 2))
				return false;
			uint32 offset = data.size();
			data.resize(offset + data[1]);
			if (data[1] != 0 && stream.read(data.data() + offset, data[1]) != data[1])
				return false;
			if (!appendBlock(blocks, id, data))
				return false;
			break;
		}
		case kTzxBlockHardwareType: { // Hardware type
			byte count = stream.readByte();
			if (stream.err())
				return false;
			Array<byte> data;
			data.resize(1 + count * 3);
			data[0] = count;
			if (count != 0 && (int)stream.read(data.data() + 1, count * 3) != count * 3)
				return false;
			if (!appendBlock(blocks, id, data))
				return false;
			break;
		}
		case kTzxBlockEmulationInfo: // Emulation info
			if (!readSizedBlock(stream, blocks, id, 8))
				return false;
			break;
		case kTzxBlockCustomInfo: { // Custom info
			Array<byte> data;
			if (!readPayload(stream, data, 0x14))
				return false;
			uint32 len = READ_LE_UINT32(data.data() + 0x10);
			uint32 offset = data.size();
			data.resize(offset + len);
			if (len != 0 && stream.read(data.data() + offset, len) != len)
				return false;
			if (!appendBlock(blocks, id, data))
				return false;
			break;
		}
		case kTzxBlockSnapshot: { // Snapshot
			Array<byte> data;
			if (!readPayload(stream, data, 4))
				return false;
			uint32 len = readUint24LE(data.data() + 1);
			uint32 offset = data.size();
			data.resize(offset + len);
			if (len != 0 && stream.read(data.data() + offset, len) != len)
				return false;
			if (!appendBlock(blocks, id, data))
				return false;
			break;
		}
		case kTzxBlockKansasCityStandard: // Kansas City Standard
			if (!readLengthPrefixedBlock(stream, blocks, id, 4))
				return false;
			break;
		case kTzxBlockGlue: // Glue
			if (!readSizedBlock(stream, blocks, id, 9))
				return false;
			break;
		default:
			return false;
		}
	}

	return true;
}

bool parseSpectrumTape(SeekableReadStream &stream, SpectrumTapeBlocks &blocks) {
	blocks.clear();

	if (!stream.seek(0) || stream.size() < 2)
		return false;

	byte signature[8] = {};
	uint32 signatureSize = MIN<uint32>(sizeof(signature), stream.size());
	if (stream.read(signature, signatureSize) != signatureSize)
		return false;

	if (!stream.seek(0))
		return false;

	if (signatureSize == sizeof(signature) && !memcmp(signature, "ZXTape!\x1a", 8))
		return parseTzx(stream, blocks);

	return parseTap(stream, blocks);
}

SpectrumTapeArchive::SpectrumTapeArchive(const SpectrumTapeBlocks &blocks) {
	SpectrumTapeHeader pendingHeader;
	uint fileIndex = 1;

	for (const SpectrumTapeBlock &block : blocks) {
		SpectrumTapeHeader header = parseTapHeader(block);
		if (header.valid) {
			pendingHeader = header;
			continue;
		}

		Array<byte> body;
		byte flag = 0x00;
		if (!getTapBody(block, body, &flag) || flag == 0x00 || body.empty())
			continue;

		addFile(Path(getTapeFileName(pendingHeader, body, fileIndex++), Path::kNoSeparator), body);

		pendingHeader.valid = false;
	}
}

void SpectrumTapeArchive::addFile(const Path &path, const Array<byte> &data) {
	ArchiveFile entry;
	entry.name = path;
	entry.data = data;
	_files.push_back(entry);
}

bool SpectrumTapeArchive::hasFile(const Path &path) const {
	for (const ArchiveFile &file : _files) {
		if (file.name.equalsIgnoreCase(path))
			return true;
	}

	return false;
}

int SpectrumTapeArchive::listMembers(ArchiveMemberList &list) const {
	for (const ArchiveFile &file : _files)
		list.push_back(ArchiveMemberList::value_type(new GenericArchiveMember(file.name, *this)));

	return _files.size();
}

const ArchiveMemberPtr SpectrumTapeArchive::getMember(const Path &path) const {
	if (!hasFile(path))
		return ArchiveMemberPtr();

	return ArchiveMemberPtr(new GenericArchiveMember(path, *this));
}

SeekableReadStream *SpectrumTapeArchive::createReadStreamForMember(const Path &path) const {
	for (const ArchiveFile &file : _files) {
		if (file.name.equalsIgnoreCase(path))
			return new MemoryReadStream(file.data.data(), file.data.size(), DisposeAfterUse::NO);
	}

	return nullptr;
}

} // End of namespace Common
