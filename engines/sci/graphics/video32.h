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

#pragma mark SEQPlayer

/**
 * SEQPlayer is used to play SEQ animations.
 * Used by DOS versions of GK1 and QFG4CD.
 */
class SEQPlayer {
public:
	SEQPlayer(SegManager *segMan);

	/**
	 * Plays a SEQ animation with the given
	 * file name, with each frame being displayed
	 * for `numTicks` ticks.
	 */
	void play(const Common::String &fileName, const int16 numTicks, const int16 x, const int16 y);

private:
	SegManager *_segMan;
	SEQDecoder *_decoder;

	/**
	 * The plane where the SEQ will be drawn.
	 */
	Plane *_plane;

	/**
	 * The screen item representing the SEQ surface.
	 */
	ScreenItem *_screenItem;

	/**
	 * Renders a single frame of video.
	 */
	void renderFrame(SciBitmap &bitmap) const;
};

#pragma mark -
#pragma mark AVIPlayer

/**
 * AVIPlayer is used to play AVI videos. Used by
 * Windows versions of GK1CD, KQ7, and QFG4CD.
 */
class AVIPlayer {
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

	enum EventFlags {
		kEventFlagNone         = 0,
		kEventFlagEnd          = 1,
		kEventFlagEscapeKey    = 2,
		kEventFlagMouseDown    = 4,
		kEventFlagHotRectangle = 8
	};

	AVIPlayer(SegManager *segMan, EventManager *eventMan);
	~AVIPlayer();

	/**
	 * Opens a stream to an AVI resource.
	 */
	IOStatus open(const Common::String &fileName);

	/**
	 * Initializes the AVI rendering parameters for the
	 * current AVI. This must be called after `open`.
	 */
	IOStatus init1x(const int16 x, const int16 y, const int16 width, const int16 height);

	/**
	 * Initializes the AVI rendering parameters for the
	 * current AVI, in pixel-doubling mode. This must
	 * be called after `open`.
	 */
	IOStatus init2x(const int16 x, const int16 y);

	/**
	 * Begins playback of the current AVI.
	 */
	IOStatus play(const int16 from, const int16 to, const int16 showStyle, const bool cue);

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

	/**
	 * Plays the AVI until an event occurs (e.g. user
	 * presses escape, clicks, etc.).
	 */
	EventFlags playUntilEvent(const EventFlags flags);

private:
	typedef Common::HashMap<uint16, AVIStatus> StatusMap;

	SegManager *_segMan;
	EventManager *_eventMan;
	Video::AVIDecoder *_decoder;

	/**
	 * Playback status of the player.
	 */
	AVIStatus _status;

	/**
	 * The plane where the AVI will be drawn.
	 */
	Plane *_plane;

	/**
	 * The screen item representing the AVI surface,
	 * in 8bpp mode. In 24bpp mode, video is drawn
	 * directly to the screen.
	 */
	ScreenItem *_screenItem;

	/**
	 * The bitmap used to render video output in
	 * 8bpp mode.
	 */
	reg_t _bitmap;

	/**
	 * The rectangle where the video will be drawn,
	 * in game script coordinates.
	 */
	Common::Rect _drawRect;

	/**
	 * The scale buffer for pixel-doubled videos
	 * drawn in 24bpp mode.
	 */
	void *_scaleBuffer;

	/**
	 * In SCI2.1, whether or not the video should
	 * be pixel doubled for playback.
	 */
	bool _pixelDouble;

	/**
	 * Performs common initialisation for both
	 * scaled and unscaled videos.
	 */
	void init();

	/**
	 * Renders video without event input until the
	 * video is complete.
	 */
	void renderVideo() const;

	/**
	 * Renders a single frame of video.
	 */
	void renderFrame() const;
};

#pragma mark -
#pragma mark VMDPlayer

/**
 * VMDPlayer is used to play VMD videos.
 * Used by Phant1, GK2, PQ:SWAT, Shivers, SQ6,
 * Torin, and Lighthouse.
 */
class VMDPlayer {
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

	enum VMDStatus {
		kVMDNotOpen  = 0,
		kVMDOpen     = 1,
		kVMDPlaying  = 2,
		kVMDPaused   = 3,
		kVMDStopped  = 4,
		kVMDFinished = 5
	};

	VMDPlayer(SegManager *segMan, EventManager *eventMan);
	~VMDPlayer();

private:
	SegManager *_segMan;
	EventManager *_eventMan;
	Video::AdvancedVMDDecoder *_decoder;

#pragma mark -
#pragma mark VMDPlayer - Playback
public:
	/**
	 * Opens a stream to a VMD resource.
	 */
	IOStatus open(const Common::String &fileName, const OpenFlags flags);

