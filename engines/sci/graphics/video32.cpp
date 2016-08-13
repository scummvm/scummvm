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

#include "common/config-manager.h"       // for ConfMan
#include "common/textconsole.h"          // for warning, error
#include "common/util.h"                 // for ARRAYSIZE
#include "common/system.h"               // for g_system
#include "engine.h"                      // for Engine, g_engine
#include "engines/util.h"                // for initGraphics
#include "sci/console.h"                 // for Console
#include "sci/engine/state.h"            // for EngineState
#include "sci/engine/vm_types.h"         // for reg_t
#include "sci/event.h"                   // for SciEvent, EventManager, SCI_...
#include "sci/graphics/celobj32.h"       // for CelInfo32, ::kLowResX, ::kLo...
#include "sci/graphics/cursor.h"         // for GfxCursor
#include "sci/graphics/frameout.h"       // for GfxFrameout
#include "sci/graphics/helpers.h"        // for Color, Palette
#include "sci/graphics/palette32.h"      // for GfxPalette32
#include "sci/graphics/plane32.h"        // for Plane, PlanePictureCodes::kP...
#include "sci/graphics/screen_item32.h"  // for ScaleInfo, ScreenItem, Scale...
#include "sci/sci.h"                     // for SciEngine, g_sci, getSciVersion
#include "sci/graphics/video32.h"
#include "sci/video/seq_decoder.h"       // for SEQDecoder
#include "video/avi_decoder.h"           // for AVIDecoder
#include "video/coktel_decoder.h"        // for AdvancedVMDDecoder
namespace Graphics { struct Surface; }

