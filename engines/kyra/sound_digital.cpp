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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/sound.h"
#include "kyra/resource.h"

#include "sound/audiostream.h"

namespace Kyra {

// Thanks to Torbjorn Andersson (eriktorbjorn) for his aud player on which
// this code is based on

// TODO: cleanup of whole AUDStream

class AUDStream : public Audio::AudioStream {
public:
	AUDStream(Common::SeekableReadStream *stream, bool loop = false);
	~AUDStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const { return false; }
	bool endOfData() const { return _endOfData; }

	int getRate() const { return _rate; }

	void beginFadeIn(uint32 millis);
	void beginFadeOut(uint32 millis);
private:
	Common::SeekableReadStream *_stream;
	bool _loop;
	uint32 _loopStart;
	bool _endOfData;
	int _rate;
	uint _processedSize;
	uint _totalSize;

	int _bytesLeft;

	byte *_outBuffer;
	int _outBufferOffset;
	uint _outBufferSize;

	byte *_inBuffer;
	uint _inBufferSize;

	int32 _fadeSamples;
	int32 _fadeCount;
	int _fading;

	int readChunk(int16 *buffer, const int maxSamples);

	static const int8 WSTable2Bit[];
	static const int8 WSTable4Bit[];
};

const int8 AUDStream::WSTable2Bit[] = { -2, -1, 0, 1 };
const int8 AUDStream::WSTable4Bit[] = {
	-9, -8, -6, -5, -4, -3, -2, -1,
	 0,  1,  2,  3,  4,  5,  6,  8
};

AUDStream::AUDStream(Common::SeekableReadStream *stream, bool loop) : _stream(stream), _endOfData(true), _rate(0),
	_processedSize(0), _totalSize(0), _bytesLeft(0), _outBuffer(0),
	_outBufferOffset(0), _outBufferSize(0), _inBuffer(0), _inBufferSize(0) {

	_rate = _stream->readUint16LE();
	_totalSize = _stream->readUint32LE();
	_loop = loop;

	_fadeSamples = 0;
	_fading = 0;

	// TODO?: add checks
	int flags = _stream->readByte();	// flags
	int type = _stream->readByte();	// type

	_loopStart = stream->pos();

	if (type == 1 && !flags)
		_endOfData = false;
	else
		warning("No AUD file (rate: %d, size: %d, flags: 0x%X, type: %d)", _rate, _totalSize, flags, type);
}

AUDStream::~AUDStream() {
	delete [] _outBuffer;
	delete [] _inBuffer;
	delete _stream;
}

void AUDStream::beginFadeIn(uint32 millis) {
	_fadeSamples = (millis * getRate()) / 1000;
	if (_fading == 0)
		_fadeCount = 0;
	_fading = 1;
}

void AUDStream::beginFadeOut(uint32 millis) {
	_fadeSamples = (millis * getRate()) / 1000;
	if (_fading == 0)
		_fadeCount = _fadeSamples;
	_fading = -1;
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
	if (sample > 255)
		return 255;
	if (sample < 0)
		return 0;
	return sample;
}

int AUDStream::readChunk(int16 *buffer, const int maxSamples) {
	int samplesProcessed = 0;

	// if no bytes of the old chunk are left, read the next one
	if (_bytesLeft <= 0) {
		if (_processedSize >= _totalSize) {
			if (_loop) {
				_stream->seek(_loopStart);
				_processedSize = 0;
			} else {
				_endOfData = true;
				return 0;
			}
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
				delete [] _outBuffer;
				_outBuffer = new uint8[_outBufferSize];
				assert(_outBuffer);
			}

			_bytesLeft = size;

			_stream->read(_outBuffer, _bytesLeft);
		} else {
			_bytesLeft = outSize;

			if (outSize > _outBufferSize) {
				_outBufferSize = outSize;
				delete [] _outBuffer;
				_outBuffer = new uint8[_outBufferSize];
				assert(_outBuffer);
			}

			if (size > _inBufferSize) {
				_inBufferSize = size;
				delete [] _inBuffer;
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
				code = (input >> 8) & 0xff;
				count = (input & 0xff) >> 2;

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

						curSample += WSTable4Bit[code & 0x0f];
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

		// To help avoid overflows for long fade times, we divide both
		// _fadeSamples and _fadeCount when calculating the new sample.

		int32 div = _fadeSamples / 256;

		while (samples--) {
			int16 sample = (_outBuffer[_outBufferOffset++] << 8) ^ 0x8000;

			if (_fading) {
				sample = (sample * (_fadeCount / 256)) / div;
				_fadeCount += _fading;

				if (_fadeCount < 0) {
					_fadeCount = 0;
					_endOfData = true;
				} else if (_fadeCount > _fadeSamples) {
					_fadeCount = _fadeSamples;
					_fading = 0;
				}
			}

			*buffer++ = sample;
		}
	}

	if (_fading < 0 && _fadeCount == 0)
		_fading = 0;

	return samplesProcessed;
}

#pragma mark -

SoundDigital::SoundDigital(KyraEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer), _sounds() {
	for (uint i = 0; i < ARRAYSIZE(_sounds); ++i)
		_sounds[i].stream = 0;
}

SoundDigital::~SoundDigital() {
	for (int i = 0; i < ARRAYSIZE(_sounds); ++i)
		stopSound(i);
}

int SoundDigital::playSound(const char *filename, uint8 priority, Audio::Mixer::SoundType type, int volume, bool loop, int channel) {
	Sound *use = 0;
	if (channel != -1 && channel < ARRAYSIZE(_sounds)) {
		stopSound(channel);
		use = &_sounds[channel];
	} else {
		for (channel = 0; !use && channel < ARRAYSIZE(_sounds); ++channel) {
			if (!isPlaying(channel)) {
				stopSound(channel);
				use = &_sounds[channel];
				break;
			}
		}

		for (channel = 0; !use && channel < ARRAYSIZE(_sounds); ++channel) {
			if (strcmp(_sounds[channel].filename, filename) == 0) {
				stopSound(channel);
				use = &_sounds[channel];
				break;
			}
		}

		for (channel = 0; !use && channel < ARRAYSIZE(_sounds); ++channel) {
			if (_sounds[channel].priority <= priority) {
				stopSound(channel);
				use = &_sounds[channel];
				break;
			}
		}

		if (!use) {
			warning("no free sound channel");
			return -1;
		}
	}

	Common::SeekableReadStream *stream = _vm->resource()->getFileStream(filename);
	if (!stream)
		return -1;

	strncpy(use->filename, filename, sizeof(use->filename));
	use->priority = priority;
	use->stream = new AUDStream(stream, loop);
	if (use->stream->endOfData()) {
		delete use->stream;
		use->stream = 0;

		return -1;
	}

	if (volume > 255)
		volume = 255;
	volume = (volume * Audio::Mixer::kMaxChannelVolume) / 255;

	_mixer->playInputStream(type, &use->handle, use->stream, -1, volume);

	return use - _sounds;
}

bool SoundDigital::isPlaying(int channel) {
	if (channel == -1)
		return false;

	assert(channel >= 0 && channel < ARRAYSIZE(_sounds));

	if (!_sounds[channel].stream)
		return false;

	return _mixer->isSoundHandleActive(_sounds[channel].handle);
}

void SoundDigital::stopSound(int channel) {
	assert(channel >= 0 && channel < ARRAYSIZE(_sounds));
	_mixer->stopHandle(_sounds[channel].handle);
	_sounds[channel].stream = 0;
}

void SoundDigital::stopAllSounds() {
	for (int i = 0; i < ARRAYSIZE(_sounds); ++i) {
		if (isPlaying(i))
			stopSound(i);
	}
}

void SoundDigital::beginFadeOut(int channel, int ticks) {
	if (isPlaying(channel))
		_sounds[channel].stream->beginFadeOut(ticks * _vm->tickLength());
}

} // end of namespace Kyra

