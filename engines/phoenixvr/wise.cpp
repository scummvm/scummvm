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

#include "phoenixvr/wise.h"

#include "common/array.h"
#include "common/compression/deflate.h"
#include "common/crc.h"
#include "common/debug.h"
#include "common/hashmap.h"
#include "common/list.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/substream.h"

namespace PhoenixVR {
namespace {
struct NE {
	struct Segment {
		uint32 offset;
		uint32 size;
		uint16 flags;
		bool automatic() const {
			return flags & 1;
		}
	};

	Common::Array<Segment> segments;
	uint16 autoSegment;

	NE(Common::SeekableReadStream &stream) {
		auto magic = stream.readUint16LE();
		if (magic != 0x5a4d)
			error("Not an MZ executable");
		stream.seek(0x3c);
		auto neOffset = stream.readUint32LE();
		stream.seek(neOffset);
		magic = stream.readUint16LE();
		if (magic != 0x454e)
			error("Not an NE executable");
		stream.seek(neOffset + 0x000e);
		autoSegment = stream.readUint16LE();
		debug("auto segment: %u", autoSegment);
		stream.seek(neOffset + 0x001c);
		auto numSegments = stream.readUint16LE();
		stream.seek(neOffset + 0x0022);
		auto segmentTableOffset = stream.readUint16LE();
		stream.seek(neOffset + 0x0032);
		auto segmentAddrShift = stream.readUint16LE();
		stream.seek(neOffset + segmentTableOffset);
		segments.reserve(numSegments);
		for (unsigned idx = 0; idx != numSegments; ++idx) {
			uint32 segmentOffset = stream.readUint16LE();
			segmentOffset <<= segmentAddrShift;
			uint32 segmentSize = stream.readUint16LE();
			if (segmentSize == 0)
				segmentSize = 0x10000;
			auto segmentFlags = stream.readUint16LE();
			auto segmentAlloc = stream.readUint16LE();
			if (idx + 1 == autoSegment && (segmentFlags & 1) == 0)
				error("automatic segment must have flag 1");
			debug("NE: segment %u: offset: %04x, size: %u, flags: %04x, alloc: %u", idx, segmentOffset, segmentSize, segmentFlags, segmentAlloc);
			segments.push_back(Segment{segmentOffset, segmentSize, segmentFlags});
		}
	}
};

class WiseArchive : public Common::Archive {
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	int64 baseStreamsOffset;

	struct Entry {
		uint32 begin;
		uint32 end;

		uint32 size() const {
			return end - begin;
		}
	};

	Common::HashMap<Common::Path, Entry, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> _entries;

	static Common::Array<byte> inflate(Common::SeekableReadStream &stream, uint size) {
		Common::MemoryWriteStreamDynamic dst(DisposeAfterUse::YES);
		if (size <= 4)
			error("compressed stream can't be shorted than 4 bytes");
		debug("inflating %u bytes of compressed data", size);
		auto startedAt = stream.pos();
		Common::SeekableSubReadStream src(&stream, stream.pos(), stream.pos() + size - 4);
		if (!Common::inflateZlibHeaderless(dst, src))
			error("inflateZlibHeaderless failed");
		stream.seek(startedAt + size - 4);
		auto crc = stream.readUint32LE();
		Common::Array<byte> uncompressed{dst.getData(), static_cast<uint>(dst.size())};
		auto crcU = Common::CRC32{}.crcFast(uncompressed.data(), uncompressed.size());
		if (crc != crcU)
			error("CRC mismatch");
		return uncompressed;
	}

	struct Script {
		Common::Array<byte> script;
		Common::Array<byte> numBytes;
		Common::Array<byte> numStrings;
		Common::Array<byte> numLocalized;

		Script() : numBytes(32), numStrings(32), numLocalized(32) {}

		struct FileEntry {
			Common::Path path;
			uint32 begin;
			uint32 end;
		};

