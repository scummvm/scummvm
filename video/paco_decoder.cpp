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

#include "common/endian.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/surface.h"
#include "video/qt_data.h"
#include "video/paco_decoder.h"


#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Video {

enum frameTypes {
	NOP = 0, // nop
	// 1 - old initialisation data?
	PALLETE = 2, // - new initialisation data (usually 0x30 0x00 0x00 ... meaning 8-bit with default QuickTime palette)
	DELAY = 3, //  - delay information
	AUDIO = 4, // - audio data (8-bit unsigned PCM)
	// 5 - should not be present
	// 6 - should not be present
	// 7 - unknown
	VIDEO = 8, // - video frame
	// 9 - unknown
	// 10 - dummy?
	EOC = 11 // - end of chunk marker
};

PacoDecoder::PacoDecoder()
	: _fileStream(nullptr), _videoTrack(nullptr), _audioTrack(nullptr) {
}

PacoDecoder::~PacoDecoder() {
	close();
}

void PacoDecoder::PacoDecoder::close() {
	Video::VideoDecoder::close();

	delete _fileStream;
	_fileStream = nullptr;
	_videoTrack = nullptr;
	_audioTrack = nullptr;
}

bool PacoDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_curFrame = 0;
	stream->readUint16BE(); // 1
	stream->readUint16BE(); // 0x26

	uint16 width = stream->readUint16BE();
	uint16 height = stream->readUint16BE();
	int16 frameRate = stream->readUint16BE();
	frameRate = ABS(frameRate); // Negative framerate is indicative of audio, but not always
	uint16 flags = stream->readUint16BE();
	bool hasAudio = (flags & 0x100) == 0x100;

	stream->readUint32BE(); // maxChunksize
	stream->readUint32BE(); // always 0
	stream->readUint32BE(); // audio related flags
	uint16 frameCount = stream->readUint16BE();
	stream->readUint16BE(); // copy of frameCount
	stream->readUint16BE(); // always 8?
	stream->readUint16BE(); // always 0x600?
	stream->readUint32BE(); // flags
	stream->readUint16BE(); // 0

	for (uint i = 0; i < frameCount; i++) {
		_frameSizes[i] = stream->readUint32BE();
	}

	_fileStream = stream;

	_videoTrack = new PacoVideoTrack(frameRate, frameCount, width, height);
	addTrack(_videoTrack);
	if (hasAudio) {
		_audioTrack = new PacoAudioTrack(getAudioSamplingRate());
		addTrack(_audioTrack);
	}
	return true;
}

int PacoDecoder::getAudioSamplingRate() {
	/**
	 * SamplingRate is found inside the audio packets
	 * Search for the first audio packet and use it.
	 */
	const Common::Array<int> samplingRates = {5563, 7418, 11127, 22254};
	int index;

	int64 startPos = _fileStream->pos();

	while (true){
		int64 currentPos = _fileStream->pos();
		int frameType = _fileStream->readByte();
		int v = _fileStream->readByte();
		uint32 chunkSize =  (v << 16 ) | _fileStream->readUint16BE();
		if (frameType != AUDIO) {
			_fileStream->seek(currentPos + chunkSize);
			continue;
		}
		uint16 header = _fileStream->readUint16BE();
		_fileStream->readUint16BE();
		index = (header >> 10) & 7;
		break;
	}
	_fileStream->seek(startPos);

	return samplingRates[index];
}

const Common::List<Common::Rect> *PacoDecoder::getDirtyRects() const {
	const Track *track = getTrack(0);

	if (track)
		return ((const PacoVideoTrack *)track)->getDirtyRects();

	return 0;
}

void PacoDecoder::clearDirtyRects() {
	Track *track = getTrack(0);

	if (track)
		((PacoVideoTrack *)track)->clearDirtyRects();
}

void PacoDecoder::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	Track *track = getTrack(0);

	if (track)
		((PacoVideoTrack *)track)->copyDirtyRectsToBuffer(dst, pitch);
}

const byte* PacoDecoder::getPalette(){
	Track *track = getTrack(0);

	if (track)
		return ((PacoVideoTrack *)track)->getPalette();
	return nullptr;
}

const byte* PacoDecoder::PacoVideoTrack::getPalette() const {
	_dirtyPalette = false;
	return _palette.data;
}

PacoDecoder::PacoVideoTrack::PacoVideoTrack(
	uint16 frameRate, uint16 frameCount, uint16 width, uint16 height) : _palette(256) {
	_curFrame = 0;
	_frameRate = frameRate;
	_frameCount = frameCount;

	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_palette.set(quickTimeDefaultPalette256, 0, 256);
	_dirtyPalette = true;
}

