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

#ifndef SCI_GRAPHICS_VIDEO32_H
#define SCI_GRAPHICS_VIDEO32_H

#ifdef USE_RGB_COLOR
#include "common/config-manager.h" // for ConfMan
#endif
#include "common/ptr.h"
#include "common/rect.h"          // for Rect
#include "common/scummsys.h"      // for int16, uint8, uint16, int32
#include "common/str.h"           // for String
#include "sci/engine/vm_types.h"  // for reg_t
#include "sci/video/robot_decoder.h" // for RobotDecoder
#include "sci/sound/audio32.h"    // for Audio32::kMaxVolume
#include "video/avi_decoder.h"    // for AVIDecoder::setVolume

namespace Video {
class AdvancedVMDDecoder;
}
namespace Sci {
class EventManager;
class Plane;
class ScreenItem;
class SegManager;
class SEQDecoder;
struct Palette;

/**
 * An abstract class implementing common video playback functionality for SCI
 * engine.
 */
class VideoPlayer {
public:
	enum EventFlags {
		kEventFlagNone         = 0,
		kEventFlagEnd          = 1,
		kEventFlagEscapeKey    = 2,
		kEventFlagMouseDown    = 4,
		kEventFlagHotRectangle = 8,
		kEventFlagToFrame      = 0x10,
		kEventFlagYieldToVM    = 0x20,
		kEventFlagReverse      = 0x80
	};

	friend EventFlags operator|(const EventFlags a, const EventFlags b) {
		return static_cast<EventFlags>((int)a | (int)b);
	}

	VideoPlayer(EventManager *eventMan, Video::VideoDecoder *decoder = nullptr) :
		_eventMan(eventMan),
		_decoder(decoder),
		_needsUpdate(false),
		_currentFrame(nullptr)
#ifdef USE_RGB_COLOR
		,
		_hqVideoMode(false)
#endif
		{}

	virtual ~VideoPlayer() {}

protected:
	EventManager *_eventMan;

	/**
	 * The video decoder to use for video playback by this player.
	 */
	Common::ScopedPtr<Video::VideoDecoder> _decoder;

	/**
	 * Attempts to open a video by filename and performs basic validation to
	 * ensure that the current system is actually capable of playing back the
	 * video.
	 */
	bool open(const Common::String &fileName);

	/**
	 * Reinitializes the system hardware surface for playback of high-quality
	 * scaled video if the current video meets the necessary criteria for this
	 * playback mode.
	 *
	 * @returns whether or not the system surface was reinitialized for
	 * high-quality scaled video.
	 */
	bool startHQVideo();

	/**
	 * Determines whether or not the currently loaded video meets the criteria
	 * for high-quality scaled output.
	 */
	virtual bool shouldStartHQVideo() const {
#ifdef USE_RGB_COLOR
		if (!ConfMan.getBool("enable_hq_video")) {
			return false;
		}

		if (_decoder->getWidth() == _drawRect.width() &&
			_decoder->getHeight() == _drawRect.height()) {
			return false;
		}

		return true;
#else
		return false;
#endif
	}

	/**
	 * Restores the hardware surface back to CLUT8 after video playback.
	 */
	bool endHQVideo();

	/**
	 * Plays a video until an event in the given `flags` is encountered, or
	 * until the end of the video is reached.
	 *
	 * @param maxSleepMs An optional parameter defining the maximum number of
	 * milliseconds that the video player should sleep between video frames.
	 */
	virtual EventFlags playUntilEvent(const EventFlags flags, const uint32 maxSleepMs = 0xFFFFFFFF);

	/**
	 * Checks to see if an event has occurred that should cause the video player
	 * to yield back to the VM.
	 */
	virtual EventFlags checkForEvent(const EventFlags flags);

	/**
	 * Submits a palette from the video to the system.
	 */
	virtual void submitPalette(const uint8 palette[256 * 3]) const;

	/**
	 * Renders a video frame to the system.
	 */
	virtual void renderFrame(const Graphics::Surface &nextFrame) const;

	/**
	 * Renders a video frame to an intermediate surface using low-quality
	 * scaling, black-lining, or direct copy, depending upon the passed flags.
	 */
	template <typename PixelType>
	void renderLQToSurface(Graphics::Surface &out, const Graphics::Surface &nextFrame, const bool doublePixels, const bool blackLines) const;

