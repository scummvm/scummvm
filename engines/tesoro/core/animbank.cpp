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

#include "tesoro/core/animbank.h"

#include "common/endian.h"
#include "common/textconsole.h"

namespace Tesoro {

static const uint32 kFileHeaderSize = 0x18;
static const uint32 kFileSequenceSize = 0x18;

enum {
	kSequenceDwords = 2, // sequence contains raw 32-bit dwords
	kSequencePoints = 4  // sequence contains points (x | y << 16) and a scale
};

/**
 * Returns true if read size bytes successfully, false otherwise.
 */
static bool readExactly(Common::SeekableReadStream &s, byte *buf, uint32 size) {
	return size == 0 || s.read(buf, size) == size;
}

bool AnimBank::load(Common::SeekableReadStream &dat, uint32 entryOffset, int mode, bool is555) {
	*this = AnimBank();

	// The data starts four bytes after the offset given by the archive.
	if (!dat.seek(entryOffset + 4, SEEK_SET))
		return false;

	byte head[kFileHeaderSize];
	if (!readExactly(dat, head, sizeof(head)))
		return false;
	const uint32 sequenceCount = READ_LE_UINT16(head);
	const uint32 imageCount = READ_LE_UINT16(head + 4);

	_mode = mode;
	_headerFlag = head[0x14] == 1;
	for (uint i = 0; i < 3; ++i)
		_header[i] = READ_LE_UINT32(head + 8 + 4 * i);

	_sequences.resize(sequenceCount);
	for (uint32 i = 0; i < sequenceCount; ++i) {
		byte rec[kFileSequenceSize];
		if (!readExactly(dat, rec, sizeof(rec))) {
			*this = AnimBank();
			return false;
		}

		AnimSequence &seq = _sequences[i];
		seq.imageX = (int16)READ_LE_UINT16(rec);
		seq.imageY = (int16)READ_LE_UINT16(rec + 2);
		seq.x = (int16)READ_LE_UINT16(rec + 4);
		seq.y = (int16)READ_LE_UINT16(rec + 6);
		seq.duration = READ_LE_UINT16(rec + 8);
		seq.imageIndex = (int16)READ_LE_UINT16(rec + 0xa);
		seq.hitX = (int16)READ_LE_UINT16(rec + 0xc);
		seq.hitY = (int16)READ_LE_UINT16(rec + 0xe);
		seq.hitW = (int16)READ_LE_UINT16(rec + 0x10);
		seq.hitH = (int16)READ_LE_UINT16(rec + 0x12);
		seq.type = READ_LE_UINT16(rec + 0x14);
		seq.firstDescent = 0;

		const uint32 extraSize = READ_LE_UINT16(rec + 0x16);
		Common::Array<byte> extra(extraSize);
		if (!readExactly(dat, extraSize ? &extra[0] : nullptr, extraSize)) {
			*this = AnimBank();
			return false;
		}

		if (seq.type == kSequenceDwords || seq.type == kSequencePoints) {
			const bool points = seq.type == kSequencePoints;
			const uint32 count = extraSize >= 2 ? READ_LE_UINT16(&extra[0]) : 0;
			const uint32 skip = points ? 8 : 6;
			const uint32 stride = points ? 6 : 4;
			if (count && skip + count * stride > extraSize)
				warning("AnimBank: sequence %u claims more points than its data holds", i);

			// Points the data does not hold read as zero.
			extra.resize(MAX<uint32>(extraSize, skip + count * stride), 0);
			for (uint32 k = 0; k < count; ++k) {
				const byte *e = &extra[skip + k * stride];
				seq.entries.push_back(READ_LE_UINT32(e));
				if (points)
					seq.scales.push_back(READ_LE_UINT16(e + 4));
			}

			// Entries are expected to be in ascending x order; remember where that stops.
			uint32 d = 0;
			while (d + 1 < count && (int16)(seq.entries[d + 1] & 0xffff) >= (int16)(seq.entries[d] & 0xffff))
				++d;
			seq.firstDescent = d;
		} else if (seq.type > kSequencePoints || seq.type == 3) {
			warning("AnimBank: sequence %u has unsupported type %u", i, seq.type);
		}
	}

	_images.resize(imageCount);
	_imagePositions.resize(imageCount);
	for (uint32 k = 0; k < imageCount; ++k) {
		_imagePositions[k] = (uint32)dat.pos();
		bool ok;
		if (mode == kLoadDeferred) {
			const uint32 size = dat.readUint32LE();
			ok = !dat.eos() && size <= (uint32)(dat.size() - dat.pos()) && dat.skip(size);
		} else {
			ok = _images[k].load(dat, is555);
		}
		if (!ok) {
			*this = AnimBank();
			return false;
		}
	}

	// Positions are relative to the first sequence.
	if (!_sequences.empty()) {
		const int16 x0 = _sequences[0].x, y0 = _sequences[0].y;
		for (uint32 i = 1; i < sequenceCount; ++i) {
			_sequences[i].x = (int16)(uint16)(_sequences[i].x - x0);
			_sequences[i].y = (int16)(uint16)(_sequences[i].y - y0);
		}
		_sequences[0].x = 0;
		_sequences[0].y = 0;
	}
	return true;
}

} // End of namespace Tesoro