PacoDecoder::PacoVideoTrack::~PacoVideoTrack() {
	_surface->free();
	delete _surface;
}

bool PacoDecoder::PacoVideoTrack::endOfTrack() const {
	return getCurFrame() >= getFrameCount();
}

uint16 PacoDecoder::PacoVideoTrack::getWidth() const {
	return _surface->w;
}

uint16 PacoDecoder::PacoVideoTrack::getHeight() const {
	return _surface->h;
}

Graphics::PixelFormat PacoDecoder::PacoVideoTrack::getPixelFormat() const {
	return _surface->format;
}

void PacoDecoder::readNextPacket() {
	uint32 nextFrame = _fileStream->pos() + _frameSizes[_curFrame];

	debug(2, " frame %3d size %d @ %lX", _curFrame, _frameSizes[_curFrame], long(_fileStream->pos()));

	_curFrame++;

	while (_fileStream->pos() < nextFrame) {
		int64 currentPos = _fileStream->pos();
		int frameType = _fileStream->readByte();
		int v = _fileStream->readByte();
		uint32 chunkSize =  (v << 16 ) | _fileStream->readUint16BE();
		debug(2, "  slot type %d size %d @ %lX", frameType, chunkSize, long(_fileStream->pos() - 4));

		switch (frameType) {
		case AUDIO:
			_audioTrack->queueSound(_fileStream, chunkSize - 4);
			break;
		case VIDEO:
			_videoTrack->handleFrame(_fileStream, chunkSize - 4, _curFrame);
			break;
		case PALLETE:
			_videoTrack->handlePalette(_fileStream);
			break;
		case EOC:
			break;
		default:
			error("PacoDecoder::decodeFrame(): unknown main chunk type (type = 0x%02X)", frameType);
			break;
		}
		_fileStream->seek(currentPos + chunkSize);
	 }


}

const Graphics::Surface *PacoDecoder::PacoVideoTrack::decodeNextFrame() {
	return _surface;
}

void PacoDecoder::PacoVideoTrack::handlePalette(Common::SeekableReadStream *fileStream) {
	uint32 header = fileStream->readUint32BE();
	if (header == 0x30000000) { // default quicktime palette
		_palette.set(quickTimeDefaultPalette256, 0, 256);
	} else {
		fileStream->readUint32BE(); // 4 bytes of 00
		for (int i = 0; i < 256 * 3; i++){
			_palette.data[i] = fileStream->readByte();
		}
	}
	_dirtyPalette = true;
}

enum {
	COPY = 0,		// raw copy pixels
	RLE, 	 		// RLE
	PRLE,			// pair RLE (read a pair of pixels and repeat it the specified number of times)
	QRLE, 			// quad RLE (read four pixels and repeat it the specified number of times)
	SKIP, 			// skip
	ENDCURRENTLINE,	// end current line and skip additional len lines
	EOFRAME = 15 	// not real opcode but 00 F0 00 00 is often seen at the end of frame and can serve as an end marker
};

#define ADJUST_LINE \
	if (compr == 1) \
		ypos2 = ypos; \
	else { \
		ypos2 = ypos * 2 - y; \
		if (ypos2 >= y + bh) { \
			ypos2 -= bh; \
			if (!(bh & 1)) \
				ypos2++; \
		} \
	}

#define PUTPIX(pix) \
	do { \
		*dst++ = pix; \
		xpos++; \
	} while(0);

#define SKIP() \
	do { \
		dst++; \
		xpos++; \
	} while(0);


