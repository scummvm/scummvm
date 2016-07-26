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

#include "common/stream.h"
#include "common/substream.h"
#include "common/memstream.h"

#include "adl/disk.h"

namespace Adl {

const DataBlockPtr DiskImage_DSK::getDataBlock(uint track, uint sector, uint offset, uint size) const {
	return Common::SharedPtr<DiskImage::DataBlock>(new DiskImage::DataBlock(this, track, sector, offset, size));
}

Common::SeekableReadStream *DiskImage_DSK::createReadStream(uint track, uint sector, uint offset, uint size) const {
	_f->seek((track * _sectorsPerTrack + sector) * _bytesPerSector + offset);
	Common::SeekableReadStream *stream = _f->readStream(size * _bytesPerSector + _bytesPerSector - offset);

	if (_f->eos() || _f->err())
		error("Error reading disk image");

	return stream;
}

bool DiskImage_DSK::open(const Common::String &filename) {
	assert(!_f->isOpen());

	if (!_f->open(filename))
		return false;

	uint filesize = _f->size();
	switch (filesize) {
	case 143360:
		_tracks = 35;
		_sectorsPerTrack = 16;
		_bytesPerSector = 256;
		break;
	default:
		warning("Unrecognized disk image '%s' of size %d bytes", filename.c_str(), filesize);
		return false;
	}

	return true;
}

const DataBlockPtr DiskImage_NIB::getDataBlock(uint track, uint sector, uint offset, uint size) const {
	return Common::SharedPtr<DiskImage::DataBlock>(new DiskImage::DataBlock(this, track, sector, offset, size));
}

Common::SeekableReadStream *DiskImage_NIB::createReadStream(uint track, uint sector, uint offset, uint size) const {
	_memStream->seek((track * _sectorsPerTrack + sector) * _bytesPerSector + offset);
	Common::SeekableReadStream *stream = _memStream->readStream(size * _bytesPerSector + _bytesPerSector - offset);

	if (_memStream->eos() || _memStream->err())
		error("Error reading NIB image");

	return stream;
}

// 4-and-4 encoding (odd-even)
static uint8 read44(Common::SeekableReadStream *f) {
	// 1s in the other fields, so we can just AND
	uint8 ret = f->readByte();
	return ((ret << 1) | 1) & f->readByte();
}

bool DiskImage_NIB::open(const Common::String &filename) {
	assert(!_f->isOpen());

	if (!_f->open(filename))
		return false;

	uint filesize = _f->size();
	switch (filesize) {
	case 232960:
		_tracks = 35;
		_sectorsPerTrack = 16; // we always pad it out
		_bytesPerSector = 256;
		break;
	default:
		error("Unrecognized NIB image '%s' of size %d bytes", filename.c_str(), filesize);
	}

	// starting at 0xaa, 32 is invalid (see below)
	const byte c_5and3_lookup[] = { 32, 0, 32, 1, 2, 3, 32, 32, 32, 32, 32, 4, 5, 6, 32, 32, 7, 8, 32, 9, 10, 11, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 12, 13, 32, 32, 14, 15, 32, 16, 17, 18, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 19, 20, 32, 21, 22, 23, 32, 32, 32, 32, 32, 24, 25, 26, 32, 32, 27, 28, 32, 29, 30, 31 };
	// starting at 0x96, 64 is invalid (see below)
	const byte c_6and2_lookup[] = { 0, 1, 64, 64, 2, 3, 64, 4, 5, 6, 64, 64, 64, 64, 64, 64, 7, 8, 64, 64, 64, 9, 10, 11, 12, 13, 64, 64, 14, 15, 16, 17, 18, 19, 64, 20, 21, 22, 23, 24, 25, 26, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 27, 64, 28, 29, 30, 64, 64, 64, 31, 64, 64, 32, 33, 64, 34, 35, 36, 37, 38, 39, 40, 64, 64, 64, 64, 64, 41, 42, 43, 64, 44, 45, 46, 47, 48, 49, 50, 64, 64, 51, 52, 53, 54, 55, 56, 64, 57, 58, 59, 60, 61, 62, 63 };

	uint32 diskSize = _tracks * _sectorsPerTrack * _bytesPerSector;
	byte *diskImage = (byte *)calloc(diskSize, 1);
	_memStream = new Common::MemoryReadStream(diskImage, diskSize, DisposeAfterUse::YES);

	bool sawAddress = false;
	uint8 volNo, track, sector;
	bool newStyle;

	while (_f->pos() < _f->size()) {
		// Read until we find two sync bytes.
		if (_f->readByte() != 0xd5 || _f->readByte() != 0xaa)
			continue;

		byte prologue = _f->readByte();

		if (sawAddress && (prologue == 0xb5 || prologue == 0x96)) {
			warning("NIB: data for %02x/%02x/%02x missing", volNo, track, sector);
			sawAddress = false;
		}

		if (!sawAddress) {
			sawAddress = true;
			newStyle = false;

			// We should always find the address field first.
			if (prologue != 0xb5) {
				// Accept a DOS 3.3(?) header at the start.
				if (prologue == 0x96) {
					newStyle = true;
				} else {
					error("unknown NIB field prologue %02x", prologue);
				}
			}

			volNo = read44(_f);
			track = read44(_f);
			sector = read44(_f);
			uint8 checksum = read44(_f);
			if ((volNo ^ track ^ sector) != checksum)
				error("invalid NIB checksum");

			// FIXME: This is a hires0/hires2-specific hack.
			if (volNo == 0xfe) {
				if (track == 1)
					track = 2;
				else if (track == 2)
					track = 1;
			}

			// Epilogue is de/aa plus a gap, but we don't care.
			continue;
		}

		sawAddress = false;

		// We should always find the data field after an address field.
		// TODO: we ignore volNo?
		byte *output = diskImage + (track * _sectorsPerTrack + sector) * _bytesPerSector;

		if (newStyle) {
			// We hardcode the DOS 3.3 mapping here. TODO: Do we also need raw/prodos?
			int raw2dos[16] = { 0, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 15 };
			sector = raw2dos[sector];
			output = diskImage + (track * _sectorsPerTrack + sector) * _bytesPerSector;

			// 6-and-2 uses 342 on-disk bytes
			byte inbuffer[342];
			_f->read(inbuffer, 342);

			byte oldVal = 0;
			for (uint n = 0; n < 342; ++n) {
				// expand
				assert(inbuffer[n] >= 0x96); // corrupt file (TODO: assert?)
				byte val = c_6and2_lookup[inbuffer[n] - 0x96];
				if (val == 0x40) {
					error("NIB: invalid nibble value %02x", inbuffer[n]);
				}
				// undo checksum
				oldVal = val ^ oldVal;
				inbuffer[n] = oldVal;
			}

			byte checksum = _f->readByte();
			if (checksum < 0x96 || oldVal != c_6and2_lookup[checksum - 0x96])
				warning("NIB: checksum mismatch @ (%x, %x)", track, sector);

			for (uint n = 0; n < 256; ++n) {
				output[n] = inbuffer[86 + n] << 2;
				if (n < 86) { // use first pair of bits
					output[n] |= ((inbuffer[n] & 1) << 1);
					output[n] |= ((inbuffer[n] & 2) >> 1);
				} else if (n < 86*2) { // second pair
					output[n] |= ((inbuffer[n-86] & 4) >> 1);
					output[n] |= ((inbuffer[n-86] & 8) >> 3);
				} else { // third pair
					output[n] |= ((inbuffer[n-86*2] & 0x10) >> 3);
					output[n] |= ((inbuffer[n-86*2] & 0x20) >> 5);
				}
			}
		} else {
			// 5-and-3 uses 410 on-disk bytes, decoding to just over 256 bytes
			byte inbuffer[410];
			_f->read(inbuffer, 410);

			bool truncated = false;
			byte oldVal = 0;
			for (uint n = 0; n < 410; ++n) {
				// expand
				assert(inbuffer[n] >= 0xaa); // corrupt file (TODO: assert?)
				if (inbuffer[n] == 0xd5) {
					// Early end of block.
					truncated = true;
					_f->seek(-(410 - n), SEEK_CUR);
					warning("NIB: early end of block @ 0x%x (%x, %x)", _f->pos(), track, sector);
					break;
				}
				byte val = c_5and3_lookup[inbuffer[n] - 0xaa];
				if (val == 0x20) {
					// Badly-encoded nibbles, stop trying to decode here.
					truncated = true;
					warning("NIB: bad nibble %02x @ 0x%x (%x, %x)", inbuffer[n], _f->pos(), track, sector);
					_f->seek(-(410 - n), SEEK_CUR);
					break;
				}
				// undo checksum
				oldVal = val ^ oldVal;
				inbuffer[n] = oldVal;
			}
			if (!truncated) {
				byte checksum = _f->readByte();
				if (checksum < 0xaa || oldVal != c_5and3_lookup[checksum - 0xaa])
					warning("NIB: checksum mismatch @ (%x, %x)", track, sector);
			}

			// 8 bytes of nibbles expand to 5 bytes
			// so we have 51 of these batches (255 bytes), plus 2 bytes of 'leftover' nibbles for byte 256
			for (uint n = 0; n < 51; ++n) {
				// e.g. figure 3.18 of Beneath Apple DOS
				byte lowbits1 = inbuffer[51*3 - n];
				byte lowbits2 = inbuffer[51*2 - n];
				byte lowbits3 = inbuffer[51*1 - n];
				byte lowbits4 = (lowbits1 & 2) << 1 | (lowbits2 & 2) | (lowbits3 & 2) >> 1;
				byte lowbits5 = (lowbits1 & 1) << 2 | (lowbits2 & 1) << 1 | (lowbits3 & 1);
				output[250 - 5*n] = (inbuffer[n + 51*3 + 1] << 3) | ((lowbits1 >> 2) & 0x7);
				output[251 - 5*n] = (inbuffer[n + 51*4 + 1] << 3) | ((lowbits2 >> 2) & 0x7);
				output[252 - 5*n] = (inbuffer[n + 51*5 + 1] << 3) | ((lowbits3 >> 2) & 0x7);
				output[253 - 5*n] = (inbuffer[n + 51*6 + 1] << 3) | lowbits4;
				output[254 - 5*n] = (inbuffer[n + 51*7 + 1] << 3) | lowbits5;
			}
			output[255] = (inbuffer[409] << 3) | (inbuffer[0] & 0x7);
		}
	}

	return true;
}

const DataBlockPtr Files_Plain::getDataBlock(const Common::String &filename, uint offset) const {
	return Common::SharedPtr<Files::DataBlock>(new Files::DataBlock(this, filename, offset));
}

Common::SeekableReadStream *Files_Plain::createReadStream(const Common::String &filename, uint offset) const {
	Common::File *f(new Common::File());

	if (!f->open(filename))
		error("Failed to open '%s'", filename.c_str());

	if (offset == 0)
		return f;
	else
		return new Common::SeekableSubReadStream(f, offset, f->size(), DisposeAfterUse::YES);
}

Files_DOS33::~Files_DOS33() {
	delete _disk;
}

Files_DOS33::Files_DOS33() :
		_disk(nullptr) {
}

void Files_DOS33::readSectorList(TrackSector start, Common::Array<TrackSector> &list) {
	TrackSector index = start;

	while (index.track != 0) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(_disk->createReadStream(index.track, index.sector));

		stream->readByte();
		index.track = stream->readByte();
		index.sector = stream->readByte();

		stream->seek(9, SEEK_CUR);

		// This only handles sequential files
		TrackSector ts;
		ts.track = stream->readByte();
		ts.sector = stream->readByte();

		while (ts.track != 0) {
			list.push_back(ts);

			ts.track = stream->readByte();
			ts.sector = stream->readByte();

			if (stream->err())
				error("Error reading sector list");

			if (stream->eos())
				break;
		}
	}
}

