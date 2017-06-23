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

#include "audio/mixer.h"                 // for Audio::Mixer::kSFXSoundType
#include "common/config-manager.h"       // for ConfMan
#include "common/textconsole.h"          // for warning, error
#include "common/util.h"                 // for ARRAYSIZE
#include "common/system.h"               // for g_system
#include "engine.h"                      // for Engine, g_engine
#include "sci/console.h"                 // for Console
#include "sci/engine/features.h"         // for GameFeatures
#include "sci/engine/state.h"            // for EngineState
#include "sci/engine/vm_types.h"         // for reg_t
#include "sci/event.h"                   // for SciEvent, EventManager, SCI_...
#include "sci/graphics/celobj32.h"       // for CelInfo32, ::kLowResX, ::kLo...
#include "sci/graphics/cursor32.h"       // for GfxCursor32
#include "sci/graphics/frameout.h"       // for GfxFrameout
#include "sci/graphics/helpers.h"        // for Color, Palette
#include "sci/graphics/palette32.h"      // for GfxPalette32
#include "sci/graphics/plane32.h"        // for Plane, PlanePictureCodes::kP...
#include "sci/graphics/screen_item32.h"  // for ScaleInfo, ScreenItem, Scale...
#include "sci/resource.h"                // for ResourceManager, ResourceId,...
#include "sci/sci.h"                     // for SciEngine, g_sci, getSciVersion
#include "sci/sound/audio32.h"           // for Audio32
#include "sci/video/seq_decoder.h"       // for SEQDecoder
#include "video/avi_decoder.h"           // for AVIDecoder
#include "video/coktel_decoder.h"        // for AdvancedVMDDecoder
#include "sci/graphics/video32.h"

namespace Graphics { struct Surface; }

namespace Sci {

/**
 * @returns true if the player should quit
 */
static bool flushEvents(EventManager *eventMan) {
	// Flushing all the keyboard and mouse events out of the event manager
	// keeps events queued from before the start of playback from accidentally
	// activating a video stop flag
	for (;;) {
		const SciEvent event = eventMan->getSciEvent(SCI_EVENT_KEYBOARD | SCI_EVENT_MOUSE_PRESS | SCI_EVENT_MOUSE_RELEASE | SCI_EVENT_HOT_RECTANGLE | SCI_EVENT_QUIT);
		if (event.type == SCI_EVENT_NONE) {
			break;
		} else if (event.type == SCI_EVENT_QUIT) {
			return true;
		}
	}

	return false;
}

#pragma mark SEQPlayer

SEQPlayer::SEQPlayer(SegManager *segMan) :
	_segMan(segMan),
	_decoder(nullptr),
	_plane(nullptr),
	_screenItem(nullptr) {}

void SEQPlayer::play(const Common::String &fileName, const int16 numTicks, const int16 x, const int16 y) {
	delete _decoder;
	_decoder = new SEQDecoder(numTicks);
	if (!_decoder->loadFile(fileName)) {
		warning("[SEQPlayer::play]: Failed to load %s", fileName.c_str());
		return;
	}

	// NOTE: In the original engine, video was output directly to the hardware,
	// bypassing the game's rendering engine. Instead of doing this, we use a
	// mechanism that is very similar to that used by the VMD player, which
	// allows the SEQ to be drawn into a bitmap ScreenItem and displayed using
	// the normal graphics system.
	reg_t bitmapId;
	SciBitmap &bitmap = *_segMan->allocateBitmap(&bitmapId, _decoder->getWidth(), _decoder->getHeight(), kDefaultSkipColor, 0, 0, kLowResX, kLowResY, 0, false, false);
	bitmap.getBuffer().fillRect(Common::Rect(_decoder->getWidth(), _decoder->getHeight()), 0);

	CelInfo32 celInfo;
	celInfo.type = kCelTypeMem;
	celInfo.bitmap = bitmapId;

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
		g_sci->sleep(_decoder->getTimeToNextFrame());
		renderFrame(bitmap);
	}

	_segMan->freeBitmap(bitmapId);
	g_sci->_gfxFrameout->deletePlane(*_plane);
	g_sci->_gfxFrameout->frameOut(true);
	_screenItem = nullptr;
	_plane = nullptr;
}