void PacoDecoder::PacoVideoTrack::handleFrame(Common::SeekableReadStream *fileStream, uint32 chunkSize, int curFrame) {
	_curFrame = curFrame;
	uint16 w = getWidth();

	uint16 x = fileStream->readUint16BE();			// x offset of the updated area
	uint16 y = fileStream->readUint16BE();			// y offset of the updated area
	uint16 bw = fileStream->readUint16BE();			// updated area width
	uint16 bh = fileStream->readUint16BE();			// updated area height
	uint compr = fileStream->readByte();	    	// compression method and flags
	fileStream->readByte();							// padding

	debug(5, "    +%d,%d - %dx%d compr %X", x, y, bw, bh, compr);

	compr = compr & 0xF;

	uint8 *fdata = new uint8[1048576];              // 0x100000 copied from original pacodec
	fileStream->read(fdata, chunkSize - 10);       // remove header length
	debug(5, "pos: %ld", long(fileStream->pos()));
	int16 xpos = x, ypos = y, ypos2 = y;
	byte *dst = (byte *)_surface->getPixels() + x + y * w;

	const uint8 *src = fdata;
	int16 i, c, c1, c2, c3, c4;
	uint8 clrs[16];


	while (ypos < y + bh) {
		c = *src++;
		debug(5, "debug info: ypos %d y %d bh %d src: %d", ypos, y, bh, c);

		if (c == 0 ){ // long operation
			int16 op = src[0] >> 4;
			int16 len = ((src[0] & 0xF) << 8) | src[1];
			src += 2;
			debug(5, "    long operation: opcode: %d", op);
			switch (op) {
			case COPY:
				while (len--)
					PUTPIX(*src++);
				break;
			case RLE:
				c1 = *src++;
				while (len--)
					PUTPIX(c1);
				break;
			case PRLE:
				c1 = *src++;
				c2 = *src++;
				while (len--){
					PUTPIX(c1);
					PUTPIX(c2);
				}
				break;
			case QRLE:
				c1 = *src++;
				c2 = *src++;
				c3 = *src++;
				c4 = *src++;
				while (len--) {
					PUTPIX(c1);
					PUTPIX(c2);
					PUTPIX(c3);
					PUTPIX(c4);
				}
				break;
			case SKIP:
				while (len--)
					SKIP();
				break;
			case ENDCURRENTLINE:
				xpos = x;
				ypos += len + 1;
				ADJUST_LINE;
				dst = (byte *)_surface->getPixels() + xpos + ypos2 * w;
				break;
			case EOFRAME:
				xpos = x;
				ypos = y + bh;
				break;
			default:
				PUTPIX(0xFF);
				debug(5, "PacoDecoder::PacoVideoTrack::handleFrame: Long op: 0x0 op %d", op);
			}

		} else if (c < 128) { // copy the same amount of pixels
			debug(5, "    copy pixels: %d", c);
			while (c--)
				PUTPIX(*src++);
		} else if (c < 254) { // repeat the following value 256 - op times
			debug(5, "    copy pixels -op: %d", 256 - c);
			c1 = *src++;
			c = 256 - c;
			while (c--)
				PUTPIX(c1);
		} else if (c < 255) {
			// next byte is either the number of pixels to skip (if non-zero) or
			// a signal of compact RLE mode
			c = *src++;

			if (!c) {                                   // compact RLE mode
				unsigned mask = (src[0] << 8) | src[1];
				src += 2;
				debug(5, "debug info compact RLE: c: %d mask: %d", c, mask);

				for (i = 0; i < 16; i++, mask >>= 1) {
					if (mask & 1)
						clrs[i] = *src++;
				}
				while (xpos < x + bw) {
					int16 op = *src++;
					int16 len = op & 0xF;
					op >>= 4;
					if (op == 0) {                      // low nibble....
						op = len;
						len = *src++;
						debug(5, "debug info compact: op: %d", op);
						switch (op) {
						case COPY:
							debug(5, "debug info COPY: %d", len);
							while (len--) {
								c = *src++;
								PUTPIX(clrs[c >> 4]);
								if (!len)
									break;
								len--;
								PUTPIX(clrs[c & 0xF]);
							}
							break;
						case RLE:
							debug(5, "debug info RLE: %d", len);
							c = *src++;
							while (len--)
								PUTPIX(clrs[c & 0xF]);
							break;
						case PRLE:
							debug(5, "debug info PRLE: %d", len);
							c = *src++;
							c1 = clrs[c >> 4];
							c2 = clrs[c & 0xF];
							while (len--) {
								PUTPIX(c1);
								PUTPIX(c2);
							}
							break;
						case QRLE:
							debug(5, "debug info QRLE: %d", len);
							c = *src++;
							c1 = clrs[c >> 4];
							c2 = clrs[c & 0xF];
							c = *src++;
							c3 = clrs[c >> 4];
							c4 = clrs[c & 0xF];
							while (len--) {
								PUTPIX(c1);
								PUTPIX(c2);
								PUTPIX(c3);
								PUTPIX(c4);
							}
							break;
						case SKIP:
							debug(5, "debug info SKIP: %d", len);
							while (len--)
								SKIP();
							break;
						case ENDCURRENTLINE:
							debug(5, "debug info ENDCURRENTLINE: %d", len);
							xpos = x + bw;
							ypos += len;
							break;
						default:
							warning("PacoDecoder::PacoVideoTrack::handleFrame: Compact RLE mode: 0x0 op %d", op);
						}
					} else if (op < 8) {                // copy 1-7 colors
						debug(5, "debug info copy 1-7 colors: %d", len);
						PUTPIX(clrs[len]);
						op--;
						while (op--) {
							c = *src++;
							PUTPIX(clrs[c >> 4]);
							if (!op)
								break;
							op--;
							PUTPIX(clrs[c & 0xF]);
						}
					} else if (op < 14) {              // repeat color
						debug(5, "debug info Repeat color: %d", len);
						op = 16 - op;
						while (op--)
							PUTPIX(clrs[len]);
					} else if (op < 15) {               // skip number of pixels in low nibbel
						debug(5, "debug info Skip number of pixels: %d", len);
						while (len--)
							SKIP();
					} else {
						if (len < 8) {                  // Pair run
							debug(5, "debug info pair run: %d", len);
							c = *src++;
							c1 = clrs[c >> 4];
							c2 = clrs[c & 0xF];
							while (len--) {
								PUTPIX(c1);
								PUTPIX(c2);
							}
						} else {                        // Quad run
							debug(5, "debug info quad run: %d", len);
							len = 16 - len;
							c = *src++;
							c1 = clrs[c >> 4];
							c2 = clrs[c & 0xF];
							c = *src++;
							c3 = clrs[c >> 4];
							c4 = clrs[c & 0xF];
							while (len--) {
								PUTPIX(c1);
								PUTPIX(c2);
								PUTPIX(c3);
								PUTPIX(c4);
							}
						}
					}
				}
			} else {
				debug(5, "debug info SKIP: %d", c);
				while (c--)
					SKIP();
			}
		} else {
			// pair or quad run. Read the next byte and if it is below 128 then read and
			// repeat a pair of pixels len times, otherwise read and repeat four pixels
			// (but 256 - len times)
			c = *src++;
			if (c < 128) {                             // pair run
				debug(5, "debug info PAIR RUN: %d", c);

				c1 = *src++;
				c2 = *src++;
				while (c--) {
					PUTPIX(c1);
					PUTPIX(c2);
				}
			} else {                                    // quad run
				debug(5, "debug info QUAD RUN: %d", c);
				c = 256 - c;
				c1 = *src++;
				c2 = *src++;
				c3 = *src++;
				c4 = *src++;
				while (c--) {
					PUTPIX(c1);
					PUTPIX(c2);
					PUTPIX(c3);
					PUTPIX(c4);
				}
			}
		}
		if (xpos > x + bw) debug(5, "!!!");
		if (xpos >= x + bw) {
			debug(5, "debug info ADJUST LINE");
			xpos = x;
			ypos++;
			ADJUST_LINE;
			dst = (byte *)_surface->getPixels() + x + ypos2 * w;
		}
	}

	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(x, y, x + bw, y + bh));
	delete[] fdata;
}