namespace Sci {

#pragma mark SEQPlayer

SEQPlayer::SEQPlayer(SegManager *segMan) :
	_segMan(segMan),
	_decoder(nullptr),
	_plane(nullptr),
	_screenItem(nullptr) {}

void SEQPlayer::play(const Common::String &fileName, const int16 numTicks, const int16 x, const int16 y) {
	delete _decoder;
	_decoder = new SEQDecoder(numTicks);
	_decoder->loadFile(fileName);

	// NOTE: In the original engine, video was output directly to the hardware,
	// bypassing the game's rendering engine. Instead of doing this, we use a
	// mechanism that is very similar to that used by the VMD player, which
	// allows the SEQ to be drawn into a bitmap ScreenItem and displayed using
	// the normal graphics system.
	_segMan->allocateBitmap(&_bitmap, _decoder->getWidth(), _decoder->getHeight(), kDefaultSkipColor, 0, 0, kLowResX, kLowResY, 0, false, false);

	CelInfo32 celInfo;
	celInfo.type = kCelTypeMem;
	celInfo.bitmap = _bitmap;

	_plane = new Plane(Common::Rect(kLowResX, kLowResY), kPlanePicColored);
	g_sci->_gfxFrameout->addPlane(*_plane);

	// Normally we would use the x, y coordinates passed into the play function
	// to position the screen item, but because the video frame bitmap is
	// drawn in low-resolution coordinates, it gets automatically scaled up by
	// the engine (pixel doubling with aspect ratio correction). As a result,
	// the animation does not need the extra offsets from the game in order to
	// be correctly positioned in the middle of the window, so we ignore them.
	_screenItem = new ScreenItem(_plane->_object, celInfo, Common::Point(0, 0), ScaleInfo());
	g_sci->_gfxFrameout->addScreenItem(*_screenItem);
	g_sci->_gfxFrameout->frameOut(true);
	_decoder->start();

	while (!g_engine->shouldQuit() && !_decoder->endOfVideo()) {
		renderFrame();
		g_sci->getEngineState()->speedThrottler(_decoder->getTimeToNextFrame());
		g_sci->getEngineState()->_throttleTrigger = true;
	}

	_segMan->freeBitmap(_screenItem->_celInfo.bitmap);
	g_sci->_gfxFrameout->deletePlane(*_plane);
	g_sci->_gfxFrameout->frameOut(true);
	_screenItem = nullptr;
	_plane = nullptr;
}

void SEQPlayer::renderFrame() const {
	const Graphics::Surface *surface = _decoder->decodeNextFrame();

	SciBitmap &bitmap = *_segMan->lookupBitmap(_bitmap);
	bitmap.getBuffer().copyRectToSurface(*surface, 0, 0, Common::Rect(surface->w, surface->h));

	const bool dirtyPalette = _decoder->hasDirtyPalette();
	if (dirtyPalette) {
		Palette palette;
		const byte *rawPalette = _decoder->getPalette();
		for (int i = 0; i < ARRAYSIZE(palette.colors); ++i) {
			palette.colors[i].r = *rawPalette++;
			palette.colors[i].g = *rawPalette++;
			palette.colors[i].b = *rawPalette++;
			palette.colors[i].used = true;
		}

		g_sci->_gfxPalette32->submit(palette);
	}

	g_sci->_gfxFrameout->updateScreenItem(*_screenItem);
	g_sci->getSciDebugger()->onFrame();
	g_sci->_gfxFrameout->frameOut(true);
}

#pragma mark -
#pragma mark AVIPlayer

AVIPlayer::AVIPlayer(SegManager *segMan, EventManager *eventMan) :
	_segMan(segMan),
	_eventMan(eventMan),
	_decoder(new Video::AVIDecoder(Audio::Mixer::kSFXSoundType)),
	_scaleBuffer(nullptr),
	_plane(nullptr),
	_screenItem(nullptr),
	_status(kAVINotOpen) {}

AVIPlayer::~AVIPlayer() {
	close();
	delete _decoder;
}

AVIPlayer::IOStatus AVIPlayer::open(const Common::String &fileName) {
	if (_status != kAVINotOpen) {
		close();
	}

	if (!_decoder->loadFile(fileName)) {
		return kIOFileNotFound;
	}

	_status = kAVIOpen;
	return kIOSuccess;
}

AVIPlayer::IOStatus AVIPlayer::init1x(const int16 x, const int16 y, int16 width, int16 height) {
	if (_status == kAVINotOpen) {
		return kIOFileNotFound;
	}

	_pixelDouble = false;

	if (!width || !height) {
		width = _decoder->getWidth();
		height = _decoder->getHeight();
	} else if (getSciVersion() == SCI_VERSION_2_1_EARLY && g_sci->getGameId() == GID_KQ7) {
		// KQ7 1.51 provides an explicit width and height when it wants scaling,
		// though the width and height it provides are not scaled
		_pixelDouble = true;
		width *= 2;
		height *= 2;
	}

	// QFG4CD gives non-multiple-of-2 values for width and height,
	// which would normally be OK except the source video is a pixel bigger
	// in each dimension
	width = (width + 1) & ~1;
	height = (height + 1) & ~1;

	_drawRect.left = x;
	_drawRect.top = y;
	_drawRect.right = x + width;
	_drawRect.bottom = y + height;

	// SCI2.1mid uses init2x to draw a pixel-doubled AVI, but SCI2 has only the
	// one play routine which automatically pixel-doubles in hi-res mode
	if (getSciVersion() == SCI_VERSION_2) {
		// This is somewhat of a hack; credits.avi from GK1 is not
		// rendered correctly in SSCI because it is a 640x480 video, but the
		// game script gives the wrong dimensions. Since this is the only
		// high-resolution AVI ever used, just set the draw rectangle to draw
		// the entire screen
		if (_decoder->getWidth() > 320) {
			_drawRect.left = 0;
			_drawRect.top = 0;
			_drawRect.right = 320;
			_drawRect.bottom = 200;
		}

		// In hi-res mode, video will be pixel doubled, so the origin (which
		// corresponds to the correct position without pixel doubling) needs to
		// be corrected
		if (g_sci->_gfxFrameout->_isHiRes && _decoder->getWidth() <= 320) {
			_drawRect.left /= 2;
			_drawRect.top /= 2;
		}
	}

	init();

	return kIOSuccess;
}

AVIPlayer::IOStatus AVIPlayer::init2x(const int16 x, const int16 y) {
	if (_status == kAVINotOpen) {
		return kIOFileNotFound;
	}

	_drawRect.left = x;
	_drawRect.top = y;
	_drawRect.right = x + _decoder->getWidth() * 2;
	_drawRect.bottom = y + _decoder->getHeight() * 2;

	_pixelDouble = true;
	init();

	return kIOSuccess;
}

void AVIPlayer::init() {
	int16 xRes;
	int16 yRes;

	bool useScreenDimensions = false;
	if (g_sci->_gfxFrameout->_isHiRes && _decoder->getWidth() > 320) {
		useScreenDimensions = true;
	}

	// KQ7 1.51 gives video position in screen coordinates, not game
	// coordinates, because in SSCI they are passed to Video for Windows, which
	// renders as an overlay on the game video. Because we put the video into a
	// ScreenItem instead of rendering directly to the hardware surface, the
	// coordinates need to be converted to game script coordinates
	if (g_sci->getGameId() == GID_KQ7 && getSciVersion() == SCI_VERSION_2_1_EARLY) {
		useScreenDimensions = !_pixelDouble;
		// This y-translation is arbitrary, based on what roughly centers the
		// videos in the game window
		_drawRect.translate(-_drawRect.left / 2, -_drawRect.top * 2 / 3);
	}

	if (useScreenDimensions) {
		xRes = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
		yRes = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
	} else {
		xRes = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		yRes = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
	}

	_plane = new Plane(_drawRect);
	g_sci->_gfxFrameout->addPlane(*_plane);

	if (_decoder->getPixelFormat().bytesPerPixel == 1) {
		_segMan->allocateBitmap(&_bitmap, _decoder->getWidth(), _decoder->getHeight(), kDefaultSkipColor, 0, 0, xRes, yRes, 0, false, false);

		CelInfo32 celInfo;
		celInfo.type = kCelTypeMem;
		celInfo.bitmap = _bitmap;

		_screenItem = new ScreenItem(_plane->_object, celInfo, Common::Point(_drawRect.left, _drawRect.top), ScaleInfo());
		g_sci->_gfxFrameout->addScreenItem(*_screenItem);
		g_sci->_gfxFrameout->frameOut(true);
	} else {
		const Buffer &currentBuffer = g_sci->_gfxFrameout->getCurrentBuffer();
		const Graphics::PixelFormat format = _decoder->getPixelFormat();
		initGraphics(currentBuffer.screenWidth, currentBuffer.screenHeight, g_sci->_gfxFrameout->_isHiRes, &format);

		if (_pixelDouble) {
			const int16 width = _drawRect.width();
			const int16 height = _drawRect.height();
			_scaleBuffer = calloc(1, width * height * format.bytesPerPixel);
		}
	}
}

AVIPlayer::IOStatus AVIPlayer::play(const int16 from, const int16 to, const int16, const bool async) {
	if (_status == kAVINotOpen) {
		return kIOFileNotFound;
	}

	if (from >= 0 && to > 0 && from <= to) {
		_decoder->seekToFrame(from);
		_decoder->setEndFrame(to);
	}

	if (!async) {
		renderVideo();
	} else if (getSciVersion() == SCI_VERSION_2_1_EARLY) {
		playUntilEvent((EventFlags)(kEventFlagEnd | kEventFlagEscapeKey));
	} else {
		_status = kAVIPlaying;
	}

	return kIOSuccess;
}

void AVIPlayer::renderVideo() const {
	_decoder->start();
	while (!g_engine->shouldQuit() && !_decoder->endOfVideo()) {
		g_sci->getEngineState()->speedThrottler(_decoder->getTimeToNextFrame());
		g_sci->getEngineState()->_throttleTrigger = true;
		if (_decoder->needsUpdate()) {
			renderFrame();
		}
	}
}

AVIPlayer::IOStatus AVIPlayer::close() {
	if (_status == kAVINotOpen) {
		return kIOSuccess;
	}

	free(_scaleBuffer);
	_scaleBuffer = nullptr;

	if (_decoder->getPixelFormat().bytesPerPixel != 1) {
		const bool isHiRes = g_sci->_gfxFrameout->_isHiRes;
		const Buffer &currentBuffer = g_sci->_gfxFrameout->getCurrentBuffer();
		const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
		initGraphics(currentBuffer.screenWidth, currentBuffer.screenHeight, isHiRes, &format);
	}

	_decoder->close();
	_status = kAVINotOpen;
	g_sci->_gfxFrameout->deletePlane(*_plane);
	_plane = nullptr;
	_screenItem = nullptr;
	return kIOSuccess;
}

AVIPlayer::IOStatus AVIPlayer::cue(const uint16 frameNo) {
	if (!_decoder->seekToFrame(frameNo)) {
		return kIOSeekFailed;
	}

	_status = kAVIPaused;
	return kIOSuccess;
}

uint16 AVIPlayer::getDuration() const {
	if (_status == kAVINotOpen) {
		return 0;
	}

	return _decoder->getFrameCount();
}

void AVIPlayer::renderFrame() const {
	const Graphics::Surface *surface = _decoder->decodeNextFrame();

	if (surface->format.bytesPerPixel == 1) {
		SciBitmap &bitmap = *_segMan->lookupBitmap(_bitmap);
		if (surface->w > bitmap.getWidth() || surface->h > bitmap.getHeight()) {
			warning("Attempted to draw a video frame larger than the destination bitmap");
			return;
		}

		// KQ7 1.51 encodes videos with palette entry 0 as white, which makes
		// the area around the video turn white too, since it is coded to use
		// palette entry 0. This happens to work in the original game because
		// the video is rendered by VfW, not in the engine itself. To fix this,
		// we just modify the incoming pixel data from the video so if a pixel
		// is using entry 0, we change it to use entry 255, which is guaranteed
		// to always be white
		if (getSciVersion() == SCI_VERSION_2_1_EARLY && g_sci->getGameId() == GID_KQ7) {
			uint8 *target = bitmap.getPixels();
			const uint8 *source = (const uint8 *)surface->getPixels();
			const uint8 *end = (const uint8 *)surface->getPixels() + surface->w * surface->h;

			while (source != end) {
				uint8 value = *source++;
				*target++ = value == 0 ? 255 : value;
			}
		} else {
			bitmap.getBuffer().copyRectToSurface(*surface, 0, 0, Common::Rect(surface->w, surface->h));
		}

		const bool dirtyPalette = _decoder->hasDirtyPalette();
		if (dirtyPalette) {
			Palette palette;
			const byte *rawPalette = _decoder->getPalette();
			for (int i = 0; i < ARRAYSIZE(palette.colors); ++i) {
				palette.colors[i].r = *rawPalette++;
				palette.colors[i].g = *rawPalette++;
				palette.colors[i].b = *rawPalette++;
				palette.colors[i].used = true;
			}

			// Prevent KQ7 1.51 from setting entry 0 to white
			palette.colors[0].used = false;

			g_sci->_gfxPalette32->submit(palette);
		}

		g_sci->_gfxFrameout->updateScreenItem(*_screenItem);
		g_sci->getSciDebugger()->onFrame();
		g_sci->_gfxFrameout->frameOut(true);
	} else {
		assert(surface->format.bytesPerPixel == 4);

		Common::Rect drawRect(_drawRect);

		if (_pixelDouble) {
			const uint32 *source = (const uint32 *)surface->getPixels();
			uint32 *target = (uint32 *)_scaleBuffer;
			// target pitch here is in uint32s, not bytes
			const uint16 pitch = surface->pitch / 2;
			for (int y = 0; y < surface->h; ++y) {
				for (int x = 0; x < surface->w; ++x) {
					const uint32 value = *source++;

					target[0] = value;
					target[1] = value;
					target[pitch] = value;
					target[pitch + 1] = value;
					target += 2;
				}
				target += pitch;
			}

			g_system->copyRectToScreen(_scaleBuffer, surface->pitch * 2, _drawRect.left, _drawRect.top, _drawRect.width(), _drawRect.height());
		} else {
			const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
			const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
			const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
			const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

			mulinc(drawRect, Ratio(screenWidth, scriptWidth), Ratio(screenHeight, scriptHeight));

			g_system->copyRectToScreen(surface->getPixels(), surface->pitch, drawRect.left, drawRect.top, surface->w, surface->h);
		}
	}
}

AVIPlayer::EventFlags AVIPlayer::playUntilEvent(EventFlags flags) {
	_decoder->start();

	EventFlags stopFlag = kEventFlagNone;
	while (!g_engine->shouldQuit()) {
		if (_decoder->endOfVideo()) {
			stopFlag = kEventFlagEnd;
			break;
		}

		g_sci->getEngineState()->speedThrottler(_decoder->getTimeToNextFrame());
		g_sci->getEngineState()->_throttleTrigger = true;
		if (_decoder->needsUpdate()) {
			renderFrame();
		}

		SciEvent event = _eventMan->getSciEvent(SCI_EVENT_MOUSE_PRESS | SCI_EVENT_PEEK);
		if ((flags & kEventFlagMouseDown) && event.type == SCI_EVENT_MOUSE_PRESS) {
			stopFlag = kEventFlagMouseDown;
			break;
		}

		event = _eventMan->getSciEvent(SCI_EVENT_KEYBOARD | SCI_EVENT_PEEK);
		if ((flags & kEventFlagEscapeKey) && event.type == SCI_EVENT_KEYBOARD) {
			bool stop = false;
			while ((event = _eventMan->getSciEvent(SCI_EVENT_KEYBOARD)),
				   event.type != SCI_EVENT_NONE) {
				if (event.character == SCI_KEY_ESC) {
					stop = true;
					break;
				}
			}

			if (stop) {
				stopFlag = kEventFlagEscapeKey;
				break;
			}
		}

		// TODO: Hot rectangles
		if ((flags & kEventFlagHotRectangle) /* && event.type == SCI_EVENT_HOT_RECTANGLE */) {
			warning("Hot rectangles not implemented in VMD player");
			stopFlag = kEventFlagHotRectangle;
			break;
		}
	}

	return stopFlag;
}

#pragma mark -
#pragma mark VMDPlayer

VMDPlayer::VMDPlayer(SegManager *segMan, EventManager *eventMan) :
	_segMan(segMan),
	_eventMan(eventMan),
	_decoder(new Video::AdvancedVMDDecoder(Audio::Mixer::kSFXSoundType)),

