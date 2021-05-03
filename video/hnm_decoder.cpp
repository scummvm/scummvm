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

#include "common/debug.h"
#include "common/endian.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "audio/decoders/raw.h"

#include "video/hnm_decoder.h"
#include "image/codecs/hlz.h"

namespace Video {

// When no sound display a frame every 80ms
HNMDecoder::HNMDecoder(bool loop, byte *initialPalette) : _regularFrameDelay(80),
	_videoTrack(nullptr), _audioTrack(nullptr), _stream(nullptr),
	_loop(loop), _initialPalette(initialPalette) {
}

HNMDecoder::~HNMDecoder() {
	close();

	delete[] _initialPalette;

	// We don't deallocate _videoTrack and _audioTrack as they are owned by base class
}

bool HNMDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	uint32 tag = stream->readUint32BE();

	/* For now, only HNM4, HNM6 in the future */
	if (tag != MKTAG('H', 'N', 'M', '4')) {
		close();
		return false;
	}

	//uint32 ukn = stream->readUint32BE();
	stream->skip(4);
	uint16 width = stream->readUint16LE();
	uint16 height = stream->readUint16LE();
	//uint32 filesize = stream->readUint32LE();
	stream->skip(4);
	uint32 frameCount = stream->readUint32LE();
	//uint32 tabOffset = stream->readUint32LE();
	stream->skip(4);
	uint16 soundBits = stream->readUint16LE();
	uint16 soundFormat = stream->readUint16LE();
	uint32 frameSize = stream->readUint32LE();

	byte unknownStr[16];
	byte copyright[16];
	stream->read(unknownStr, sizeof(unknownStr));
	stream->read(copyright, sizeof(copyright));

	if (_loop) {
		// This will force loop mode
		frameCount = 0;
	}

	_videoTrack = new HNM4VideoTrack(width, height, frameSize, frameCount, _regularFrameDelay,
	                                 _initialPalette);
	addTrack(_videoTrack);
	if (soundFormat == 2 && soundBits != 0) {
		// HNM4 is 22050Hz
		_audioTrack = new DPCMAudioTrack(soundFormat, soundBits, 22050, getSoundType());
		addTrack(_audioTrack);
	} else {
		_audioTrack = nullptr;
	}

	_stream = stream;

	return true;
}

void HNMDecoder::close() {
	VideoDecoder::close();
	// Tracks are cleant by VideoDecoder::close
	_videoTrack = nullptr;
	_audioTrack = nullptr;

	delete _stream;
	_stream = nullptr;
}

void HNMDecoder::readNextPacket() {
	// We are called to feed a frame
	// Each chunk is packetized and a packet seems to contain only one frame
	uint32 superchunkRemaining = _stream->readUint32LE();
	if (!superchunkRemaining) {
		if (!_loop) {
			error("End of file but still requesting data");
		} else {
			// Looping: read back from start of file, skip header and read a new super chunk header
			_videoTrack->restart();
			_stream->seek(64, SEEK_SET);
			superchunkRemaining = _stream->readUint32LE();
		}
	}
	superchunkRemaining = (superchunkRemaining & 0x00ffffff) - 4;

	while (superchunkRemaining) {
		uint32 chunkSize = _stream->readUint32LE();
		uint16 chunkType = _stream->readUint16BE();
		uint16 flags     = _stream->readUint16LE();

		if (chunkType == MKTAG16('P', 'L')) {
			_videoTrack->decodePalette(_stream, chunkSize - 8);
		} else if (chunkType == MKTAG16('I', 'Z')) {
			_stream->skip(4);
			_videoTrack->decodeIntraframe(_stream, chunkSize - 8 - 4);
			_videoTrack->presentFrame(flags);
		} else if (chunkType == MKTAG16('I', 'U')) {
			if ((flags & 1) == 1) {
				_videoTrack->decodeInterframeA(_stream, chunkSize - 8);
			} else {
				_videoTrack->decodeInterframe(_stream, chunkSize - 8);
			}
			_videoTrack->presentFrame(flags);
		} else if (chunkType == MKTAG16('S', 'D')) {
			if (_audioTrack) {
				Audio::Timestamp duration = _audioTrack->decodeSound(_stream, chunkSize - 8);
				_videoTrack->setFrameDelay(duration.msecs());
			} else {
				warning("Got audio data without an audio track");
			}
		} else {
			error("Got %d chunk: size %d", chunkType, chunkSize);
		}

		superchunkRemaining -= chunkSize;
	}
}