		Common::List<FileEntry> parse() {
			Common::MemoryReadStream ms(script.data(), script.size());
			ms.seek(17);
			for (uint i = 0; i != 4; ++i) {
				ms.readString();
			}
			ms.skip(6);
			const auto numLanguages = ms.readByte();
			if (numLanguages == 0 || numLanguages >= 128)
				error("invalid number of languages (%u)", numLanguages);
			auto numLangStrings = 6 + (numLanguages > 1 ? 2 + 2 * numLanguages : 1) + 44 * numLanguages;
			while (numLangStrings--)
				auto str = ms.readString();

			Common::Array<byte> byteArguments;
			Common::Array<Common::String> strArguments;
			Common::List<FileEntry> entries;
			while (!ms.eos()) {
				auto opcode = ms.readByte();
				if (opcode >= numBytes.size() || numBytes[opcode] == 0)
					error("invalid opcode");
				byteArguments.resize(numBytes[opcode] - 1);
				ms.read(byteArguments.data(), byteArguments.size());
				strArguments.resize(numStrings[opcode] + numLanguages * numLocalized[opcode]);
				for (uint i = 0; i != strArguments.size(); ++i) {
					strArguments[i] = ms.readString();
				}
				if (opcode == 6)
					ms.skip(12 * (numLanguages - 1));
				if (opcode != 0)
					continue;

				assert(byteArguments.size() >= 11);

				// opcode 0: copy from packed source.
				auto &dst = strArguments[0];
				static Common::String prefix = "%MAINDIR%\\";
				if (!dst.hasPrefixIgnoreCase(prefix))
					continue;

				Common::Path path(dst.substr(prefix.size(), '\\'));
				auto begin = READ_LE_UINT32(byteArguments.data() + 2);
				auto end = READ_LE_UINT32(byteArguments.data() + 6);
				debug("adding entry %s %08x %08x", path.toString().c_str(), begin, end);
				entries.push_back(FileEntry{Common::move(path), begin, end});
			}
			return entries;
		}
	};

