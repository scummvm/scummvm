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

#ifndef VOYEUR_ANIMATION_H
#define VOYEUR_ANIMATION_H

#include "video/video_decoder.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/timestamp.h"
#include "common/array.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/stream.h"
#include "voyeur/files.h"

namespace Voyeur {

class VoyeurEngine;

/**
 * Decoder for RL2 videos.
 *
 * Video decoder used in engines:
 *  - voyeur
 */
class RL2Decoder : public Video::VideoDecoder {
private:
	class RL2FileHeader {
	public:
		RL2FileHeader();
		~RL2FileHeader();

		int _channels;
		int _colorCount;
		int _numFrames;
		int _rate;
		int _soundRate;
		int _videoBase;
		int *_frameSoundSizes;

		uint32 _backSize;
		uint32 _signature;
		uint32 *_frameOffsets;

		byte _palette[768];

		void load(Common::SeekableReadStream *stream);
		Common::Rational getFrameRate() const;
		bool isValid() const;

	private:
		uint32 _form;      // Unused variable
		uint32 _dataSize;  // Unused variable
		int _method;       // Unused variable
		int _defSoundSize;
	};

	class SoundFrame {
	public:
		int _offset;
		int _size;

		SoundFrame(int  offset, int size);
	};

	class RL2AudioTrack : public AudioTrack {
	private:
		Audio::Mixer::SoundType _soundType;
		const RL2FileHeader &_header;
		Audio::QueuingAudioStream *_audStream;
	protected:
		Audio::AudioStream *getAudioStream() const;
	public:
		RL2AudioTrack(const RL2FileHeader &header, Common::SeekableReadStream *stream,
			Audio::Mixer::SoundType soundType);
		~RL2AudioTrack();

		Audio::Mixer::SoundType getSoundType() const { return _soundType; }
		int numQueuedStreams() const { return _audStream->numQueuedStreams(); }
		virtual bool isSeekable() const { return true; }
		virtual bool seek(const Audio::Timestamp &time) { return true; }

		void queueSound(Common::SeekableReadStream *stream, int size);
	};

	class RL2VideoTrack : public FixedRateVideoTrack {
	public:
		RL2VideoTrack(const RL2FileHeader &header, RL2AudioTrack *audioTrack, 
			Common::SeekableReadStream *stream);
		~RL2VideoTrack();

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::Surface *getSurface() { return _surface; }
		Graphics::Surface *getBackSurface();
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _header._numFrames; }
		const Graphics::Surface *decodeNextFrame();
		const byte *getPalette() const { _dirtyPalette = false; return _header._palette; }
		int getPaletteCount() const { return _header._colorCount; }
		bool hasDirtyPalette() const { return _dirtyPalette; }
		const Common::List<Common::Rect> *getDirtyRects() const { return &_dirtyRects; }
		void clearDirtyRects() { _dirtyRects.clear(); }
		void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

		virtual Common::Rational getFrameRate() const { return _header.getFrameRate(); }
		virtual bool isSeekable() const { return true; }
		virtual bool seek(const Audio::Timestamp &time);
	private:
		Common::SeekableReadStream *_fileStream;
		const RL2FileHeader &_header;
		Graphics::Surface *_surface;
		Graphics::Surface *_backSurface;
		bool _hasBackFrame;

		mutable bool _dirtyPalette;

		bool _initialFrame;
		int _curFrame;
		uint32 _videoBase;
		uint32 *_frameOffsets;

		Common::List<Common::Rect> _dirtyRects;

		void copyFrame(uint8 *data);
		void rl2DecodeFrameWithTransparency(int screenOffset);
		void rl2DecodeFrameWithoutTransparency(int screenOffset = -1);
		void initBackSurface();
	};

private:
	RL2AudioTrack *_audioTrack;
	RL2VideoTrack *_videoTrack;
	Common::SeekableReadStream *_fileStream;
	Audio::Mixer::SoundType _soundType;
	RL2FileHeader _header;
	int _paletteStart;
	Common::Array<SoundFrame> _soundFrames;
	int _soundFrameNumber;
	const Common::List<Common::Rect> *getDirtyRects() const;

	void clearDirtyRects();
	void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);
	int getPaletteStart() const { return _paletteStart; }
	const RL2FileHeader &getHeader() { return _header; }
	virtual void readNextPacket();
	virtual bool seekIntern(const Audio::Timestamp &time);

public:
	RL2Decoder(Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	virtual ~RL2Decoder();

	virtual void close();

	bool loadStream(Common::SeekableReadStream *stream);
	bool loadRL2File(const Common::String &file, bool palFlag);
	bool loadVideo(int videoId);
	int getPaletteCount() const { return _header._colorCount; }

	/**
	 * Play back a given Voyeur RL2 video
	 * @param vm			Engine reference
	 * @param resourceOffset Starting resource to use for frame pictures
	 * @param frames		Optional frame numbers resource for when to apply image data
	 * @param imgPos		Position to draw image data
	 */
	void play(VoyeurEngine *vm, int resourceOffset = 0, byte *frames = NULL, byte *imgPos = NULL);
	RL2VideoTrack *getRL2VideoTrack() { return _videoTrack; }
	RL2AudioTrack *getRL2AudioTrack() { return _audioTrack; }
};

} // End of namespace Voyeur

#endif /* VOYEUR_ANIMATION_H */
