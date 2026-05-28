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

#include "common/debug.h"
#include "common/endian.h"
#include "common/system.h"
#include "common/memstream.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "audio/decoders/raw.h"
#include "audio/decoders/apc.h"

#include "image/codecs/hlz.h"
#include "image/codecs/hnm.h"

#include "video/hnm_decoder.h"

namespace Video {

HNMDecoder::HNMDecoder(const Graphics::PixelFormat &format, bool loop,
                       byte *initialPalette) : _regularFrameDelayMs(uint32(-1)),
	_videoTrack(nullptr), _audioTrack(nullptr), _stream(nullptr), _format(format),
	_loop(loop), _initialPalette(initialPalette), _alignedChunks(false),
	_dataBuffer(nullptr), _dataBufferAlloc(0) {
	if (initialPalette && format.bytesPerPixel >= 2) {
		error("Invalid pixel format while initial palette is set");
	}
}

HNMDecoder::~HNMDecoder() {
	close();

	delete[] _initialPalette;

	// We don't deallocate _videoTrack and _audioTrack as they are owned by base class
}

bool HNMDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	// Take the ownership of stream immediately
	_stream = stream;

	uint32 tag = stream->readUint32BE();

	/* All HNM versions are supported */
	if (tag != MKTAG('H', 'N', 'M', '4') &&
	    tag != MKTAG('U', 'B', 'B', '2') &&
	    tag != MKTAG('H', 'N', 'M', '6')) {
		close();
		return false;
	}

	byte audioflags = 0;
	uint16 soundBits = 0, soundFormat = 0;
	if (tag == MKTAG('H', 'N', 'M', '6')) {
		//uint16 ukn6_1 = stream->readUint16LE();
		stream->skip(2);
		audioflags = stream->readByte();
		//byte bpp = stream->readByte();
		stream->skip(1);
	} else {
		//uint32 ukn = stream->readUint32BE();
		stream->skip(4);
	}
	uint16 width = stream->readUint16LE();
	uint16 height = stream->readUint16LE();
	//uint32 filesize = stream->readUint32LE();
	stream->skip(4);
	uint32 frameCount = stream->readUint32LE();
	//uint32 tabOffset = stream->readUint32LE();
	stream->skip(4);
	if (tag == MKTAG('H', 'N', 'M', '4') ||
	    tag == MKTAG('U', 'B', 'B', '2')) {
		soundBits = stream->readUint16LE();
		soundFormat = stream->readUint16LE();
	} else {
		//uint16 ukn6_2 = stream->readUint16LE();
		//uint16 ukn6_3 = stream->readUint16LE();
		stream->skip(4);
	}
	uint32 frameSize = stream->readUint32LE();

	byte unknownStr[16];
	byte copyright[16];
	stream->read(unknownStr, sizeof(unknownStr));
	stream->read(copyright, sizeof(copyright));

	if (_loop) {
		// This will force loop mode
		frameCount = 0;
	}

	// When no audio use a factor of 1 for audio timestamp
	uint32 audioSampleRate = 1;
	// When no sound display a frame every X ms if not overriden
	if (_regularFrameDelayMs == uint32(-1)) {
		if (tag == MKTAG('H', 'N', 'M', '4') ||
		    tag == MKTAG('U', 'B', 'B', '2')) {
			// For HNM4&5 it's every 80 ms
			_regularFrameDelayMs = 80;
		} else if (tag == MKTAG('H', 'N', 'M', '6')) {
			// For HNM6 it's every 66 ms
			_regularFrameDelayMs = 66;
		}
	}

	_videoTrack = nullptr;
	_audioTrack = nullptr;
	if (tag == MKTAG('H', 'N', 'M', '4')) {
		if (_format.bytesPerPixel >= 2) {
			// Bad constructor used
			close();
			return false;
		}
		if (soundFormat == 2 && soundBits != 0) {
			// HNM4 is Mono 22050Hz
			_audioTrack = new DPCMAudioTrack(soundFormat, soundBits, 22050, false, getSoundType());
			audioSampleRate = 22050;
		}
		_videoTrack = new HNM4VideoTrack(width, height, frameSize, frameCount,
		                                 _regularFrameDelayMs, audioSampleRate,
		                                 _initialPalette);
	} else if (tag == MKTAG('U', 'B', 'B', '2')) {
		if (_format.bytesPerPixel >= 2) {
			// Bad constructor used
			close();
			return false;
		}
		if (soundFormat == 2 && soundBits == 0) {
			// UBB2 is Stereo 22050Hz
			_audioTrack = new DPCMAudioTrack(soundFormat, 16, 22050, true, getSoundType());
			audioSampleRate = 22050;
		}
		_videoTrack = new HNM5VideoTrack(width, height, frameSize, frameCount,
		                                 _regularFrameDelayMs, audioSampleRate,
		                                 _initialPalette);
	} else if (tag == MKTAG('H', 'N', 'M', '6')) {
		if (_format.bytesPerPixel < 2) {
			// Bad constructor used or bad format given
			close();
			return false;
		}
		_alignedChunks = true;
		if (audioflags & 1) {
			byte stereo = (audioflags >> 7) & 1;
			audioSampleRate = ((audioflags >> 4) & 6) * 11025;
			_audioTrack = new APCAudioTrack(audioSampleRate, stereo, getSoundType());
		}
		// Peek at the first video chunk to determine if this is a warp-mode (IW) file.
		// Warp mode uses a keyframe-only panorama decoder and is mutually exclusive with
		// video mode (which allocates a second buffer for inter-frame delta decoding).
		bool hnm6WarpMode = false;
		{
			int64 savedPos = _stream->pos();
			uint32 scSize = _stream->readUint32LE() & 0x00ffffff;
			uint32 scRemaining = scSize >= 4 ? scSize - 4 : 0;
			while (scRemaining >= 8) {
				uint32 cSize = _stream->readUint32LE();
				uint16 cType = _stream->readUint16BE();
				_stream->skip(2); // flags
				if (cType == MKTAG16('I', 'W')) {
					hnm6WarpMode = true;
					break;
				} else if (cType == MKTAG16('I', 'X')) {
					break;
				}
				// Skip non-video chunks (audio etc.)
				uint32 alignedSize = ((cSize + 3) / 4) * 4;
				if (alignedSize < 8) break;
				_stream->skip(alignedSize - 8);
				scRemaining -= MIN(alignedSize, scRemaining);
			}
			_stream->seek(savedPos);
		}
		_videoTrack = new HNM6VideoTrack(width, height, frameSize, frameCount,
		                                 _regularFrameDelayMs, audioSampleRate,
		                                 _format, hnm6WarpMode);
	} else {
		// We should never be here
		close();
		return false;
	}
	addTrack(_videoTrack);
	if (_audioTrack) {
		addTrack(_audioTrack);
	}

