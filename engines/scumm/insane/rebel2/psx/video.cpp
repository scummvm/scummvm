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
 */

#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"

#include "scumm/insane/rebel2/psx/video.h"

namespace Scumm {

static const uint32 kRawSectorSize = 2352;
static const uint32 kFrameDataOffset = 56;
static const byte kSectorTypeMask = 0x0e;
static const byte kDataSectorType = 0x08;
static const byte kVideoSectorType = 0x02;

class RA2PSXVideoReadStream final : public Common::SeekableReadStream {
public:
	RA2PSXVideoReadStream(Common::SeekableReadStream *stream,
			RA2PSXStreamDecoder::FrameVersion version) :
			_stream(stream), _version(version), _position(0), _cachedSector(-1),
			_cachedSize(0), _eos(false), _err(false) {
	}

	~RA2PSXVideoReadStream() override {
		delete _stream;
	}

	uint32 read(void *dataPtr, uint32 dataSize) override {
		byte *destination = static_cast<byte *>(dataPtr);
		uint32 total = 0;

		while (total < dataSize && _position < size()) {
			const int64 sector = _position / kRawSectorSize;
			if (!loadSector(sector))
				break;

			const uint32 sectorOffset = _position % kRawSectorSize;
			if (sectorOffset >= _cachedSize) {
				_err = true;
				break;
			}

			const uint32 amount = MIN<uint32>(dataSize - total, _cachedSize - sectorOffset);
			memcpy(destination + total, _sector + sectorOffset, amount);
			total += amount;
			_position += amount;
		}

		if (total < dataSize && _position >= size())
			_eos = true;
		return total;
	}

	bool eos() const override { return _eos; }
	bool err() const override { return _err || _stream->err(); }

	void clearErr() override {
		_eos = false;
		_err = false;
		_stream->clearErr();
	}

	int64 pos() const override { return _position; }
	int64 size() const override { return _stream->size(); }

	bool seek(int64 offset, int whence = SEEK_SET) override {
		int64 position;
		switch (whence) {
		case SEEK_SET:
			position = offset;
			break;
		case SEEK_CUR:
			position = _position + offset;
			break;
		case SEEK_END:
			position = size() + offset;
			break;
		default:
			return false;
		}

		if (position < 0 || position > size())
			return false;
		_position = position;
		_eos = false;
		return true;
	}

private:
	bool loadSector(int64 sector) {
		if (_cachedSector == sector)
			return true;

		if (!_stream->seek(sector * kRawSectorSize)) {
			_err = true;
			return false;
		}

		_cachedSize = _stream->read(_sector, kRawSectorSize);
		if (!_cachedSize) {
			_err = true;
			return false;
		}
		_cachedSector = sector;

		if (_cachedSize >= kFrameDataOffset + 12) {
			const byte sectorType = _sector[0x12] & kSectorTypeMask;
			if ((sectorType == kDataSectorType || sectorType == kVideoSectorType) &&
					READ_LE_UINT16(_sector + 28) == 0) {
				byte *frame = _sector + kFrameDataOffset;
				frame[6] = _version;
				frame[7] = 0;
				SWAP(frame[8], frame[10]);
				SWAP(frame[9], frame[11]);
			}
		}
		return true;
	}

	Common::SeekableReadStream *_stream;
	RA2PSXStreamDecoder::FrameVersion _version;
	int64 _position;
	int64 _cachedSector;
	uint32 _cachedSize;
	bool _eos;
	bool _err;
	byte _sector[kRawSectorSize];
};

RA2PSXStreamDecoder::RA2PSXStreamDecoder(FrameVersion version, uint32 frameCount) :
		Video::PSXStreamDecoder(Video::PSXStreamDecoder::kCD2x, frameCount), _version(version) {
}

bool RA2PSXStreamDecoder::loadStream(Common::SeekableReadStream *stream) {
	return Video::PSXStreamDecoder::loadStream(new RA2PSXVideoReadStream(stream, _version));
}

} // End of namespace Scumm