	/**
	 * Sets the draw rect, clipping it to the screen's dimensions if necessary.
	 */
	void setDrawRect(const int16 x, const int16 y, const int16 width, const int16 height);

	/**
	 * The rectangle where the video will be drawn, in screen coordinates.
	 */
	Common::Rect _drawRect;

	/**
	 * If true, playUntilEvent() will immediately render a frame.
	 * Used by VMDPlayer when censorship blobs are added or removed in Phant1
	 * in order to immediately update the screen upon resuming playback.
	 */
	bool _needsUpdate;

	/**
	 * Current frame rendered by playUntilEvent() 
	 */
	const Graphics::Surface* _currentFrame;

#ifdef USE_RGB_COLOR
	/**
	 * Whether or not the player is currently in high-quality video rendering
	 * mode.
	 */
	bool _hqVideoMode;
#endif
};

#pragma mark SEQPlayer

/**
 * SEQPlayer is used to play SEQ animations.
 * Used by DOS versions of GK1 and QFG4CD.
 */
class SEQPlayer : public VideoPlayer {
public:
	SEQPlayer(EventManager *eventMan);

	/**
	 * Plays a SEQ animation with the given file name, with each frame being
	 * displayed for `numTicks` ticks.
	 */
	void play(const Common::String &fileName, const int16 numTicks, const int16 x, const int16 y);
};

#pragma mark -
#pragma mark AVIPlayer

/**
 * AVIPlayer is used to play AVI videos.
 * Used by Windows versions of GK1CD, KQ7, and QFG4CD.
 */
class AVIPlayer : public VideoPlayer {
public:
	enum IOStatus {
		kIOSuccess      = 0,
		kIOFileNotFound = 2,
		kIOSeekFailed   = 12
	};

	enum AVIStatus {
		kAVINotOpen  = 0,
		kAVIOpen     = 1,
		kAVIPlaying  = 2,
		kAVIPaused   = 3
	};

	AVIPlayer(EventManager *eventMan);

	/**
	 * Opens a stream to an AVI resource.
	 */
	IOStatus open(const Common::String &fileName);

	/**
	 * Initializes the AVI rendering parameters for the current AVI. This must
	 * be called after `open`.
	 */
	IOStatus init(const bool doublePixels);

	/**
	 * Begins playback of the current AVI.
	 */
	IOStatus play(const int16 from, const int16 to, const int16 showStyle, const bool cue);

	EventFlags playUntilEvent(const EventFlags flags, const uint32 maxSleepMs = 0xFFFFFFFF) override;

	/**
	 * Stops playback and closes the currently open AVI stream.
	 */
	IOStatus close();

	/**
	 * Seeks the currently open AVI stream to the given frame.
	 */
	IOStatus cue(const uint16 frameNo);

	/**
	 * Returns the duration of the current video.
	 */
	uint16 getDuration() const;

private:
	/**
	 * Playback status of the player.
	 */
	AVIStatus _status;
};

#pragma mark -
#pragma mark QuickTimePlayer

/**
 * QuickTimePlayer is used to play QuickTime animations.
 * Used by Mac version of KQ7.
 */
class QuickTimePlayer : public VideoPlayer {
public:
	QuickTimePlayer(EventManager *eventMan);
	
	/**
	 * Plays a QuickTime animation with the given file name
	 */
	void play(const Common::String& fileName);
};

#pragma mark -
#pragma mark VMDPlayer

/**
 * VMDPlayer is used to play VMD videos.
 * Used by LSL7, Phant1, GK2, PQ:SWAT, Shivers, SQ6, Rama, Torin, and
 * Lighthouse.
 */
class VMDPlayer : public VideoPlayer {
public:
	enum OpenFlags {
		kOpenFlagNone = 0,
		kOpenFlagMute = 1
	};

	enum IOStatus {
		kIOSuccess = 0,
		kIOError   = 0xFFFF
	};

	enum PlayFlags {
		kPlayFlagNone             = 0,
		kPlayFlagDoublePixels     = 1,
		kPlayFlagBlackLines       = 4,
		kPlayFlagBoost            = 0x10,
		kPlayFlagLeaveScreenBlack = 0x20,
		kPlayFlagLeaveLastFrame   = 0x40,
		kPlayFlagBlackPalette     = 0x80,
		kPlayFlagStretchVertical  = 0x100
	};

	enum VMDStatus {
		kVMDNotOpen  = 0,
		kVMDOpen     = 1,
		kVMDPlaying  = 2,
		kVMDPaused   = 3,
		kVMDStopped  = 4,
		kVMDFinished = 5
	};

