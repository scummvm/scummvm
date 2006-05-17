/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "sound/audiostream.h"

namespace Kyra {

// Thanks to Torbjörn Andersson (eriktorbjorn) for his aud player on which
// this code is based on

// TODO: cleanup of whole AUDStream
// FIXME: sound 'stutters' a bit, maybe a problem while converting int8 samples to int16?
class AUDStream : public Audio::AudioStream {
public:
	AUDStream(Common::File *file, bool loop = false);
	~AUDStream();
	
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const { return false; }
	bool endOfData() const { return _endOfData; }
	
	int getRate() const { return _rate; }
private:
	Common::File *_file;
	bool _loop;
	uint32 _loopStart;
	bool _endOfData;
	int _rate;
	uint _processedSize;
	uint _totalSize;
	
	int _bytesLeft;
	
	uint8 *_outBuffer;
	int _outBufferOffset;
	uint _outBufferSize;
	
	uint8 *_inBuffer;
	uint _inBufferSize;
	
	int readChunk(int16 *buffer, const int maxSamples);
	
	static const int8 WSTable2Bit[];
	static const int8 WSTable4Bit[];
};

const int8 AUDStream::WSTable2Bit[] = { -2, -1, 0, 1 };
const int8 AUDStream::WSTable4Bit[] = {
	-9, -8, -6, -5, -4, -3, -2, -1,
	 0,  1,  2,  3,  4,  5,  6,  8
};

AUDStream::AUDStream(Common::File *file, bool loop) : _file(0), _endOfData(true), _rate(0),
	_processedSize(0), _totalSize(0), _bytesLeft(0), _outBuffer(0),
	_outBufferOffset(0), _outBufferSize(0), _inBuffer(0), _inBufferSize(0) {
#if defined(__SYMBIAN32__)
	// Symbian can't share filehandles between different threads.
	// So create a new file and seek that to the other filehandle's
	// position
	_file = new File;
	_file->open(file->name());
	_file->seek(file->pos());
#else
	_file = file;
#endif
	_file->incRef();
	
	_rate = _file->readUint16LE();
	_totalSize = _file->readUint32LE();
	_loop = loop;
	// TODO?: add checks
	int flags = _file->readByte();	// flags
	int type = _file->readByte();	// type

	_loopStart = file->pos();
	
	if (type == 1 && !flags) {
		_endOfData = false;
	} else
		warning("No AUD file (rate: %d, size: %d, flags: 0x%X, type: %d)", _rate, _totalSize, flags, type);
}

AUDStream::~AUDStream() {
	delete [] _outBuffer;
	delete [] _inBuffer;

	if (_file)
		_file->decRef();
#ifdef __SYMBIAN32__
	delete _file;
#endif
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
				_file->seek(_loopStart);
				_processedSize = 0;
			} else {
				_endOfData = true;
				return 0;
			}
		}

		uint16 size = _file->readUint16LE();
		uint16 outSize = _file->readUint16LE();
		uint32 id = _file->readUint32LE();
		
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
			
			_file->read(_outBuffer, _bytesLeft);
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
			
			if (_file->read(_inBuffer, size) != size) {
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

		while (samples--) {
			int16 sample = (int8)_outBuffer[_outBufferOffset++];
			*buffer++ = (sample << 8) ^ 0x8000;
		}
	}

	return samplesProcessed;
}

#pragma mark -

SoundDigital::SoundDigital(KyraEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer), _sounds() {}

SoundDigital::~SoundDigital() {
	for (int i = 0; i < SOUND_STREAMS; ++i) {
		stopSound(i);
	}
}

bool SoundDigital::init() {
	return true;
}

int SoundDigital::playSound(Common::File *fileHandle, bool loop, int channel) {
	Sound *use = 0;
	if (channel != -1 && channel < SOUND_STREAMS) {
		stopSound(channel);
		use = &_sounds[channel];
	} else {
		for (int i = 0; i < SOUND_STREAMS; ++i) {
			if (!_sounds[i].fileHandle) {
				use = &_sounds[i];
				break;
			}
		}
		
		if (!use) {
			warning("no free sound channel");
			return -1;
		}
	}
	
	Audio::AudioStream *stream = new AUDStream(fileHandle, loop);
	if (stream->endOfData()) {
		delete stream;
		delete fileHandle;

		return -1;
	}
	
	// TODO: set correct sound type from channel id
	_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &use->handle, stream);
	use->fileHandle = fileHandle;
	
	return use - _sounds;
}

bool SoundDigital::isPlaying(int channel) {
	if (channel == -1)
		return false;

	assert(channel >= 0 && channel < SOUND_STREAMS);

	return _mixer->isSoundHandleActive(_sounds[channel].handle);
}

void SoundDigital::stopSound(int channel) {
	if (isPlaying(channel)) {
		_mixer->stopHandle(_sounds[channel].handle);
	}
	
	if (_sounds[channel].fileHandle) {
		delete _sounds[channel].fileHandle;
		_sounds[channel].fileHandle = 0;			
	}
}

} // end of namespace Kyra
