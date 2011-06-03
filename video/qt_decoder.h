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

//
// Partially based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#ifndef VIDEO_QT_DECODER_H
#define VIDEO_QT_DECODER_H

#include "common/scummsys.h"
#include "common/rational.h"

#include "video/video_decoder.h"

#include "audio/mixer.h"
#include "audio/decoders/quicktime_intern.h"

namespace Common {
	class Rational;
}

namespace Video {

class Codec;

/**
 * Decoder for QuickTime videos.
 *
 * Video decoder used in engines:
 *  - mohawk
 *  - sci
 */
class QuickTimeDecoder : public SeekableVideoDecoder, public Audio::QuickTimeAudioDecoder {
public:
	QuickTimeDecoder();
	virtual ~QuickTimeDecoder();

	/**
	 * Returns the width of the video
	 * @return the width of the video
	 */
	uint16 getWidth() const;

	/**
	 * Returns the height of the video
	 * @return the height of the video
	 */
	uint16 getHeight() const;

	/**
	 * Returns the amount of frames in the video
	 * @return the amount of frames in the video
	 */
	uint32 getFrameCount() const;

	/**
	 * Load a video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const Common::String &filename);

	/**
	 * Load a QuickTime video file from a SeekableReadStream
	 * @param stream	the stream to load
	 */
	bool loadStream(Common::SeekableReadStream *stream);

	/**
	 * Close a QuickTime encoded video file
	 */
	void close();

	/**
	 * Returns the palette of the video
	 * @return the palette of the video
	 */
	const byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }

	bool isVideoLoaded() const { return isOpen(); }
	const Graphics::Surface *decodeNextFrame();
	bool endOfVideo() const;
	uint32 getElapsedTime() const;
	uint32 getTimeToNextFrame() const;
	Graphics::PixelFormat getPixelFormat() const;

	// SeekableVideoDecoder API
	void seekToFrame(uint32 frame);
	void seekToTime(Audio::Timestamp time);
	uint32 getDuration() const { return _duration * 1000 / _timeScale; }

protected:
	class VideoSampleDesc : public Common::QuickTimeParser::SampleDesc {
	public:
		VideoSampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag);
		~VideoSampleDesc();

		void initCodec();

		// TODO: Make private in the long run
		uint16 _bitsPerSample;
		char _codecName[32];
		uint16 _colorTableId;
		byte *_palette;
		Codec *_videoCodec;
	};

	Common::QuickTimeParser::SampleDesc *readSampleDesc(Track *track, uint32 format);

private:
	Common::SeekableReadStream *getNextFramePacket(uint32 &descId);
	uint32 getFrameDuration();
	void init();

	void startAudio();
	void stopAudio();
	void updateAudioBuffer();
	void readNextAudioChunk();
	Audio::SoundHandle _audHandle;
	Audio::Timestamp _audioStartOffset;

	Codec *createCodec(uint32 codecTag, byte bitsPerPixel);
	Codec *findDefaultVideoCodec() const;
	uint32 _nextFrameStartTime;
	int _videoTrackIndex;
	uint32 findKeyFrame(uint32 frame) const;

	bool _dirtyPalette;
	const byte *_palette;

	Graphics::Surface *_scaledSurface;
	const Graphics::Surface *scaleSurface(const Graphics::Surface *frame);
	Common::Rational getScaleFactorX() const;
	Common::Rational getScaleFactorY() const;

	void pauseVideoIntern(bool pause);
};

} // End of namespace Video

#endif