HNMDecoder::HNM4VideoTrack::HNM4VideoTrack(uint32 width, uint32 height, uint32 frameSize,
		uint32 frameCount, uint32 regularFrameDelay, const byte *initialPalette) :
	_frameCount(frameCount), _regularFrameDelay(regularFrameDelay), _nextFrameStartTime(0) {

	restart();

	_curFrame = -1;
	// Get the currently loaded palette for undefined colors
	if (initialPalette) {
		memcpy(_palette, initialPalette, 256 * 3);
	} else {
		memset(_palette, 0, 256 * 3);
	}
	_dirtyPalette = true;

	if (width * height > frameSize) {
		error("Invalid frameSize: expected %d, got %d", width * height, frameSize);
	}

	_frameBufferF = new byte[frameSize];
	memset(_frameBufferF, 0, frameSize);
	_frameBufferC = new byte[frameSize];
	memset(_frameBufferC, 0, frameSize);
	_frameBufferP = new byte[frameSize];
	memset(_frameBufferP, 0, frameSize);

	// We will use _frameBufferF/C/P as the surface pixels, just init there with nullptr to avoid unintended usage of surface
	const Graphics::PixelFormat &f = Graphics::PixelFormat::createFormatCLUT8();
	_surface.init(width, height, width * f.bytesPerPixel, nullptr, f);
}

HNMDecoder::HNM4VideoTrack::~HNM4VideoTrack() {
	// Don't free _surface as we didn't used create() but init()
	delete[] _frameBufferF;
	_frameBufferF = nullptr;
	delete[] _frameBufferC;
	_frameBufferC = nullptr;
	delete[] _frameBufferP;
	_frameBufferP = nullptr;
}

void HNMDecoder::HNM4VideoTrack::setFrameDelay(uint32 frameDelay) {
	if (_nextFrameDelay == uint32(-1)) {
		_nextFrameDelay = frameDelay;
	} else if (_nextNextFrameDelay == uint32(-1)) {
		_nextNextFrameDelay = frameDelay;
	} else {
		_nextNextFrameDelay += frameDelay;
	}
}


void HNMDecoder::HNM4VideoTrack::decodePalette(Common::SeekableReadStream *stream, uint32 size) {
	while (true) {
		if (size < 2) {
			break;
		}
		uint start = stream->readByte();
		uint count = stream->readByte();
		size -= 2;

		if (start == 255 && count == 255) {
			break;
		}
		if (count == 0) {
			count = 256;
		}

		if (size < count * 3) {
			error("Invalid palette chunk data");
		}
		if (start + count > 256) {
			error("Invalid palette start/count values");
		}

		size -= count * 3;
		byte *palette_ptr = &_palette[start * 3];
		for (; count > 0; count--) {
			byte r = stream->readByte();
			byte g = stream->readByte();
			byte b = stream->readByte();
			*(palette_ptr++) = r * 4;
			*(palette_ptr++) = g * 4;
			*(palette_ptr++) = b * 4;
		}
	}
	_dirtyPalette = true;

	if (size > 0) {
		stream->skip(size);
	}
}

