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

#include "dune/detection.h"

#include "dune/dune.h"
#include "dune/graphics.h"
#include "dune/hsq.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
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
	stopAudio();

	delete[] _decodeBuffer;
	_decodeBuffer = nullptr;

	delete[] _frameBuffer;
	_frameBuffer = nullptr;
}


/**
 * Reads past the VOC header a movie's sound opens with and reports the rate
 * its samples were taken at.  Returns how many bytes came before the samples,
 * or zero if what is there is not a VOC file after all.
 */
static uint32 parseVOCHeader(const byte *data, uint32 size, uint &rate) {
	static const char *kSignature = "Creative Voice File\x1a";
	const uint32 signatureLength = 20;

	if (size < 32 || memcmp(data, kSignature, signatureLength) != 0)
		return 0;

	const uint16 headerSize = READ_LE_UINT16(data + 20);
	if (headerSize + 6u > size || data[headerSize] != 1)
		return 0;

	// The rate is kept as the divisor the DOS driver loaded its timer with.
	const byte timeConstant = data[headerSize + 4];
	rate = 1000000 / (256 - timeConstant);

	return headerSize + 6;
}

/**
 * Ends the sound of a movie.  Each movie's sound is its own stream, taken at
 * its own rate and opening with its own VOC header, so one must be finished
 * and let go before the next begins; keeping one stream for all of them ran
 * the second movie's samples out at the first one's rate and left the sound
 * of a movie the player had cut short still playing over what came after it.
 */
void HnmPlayer::stopAudio() {
	if (_audioIsStarted) {
		_vm->_mixer->stopHandle(_audioHandle);
		_audioIsStarted = false;
	}

	_audioQueue = nullptr;
}

void HnmPlayer::setReader(Resource reader) {
	stopAudio();

	// The words Irulan speaks over the desert belong to her movie alone.  They
	// were left set when the movie was let go of, so cutting her short left
	// them being drawn over everything that followed her.
	_subtitleFrames = nullptr;
	_subtitleCurrentPart = 0;

	_reader = reader;
	// Loop playback reuses the same Resource after its final frame. Reset
	// the stream as well as the frame index before start rereads its header.
	if (_reader)
		_reader->seek(0);
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
	// A caller may have drawn a scene/overlay since the previous movie frame.
	// Every delta (and subtitle-only frame) belongs to this decoder's page,
	// never whichever drawing destination that intervening scene installed.
	_vm->_graphics.setFrameBuffer(_frameBuffer);
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
				_vm->drawSprite(_subtitleCurrentPart / 2, 0, 190);
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

				uint32 length = size - 4;
				byte *audioBuffer = (byte *)malloc(length);
				_reader->read(audioBuffer, length);

				// The sound of a movie is one VOC file cut across its frames,
				// so the first slice opens with that file's header.  Play its
				// samples and not the header, and take the rate it names.
				if (!_audioQueue) {
					uint rate = 11111;
					const uint32 skipped = parseVOCHeader(audioBuffer, length, rate);

					if (skipped) {
						memmove(audioBuffer, audioBuffer + skipped, length - skipped);
						length -= skipped;
					}

					debugC(1, kDebugVideo, "movie sound: %d Hz", rate);
					_audioQueue = Audio::makeQueuingAudioStream(rate, false);
				}

				_audioQueue->queueBuffer(audioBuffer, length, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

				// A movie carries one frame's worth of sound in each of its
				// frames, so starting to play the moment the first arrives
				// leaves the sound a hair's breadth ahead of the decoding for
				// the whole movie, and every frame that takes a moment longer
				// than its share empties the queue and is heard as a blip.  A
				// few frames' worth is let build up first.
				const uint kAhead = 4;

				if (!_audioIsStarted && _audioQueue->numQueuedStreams() >= kAhead) {
					_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_audioHandle, _audioQueue);
					_audioIsStarted = true;
				}

				g_framesPlayed += 1;
				g_bytesPlayed += length;

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

				debugC(2, kDebugVideo, "frame %d: %dx%d flags %02x mode %02x", _currentFrame,
					   _frameHeader.width, _frameHeader.height, _frameHeader.flags, _frameHeader.mode);

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

				assert(sum == 0xab);
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
		// The movies of the book are kept at half the screen's resolution and
		// are drawn every other pixel and every other line, which is the
		// pattern of black squares the originals show them through.
		for (int y = 0; y < h; ++y) {
			const int row = 2 * y + y_offset;
			if (row > 199)
				break;

			const byte *src = _decodeBuffer + y * w + src_offset;
			byte *dst = _frameBuffer + 320 * row + x_offset;

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
