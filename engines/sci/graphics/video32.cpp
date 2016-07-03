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

#include "audio/mixer.h"
#include "sci/console.h"
#include "sci/event.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/video32.h"
#include "sci/sci.h"
#include "video/coktel_decoder.h"

namespace Sci {
VMDPlayer::VMDPlayer(SegManager *segMan, EventManager *eventMan) :
	_segMan(segMan),
	_eventMan(eventMan),
	_decoder(new Video::AdvancedVMDDecoder(Audio::Mixer::kSFXSoundType)),
	_isOpen(false),
	_isInitialized(false),
	_startColor(0),
	_planeSet(false),
	_endColor(255),
	_blackLines(false),
	_doublePixels(false),
	_lastYieldedFrameNo(0),
	_blackoutRect(),
	_blackPalette(false),
	_boostPercent(100),
	_boostStartColor(0),
	_boostEndColor(255),
	_leaveLastFrame(false),
	_leaveScreenBlack(false),
	_plane(nullptr),
	_screenItem(nullptr),
	_stretchVertical(false),
	_priority(0),
	_blackoutPlane(nullptr),
	_yieldInterval(0) {}

VMDPlayer::~VMDPlayer() {
	close();
	delete _decoder;
}

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
	_leaveScreenBlack = flags & kPlayFlagLeaveScreenBlack;
	_leaveLastFrame = flags & kPlayFlagLeaveLastFrame;
	_doublePixels = flags & kPlayFlagDoublePixels;
	_blackLines = flags & kPlayFlagBlackLines;
	_boostPercent = 100 + (flags & kPlayFlagBoost ? boostPercent : 0);
	_blackPalette = flags & kPlayFlagBlackPalette;
	_stretchVertical = flags & kPlayFlagStretchVertical;
	_boostStartColor = CLIP<int16>(boostStartColor, 0, 255);
	_boostEndColor = CLIP<int16>(boostEndColor, 0, 255);
}

void VMDPlayer::restrictPalette(const uint8 startColor, const uint8 endColor) {
	_startColor = startColor;
	_endColor = endColor;
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

		const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
		const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
		const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

		Common::Rect vmdRect(
			_x,
			_y,
			_x + _decoder->getWidth(),
			_y + _decoder->getHeight()
		);
		ScaleInfo vmdScaleInfo;

		if (!_blackoutRect.isEmpty() && !_planeSet) {
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

		BitmapResource vmdBitmap(_segMan, vmdRect.width(), vmdRect.height(), 255, 0, 0, screenWidth, screenHeight, 0, false);

		if (screenWidth != scriptWidth || screenHeight != scriptHeight) {
			mulru(vmdRect, Ratio(scriptWidth, screenWidth), Ratio(scriptHeight, screenHeight), 1);
		}

		CelInfo32 vmdCelInfo;
		vmdCelInfo.bitmap = vmdBitmap.getObject();
		_decoder->setSurfaceMemory(vmdBitmap.getPixels(), vmdBitmap.getWidth(), vmdBitmap.getHeight(), 1);

		if (!_planeSet) {
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

		if (
			_yieldInterval > 0 &&
			currentFrameNo != _lastYieldedFrameNo &&
			(currentFrameNo % _yieldInterval) == 0
		) {
			_lastYieldedFrameNo = currentFrameNo;
			stopFlag = kEventFlagYieldToVM;
			break;
		}

		if (flags & kEventFlagMouseDown && _eventMan->getSciEvent(SCI_EVENT_MOUSE_PRESS | SCI_EVENT_PEEK).type != SCI_EVENT_NONE) {
			stopFlag = kEventFlagMouseDown;
			break;
		}

		if (flags & kEventFlagEscapeKey) {
			const SciEvent event = _eventMan->getSciEvent(SCI_EVENT_KEYBOARD | SCI_EVENT_PEEK);
			if (event.type != SCI_EVENT_NONE && event.character == SCI_KEY_ESC) {
				stopFlag = kEventFlagEscapeKey;
				break;
			}
		}

		if (flags & kEventFlagHotRectangle) {
			// TODO: Hot rectangles
			warning("Hot rectangles not implemented in VMD player");
			stopFlag = kEventFlagHotRectangle;
			break;
		}
	}

	_decoder->pauseVideo(true);
	return stopFlag;
}

void VMDPlayer::renderFrame() {
	// TODO: This is kind of different from the original implementation
	// which has access to dirty rects from the decoder; we probably do
	// not need to care to limit output this way

	_decoder->decodeNextFrame();

	// NOTE: Normally this would write a hunk palette at the end of the
	// video bitmap that CelObjMem would read out and submit, but instead
	// we are just submitting it directly here because the decoder exposes
	// this information a little bit differently than the one in SSCI
	const bool dirtyPalette = _decoder->hasDirtyPalette();
	if (dirtyPalette) {
		Palette palette;
		if (_blackPalette) {
			for (uint16 i = _startColor; i <= _endColor; ++i) {
				palette.colors[i].r = palette.colors[i].g = palette.colors[i].b = 0;
				palette.colors[i].used = true;
			}
		} else {
			const byte *vmdPalette = _decoder->getPalette() + _startColor * 3;
			for (uint16 i = _startColor; i <= _endColor; ++i) {
				palette.colors[i].r = *vmdPalette++;
				palette.colors[i].g = *vmdPalette++;
				palette.colors[i].b = *vmdPalette++;
				palette.colors[i].used = true;
			}
		}

		g_sci->_gfxPalette32->submit(palette);
		g_sci->_gfxFrameout->updateScreenItem(*_screenItem);
		g_sci->_gfxFrameout->frameOut(true);

		if (_blackPalette) {
			const byte *vmdPalette = _decoder->getPalette() + _startColor * 3;
			for (uint16 i = _startColor; i <= _endColor; ++i) {
				palette.colors[i].r = *vmdPalette++;
				palette.colors[i].g = *vmdPalette++;
				palette.colors[i].b = *vmdPalette++;
				palette.colors[i].used = true;
			}

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

VMDPlayer::IOStatus VMDPlayer::close() {
	if (!_isOpen) {
		return kIOSuccess;
	}

	_decoder->close();
	_isOpen = false;
	_isInitialized = false;

	if (_planeSet && _screenItem != nullptr) {
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
		g_sci->_gfxCursor->kernelShow();
	}

	_planeSet = false;
	_priority = 0;
	return kIOSuccess;
}

} // End of namespace Sci
