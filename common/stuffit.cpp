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

// StuffIt parsing based on http://code.google.com/p/theunarchiver/wiki/StuffItFormat
// Compression 14 based on libxad (http://sourceforge.net/projects/libxad/)

#include "common/stuffit.h"

#include "common/archive.h"
#include "common/bitstream.h"
#include "common/debug.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/substream.h"

namespace Common {

struct SIT14Data;

class StuffItArchive : public Common::Archive {
public:
	StuffItArchive();
	~StuffItArchive() override;

	bool open(const Common::String &filename);
	void close();
	bool isOpen() const { return _stream != 0; }

	// Common::Archive API implementation
	bool hasFile(const Common::String &name) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;

private:
	struct FileEntry {
		byte compression;
		uint32 uncompressedSize;
		uint32 compressedSize;
		uint32 offset;
	};

	Common::SeekableReadStream *_stream;

	typedef Common::HashMap<Common::String, FileEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
	FileMap _map;

	// Decompression Functions
	Common::SeekableReadStream *decompress14(Common::SeekableReadStream *src, uint32 uncompressedSize) const;

	// Decompression Helpers
	void update14(uint16 first, uint16 last, byte *code, uint16 *freq) const;
	void readTree14(Common::BitStream8LSB *bits, SIT14Data *dat, uint16 codesize, uint16 *result) const;
};

StuffItArchive::StuffItArchive() : Common::Archive() {
	_stream = nullptr;
}

StuffItArchive::~StuffItArchive() {
	close();
}

// Some known values of StuffIt FourCC's
// 11H Mac in particular uses ST46, while EMI Mac uses ST65
static const uint32 s_magicNumbers[] = {
	MKTAG('S', 'I', 'T', '!'), MKTAG('S', 'T', '6', '5'), MKTAG('S', 'T', '5', '0'),
	MKTAG('S', 'T', '6', '0'), MKTAG('S', 'T', 'i', 'n'), MKTAG('S', 'T', 'i', '2'),
	MKTAG('S', 'T', 'i', '3'), MKTAG('S', 'T', 'i', '4'), MKTAG('S', 'T', '4', '6')
};

bool StuffItArchive::open(const Common::String &filename) {
	close();

	_stream = SearchMan.createReadStreamForMember(filename);

	if (!_stream)
		return false;

	uint32 tag = _stream->readUint32BE();

	// Check all the possible FourCC's
	bool found = false;
	for (int i = 0; i < ARRAYSIZE(s_magicNumbers); i++) {
		if (tag == s_magicNumbers[i]) {
			found = true;
			break;
		}
	}

	// Didn't find one, let's bail out
	if (!found) {
		close();
		return false;
	}

	/* uint16 fileCount = */ _stream->readUint16BE();
	/* uint32 archiveSize = */ _stream->readUint32BE();

	// Some sort of second magic number
	if (_stream->readUint32BE() != MKTAG('r', 'L', 'a', 'u')) {
		close();
		return false;
	}

	/* byte version = */ _stream->readByte(); // meaning not clear

	_stream->skip(7); // unknown

	while (_stream->pos() < _stream->size() && !_stream->eos()) {
		byte resForkCompression = _stream->readByte();
		byte dataForkCompression = _stream->readByte();

		byte fileNameLength = _stream->readByte();
		Common::String name;

		for (byte i = 0; i < fileNameLength; i++)
			name += (char)_stream->readByte();

		// Skip remaining bytes
		_stream->skip(63 - fileNameLength);

		/* uint32 fileType = */ _stream->readUint32BE();
		/* uint32 fileCreator = */ _stream->readUint32BE();
		/* uint16 finderFlags = */ _stream->readUint16BE();
		/* uint32 creationDate = */ _stream->readUint32BE();
		/* uint32 modificationDate = */ _stream->readUint32BE();
		uint32 resForkUncompressedSize = _stream->readUint32BE();
		uint32 dataForkUncompressedSize = _stream->readUint32BE();
		uint32 resForkCompressedSize = _stream->readUint32BE();
		uint32 dataForkCompressedSize = _stream->readUint32BE();
		/* uint16 resForkCRC = */ _stream->readUint16BE();
		/* uint16 dataForkCRC = */ _stream->readUint16BE();
		_stream->skip(6); // unknown
		/* uint16 headerCRC = */ _stream->readUint16BE();

		// Ignore directories for now
		if (dataForkCompression == 32 || dataForkCompression == 33)
			continue;

		if (dataForkUncompressedSize != 0) {
			// We have a data fork

			FileEntry entry;
			entry.compression = dataForkCompression;
			entry.uncompressedSize = dataForkUncompressedSize;
			entry.compressedSize = dataForkCompressedSize;
			entry.offset = _stream->pos() + resForkCompressedSize;
			_map[name] = entry;

			debug(0, "StuffIt file '%s', Compression = %d", name.c_str(), entry.compression);
		}

		if (resForkUncompressedSize != 0) {
			// We have a resource fork

			// Add a .rsrc extension so we know it's the resource fork
			name += ".rsrc";

			FileEntry entry;
			entry.compression = resForkCompression;
			entry.uncompressedSize = resForkUncompressedSize;
			entry.compressedSize = resForkCompressedSize;
			entry.offset = _stream->pos();
			_map[name] = entry;

			debug(0, "StuffIt file '%s', Compression = %d", name.c_str(), entry.compression);
		}

		// Go to the next entry
		_stream->skip(dataForkCompressedSize + resForkCompressedSize);
	}

	return true;
}

void StuffItArchive::close() {
	delete _stream; _stream = nullptr;
	_map.clear();
}

bool StuffItArchive::hasFile(const Common::String &name) const {
	return _map.contains(name);
}

int StuffItArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (FileMap::const_iterator it = _map.begin(); it != _map.end(); it++)
		list.push_back(getMember(it->_key));

