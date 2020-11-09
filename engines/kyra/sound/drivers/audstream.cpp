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

#include "kyra/resource/resource.h"
#include "audio/audiostream.h"

#include "common/util.h"

namespace Kyra {

// Thanks to Torbjorn Andersson (eriktorbjorn) for his aud player on which
// this code is based on

// TODO: cleanup of whole AUDStream

class AUDStream : public Audio::SeekableAudioStream {
public:
	AUDStream(Common::SeekableReadStream* stream);
	~AUDStream() override;

	int readBuffer(int16* buffer, const int numSamples) override;

	bool isStereo() const override { return false; }
	bool endOfData() const override { return _endOfData; }

	int getRate() const override { return _rate; }

	bool seek(const Audio::Timestamp& where) override;
	Audio::Timestamp getLength() const override { return _length; }
private:
	Common::SeekableReadStream* _stream;
	uint32 _streamStart;
	bool _endOfData;
	int _rate;
	uint _processedSize;
	uint _totalSize;
	Audio::Timestamp _length;

	int _bytesLeft;

	byte* _outBuffer;
	int _outBufferOffset;
	uint _outBufferSize;

	byte* _inBuffer;
	uint _inBufferSize;

	int readChunk(int16* buffer, const int maxSamples);

	static const int8 WSTable2Bit[];
	static const int8 WSTable4Bit[];
};

const int8 AUDStream::WSTable2Bit[] = { -2, -1, 0, 1 };
const int8 AUDStream::WSTable4Bit[] = {
	-9, -8, -6, -5, -4, -3, -2, -1,
	 0,  1,  2,  3,  4,  5,  6,  8
};

AUDStream::AUDStream(Common::SeekableReadStream *stream) : _stream(stream), _endOfData(true), _rate(0),
	_processedSize(0), _totalSize(0), _length(0, 1), _bytesLeft(0), _outBuffer(0),
	_outBufferOffset(0), _outBufferSize(0), _inBuffer(0), _inBufferSize(0) {

	if (_stream->size() < 8) {
		warning("No AUD file: too short");
		return;
	}

	_rate = _stream->readUint16LE();
	_totalSize = _stream->readUint32LE();

	// TODO?: add checks
	int flags = _stream->readByte();	// flags
	int type = _stream->readByte();	// type

	_streamStart = stream->pos();

	debugC(5, kDebugLevelSound, "AUD Info: rate: %d, totalSize: %d, flags: %d, type: %d, streamStart: %d", _rate, _totalSize, flags, type, _streamStart);

	_length = Audio::Timestamp(0, _rate);
	for (uint32 i = 0; i < _totalSize;) {
		uint16 size = _stream->readUint16LE();
		uint16 outSize = _stream->readUint16LE();

		_length = _length.addFrames(outSize);
		stream->seek(size + 4, SEEK_CUR);
		i += size + 8;
	}

	stream->seek(_streamStart, SEEK_SET);

	if (type == 1 && !flags)
		_endOfData = false;
	else
		warning("No AUD file (rate: %d, size: %d, flags: 0x%X, type: %d)", _rate, _totalSize, flags, type);
}

AUDStream::~AUDStream() {
	delete[] _outBuffer;
	delete[] _inBuffer;
	delete _stream;
}

int AUDStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesRead = 0, samplesLeft = numSamples;

	while (samplesLeft > 0 && !_endOfData) {
		int samples = readChunk(buffer, samplesLeft);
		samplesRead += samples;
		samplesLeft -= samples;
		buffer += samples;
	}

	return samplesRead;
}

inline int16 clip8BitSample(int16 sample) {
	return CLIP<int16>(sample, 0, 255);
}