void SEQPlayer::renderFrame(SciBitmap &bitmap) const {
	const Graphics::Surface *surface = _decoder->decodeNextFrame();

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
	g_sci->_gfxFrameout->frameOut(true);
	g_sci->getSciDebugger()->onFrame();
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
	_bitmap(NULL_REG),
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
		const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
		const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
		const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
		const Ratio screenToScriptX(scriptWidth, screenWidth);
		const Ratio screenToScriptY(scriptHeight, screenHeight);
		width = (_decoder->getWidth() * screenToScriptX).toInt();
		height = (_decoder->getHeight() * screenToScriptY).toInt();
	}

	// QFG4CD gives non-multiple-of-2 values for width and height of the intro
	// video, which would normally be OK except the source video is a pixel
	// bigger in each dimension so it just causes part of the video to get cut
	// off
	width = (width + 1) & ~1;
	height = (height + 1) & ~1;

	// GK1 CREDITS.AVI is not rendered correctly in SSCI because it is a 640x480
	// video and the game script gives the wrong dimensions.
	// Since this is the only high-resolution AVI ever used by any SCI game,
	// just set the draw rectangle to draw across the entire screen
	if (g_sci->getGameId() == GID_GK1 && _decoder->getWidth() > 320) {
		_drawRect.left = 0;
		_drawRect.top = 0;
		_drawRect.right = 320;
		_drawRect.bottom = 200;
	} else {
		_drawRect.left = x;
		_drawRect.top = y;
		_drawRect.right = x + width;
		_drawRect.bottom = y + height;
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

	// GK1 CREDITS.AVI or KQ7 1.51 half-size videos
	if ((g_sci->_gfxFrameout->_isHiRes && _decoder->getWidth() > 320) ||
		(g_sci->getGameId() == GID_KQ7 && getSciVersion() == SCI_VERSION_2_1_EARLY && _drawRect.width() <= 160)) {
		xRes = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
		yRes = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
	} else {
		xRes = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;

		const Ratio videoRatio(_decoder->getWidth(), _decoder->getHeight());
		const Ratio screenRatio(4, 3);

		// Videos that already have a 4:3 aspect ratio should not receive any
		// aspect ratio correction
		if (videoRatio == screenRatio) {
			yRes = 240;
		} else {
			yRes = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
		}
	}

	_plane = new Plane(_drawRect);
	g_sci->_gfxFrameout->addPlane(*_plane);

	if (_decoder->getPixelFormat().bytesPerPixel == 1) {
		SciBitmap &bitmap = *_segMan->allocateBitmap(&_bitmap, _decoder->getWidth(), _decoder->getHeight(), kDefaultSkipColor, 0, 0, xRes, yRes, 0, false, false);
		bitmap.getBuffer().fillRect(Common::Rect(_decoder->getWidth(), _decoder->getHeight()), 0);

		CelInfo32 celInfo;
		celInfo.type = kCelTypeMem;
		celInfo.bitmap = _bitmap;

		_screenItem = new ScreenItem(_plane->_object, celInfo, Common::Point(), ScaleInfo());
		g_sci->_gfxFrameout->addScreenItem(*_screenItem);
		g_sci->_gfxFrameout->frameOut(true);
	} else {
		// Attempting to draw a palettized cursor into a 24bpp surface will
		// cause memory corruption, so hide the cursor in this mode (SCI did not
		// have a 24bpp mode but just directed VFW to display videos instead)
		g_sci->_gfxCursor32->hide();

		const Graphics::PixelFormat format = _decoder->getPixelFormat();
		g_sci->_gfxFrameout->setPixelFormat(format);

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
		g_sci->sleep(_decoder->getTimeToNextFrame());
		while (_decoder->needsUpdate()) {
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
		const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
		g_sci->_gfxFrameout->setPixelFormat(format);
		g_sci->_gfxCursor32->unhide();
	}

	_decoder->close();
	_status = kAVINotOpen;
	if (_bitmap != NULL_REG) {
		_segMan->freeBitmap(_bitmap);
		_bitmap = NULL_REG;
	}
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
				const uint8 value = *source++;
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
		g_sci->_gfxFrameout->frameOut(true);
		g_sci->getSciDebugger()->onFrame();
	} else {
		assert(surface->format.bytesPerPixel == 4);

		const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
		const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
		const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

		Common::Rect drawRect(_drawRect);
		mulru(drawRect, Ratio(screenWidth, scriptWidth), Ratio(screenHeight, scriptHeight), 1);

		if (_pixelDouble) {
			const uint32 *source = (const uint32 *)surface->getPixels();
			uint32 *target = (uint32 *)_scaleBuffer;
			// target pitch here is in uint32s, not bytes, because the surface
			// bpp is 4
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

			g_system->copyRectToScreen(_scaleBuffer, surface->pitch * 2, drawRect.left, drawRect.top, _drawRect.width(), _drawRect.height());
		} else {
			g_system->copyRectToScreen(surface->getPixels(), surface->pitch, drawRect.left, drawRect.top, surface->w, surface->h);
		}

		g_system->updateScreen();
		g_sci->getSciDebugger()->onFrame();
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

		g_sci->sleep(_decoder->getTimeToNextFrame());
		while (_decoder->needsUpdate()) {
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
	_bundledVmd(nullptr),
	_yieldFrame(0),
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
	_ignorePalettes(false),

	_blackoutPlane(nullptr),

	_startColor(0),
	_endColor(255),
#ifdef SCI_VMD_BLACK_PALETTE
	_blackPalette(false),
#endif
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

	if (g_sci->_features->VMDOpenStopsAudio()) {
		g_sci->_audio32->stop(kAllChannels);
	}

	Resource *bundledVmd = g_sci->getResMan()->findResource(ResourceId(kResourceTypeVMD, fileName.asUint64()), true);

	if (bundledVmd != nullptr) {
		Common::SeekableReadStream *stream = bundledVmd->makeStream();
		if (_decoder->loadStream(stream)) {
			_bundledVmd = bundledVmd;
			_isOpen = true;
		} else {
			delete stream;
			g_sci->getResMan()->unlockResource(bundledVmd);
		}
	} else if (_decoder->loadFile(fileName)) {
		_isOpen = true;
	}

	if (_isOpen) {
		if (flags & kOpenFlagMute) {
			_decoder->setVolume(0);
		}
		return kIOSuccess;
	}

	return kIOError;
}

void VMDPlayer::init(const int16 x, const int16 y, const PlayFlags flags, const int16 boostPercent, const int16 boostStartColor, const int16 boostEndColor) {
	_x = getSciVersion() >= SCI_VERSION_3 ? x : (x & ~1);
	_y = y;
	_doublePixels = flags & kPlayFlagDoublePixels;
	_blackLines = ConfMan.getBool("enable_black_lined_video") && (flags & kPlayFlagBlackLines);
	// If ScummVM has been configured to disable black lines on video playback,
	// the boosts need to be ignored too or else the brightness of the video
	// will be too high
	_boostPercent = 100 + (_blackLines && (flags & kPlayFlagBoost) ? boostPercent : 0);
	_boostStartColor = CLIP<int16>(boostStartColor, 0, 255);
	_boostEndColor = CLIP<int16>(boostEndColor, 0, 255);
	_leaveScreenBlack = flags & kPlayFlagLeaveScreenBlack;
	_leaveLastFrame = flags & kPlayFlagLeaveLastFrame;
#ifdef SCI_VMD_BLACK_PALETTE
	_blackPalette = flags & kPlayFlagBlackPalette;
#endif
	_stretchVertical = flags & kPlayFlagStretchVertical;
}

VMDPlayer::IOStatus VMDPlayer::close() {
	if (!_isOpen) {
		return kIOSuccess;
	}

	_decoder->close();
	_isOpen = false;
	_isInitialized = false;
	_ignorePalettes = false;

	if (_bundledVmd) {
		g_sci->getResMan()->unlockResource(_bundledVmd);
		_bundledVmd = nullptr;
	}

	if (_bitmapId != NULL_REG) {
		_segMan->freeBitmap(_bitmapId);
		_bitmapId = NULL_REG;
	}

	if (!_planeIsOwned && _screenItem != nullptr) {
		g_sci->_gfxFrameout->deleteScreenItem(*_screenItem);
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
		g_sci->_gfxCursor32->unhide();
	}

	_lastYieldedFrameNo = 0;
	_planeIsOwned = true;
	_priority = 0;
	return kIOSuccess;
}

VMDPlayer::VMDStatus VMDPlayer::getStatus() const {
	if (!_isOpen) {
		return kVMDNotOpen;
	}
	if (_decoder->isPaused()) {
		return kVMDPaused;
	}
	if (_decoder->isPlaying()) {
		return kVMDPlaying;
	}
	if (_decoder->endOfVideo()) {
		return kVMDFinished;
	}
	return kVMDOpen;
}

VMDPlayer::EventFlags VMDPlayer::kernelPlayUntilEvent(const EventFlags flags, const int16 lastFrameNo, const int16 yieldInterval) {
	assert(lastFrameNo >= -1);

	const int32 maxFrameNo = _decoder->getFrameCount() - 1;

	if (flags & kEventFlagToFrame) {
		_yieldFrame = MIN<int32>(lastFrameNo, maxFrameNo);
	} else {
		_yieldFrame = maxFrameNo;
	}

	if (flags & kEventFlagYieldToVM) {
		_yieldInterval = 3;
		if (yieldInterval == -1 && !(flags & kEventFlagToFrame)) {
			_yieldInterval = lastFrameNo;
		} else if (yieldInterval != -1) {
			_yieldInterval = MIN<int32>(yieldInterval, maxFrameNo);
		}
	} else {
		_yieldInterval = maxFrameNo;
	}

	return playUntilEvent(flags);
}

VMDPlayer::EventFlags VMDPlayer::playUntilEvent(const EventFlags flags) {
	if (flushEvents(_eventMan)) {
		return kEventFlagEnd;
	}

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
			g_sci->_gfxCursor32->hide();
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
			vmdScaleInfo.signal = kScaleSignalManual;
			vmdRect.right += vmdRect.width();
			vmdRect.bottom += vmdRect.height();
		} else if (_stretchVertical) {
			vmdScaleInfo.y = 256;
			vmdScaleInfo.signal = kScaleSignalManual;
			vmdRect.bottom += vmdRect.height();
		}

		const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
		const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
		const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

		SciBitmap &vmdBitmap = *_segMan->allocateBitmap(&_bitmapId, vmdRect.width(), vmdRect.height(), 255, 0, 0, screenWidth, screenHeight, 0, false, false);
		vmdBitmap.getBuffer().fillRect(Common::Rect(vmdRect.width(), vmdRect.height()), 0);

		if (screenWidth != scriptWidth || screenHeight != scriptHeight) {
			mulru(vmdRect, Ratio(scriptWidth, screenWidth), Ratio(scriptHeight, screenHeight), 1);
		}

		CelInfo32 vmdCelInfo;
		vmdCelInfo.bitmap = _bitmapId;
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

		// Sleeping any more than 1/60th of a second will make the mouse feel
		// very sluggish during VMD action sequences because the frame rate of
		// VMDs is usually only 15fps
		g_sci->sleep(MIN<uint32>(10, _decoder->getTimeToNextFrame()));
		while (_decoder->needsUpdate()) {
			renderFrame();
		}

		const int currentFrameNo = _decoder->getCurFrame();

		if (currentFrameNo == _yieldFrame) {
			stopFlag = kEventFlagEnd;
			break;
		}

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

		event = _eventMan->getSciEvent(SCI_EVENT_HOT_RECTANGLE | SCI_EVENT_PEEK);
		if ((flags & kEventFlagHotRectangle) && event.type == SCI_EVENT_HOT_RECTANGLE) {
			stopFlag = kEventFlagHotRectangle;
			break;
		}
	}

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
	if (dirtyPalette && !_ignorePalettes) {
		Palette palette;
		palette.timestamp = g_sci->getTickCount();
		for (uint16 i = 0; i < _startColor; ++i) {
			palette.colors[i].used = false;
		}
		for (uint16 i = _endColor; i < 256; ++i) {
			palette.colors[i].used = false;
		}
#if SCI_VMD_BLACK_PALETTE
		if (_blackPalette) {
			for (uint16 i = _startColor; i <= _endColor; ++i) {
				palette.colors[i].r = palette.colors[i].g = palette.colors[i].b = 0;
				palette.colors[i].used = true;
			}
		} else
#endif
			fillPalette(palette);

		g_sci->_gfxPalette32->submit(palette);
		g_sci->_gfxFrameout->updateScreenItem(*_screenItem);
		g_sci->_gfxFrameout->frameOut(true);
		g_sci->getSciDebugger()->onFrame();

#if SCI_VMD_BLACK_PALETTE
		if (_blackPalette) {
			fillPalette(palette);
			g_sci->_gfxPalette32->submit(palette);
			g_sci->_gfxPalette32->updateForFrame();
			g_sci->_gfxPalette32->updateHardware();
		}
#endif
	} else {
		g_sci->_gfxFrameout->updateScreenItem(*_screenItem);
		g_sci->_gfxFrameout->frameOut(true);
		g_sci->getSciDebugger()->onFrame();
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

void VMDPlayer::setPlane(const int16 priority, const reg_t planeId) {
	_priority = priority;
	if (planeId != NULL_REG) {
		_plane = g_sci->_gfxFrameout->getPlanes().findByObject(planeId);
		assert(_plane != nullptr);
		_planeIsOwned = false;
	}
}

#pragma mark -
#pragma mark VMDPlayer - Palette

void VMDPlayer::restrictPalette(const uint8 startColor, const int16 endColor) {
	_startColor = startColor;
	// At least GK2 sends 256 as the end color, which is wrong,
	// but works in the original engine as the storage size is 4 bytes
	// and used values are clamped to 0-255
	_endColor = MIN<int16>(255, endColor);
}

#pragma mark -
#pragma mark DuckPlayer

DuckPlayer::DuckPlayer(SegManager *segMan, EventManager *eventMan) :
	_eventMan(eventMan),
	_decoder(new Video::AVIDecoder(Audio::Mixer::kSFXSoundType)),
	_plane(nullptr),
	_status(kDuckClosed),
	_drawRect(),
	_volume(Audio::Mixer::kMaxChannelVolume),
	_doFrameOut(false),
	_pixelDouble(false),
	_scaleBuffer(nullptr) {}

DuckPlayer::~DuckPlayer() {
	close();
	delete _decoder;
}

void DuckPlayer::open(const GuiResourceId resourceId, const int displayMode, const int16 x, const int16 y) {
	if (_status != kDuckClosed) {
		error("Attempted to play %u.duk, but another video was loaded", resourceId);
	}

	const Common::String fileName = Common::String::format("%u.duk", resourceId);
	if (!_decoder->loadFile(fileName)) {
		error("Can't open %s", fileName.c_str());
	}

	_decoder->setVolume(_volume);
	_pixelDouble = displayMode != 0;

	const int16 scale = _pixelDouble ? 2 : 1;
	// SSCI seems to incorrectly calculate the draw rect by scaling the origin
	// in addition to the width/height for the BR point
	_drawRect = Common::Rect(x, y,
							 x + _decoder->getWidth() * scale,
							 y + _decoder->getHeight() * scale);

	g_sci->_gfxCursor32->hide();

	if (_doFrameOut) {
		_plane = new Plane(_drawRect, kPlanePicColored);
		g_sci->_gfxFrameout->addPlane(*_plane);
		g_sci->_gfxFrameout->frameOut(true);
	}

	const Graphics::PixelFormat format = _decoder->getPixelFormat();

	if (_pixelDouble) {
		assert(_scaleBuffer == nullptr);
		_scaleBuffer = new byte[_drawRect.width() * _drawRect.height() * format.bytesPerPixel];
	}

	g_sci->_gfxFrameout->setPixelFormat(format);

	_status = kDuckOpen;
}

void DuckPlayer::play(const int lastFrameNo) {
	flushEvents(_eventMan);

	if (_status != kDuckPlaying) {
		_status = kDuckPlaying;
		_decoder->start();
	}

	while (!g_engine->shouldQuit()) {
		if (_decoder->endOfVideo() || (lastFrameNo != -1 && _decoder->getCurFrame() >= lastFrameNo)) {
			break;
		}

		g_sci->sleep(_decoder->getTimeToNextFrame());
		while (_decoder->needsUpdate()) {
			renderFrame();
		}

		SciEvent event = _eventMan->getSciEvent(SCI_EVENT_MOUSE_PRESS | SCI_EVENT_PEEK);
		if (event.type == SCI_EVENT_MOUSE_PRESS) {
			flushEvents(_eventMan);
			break;
		}

		event = _eventMan->getSciEvent(SCI_EVENT_KEYBOARD | SCI_EVENT_PEEK);
		if (event.type == SCI_EVENT_KEYBOARD) {
			bool stop = false;
			while ((event = _eventMan->getSciEvent(SCI_EVENT_KEYBOARD)),
				   event.type != SCI_EVENT_NONE) {
				if (event.character == SCI_KEY_ESC) {
					stop = true;
					break;
				}
			}

			if (stop) {
				flushEvents(_eventMan);
				break;
			}
		}
	}
}

void DuckPlayer::close() {
	if (_status == kDuckClosed) {
		return;
	}

	_decoder->close();

	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	g_sci->_gfxFrameout->setPixelFormat(format);

	g_sci->_gfxCursor32->unhide();

	if (_doFrameOut) {
		g_sci->_gfxFrameout->deletePlane(*_plane);
		g_sci->_gfxFrameout->frameOut(true);
		_plane = nullptr;
	}

	_pixelDouble = false;
	delete[] _scaleBuffer;
	_scaleBuffer = nullptr;

	_status = kDuckClosed;
}

static inline uint16 interpolate(const Graphics::PixelFormat &format, const uint16 p1, const uint16 p2) {
	uint8 r1, g1, b1, r2, g2, b2;
	format.colorToRGB(p1, r1, g1, b1);
	format.colorToRGB(p2, r2, g2, b2);
	return format.RGBToColor((r1 + r2) >> 1, (g1 + g2) >> 1, (b1 + b2) >> 1);
}

void DuckPlayer::renderFrame() const {
	const Graphics::Surface *surface = _decoder->decodeNextFrame();

	// Audio-only or non-updated frame
	if (surface == nullptr) {
		return;
	}

	assert(surface->format.bytesPerPixel == 2);

	if (_pixelDouble) {
		const uint16 *source = (const uint16 *)surface->getPixels();
		const Graphics::PixelFormat &format = surface->format;
		uint16 *target = (uint16 *)_scaleBuffer;

#ifndef SCI_DUCK_NO_INTERPOLATION
		// divide by 2 gets pixel pitch instead of byte pitch for source
		const uint16 sourcePitch = surface->pitch >> 1;
#endif

		const uint16 targetPitch = surface->pitch;
		const bool blackLined = ConfMan.getBool("enable_black_lined_video");
		for (int y = 0; y < surface->h - 1; ++y) {
			for (int x = 0; x < surface->w - 1; ++x) {
#ifndef SCI_DUCK_NO_INTERPOLATION
				const uint16 a = source[0];
				const uint16 b = source[1];
				const uint16 c = source[sourcePitch];
				const uint16 d = source[sourcePitch + 1];

				target[0] = a;
				target[1] = interpolate(format, a, b);
#else
				const uint16 value = *source;
				target[0] = value;
				target[1] = value;
#endif
				if (!blackLined) {
#ifndef SCI_DUCK_NO_INTERPOLATION
					target[targetPitch] = interpolate(format, a, c);
					target[targetPitch + 1] = interpolate(format, target[1], interpolate(format, c, d));
#else
					target[targetPitch] = value;
					target[targetPitch + 1] = value;
#endif
				}

				target += 2;
				++source;
			}

			const uint16 value = *source++;
			target[0] = value;
			target[1] = value;
			if (!blackLined) {
				target[targetPitch] = value;
				target[targetPitch + 1] = value;
			}
			target += 2;

			if (blackLined) {
				memset(target, 0, targetPitch * format.bytesPerPixel);
			}

			target += targetPitch;
		}

		for (int x = 0; x < surface->w; ++x) {
			const uint16 lastValue = *source++;
			target[0] = lastValue;
			target[1] = lastValue;

			if (!blackLined) {
				target[targetPitch] = lastValue;
				target[targetPitch + 1] = lastValue;
				target += 2;
			}
		}

		if (blackLined) {
			memset(target, 0, targetPitch);
		}

		g_system->copyRectToScreen(_scaleBuffer, surface->pitch * 2, _drawRect.left, _drawRect.top, _drawRect.width(), _drawRect.height());
	} else {
		g_system->copyRectToScreen(surface->getPixels(), surface->pitch, _drawRect.left, _drawRect.top, surface->w, surface->h);
	}

	g_system->updateScreen();
	g_sci->getSciDebugger()->onFrame();
}

} // End of namespace Sci
