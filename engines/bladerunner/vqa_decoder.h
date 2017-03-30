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

#ifndef BLADERUNNER_VQA_DECODER_H
#define BLADERUNNER_VQA_DECODER_H

#include "bladerunner/adpcm_decoder.h"

#include "audio/audiostream.h"

#include "common/debug.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/types.h"

#include "graphics/surface.h"

#include "video/video_decoder.h"

namespace BladeRunner {

class Lights;
class View;
class ZBuffer;

class VQADecoder {
public:
	VQADecoder();
	~VQADecoder();

	bool loadStream(Common::SeekableReadStream *s);

	void readFrame(int frame, int skipFlags);

	const Graphics::Surface    *decodeVideoFrame();
	void                        decodeZBuffer(ZBuffer *zbuffer);
	Audio::SeekableAudioStream *decodeAudioFrame();
	void                        decodeView(View *view);
	void                        decodeLights(Lights *lights);

	uint16 numFrames() const { return _header.numFrames; }
	uint8  frameRate() const { return _header.frameRate; }

	uint16 offsetX() const { return _header.offsetX; }
	uint16 offsetY() const { return _header.offsetY; }

	bool   hasAudio() const { return _header.channels != 0; }
	uint16 frequency() const { return _header.freq; }

	bool getLoopBeginAndEndFrame(int loop, int *begin, int *end);

protected:

private:
	struct Header {
		uint16 version;     // 0x00
		uint16 flags;       // 0x02
		uint16 numFrames;   // 0x04
		uint16 width;       // 0x06
		uint16 height;      // 0x08
		uint8  blockW;      // 0x0A
		uint8  blockH;      // 0x0B
		uint8  frameRate;   // 0x0C
		uint8  cbParts;     // 0x0D
		uint16 colors;      // 0x0E
		uint16 maxBlocks;   // 0x10
		uint16 offsetX;     // 0x12
		uint16 offsetY;     // 0x14
		uint16 maxVPTRSize; // 0x16
		uint16 freq;        // 0x18
		uint8  channels;    // 0x1A
		uint8  bits;        // 0x1B
		uint32 unk3;        // 0x1C
		uint16 unk4;        // 0x20
		uint32 maxCBFZSize; // 0x22
		uint32 unk5;        // 0x26
		                    // 0x2A
	};

	struct Loop {
		uint16         begin;
		uint16         end;
		Common::String name;

		Loop() :
			begin(0),
			end(0)
		{}
	};

	struct LoopInfo {
		uint16  loopCount;
		uint32  flags;
		Loop   *loops;

		LoopInfo() : loopCount(0), loops(nullptr) {}
		~LoopInfo() {
			delete[] loops;
		}
	};

	struct ClipInfo {
		uint16 clipCount;
	};

	class VQAVideoTrack;
	class VQAAudioTrack;

	Common::SeekableReadStream *_s;

	Header   _header;
	LoopInfo _loopInfo;
	ClipInfo _clipInfo;

	uint32  *_frameInfo;

	uint32   _maxVIEWChunkSize;
	uint32   _maxZBUFChunkSize;
	uint32   _maxAESCChunkSize;

	VQAVideoTrack *_videoTrack;
	VQAAudioTrack *_audioTrack;

	void readPacket(int skipFlags);

	bool readVQHD(Common::SeekableReadStream *s, uint32 size);
	bool readMSCI(Common::SeekableReadStream *s, uint32 size);
	bool readMFCI(Common::SeekableReadStream *s, uint32 size);
	bool readLINF(Common::SeekableReadStream *s, uint32 size);
	bool readCINF(Common::SeekableReadStream *s, uint32 size);
	bool readFINF(Common::SeekableReadStream *s, uint32 size);
	bool readLNIN(Common::SeekableReadStream *s, uint32 size);
	bool readCLIP(Common::SeekableReadStream *s, uint32 size);

	class VQAVideoTrack {
	public:
		VQAVideoTrack(VQADecoder *vqaDecoder);
		~VQAVideoTrack();

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const;
		int getFrameCount() const;
		const Graphics::Surface *decodeVideoFrame();
		void decodeZBuffer(ZBuffer *zbuffer);
		void decodeView(View *view);
		void decodeLights(Lights *lights);

		bool readVQFR(Common::SeekableReadStream *s, uint32 size);
		bool readVPTR(Common::SeekableReadStream *s, uint32 size);
		bool readVQFL(Common::SeekableReadStream *s, uint32 size);
		bool readCBFZ(Common::SeekableReadStream *s, uint32 size);
		bool readZBUF(Common::SeekableReadStream *s, uint32 size);
		bool readVIEW(Common::SeekableReadStream *s, uint32 size);
		bool readAESC(Common::SeekableReadStream *s, uint32 size);
		bool readLITE(Common::SeekableReadStream *s, uint32 size);

	protected:
		Common::Rational getFrameRate() const;

		bool useAudioSync() const { return false; }

	private:
		Graphics::Surface *_surface;
		bool _hasNewFrame;

		uint16 _numFrames;
		uint16 _width, _height;
		uint8  _blockW, _blockH;
		uint8  _frameRate;
		uint16 _maxBlocks;
		uint16 _offsetX, _offsetY;

		uint16  _maxVPTRSize;
		uint32  _maxCBFZSize;
		uint32  _maxZBUFChunkSize;

		uint32   _codebookSize;
		uint8   *_codebook;
		uint8   *_cbfz;
		bool     _zbufChunkComplete;
		uint32   _zbufChunkSize;
		uint8   *_zbufChunk;

		uint32   _vpointerSize;
		uint8   *_vpointer;

		int      _curFrame;

		uint8   *_viewData;
		uint32	_viewDataSize;
		uint8   *_lightsData;
		uint32	_lightsDataSize;

		void VPTRWriteBlock(uint16 *frame, unsigned int dstBlock, unsigned int srcBlock, int count, bool alpha = false);
		bool decodeFrame(uint16 *frame);
	};

	class VQAAudioTrack {
	public:
		VQAAudioTrack(VQADecoder *vqaDecoder);
		~VQAAudioTrack();

		bool readSND2(Common::SeekableReadStream *s, uint32 size);
		bool readSN2J(Common::SeekableReadStream *s, uint32 size);

		Audio::SeekableAudioStream *decodeAudioFrame();
	protected:

	private:
		uint16               _frequency;
		ADPCMWestwoodDecoder _adpcmDecoder;
		uint8                _compressedAudioFrame[735];
	};
};

} // End of namespace BladeRunner

#endif
