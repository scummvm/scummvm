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
#ifndef USE_RGB_COLOR
#include "common/translation.h"          // for _
#endif
#include "common/util.h"                 // for ARRAYSIZE
#include "common/system.h"               // for g_system
#include "engine.h"                      // for Engine, g_engine
#include "graphics/colormasks.h"         // for createPixelFormat
#include "graphics/palette.h"            // for PaletteManager
#include "graphics/transparent_surface.h" // for TransparentSurface
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

static void directWriteToSystem(Video::VideoDecoder *decoder, const Common::Rect &drawRect, const bool setSystemPalette, const Graphics::Surface *nextFrame = nullptr) {

	// VMDPlayer needs to decode the frame early so it can submit palette
	// updates; calling decodeNextFrame again loses frames
	if (!nextFrame) {
		nextFrame = decoder->decodeNextFrame();
	}
	assert(nextFrame);

	if (setSystemPalette &&
		g_system->getScreenFormat().bytesPerPixel == 1 &&
		decoder->hasDirtyPalette()) {

		const uint8 *palette = decoder->getPalette();
		assert(palette);
		g_system->getPaletteManager()->setPalette(palette, 0, 256);

		// KQ7 1.x has videos encoded using Microsoft Video 1 where palette 0 is
		// white and 255 is black, which is basically the opposite of DOS/Win
		// SCI palettes. So, when drawing to an 8bpp hwscreen, whenever a new
		// palette is seen, the screen must be re-filled with the new black
		// entry to ensure areas outside the video are always black and not some
		// other color
		for (int color = 0; color < 256; ++color) {
			if (palette[0] == 0 && palette[1] == 0 && palette[2] == 0) {
				g_system->fillScreen(color);
				break;
			}
			palette += 3;
		}
	}

	bool freeConvertedFrame;
	Graphics::Surface *convertedFrame;
	// Avoid creating a duplicate copy of the surface when it is not necessary
	if (decoder->getPixelFormat() == g_system->getScreenFormat()) {
		freeConvertedFrame = false;
		convertedFrame = const_cast<Graphics::Surface *>(nextFrame);
	} else {
		freeConvertedFrame = true;
		convertedFrame = nextFrame->convertTo(g_system->getScreenFormat(), decoder->getPalette());
	}
	assert(convertedFrame);

	if (decoder->getWidth() != drawRect.width() || decoder->getHeight() != drawRect.height()) {
		Graphics::Surface *const unscaledFrame(convertedFrame);
		const Graphics::TransparentSurface tsUnscaledFrame(*unscaledFrame);
#ifdef USE_RGB_COLOR
		if (g_system->getScreenFormat().bytesPerPixel != 1) {
			convertedFrame = tsUnscaledFrame.scaleT<Graphics::FILTER_BILINEAR>(drawRect.width(), drawRect.height());
		} else {
#else
		{
#endif
			convertedFrame = tsUnscaledFrame.scaleT<Graphics::FILTER_NEAREST>(drawRect.width(), drawRect.height());
		}
		assert(convertedFrame);
		if (freeConvertedFrame) {
			unscaledFrame->free();
			delete unscaledFrame;
		}
		freeConvertedFrame = true;
	}

	g_system->copyRectToScreen(convertedFrame->getPixels(), convertedFrame->pitch, drawRect.left, drawRect.top, convertedFrame->w, convertedFrame->h);
	g_sci->_gfxFrameout->updateScreen();
	if (freeConvertedFrame) {
		convertedFrame->free();
		delete convertedFrame;
	}
}

#pragma mark SEQPlayer

SEQPlayer::SEQPlayer(SegManager *segMan, EventManager *eventMan) :
	_segMan(segMan),
	_eventMan(eventMan),
	_decoder(nullptr) {}

void SEQPlayer::play(const Common::String &fileName, const int16 numTicks, const int16 x, const int16 y) {

	close();

	_decoder = new SEQDecoder(numTicks);
	if (!_decoder->loadFile(fileName)) {
		warning("[SEQPlayer::play]: Failed to load %s", fileName.c_str());
		delete _decoder;
		return;
	}

	const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
	const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
	const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;

	const int16 scaledWidth = (_decoder->getWidth() * Ratio(screenWidth, scriptWidth)).toInt();
	const int16 scaledHeight = (_decoder->getHeight() * Ratio(screenHeight, scriptHeight)).toInt();

	// Normally we would use the coordinates passed into the play function
	// to position the video, but since we are scaling the video (which SSCI
	// did not do), the coordinates are not correct. Since videos are always
	// intended to play in the center of the screen, we just recalculate the
	// origin here.
	_drawRect.left = (screenWidth - scaledWidth) / 2;
	_drawRect.top = (screenHeight - scaledHeight) / 2;
	_drawRect.setWidth(scaledWidth);
	_drawRect.setHeight(scaledHeight);

#ifdef USE_RGB_COLOR
	// Optimize rendering performance for unscaled videos, and allow
	// better-than-NN interpolation for videos that are scaled
	if (ConfMan.getBool("enable_hq_video") &&
		(_decoder->getWidth() != scaledWidth || _decoder->getHeight() != scaledHeight)) {
		// TODO: Search for and use the best supported format (which may be
		// lower than 32bpp) once the scaling code in Graphics supports
		// 16bpp/24bpp, and once the SDL backend can correctly communicate
		// supported pixel formats above whatever format is currently used by
		// _hwsurface. Right now, this will just crash ScummVM if the backend
		// does not support a 32bpp pixel format, which sucks since this code
		// really ought to be able to fall back to NN scaling for games with
		// 256-color videos.
		const Graphics::PixelFormat format = Graphics::createPixelFormat<8888>();
		g_sci->_gfxFrameout->setPixelFormat(format);
	}
#endif

	_decoder->start();

	while (!g_engine->shouldQuit() && !_decoder->endOfVideo()) {
		g_sci->sleep(_decoder->getTimeToNextFrame());
		while (_decoder->needsUpdate()) {
			renderFrame();
		}

		// SSCI did not allow SEQ animations to be bypassed like this
		SciEvent event = _eventMan->getSciEvent(SCI_EVENT_MOUSE_PRESS | SCI_EVENT_PEEK);
		if (event.type == SCI_EVENT_MOUSE_PRESS) {
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
				break;
			}
		}
	}

	close();
}

