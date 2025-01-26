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

//
// Partially based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#ifndef VIDEO_QT_DECODER_H
#define VIDEO_QT_DECODER_H

#include "audio/decoders/quicktime_intern.h"
#include "common/scummsys.h"

#include "video/video_decoder.h"

namespace Common {
class Archive;
class Rational;
}

namespace Graphics {
class Cursor;
struct PixelFormat;
}

namespace Image {
class Codec;
}

namespace Video {

/**
 * Decoder for QuickTime videos.
 *
 * Video decoder used in engines:
 *  - mohawk
 *  - pegasus
 *  - sci
 */
class QuickTimeDecoder : public VideoDecoder, public Audio::QuickTimeAudioDecoder {
public:
	QuickTimeDecoder();
	virtual ~QuickTimeDecoder();

	bool loadFile(const Common::Path &filename);
	bool loadStream(Common::SeekableReadStream *stream);
	void close();
	uint16 getWidth() const { return _width; }
	uint16 getHeight() const { return _height; }
	const Graphics::Surface *decodeNextFrame();
	Audio::Timestamp getDuration() const { return Audio::Timestamp(0, _duration, _timeScale); }

	void enableEditListBoundsCheckQuirk(bool enable) { _enableEditListBoundsCheckQuirk = enable; }
	Common::String getAliasPath();

	void handleMouseMove(int16 x, int16 y);
	void handleMouseButton(bool isDown, int16 x = -1, int16 y = -1);

	float getPanAngle() const { return _panAngle; }
	void setPanAngle(float panAngle) { _panAngle = panAngle; }
	float getTiltAngle() const { return _tiltAngle; }
	void setTiltAngle(float tiltAngle) { _tiltAngle = tiltAngle; }
	float getFOV() const { return _fov; }
	void setFOV(float fov) { _fov = fov; }

	int getCurrentRow() { return _nextVideoTrack->getCurFrame() / _nav.columns; }
	void setCurrentRow(int row);
	int getCurrentColumn() { return _nextVideoTrack->getCurFrame() % _nav.columns; }
	void setCurrentColumn(int column);

	void nudge(const Common::String &direction);

	bool isVR() const { return _isVR; }
	QTVRType getQTVRType() const { return _qtvrType; }

	uint8 getWarpMode() const { return _warpMode; }
	void setWarpMode(uint8 warpMode) { _warpMode = warpMode; }

	struct NodeData {
		uint32 nodeID;

		float defHPan;
		float defVPan;
		float defZoom;

		float minHPan;
		float minVPan;
		float maxHPan;
		float maxVPan;
		float minZoom;

		Common::String name;
	};

	NodeData getNodeData(uint32 nodeID);

protected:
	Common::QuickTimeParser::SampleDesc *readSampleDesc(Common::QuickTimeParser::Track *track, uint32 format, uint32 descSize);

private:
	void init();

	void updateAudioBuffer();

	void closeQTVR();
	void updateAngles();
	void updateQTVRCursor(int16 x, int16 y);
	void setCursor(int curId);
	void cleanupCursors();
	void computeInteractivityZones();

	uint16 _width, _height;

	uint16 _prevMouseX, _prevMouseY;
	bool _isMouseButtonDown;
	Common::Rect _curBbox;

	int _currentQTVRCursor = -1;
	Common::Archive *_dataBundle = nullptr;
	Graphics::Cursor **_cursorCache = nullptr;

	bool _isVR;

	uint8 _warpMode; // (2 | 1 | 0) for 2-d, 1-d or no warping

	float _panAngle = 0.0f;
	float _tiltAngle = 0.0f;
	float _fov = 0.0f;

	Graphics::Surface *_scaledSurface;
	void scaleSurface(const Graphics::Surface *src, Graphics::Surface *dst,
			const Common::Rational &scaleFactorX, const Common::Rational &scaleFactorY);

	bool _enableEditListBoundsCheckQuirk;

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
		Image::Codec *_videoCodec;
	};