	return true;
}

void HNMDecoder::close() {
	VideoDecoder::close();
	// Tracks are cleant by VideoDecoder::close
	_videoTrack = nullptr;
	_audioTrack = nullptr;

	delete _stream;
	_stream = nullptr;

	delete[] _dataBuffer;
	_dataBuffer = nullptr;
	_dataBufferAlloc = 0;
}

void HNMDecoder::readNextPacket() {
	// We are called to feed a frame
	// Each chunk is packetized and a packet seems to contain only one frame
	uint32 superchunkRemaining = _stream->readUint32LE();
	if (!superchunkRemaining || _stream->eos()) {
		if (!_loop) {
			error("End of file but still requesting data");
		} else {
			// Looping: read back from start of file, skip header and read a new super chunk header
			_videoTrack->restart();
			_stream->seek(64, SEEK_SET);
			superchunkRemaining = _stream->readUint32LE();
		}
	}
	superchunkRemaining = superchunkRemaining & 0x00ffffff;
	if (superchunkRemaining < 4) {
		error("Invalid superchunk header");
	}
	superchunkRemaining -= 4;

	if (_dataBufferAlloc < superchunkRemaining) {
		delete[] _dataBuffer;
		_dataBuffer = new byte[superchunkRemaining];
		_dataBufferAlloc = superchunkRemaining;
	}
	if (_stream->read(_dataBuffer, superchunkRemaining) != superchunkRemaining) {
		if (!_loop) {
			error("Not enough data in file");
		}
		// UBB2 files may not have a zero-terminator at EOF; treat as end of stream and loop.
		_videoTrack->restart();
		_stream->seek(64, SEEK_SET);
		superchunkRemaining = _stream->readUint32LE() & 0x00ffffff;
		if (superchunkRemaining < 4) {
			error("Invalid superchunk header after loop restart");
		}
		superchunkRemaining -= 4;
		if (_dataBufferAlloc < superchunkRemaining) {
			delete[] _dataBuffer;
			_dataBuffer = new byte[superchunkRemaining];
			_dataBufferAlloc = superchunkRemaining;
		}
		if (_stream->read(_dataBuffer, superchunkRemaining) != superchunkRemaining) {
			error("Not enough data in file after loop restart");
		}
	}

	// We use -1 here to discrimate a possibly empty sound frame
	uint32 audioNumSamples = uint32(-1);

	byte *data_p = _dataBuffer;
	while (superchunkRemaining > 0) {
		if (superchunkRemaining < 8) {
			error("Not enough data in superchunk");
		}

		uint32 chunkSize = READ_LE_UINT32(data_p);
		data_p += sizeof(uint32);
		uint16 chunkType = READ_BE_UINT16(data_p);
		data_p += sizeof(uint16);
		uint16 flags     = READ_LE_UINT16(data_p);
		data_p += sizeof(uint16);

		if (superchunkRemaining < chunkSize) {
			error("Chunk has a bogus size");
		}

		debug(5, "HNM chunk: type='%c%c' (0x%04x) size=%u",
		      (char)(chunkType >> 8), (char)(chunkType & 0xff), chunkType, chunkSize);
		if (chunkType == MKTAG16('S', 'D') ||
		    chunkType == MKTAG16('A', 'A') ||
		    chunkType == MKTAG16('B', 'B')) {
			if (_audioTrack) {
				audioNumSamples = _audioTrack->decodeSound(chunkType, data_p, chunkSize - 8);
			} else {
				warning("Got audio data without an audio track");
			}
		} else {
			_videoTrack->decodeChunk(data_p, chunkSize - 8, chunkType, flags);
		}

		if (_alignedChunks) {
			chunkSize = ((chunkSize + 3) / 4) * 4;
		}

		data_p += (chunkSize - 8);
		superchunkRemaining -= chunkSize;
	}
	_videoTrack->newFrame(audioNumSamples);
}

HNMDecoder::HNMVideoTrack::HNMVideoTrack(uint32 frameCount,
        uint32 regularFrameDelayMs, uint32 audioSampleRate) :
	_frameCount(frameCount), _curFrame(-1), _regularFrameDelayMs(regularFrameDelayMs),
	_nextFrameStartTime(0, audioSampleRate) {
	restart();
}

