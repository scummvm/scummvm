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

#include "eem/installer.h"

#include "common/algorithm.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/fs.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "eem/detection.h"

namespace EEM {

// ===========================================================================
//  Compact Pro decompressor
//
//  The installer compresses each fork with Compact Pro's scheme: an outer RLE
//  escape layer (ESC1 = 0x81, ESC2 = 0x82) wrapped around an LZH stage built
//  from three Huffman tables (256-entry literals, 64-entry match lengths,
//  128-entry match offsets) feeding an 8 KiB sliding window. Ported from the
//  reference cpt.c (macutils) decoder.
// ===========================================================================

namespace {

class CompactProDecompressor {
public:
	CompactProDecompressor(const byte *src, uint32 srcLen, byte *dst, uint32 dstLen)
		: _src(src), _srcLen(srcLen), _pos(0), _dst(dst), _dstLen(dstLen), _outPos(0),
		  _remaining((int32)dstLen), _outstat(kNone), _save(0), _lzptr(0),
		  _newbits(0), _bitsavail(0), _error(false) {
		memset(_lz, 0, sizeof(_lz));
	}

	// `compressed` selects the LZH stage; otherwise the bytes are only RLE
	// escaped. `inLen` is the size of the compressed span (the RLE stage reads
	// exactly that many input bytes).
	bool unpack(bool compressed, uint32 inLen) {
		if (compressed)
			unpackLZH();
		else
			unpackRLE(inLen);
		return !_error && _remaining == 0;
	}

private:
	enum { ESC1 = 0x81, ESC2 = 0x82 };
	enum { kNone = 0, kEsc1Seen = 1, kEsc2Seen = 2 };
	enum { kBlockSize = 0x1fff0 };

	struct HuffNode {
		byte leaf;
		int value;
		int left;
		int right;
	};

	struct HuffEnt {
		int value;
		int length;
	};

	struct HuffEntLess {
		bool operator()(const HuffEnt &a, const HuffEnt &b) const {
			return a.length != b.length ? a.length < b.length : a.value < b.value;
		}
	};

	byte getByte() {
		if (_pos >= _srcLen) {
			_error = true;
			return 0;
		}
		return _src[_pos++];
	}

	void emit(byte b) {
		if (_outPos < _dstLen)
			_dst[_outPos++] = b;
		_remaining--;
	}

	void outCh(int chIn) {
		byte ch = chIn & 0xff;
		_lz[_lzptr & 8191] = ch;
		_lzptr++;

		if (_outstat == kNone) {
			if (ch == ESC1 && _remaining != 1) {
				_outstat = kEsc1Seen;
			} else {
				_save = ch;
				emit(ch);
			}
		} else if (_outstat == kEsc1Seen) {
			if (ch == ESC2) {
				_outstat = kEsc2Seen;
			} else {
				_save = ESC1;
				emit(ESC1);
				if (_remaining == 0)
					return;
				if (ch == ESC1 && _remaining != 1)
					return; // remains in kEsc1Seen
				_outstat = kNone;
				_save = ch;
				emit(ch);
			}
		} else { // kEsc2Seen: ch is a run length for the previous byte
			_outstat = kNone;
			if (ch != 0) {
				int c = (ch - 1) & 0xff;
				while (c != 0) {
					emit(_save);
					if (_remaining == 0)
						return;
					c = (c - 1) & 0xff;
				}
			} else {
				emit(ESC1);
				if (_remaining == 0)
					return;
				_save = ESC2;
				emit(_save);
			}
		}
	}

	void unpackRLE(uint32 inLen) {
		for (uint32 i = 0; i < inLen; i++) {
			outCh(getByte());
			if (_error || _remaining == 0)
				break;
		}
	}