	_isOpen(false),
	_isInitialized(false),
	_yieldInterval(0),
	_lastYieldedFrameNo(0),

	_plane(nullptr),
	_screenItem(nullptr),
	_planeIsOwned(true),
	_priority(0),
	_doublePixels(false),
	_stretchVertical(false),
	_blackLines(false),
	_leaveScreenBlack(false),
	_leaveLastFrame(false),

	_blackoutPlane(nullptr),

	_startColor(0),
	_endColor(255),
	_blackPalette(false),

	_boostPercent(100),
	_boostStartColor(0),
	_boostEndColor(255),

	_showCursor(false) {}

VMDPlayer::~VMDPlayer() {
	close();
	delete _decoder;
}

#pragma mark -
#pragma mark VMDPlayer - Playback

VMDPlayer::IOStatus VMDPlayer::open(const Common::String &fileName, const OpenFlags flags) {
	if (_isOpen) {
		error("Attempted to play %s, but another VMD was loaded", fileName.c_str());
	}

	if (_decoder->loadFile(fileName)) {
		if (flags & kOpenFlagMute) {
			_decoder->setVolume(0);
		}
		_isOpen = true;
		return kIOSuccess;
	} else {
		return kIOError;
	}
}

void VMDPlayer::init(const int16 x, const int16 y, const PlayFlags flags, const int16 boostPercent, const int16 boostStartColor, const int16 boostEndColor) {
	_x = getSciVersion() >= SCI_VERSION_3 ? x : (x & ~1);
	_y = y;
	_doublePixels = flags & kPlayFlagDoublePixels;
	_blackLines = ConfMan.getBool("enable_black_lined_video") && (flags & kPlayFlagBlackLines);
	_boostPercent = 100 + (flags & kPlayFlagBoost ? boostPercent : 0);
	_boostStartColor = CLIP<int16>(boostStartColor, 0, 255);
	_boostEndColor = CLIP<int16>(boostEndColor, 0, 255);
	_leaveScreenBlack = flags & kPlayFlagLeaveScreenBlack;
	_leaveLastFrame = flags & kPlayFlagLeaveLastFrame;
	_blackPalette = flags & kPlayFlagBlackPalette;
	_stretchVertical = flags & kPlayFlagStretchVertical;
}

VMDPlayer::IOStatus VMDPlayer::close() {
	if (!_isOpen) {
		return kIOSuccess;
	}

	_decoder->close();
	_isOpen = false;
	_isInitialized = false;

	if (!_planeIsOwned && _screenItem != nullptr) {
		g_sci->_gfxFrameout->deleteScreenItem(*_screenItem);
		_segMan->freeBitmap(_screenItem->_celInfo.bitmap);
		_screenItem = nullptr;
	} else if (_plane != nullptr) {
		g_sci->_gfxFrameout->deletePlane(*_plane);
		_plane = nullptr;
	}

	if (!_leaveLastFrame && _leaveScreenBlack) {
		// This call *actually* deletes the plane/screen item
		g_sci->_gfxFrameout->frameOut(true);
	}

	if (_blackoutPlane != nullptr) {
		g_sci->_gfxFrameout->deletePlane(*_blackoutPlane);
		_blackoutPlane = nullptr;
	}

	if (!_leaveLastFrame && !_leaveScreenBlack) {
		// This call *actually* deletes the blackout plane
		g_sci->_gfxFrameout->frameOut(true);
	}

	if (!_showCursor) {
		g_sci->_gfxCursor->kernelShow();
	}

	_lastYieldedFrameNo = 0;
	_planeIsOwned = true;
	_priority = 0;
	return kIOSuccess;
}

VMDPlayer::EventFlags VMDPlayer::kernelPlayUntilEvent(const EventFlags flags, const int16 lastFrameNo, const int16 yieldInterval) {
	assert(lastFrameNo >= -1);

	const int32 maxFrameNo = (int32)(_decoder->getFrameCount() - 1);

	if ((flags & kEventFlagToFrame) && lastFrameNo > 0) {
		_decoder->setEndFrame(MIN((int32)lastFrameNo, maxFrameNo));
	} else {
		_decoder->setEndFrame(maxFrameNo);
	}

	if (flags & kEventFlagYieldToVM) {
		_yieldInterval = 3;
		if (yieldInterval == -1 && !(flags & kEventFlagToFrame)) {
			_yieldInterval = lastFrameNo;
		} else if (yieldInterval != -1) {
			_yieldInterval = MIN((int32)yieldInterval, maxFrameNo);
		}
	} else {
		_yieldInterval = maxFrameNo;
	}

	return playUntilEvent(flags);
}

VMDPlayer::EventFlags VMDPlayer::playUntilEvent(const EventFlags flags) {
	// Flushing all the keyboard and mouse events out of the event manager to
	// avoid letting any events queued from before the video started from
	// accidentally activating an event callback
	for (;;) {
		const SciEvent event = _eventMan->getSciEvent(SCI_EVENT_KEYBOARD | SCI_EVENT_MOUSE_PRESS | SCI_EVENT_MOUSE_RELEASE | SCI_EVENT_QUIT);
		if (event.type == SCI_EVENT_NONE) {
			break;
		} else if (event.type == SCI_EVENT_QUIT) {
			return kEventFlagEnd;
		}
	}

	_decoder->pauseVideo(false);

	if (flags & kEventFlagReverse) {
		// NOTE: This flag may not work properly since SSCI does not care
		// if a video has audio, but the VMD decoder does.
		const bool success = _decoder->setReverse(true);
		assert(success);
		_decoder->setVolume(0);
	}

	if (!_isInitialized) {
		_isInitialized = true;

		if (!_showCursor) {
			g_sci->_gfxCursor->kernelHide();
		}

		Common::Rect vmdRect(_x,
							 _y,
							 _x + _decoder->getWidth(),
							 _y + _decoder->getHeight());
		ScaleInfo vmdScaleInfo;

		if (!_blackoutRect.isEmpty() && _planeIsOwned) {
			_blackoutPlane = new Plane(_blackoutRect);
			g_sci->_gfxFrameout->addPlane(*_blackoutPlane);
		}

		if (_doublePixels) {
			vmdScaleInfo.x = 256;
			vmdScaleInfo.y = 256;
			vmdScaleInfo.signal = kScaleSignalDoScaling32;
			vmdRect.right += vmdRect.width();
			vmdRect.bottom += vmdRect.height();
		} else if (_stretchVertical) {
			vmdScaleInfo.y = 256;
			vmdScaleInfo.signal = kScaleSignalDoScaling32;
			vmdRect.bottom += vmdRect.height();
		}

		const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
		const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
		const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

		reg_t bitmapId;
		SciBitmap &vmdBitmap = *_segMan->allocateBitmap(&bitmapId, vmdRect.width(), vmdRect.height(), 255, 0, 0, screenWidth, screenHeight, 0, false, false);

		if (screenWidth != scriptWidth || screenHeight != scriptHeight) {
			mulru(vmdRect, Ratio(scriptWidth, screenWidth), Ratio(scriptHeight, screenHeight), 1);
		}

		CelInfo32 vmdCelInfo;
		vmdCelInfo.bitmap = bitmapId;
		_decoder->setSurfaceMemory(vmdBitmap.getPixels(), vmdBitmap.getWidth(), vmdBitmap.getHeight(), 1);

		if (_planeIsOwned) {
			_x = 0;
			_y = 0;
			_plane = new Plane(vmdRect, kPlanePicColored);
			if (_priority) {
				_plane->_priority = _priority;
			}
			g_sci->_gfxFrameout->addPlane(*_plane);
			_screenItem = new ScreenItem(_plane->_object, vmdCelInfo, Common::Point(), vmdScaleInfo);
		} else {
			_screenItem = new ScreenItem(_plane->_object, vmdCelInfo, Common::Point(_x, _y), vmdScaleInfo);
			if (_priority) {
				_screenItem->_priority = _priority;
			}
		}

		if (_blackLines) {
			_screenItem->_drawBlackLines = true;
		}

		// NOTE: There was code for positioning the screen item using insetRect
		// here, but none of the game scripts seem to use this functionality.

		g_sci->_gfxFrameout->addScreenItem(*_screenItem);

		_decoder->start();
	}

	EventFlags stopFlag = kEventFlagNone;
	while (!g_engine->shouldQuit()) {
		if (_decoder->endOfVideo()) {
			stopFlag = kEventFlagEnd;
			break;
		}

		g_sci->getEngineState()->speedThrottler(_decoder->getTimeToNextFrame());
		g_sci->getEngineState()->_throttleTrigger = true;
		if (_decoder->needsUpdate()) {
			renderFrame();
		}

		const int currentFrameNo = _decoder->getCurFrame();

		if (_yieldInterval > 0 &&
			currentFrameNo != _lastYieldedFrameNo &&
			(currentFrameNo % _yieldInterval) == 0
		) {
			_lastYieldedFrameNo = currentFrameNo;
			stopFlag = kEventFlagYieldToVM;
			break;
		}

		SciEvent event = _eventMan->getSciEvent(SCI_EVENT_MOUSE_PRESS | SCI_EVENT_PEEK);
		if ((flags & kEventFlagMouseDown) && event.type == SCI_EVENT_MOUSE_PRESS) {
			stopFlag = kEventFlagMouseDown;
			break;
		}

		event = _eventMan->getSciEvent(SCI_EVENT_KEYBOARD | SCI_EVENT_PEEK);
		if ((flags & kEventFlagEscapeKey) && event.type == SCI_EVENT_KEYBOARD) {
			bool stop = false;
			if (getSciVersion() < SCI_VERSION_3) {
				while ((event = _eventMan->getSciEvent(SCI_EVENT_KEYBOARD)),
					   event.type != SCI_EVENT_NONE) {
					if (event.character == SCI_KEY_ESC) {
						stop = true;
						break;
					}
				}
			} else {
				stop = (event.character == SCI_KEY_ESC);
			}

			if (stop) {
				stopFlag = kEventFlagEscapeKey;
				break;
			}
		}

		// TODO: Hot rectangles
		if ((flags & kEventFlagHotRectangle) /* && event.type == SCI_EVENT_HOT_RECTANGLE */) {
			warning("Hot rectangles not implemented in VMD player");
			stopFlag = kEventFlagHotRectangle;
			break;
		}
	}

	_decoder->pauseVideo(true);
	return stopFlag;
}

#pragma mark -
#pragma mark VMDPlayer - Rendering

void VMDPlayer::renderFrame() const {
	// This writes directly to the CelObjMem we already created,
	// so no need to take its return value
	_decoder->decodeNextFrame();

	// NOTE: Normally this would write a hunk palette at the end of the
	// video bitmap that CelObjMem would read out and submit, but instead
	// we are just submitting it directly here because the decoder exposes
	// this information a little bit differently than the one in SSCI
	const bool dirtyPalette = _decoder->hasDirtyPalette();
	if (dirtyPalette) {
		Palette palette;
		palette.timestamp = g_sci->getTickCount();
		for (uint16 i = 0; i < _startColor; ++i) {
			palette.colors[i].used = false;
		}
		for (uint16 i = _endColor; i < 256; ++i) {
			palette.colors[i].used = false;
		}
		if (_blackPalette) {
			for (uint16 i = _startColor; i <= _endColor; ++i) {
				palette.colors[i].r = palette.colors[i].g = palette.colors[i].b = 0;
				palette.colors[i].used = true;
			}
		} else {
			fillPalette(palette);
		}

		g_sci->_gfxPalette32->submit(palette);
		g_sci->_gfxFrameout->updateScreenItem(*_screenItem);
		g_sci->_gfxFrameout->frameOut(true);

		if (_blackPalette) {
			fillPalette(palette);
			g_sci->_gfxPalette32->submit(palette);
			g_sci->_gfxPalette32->updateForFrame();
			g_sci->_gfxPalette32->updateHardware();
		}
	} else {
		g_sci->_gfxFrameout->updateScreenItem(*_screenItem);
		g_sci->getSciDebugger()->onFrame();
		g_sci->_gfxFrameout->frameOut(true);
		g_sci->_gfxFrameout->throttle();
	}
}

void VMDPlayer::fillPalette(Palette &palette) const {
	const byte *vmdPalette = _decoder->getPalette() + _startColor * 3;
	for (uint16 i = _startColor; i <= _endColor; ++i) {
		int16 r = *vmdPalette++;
		int16 g = *vmdPalette++;
		int16 b = *vmdPalette++;

		if (_boostPercent != 100 && i >= _boostStartColor && i <= _boostEndColor) {
			r = CLIP<int16>(r * _boostPercent / 100, 0, 255);
			g = CLIP<int16>(g * _boostPercent / 100, 0, 255);
			b = CLIP<int16>(b * _boostPercent / 100, 0, 255);
		}

		palette.colors[i].r = r;
		palette.colors[i].g = g;
		palette.colors[i].b = b;
		palette.colors[i].used = true;
	}
}

#pragma mark -
#pragma mark VMDPlayer - Palette

void VMDPlayer::restrictPalette(const uint8 startColor, const int16 endColor) {
	_startColor = startColor;
	// At least GK2 sends 256 as the end color, which is wrong,
	// but works in the original engine as the storage size is 4 bytes
	// and used values are clamped to 0-255
	_endColor = MIN((int16)255, endColor);
}

} // End of namespace Sci
