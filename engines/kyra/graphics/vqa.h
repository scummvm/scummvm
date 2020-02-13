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

#ifndef KYRA_VQA_H
#define KYRA_VQA_H

#include "video/video_decoder.h"
#include "common/file.h"
#include "common/rational.h"

class OSystem;

namespace Audio {
class QueuingAudioStream;
} // End of namespace Audio

namespace Kyra {

class KyraEngine_v1;
class Screen;

class VQADecoder : public Video::VideoDecoder {
public:
	VQADecoder();
	~VQADecoder() override;

	bool loadStream(Common::SeekableReadStream *stream) override;
	void readNextPacket() override;

private:
	Common::SeekableReadStream *_fileStream;

	void handleVQHD(Common::SeekableReadStream *stream);
	void handleFINF(Common::SeekableReadStream *stream);

	struct VQAHeader {
		uint16 version;
		uint16 flags;
		uint16 numFrames;
		uint16 width;
		uint16 height;
		uint8 blockW;
		uint8 blockH;
		uint8 frameRate;
		uint8 cbParts;
		uint16 colors;
		uint16 maxBlocks;
		uint32 unk1;
		uint16 unk2;
		uint16 freq;
		uint8 channels;
		uint8 bits;
		uint32 unk3;
		uint16 unk4;
		uint32 maxCBFZSize;
		uint32 unk5;
	};

	VQAHeader _header;
	uint32 *_frameInfo;

	class VQAAudioTrack : public AudioTrack {
	public:
		VQAAudioTrack(const VQAHeader *header, Audio::Mixer::SoundType soundType);
		~VQAAudioTrack() override;

		void handleSND0(Common::SeekableReadStream *stream);
		void handleSND1(Common::SeekableReadStream *stream);
		void handleSND2(Common::SeekableReadStream *stream);

	protected:
		Audio::AudioStream *getAudioStream() const override;

	private:
		Audio::QueuingAudioStream *_audioStream;
	};

	class VQAVideoTrack : public FixedRateVideoTrack {
	public:
		VQAVideoTrack(const VQAHeader *header);
		~VQAVideoTrack() override;

		uint16 getWidth() const override;
		uint16 getHeight() const override;
		Graphics::PixelFormat getPixelFormat() const override;
		int getCurFrame() const override;
		int getFrameCount() const override;
		const Graphics::Surface *decodeNextFrame() override;

		void setHasDirtyPalette();
		bool hasDirtyPalette() const override;
		const byte *getPalette() const override;

		void handleVQFR(Common::SeekableReadStream *stream);

	protected:
		Common::Rational getFrameRate() const override;

	private:
		Graphics::Surface *_surface;
		byte _palette[3 * 256];
		mutable bool _dirtyPalette;

		bool _newFrame;

		uint16 _width, _height;
		uint8 _blockW, _blockH;
		uint8 _cbParts;
		int _frameCount;
		int _curFrame;
		byte _frameRate;

		uint32 _codeBookSize;
		bool _compressedCodeBook;
		byte *_codeBook;
		int _partialCodeBookSize;
		int _numPartialCodeBooks;
		byte *_partialCodeBook;
		uint32 _numVectorPointers;
		uint16 *_vectorPointers;
	};
};

class VQAMovie {
public:
	VQAMovie(KyraEngine_v1 *vm, OSystem *system);
	~VQAMovie();

	bool open(const char *filename);
	void close();
	void play();
private:
	OSystem *_system;
	KyraEngine_v1 *_vm;
	Screen *_screen;
	VQADecoder *_decoder;
	Common::File _file;
};

} // End of namespace Kyra

#endif
