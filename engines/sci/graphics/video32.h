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

namespace Video { class AdvancedVMDDecoder; }
namespace Sci {
class Plane;
class ScreenItem;
class SegManager;

#pragma mark VMDPlayer

/**
 * VMDPlayer is used to play VMD videos.
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
	 */
	bool _blackPalette;

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

class Video32 {
public:
	Video32(SegManager *segMan, EventManager *eventMan) :
	_VMDPlayer(segMan, eventMan) {}

	VMDPlayer &getVMDPlayer() { return _VMDPlayer; }

private:
	VMDPlayer _VMDPlayer;
};
} // End of namespace Sci

#endif