	/**
	 * Initializes the VMD rendering parameters for the
	 * current VMD. This must be called after `open`.
	 */
	void init(const int16 x, const int16 y, const PlayFlags flags, const int16 boostPercent, const int16 boostStartColor, const int16 boostEndColor);

	/**
	 * Stops playback and closes the currently open VMD stream.
	 */
	IOStatus close();

	/**
	 * Gets the playback status of the VMD player.
	 */
	VMDStatus getStatus() const;

	// NOTE: Was WaitForEvent in SSCI
	EventFlags kernelPlayUntilEvent(const EventFlags flags, const int16 lastFrameNo, const int16 yieldInterval);

private:
	/**
	 * Whether or not a VMD stream has been opened with
	 * `open`.
	 */
	bool _isOpen;

	/**
	 * Whether or not a VMD player has been initialised
	 * with `init`.
	 */
	bool _isInitialized;

	/**
	 * The Resource object for VMDs that are read out
	 * of a resource bundle instead of being streamed
	 * from the filesystem.
	 */
	Resource *_bundledVmd;

	/**
	 * For VMDs played with the `kEventFlagToFrame` flag,
	 * the target frame for yielding back to the SCI VM.
	 */
	int32 _yieldFrame;

	/**
	 * For VMDs played with the `kEventFlagYieldToVM` flag,
	 * the number of frames that should be rendered until
	 * yielding back to the SCI VM.
	 */
	int32 _yieldInterval;

	/**
	 * For VMDs played with the `kEventFlagYieldToVM` flag,
	 * the last frame when control of the main thread was
	 * yielded back to the SCI VM.
	 */
	int _lastYieldedFrameNo;

	/**
	 * Plays the VMD until an event occurs (e.g. user
	 * presses escape, clicks, etc.).
	 */
	EventFlags playUntilEvent(const EventFlags flags);

#pragma mark -
#pragma mark VMDPlayer - Rendering
public:
	/**
	 * Causes the VMD player to ignore all palettes in
	 * the currently playing video.
	 */
	void ignorePalettes() { _ignorePalettes = true; }

	/**
	 * Sets the plane and plane priority used to render video.
	 */
	void setPlane(const int16 priority, const reg_t planeId);

private:
	/**
	 * The location of the VMD plane, in game script
	 * coordinates.
	 */
	int16 _x, _y;

	/**
	 * The plane where the VMD will be drawn.
	 */
	Plane *_plane;

	/**
	 * The screen item representing the VMD surface.
	 */
	ScreenItem *_screenItem;

	/**
	 * The bitmap used to render the VMD.
	 */
	reg_t _bitmapId;

	// TODO: planeIsOwned and priority are used in SCI3+ only

	/**
	 * If true, the plane for this VMD was set
	 * externally and is not owned by this VMDPlayer.
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
	 * Whether or not the video should be pixel doubled
	 * vertically only.
	 */
	bool _stretchVertical;

	/**
	 * Whether or not black lines should be rendered
	 * across the video.
	 */
	bool _blackLines;

	/**
	 * Whether or not the playback area of the VMD
	 * should be left black at the end of playback.
	 */
	bool _leaveScreenBlack;

	/**
	 * Whether or not the area of the VMD should be left
	 * displaying the final frame of the video.
	 */
	bool _leaveLastFrame;

	/**
	 * Whether or not palettes from the VMD should be ignored.
	 */
	bool _ignorePalettes;

	/**
	 * Renders a frame of video to the output bitmap.
	 */
	void renderFrame() const;

	/**
	 * Fills the given palette with RGB values from
	 * the VMD palette, applying brightness boost if
	 * it is enabled.
	 */
	void fillPalette(Palette &palette) const;

#pragma mark -
#pragma mark VMDPlayer - Blackout
public:
	/**
	 * Sets the area of the screen that should be blacked out
	 * during VMD playback.
	 */
	void setBlackoutArea(const Common::Rect &rect) { _blackoutRect = rect; }

private:
	/**
	 * The dimensions of the blackout plane.
	 */
	Common::Rect _blackoutRect;

	/**
	 * An optional plane that will be used to black out
	 * areas of the screen outside of the VMD surface.
	 */
	Plane *_blackoutPlane;

#pragma mark -
#pragma mark VMDPlayer - Palette
public:
	/**
	 * Restricts use of the system palette by VMD playback to
	 * the given range of palette indexes.
	 */
	void restrictPalette(const uint8 startColor, const int16 endColor);

private:
	/**
	 * The first color in the system palette that the VMD
	 * can write to.
	 */
	uint8 _startColor;

