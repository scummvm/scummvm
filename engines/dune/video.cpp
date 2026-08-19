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

#include "dune/video.h"

#include "dune/dune.h"
#include "dune/graphics.h"
#include "dune/hsq.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/palette.h"

namespace Dune {

#define MAX_DECODE_BUFFER_SIZE 64000

HnmPlayer::HnmPlayer(DuneEngine *vm) : _vm(vm) {
	_reader = nullptr;
	_done = false;
	_currentFrame = 0;
	_frameCount = 0;
	_frameBuffer = new byte[320*200];
	_decodeBufferSize = 0;
	_decodeBuffer = new byte[MAX_DECODE_BUFFER_SIZE];
	_interlace = false;
	_subtitleFrames = nullptr;
	_audioQueue = nullptr;
	_audioIsStarted = false;
}

HnmPlayer::~HnmPlayer() {
	if (_audioIsStarted) {
		_audioQueue->finish();
	}
	_audioQueue = nullptr;

	delete[] _decodeBuffer;
	_decodeBuffer = nullptr;

	delete[] _frameBuffer;
	_frameBuffer = nullptr;
}

void HnmPlayer::setReader(Resource reader) {
	_reader = reader;
	_currentFrame = 0;

	_done = false;
	_decodeBufferSize = 0;
	_interlace = false;
}

void HnmPlayer::start() {
	_headerSize = _reader->readUint16LE();

	_vm->applyPaletteBlock(_reader.get());

	// Skip filler 0xff's after palette
	while (_reader->readByte() == 0xff)
		;
	_reader->seek(-1, SEEK_CUR);

	_frameCount = (_headerSize - _reader->pos()) / 4 - 1;

	_frameOffsets.clear();
	_frameOffsets.reserve(_frameCount + 1);
	for (int i = 0; i != _frameCount+1; ++i) {
		_frameOffsets.push_back(_reader->readUint32LE());

	}

#if 0
	for (int i = 0; i != _frameCount; ++i) {
		debug("frame %4d: %08x + %08x", i, _frameOffsets[i] + _headerSize, _frameOffsets[i+1] - _frameOffsets[i]);
	}
#endif

	memset(_frameBuffer, 0, 320*200);

	// The blitter draws wherever the graphics driver points; while a movie runs
	// that is the movie's own frame buffer.
	_vm->_graphics.setFrameBuffer(_frameBuffer);
}

static void readFrameHeader(Common::ReadStream *r, int *w, int *h, byte *flags, byte *mode) {
	byte b[4];
	r->read(b, 4);

	/*
	 * | w7 w6 w5 w4 w3 w2 w1 w0 | f6 f5 f4 f3 f2 f1 f0 w8 | h7 h6 h5 h4 h3 h2 h1 h0 | m7 m6 m5 m4 m3 m2 m1 m0 |
	 */

	*w     = ((0x1 & b[1]) << 8) | b[0];
	*flags = b[1] & 0xfe;
	*h     = b[2];
	*mode  = b[3];
}

static void readFrameHeader(uint16_t tag, Common::ReadStream *r, int *w, int *h, byte *flags, byte *mode) {
	byte b[4];
	b[0] = tag & 0xff;
	b[1] = tag >> 8;
	r->read(b+2, 2);

	*w     = ((0x1 & b[1]) << 8) | b[0];
	*flags = b[1] & 0xfe;
	*h     = b[2];
	*mode  = b[3];
}

void HnmPlayer::decodeAVFrame() {
	_reader->seek(_frameOffsets[_currentFrame] + _headerSize, SEEK_SET);

	uint32 avFrameSize = _reader->readUint16LE();
	assert(avFrameSize == _frameOffsets[_currentFrame+1] - _frameOffsets[_currentFrame]);

	decodeAVFrameChunks();
	decodeVideoFrame();

	if (_subtitleFrames) {
		const int16 frame = READ_LE_INT16(_subtitleFrames + 2 * _subtitleCurrentPart);
		if (frame >= 0 && _currentFrame >= frame) {
			if (_subtitleCurrentPart % 2) {
				// Clear subtitle area
				int y_offset = 190;
				for (int y = 0; y != 10; ++y) {
					byte *dst = _frameBuffer + 320*(y+y_offset);
					for (int x = 0; x != 320; ++x) {
						*dst++ = 0;
					}
				}
			} else {
				// Draw subtitle image
				_vm->openSpriteSheet("IRUL1.HSQ");
				_vm->drawSprite(_subtitleCurrentPart / 2, 0, 190, 0x00);
			}
			_subtitleCurrentPart++;
		}
	}

	_currentFrame += 1;
	if (_currentFrame == _frameCount) {
		_done = 1;
	}
}

int g_framesPlayed = 0;
int g_bytesPlayed = 0;

void HnmPlayer::decodeAVFrameChunks() {
	uint16 size, tag, chunkPos;

	for (;;) {
		chunkPos = _reader->pos();

		tag = _reader->readUint16LE();
		size = 0;

		if (_reader->eos()) {
			_done = true;
			return;
		}

		switch (tag) {
			case 0x6c70: // pl
			{
				size = _reader->readUint16LE();
				int pos = _reader->pos();
				Common::ReadStream *palReader = new Common::SubReadStream(_reader.get(), size - 4);
				_vm->applyPaletteBlock(palReader);
				delete palReader;
				_reader->seek(pos + size - 4, SEEK_SET);
				break;
			}
			case 0x6473: // sd
			{
				size = _reader->readUint16LE();
				assert(size >= 4);

				if (!_audioQueue) {
					_audioQueue = Audio::makeQueuingAudioStream(11111, false);
				}

				byte *audioBuffer = (byte *)malloc(size - 4);
				_reader->read(audioBuffer, size - 4);

				_audioQueue->queueBuffer(audioBuffer, size - 4, DisposeAfterUse::YES, 1);

				if (!_audioIsStarted) {
					_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, nullptr, _audioQueue);
					_audioIsStarted = true;
				}

				g_framesPlayed += 1;
				g_bytesPlayed += size - 4;

				// debug("%3d %d / %d", size - 4, g_bytesPlayed, g_framesPlayed);
				break;
			}
			case 0x6d6d: // mm
			{
				debug("MM block found: %d bytes", size - 4);
				size = _reader->readUint16LE();
				_reader->skip(size - 4);
				break;
			}
			default: // video frame
			{
				size = _frameOffsets[_currentFrame+1] + _headerSize - chunkPos;
				assert(size >= 4);

				readFrameHeader(tag, _reader.get(),
					&_frameHeader.width,
					&_frameHeader.height,
					&_frameHeader.flags,
					&_frameHeader.mode);

				if ((_frameHeader.flags & 0x04) == 0) {
					// Original has tag == 0 which causes it to decompress invalid data
					if (_frameHeader.width == 0) {
						_decodeBufferSize = 0;
						return;
					}

					if ((_frameHeader.flags & 0x02) == 0) {
						// Uncompressed data
						_decodeBufferSize = size - 4;
						assert(_decodeBufferSize <= MAX_DECODE_BUFFER_SIZE);
						_reader->read(_decodeBuffer, _decodeBufferSize);
						return;
					}
				}

				int  unpackedLength = _reader->readUint16LE();
				byte zero           = _reader->readByte();
				int  packedLength   = _reader->readUint16LE();
				int  checksum       = _reader->readByte();

				(void)zero;
				(void)checksum;

#if 1
				(void)zero;
				assert(zero == 0);
				byte sum = (unpackedLength >> 8)
				         + (unpackedLength & 0xff)
				         + (packedLength >> 8)
				         + (packedLength & 0xff)
				         + checksum;

				assert(sum = 0xab);
#endif

				assert(unpackedLength <= MAX_DECODE_BUFFER_SIZE);
				_decodeBufferSize = unpackedLength;

				decompressHSQ(_reader.get(), packedLength, _decodeBuffer, _decodeBufferSize);
				return;
			}
		}
	}
}

void HnmPlayer::decodeVideoFrame() {
	if (_decodeBufferSize == 0) {
		return;
	}
	assert(_decodeBufferSize >= 4);

	int x_offset = 0;
	int y_offset = 0;
	int src_offset = 0;

	if ((_frameHeader.flags & 0x04) == 0) {
		x_offset += READ_LE_UINT16(_decodeBuffer+0);
		y_offset += READ_LE_UINT16(_decodeBuffer+2);
		src_offset = 4;
	}

	int w = _frameHeader.width;
	int h = _frameHeader.height;
	byte flags = _frameHeader.flags;
	byte mode = _frameHeader.mode;

	if (_interlace) {
		for (int y = 0; y < h; ++y) {
			byte *src = _decodeBuffer + y*w + src_offset;
			byte *dst = _frameBuffer  + 320*(2*y+y_offset) + x_offset;
			for (int x = 0; x < w; ++x) {
				*dst++ = *src++;
				*dst++ = 0;
			}
		}
	} else {
		_vm->_graphics.blitGraphics(
			_decodeBuffer + src_offset, _decodeBufferSize - src_offset,
			x_offset, y_offset,
			w, h,
			flags,
			mode
		);
	}
}

} // namespace Dune