	bool readHuff(Common::Array<HuffNode> &nodes, int size) {
		int treeBytes = getByte();
		if (size < treeBytes * 2) {
			_error = true;
			return false;
		}

		int treeCount[32];
		memset(treeCount, 0, sizeof(treeCount));
		Common::Array<HuffEnt> entries;
		int i = 0, maxlen = 0;
		for (int t = 0; t < treeBytes; t++) {
			int b = getByte();
			int lens[2] = { b >> 4, b & 0xf };
			for (int k = 0; k < 2; k++) {
				int length = lens[k];
				if (length) {
					if (length > maxlen)
						maxlen = length;
					treeCount[length]++;
					HuffEnt e;
					e.value = i;
					e.length = length;
					entries.push_back(e);
				}
				i++;
			}
		}

		int j = 0;
		for (int x = 0; x <= maxlen; x++)
			j = (j << 1) + treeCount[x];
		int unused = maxlen ? ((1 << maxlen) - j) : 0;
		for (int u = 0; u < unused; u++) {
			HuffEnt e;
			e.value = size;
			e.length = maxlen;
			entries.push_back(e);
		}

		Common::sort(entries.begin(), entries.end(), HuffEntLess());

		const int total = size * 2 + 32;
		nodes.clear();
		nodes.resize(total);
		for (int n = 0; n < total; n++) {
			nodes[n].leaf = 0;
			nodes[n].value = 0;
			nodes[n].left = 0;
			nodes[n].right = 0;
		}

		int idx = (int)entries.size() - 1;
		int lvlstart = total - 1, nxt = total - 1;
		for (int codelen = maxlen; codelen >= 1; codelen--) {
			while (idx >= 0 && entries[idx].length == codelen) {
				nodes[nxt].leaf = 1;
				nodes[nxt].value = entries[idx].value;
				nxt--;
				idx--;
			}
			int parents = nxt;
			if (codelen > 1) {
				int jj = lvlstart;
				while (jj > parents + 1) {
					nodes[nxt].leaf = 0;
					nodes[nxt].left = jj - 1;
					nodes[nxt].right = jj;
					nxt--;
					jj -= 2;
				}
			}
			lvlstart = parents;
		}
		nodes[0].leaf = 0;
		nodes[0].left = nxt + 1;
		nodes[0].right = nxt + 2;
		return true;
	}

	int getBit() {
		int b = (_newbits >> 31) & 1;
		_bitsavail--;
		if (_bitsavail < 16) {
			uint32 hi = getByte();
			uint32 lo = getByte();
			_newbits |= (hi << 8) | lo;
			_bitsavail += 16;
		}
		_newbits = _newbits << 1;
		return b;
	}

	int get6Bits() {
		int b = (_newbits >> 26) & 0x3f;
		_bitsavail -= 6;
		_newbits = _newbits << 6;
		if (_bitsavail < 16) {
			uint32 hi = getByte();
			uint32 lo = getByte();
			uint32 cn = (hi << 8) | lo;
			_newbits |= cn << (16 - _bitsavail);
			_bitsavail += 16;
		}
		return b;
	}

	int getHuffByte(const Common::Array<HuffNode> &nodes) {
		int idx = 0;
		while (!nodes[idx].leaf)
			idx = getBit() ? nodes[idx].right : nodes[idx].left;
		return nodes[idx].value;
	}

	void unpackLZH() {
		_lz[8189] = _lz[8190] = _lz[8191] = 0;
		_lzptr = 0;
		while (_remaining != 0 && !_error) {
			Common::Array<HuffNode> huffData, huffLen, huffOff;
			if (!readHuff(huffData, 256) || !readHuff(huffLen, 64) || !readHuff(huffOff, 128))
				return;

			int blockCount = 0;
			uint32 hi = getByte();
			uint32 lo = getByte();
			_newbits = ((hi << 8) | lo) << 16;
			_bitsavail = 16;

			while (blockCount < (int)kBlockSize && _remaining != 0 && !_error) {
				if (getBit()) {
					outCh(getHuffByte(huffData));
					blockCount += 2;
				} else {
					int length = getHuffByte(huffLen);
					int off = (getHuffByte(huffOff) << 6) | get6Bits();
					uint32 bptr = _lzptr - off;
					while (length > 0) {
						outCh(_lz[bptr & 8191]);
						bptr++;
						length--;
					}
					blockCount += 3;
				}
			}
		}
	}

	const byte *_src;
	uint32 _srcLen;
	uint32 _pos;
	byte *_dst;
	uint32 _dstLen;
	uint32 _outPos;
	int32 _remaining;
	int _outstat;
	int _save;
	uint32 _lzptr;
	byte _lz[8192];
	uint32 _newbits;
	int _bitsavail;
	bool _error;
};

} // End of anonymous namespace

class InstallerArchive : public Common::Archive {
private:
	struct Entry {
		Common::String name;
		uint32 pos;       // logical offset of the resource span within _archive
		uint32 rsrcLen;
		uint32 dataLen;
		uint32 compRLen;
		uint32 compDLen;
		uint16 cptFlag;
	};

	class Member : public Common::ArchiveMember {
	public:
		Member(const InstallerArchive *archive, const Entry *entry)
			: _archive(archive), _entry(entry) {}

