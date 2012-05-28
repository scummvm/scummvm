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

#include "audio/mixer.h"
#include "audio/decoders/quicktime_intern.h"
#include "common/scummsys.h"
#include "common/rational.h"
#include "graphics/pixelformat.h"

#include "video/video_decoder.h"

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
	uint16 getWidth() const { return _width; }

	/**
	 * Returns the height of the video
	 * @return the height of the video
	 */
	uint16 getHeight() const { return _height; }

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

	int32 getCurFrame() const;

	bool isVideoLoaded() const { return isOpen(); }
	const Graphics::Surface *decodeNextFrame();
	bool endOfVideo() const;
	uint32 getTime() const;
	uint32 getTimeToNextFrame() const;
	Graphics::PixelFormat getPixelFormat() const;

	// SeekableVideoDecoder API
	void seekToFrame(uint32 frame);
	void seekToTime(const Audio::Timestamp &time);
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

	// VideoDecoder API
	void updateVolume();
	void updateBalance();

private:
	void init();

	void startAudio();
	void stopAudio();
	void updateAudioBuffer();
	void readNextAudioChunk();
	Common::Array<Audio::SoundHandle> _audioHandles;
	Audio::Timestamp _audioStartOffset;

	Codec *createCodec(uint32 codecTag, byte bitsPerPixel);
	uint32 findKeyFrame(uint32 frame) const;

	bool _dirtyPalette;
	const byte *_palette;
	bool _setStartTime;
	bool _needUpdate;

	uint16 _width, _height;

	Graphics::Surface *_scaledSurface;
	void scaleSurface(const Graphics::Surface *src, Graphics::Surface *dst,
			Common::Rational scaleFactorX, Common::Rational scaleFactorY);

	void pauseVideoIntern(bool pause);
	bool endOfVideoTracks() const;

	// The TrackHandler is a class that wraps around a QuickTime Track
	// and handles playback in this decoder class.
	class TrackHandler {
	public:
		TrackHandler(QuickTimeDecoder *decoder, Track *parent);
		virtual ~TrackHandler() {}

		enum TrackType {
			kTrackTypeAudio,
			kTrackTypeVideo
		};

		virtual TrackType getTrackType() const = 0;

		virtual void seekToTime(Audio::Timestamp time) = 0;

		virtual bool endOfTrack();

	protected:
		uint32 _curEdit;
		QuickTimeDecoder *_decoder;
		Common::SeekableReadStream *_fd;
		Track *_parent;
	};

	// The AudioTrackHandler is currently just a wrapper around some
	// QuickTimeDecoder functions.
	class AudioTrackHandler : public TrackHandler {
	public:
		AudioTrackHandler(QuickTimeDecoder *decoder, QuickTimeAudioTrack *audioTrack);
		TrackType getTrackType() const { return kTrackTypeAudio; }

		void updateBuffer();
		void seekToTime(Audio::Timestamp time);
		bool endOfTrack();

	private:
		QuickTimeAudioTrack *_audioTrack;
	};

	// The VideoTrackHandler is the bridge between the time of playback
	// and the media for the given track. It calculates when to start
	// tracks and at what rate to play the media using the edit list.
	class VideoTrackHandler : public TrackHandler {
	public:
		VideoTrackHandler(QuickTimeDecoder *decoder, Track *parent);
		~VideoTrackHandler();

		TrackType getTrackType() const { return kTrackTypeVideo; }

		const Graphics::Surface *decodeNextFrame();

		uint32 getNextFrameStartTime();

		uint32 getFrameCount();

		int32 getCurFrame() { return _curFrame; }

		Graphics::PixelFormat getPixelFormat() const;

		void seekToTime(Audio::Timestamp time);

		Common::Rational getWidth() const;
		Common::Rational getHeight() const;

	private:
		int32 _curFrame;
		uint32 _nextFrameStartTime;
		Graphics::Surface *_scaledSurface;
		bool _holdNextFrameStartTime;
		int32 _durationOverride;

		Common::SeekableReadStream *getNextFramePacket(uint32 &descId);
		uint32 getFrameDuration();
		uint32 findKeyFrame(uint32 frame) const;
		void enterNewEditList(bool bufferFrames);
		const Graphics::Surface *bufferNextFrame();
		uint32 getRateAdjustedFrameTime() const;
		uint32 getCurEditTimeOffset() const;
		uint32 getCurEditTrackDuration() const;
	};

	Common::Array<TrackHandler *> _handlers;
	VideoTrackHandler *_nextVideoTrack;
	VideoTrackHandler *findNextVideoTrack() const;

	void freeAllTrackHandlers();
};

} // End of namespace Video

#endif