void HNMDecoder::HNM4VideoTrack::decodeInterframe(Common::SeekableReadStream *stream, uint32 size) {
	SWAP(_frameBufferC, _frameBufferP);

	uint16 width = _surface.w;
	bool eop = false;

	uint currentPos = 0;

	while (!eop) {
		if (size < 1) {
			warning("Not enough data in chunk for interframe block");
			break;
		}
		byte countFlgs = stream->readByte();
		size -= 1;
		byte count = countFlgs & 0x1f;
		byte flgs = (countFlgs >> 5) & 0x7;

		if (count == 0) {
			int c, fill;
			switch (flgs) {
			case 0:
				if (size < 2) {
					error("Not enough data for case 0");
				}
				// Copy next two bytes of input to the output
				c = stream->readByte();
				_frameBufferC[currentPos++] = c;
				c = stream->readByte();
				_frameBufferC[currentPos++] = c;
				size -= 2;
				break;
			case 1:
				if (size < 1) {
					error("Not enough data for case 1");
				}
				// Skip (next byte of input) * 2 bytes of output
				c = stream->readByte() * 2;
				currentPos += c;
				size -= 1;
				break;
			case 2:
				if (size < 2) {
					error("Not enough data for case 2");
				}
				// Skip (next word of input) * 2 bytes of output
				c = stream->readUint16LE() * 2;
				currentPos += c;
				size -= 2;
				break;
			case 3:
				if (size < 2) {
					error("Not enough data for case 3");
				}
				// Fill (next byte of input) * 2 of output with (next byte of input)
				c = stream->readByte() * 2;
				fill = stream->readByte();
				memset(&_frameBufferC[currentPos], fill, c);
				currentPos += c;
				size -= 2;
				break;
			default:
				// End of picture
				eop = true;
				break;
			}
		} else {
			if (size < 2) {
				error("Not enough data for count > 0");
			}

			bool backward = (flgs & 0x4) != 0;
			bool backline = (flgs & 0x2) != 0;
			bool previous = (flgs & 0x1) != 0;
			int offset = stream->readUint16LE();
			bool swap = (offset & 0x1) != 0;
			size -= 2;

			offset = currentPos + (offset & 0xFFFE) - 0x8000;
			if (offset < 0) {
				error("Invalid offset");
			}

			byte *ptr;
			if (previous) {
				ptr = _frameBufferP;
			} else {
				ptr = _frameBufferC;
			}

			int shft1, shft2;
			if (backline) {
				const int twolinesabove = -(width * 2);
				shft1 = twolinesabove + 1;
				shft2 = 0;
			} else {
				shft1 = 0;
				shft2 = 1;
			}
			if (swap)
				SWAP(shft1, shft2);

			int src_inc = backward ? -2 : 2;

			while (count--) {
				byte b0 = ptr[offset + shft1];
				byte b1 = ptr[offset + shft2];
				_frameBufferC[currentPos++] = b0;
				_frameBufferC[currentPos++] = b1;

				offset += src_inc;
			}
		}
	}
	if (size > 0) {
		stream->skip(size);
	}
}

void HNMDecoder::HNM4VideoTrack::decodeInterframeA(Common::SeekableReadStream *stream, uint32 size) {
	SWAP(_frameBufferC, _frameBufferP);

	uint16 width = _surface.w;
	bool eop = false;

	uint currentPos = 0;

	while (!eop) {
		if (size < 1) {
			warning("Not enough data in chunk for interframe block");
			break;
		}
		byte countFlgs = stream->readByte();
		size -= 1;
		byte count = countFlgs & 0x3f;
		byte flgs = (countFlgs >> 6) & 0x3;

		if (count == 0) {
			byte c;
			switch (flgs) {
			case 0:
				if (size < 1) {
					error("Not enough data for case 0");
				}
				// Move in image
				c = stream->readByte();
				currentPos += c;
				size -= 1;
				break;
			case 1:
				if (size < 1) {
					error("Not enough data for case 1");
				}
				// New pixels
				c = stream->readByte();
				_frameBufferC[currentPos] = c;
				c = stream->readByte();
				_frameBufferC[currentPos + width] = c;
				currentPos++;
				size -= 2;
				break;
			case 2:
				// New line
				currentPos += width;
				break;
			case 3:
				// End of picture
				eop = true;
				break;
			default:
				error("BUG: Shouldn't be here");
				break;
			}
		} else {
			if (size < 2) {
				error("Not enough data for count > 0");
			}

			bool negative = (flgs & 0x2) != 0;
			bool previous = (flgs & 0x1) != 0;
			int offset = stream->readUint16LE();
			size -= 2;

			if (negative) {
				offset -= 0x10000;
			}
			offset += currentPos;
			if (offset < 0) {
				error("Invalid offset");
			}

			byte *ptr;
			if (previous) {
				ptr = _frameBufferP;
			} else {
				ptr = _frameBufferC;
			}
			for (; count > 0; count--) {
				_frameBufferC[currentPos] = ptr[offset];
				_frameBufferC[currentPos + width] = ptr[offset + width];
				currentPos++;
				offset++;
			}
		}
	}

	if (size > 0) {
		stream->skip(size);
	}
}

void HNMDecoder::HNM4VideoTrack::decodeIntraframe(Common::SeekableReadStream *stream, uint32 size) {
	Image::HLZDecoder::decodeFrameInPlace(*stream, size, _frameBufferC);
	memcpy(_frameBufferP, _frameBufferC, (uint)_surface.w * (uint)_surface.h);
}