	return _map.size();
}

const Common::ArchiveMemberPtr StuffItArchive::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *StuffItArchive::createReadStreamForMember(const Common::String &name) const {
	if (!_stream || !_map.contains(name))
		return nullptr;

	const FileEntry &entry = _map[name];

	if (entry.compression & 0xF0)
		error("Unhandled StuffIt encryption");

	Common::SeekableSubReadStream subStream(_stream, entry.offset, entry.offset + entry.compressedSize);

	// We currently only support type 14 compression
	switch (entry.compression) {
	case 0: // Uncompressed
		return subStream.readStream(subStream.size());
	case 14: // Installer
		return decompress14(&subStream, entry.uncompressedSize);
	default:
		error("Unhandled StuffIt compression %d", entry.compression);
	}

	return nullptr;
}

void StuffItArchive::update14(uint16 first, uint16 last, byte *code, uint16 *freq) const {
	uint16 i, j;

	while (last - first > 1) {
		i = first;
		j = last;

		do {
			while (++i < last && code[first] > code[i])
				;

			while (--j > first && code[first] < code[j])
				;

			if (j > i) {
				SWAP(code[i], code[j]);
				SWAP(freq[i], freq[j]);
			}
		} while (j > i);

		if (first != j) {
			SWAP(code[first], code[j]);
			SWAP(freq[first], freq[j]);

			i = j + 1;

			if (last - i <= j - first) {
				update14(i, last, code, freq);
				last = j;
			} else {
				update14(first, j, code, freq);
				first = i;
			}
		} else {
			++first;
		}
	}
}

struct SIT14Data {
	byte code[308];
	byte codecopy[308];
	uint16 freq[308];
	uint32 buff[308];

	byte var1[52];
	uint16 var2[52];
	uint16 var3[75 * 2];

	byte var4[76];
	uint32 var5[75];
	byte var6[1024];
	uint16 var7[308 * 2];
	byte var8[0x4000];

	byte window[0x40000];
};

// Realign to a byte boundary
#define ALIGN_BITS(b) \
	if (b->pos() & 7) \
		b->skip(8 - (b->pos() & 7))

void StuffItArchive::readTree14(Common::BitStream8LSB *bits, SIT14Data *dat, uint16 codesize, uint16 *result) const {
	uint32 i, l, n;
	uint32 k = bits->getBit();
	uint32 j = bits->getBits(2) + 2;
	uint32 o = bits->getBits(3) + 1;
	uint32 size = 1 << j;
	uint32 m = size - 1;
	k = k ? (m - 1) : 0xFFFFFFFF;

	if (bits->getBits(2) & 1) { // skip 1 bit!
		// requirements for this call: dat->buff[32], dat->code[32], dat->freq[32*2]
		readTree14(bits, dat, size, dat->freq);

		for (i = 0; i < codesize; ) {
			l = 0;

			do {
				l = dat->freq[l + bits->getBit()];
				n = size << 1;
			} while (n > l);

			l -= n;

			if (k != l) {
				if (l == m) {
					l = 0;

					do {
						l = dat->freq[l + bits->getBit()];
						n = size <<  1;
					} while (n > l);

					l += 3 - n;

					while (l--) {
						dat->code[i] = dat->code[i - 1];
						++i;
					}
				} else {
					dat->code[i++] = l + o;
				}
			} else {
				dat->code[i++] = 0;
			}
		}
	} else {
		for (i = 0; i < codesize; ) {
			l = bits->getBits(j);

			if (k != l) {
				if (l == m) {
					l = bits->getBits(j) + 3;

					while (l--) {
						dat->code[i] = dat->code[i - 1];
						++i;
					}
				} else {
					dat->code[i++] = l + o;
				}
			} else {
				dat->code[i++] = 0;
			}
		}
	}

	for (i = 0; i < codesize; ++i) {
		dat->codecopy[i] = dat->code[i];
		dat->freq[i] = i;
	}

	update14(0, codesize, dat->codecopy, dat->freq);

	for (i = 0; i < codesize && !dat->codecopy[i]; ++i)
		; // find first nonempty

	for (j = 0; i < codesize; ++i, ++j) {
		if (i)
			j <<= (dat->codecopy[i] - dat->codecopy[i - 1]);

		k = dat->codecopy[i];
		m = 0;

		for (l = j; k--; l >>= 1)
			m = (m << 1) | (l & 1);

		dat->buff[dat->freq[i]] = m;
	}

	for (i = 0; i < (uint32)codesize * 2; ++i)
		result[i] = 0;

	j = 2;

	for (i = 0; i < codesize; ++i) {
		l = 0;
		m = dat->buff[i];

		for (k = 0; k < dat->code[i]; ++k) {
			l += (m & 1);

			if (dat->code[i] - 1 <= (int32)k) {
				result[l] = codesize * 2 + i;
			} else {
				if (!result[l]) {
					result[l] = j;
					j += 2;
				}

				l = result[l];
			}

			m >>= 1;
		}
	}

	ALIGN_BITS(bits);
}