	VMDPlayer(EventManager *eventMan, SegManager *segMan);
	~VMDPlayer() override;

private:
	SegManager *_segMan;

#pragma mark -
#pragma mark VMDPlayer - Playback
public:
	/**
	 * Opens a stream to a VMD resource.
	 */
	IOStatus open(const Common::String &fileName, const OpenFlags flags);

	/**
	 * Initializes the VMD rendering parameters for the current VMD. This must
	 * be called after `open`.
	 */
	void init(int16 x, int16 y, const PlayFlags flags, const int16 boostPercent, const int16 boostStartColor, const int16 boostEndColor);

	/**
	 * Stops playback and closes the currently open VMD stream.
	 */
	IOStatus close();

	/**
	 * Gets the playback status of the VMD player.
	 */
	VMDStatus getStatus() const;

	// Was WaitForEvent in SSCI
	EventFlags kernelPlayUntilEvent(const EventFlags flags, const int16 lastFrameNo, const int16 yieldInterval);

private:
	/**
	 * Whether or not a VMD stream has been opened with `open`.
	 */
	bool _isOpen;

	/**
	 * Whether or not a VMD player has been initialized with `init`.
	 */
	bool _isInitialized;

	/**
	 * The Resource object for VMDs that are read out of a resource bundle
	 * instead of being streamed from the filesystem. The resource is owned by
	 * ResourceManager.
	 */
	Resource *_bundledVmd;

	/**
	 * For VMDs played with the `kEventFlagToFrame` flag, the target frame for
	 * yielding back to the SCI VM.
	 */
	int32 _yieldFrame;

	/**
	 * For VMDs played with the `kEventFlagYieldToVM` flag, the number of frames
	 * that should be rendered until yielding back to the SCI VM.
	 */
	int32 _yieldInterval;

	/**
	 * For VMDs played with the `kEventFlagYieldToVM` flag, the last frame when
	 * control of the main thread was yielded back to the SCI VM.
	 */
	int _lastYieldedFrameNo;

	EventFlags playUntilEvent(const EventFlags flags, const uint32 = 0xFFFFFFFF) override;
	EventFlags checkForEvent(const EventFlags flags) override;

#pragma mark -
#pragma mark VMDPlayer - Rendering
public:
	/**
	 * Causes the VMD player to ignore all palettes in the currently playing
	 * video.
	 */
	void ignorePalettes() { _ignorePalettes = true; }

	/**
	 * Sets the plane and plane priority used to render video.
	 */
	void setPlane(const int16 priority, const reg_t planeId);

protected:
	/**
	 * Renders a frame of video to the output bitmap.
	 */
	void renderFrame(const Graphics::Surface &nextFrame) const override;

	/**
	 * Updates the system with palette data from the video.
	 */
	void submitPalette(const uint8 palette[256 * 3]) const override;

private:
	/**
	 * The plane where the VMD will be drawn. The plane is owned by GfxFrameout.
	 */
	Plane *_plane;

	/**
	 * The screen item representing the VMD surface. The screen item is owned by
	 * GfxFrameout.
	 */
	ScreenItem *_screenItem;

	/**
	 * The bitmap used to render the VMD.
	 */
	reg_t _bitmapId;

	/**
	 * If true, the plane for this VMD was set externally and is not owned by
	 * this VMDPlayer.
	 */
	bool _planeIsOwned;

	/**
	 * The screen priority of the video.
	 * @see ScreenItem::_priority
	 */
	int _priority;

	/**
	 * Whether or not the video should be pixel doubled.
	 */
	bool _doublePixels;

	/**
	 * Whether or not the video should be pixel doubled vertically only.
	 */
	bool _stretchVertical;

	/**
	 * Whether or not black lines should be rendered across the video.
	 */
	bool _blackLines;

	/**
	 * Whether or not the playback area of the VMD should be left black at the
	 * end of playback.
	 */
	bool _leaveScreenBlack;

	/**
	 * Whether or not the area of the VMD should be left displaying the final
	 * frame of the video.
	 */
	bool _leaveLastFrame;

	/**
	 * Whether or not palettes from the VMD should be ignored.
	 */
	bool _ignorePalettes;

	/**
	 * Whether or not rendering mode is composited.
	 */
	bool _isComposited;

