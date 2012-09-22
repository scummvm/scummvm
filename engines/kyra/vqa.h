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

class VqaDecoder : public Video::VideoDecoder {
public:
	VqaDecoder();
	virtual ~VqaDecoder();

	bool loadStream(Common::SeekableReadStream *stream);

private:
	class VqaAudioTrack : public AudioTrack {
	public:
		VqaAudioTrack(Common::SeekableReadStream *stream, int freq);
		~VqaAudioTrack();

		void handleSND0();
		void handleSND1();
		void handleSND2();

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::QueuingAudioStream *_audioStream;
		Common::SeekableReadStream *_fileStream;
	};

	class VqaVideoTrack : public FixedRateVideoTrack {
	public:
		VqaVideoTrack(Common::SeekableReadStream *stream);
		~VqaVideoTrack();

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const;
		int getFrameCount() const;
		const Graphics::Surface *decodeNextFrame();

		bool hasSound() const;
		int getAudioFreq() const;
		bool hasDirtyPalette() const;
		const byte *getPalette() const;

		void setAudioTrack(VqaAudioTrack *audioTrack);

		void handleVQHD();
		void handleFINF();
		void handleVQFR();

	protected:
		Common::Rational getFrameRate() const;

	private:
		Common::SeekableReadStream *_fileStream;
		Graphics::Surface *_surface;
		byte _palette[3 * 256];
		mutable bool _dirtyPalette;
		VqaAudioTrack *_audioTrack;

		int _curFrame;

		struct VqaHeader {
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

		VqaHeader _header;
		uint32 *_frameInfo;
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

class VqaMovie {
public:
	VqaMovie(KyraEngine_v1 *vm, OSystem *system);
	~VqaMovie();

	bool open(const char *filename);
	void close();
	void play();
private:
	OSystem *_system;
	KyraEngine_v1 *_vm;
	Screen *_screen;
	VqaDecoder *_decoder;
	Common::File _file;
};

} // End of namespace Kyra

#endif