HNMDecoder::HNM45VideoTrack::HNM45VideoTrack(uint32 width, uint32 height, uint32 frameSize,
        uint32 frameCount, uint32 regularFrameDelayMs, uint32 audioSampleRate,
        const byte *initialPalette) :
	HNMVideoTrack(frameCount, regularFrameDelayMs, audioSampleRate), _palette(256) {

	// Get the currently loaded palette for undefined colors
	if (initialPalette) {
		_palette.set(initialPalette, 0, 256);
	}
	_dirtyPalette = true;

	if (width * height > frameSize) {
		error("Invalid frameSize: expected %d, got %d", width * height, frameSize);
	}

	_frameBufferC = new byte[frameSize]();
	_frameBufferP = new byte[frameSize]();

	// We will use _frameBufferC/P as the surface pixels, just init there with nullptr to avoid unintended usage of surface
	const Graphics::PixelFormat &f = Graphics::PixelFormat::createFormatCLUT8();
	_surface.init(width, height, width * f.bytesPerPixel, nullptr, f);
}

HNMDecoder::HNM45VideoTrack::~HNM45VideoTrack() {
	// Don't free _surface as we didn't used create() but init()
	delete[] _frameBufferC;
	_frameBufferC = nullptr;
	delete[] _frameBufferP;
	_frameBufferP = nullptr;
}

void HNMDecoder::HNM45VideoTrack::newFrame(uint32 frameDelay) {
	// Frame done
	_curFrame++;

	// Add regular frame delay if we had no sound
	// We can't rely on a detection in the header as some soundless HNM indicate they have
	if (frameDelay == uint32(-1)) {
		_nextFrameStartTime = _nextFrameStartTime.addMsecs(_regularFrameDelayMs);
		return;
	}

	// HNM decoders use sound double buffering to pace the frames
	// First frame is loaded in first buffer, second frame in second buffer
	// It's only for third frame that we wait for the first buffer to be free
	// It's presentation time is then delayed by the number of sound samples in the first frame
	if (_lastFrameDelaySamps) {
		_nextFrameStartTime = _nextFrameStartTime.addFrames(_lastFrameDelaySamps);
	}
	_lastFrameDelaySamps = frameDelay;
}

void HNMDecoder::HNM45VideoTrack::decodePalette(byte *data, uint32 size) {
	while (true) {
		if (size < 2) {
			break;
		}
		uint start = *(data++);
		uint count = *(data++);
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

		if (size < count * 3) {
			error("Not enough data for palette");
		}

		for (int i = start; count > 0; i++, count--) {
			byte r = *(data++);
			byte g = *(data++);
			byte b = *(data++);
			_palette.set(i, r * 4, g * 4, b * 4);
		}
		size -= count * 3;
	}
	_dirtyPalette = true;
}

HNMDecoder::HNM4VideoTrack::HNM4VideoTrack(uint32 width, uint32 height, uint32 frameSize,
        uint32 frameCount, uint32 regularFrameDelayMs, uint32 audioSampleRate,
        const byte *initialPalette) :
	HNM45VideoTrack(width, height, frameSize, frameCount,
	                regularFrameDelayMs, audioSampleRate, initialPalette) {

	_frameBufferF = new byte[frameSize]();
}

HNMDecoder::HNM4VideoTrack::~HNM4VideoTrack() {
	// Don't free _surface as we didn't used create() but init()
	delete[] _frameBufferF;
	_frameBufferF = nullptr;
}

void HNMDecoder::HNM4VideoTrack::decodeChunk(byte *data, uint32 size,
        uint16 chunkType, uint16 flags) {
	if (chunkType == MKTAG16('P', 'L')) {
		decodePalette(data, size);
	} else if (chunkType == MKTAG16('I', 'Z')) {
		if (size < 4) {
			error("Not enough data for IZ");
		}
		decodeIntraframe(data + 4, size - 4);
		presentFrame(flags);
	} else if (chunkType == MKTAG16('I', 'U')) {
		if ((flags & 1) == 1) {
			decodeInterframeA(data, size);
		} else {
			decodeInterframe(data, size);
		}
		presentFrame(flags);
	} else {
		error("HNM4: Got %d chunk: size %d", chunkType, size);
	}
}

void HNMDecoder::HNM4VideoTrack::decodeInterframe(byte *data, uint32 size) {
	SWAP(_frameBufferC, _frameBufferP);

	uint16 width = _surface.w;
	bool eop = false;

	uint32 currentPos = 0;

	while (!eop) {
		if (size < 1) {
			warning("Not enough data in chunk for interframe block");
			break;
		}
		byte countFlgs = *(data++);
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
				c = *(data++);
				_frameBufferC[currentPos++] = c;
				c = *(data++);
				_frameBufferC[currentPos++] = c;
				size -= 2;
				break;
			case 1:
				if (size < 1) {
					error("Not enough data for case 1");
				}
				// Skip (next byte of input) * 2 bytes of output
				c = *(data++) * 2;
				currentPos += c;
				size -= 1;
				break;
			case 2:
				if (size < 2) {
					error("Not enough data for case 2");
				}
				// Skip (next word of input) * 2 bytes of output
				c = READ_LE_UINT16(data) * 2;
				data += 2;
				currentPos += c;
				size -= 2;
				break;
			case 3:
				if (size < 2) {
					error("Not enough data for case 3");
				}
				// Fill (next byte of input) * 2 of output with (next byte of input)
				c = *(data++) * 2;
				fill = *(data++);
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
			int offset = READ_LE_UINT16(data);
			data += 2;
			size -= 2;
			bool swap = (offset & 0x1) != 0;

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
			if (swap) {
				SWAP(shft1, shft2);
			}

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
}

void HNMDecoder::HNM4VideoTrack::decodeInterframeA(byte *data, uint32 size) {
	SWAP(_frameBufferC, _frameBufferP);

	uint16 width = _surface.w;
	bool eop = false;

	uint32 currentPos = 0;

	while (!eop) {
		if (size < 1) {
			warning("Not enough data in chunk for interframe block");
			break;
		}
		byte countFlgs = *(data++);
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
				c = *(data++);
				currentPos += c;
				size -= 1;
				break;
			case 1:
				if (size < 2) {
					error("Not enough data for case 1");
				}
				// New pixels
				c = *(data++);
				_frameBufferC[currentPos] = c;
				c = *(data++);
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
			int offset = READ_LE_UINT16(data);
			data += 2;
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
			memcpy(&_frameBufferC[currentPos], &ptr[offset], count);
			memcpy(&_frameBufferC[currentPos + width], &ptr[offset + width], count);
			currentPos += count;
		}
	}
}