	// The AudioTrackHandler is currently just a wrapper around some
	// QuickTimeDecoder functions.
	class AudioTrackHandler : public SeekableAudioTrack {
	public:
		AudioTrackHandler(QuickTimeDecoder *decoder, QuickTimeAudioTrack *audioTrack);

		void updateBuffer();

	protected:
		Audio::SeekableAudioStream *getSeekableAudioStream() const;

	private:
		QuickTimeDecoder *_decoder;
		QuickTimeAudioTrack *_audioTrack;
	};

	// The VideoTrackHandler is the bridge between the time of playback
	// and the media for the given track. It calculates when to start
	// tracks and at what rate to play the media using the edit list.
	class VideoTrackHandler : public VideoTrack {
	public:
		VideoTrackHandler(QuickTimeDecoder *decoder, Common::QuickTimeParser::Track *parent);
		~VideoTrackHandler();

		bool endOfTrack() const;
		bool isSeekable() const { return true; }
		bool seek(const Audio::Timestamp &time);
		Audio::Timestamp getDuration() const;

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		bool setOutputPixelFormat(const Graphics::PixelFormat &format);
		int getCurFrame() const { return _curFrame; }
		void setCurFrame(int32 curFrame) { _curFrame = curFrame; }
		int getFrameCount() const;
		uint32 getNextFrameStartTime() const; // milliseconds
		const Graphics::Surface *decodeNextFrame();
		Audio::Timestamp getFrameTime(uint frame) const;
		const byte *getPalette() const;
		bool hasDirtyPalette() const { return _curPalette; }
		bool setReverse(bool reverse);
		bool isReversed() const { return _reversed; }
		bool canDither() const;
		void setDither(const byte *palette);

		Common::Rational getScaledWidth() const;
		Common::Rational getScaledHeight() const;

	private:
		QuickTimeDecoder *_decoder;
		Common::QuickTimeParser::Track *_parent;
		uint32 _curEdit;
		int32 _curFrame;
		int32 _delayedFrameToBufferTo;
		uint32 _nextFrameStartTime; // media time
		Graphics::Surface *_scaledSurface;
		int32 _durationOverride;    // media time
		const byte *_curPalette;
		mutable bool _dirtyPalette;
		bool _reversed;

		// Forced dithering of frames
		byte *_forcedDitherPalette;
		byte *_ditherTable;
		Graphics::Surface *_ditherFrame;
		const Graphics::Surface *forceDither(const Graphics::Surface &frame);

		Common::SeekableReadStream *getNextFramePacket(uint32 &descId);
		uint32 getCurFrameDuration();            // media time
		uint32 findKeyFrame(uint32 frame) const;
		bool isEmptyEdit() const;
		void enterNewEditListEntry(bool bufferFrames, bool intializingTrack = false);
		const Graphics::Surface *bufferNextFrame();
		uint32 getRateAdjustedFrameTime() const; // media time
		uint32 getCurEditTimeOffset() const;     // media time
		uint32 getCurEditTrackDuration() const;  // media time
		bool atLastEdit() const;
		bool endOfCurEdit() const;
		void checkEditListBounds();
	};

	class PanoTrackHandler {
	public:
		PanoTrackHandler(QuickTimeDecoder *decoder, Common::QuickTimeParser::Track *parent);
		~PanoTrackHandler();

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		bool setOutputPixelFormat(const Graphics::PixelFormat &format);
		const Graphics::Surface *decodeNextFrame();
		const byte *getPalette() const;
		bool hasDirtyPalette() const { return _curPalette; }
		bool canDither() const;
		void setDither(const byte *palette);

		Common::Rational getScaledWidth() const;
		Common::Rational getScaledHeight() const;

	private:
		QuickTimeDecoder *_decoder;
		Common::QuickTimeParser::Track *_parent;

		const byte *_curPalette;

		void constructPanorama();
		void projectPanorama();

		const Graphics::Surface *bufferNextFrame();

		Graphics::Surface *_constructedPano;
		Graphics::Surface *_projectedPano;

		bool _isPanoConstructed;
	};
};

} // End of namespace Video

#endif