		Common::SeekableReadStream *createReadStream() const override {
			return _archive->createStream(*_entry, false);
		}
		Common::SeekableReadStream *createReadStreamForAltStream(Common::AltStreamType altStreamType) const override {
			if (altStreamType == Common::AltStreamType::MacResourceFork)
				return _archive->createStream(*_entry, true);
			return nullptr;
		}
		Common::String getName() const override { return _entry->name; }
		Common::Path getPathInArchive() const override { return Common::Path(_entry->name); }
		Common::String getFileName() const override { return _entry->name; }
		bool isInMacArchive() const override { return true; }

	private:
		const InstallerArchive *_archive;
		const Entry *_entry;
	};

public:
	bool load(const Common::FSNode &dir);

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMemberAltStream(const Common::Path &path,
			Common::AltStreamType altStreamType) const override;
	char getPathSeparator() const override { return ':'; }

private:
	const Entry *findEntry(const Common::Path &path) const;
	Common::SeekableReadStream *createStream(const Entry &entry, bool resFork) const;
	void parseCatalog();

	Common::Array<byte> _archive;
	Common::Array<Entry> _entries;
	Common::Array<uint32> _diskBase; // [1..6] start offset of each disk within _archive
};

bool InstallerArchive::load(const Common::FSNode &dir) {
	// disk 1 first, then the five continuation segments, concatenated into one
	// logical stream (payload spans cross floppy boundaries).
	Common::Array<Common::String> names;
	names.push_back("Eagle Eye Installer");
	for (int i = 2; i <= 6; i++)
		names.push_back(Common::String::format("EEM Install Data %d", i));

	_diskBase.resize(names.size() + 1); // 1-based; index 0 unused
	for (uint i = 0; i < _diskBase.size(); i++)
		_diskBase[i] = 0xffffffff;

	for (uint i = 0; i < names.size(); i++) {
		Common::FSNode node = dir.getChild(names[i]);
		if (!node.exists()) {
			if (i == 0)
				return false; // installer not present here
			// Missing continuation segments are tolerated (e.g. CD layout);
			// only entries that reference them will fail to load.
			break;
		}
		Common::ScopedPtr<Common::SeekableReadStream> file(node.createReadStream());
		if (!file) {
			warning("EEM installer: cannot read %s", names[i].c_str());
			return i != 0;
		}

		// "Eagle Eye Installer" may be a raw data fork (CD/floppy) or
		// MacBinary-wrapped; read just its data fork so both yield the same
		// stream. The continuation segments are always raw.
		Common::ScopedPtr<Common::SeekableReadStream> dataFork;
		if (i == 0)
			dataFork.reset(Common::MacResManager::openDataForkFromMacBinary(file.get()));
		Common::SeekableReadStream *src = dataFork ? dataFork.get() : file.get();
		src->seek(0);

		const uint32 oldSize = _archive.size();
		const uint32 addSize = (uint32)src->size();
		_diskBase[i + 1] = oldSize; // disk numbers are 1-based
		_archive.resize(oldSize + addSize);
		if (src->read(_archive.data() + oldSize, addSize) != addSize) {
			warning("EEM installer: short read on %s", names[i].c_str());
			return false;
		}
	}

	parseCatalog();
	if (_entries.empty()) {
		warning("EEM installer: no catalog entries found");
		return false;
	}
	debugC(1, kDebugGeneral, "EEM installer: %u entries, %u bytes",
		   _entries.size(), _archive.size());
	return true;
}

void InstallerArchive::parseCatalog() {
	// The catalog occupies the start of disk 1. Each entry is a Pascal name,
	// followed by a disk byte + big-endian 32-bit offset, an 8-byte marker
	// ("DATASFS1"/"APPLSFS1") and a 32-byte field block.
	const byte *cat = _archive.data();
	const uint32 catSize = _archive.size();
	const uint32 scanEnd = MIN<uint32>(catSize, 0x500);
	const char *markers[2] = { "DATASFS1", "APPLSFS1" };

	for (int mi = 0; mi < 2; mi++) {
		const char *mk = markers[mi];
		for (uint32 pos = 5; pos + 8 + 32 <= catSize && pos + 8 <= scanEnd; pos++) {
			if (memcmp(cat + pos, mk, 8) != 0)
				continue;

			const int disk = cat[pos - 5];
			const uint32 off = READ_BE_UINT32(cat + pos - 4);

			// Recover the file name: a Pascal string ending right before the
			// 5-byte disk/offset prefix.
			const uint32 nameEnd = pos - 5;
			Common::String name;
			const uint32 lo = nameEnd > 80 ? nameEnd - 80 : 0;
			for (uint32 lpos = lo; lpos < nameEnd; lpos++) {
				const uint32 len = cat[lpos];
				if (lpos + 1 + len != nameEnd)
					continue;
				bool printable = true;
				for (uint32 c = lpos + 1; c < nameEnd; c++) {
					if (cat[c] < 32 || cat[c] >= 127) {
						printable = false;
						break;
					}
				}
				if (printable)
					name = Common::String((const char *)cat + lpos + 1, len);
			}
			if (name.empty())
				continue;

			const byte *f = cat + pos + 8;
			Entry e;
			e.name = name;
			e.cptFlag = READ_BE_UINT16(f + 14);
			e.rsrcLen = READ_BE_UINT32(f + 16);
			e.dataLen = READ_BE_UINT32(f + 20);
			e.compRLen = READ_BE_UINT32(f + 24);
			e.compDLen = READ_BE_UINT32(f + 28);

			// Map the per-disk relative offset to the concatenated stream. The
			// segments were appended in order, so the global position is the
			// loaded base of `disk` plus `off`; spans flowing into later disks
			// stay contiguous. Skip entries on segments that were not loaded.
			if (disk < 1 || (uint)disk >= _diskBase.size() ||
				_diskBase[disk] == 0xffffffff)
				continue;
			e.pos = _diskBase[disk] + off;
			_entries.push_back(e);
		}
	}
}

bool InstallerArchive::hasFile(const Common::Path &path) const {
	return findEntry(path) != nullptr;
}

int InstallerArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (uint i = 0; i < _entries.size(); i++)
		list.push_back(Common::ArchiveMemberPtr(new Member(this, &_entries[i])));
	return _entries.size();
}