void HNMDecoder::HNM4VideoTrack::decodeIntraframe(byte *data, uint32 size) {
	Common::MemoryReadStream stream(data, size);
	Image::HLZDecoder::decodeFrameInPlace(stream, size, _frameBufferC);
	memcpy(_frameBufferP, _frameBufferC, (uint32)_surface.w * (uint32)_surface.h);
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
				*line0++ = ((p4 & 0xFF00) >> 8) | ((p4 & 0xFF000000) >> 16) |
				           ((p0 & 0xFF00) << 8) | (p0 & 0xFF000000);
				*line1++ = ((p0 & 0xFF0000) << 8) | ((p0 & 0xFF) << 16) |
				           ((p4 & 0xFF0000) >> 8) | (p4 & 0xFF);
#else
				*line0++ = (p0 & 0xFF) | ((p0 & 0xFF0000) >> 8) |
				           ((p4 & 0xFF) << 16) | ((p4 & 0xFF0000) << 8);
				*line1++ = ((p0 & 0xFF00) >> 8) | ((p0 & 0xFF000000) >> 16) |
				           ((p4 & 0xFF00) << 8) | (p4 & 0xFF000000);
#endif
			}
			line0 += width / 4;
			line1 += width / 4;
		}
		_surface.setPixels(_frameBufferF);
	} else {
		error("HNMDecoder::HNM4VideoTrack::postprocess(%x): Unexpected width: %d", flags, width);
	}
}

void HNMDecoder::HNM5VideoTrack::decodeChunk(byte *data, uint32 size,
        uint16 chunkType, uint16 flags) {
	if (chunkType == MKTAG16('P', 'L')) {
		decodePalette(data, size);
	} else if (chunkType == MKTAG16('I', 'V')) {
		if (_dialogCodec)
			decodeFrameAtlantisDialog(data, size);
		else
			decodeFrame(data, size);
	} else if (chunkType == MKTAG16('I', 'A')) {
		// IA chunk in Atlantis dialog UBBs carries the FUN_00450b08
		// compositor opcode stream (SKIP/RUN/RUN-shaded), NOT inline
		// audio.  When opcode capture is enabled, stash the payload
		// verbatim for the engine to walk after the frame is decoded.
		// Other HNM consumers leave _captureOpcodes false and the
		// chunk is silently discarded as before.
		if (_captureOpcodes) {
			if (_opcodesAlloc < size) {
				delete[] _opcodes;
				_opcodes = new byte[size];
				_opcodesAlloc = size;
			}
			memcpy(_opcodes, data, size);
			_opcodesSize = size;
		}
	} else {
		warning("HNM5: Unknown chunk type 0x%04x: size %d, skipping", chunkType, size);
	}
}

void HNMDecoder::setDialogCodec(bool b) {
	if (HNM5VideoTrack *v5 = dynamic_cast<HNM5VideoTrack *>(_videoTrack))
		v5->setDialogCodec(b);
}

void HNMDecoder::setCaptureOpcodeStream(bool b) {
	if (HNM5VideoTrack *v5 = dynamic_cast<HNM5VideoTrack *>(_videoTrack))
		v5->setCaptureOpcodes(b);
}

const byte *HNMDecoder::getOpcodeStream(uint32 &size) const {
	if (HNM5VideoTrack *v5 = dynamic_cast<HNM5VideoTrack *>(_videoTrack))
		return v5->getOpcodes(size);
	size = 0;
	return nullptr;
}

