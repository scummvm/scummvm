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

#ifndef VIDEO_MVEDECODER_H
#define VIDEO_MVEDECODER_H

#include "audio/audiostream.h"
#include "video/video_decoder.h"
#include "graphics/surface.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/memstream.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
}

class PaletteManager;

namespace Video {

/**
 * Decoder for Interplay MVE videos.
 *
 * Video decoder used in engines:
 *  - kingdom
 */
class MveDecoder : public VideoDecoder {
	bool _done;
	Common::SeekableReadStream *_s;

	uint16 _packetLen;
	uint16 _packetKind;

	Graphics::Surface _decodeSurface0;
	Graphics::Surface _decodeSurface1;
	Graphics::Surface _frameSurface;

	uint16 _widthInBlocks;
	uint16 _heightInBlocks;

	uint16 _width;
	uint16 _height;

	Common::Rational _frameRate;

	bool      _trueColor;
	Graphics::PixelFormat _pixelFormat;
	bool      _dirtyPalette;
	uint16    _palStart;
	uint16    _palCount;
	byte      _palette[0x300];

	uint16    _skipMapSize;
	byte     *_skipMap;

	uint16    _decodingMapSize;
	byte     *_decodingMap;

	int       _frameNumber;
	uint16    _frameSize;
	byte     *_frameData;

	int _audioTrack;
	Audio::QueuingAudioStream *_audioStream;

	void readPacketHeader();
	void copyBlock_8bit(Graphics::Surface &dst, Common::MemoryReadStream &s, int block);
	void copyBlock_16bit(Graphics::Surface &dst, Common::MemoryReadStream &s, int block);
	void copyBlock(Graphics::Surface &dst, Graphics::Surface &src, int block, int offset = 0);
	void copyBlock(Graphics::Surface &dst, Graphics::Surface &src, int dx, int dy, int off_x, int off_y);

	void decodeFormat6();
	void decodeFormat10();

	class MveVideoTrack : public FixedRateVideoTrack {
		MveDecoder *_decoder;
	public:
		MveVideoTrack(MveDecoder *decoder);

		bool endOfTrack() const;

		uint16 getWidth() const;
		uint16 getHeight() const;

		Graphics::PixelFormat getPixelFormat() const;

		int getCurFrame() const;
		// int getFrameCount() const;

		const Graphics::Surface *decodeNextFrame();
		const byte *getPalette() const;
		bool hasDirtyPalette() const;

	protected:
		Common::Rational getFrameRate() const;
	};

	class MveAudioTrack : public AudioTrack {
		MveDecoder *_decoder;
	public:
		MveAudioTrack(MveDecoder *decoder);

		Audio::AudioStream *getAudioStream() const;
	};

	class MveSkipStream {
		Common::MemoryReadStream s;
		uint16 queue;
	public:
		MveSkipStream(byte *p, size_t sz)
			: s(p, sz), queue(0x8000)
		{}

		void reset() {
			s.seek(0);
			queue = 0x8000;
		}

		bool skip() {
			if (queue == 0x8000) {
				queue = s.readUint16LE();
				assert(queue != 0);
			}
			bool r = (queue & 0x8000) == 0;
			queue <<= 1;
			return r;
		}
	};

public:
	MveDecoder();
	virtual ~MveDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void setAudioTrack(int track);
	void applyPalette(PaletteManager *paletteManager);

	// const Common::List<Common::Rect> *getDirtyRects() const;
	// void clearDirtyRects();
	// void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

	Common::Rational getFrameRate() { return _frameRate; }
	void readNextPacket();
};

} // End of namespace Video

#endif