const Common::ArchiveMemberPtr InstallerArchive::getMember(const Common::Path &path) const {
	const Entry *e = findEntry(path);
	if (!e)
		return nullptr;
	return Common::ArchiveMemberPtr(new Member(this, e));
}

Common::SeekableReadStream *InstallerArchive::createReadStreamForMember(const Common::Path &path) const {
	const Entry *e = findEntry(path);
	if (!e)
		return nullptr;
	return createStream(*e, false);
}

Common::SeekableReadStream *InstallerArchive::createReadStreamForMemberAltStream(
		const Common::Path &path, Common::AltStreamType altStreamType) const {
	const Entry *e = findEntry(path);
	if (!e || altStreamType != Common::AltStreamType::MacResourceFork)
		return nullptr;
	return createStream(*e, true);
}

const InstallerArchive::Entry *InstallerArchive::findEntry(const Common::Path &path) const {
	const Common::String want = path.baseName();
	for (uint i = 0; i < _entries.size(); i++) {
		if (_entries[i].name.equalsIgnoreCase(want))
			return &_entries[i];
	}
	return nullptr;
}

Common::SeekableReadStream *InstallerArchive::createStream(const Entry &entry, bool resFork) const {
	const uint32 outLen = resFork ? entry.rsrcLen : entry.dataLen;
	if (outLen == 0) {
		if (!resFork)
			return new Common::MemoryReadStream(nullptr, 0, DisposeAfterUse::NO);
		return nullptr;
	}

	const uint32 srcPos = resFork ? entry.pos : entry.pos + entry.compRLen;
	const uint32 srcLen = resFork ? entry.compRLen : entry.compDLen;
	const bool compressed = resFork ? (entry.cptFlag & 0x02) != 0
									: (entry.cptFlag & 0x04) != 0;

	if (srcPos > _archive.size() || srcLen > _archive.size() - srcPos) {
		warning("EEM installer: %s span out of range", entry.name.c_str());
		return nullptr;
	}

	byte *dst = (byte *)malloc(outLen);
	if (!dst)
		return nullptr;

	CompactProDecompressor cp(_archive.data() + srcPos, srcLen, dst, outLen);
	if (!cp.unpack(compressed, srcLen)) {
		warning("EEM installer: failed to decompress %s%s", entry.name.c_str(),
				resFork ? " (resource fork)" : "");
		free(dst);
		return nullptr;
	}

	return new Common::MemoryReadStream(dst, outLen, DisposeAfterUse::YES);
}

Common::Archive *createInstallerArchive(const Common::FSNode &dir) {
	InstallerArchive *archive = new InstallerArchive();
	if (!archive->load(dir)) {
		delete archive;
		return nullptr;
	}
	return archive;
}

} // End of namespace EEM