static inline byte *HNM5_getSourcePtr(byte *&data, uint32 &size,
                                      byte *previous, byte *current, int16 pitch, byte currentMode) {
	int32 offset;
	byte offb;

#define HNM5_DECODE_OFFSET_CST(src, constant_off) \
    offset = constant_off + READ_LE_UINT16(data); \
    data += 2; \
    size -= 2; \
    return (src + offset)
#define HNM5_DECODE_OFFSET(src, xbits, xbase, ybase) \
    offb = *(data++); \
    size -= 1; \
    offset = ((offb >> xbits) + ybase) * pitch + \
        (offb & ((1 << xbits) - 1)) + xbase; \
    return (src + offset)

	switch (currentMode) {
	case  2:
		HNM5_DECODE_OFFSET_CST(previous, -32768);
	case  3:
		HNM5_DECODE_OFFSET_CST(previous, -32768);
	case  4:
		HNM5_DECODE_OFFSET_CST(previous, -32768);
	case  5:
		HNM5_DECODE_OFFSET(previous, 4,  -8,  -8);
	case  6:
		HNM5_DECODE_OFFSET(previous, 4,  -8,  -8);
	case  7:
		HNM5_DECODE_OFFSET(previous, 4,  -8,  -8);
	case  8:
		HNM5_DECODE_OFFSET(previous, 4,  -2,  -8);
	case  9:
		HNM5_DECODE_OFFSET(previous, 4, -14,  -8);
	case 10:
		HNM5_DECODE_OFFSET(previous, 4,  -8,  -2);
	case 11:
		HNM5_DECODE_OFFSET(previous, 4,  -8, -14);
	case 12:
		HNM5_DECODE_OFFSET(previous, 4,  -2,  -2);
	case 13:
		HNM5_DECODE_OFFSET(previous, 4, -14,  -2);
	case 14:
		HNM5_DECODE_OFFSET(previous, 4,  -2, -14);
	case 15:
		HNM5_DECODE_OFFSET(previous, 4, -14, -14);
	case 16:
		HNM5_DECODE_OFFSET(previous, 4,  -2,  -8);
	case 17:
		HNM5_DECODE_OFFSET(previous, 4, -14,  -8);
	case 18:
		HNM5_DECODE_OFFSET(previous, 4,  -8,  -2);
	case 19:
		HNM5_DECODE_OFFSET(previous, 4,  -8, -14);
	case 20:
		HNM5_DECODE_OFFSET(previous, 4,  -2,  -2);
	case 21:
		HNM5_DECODE_OFFSET(previous, 4, -14,  -2);
	case 22:
		HNM5_DECODE_OFFSET(previous, 4,  -2, -14);
	case 23:
		HNM5_DECODE_OFFSET(previous, 4, -14, -14);
	case 24:
		HNM5_DECODE_OFFSET(previous, 4,  -2,  -8);
	case 25:
		HNM5_DECODE_OFFSET(previous, 4, -14,  -8);
	case 26:
		HNM5_DECODE_OFFSET(previous, 4,  -8,  -2);
	case 27:
		HNM5_DECODE_OFFSET(previous, 4,  -8, -14);
	case 28:
		HNM5_DECODE_OFFSET(previous, 4,  -2,  -2);
	case 29:
		HNM5_DECODE_OFFSET(previous, 4, -14,  -2);
	case 30:
		HNM5_DECODE_OFFSET(previous, 4,  -2, -14);
	case 31:
		HNM5_DECODE_OFFSET(previous, 4, -14, -14);
	case 32:
		HNM5_DECODE_OFFSET_CST(current, -65536);
	case 33:
		HNM5_DECODE_OFFSET(current, 5, -16,  -8);
	case 34:
		HNM5_DECODE_OFFSET(current, 4,  -8, -16);
	case 35:
		HNM5_DECODE_OFFSET(current, 4, -24, -16);
	case 36:
		HNM5_DECODE_OFFSET(current, 4,   8, -16);
	case 37:
		HNM5_DECODE_OFFSET(current, 3,  -4, -32);
	case 38:
		HNM5_DECODE_OFFSET(current, 3, -12, -32);
	case 39:
		HNM5_DECODE_OFFSET(current, 3,   4, -32);
	case 40:
		HNM5_DECODE_OFFSET_CST(current, -65536);
	case 41:
		HNM5_DECODE_OFFSET(current, 5, -16,  -8);
	case 42:
		HNM5_DECODE_OFFSET(current, 4,  -8, -16);
	case 43:
		HNM5_DECODE_OFFSET(current, 4, -24, -16);
	case 44:
		HNM5_DECODE_OFFSET(current, 4,   8, -16);
	case 45:
		HNM5_DECODE_OFFSET(current, 3,  -4, -32);
	case 46:
		HNM5_DECODE_OFFSET(current, 3, -12, -32);
	case 47:
		HNM5_DECODE_OFFSET(current, 3,   4, -32);
	default:
		error("BUG: Invalid offset mode");
	}

#undef HNM5_DECODE_OFFSET_CST
#undef HNM5_DECODE_OFFSET
}

static inline void HNM5_copy(byte *dst, byte *src, int16 pitch,
                             byte copyMode, byte width, byte height) {
	switch (copyMode) {
	case 0:
		// Copy
		for (; height > 0; height--) {
			memcpy(dst, src, width);
			dst += pitch;
			src += pitch;
		}
		break;
	case 1:
		// Horizontal reverse
		for (; height > 0; height--) {
			byte *dp = dst;
			byte *sp = src;
			for (byte col = width; col > 0; col--) {
				*(dp++) = *(sp--);
			}
			dst += pitch;
			src += pitch;
		}
		break;
	case 2:
		// Vertical reverse
		for (; height > 0; height--) {
			memcpy(dst, src, width);
			dst += pitch;
			src -= pitch;
		}
		break;
	case 3:
		// Horiz-Vert reverse
		for (; height > 0; height--) {
			byte *dp = dst;
			byte *sp = src;
			for (byte col = width; col > 0; col--) {
				*(dp++) = *(sp--);
			}
			dst += pitch;
			src -= pitch;
		}
		break;
	case 4:
		// Swap
		for (; height > 0; height--) {
			byte *dp = dst;
			byte *sp = src;
			for (byte col = width; col > 0; col--) {
				*dp = *sp;
				dp++;
				sp += pitch;
			}
			dst += pitch;
			src += 1;
		}
		break;
	case 5:
		// Swap Horiz-Reverse
		for (; height > 0; height--) {
			byte *dp = dst;
			byte *sp = src;
			for (byte col = width; col > 0; col--) {
				*dp = *sp;
				dp++;
				sp -= pitch;
			}
			dst += pitch;
			src += 1;
		}
		break;
	case 6:
		// Swap Vert-Reverse
		for (; height > 0; height--) {
			byte *dp = dst;
			byte *sp = src;
			for (byte col = width; col > 0; col--) {
				*dp = *sp;
				dp++;
				sp += pitch;
			}
			dst += pitch;
			src -= 1;
		}
		break;
	case 7:
		// Swap Vert-Reverse
		for (; height > 0; height--) {
			byte *dp = dst;
			byte *sp = src;
			for (byte col = width; col > 0; col--) {
				*dp = *sp;
				dp++;
				sp -= pitch;
			}
			dst += pitch;
			src -= 1;
		}
		break;
	default:
		error("BUG: Invalid copy mode");
		return;
	}
}