void Files_DOS33::readVTOC() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_disk->createReadStream(0x11, 0x00));
	stream->readByte();
	byte track = stream->readByte();
	byte sector = stream->readByte();

	while (track != 0) {
		char name[kFilenameLen + 1] = { };

		stream.reset(_disk->createReadStream(track, sector));
		stream->readByte();
		track = stream->readByte();
		sector = stream->readByte();
		stream->seek(8, SEEK_CUR);

		for (uint i = 0; i < 7; ++i) {
			TOCEntry entry;
			TrackSector sectorList;
			sectorList.track = stream->readByte();
			sectorList.sector = stream->readByte();
			entry.type = stream->readByte();
			stream->read(name, kFilenameLen);

			// Convert to ASCII
			for (uint j = 0; j < kFilenameLen; j++)
				name[j] &= 0x7f;

			// Strip trailing spaces
			for (int j = kFilenameLen - 1; j >= 0; --j) {
				if (name[j] == ' ')
					name[j] = 0;
				else
					break;
			}

			entry.totalSectors = stream->readUint16BE();

			if (sectorList.track != 0) {
				readSectorList(sectorList, entry.sectors);
				_toc[name] = entry;
			}
		}
	}
}

const DataBlockPtr Files_DOS33::getDataBlock(const Common::String &filename, uint offset) const {
	return Common::SharedPtr<Files::DataBlock>(new Files::DataBlock(this, filename, offset));
}