void HNMDecoder::HNM4VideoTrack::presentFrame(uint16 flags) {
	int width = _surface.w;
	int height = _surface.h;

	if ((flags & 1) == 1) {
		// High resolution HNM4A: no deinterlacing, use current image directly
		_surface.setPixels(_frameBufferC);
	} else if ((width % 4) == 0) {
		// HNM4: deinterlacing must not alter the framebuffer as it will get reused as previous source for next frame
		uint32 *input = (uint32 *)_frameBufferC;
		uint32 *line0 = (uint32 *)_frameBufferF;
		uint32 *line1 = (uint32 *)(_frameBufferF + width);
		int count = (height) / 2;
		while (count--) {
			int i;
			for (i = 0; i < width / 4; i++) {
				uint32 p0 = *input++;
				uint32 p4 = *input++;

#ifndef SCUMM_LITTLE_ENDIAN
				*line0++ = ((p4 & 0xFF00) >> 8) | ((p4 & 0xFF000000) >> 16) | ((p0 & 0xFF00) << 8) | (p0 & 0xFF000000);
				*line1++ = ((p0 & 0xFF0000) << 8) | ((p0 & 0xFF) << 16) | ((p4 & 0xFF0000) >> 8) | (p4 & 0xFF);
#else
				*line0++ = (p0 & 0xFF) | ((p0 & 0xFF0000) >> 8) | ((p4 & 0xFF) << 16) | ((p4 & 0xFF0000) << 8);
				*line1++ = ((p0 & 0xFF00) >> 8) | ((p0 & 0xFF000000) >> 16) | ((p4 & 0xFF00) << 8) | (p4 & 0xFF000000);
#endif
			}
			line0 += width / 4;
			line1 += width / 4;
		}
		_surface.setPixels(_frameBufferF);
	} else {
		error("HNMDecoder::HNM4VideoTrack::postprocess(%x): Unexpected width: %d", flags, width);
	}

	// Frame done
	_curFrame++;
	_nextFrameStartTime += _nextFrameDelay != uint32(-1) ? _nextFrameDelay : _regularFrameDelay;
	_nextFrameDelay = _nextNextFrameDelay;
	_nextNextFrameDelay = uint32(-1);
}

HNMDecoder::DPCMAudioTrack::DPCMAudioTrack(uint16 format, uint16 bits, uint sampleRate,
        Audio::Mixer::SoundType soundType) : AudioTrack(soundType), _audioStream(nullptr),
	_gotLUT(false), _lastSample(0) {
	if (bits != 16) {
		error("Unsupported audio bits");
	}
	if (format != 2) {
		warning("Unsupported %d audio format", format);
	}
	// Format 2 is Mono 16-bits DPCM
	_audioStream = Audio::makeQueuingAudioStream(sampleRate, false);
}

HNMDecoder::DPCMAudioTrack::~DPCMAudioTrack() {
	delete _audioStream;
}

Audio::Timestamp HNMDecoder::DPCMAudioTrack::decodeSound(Common::SeekableReadStream *stream,
		uint32 size) {
	if (!_gotLUT) {
		if (size < 256 * sizeof(*_lut)) {
			error("Invalid first sound chunk");
		}
		stream->read(_lut, 256 * sizeof(*_lut));
		size -= 256 * sizeof(*_lut);
#ifndef SCUMM_LITTLE_ENDIAN
		for (uint i = 0; i < 256; i++) {
			_lut[i] = FROM_LE_16(_lut[i]);
		}
#endif
		_gotLUT = true;
	}

	if (size > 0) {
		uint16 *out = (uint16 *)malloc(size * sizeof(*out));
		uint16 *p = out;
		uint16 sample = _lastSample;
		for (uint32 i = 0; i < size; i++, p++) {
			byte deltaId = stream->readByte();
			sample += _lut[deltaId];
			*p = sample;
		}
		_lastSample = sample;

		byte flags = Audio::FLAG_16BITS;
#ifdef SCUMM_LITTLE_ENDIAN
		flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
		_audioStream->queueBuffer((byte *)out, size * sizeof(*out), DisposeAfterUse::YES, flags);
	}
	return Audio::Timestamp(0, size, 22050);
}

} // End of namespace Video