static const byte HNM5_WIDTHS[3][32] = {
	{
		 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
		18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 44, 48, 52, 56
	}, /* 2 */
	{
		 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
		17, 18, 19, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44
	}, /* 3 */
	{
		 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
		17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 28, 30, 32, 34, 36, 38
	}, /* 4 */
};

// Atlantis: The Lost Tale dialog video 'IV' codec (FUN_0041101f in atlantis.exe).
// All non-control tokens are 3 bytes: [count_code, u16lo, u16hi].
//   count = (count_code & 0x1f) + 1 bytes per row-pair.
// Mode 0x28 (keyframe): src = dst + u16off - 65536 (backward LZ in output buffer).
// All other modes: src = prev + (dst - curr) + u16off - 0x8000 (inter-frame ref).
// Strip control: [0xe0, next_mode] = advance 2 rows; [0xe0, 0x00, cnt, col] = RLE fill.
// Opcode 0x20: [0x20, count] = copy count+1 bytes from same position in prev frame.
void HNMDecoder::HNM5VideoTrack::decodeFrameAtlantisDialog(byte *data, uint32 size) {
	SWAP(_frameBufferC, _frameBufferP);

	if (size < 2) {
		_surface.setPixels(_frameBufferC);
		return;
	}

	int mode = data[1];
	debug(3, "AtlantisDialog IV: size=%u mode=0x%02x", size, mode);

	if (mode == 1) {
		_surface.setPixels(_frameBufferC);
		return;
	}

	byte *tokens  = data + 2;
	byte *tokEnd  = data + size;
	byte *curr    = _frameBufferC;
	byte *prev    = _frameBufferP;
	int   pitch   = _surface.pitch;
	int   frameBytes = pitch * _surface.h;
	byte *currEnd = curr + frameBytes;
	byte *prevEnd = prev + frameBytes;
	ptrdiff_t prevDiff = prev - curr;

	byte *dst       = curr;
	byte *stripBase = curr;
	bool  done      = false;

	// Copy n bytes from src rows N and N+1 to dst rows N and N+1.
	// sBuf/sBufEnd guard source reads; byte-by-byte loop supports LZ self-overlap.
	auto copy2 = [&](byte *d, byte *s, int n, byte *sBuf, byte *sBufEnd) {
		for (int i = 0; i < n; i++) {
			byte *s0 = s + i;
			byte *s1 = s + pitch + i;
			byte v0 = (s0 >= sBuf && s0 < sBufEnd) ? *s0 : 0;
			byte v1 = (s1 >= sBuf && s1 < sBufEnd) ? *s1 : 0;
			if (d + i < currEnd)         d[i]         = v0;
			if (d + pitch + i < currEnd) (d + pitch)[i] = v1;
		}
	};

	while (!done && tokens < tokEnd) {
		byte b0 = tokens[0];

		if (b0 == 0xe0) {
			if (tokens + 2 > tokEnd) break;
			byte b1 = tokens[1];
			if (b1 == 0) {
				if (tokens + 4 > tokEnd) break;
				int  cnt = tokens[2] + 1;
				byte col = tokens[3];
				tokens += 4;
				byte *d1 = dst + pitch;
				for (int i = 0; i < cnt; i++, dst++, d1++) {
					if (dst < currEnd) *dst = col;
					if (d1  < currEnd) *d1  = col;
				}
			} else {
				tokens    += 2;
				stripBase += 2 * pitch;
				dst        = stripBase;
				if (b1 == 1) done = true; else mode = b1;
			}
			continue;
		}

		if (b0 == 0x20) {
			if (tokens + 2 > tokEnd) break;
			int cnt = tokens[1] + 1;
			tokens += 2;
			byte *src = dst + prevDiff;
			copy2(dst, src, cnt, prev, prevEnd);
			dst += cnt;
			continue;
		}

		// 3-byte token: [count_code, u16lo, u16hi]
		if (tokens + 3 > tokEnd) break;
		uint16 u16off = READ_LE_UINT16(tokens + 1);
		int    cnt    = (b0 & 0x1f) + 1;
		tokens += 3;

		byte *src;
		byte *sBuf, *sBufEnd;
		if (mode == 0x28) {
			src  = dst + ((ptrdiff_t)(uint32)u16off - 65536);
			sBuf = curr; sBufEnd = currEnd;
			if (src < curr) src = curr;
		} else {
			src  = dst + prevDiff + ((ptrdiff_t)u16off - 0x8000);
			sBuf = prev; sBufEnd = prevEnd;
			if (src < prev) src = prev;
		}
		copy2(dst, src, cnt, sBuf, sBufEnd);
		dst += cnt;
	}

	_surface.setPixels(_frameBufferC);
}