#define OUTPUT_VAL(x) \
	out.writeByte(x); \
	dat->window[j++] = x; \
	j &= 0x3FFFF

Common::SeekableReadStream *StuffItArchive::decompress14(Common::SeekableReadStream *src, uint32 uncompressedSize) const {
	byte *dst = (byte *)malloc(uncompressedSize);
	Common::MemoryWriteStream out(dst, uncompressedSize);

	Common::BitStream8LSB *bits = new Common::BitStream8LSB(src);

	uint32 i, j, k, l, m, n;

	SIT14Data *dat = new SIT14Data();

	// initialization
	for (i = k = 0; i < 52; ++i) {
		dat->var2[i] = k;
		k += (1 << (dat->var1[i] = ((i >= 4) ? ((i - 4) >> 2) : 0)));
	}

	for (i = 0; i < 4; ++i)
		dat->var8[i] = i;

	for (m = 1, l = 4; i < 0x4000; m <<= 1) // i is 4
		for (n = l + 4; l < n; ++l)
			for (j = 0; j < m; ++j)
				dat->var8[i++] = l;

	for (i = 0, k = 1; i < 75; ++i) {
		dat->var5[i] = k;
		k += (1 << (dat->var4[i] = (i >= 3 ? ((i - 3) >> 2) : 0)));
	}

	for (i = 0; i < 4; ++i)
		dat->var6[i] = i - 1;

	for (m = 1, l = 3; i < 0x400; m <<= 1) // i is 4
		for (n = l + 4; l < n; ++l)
			for (j = 0; j < m; ++j)
				dat->var6[i++] = l;

	m = bits->getBits(16); // number of blocks
	j = 0; // window position

	while (m-- && !bits->eos()) {
		bits->getBits(16); // skip crunched block size
		bits->getBits(16);
		n = bits->getBits(16); // number of uncrunched bytes
		n |= bits->getBits(16) << 16;
		readTree14(bits, dat, 308, dat->var7);
		readTree14(bits, dat, 75, dat->var3);

		while (n && !bits->eos()) {
			for (i = 0; i < 616;)
				i = dat->var7[i + bits->getBit()];

			i -= 616;

			if (i < 0x100) {
				OUTPUT_VAL(i);
				--n;
			} else {
				i -= 0x100;
				k = dat->var2[i] + 4;
				i = dat->var1[i];

				if (i)
					k += bits->getBits(i);

				for (i = 0; i < 150;)
					i = dat->var3[i + bits->getBit()];

				i -= 150;
				l = dat->var5[i];
				i = dat->var4[i];

				if (i)
					l += bits->getBits(i);

				n -= k;
				l = j + 0x40000 - l;

				while (k--) {
					l &= 0x3FFFF;
					OUTPUT_VAL(dat->window[l]);
					l++;
				}
			}
		}

		ALIGN_BITS(bits);
	}

	delete dat;
	delete bits;

	return new Common::MemoryReadStream(dst, uncompressedSize, DisposeAfterUse::YES);
}

#undef OUTPUT_VAL
#undef ALIGN_BITS

Common::Archive *createStuffItArchive(const Common::String &fileName) {
	StuffItArchive *archive = new StuffItArchive();

	if (!archive->open(fileName)) {
		delete archive;
		return 0;
	}

	return archive;
}

} // End of namespace Common