void PacoDecoder::PacoVideoTrack::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	for (Common::List<Common::Rect>::const_iterator it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
		for (int y = (*it).top; y < (*it).bottom; ++y) {
			const int x = (*it).left;
			memcpy(dst + y * pitch + x, (byte *)_surface->getBasePtr(x, y), (*it).right - x);
		}
	}
	clearDirtyRects();
}

PacoDecoder::PacoAudioTrack::PacoAudioTrack(int samplingRate)
	: AudioTrack(Audio::Mixer::kPlainSoundType) {
	_samplingRate = samplingRate;
	byte audioFlags = Audio::FLAG_UNSIGNED;
	_packetStream = Audio::makePacketizedRawStream(samplingRate, audioFlags);
}

PacoDecoder::PacoAudioTrack::~PacoAudioTrack() {
	delete _packetStream;
}

void PacoDecoder::PacoAudioTrack::queueSound(Common::SeekableReadStream *fileStream, uint32 chunkSize) {
	const Common::Array<int> samplingRates = {5563, 7418, 11127, 22254};
	uint16 header = fileStream->readUint16BE();
	fileStream->readUint16BE();
	int index = (header >> 10) & 7;
	int currentRate = samplingRates[index];
	if ( currentRate != _samplingRate)
		warning("PacoDecoder::PacoAudioTrack: Sampling rate differs from first frame: %i != %i", currentRate, _samplingRate);

	_packetStream->queuePacket(fileStream->readStream(chunkSize - 4));
}


} // End of namespace Video