void HNMDecoder::HNM5VideoTrack::decodeFrame(byte *data, uint32 size) {
	SWAP(_frameBufferC, _frameBufferP);

	uint16 pitch = _surface.pitch;
	bool eop = false;

	byte height = 0xff;
	byte currentMode = 0xff;
	uint32 currentPos = 0;

	while (!eop) {
		if (size < 1) {
			warning("Not enough data in chunk for frame block");
			break;
		}
		byte opcode = *(data++);
		size -= 1;

		if (opcode == 0x20) {
			assert(height != 0xff);
			if (size < 1) {
				error("Not enough data for opcode 0x20");
			}
			uint width = *(data++);
			size -= 1;
			width++;
			for (byte row = 0; row < height; row++) {
				memcpy(&_frameBufferC[currentPos + row * pitch], &_frameBufferP[currentPos + row * pitch], width);
			}
			currentPos += width;
		} else if (opcode == 0x60) {
			// Maximal pixels height is 4
			assert(height != byte(-1) && height <= 4);
			if (size < height) {
				error("Not enough data for opcode 0x60");
			}
			for (byte row = 0; row < height; row++) {
				_frameBufferC[currentPos + row * pitch] = *(data++);
			}
			size -= height;
			currentPos += 1;
		} else if (opcode == 0xA0) {
			assert(height != byte(-1));
			if (size < 1) {
				error("Not enough data for opcode 0x20");
			}
			uint width = *(data++);
			size -= 1;
			width += 2;

			if (size < height * width) {
				error("Not enough data for opcode 0xA0");
			}
			for (uint col = 0; col < width; col++) {
				for (byte row = 0; row < height; row++) {
					_frameBufferC[currentPos + row * pitch + col] = *(data++);
				}
			}
			size -= height * width;
			currentPos += width;
		} else if (opcode == 0xE0) {
			if (size < 1) {
				error("Not enough data for opcode 0xE0");
			}
			byte subop = *(data++);
			size -= 1;

			if (subop == 0x00) {
				assert(height != byte(-1));
				if (size < 2) {
					error("Not enough data for opcode 0xE0 0x00");
				}
				uint width = *(data++);
				byte px = *(data++);
				size -= 2;

				width += 1;

				for (byte row = 0; row < height; row++) {
					memset(&_frameBufferC[currentPos + row * pitch], px, width);
				}
				currentPos += width;
			} else if (subop == 0x01) {
				if (height != byte(-1)) {
					currentPos += (height - 1) * pitch;
				}

				eop = true;
			} else {
				// Reconfigure decoder at line start
				assert((currentPos % pitch) == 0);
				assert(subop < 48);

				if (height != byte(-1)) {
					currentPos += (height - 1) * pitch;
				}

				currentMode = subop;

				if        (( 8 <= subop && subop <= 15) ||
				           (32 <= subop && subop <= 39) ||
				           (subop == 2) || (subop == 5)) {
					height = 2;
				} else if ((16 <= subop && subop <= 23) ||
				           (40 <= subop && subop <= 47) ||
				           (subop == 3) || (subop == 6)) {
					height = 3;
				} else if ((24 <= subop && subop <= 31) ||
				           (subop == 4) || (subop == 7)) {
					height = 4;
				}

			}
		} else {
			assert(height != byte(-1));
			assert(2 <= height && height <= 4);
			byte index = opcode & 0x1f;
			byte copyMode = (opcode >> 5) & 0x7;
			byte width = HNM5_WIDTHS[height - 2][index];

			// HNM5_getSourcePtr can consume 1 byte but the data can not end like this so check for maximum
			if (size < 2) {
				error("Not enough data for opcode 0x%02X", opcode);
			}
			byte *src = HNM5_getSourcePtr(data, size, _frameBufferP, _frameBufferC, pitch, currentMode);

			HNM5_copy(_frameBufferC + currentPos, src + currentPos, pitch, copyMode, width, height);
			currentPos += width;
		}
	}

	_surface.setPixels(_frameBufferC);
}

HNMDecoder::HNM6VideoTrack::HNM6VideoTrack(uint32 width, uint32 height, uint32 frameSize,
        uint32 frameCount, uint32 regularFrameDelayMs, uint32 audioSampleRate,
        const Graphics::PixelFormat &format, bool warpMode) :
	HNMVideoTrack(frameCount, regularFrameDelayMs, audioSampleRate),
	_decoder(Image::createHNM6Decoder(width, height, format, frameSize, !warpMode)),
	_surface(nullptr) {
	if (warpMode)
		_decoder->setWarpMode(true);
}

HNMDecoder::HNM6VideoTrack::~HNM6VideoTrack() {
	delete _decoder;
}

uint16 HNMDecoder::HNM6VideoTrack::getWidth() const {
	return _decoder->getWidth();
}

uint16 HNMDecoder::HNM6VideoTrack::getHeight() const {
	return _decoder->getHeight();
}

Graphics::PixelFormat HNMDecoder::HNM6VideoTrack::getPixelFormat() const {
	return _decoder->getPixelFormat();
}

bool HNMDecoder::HNM6VideoTrack::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	return _decoder->setOutputPixelFormat(format);
}

