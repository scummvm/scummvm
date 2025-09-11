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
#include "common/keyboard.h"
#include "common/scummsys.h"

#include "graphics/palette.h"
#include "graphics/transform_tools.h"

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

	////////////////
	// QTVR stuff
	////////////////
	void setTargetSize(uint16 w, uint16 h);
	void setOrigin(int left, int top) { _origin = Common::Point(left, top); }

	void handleMouseMove(int16 x, int16 y);
	void handleMouseButton(bool isDown, int16 x = -1, int16 y = -1, bool repeat = false);
	void handleKey(Common::KeyState &state, bool down, bool repeat = false);
	void handleQuit();

	Common::Point getLastClick() { return _mouseDrag; }

	float getPanAngle() const { return _panAngle; }
	void setPanAngle(float panAngle);
	float getTiltAngle() const { return _tiltAngle; }
	void setTiltAngle(float tiltAngle);
	float getFOV() const { return _fov; }
	float getHFOV() const { return _hfov; }
	bool setFOV(float fov);
	int getCurrentNodeID() { return _currentSample == -1 ? 0 : _panoTrack->panoSamples[_currentSample].hdr.nodeID; }
	Common::String getCurrentNodeName();

	int getCurrentRow() { return _nextVideoTrack->getCurFrame() / _nav.columns; }
	void setCurrentRow(int row);
	int getCurrentColumn() { return _nextVideoTrack->getCurFrame() % _nav.columns; }
	void setCurrentColumn(int column);

	int getZoomState() { return _zoomState; }

	const PanoHotSpot *getRolloverHotspot() { return _rolloverHotspot; }
	int getRolloverHotspotID() { return _rolloverHotspotID; }
	const PanoHotSpot *getClickedHotspot() { return _clickedHotspot; }
	int getClickedHotspotID() { return _clickedHotspotID; }
	Common::Point getPanLoc(int16 x, int16 y);
	Graphics::FloatPoint getPanAngles(int16 x, int16 y);

	Common::String getHotSpotName(int id);
	void setClickedHotSpot(int id);
	const PanoHotSpot *getHotSpotByID(int id);
	const PanoNavigation *getHotSpotNavByID(int id);

	void nudge(const Common::String &direction);

	bool isVR() const { return _isVR; }
	QTVRType getQTVRType() const { return _qtvrType; }

	int getWarpMode() const { return _warpMode; }
	void setWarpMode(int warpMode);
	float getQuality() const { return _quality; }
	void setQuality(float quality);
	Common::String getTransitionMode() const { return _transitionMode == kTransitionModeNormal ? "normal" : "swing"; }
	void setTransitionMode(Common::String mode);
	float getTransitionSpeed() const { return _transitionSpeed; }
	void setTransitionSpeed(float speed);
	Common::String getUpdateMode() const;
	void setUpdateMode(Common::String mode);

	void renderHotspots(bool mode);

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
	void goToNode(uint32 nodeID);

protected:
	Common::QuickTimeParser::SampleDesc *readSampleDesc(Common::QuickTimeParser::Track *track, uint32 format, uint32 descSize);
	Common::QuickTimeParser::SampleDesc *readPanoSampleDesc(Common::QuickTimeParser::Track *track, uint32 format, uint32 descSize);

private:
	void init();

	void updateAudioBuffer();

	void handleObjectMouseMove(int16 x, int16 y);
	void handleObjectMouseButton(bool isDown, int16 x, int16 y, bool repeat);
	void handlePanoMouseMove(int16 x, int16 y);
	void handlePanoMouseButton(bool isDown, int16 x, int16 y, bool repeat);

	void handleObjectKey(Common::KeyState &state, bool down, bool repeat);
	void handlePanoKey(Common::KeyState &state, bool down, bool repeat);

	void closeQTVR();
	void updateAngles();
	void lookupHotspot(int16 x, int16 y);
	void updateQTVRCursor(int16 x, int16 y);
	void setCursor(int curId);
	void cleanupCursors();
	void computeInteractivityZones();

	uint16 _width, _height;
	// _origin is the top left corner point of the panorama video being played
	// by director engine or whichever engine is using QTVR decoder currently
	// decoder handles swing transitions (in QTVR xtra) internally
	// Hence, it needs to know where to blit the projected panorama during transition
	Common::Point _origin;