void SEQPlayer::renderFrame() const {
	directWriteToSystem(_decoder, _drawRect, true);
}

void SEQPlayer::close() {
#ifdef USE_RGB_COLOR
	if (g_system->getScreenFormat().bytesPerPixel != 1) {
		const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
		g_sci->_gfxFrameout->setPixelFormat(format);
	}
#endif

	g_system->fillScreen(0);
	delete _decoder;
	_decoder = nullptr;
}

#pragma mark -
#pragma mark AVIPlayer

AVIPlayer::AVIPlayer(EventManager *eventMan) :
	_eventMan(eventMan),
	_decoder(new Video::AVIDecoder(Audio::Mixer::kSFXSoundType)),
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

#ifndef USE_RGB_COLOR
	// KQ7 2.00b videos are compressed in 24bpp Cinepak, so cannot play on
	// a system with no RGB support
	if (_decoder->getPixelFormat().bytesPerPixel != 1) {
		void showScummVMDialog(const Common::String &message);
		showScummVMDialog(Common::String::format(_("Cannot play back %dbpp video on a system with maximum color depth of 8bpp"), _decoder->getPixelFormat().bpp()));
		_decoder->close();
		return kIOFileNotFound;
	}
#endif

	_status = kAVIOpen;
	return kIOSuccess;
}