	/**
	 * Fills the given palette with RGB values from the VMD palette, applying
	 * brightness boost if it is enabled.
	 */
	void fillPalette(const uint8 rawPalette[256 * 3], Palette &outPalette) const;

#ifdef USE_RGB_COLOR
	/**
	 * Redraws areas of the screen outside of the video to the system buffer.
	 * This is used whenever palette changes occur and the video is rendering in
	 * high color mode.
	 */
	void redrawGameScreen() const;

	/**
	 * Determines whether or not the VMD player should upgrade the renderer to
	 * high color depth when rendering the video.
	 *
	 * @TODO It should be possible in the future to allow high color composited
	 * video, but this will require additional work in GfxFrameout and
	 * GfxCursor32 since the internal buffer and cursor code are 8bpp only.
	 */
	bool shouldStartHQVideo() const override {
		if (!VideoPlayer::shouldStartHQVideo()) {
			return false;
		}

		if (_priority != 0 || _leaveLastFrame || _showCursor || _blackLines) {
			return false;
		}

		return true;
	}
#endif

	/**
	 * Determines whether or not the video should use the compositing renderer
	 * instead of the overlay renderer.
	 */
	bool shouldUseCompositing() const {
#ifdef USE_RGB_COLOR
		return isNormallyComposited() && !shouldStartHQVideo();
#else
		return isNormallyComposited();
#endif
	}

	bool isNormallyComposited() const {
		return (getSciVersion() == SCI_VERSION_3) || 
				(g_sci->getPlatform() == Common::kPlatformMacintosh &&
				 getSciVersion() >= SCI_VERSION_2_1_LATE);
	}

	void initOverlay();
	void renderOverlay(const Graphics::Surface &nextFrame) const;
	void closeOverlay();

	void initComposited();
	void renderComposited() const;
	void closeComposited();

#pragma mark -
#pragma mark VMDPlayer - Blackout
public:
	/**
	 * Sets the area of the screen that should be blacked out during VMD
	 * playback.
	 */
	void setBlackoutArea(const Common::Rect &rect) { _blackoutRect = rect; }

private:
	/**
	 * The dimensions of the blackout plane.
	 */
	Common::Rect _blackoutRect;

	/**
	 * An optional plane that will be used to black out areas of the screen
	 * outside of the VMD surface. The plane is owned by GfxFrameout.
	 */
	Plane *_blackoutPlane;

#pragma mark -
#pragma mark VMDPlayer - Palette
public:
	/**
	 * Restricts use of the system palette by VMD playback to the given range of
	 * palette indexes.
	 */
	void restrictPalette(const uint8 startColor, const int16 endColor);

private:
	/**
	 * The first color in the system palette that the VMD can write to.
	 */
	uint8 _startColor;

	/**
	 * The last color in the system palette that the VMD can write to.
	 */
	uint8 _endColor;

	/**
	 * If true, video frames are rendered after a blank palette is submitted to
	 * the palette manager, which is then restored after the video pixels have
	 * already been rendered.
	 *
	 * This functionality is currently disabled because it seems like it was
	 * designed for a different graphics architecture where pixel data could be
	 * rendered before the video card's palette had been updated. This is not
	 * possible in ScummVM because the palette & pixel data are rendered
	 * simultaneously when OSystem::updateScreen is called, rather than
	 * immediately after they are sent to the backend.
	 */
#ifdef SCI_VMD_BLACK_PALETTE
	bool _blackPalette;
#endif

#pragma mark -
#pragma mark VMDPlayer - Brightness boost
private:
	/**
	 * The amount of brightness boost for the video. Values above 100 increase
	 * brightness; values below 100 reduce it.
	 */
	int16 _boostPercent;

	/**
	 * The first color in the palette that should be brightness boosted.
	 */
	uint8 _boostStartColor;

	/**
	 * The last color in the palette that should be brightness boosted.
	 */
	uint8 _boostEndColor;

#pragma mark -
#pragma mark VMDPlayer - Mouse cursor
public:
	/**
	 * Sets whether or not the mouse cursor should be drawn. This does not have
	 * any effect during playback, but can be used to prevent the mouse cursor
	 * from being shown again after the video has finished.
	 */
	void setShowCursor(const bool shouldShow) { _showCursor = shouldShow; }

private:
	/**
	 * Whether or not the mouse cursor should be shown during playback.
	 */
	bool _showCursor;

#pragma mark -
#pragma mark VMDPlayer - Censorship blobs
public:
	/**
	 * Censorship blobs are pixelated rectangles which are added and removed by
	 * game scripts. Phant1 is the only game known to use this and always sets a
	 * blockSize of 10. Each block's color comes from the pixel in the upper left
	 * corner of the block's location.
	 */
	int16 addBlob(int16 blockSize, int16 top, int16 left, int16 bottom, int16 right);
	void deleteBlobs();
	void deleteBlob(int16 blobNumber);

private:
	enum {
		kMaxBlobs = 10
	};