void HNMDecoder::HNM6VideoTrack::decodeChunk(byte *data, uint32 size,
        uint16 chunkType, uint16 flags) {
	if (chunkType == MKTAG16('I', 'X') ||
	    chunkType == MKTAG16('I', 'W')) {
		// IW chunks use the warp (panorama) decoder; IX uses the standard decoder.
		_decoder->setWarpMode(chunkType == MKTAG16('I', 'W'));
		Common::MemoryReadStream stream(data, size);
		_surface = _decoder->decodeFrame(stream);
	} else {
		error("HNM6: Got %d chunk: size %d", chunkType, size);
	}
}

void HNMDecoder::HNM6VideoTrack::newFrame(uint32 frameDelay) {
	// Frame done
	_curFrame++;

	// In HNM6 first frame contains the sound for the 32 following frames (pre-buffering)
	// Then other frames contain constant size sound chunks except for the 32 last ones
	if (!_lastFrameDelaySamps) {
		// Add regular frame delay if we had no sound
		if (frameDelay == uint32(-1)) {
			_nextFrameStartTime = _nextFrameStartTime.addMsecs(_regularFrameDelayMs);
			return;
		}

		assert((frameDelay & 31) == 0);
		_lastFrameDelaySamps = frameDelay / 32;
		_nextFrameStartTime = _nextFrameStartTime.addFrames(_lastFrameDelaySamps);
	} else {
		if (frameDelay != uint32(-1)) {
			assert(frameDelay == _lastFrameDelaySamps);
		}
		_nextFrameStartTime = _nextFrameStartTime.addFrames(_lastFrameDelaySamps);
	}
}

HNMDecoder::DPCMAudioTrack::DPCMAudioTrack(uint16 format, uint16 bits, uint sampleRate, bool stereo,
        Audio::Mixer::SoundType soundType) : HNMAudioTrack(soundType), _audioStream(nullptr),
	_gotLUT(false), _lastSampleL(0), _lastSampleR(0), _sampleRate(sampleRate), _stereo(stereo) {
	if (bits != 16) {
		error("Unsupported audio bits");
	}
	if (format != 2) {
		warning("Unsupported %d audio format", format);
	}
	// Format 2 is 16-bits DPCM
	_audioStream = Audio::makeQueuingAudioStream(_sampleRate, _stereo);
}

HNMDecoder::DPCMAudioTrack::~DPCMAudioTrack() {
	delete _audioStream;
}

uint32 HNMDecoder::DPCMAudioTrack::decodeSound(uint16 chunkType, byte *data, uint32 size) {
	if (!_gotLUT) {
		if (size < 256 * sizeof(*_lut)) {
			error("Invalid first sound chunk");
		}
		memcpy(_lut, data, 256 * sizeof(*_lut));
		data += 256 * sizeof(*_lut);
		size -= 256 * sizeof(*_lut);
#ifndef SCUMM_LITTLE_ENDIAN
		for (uint i = 0; i < 256; i++) {
			_lut[i] = FROM_LE_16(_lut[i]);
		}
#endif
		_gotLUT = true;
	}

	if (size == 0) {
		return 0;
	}

	uint16 *out = (uint16 *)malloc(size * sizeof(*out));
	uint16 *p = out;

	uint32 numSamples = size;

	byte flags = Audio::FLAG_16BITS;
#ifdef SCUMM_LITTLE_ENDIAN
	flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
	if (_audioStream->isStereo()) {
		numSamples /= 2;

		uint16 sampleL = _lastSampleL;
		uint16 sampleR = _lastSampleR;
		byte deltaId;
		for (uint32 i = 0; i < numSamples; i++, p += 2) {
			deltaId = *(data++);
			sampleL += _lut[deltaId];
			deltaId = *(data++);
			sampleR += _lut[deltaId];
			p[0] = sampleL;
			p[1] = sampleR;
		}
		_lastSampleL = sampleL;
		_lastSampleR = sampleR;

		flags |= Audio::FLAG_STEREO;
	} else {
		uint16 sample = _lastSampleL;
		byte deltaId;
		for (uint32 i = 0; i < numSamples; i++, p++) {
			deltaId = *(data++);
			sample += _lut[deltaId];
			*p = sample;
		}
		_lastSampleL = sample;
	}

	_audioStream->queueBuffer((byte *)out, size * sizeof(*out), DisposeAfterUse::YES, flags);
	return numSamples;
}

HNMDecoder::APCAudioTrack::APCAudioTrack(uint sampleRate, byte stereo,
        Audio::Mixer::SoundType soundType) : HNMAudioTrack(soundType),
	_audioStream(Audio::makeAPCStream(sampleRate, stereo)) {
}

HNMDecoder::APCAudioTrack::~APCAudioTrack() {
	delete _audioStream;
}

Audio::AudioStream *HNMDecoder::APCAudioTrack::getAudioStream() const {
	return _audioStream;
}

uint32 HNMDecoder::APCAudioTrack::decodeSound(uint16 chunkType, byte *data, uint32 size) {
	if (chunkType == MKTAG16('A', 'A')) {
		Common::MemoryReadStream *stream = new Common::MemoryReadStream(data, size);
		if (!_audioStream->init(*stream)) {
			error("Invalid APC stream");
		}
		// Remove header for sample counting
		size -= stream->pos();
		_audioStream->queuePacket(stream);
		// stream is freed now
		stream = nullptr;
	} else if (chunkType == MKTAG16('B', 'B')) {
		_audioStream->queuePacket(new Common::MemoryReadStream(data, size));
	}
	// 2 4-bit samples in one byte
	uint32 numSamples = size;
	if (!_audioStream->isStereo()) {
		numSamples *= 2;
	}
	return numSamples;
}

} // End of namespace Video