AVIPlayer::IOStatus AVIPlayer::init(const bool pixelDouble) {
	// Calls to initialize the AVI player in SCI can be made in a few ways:
	//
	// * kShowMovie(WinInit, x, y) to render the video at (x,y) using its
	//   original resolution, or
	// * kShowMovie(WinInit, x, y, w, h) to render the video at (x,y) with
	//   rescaling to the given width and height, or
	// * kShowMovie(WinInitDouble, x, y) to render the video at (x,y) with
	//   rescaling to double the original resolution.
	//
	// Unfortunately, the values passed by game scripts are frequently wrong:
	//
	// * KQ7 passes origin coordinates that cause videos to be misaligned on the
	//   Y-axis;
	// * GK1 passes width and height that change the aspect ratio of the videos,
	//   even though they were rendered with square pixels (and in the case of
	//   CREDITS.AVI, cause the video to be badly downscaled);
	// * The GK2 demo does all of these things at the same time.
	//
	// Fortunately, whenever all of these games play an AVI, they are just
	// trying to play a video at the center of the screen. So, we ignore the
	// values that the game sends, and instead calculate the correct dimensions
	// and origin based on the video data, only allowing games to specify
	// whether or not the videos should be scaled up 2x.

	if (_status == kAVINotOpen) {
		return kIOFileNotFound;
	}

	g_sci->_gfxCursor32->hide();

	int16 width = _decoder->getWidth();
	int16 height = _decoder->getHeight();
	if (pixelDouble) {
		width *= 2;
		height *= 2;
	}

	const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
	const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;

	// When scaling videos, they must not grow larger than the hardware screen
	// or else the engine will crash. This is particularly important for the GK1
	// CREDITS.AVI since the game sends extra width/height arguments, causing it
	// to be treated as needing upscaling even though it does not.
	width = MIN<int16>(width, screenWidth);
	height = MIN<int16>(height, screenHeight);

	_drawRect.left = (screenWidth - width) / 2;
	_drawRect.top = (screenHeight - height) / 2;
	_drawRect.setWidth(width);
	_drawRect.setHeight(height);

#ifdef USE_RGB_COLOR
	// Optimize rendering performance for unscaled videos, and allow
	// better-than-NN interpolation for videos that are scaled
	if (ConfMan.getBool("enable_hq_video") &&
		(_decoder->getWidth() != width || _decoder->getHeight() != height)) {

		// TODO: Search for and use the best supported format (which may be
		// lower than 32bpp) once the scaling code in Graphics supports
		// 16bpp/24bpp, and once the SDL backend can correctly communicate
		// supported pixel formats above whatever format is currently used by
		// _hwsurface. Right now, this will just crash ScummVM if the backend
		// does not support a 32bpp pixel format, which sucks since this code
		// really ought to be able to fall back to NN scaling for games with
		// 256-color videos.
		const Graphics::PixelFormat format = Graphics::createPixelFormat<8888>();
		g_sci->_gfxFrameout->setPixelFormat(format);
	} else {
#else
	{
#endif
		const Graphics::PixelFormat format = _decoder->getPixelFormat();
		g_sci->_gfxFrameout->setPixelFormat(format);
	}

	return kIOSuccess;
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

#ifdef USE_RGB_COLOR
	if (g_system->getScreenFormat().bytesPerPixel != 1) {
		const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
		g_sci->_gfxFrameout->setPixelFormat(format);
	}
#endif

	g_system->fillScreen(0);
	g_sci->_gfxCursor32->unhide();

	_decoder->close();
	_status = kAVINotOpen;
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
	directWriteToSystem(_decoder, _drawRect, true);
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

void VMDPlayer::init(int16 x, const int16 y, const PlayFlags flags, const int16 boostPercent, const int16 boostStartColor, const int16 boostEndColor) {
	if (getSciVersion() < SCI_VERSION_3) {
		x &= ~1;
	}
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

	_drawRect = Common::Rect(x,
							 y,
							 x + (_decoder->getWidth() << _doublePixels),
							 y + (_decoder->getHeight() << (_doublePixels || _stretchVertical)));
}

VMDPlayer::IOStatus VMDPlayer::close() {
	if (!_isOpen) {
		return kIOSuccess;
	}

	if (_isComposited) {
		closeComposited();
	} else {
		closeOverlay();
	}

	if (_blackoutPlane != nullptr) {
		g_sci->_gfxFrameout->deletePlane(*_blackoutPlane);
		_blackoutPlane = nullptr;
	}

	if (!_leaveLastFrame && !_leaveScreenBlack) {
		// This call *actually* deletes the blackout plane
		g_sci->_gfxFrameout->frameOut(true);
	}

	_decoder->close();

	if (_bundledVmd) {
		g_sci->getResMan()->unlockResource(_bundledVmd);
		_bundledVmd = nullptr;
	}

	if (!_showCursor) {
		g_sci->_gfxCursor32->unhide();
	}

	_isOpen = false;
	_isInitialized = false;
	_ignorePalettes = false;
	_lastYieldedFrameNo = 0;
	_planeIsOwned = true;
	_priority = 0;
	_drawRect = Common::Rect();
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

		if (!_blackoutRect.isEmpty() && _planeIsOwned) {
			_blackoutPlane = new Plane(_blackoutRect);
			g_sci->_gfxFrameout->addPlane(*_blackoutPlane);
		}

		if (shouldUseCompositing()) {
			_isComposited = true;
			_usingHighColor = false;
			initComposited();
		} else {
			_isComposited = false;
			_usingHighColor = shouldUseHighColor();
			initOverlay();
		}

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

void VMDPlayer::initOverlay() {
	g_sci->_gfxFrameout->frameOut(true);

#ifdef USE_RGB_COLOR
	// TODO: Allow interpolation for videos where the cursor is drawn, either by
	// writing to an intermediate 4bpp surface and using that surface during
	// cursor drawing, or by promoting the cursor code to use CursorMan, if
	// possible
	if (_usingHighColor) {
		// TODO: 8888 is used here because 4bpp is the only format currently
		// supported by the common scaling code
		const Graphics::PixelFormat format = Graphics::createPixelFormat<8888>();
		g_sci->_gfxFrameout->setPixelFormat(format);
		redrawGameScreen();
	}
#endif
}

#ifdef USE_RGB_COLOR
void VMDPlayer::redrawGameScreen() const {
	Graphics::Surface *game = g_sci->_gfxFrameout->getCurrentBuffer().convertTo(g_system->getScreenFormat(), g_sci->_gfxPalette32->getHardwarePalette());

	Common::Rect rects[4];
	int splitCount = splitRects(Common::Rect(game->w, game->h), _drawRect, rects);
	if (splitCount != -1) {
		while (splitCount--) {
			const Common::Rect &drawRect = rects[splitCount];
			g_system->copyRectToScreen(game->getBasePtr(drawRect.left, drawRect.top), game->pitch, drawRect.left, drawRect.top, drawRect.width(), drawRect.height());
		}
	}

	game->free();
	delete game;
}
#endif

void VMDPlayer::renderOverlay() const {
	const Graphics::Surface *nextFrame = _decoder->decodeNextFrame();

#ifdef USE_RGB_COLOR
	if (_usingHighColor) {
		if (updatePalette()) {
			redrawGameScreen();
		}

		directWriteToSystem(_decoder, _drawRect, false, nextFrame);
	} else {
#else
	{
#endif
		updatePalette();

		Graphics::Surface out = g_sci->_gfxFrameout->getCurrentBuffer().getSubArea(_drawRect);

		const int lineCount = _blackLines ? 2 : 1;
		if (_doublePixels) {
			for (int16 y = 0; y < _drawRect.height(); y += lineCount) {
				const uint8 *source = (uint8 *)nextFrame->getBasePtr(0, y >> 1);
				uint8 *target = (uint8 *)out.getBasePtr(0, y);
				for (int16 x = 0; x < _decoder->getWidth(); ++x) {
					*target++ = *source;
					*target++ = *source++;
				}
			}
		} else if (_blackLines) {
			for (int16 y = 0; y < _drawRect.height(); y += lineCount) {
				const uint8 *source = (uint8 *)nextFrame->getBasePtr(0, y);
				uint8 *target = (uint8 *)out.getBasePtr(0, y);
				memcpy(target, source, _drawRect.width());
			}
		} else {
			out.copyRectToSurface(nextFrame->getPixels(), nextFrame->pitch, 0, 0, nextFrame->w, nextFrame->h);
		}

		g_sci->_gfxFrameout->directFrameOut(_drawRect);
	}
}

bool VMDPlayer::updatePalette() const {
	if (_ignorePalettes || !_decoder->hasDirtyPalette()) {
		return false;
	}

	Palette palette;
	for (uint16 i = 0; i < _startColor; ++i) {
		palette.colors[i].used = false;
	}
	for (uint16 i = _endColor + 1; i < ARRAYSIZE(palette.colors); ++i) {
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

	if (_isComposited) {
		SciBitmap *bitmap = _segMan->lookupBitmap(_bitmapId);
		bitmap->setPalette(palette);
		g_sci->_gfxFrameout->updateScreenItem(*_screenItem);
		g_sci->_gfxFrameout->frameOut(true);
	} else {
		g_sci->_gfxPalette32->submit(palette);
		g_sci->_gfxPalette32->updateForFrame();
		g_sci->_gfxPalette32->updateHardware();
	}

#if SCI_VMD_BLACK_PALETTE
	if (_blackPalette) {
		fillPalette(palette);
		if (_isComposited) {
			SciBitmap *bitmap = _segMan->lookupBitmap(_bitmapId);
			bitmap->setPalette(palette);
		}
		g_sci->_gfxPalette32->submit(palette);
		g_sci->_gfxPalette32->updateForFrame();
		g_sci->_gfxPalette32->updateHardware();
	}
#endif

	return true;
}

void VMDPlayer::closeOverlay() {
#ifdef USE_RGB_COLOR
	if (_usingHighColor) {
		g_sci->_gfxFrameout->setPixelFormat(Graphics::PixelFormat::createFormatCLUT8());
		g_sci->_gfxFrameout->resetHardware();
	} else {
#else
	{
#endif
		g_sci->_gfxFrameout->frameOut(true, _drawRect);
	}
}

void VMDPlayer::initComposited() {
	ScaleInfo vmdScaleInfo;

	if (_doublePixels) {
		vmdScaleInfo.x *= 2;
		vmdScaleInfo.y *= 2;
		vmdScaleInfo.signal = kScaleSignalManual;
	} else if (_stretchVertical) {
		vmdScaleInfo.y *= 2;
		vmdScaleInfo.signal = kScaleSignalManual;
	}

	const uint32 hunkPaletteSize = HunkPalette::calculateHunkPaletteSize(256, false);
	const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
	const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;

	SciBitmap &vmdBitmap = *_segMan->allocateBitmap(&_bitmapId, _drawRect.width(), _drawRect.height(), 255, 0, 0, screenWidth, screenHeight, hunkPaletteSize, false, false);
	vmdBitmap.getBuffer().fillRect(Common::Rect(_drawRect.width(), _drawRect.height()), 0);

	CelInfo32 vmdCelInfo;
	vmdCelInfo.bitmap = _bitmapId;
	_decoder->setSurfaceMemory(vmdBitmap.getPixels(), vmdBitmap.getWidth(), vmdBitmap.getHeight(), 1);

	if (_planeIsOwned) {
		_plane = new Plane(_drawRect, kPlanePicColored);
		if (_priority) {
			_plane->_priority = _priority;
		}
		g_sci->_gfxFrameout->addPlane(*_plane);
		_screenItem = new ScreenItem(_plane->_object, vmdCelInfo, Common::Point(), vmdScaleInfo);
	} else {
		_screenItem = new ScreenItem(_plane->_object, vmdCelInfo, Common::Point(_drawRect.left, _drawRect.top), vmdScaleInfo);
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
}

void VMDPlayer::renderComposited() const {
	// This writes directly to the CelObjMem we already created,
	// so no need to take its return value
	_decoder->decodeNextFrame();
	if (!updatePalette()) {
		g_sci->_gfxFrameout->updateScreenItem(*_screenItem);
		g_sci->_gfxFrameout->frameOut(true);
	}
}

void VMDPlayer::closeComposited() {
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
}

#pragma mark -
#pragma mark VMDPlayer - Rendering

void VMDPlayer::renderFrame() const {
	if (_isComposited) {
		renderComposited();
	} else {
		renderOverlay();
	}
}

void VMDPlayer::fillPalette(Palette &palette) const {
	const byte *vmdPalette = _decoder->getPalette() + _startColor * 3;
	for (uint16 i = _startColor; i <= _endColor; ++i) {
		uint8 r = *vmdPalette++;
		uint8 g = *vmdPalette++;
		uint8 b = *vmdPalette++;

		if (_boostPercent != 100 && i >= _boostStartColor && i <= _boostEndColor) {
			r = CLIP(r * _boostPercent / 100, 0, 255);
			g = CLIP(g * _boostPercent / 100, 0, 255);
			b = CLIP(b * _boostPercent / 100, 0, 255);
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

	g_sci->_gfxFrameout->updateScreen();
}

} // End of namespace Sci