int AUDStream::readChunk(int16 *buffer, const int maxSamples) {
	int samplesProcessed = 0;

	// if no bytes of the old chunk are left, read the next one
	if (_bytesLeft <= 0) {
		if (_processedSize >= _totalSize) {
			_endOfData = true;
			return 0;
		}

		uint16 size = _stream->readUint16LE();
		uint16 outSize = _stream->readUint16LE();
		uint32 id = _stream->readUint32LE();

		assert(id == 0x0000DEAF);

		_processedSize += 8 + size;

		_outBufferOffset = 0;
		if (size == outSize) {
			if (outSize > _outBufferSize) {
				_outBufferSize = outSize;
				delete[] _outBuffer;
				_outBuffer = new uint8[_outBufferSize];
				assert(_outBuffer);
			}

			_bytesLeft = size;

			_stream->read(_outBuffer, _bytesLeft);
		} else {
			_bytesLeft = outSize;

			if (outSize > _outBufferSize) {
				_outBufferSize = outSize;
				delete[] _outBuffer;
				_outBuffer = new uint8[_outBufferSize];
				assert(_outBuffer);
			}

			if (size > _inBufferSize) {
				_inBufferSize = size;
				delete[] _inBuffer;
				_inBuffer = new uint8[_inBufferSize];
				assert(_inBuffer);
			}

			if (_stream->read(_inBuffer, size) != size) {
				_endOfData = true;
				return 0;
			}

			int16 curSample = 0x80;
			byte code = 0;
			int8 count = 0;
			uint16 input = 0;
			int j = 0;
			int i = 0;

			while (outSize > 0) {
				input = _inBuffer[i++] << 2;
				code = (input >> 8) & 0xFF;
				count = (input & 0xFF) >> 2;

				switch (code) {
				case 2:
					if (count & 0x20) {
						/* NOTE: count is signed! */
						count <<= 3;
						curSample += (count >> 3);
						_outBuffer[j++] = curSample & 0xFF;
						outSize--;
					} else {
						for (; count >= 0; count--) {
							_outBuffer[j++] = _inBuffer[i++];
							outSize--;
						}
						curSample = _inBuffer[i - 1];
					}
					break;
				case 1:
					for (; count >= 0; count--) {
						code = _inBuffer[i++];

						curSample += WSTable4Bit[code & 0x0F];
						curSample = clip8BitSample(curSample);
						_outBuffer[j++] = curSample;

						curSample += WSTable4Bit[code >> 4];
						curSample = clip8BitSample(curSample);
						_outBuffer[j++] = curSample;

						outSize -= 2;
					}
					break;
				case 0:
					for (; count >= 0; count--) {
						code = (uint8)_inBuffer[i++];

						curSample += WSTable2Bit[code & 0x03];
						curSample = clip8BitSample(curSample);
						_outBuffer[j++] = curSample & 0xFF;

						curSample += WSTable2Bit[(code >> 2) & 0x03];
						curSample = clip8BitSample(curSample);
						_outBuffer[j++] = curSample & 0xFF;

						curSample += WSTable2Bit[(code >> 4) & 0x03];
						curSample = clip8BitSample(curSample);
						_outBuffer[j++] = curSample & 0xFF;

						curSample += WSTable2Bit[(code >> 6) & 0x03];
						curSample = clip8BitSample(curSample);
						_outBuffer[j++] = curSample & 0xFF;

						outSize -= 4;
					}
					break;
				default:
					for (; count >= 0; count--) {
						_outBuffer[j++] = curSample & 0xFF;
						outSize--;
					}
				}
			}
		}
	}

	// copies the chunk data to the output buffer
	if (_bytesLeft > 0) {
		int samples = MIN(_bytesLeft, maxSamples);
		samplesProcessed += samples;
		_bytesLeft -= samples;

		while (samples--) {
			int16 sample = (_outBuffer[_outBufferOffset++] << 8) ^ 0x8000;

			*buffer++ = sample;
		}
	}

	return samplesProcessed;
}

bool AUDStream::seek(const Audio::Timestamp &where) {
	const uint32 seekSample = Audio::convertTimeToStreamPos(where, getRate(), isStereo()).totalNumberOfFrames();

	_stream->seek(_streamStart);
	_processedSize = 0;
	_bytesLeft = 0;
	_endOfData = false;

	uint32 curSample = 0;

	while (!endOfData()) {
		uint16 size = _stream->readUint16LE();
		uint16 outSize = _stream->readUint16LE();

		if (curSample + outSize > seekSample) {
			_stream->seek(-4, SEEK_CUR);

			uint32 samples = seekSample - curSample;
			int16 *temp = new int16[samples];
			assert(temp);

			readChunk(temp, samples);
			delete[] temp;
			curSample += samples;
			break;
		} else {
			curSample += outSize;
			_processedSize += 8 + size;
			_stream->seek(size + 4, SEEK_CUR);
		}
	}

	_endOfData = (_processedSize >= _totalSize);

	return (curSample == seekSample);
}

Audio::SeekableAudioStream* makeAUDStream(Common::SeekableReadStream* stream, DisposeAfterUse::Flag disposeAfterUse) {
	return new AUDStream(stream);
}

} // End of namespace Kyra