public:
	int _currentSample = -1;
	Common::Point _prevMouse;
	bool _isMouseButtonDown = false;
	Common::Point _mouseDrag;

	bool _isKeyDown = false;
	Common::KeyState _lastKey;

	enum {
		kZoomNone,
		kZoomQuestion,
		kZoomIn,
		kZoomOut,
		kZoomLimit,

		kTransitionModeNormal,
		kTransitionModeSwing,

		kUpdateModeNormal,
		kUpdateModeUpdateBoth,
		kUpdateModeOffscreenOnly,
		kUpdateModeFromOffscreen,
		kUpdateModeDirectToScreen,
	};

private:
	Common::Rect _curBbox;

	int _currentQTVRCursor = -1;
	Common::Archive *_dataBundle = nullptr;
	Graphics::Cursor **_cursorCache = nullptr;
	int _cursorDirMap[256];

	bool _isVR = false;

	uint8 _warpMode = 2; // (2 | 1 | 0) for 2-d, 1-d or no warping
	float _quality = 0.0f;
	int _transitionMode = kTransitionModeNormal;
	float _transitionSpeed = 1.0f;
	int _updateMode = kUpdateModeNormal;

	float _panAngle = 0.0f;
	float _tiltAngle = 0.0f;
	float _fov = 56.0f;
	float _hfov = 56.0f;
	int _zoomState = kZoomNone;

	const PanoHotSpot *_rolloverHotspot = nullptr;
	int _rolloverHotspotID = 0;
	const PanoHotSpot *_clickedHotspot = nullptr;
	int _clickedHotspotID = 0;
	bool _renderHotspots = false;

	Graphics::Surface *_scaledSurface;
	void scaleSurface(const Graphics::Surface *src, Graphics::Surface *dst,
			const Common::Rational &scaleFactorX, const Common::Rational &scaleFactorY);

	bool _enableEditListBoundsCheckQuirk;

	bool _cursorDirty;
	Common::Point _cursorPos;

	class VideoSampleDesc : public Common::QuickTimeParser::SampleDesc {
	public:
		VideoSampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag);
		~VideoSampleDesc();

		void initCodec();

		// TODO: Make private in the long run
		uint16 _bitsPerSample;
		char _codecName[32];
		uint16 _colorTableId;
		Graphics::Palette _palette;
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

	class PanoSampleDesc : public Common::QuickTimeParser::SampleDesc {
	public:
		PanoSampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag);
		~PanoSampleDesc();

		int16 _majorVersion;		// must be zero, also observed to be 1
		int16 _minorVersion;		// must be zero, also observed to be 1

		int32 _sceneTrackID;		// ID of video track that contains panoramic scene
		int32 _loResSceneTrackID;	// ID of video track that contains low res panoramic scene
		byte _reserved3[4 * 6];		// must be zero
		int32 _hotSpotTrackID;		// ID of video track that contains hotspot image
		byte _reserved4[4 * 9];		// must be zero

		float _hPanStart;			// horizontal pan range start angle (e.g. 0)
		float _hPanEnd;				// horizontal pan range end angle (e.g. 360)
		float _vPanTop;				// vertical pan range top angle (e.g. 42.5)
		float _vPanBottom;			// vertical pan range bottom angle (e.g. -42.5)
		float _minimumZoom;			// minimum zoom angle (e.g. 5; use 0 for default)
		float _maximumZoom;			// maximum zoom angle (e.g. 65; use 0 for default)

		// info for the highest res version of scene track
		uint32 _sceneSizeX;			// pixel width of the panorama (e.g. 768)
		uint32 _sceneSizeY;			// pixel height of the panorama (e.g. 2496)
		uint32 _numFrames;			// number of diced frames (e.g. 24)
		int16 _reserved5;			// must be zero
		int16 _sceneNumFramesX;		// diced frames wide (e.g. 1)
		int16 _sceneNumFramesY;		// diced frames high (e.g. 24)
		int16 _sceneColorDepth;		// bit depth of the scene track (e.g. 32)

		// info for the highest rest version of hotSpot track
		int32 _hotSpotSizeX;		// pixel width of the hot spot panorama (e.g. 768)
		int32 _hotSpotSizeY;		// pixel height of the hot spot panorama  (e.g. 2496)
		int16 _reserved6;			// must be zero
		int16 _hotSpotNumFramesX;	// diced frames wide (e.g. 1)
		int16 _hotSpotNumFramesY;	// diced frames high (e.g. 24)
		int16 _hotSpotColorDepth;	// must be 8
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
		bool hasDirtyPalette() const { return _dirtyPalette; }
		bool setReverse(bool reverse);
		bool isReversed() const { return _reversed; }
		bool canDither() const;
		void setDither(const byte *palette);

		Common::Rational getScaledWidth() const;
		Common::Rational getScaledHeight() const;

		const Graphics::Surface *bufferNextFrame();

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

		Common::SeekableReadStream *getNextFramePacket(uint32 &descId);
		uint32 getCurFrameDuration();            // media time
		uint32 findKeyFrame(uint32 frame) const;
		bool isEmptyEdit() const;
		void enterNewEditListEntry(bool bufferFrames, bool intializingTrack = false);
		uint32 getRateAdjustedFrameTime() const; // media time
		uint32 getCurEditTimeOffset() const;     // media time
		uint32 getCurEditTrackDuration() const;  // media time
		bool atLastEdit() const;
		bool endOfCurEdit() const;
		void checkEditListBounds();
	};

	class PanoTrackHandler : public VideoTrack {
	public:
		PanoTrackHandler(QuickTimeDecoder *decoder, Common::QuickTimeParser::Track *parent);
		~PanoTrackHandler();

		bool endOfTrack() const { return false; }
		uint16 getWidth() const;
		uint16 getHeight() const;
		int getCurFrame() const { return 1; }
		uint32 getNextFrameStartTime() const { return 0; }
		Graphics::PixelFormat getPixelFormat() const;
		const Graphics::Surface *decodeNextFrame();

		Common::Rational getScaledWidth() const;
		Common::Rational getScaledHeight() const;

		void initPanorama();
		void constructPanorama();
		Graphics::Surface *constructMosaic(VideoTrackHandler *track, uint w, uint h, Common::String fname);

		Common::Point projectPoint(int16 x, int16 y);

		void setDirty() { _dirty = true; }

	private:
		QuickTimeDecoder *_decoder;
		Common::QuickTimeParser::Track *_parent;

		void projectPanorama(uint8 scaleFactor, float fov, float hfov, float panAngle, float tiltAngle);
		void swingTransitionHandler();
		void boxAverage(Graphics::Surface *sourceSurface, uint8 scaleFactor);
		Graphics::Surface* upscalePanorama(Graphics::Surface *sourceSurface, int8 level);

		const Graphics::Surface *bufferNextFrame();

	public:
		Graphics::Surface *_constructedPano;
		Graphics::Surface *_upscaledConstructedPano;
		Graphics::Surface *_constructedHotspots;
		Graphics::Surface *_projectedPano;
		Graphics::Surface *_planarProjection;

		// Current upscale level (0 or 1 or 2) of _upscaledConstructedPanorama compared to _constructedPano
		// level 0 means that constructedPano was just contructed and hasn't been upscaled yet
		// level 1 means only upscaled height (2x pixels)
		// level 2 means upscaled height and width (4x pixels)
		uint8 _upscaleLevel = 0;

		// Defining these to make the swing transition happen
		// which requires storing the previous point during every change in FOV, Pan Angle and Tilt Angle
		// If swing transition is called, this will be the start point of the transition
		float _currentFOV = 0;
		float _currentHFOV = 0;
		float _currentPanAngle = 0;
		float _currentTiltAngle = 0;

	private:
		bool _isPanoConstructed;
		bool _dirty;
	};
};

} // End of namespace Video

#endif
