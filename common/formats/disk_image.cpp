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

#include "common/stream.h"
#include "common/substream.h"
#include "common/memstream.h"
#include "common/md5.h"
#include "common/algorithm.h"
#include "common/bitstream.h"

#include "common/formats/disk_image.h"

namespace Common {

const uint kNibTrackLen = 256 * 26;

static bool detectDOS33(Common::SeekableReadStream &f, uint size) {
	uint count = 0;
	uint dos32 = 0, dos33 = 0;
	uint32 window = 0;

	while (count++ < size) {
		window &= 0xffff;
		window <<= 8;
		window |= f.readByte();

		if (f.err() || f.eos())
			return false;

		if (window == 0xd5aa96)
			++dos33;
		else if (window == 0xd5aab5)
			++dos32;
	}

	return dos33 > dos32;
}

static bool readSector_NIB(byte outBuf[], uint outBufSize, const byte inBuf[], uint inBufSize, uint &pos, const byte minNibble, const byte lookup[], const uint track, const uint sector) {
	uint z = inBufSize - (pos % inBufSize);
	if (z < outBufSize) {
		memcpy(outBuf, inBuf + (pos % inBufSize), z);
		memcpy(outBuf + z, inBuf, outBufSize - z);
	} else
		memcpy(outBuf, inBuf + (pos % inBufSize), outBufSize);
	pos += outBufSize;

	byte oldVal = 0;
	for (uint n = 0; n < outBufSize; ++n) {
		// expand
		if (outBuf[n] == 0xd5) {
			// Early end of block.
			pos -= (outBufSize - n);
			debug(2, "NIB: early end of block @ %x (%d, %d)", n, track, sector);
			return false;
		}

		byte val = 0x40;

		if (outBuf[n] >= minNibble)
			val = lookup[outBuf[n] - minNibble];

		if (val == 0x40) {
			// Badly-encoded nibbles, stop trying to decode here.
			pos -= (outBufSize - n);
			debug(2, "NIB: bad nibble %02x @ %x (%d, %d)", outBuf[n], n, track, sector);
			return false;
		}

		// undo checksum
		oldVal = val ^ oldVal;
		outBuf[n] = oldVal;
	}

	byte checksum = inBuf[pos++ % inBufSize];
	if (checksum < minNibble || oldVal != lookup[checksum - minNibble]) {
		debug(2, "NIB: checksum mismatch @ (%d, %d)", track, sector);
		return false;
	}

	return true;
}

// 4-and-4 encoding (odd-even)
static uint8 read44(byte *buffer, uint size, uint &pos) {
	// 1s in the other fields, so we can just AND
	uint8 ret = buffer[pos++ % size];
	return ((ret << 1) | 1) & buffer[pos++ % size];
}

static bool decodeTrack(Common::SeekableReadStream &stream, uint trackLen, bool dos33, byte *const diskImage, uint tracks, Common::BitArray &goodSectors) {
	// starting at 0xaa, 64 is invalid (see below)
	const byte c_5and3_lookup[] = { 64, 0, 64, 1, 2, 3, 64, 64, 64, 64, 64, 4, 5, 6, 64, 64, 7, 8, 64, 9, 10, 11, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 12, 13, 64, 64, 14, 15, 64, 16, 17, 18, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 19, 20, 64, 21, 22, 23, 64, 64, 64, 64, 64, 24, 25, 26, 64, 64, 27, 28, 64, 29, 30, 31 };
	// starting at 0x96, 64 is invalid (see below)
	const byte c_6and2_lookup[] = { 0, 1, 64, 64, 2, 3, 64, 4, 5, 6, 64, 64, 64, 64, 64, 64, 7, 8, 64, 64, 64, 9, 10, 11, 12, 13, 64, 64, 14, 15, 16, 17, 18, 19, 64, 20, 21, 22, 23, 24, 25, 26, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 27, 64, 28, 29, 30, 64, 64, 64, 31, 64, 64, 32, 33, 64, 34, 35, 36, 37, 38, 39, 40, 64, 64, 64, 64, 64, 41, 42, 43, 64, 44, 45, 46, 47, 48, 49, 50, 64, 64, 51, 52, 53, 54, 55, 56, 64, 57, 58, 59, 60, 61, 62, 63 };

	const uint sectorsPerTrack = (dos33 ? 16 : 13);
	const uint bytesPerSector = 256;

	bool sawAddress = false;
	uint8 volNo = 0, track = 0, sector = 0;

	byte *const buffer = (byte *)malloc(trackLen);
	uint firstGoodTrackPos = 0;
	uint pos = 0;

	if (stream.read(buffer, trackLen) < trackLen) {
		free(buffer);
		return false;
	}

	while (true) {
		if (pos >= trackLen + firstGoodTrackPos)
			break;

		// Read until we find two sync bytes.
		if (buffer[pos++ % trackLen] != 0xd5 || buffer[pos++ % trackLen] != 0xaa)
			continue;

		byte prologue = buffer[pos++ % trackLen];

		if (sawAddress && prologue == (dos33 ? 0x96 : 0xb5)) {
			sawAddress = false;
		}

		if (!sawAddress) {
			sawAddress = true;

			// We should always find the address field first.
			if (prologue != (dos33 ? 0x96 : 0xb5)) {
				// Accept a DOS 3.3(?) header at the start.
				if (prologue != (dos33 ? 0xb5 : 0x96) && prologue != 0xad && prologue != 0xfd)
					debug(2, "NIB: unknown field prologue %02x", prologue);
				sawAddress = false;
				continue;
			}

			volNo = read44(buffer, trackLen, pos);
			track = read44(buffer, trackLen, pos);
			sector = read44(buffer, trackLen, pos);
			uint8 checksum = read44(buffer, trackLen, pos);
			if ((volNo ^ track ^ sector) != checksum) {
				debug(2, "NIB: invalid checksum (volNo %d, track %d, sector %d)", volNo, track, sector);
				sawAddress = false;
				continue;
			}

			if (track >= tracks || sector >= sectorsPerTrack) {
				debug(2, "NIB: sector out of bounds @ (%d, %d)", track, sector);
				sawAddress = false;
				continue;
			}

			if (!firstGoodTrackPos)
				firstGoodTrackPos = pos;

			// Epilogue is de/aa plus a gap, but we don't care.
			continue;
		}

		sawAddress = false;

		// We should always find the data field after an address field.
		// TODO: we ignore volNo?
		byte *output = diskImage + (track * sectorsPerTrack + sector) * bytesPerSector;

		if (dos33) {
			// We hardcode the DOS 3.3 mapping here. TODO: Do we also need raw/prodos?
			int raw2dos[16] = { 0, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 15 };
			sector = raw2dos[sector];
			output = diskImage + (track * sectorsPerTrack + sector) * bytesPerSector;

			byte inbuffer[342];

			if (!readSector_NIB(inbuffer, sizeof(inbuffer), buffer, trackLen, pos, 0x96, c_6and2_lookup, track, sector))
				continue;

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

			if (!readSector_NIB(inbuffer, sizeof(inbuffer), buffer, trackLen, pos, 0xaa, c_5and3_lookup, track, sector))
				continue;

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

		goodSectors.set(track * sectorsPerTrack + sector);
	}

	free(buffer);
	return true;
}

static void printGoodSectors(const Common::BitArray &goodSectors, uint sectorsPerTrack) {
	if (gDebugLevel < 1) {
		return;
	}

	bool foundBadSector = false;
	for (uint i = 0; i < goodSectors.size(); ++i) {
		if (!goodSectors.get(i)) {
			if (!foundBadSector) {
				debugN(1, "Bad/missing sectors:");
				foundBadSector = true;
			}
			debugN(1, " (%d, %d)", i / sectorsPerTrack, i % sectorsPerTrack);
		}
	}
	if (foundBadSector) {
		debugN(1, "\n");
	}
}

static bool readImage_NIB(
	const char *filename,
	Common::SeekableReadStream &f,
	byte *diskImage,
	bool dos33,
	uint startTrack = 0,
	uint tracksToRead = 35) {

	if (f.size() != 35 * kNibTrackLen) {
		warning("NIB: image '%s' has invalid size of %d bytes", filename, (int)f.size());
		return false;
	}

	const uint sectorsPerTrack = (dos33 ? 16 : 13);
	Common::BitArray goodSectors(35 * sectorsPerTrack);

	f.seek(startTrack * kNibTrackLen);
	uint endTrack = startTrack + tracksToRead - 1;
	for (uint track = startTrack; track <= endTrack; ++track) {
		if (!decodeTrack(f, kNibTrackLen, dos33, diskImage, 35, goodSectors)) {
			warning("NIB: error decoding track %d in '%s'", track, filename);
			return false;
		}
	}

	printGoodSectors(goodSectors, sectorsPerTrack);
	return true;
}

static Common::SeekableReadStream *readImageToStream_NIB(Common::File &f, bool dos33, uint tracks = 35) {
	const uint sectorsPerTrack = (dos33 ? 16 : 13);
	const uint imageSize = tracks * sectorsPerTrack * 256;
	byte *const diskImage = (byte *)calloc(imageSize, 1);

	if (!readImage_NIB(f.getName(), f, diskImage, dos33, 0, tracks)) {
		warning("NIB: error reading '%s'", f.getName());
		free(diskImage);
		return nullptr;
	}

	return new Common::MemoryReadStream(diskImage, imageSize, DisposeAfterUse::YES);
}

static int getVersion_WOZ(const char *filename, Common::SeekableReadStream &f) {
	f.seek(0);
	const uint32 fileId = f.readUint32BE();

	if (f.eos() || f.err()) {
		warning("WOZ: error reading '%s'", filename);
		return 0;
	}

	if (fileId == MKTAG('W', 'O', 'Z', '1'))
		return 1;
	else if (fileId == MKTAG('W', 'O', 'Z', '2'))
		return 2;

	warning("WOZ: unsupported ID '%s' found in '%s'", tag2str(fileId), filename);
	return 0;
}

static Common::SeekableReadStream *readTrack_WOZ(const char *filename, Common::SeekableReadStream &f, uint track, bool woz2) {
	f.seek(88 + track * 4);
	const byte index = f.readByte();

	if (index == 0xff) {
		warning("WOZ: track %u not found in '%s', skipping", track, filename);
		return nullptr;
	}

	uint32 offset, byteSize, bitSize;

	if (woz2) {
		f.seek(256 + index * 8);
		offset = f.readUint16LE() << 9;
		byteSize = f.readUint16LE() << 9;
		bitSize = f.readUint32LE();
	} else {
		offset = 256 + index * 6656;
		f.seek(offset + 6646);
		byteSize = f.readUint16LE();
		bitSize = f.readUint16LE();
	}

	f.seek(offset);

	if (f.eos() || f.err() || byteSize == 0) {
		warning("WOZ: failed to read track %u in '%s', aborting", track, filename);
		return nullptr;
	}

	byte *inBuf = (byte *)malloc(byteSize);
	byte *outBuf = (byte *)malloc(byteSize);
	uint32 outSize = 0;
	if (!inBuf || !outBuf) {
		warning("WOZ: failed to create buffers of size %u for track %u in '%s'", byteSize, track, filename);
		free(inBuf);
		free(outBuf);
		return nullptr;
	}

	if (f.read(inBuf, byteSize) < byteSize) {
		warning("WOZ: error reading track %u in '%s'", track, filename);
		free(inBuf);
		free(outBuf);
		return nullptr;
	}

	Common::BitStreamMemory8MSB bitStream(new Common::BitStreamMemoryStream(inBuf, byteSize, DisposeAfterUse::YES), DisposeAfterUse::YES);

	byte nibble = 0;
	bool stop = false;
	for (;;) {
		nibble = (nibble << 1) | bitStream.getBit();

		if (nibble & 0x80) {
			if (stop)
				break;
			nibble = 0;
		}

		if (bitStream.pos() == bitSize) {
			bitStream.rewind();
			if (stop) {
				warning("WOZ: failed to find sync point for track %u in '%s'", track, filename);
				break;
			}
			stop = true;
		}
	}

	nibble = 0;
	uint32 bitsRead = 0;
	do {
		nibble = (nibble << 1) | bitStream.getBit();
		++bitsRead;

		if (nibble & 0x80) {
			outBuf[outSize++] = nibble;
			nibble = 0;
		}

		if (bitStream.pos() == bitSize)
			bitStream.rewind();
	} while (bitsRead < bitSize);

	if (nibble != 0)
		warning("WOZ: failed to sync track %u in '%s'", track, filename);

	if (outSize == 0) {
		warning("WOZ: track %u in '%s' is empty", track, filename);
		free(outBuf);
		return nullptr;
	}

	return new Common::MemoryReadStream(outBuf, outSize, DisposeAfterUse::YES);
}

static bool readImage_WOZ(
	const char *filename,
	Common::SeekableReadStream &f,
	byte *diskImage,
	bool dos33,
	uint startTrack = 0,
	uint tracksToRead = 35) {

	int version = getVersion_WOZ(filename, f);
	if (version == 0) {
		return false;
	}

	const uint sectorsPerTrack = (dos33 ? 16 : 13);
	Common::BitArray goodSectors(35 * sectorsPerTrack);

	uint endTrack = startTrack + tracksToRead - 1;
	for (uint track = startTrack; track <= endTrack; ++track) {
		StreamPtr stream(readTrack_WOZ(filename, f, track, version == 2));

		if (stream) {
			if (!decodeTrack(*stream, stream->size(), dos33, diskImage, 35, goodSectors)) {
				warning("WOZ: error decoding track %d in '%s'", track, filename);
				return false;
			}
		}
	}

	printGoodSectors(goodSectors, sectorsPerTrack);
	return true;
}

static Common::SeekableReadStream *readImageToStream_WOZ(Common::File &f, bool dos33, uint tracks = 35) {
	const uint sectorsPerTrack = (dos33 ? 16 : 13);
	const uint imageSize = tracks * sectorsPerTrack * 256;
	byte *const diskImage = (byte *)calloc(imageSize, 1);

	if (!readImage_WOZ(f.getName(), f, diskImage, dos33, 0, tracks)) {
		warning("WOZ: error reading '%s'", f.getName());
		free(diskImage);
		return nullptr;
	}

	return new Common::MemoryReadStream(diskImage, imageSize, DisposeAfterUse::YES);
}

bool DiskImage::open(const Common::Path &filename) {
	Common::File *file = new Common::File();
	debug(1, "Opening '%s'", filename.toString(Common::Path::kNativeSeparator).c_str());
	if (file->open(filename)) {
		if (open(filename.baseName(), file)) {
			return true;
		}
	} else {
		warning("Failed to open '%s'", filename.toString(Common::Path::kNativeSeparator).c_str());
	}
	delete file;
	return false;
}

bool DiskImage::open(const Common::FSNode &node) {
	Common::File *file = new Common::File();
	debug(1, "Opening '%s'", node.getPath().toString(Common::Path::kNativeSeparator).c_str());
	if (file->open(node)) {
		if (open(node.getFileName(), file)) {
			return true;
		}
	} else {
		warning("Failed to open '%s'", node.getPath().toString(Common::Path::kNativeSeparator).c_str());
	}
	delete file;
	return false;
}

bool DiskImage::open(const Common::String &name, Common::File *f) {
	if (name.hasSuffixIgnoreCase(".dsk") ||
		name.hasSuffixIgnoreCase(".do")) {
		_tracks = 35;
		_sectorsPerTrack = 16;
		_bytesPerSector = 256;
		_inputStream = f;
	} else if (name.hasSuffixIgnoreCase(".d13")) {
		_tracks = 35;
		_sectorsPerTrack = 13;
		_bytesPerSector = 256;
		_inputStream = f;
	} else if (name.hasSuffixIgnoreCase(".nib")) {
		_tracks = 35;

		if (detectDOS33(*f, kNibTrackLen))
			_sectorsPerTrack = 16;
		else
			_sectorsPerTrack = 13;

		_bytesPerSector = 256;
		if (_lazyDecoding) {
			// store the file stream and create an empty decode stream.
			// tracks will be decoded into the decode stream as they're read.
			uint32 imageSize = _tracks * _sectorsPerTrack * _bytesPerSector;
			_inputStream = f;
			_decodeBuffer = (byte *)calloc(imageSize, 1);
			_decodeStream = new Common::MemoryReadStream(_decodeBuffer, imageSize, DisposeAfterUse::YES);
			_decodedTracks.set_size(_tracks);
			_encoding = DiskImageEncodingNib;
		} else {
			// decode the entire image
			_inputStream = readImageToStream_NIB(*f, (_sectorsPerTrack == 16));
		}
	} else if (name.hasSuffixIgnoreCase(".woz")) {
		_tracks = 35;
		_sectorsPerTrack = 13;
		_bytesPerSector = 256;

		int version = getVersion_WOZ(name.c_str(), *f);

		if (version > 0) {
			StreamPtr bitStream(readTrack_WOZ(name.c_str(), *f, 0, version == 2));
			if (bitStream) {
				if (detectDOS33(*bitStream, bitStream->size()))
					_sectorsPerTrack = 16;

				if (_lazyDecoding) {
					// store the file stream and create an empty decode stream.
					// tracks will be decoded into the decode stream as they're read.
					uint32 imageSize = _tracks * _sectorsPerTrack * _bytesPerSector;
					_inputStream = f;
					_decodeBuffer = (byte *)calloc(imageSize, 1);
					_decodeStream = new Common::MemoryReadStream(_decodeBuffer, imageSize, DisposeAfterUse::YES);
					_decodedTracks.set_size(_tracks);
					_encoding = DiskImageEncodingWoz;
				} else {
					// decode the entire image
					_inputStream = readImageToStream_WOZ(*f, (_sectorsPerTrack == 16), _tracks);
				}
			} else {
				warning("WOZ: failed to load bitstream for track 0 in '%s'", name.c_str());
			}
		}
	} else if (name.hasSuffixIgnoreCase(".xfd")) {
		_tracks = 40;
		_sectorsPerTrack = 18;
		_bytesPerSector = 128;
		_inputStream = f;
	} else if (name.hasSuffixIgnoreCase(".img")) {
		_tracks = 40;
		_sectorsPerTrack = 8;
		_bytesPerSector = 512;
		_firstSector = 1;
		_inputStream = f;
	}
	
	if (_inputStream == nullptr) {
		return false;
	}

	int expectedSize = _tracks * _sectorsPerTrack * _bytesPerSector;
	if (getDiskStream()->size() != expectedSize) {
		warning("Unrecognized disk image '%s' of size %d bytes (expected %d bytes)", name.c_str(), (int)getDiskStream()->size(), expectedSize);
		if (_inputStream != f) {
			delete _inputStream;
		}
		_inputStream = nullptr;
		delete _decodeStream;
		_decodeStream = nullptr;
		return false;
	};

	if (_inputStream != f) {
		delete f;
	}

	_name = name;
	return true;
}

const DataBlockPtr DiskImage::getDataBlock(uint track, uint sector, uint offset, uint size) const {
	return DataBlockPtr(new DiskImage::DataBlock(this, track, sector, offset, size, _sectorLimit));
}

Common::SeekableReadStream *DiskImage::createReadStream(uint track, uint sector, uint offset, uint size, uint sectorLimit) const {
	const uint bytesToRead = size * _bytesPerSector + _bytesPerSector - offset;
	byte *const data = (byte *)malloc(bytesToRead);
	uint dataOffset = 0;

	if (sectorLimit == 0)
		sectorLimit = _sectorsPerTrack;

	if (sector < _firstSector || sector >= sectorLimit + _firstSector) {
		warning("Sector %u is out of bounds for %u-sector %u-based reading", sector, sectorLimit, _firstSector);
		free(data);
		return nullptr;
	}

	// lazy decoding not supported for createReadStream(), because decoding
	// tracks here requires removing way too many existing const keywords.
	// if it's ever needed, enable this code and remove all those consts.
#if 0
	if (_decodeBuffer != nullptr) {
		// lazy decoding
		uint32 bytesPerTrack = _sectorsPerTrack * _bytesPerSector;
		uint32 endTrack = track + (((sector - _firstSector) * _bytesPerSector + offset + bytesToRead - 1) / bytesPerTrack);
		for (uint32 t = track; t <= endTrack; t++) {
			if (!_decodedTracks.get(t)) {
				decodeTrack(t);
			}
		}
	}
#endif

	sector -= _firstSector;
	Common::SeekableReadStream *stream = getDiskStream();
	while (dataOffset < bytesToRead) {
		uint bytesRemInTrack = (sectorLimit - 1 - sector) * _bytesPerSector + _bytesPerSector - offset;
		stream->seek((track * _sectorsPerTrack + sector) * _bytesPerSector + offset);

		if (bytesToRead - dataOffset < bytesRemInTrack)
			bytesRemInTrack = bytesToRead - dataOffset;

		if (stream->read(data + dataOffset, bytesRemInTrack) < bytesRemInTrack) {
			warning("Error reading disk image at track %d; sector %d", track, sector);
			free(data);
			return nullptr;
		}

		++track;

		sector = 0;
		offset = 0;

		dataOffset += bytesRemInTrack;
	}

	return new Common::MemoryReadStream(data, bytesToRead, DisposeAfterUse::YES);
}

Common::SeekableReadStream *DiskImage::releaseStream() {
	Common::SeekableReadStream *stream = getDiskStream();
	
	// reset class
	if (stream != _inputStream) {
		delete _inputStream;
	}
	_inputStream = nullptr;
	_decodeStream = nullptr;
	_decodeBuffer = nullptr;
	_decodedTracks.clear();
	_encoding = DiskImageEncodingNone;
	_lazyDecoding = false;

	_tracks = 0;
	_sectorsPerTrack = 0;
	_bytesPerSector = 0;
	_sectorLimit = 0;
	_firstSector = 0;

	return stream;
}

uint32 DiskImage::read(void *dataPtr, uint32 diskPosition, uint32 dataSize) {
	Common::SeekableReadStream *stream;
	if (_decodeBuffer != nullptr) {
		// lazy decoding
		uint32 bytesPerTrack = _sectorsPerTrack * _bytesPerSector;
		uint32 startTrack = diskPosition / bytesPerTrack;
		uint32 endTrack = (diskPosition + dataSize - 1) / bytesPerTrack;
		for (uint32 t = startTrack; t <= endTrack; t++) {
			if (!_decodedTracks.get(t)) {
				decodeTrack(t);
			}
		}
		stream = _decodeStream;
	}
	else {
		stream = _inputStream;
	}
	stream->seek(diskPosition);
	return stream->read(dataPtr, dataSize);
}

void DiskImage::decodeTrack(uint track) {
	switch (_encoding) {
	case DiskImageEncodingNib:
		readImage_NIB(_name.c_str(), *_inputStream, _decodeBuffer, (_sectorsPerTrack == 16), track, 1);
		break;
	case DiskImageEncodingWoz:
		readImage_WOZ(_name.c_str(), *_inputStream, _decodeBuffer, (_sectorsPerTrack == 16), track, 1);
		break;
	default:
		return;
	}

	_decodedTracks.set(track);
}

int32 computeMD5(const Common::FSNode &node, Common::String &md5, uint32 md5Bytes) {
	Common::File f;

	if (!f.open(node))
		return -1;

	const uint tracks = md5Bytes / (13 * 256) + 1;

	if (node.getName().matchString("*.nib", true) && f.size() == 35 * kNibTrackLen) {
		bool isDOS33 = detectDOS33(f, kNibTrackLen);

		Common::SeekableReadStream *stream = readImageToStream_NIB(f, isDOS33, tracks);
		if (stream) {
			md5 = Common::computeStreamMD5AsString(*stream, md5Bytes);
			delete stream;
			return 35 * (isDOS33 ? 16 : 13) * 256;
		}

		return -1;
	} else if (node.getName().matchString("*.woz", true)) {
		int version = getVersion_WOZ(f.getName(), f);

		if (version > 0) {
			StreamPtr nibbles(readTrack_WOZ(f.getName(), f, 0, version == 2));
			if (nibbles) {
				bool isDOS33 = detectDOS33(*nibbles, nibbles->size());
				StreamPtr stream(readImageToStream_WOZ(f, isDOS33, tracks));
				if (stream) {
					md5 = Common::computeStreamMD5AsString(*stream, md5Bytes);
					return 35 * (isDOS33 ? 16 : 13) * 256;
				}
			}
		}

		return -1;
	} else {
		md5 = Common::computeStreamMD5AsString(f, md5Bytes);
		return f.size();
	}
}

const DataBlockPtr Files_Plain::getDataBlock(const Common::Path &filename, uint offset) const {
	return Common::SharedPtr<Files::DataBlock>(new Files::DataBlock(this, filename, offset));
}

Common::SeekableReadStream *Files_Plain::createReadStream(const Common::Path &filename, uint offset) const {
	Common::File *f(new Common::File());

	if (!f->open(filename))
		error("Failed to open '%s'", filename.toString(Common::Path::kNativeSeparator).c_str());

	if (offset == 0)
		return f;
	else
		return new Common::SeekableSubReadStream(f, offset, f->size(), DisposeAfterUse::YES);
}

Files_AppleDOS::~Files_AppleDOS() {
	delete _disk;
}

Files_AppleDOS::Files_AppleDOS() :
		_disk(nullptr) {
}

void Files_AppleDOS::readSectorList(TrackSector start, Common::Array<TrackSector> &list) {
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

void Files_AppleDOS::readVTOC() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_disk->createReadStream(0x11, 0x00));
	stream->readByte();
	byte track = stream->readByte();

	if (!track) {
		// VTOC probably obfuscated, try track 0x10
		stream.reset(_disk->createReadStream(0x10, 0x00));
		stream->readByte();
		track = stream->readByte();
	}

	if (!track)
		error("VTOC not found");

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

			// 0 is empty slot, 255 is deleted file
			if (sectorList.track != 0 && sectorList.track != 255) {
				readSectorList(sectorList, entry.sectors);
				_toc[name] = entry;
			}
		}
	}
}

const DataBlockPtr Files_AppleDOS::getDataBlock(const Common::Path &filename, uint offset) const {
	return Common::SharedPtr<Files::DataBlock>(new Files::DataBlock(this, filename, offset));
}

Common::SeekableReadStream *Files_AppleDOS::createReadStreamText(const TOCEntry &entry) const {
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

Common::SeekableReadStream *Files_AppleDOS::createReadStreamBinary(const TOCEntry &entry) const {
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

Common::SeekableReadStream *Files_AppleDOS::createReadStream(const Common::Path &filename, uint offset) const {
	if (!_toc.contains(filename))
		error("Failed to locate '%s'", filename.toString().c_str());

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

bool Files_AppleDOS::open(const Common::Path &filename) {
	_disk = new DiskImage();
	if (!_disk->open(filename))
		return false;

	readVTOC();
	return true;
}

} // End of namespace Common