	struct Blob {
		int16 blobNumber;
		int16 blockSize;
		int16 top;
		int16 left;
		int16 bottom;
		int16 right;
	};

	Common::List<Blob> _blobs;

	void drawBlobs(Graphics::Surface& frame) const;
};

#pragma mark -
#pragma mark DuckPlayer

/**
 * DuckPlayer is used to play Duck TrueMotion videos.
 * Used by Phantasmagoria 2.
 */
class DuckPlayer : public VideoPlayer {
public:
	enum DuckStatus {
		kDuckClosed  = 0,
		kDuckOpen    = 1,
		kDuckPlaying = 2,
		kDuckPaused  = 3
	};

	DuckPlayer(EventManager *eventMan, SegManager *segMan);

	/**
	 * Opens a stream to a Duck resource.
	 */
	void open(const GuiResourceId resourceId, const int displayMode, const int16 x, const int16 y);

	/**
	 * Stops playback and closes the currently open Duck stream.
	 */
	void close();

	/**
	 * Begins playback of the current Duck video.
	 */
	void play(const int lastFrameNo);

	/**
	 * Sets a flag indicating that an opaque plane should be added to the
	 * graphics manager underneath the video surface during playback.
	 */
	void setDoFrameOut(const bool value) { _doFrameOut = value; }

	/**
	 * Sets the volume of the decoder.
	 */
	void setVolume(const uint8 value) {
		_volume = value * Audio::Mixer::kMaxChannelVolume / Audio32::kMaxVolume;
		_decoder->setVolume(_volume);
	}

protected:
	bool shouldStartHQVideo() const override {
		if (!VideoPlayer::shouldStartHQVideo() || _blackLines) {
			return false;
		}

		return true;
	}

	void renderFrame(const Graphics::Surface &nextFrame) const override;

private:
	/**
	 * An empty plane drawn behind the video when the doFrameOut flag is true.
	 * The plane is owned by GfxFrameout.
	 */
	Plane *_plane;

	/**
	 * The player status.
	 */
	DuckStatus _status;

	/**
	 * The playback volume for the player.
	 */
	uint8 _volume;

	/**
	 * If true, frameOut will be called during Duck video playback to update
	 * other parts of the screen.
	 */
	bool _doFrameOut;

	/**
	 * Whether or not the video should be pixel doubled.
	 */
	bool _doublePixels;

	/**
	 * Whether or not black lines should be rendered across the video.
	 */
	bool _blackLines;
};

#pragma mark -
#pragma mark Video32

/**
 * Video32 provides facilities for playing back video in SCI engine.
 */
class Video32 : public Common::Serializable {
public:
	Video32(SegManager *segMan, EventManager *eventMan) :
	_SEQPlayer(eventMan),
	_AVIPlayer(eventMan),
	_QuickTimePlayer(eventMan),
	_VMDPlayer(eventMan, segMan),
	_robotPlayer(segMan),
	_duckPlayer(eventMan, segMan) {}

	void beforeSaveLoadWithSerializer(Common::Serializer &ser);
	void saveLoadWithSerializer(Common::Serializer &ser) override;

	SEQPlayer &getSEQPlayer() { return _SEQPlayer; }
	AVIPlayer &getAVIPlayer() { return _AVIPlayer; }
	QuickTimePlayer &getQuickTimePlayer() { return _QuickTimePlayer; }
	VMDPlayer &getVMDPlayer() { return _VMDPlayer; }
	RobotDecoder &getRobotPlayer() { return _robotPlayer; }
	DuckPlayer &getDuckPlayer() { return _duckPlayer; }

private:
	SEQPlayer _SEQPlayer;
	AVIPlayer _AVIPlayer;
	QuickTimePlayer _QuickTimePlayer;
	VMDPlayer _VMDPlayer;
	RobotDecoder _robotPlayer;
	DuckPlayer _duckPlayer;
};
} // End of namespace Sci

#endif