Common::SeekableReadStream *Files_DOS33::createReadStreamText(const TOCEntry &entry) const {
	byte *buf = (byte *)malloc(entry.sectors.size() * kSectorSize);
	byte *p = buf;

	for (uint i = 0; i < entry.sectors.size(); ++i) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(_disk->createReadStream(entry.sectors[i].track, entry.sectors[i].sector));

		assert(stream->size() == kSectorSize);

		while (true) {
			byte textChar = stream->readByte();

			if (stream->eos() || textChar == 0)
				break;

			if (stream->err())
				error("Error reading text file");

			*p++ = textChar;
		}
	}

	return new Common::MemoryReadStream(buf, p - buf, DisposeAfterUse::YES);
}

Common::SeekableReadStream *Files_DOS33::createReadStreamBinary(const TOCEntry &entry) const {
	byte *buf = (byte *)malloc(entry.sectors.size() * kSectorSize);

	Common::ScopedPtr<Common::SeekableReadStream> stream(_disk->createReadStream(entry.sectors[0].track, entry.sectors[0].sector));

	if (entry.type == kFileTypeBinary)
		stream->readUint16LE(); // Skip start address

	uint16 size = stream->readUint16LE();
	uint16 offset = 0;
	uint16 sectorIdx = 1;

	while (true) {
		offset += stream->read(buf + offset, size - offset);

		if (offset == size)
			break;

		if (stream->err())
			error("Error reading binary file");

		assert(stream->eos());

		if (sectorIdx == entry.sectors.size())
			error("Not enough sectors for binary file size");

		stream.reset(_disk->createReadStream(entry.sectors[sectorIdx].track, entry.sectors[sectorIdx].sector));
		++sectorIdx;
	}

	return new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES);
}

Common::SeekableReadStream *Files_DOS33::createReadStream(const Common::String &filename, uint offset) const {
	if (!_toc.contains(filename))
		error("Failed to locate '%s'", filename.c_str());

	const TOCEntry &entry = _toc[filename];

	Common::SeekableReadStream *stream;

	switch(entry.type) {
	case kFileTypeText:
		stream = createReadStreamText(entry);
		break;
	case kFileTypeAppleSoft:
	case kFileTypeBinary:
		stream = createReadStreamBinary(entry);
		break;
	default:
		error("Unsupported file type %i", entry.type);
	}

	return new Common::SeekableSubReadStream(stream, offset, stream->size(), DisposeAfterUse::YES);
}

bool Files_DOS33::open(const Common::String &filename) {
	_disk = new DiskImage_DSK();
	if (!_disk->open(filename))
		return false;

	readVTOC();
	return true;
}

} // End of namespace Adl