	template<typename Callback>
	static long patternFind(const Common::Array<byte> &data, byte initial, uint patternSize, const Callback &func) {
		if (data.size() < patternSize)
			return -1;
		auto *start = data.data();
		while ((data.size() - (start - data.data())) >= patternSize) {
			auto tailSize = (data.size() - (start - data.data()));
			auto *next = static_cast<const byte *>(memchr(start, initial, tailSize));
			if (!next)
				break;
			if (func(next))
				return next - data.data();
			start = next + 1;
		}
		return -1;
	}

public:
	WiseArchive(Common::SeekableReadStream *s) : stream(s) {
		NE ne(*s);
		long overlayOffset = -1;
		for (auto &seg : ne.segments) {
			if (seg.automatic())
				continue;

			stream->seek(seg.offset);
			Common::Array<byte> data(seg.size);
			if (stream->read(data.data(), data.size()) != data.size())
				error("NE segment: short read");

			if (overlayOffset < 0) {
				overlayOffset = patternFind(data, 0xa1, 30, [&](const byte *src) -> bool {
					return src[0] == 0xa1 &&
						   src[3] == 0x8b &&
						   src[4] == 0x16 &&
						   src[7] == 0xa3 &&
						   src[10] == 0x89 &&
						   src[11] == 0x16 &&
						   src[14] == 0xa3 &&
						   src[17] == 0x89 &&
						   src[18] == 0x16 &&
						   src[21] == 0xff &&
						   src[22] == 0x36 &&
						   src[25] == 0x52 &&
						   src[26] == 0x50 &&
						   src[27] == 0x6a &&
						   src[28] == 0x00 &&
						   src[29] == 0x9a;
				});
				if (overlayOffset >= 0) {
					auto low = READ_LE_UINT16(data.data() + overlayOffset + 1);
					auto high = READ_LE_UINT16(data.data() + overlayOffset + 5);
					if (low + 2 != high)
						error("non-contiguous overlay pointer");
					auto &autoSegment = ne.segments[ne.autoSegment - 1];
					stream->seek(autoSegment.offset + low);
					overlayOffset = stream->readUint32LE();
					debug("found overlay at %08lx", overlayOffset);
					break;
				}
			}
		}
		if (overlayOffset < 0)
			error("can't find overlay offset");

		stream->seek(overlayOffset);
		Common::Array<uint> header(16);
		for (size_t i = 0; i != header.size(); ++i)
			header[i] = stream->readUint32LE();

		auto &flags = header[0];
		auto &advertisedEOF = header[15];
		debug("flags: %08x, eof: %08x", flags, advertisedEOF);
		if (advertisedEOF != stream->size())
			error("invalid overlay header - EOF does not match stream size");

		debug("script %u/%u", header[7], header[6]);
		Script script;
		script.script = inflate(*stream, header[7]);
		if (script.script.size() != header[6])
			error("script uncompressed size does not match header");

		debug("packed dll at %08x, size: %u", (uint32)stream->pos(), header[8]);
		Common::Array<byte> dllData;
		dllData = inflate(*stream, header[8]);

		Common::MemoryReadStream dllStream(dllData.data(), dllData.size());
		NE dll(dllStream);
		for (auto &seg : dll.segments) {
			if (seg.automatic())
				continue;

			dllStream.seek(seg.offset);
			Common::Array<byte> data(seg.size);
			if (dllStream.read(data.data(), data.size()) != data.size())
				error("NE segment: short read");

			auto instructionTables = patternFind(data, 0x8a, 26, [&](const byte *src) -> bool {
				return src[0] == 0x8a &&
					   src[1] == 0x87 &&
					   src[4] == 0x8b &&
					   src[5] == 0xc8 &&
					   src[6] == 0x8a &&
					   src[7] == 0x87 &&
					   src[10] == 0x2a &&
					   src[11] == 0xe4 &&
					   src[12] == 0xf7 &&
					   src[13] == 0x2e &&
					   src[16] == 0x8b &&
					   src[17] == 0xf0 &&
					   src[18] == 0x2a &&
					   src[19] == 0xed &&
					   src[20] == 0x03 &&
					   src[21] == 0xf1 &&
					   src[22] == 0x8a &&
					   src[23] == 0x87;
			});
			if (instructionTables >= 0) {
				debug("instruction tables at %08lx", instructionTables);
				auto stringsOffset = READ_LE_UINT16(data.data() + instructionTables + 2);
				auto localizedStringsOffset = READ_LE_UINT16(data.data() + instructionTables + 8);
				auto numBytesOffset = READ_LE_UINT16(data.data() + instructionTables + 24);
				debug("tables at: %06x %06x %06x", stringsOffset, localizedStringsOffset, numBytesOffset);
				assert(stringsOffset + script.numStrings.size() <= data.size());
				assert(localizedStringsOffset + script.numLocalized.size() <= data.size());
				assert(numBytesOffset + script.numBytes.size() <= data.size());
				auto &autoSegment = dll.segments[dll.autoSegment - 1];
				dllStream.seek(autoSegment.offset + stringsOffset);
				dllStream.read(script.numStrings.data(), script.numStrings.size());
				dllStream.seek(autoSegment.offset + localizedStringsOffset);
				dllStream.read(script.numLocalized.data(), script.numLocalized.size());
				dllStream.seek(autoSegment.offset + numBytesOffset);
				dllStream.read(script.numBytes.data(), script.numBytes.size());
				break;
			}
		}

		auto skipStream = [&](uint size) {
			debug("skipping compressed stream %u bytes", size);
			Common::SeekableSubReadStream ss(stream.get(), stream->pos(), stream->pos() + size);
			auto skip = inflate(ss, size);
			debug("skipped %u bytes", skip.size());
		};
		if (flags & 0x10)
			skipStream(header[9]);

		if (flags & 0x8000)
			skipStream(header[10]);

		if (flags & 0x20000)
			skipStream(header[11]);

		if (header[12])
			skipStream(header[12]);

		if (header[13])
			skipStream(header[13]);

		baseStreamsOffset = stream->pos();
		debug("base stream offset: %08x", (uint32)baseStreamsOffset);
		for (auto entry : script.parse()) {
			_entries.setVal(Common::move(entry.path), {entry.begin, entry.end});
		}
	}

	char getPathSeparator() const override {
		return '\\';
	}

	bool hasFile(const Common::Path &path) const override {
		return _entries.find(path) != _entries.end();
	}

	int listMembers(Common::ArchiveMemberList &list) const override {
		int added = 0;
		for (auto &kv : _entries) {
			list.push_back(getMember(kv._key));
			++added;
		}
		return added;
	}

	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override {
		return Common::ArchiveMemberPtr{new Common::GenericArchiveMember(path, *this)};
	}

	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override {
		auto it = _entries.find(path);
		if (it == _entries.end())
			return nullptr;

		auto &entry = it->_value;
		stream->seek(baseStreamsOffset + entry.begin);
		auto dst = inflate(*stream, entry.size());
		auto *ptr = static_cast<byte *>(malloc(dst.size()));
		if (!ptr)
			error("createReadStreamForMember: malloc failed");
		memcpy(ptr, dst.data(), dst.size());
		return new Common::MemoryReadStream(ptr, dst.size(), DisposeAfterUse::YES);
	}
};
} // namespace

Common::Archive *createWISEArchive(Common::SeekableReadStream *stream) {
	return new WiseArchive(stream);
}
} // namespace PhoenixVR