	/**
	 * The last color in the system palette that the VMD
	 * can write to.
	 */
	uint8 _endColor;

	/**
	 * If true, video frames are rendered after a blank
	 * palette is submitted to the palette manager,
	 * which is then restored after the video pixels
	 * have already been rendered.
	 *
	 * This functionality is currently disabled because it seems like
	 * it was designed for a different graphics architecture where
	 * pixel data could be rendered before the video card's palette
	 * had been updated. This is not possible in ScummVM because the
	 * palette & pixel data are rendered simultaneously when
	 * OSystem::updateScreen is called, rather than immediately
	 * after they are sent to the backend.
	 */
#ifdef SCI_VMD_BLACK_PALETTE
	bool _blackPalette;
#endif

#pragma mark -
#pragma mark VMDPlayer - Brightness boost
private:
	/**
	 * The amount of brightness boost for the video.
	 * Values above 100 increase brightness; values below
	 * 100 reduce it.
	 */
	int16 _boostPercent;

	/**
	 * The first color in the palette that should be
	 * brightness boosted.
	 */
	uint8 _boostStartColor;

	/**
	 * The last color in the palette that should be
	 * brightness boosted.
	 */
	uint8 _boostEndColor;

#pragma mark -
#pragma mark VMDPlayer - Mouse cursor
public:
	/**
	 * Sets whether or not the mouse cursor should be drawn.
	 * This does not have any effect during playback, but can
	 * be used to prevent the mouse cursor from being shown
	 * again after the video has finished.
	 */
	void setShowCursor(const bool shouldShow) { _showCursor = shouldShow; }

private:
	/**
	 * Whether or not the mouse cursor should be shown
	 * during playback.
	 */
	bool _showCursor;
};

#pragma mark -
#pragma mark DuckPlayer

class DuckPlayer {
public:
	enum DuckStatus {
		kDuckClosed  = 0,
		kDuckOpen    = 1,
		kDuckPlaying = 2,
		kDuckPaused  = 3
	};

	DuckPlayer(SegManager *segMan, EventManager *eventMan);

	~DuckPlayer();

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
	 * Sets a flag indicating that an opaque plane should be added
	 * to the graphics manager underneath the video surface during
	 * playback.
	 */
	void setDoFrameOut(const bool value) { _doFrameOut = value; }

	/**
	 * Sets the volume of the decoder.
	 */
	void setVolume(const uint8 value) {
		_volume = (uint)value * Audio::Mixer::kMaxChannelVolume / Audio32::kMaxVolume;
		_decoder->setVolume(_volume);
	}

private:
	EventManager *_eventMan;
	Video::AVIDecoder *_decoder;

	/**
	 * An empty plane drawn behind the video when the doFrameOut
	 * flag is true.
	 */
	Plane *_plane;

	/**
	 * The player status.
	 */
	DuckStatus _status;

	/**
	 * The screen rect where the video should be drawn.
	 */
	Common::Rect _drawRect;

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
	 * If true, the video will be pixel doubled during playback.
	 */
	bool _pixelDouble;

	/**
	 * The buffer used to perform scaling of the video.
	 */
	byte *_scaleBuffer;

	/**
	 * Renders the current frame to the system video buffer.
	 */
	void renderFrame() const;
};

#pragma mark -
#pragma mark Video32

/**
 * Video32 provides facilities for playing back
 * video in SCI engine.
 */
class Video32 : public Common::Serializable {
public:
	Video32(SegManager *segMan, EventManager *eventMan) :
	_SEQPlayer(segMan),
	_AVIPlayer(segMan, eventMan),
	_VMDPlayer(segMan, eventMan),
	_robotPlayer(segMan),
	_duckPlayer(segMan, eventMan) {}

	void beforeSaveLoadWithSerializer(Common::Serializer &ser);
	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	SEQPlayer &getSEQPlayer() { return _SEQPlayer; }
	AVIPlayer &getAVIPlayer() { return _AVIPlayer; }
	VMDPlayer &getVMDPlayer() { return _VMDPlayer; }
	RobotDecoder &getRobotPlayer() { return _robotPlayer; }
	DuckPlayer &getDuckPlayer() { return _duckPlayer; }

private:
	SEQPlayer _SEQPlayer;
	AVIPlayer _AVIPlayer;
	VMDPlayer _VMDPlayer;
	RobotDecoder _robotPlayer;
	DuckPlayer _duckPlayer;
};
} // End of namespace Sci

#endif
