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
 * $URL$
 * $Id$
 *
 */

#include <unistd.h>
#include <pthread.h>

#include <sys/time.h>

#include "common/scummsys.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/file.h"
#include "common/fs.h"

#include "base/main.h"

#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "sound/mixer.h"
#include "sound/mixer_intern.h"
#include "gui/message.h"

#include "osys_iphone.h"


const OSystem::GraphicsMode OSystem_IPHONE::s_supportedGraphicsModes[] = {
	{0, 0, 0}
};

AQCallbackStruct OSystem_IPHONE::s_AudioQueue;
SoundProc OSystem_IPHONE::s_soundCallback = NULL;
void *OSystem_IPHONE::s_soundParam = NULL;

OSystem_IPHONE::OSystem_IPHONE() :
	_savefile(NULL), _mixer(NULL), _timer(NULL), _offscreen(NULL),
	_overlayVisible(false), _overlayBuffer(NULL), _fullscreen(NULL),
	_mouseHeight(0), _mouseWidth(0), _mouseBuf(NULL), _lastMouseTap(0),
	_secondaryTapped(false), _lastSecondaryTap(0), _screenOrientation(kScreenOrientationFlippedLandscape),
	_needEventRestPeriod(false), _mouseClickAndDragEnabled(false), _touchpadModeEnabled(true),
	_gestureStartX(-1), _gestureStartY(-1), _fullScreenIsDirty(false), _fullScreenOverlayIsDirty(false),
	_mouseDirty(false), _timeSuspended(0), _lastDragPosX(-1), _lastDragPosY(-1), _screenChangeCount(0)

{
	_queuedInputEvent.type = (Common::EventType)0;
	_lastDrawnMouseRect = Common::Rect(0, 0, 0, 0);

	_fsFactory = new POSIXFilesystemFactory();
}

OSystem_IPHONE::~OSystem_IPHONE() {
	AudioQueueDispose(s_AudioQueue.queue, true);

	delete _fsFactory;
	delete _savefile;
	delete _mixer;
	delete _timer;
	delete _offscreen;
	delete _fullscreen;
}

int OSystem_IPHONE::timerHandler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

void OSystem_IPHONE::initBackend() {
#ifdef IPHONE_OFFICIAL
	_savefile = new DefaultSaveFileManager(iPhone_getDocumentsDir());
#else
	_savefile = new DefaultSaveFileManager(SCUMMVM_SAVE_PATH);
#endif

	_timer = new DefaultTimerManager();

	gettimeofday(&_startTime, NULL);

	setupMixer();

	setTimerCallback(&OSystem_IPHONE::timerHandler, 10);

	OSystem::initBackend();
}

bool OSystem_IPHONE::hasFeature(Feature f) {
	return false;
}

void OSystem_IPHONE::setFeatureState(Feature f, bool enable) {
}

bool OSystem_IPHONE::getFeatureState(Feature f) {
	return false;
}

const OSystem::GraphicsMode* OSystem_IPHONE::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_IPHONE::getDefaultGraphicsMode() const {
	return -1;
}

bool OSystem_IPHONE::setGraphicsMode(const char *mode) {
	return true;
}

bool OSystem_IPHONE::setGraphicsMode(int mode) {
	return true;
}

int OSystem_IPHONE::getGraphicsMode() const {
	return -1;
}

void OSystem_IPHONE::initSize(uint width, uint height) {
	//printf("initSize(%i, %i)\n", width, height);

	_screenWidth = width;
	_screenHeight = height;

	free(_offscreen);

	_offscreen = (byte *)malloc(width * height);
	bzero(_offscreen, width * height);

	free(_overlayBuffer);

	int fullSize = _screenWidth * _screenHeight * sizeof(OverlayColor);
	_overlayBuffer = (OverlayColor *)malloc(fullSize);
	clearOverlay();

	free(_fullscreen);

	_fullscreen = (uint16 *)malloc(fullSize);
	bzero(_fullscreen, fullSize);

	iPhone_initSurface(width, height);

	_fullScreenIsDirty = false;
	dirtyFullScreen();
	_mouseVisible = false;
	_screenChangeCount++;
	updateScreen();
}

int16 OSystem_IPHONE::getHeight() {
	return _screenHeight;
}

int16 OSystem_IPHONE::getWidth() {
	return _screenWidth;
}

void OSystem_IPHONE::setPalette(const byte *colors, uint start, uint num) {
	//printf("setPalette()\n");
	const byte *b = colors;

	for (uint i = start; i < start + num; ++i) {
		_palette[i] = Graphics::RGBToColor<Graphics::ColorMasks<565> >(b[0], b[1], b[2]);
		b += 4;
	}

	dirtyFullScreen();
}

void OSystem_IPHONE::grabPalette(byte *colors, uint start, uint num) {
	//printf("grabPalette()\n");
}

void OSystem_IPHONE::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	//printf("copyRectToScreen(%i, %i, %i, %i)\n", x, y, w, h);
	//Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight - y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	if (!_fullScreenIsDirty) {
		_dirtyRects.push_back(Common::Rect(x, y, x + w, y + h));
	}


	byte *dst = _offscreen + y * _screenWidth + x;
	if (_screenWidth == pitch && pitch == w)
		memcpy(dst, buf, h * w);
	else {
		do {
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _screenWidth;
		} while (--h);
	}
}

void OSystem_IPHONE::clipRectToScreen(int16 &x, int16 &y, int16 &w, int16 &h) {
	if (x < 0) {
		w += x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		y = 0;
	}

	if (w > _screenWidth - x)
		w = _screenWidth - x;

	if (h > _screenHeight - y)
		h = _screenHeight - y;

	if (w < 0) {
		w = 0;
	}

	if (h < 0) {
		h = 0;
	}
}

void OSystem_IPHONE::updateScreen() {
	//printf("updateScreen(): %i dirty rects.\n", _dirtyRects.size());

	if (_dirtyRects.size() == 0 && _dirtyOverlayRects.size() == 0 && !_mouseDirty)
		return;

	internUpdateScreen();
	_fullScreenIsDirty = false;
	_fullScreenOverlayIsDirty = false;

	iPhone_updateScreen();
}

void OSystem_IPHONE::internUpdateScreen() {
	int16 mouseX = _mouseX - _mouseHotspotX;
	int16 mouseY = _mouseY - _mouseHotspotY;
	int16 mouseWidth = _mouseWidth;
	int16 mouseHeight = _mouseHeight;

	clipRectToScreen(mouseX, mouseY, mouseWidth, mouseHeight);

	Common::Rect mouseRect(mouseX, mouseY, mouseX + mouseWidth, mouseY + mouseHeight);

	if (_mouseDirty) {
		if (!_fullScreenIsDirty) {
			_dirtyRects.push_back(_lastDrawnMouseRect);
			_dirtyRects.push_back(mouseRect);
		}
		if (!_fullScreenOverlayIsDirty && _overlayVisible) {
			_dirtyOverlayRects.push_back(_lastDrawnMouseRect);
			_dirtyOverlayRects.push_back(mouseRect);
		}
		_mouseDirty = false;
		_lastDrawnMouseRect = mouseRect;
	}

	while (_dirtyRects.size()) {
		Common::Rect dirtyRect = _dirtyRects.remove_at(_dirtyRects.size() - 1);

		//printf("Drawing: (%i, %i) -> (%i, %i)\n", dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);

		drawDirtyRect(dirtyRect);

		if (_overlayVisible)
			drawDirtyOverlayRect(dirtyRect);

		drawMouseCursorOnRectUpdate(dirtyRect, mouseRect);
		updateHardwareSurfaceForRect(dirtyRect);
	}

	if (_overlayVisible) {
		while (_dirtyOverlayRects.size()) {
			Common::Rect dirtyRect = _dirtyOverlayRects.remove_at(_dirtyOverlayRects.size() - 1);

			//printf("Drawing: (%i, %i) -> (%i, %i)\n", dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);

			drawDirtyOverlayRect(dirtyRect);
			drawMouseCursorOnRectUpdate(dirtyRect, mouseRect);
			updateHardwareSurfaceForRect(dirtyRect);
		}
	}
}

void OSystem_IPHONE::drawDirtyRect(const Common::Rect& dirtyRect) {
	int h = dirtyRect.bottom - dirtyRect.top;
	int w = dirtyRect.right - dirtyRect.left;

	byte  *src = &_offscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
	uint16 *dst = &_fullscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
	for (int y = h; y > 0; y--) {
		for (int x = w; x > 0; x--)
			*dst++ = _palette[*src++];

		dst += _screenWidth - w;
		src += _screenWidth - w;
	}
}

void OSystem_IPHONE::drawDirtyOverlayRect(const Common::Rect& dirtyRect) {
	int h = dirtyRect.bottom - dirtyRect.top;

	uint16 *src = (uint16 *)&_overlayBuffer[dirtyRect.top * _screenWidth + dirtyRect.left];
	uint16 *dst = &_fullscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
	int x = (dirtyRect.right - dirtyRect.left) * 2;
	for (int y = h; y > 0; y--) {
		memcpy(dst, src, x);
		src += _screenWidth;
		dst += _screenWidth;
	}
}

void OSystem_IPHONE::drawMouseCursorOnRectUpdate(const Common::Rect& updatedRect, const Common::Rect& mouseRect) {
	//draw mouse on top
	if (_mouseVisible && (updatedRect.intersects(mouseRect))) {
		int srcX = 0;
		int srcY = 0;
		int left = _mouseX - _mouseHotspotX;
		if (left < 0) {
			srcX -= left;
			left = 0;
		}
		int top = _mouseY - _mouseHotspotY;
		if (top < 0) {
			srcY -= top;
			top = 0;
		}
			//int right = left + _mouseWidth;
		int bottom = top + _mouseHeight;
		if (bottom > _screenWidth)
			bottom = _screenWidth;
			int displayWidth = _mouseWidth;
		if (_mouseWidth + left > _screenWidth)
			displayWidth = _screenWidth - left;
			int displayHeight = _mouseHeight;
		if (_mouseHeight + top > _screenHeight)
			displayHeight = _screenHeight - top;
		byte *src = &_mouseBuf[srcY * _mouseWidth + srcX];
		uint16 *dst = &_fullscreen[top * _screenWidth + left];
		for (int y = displayHeight; y > srcY; y--) {
			for (int x = displayWidth; x > srcX; x--) {
				if (*src != _mouseKeyColour)
					*dst = _palette[*src];
				dst++;
				src++;
			}
			dst += _screenWidth - displayWidth + srcX;
			src += _mouseWidth - displayWidth + srcX;
		}
	}
}

void OSystem_IPHONE::updateHardwareSurfaceForRect(const Common::Rect& updatedRect) {
	iPhone_updateScreenRect(_fullscreen, updatedRect.left, updatedRect.top, updatedRect.right, updatedRect.bottom );
}

Graphics::Surface *OSystem_IPHONE::lockScreen() {
	//printf("lockScreen()\n");

	_framebuffer.pixels = _offscreen;
	_framebuffer.w = _screenWidth;
	_framebuffer.h = _screenHeight;
	_framebuffer.pitch = _screenWidth;
	_framebuffer.bytesPerPixel = 1;

	return &_framebuffer;
}

void OSystem_IPHONE::unlockScreen() {
	//printf("unlockScreen()\n");
	dirtyFullScreen();
}

void OSystem_IPHONE::setShakePos(int shakeOffset) {
	//printf("setShakePos(%i)\n", shakeOffset);
}

void OSystem_IPHONE::showOverlay() {
	//printf("showOverlay()\n");
	_overlayVisible = true;
	dirtyFullOverlayScreen();
}

void OSystem_IPHONE::hideOverlay() {
	//printf("hideOverlay()\n");
	_overlayVisible = false;
	_dirtyOverlayRects.clear();
	dirtyFullScreen();
}

void OSystem_IPHONE::clearOverlay() {
	//printf("clearOverlay()\n");
	bzero(_overlayBuffer, _screenWidth * _screenHeight * sizeof(OverlayColor));
	dirtyFullOverlayScreen();
}

void OSystem_IPHONE::grabOverlay(OverlayColor *buf, int pitch) {
	//printf("grabOverlay()\n");
	int h = _screenHeight;
	OverlayColor *src = _overlayBuffer;

	do {
		memcpy(buf, src, _screenWidth * sizeof(OverlayColor));
		src += _screenWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_IPHONE::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	//printf("copyRectToOverlay(buf, pitch=%i, x=%i, y=%i, w=%i, h=%i)\n", pitch, x, y, w, h);

	//Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x)
		w = _screenWidth - x;

	if (h > _screenHeight - y)
		h = _screenHeight - y;

	if (w <= 0 || h <= 0)
		return;

	if (!_fullScreenOverlayIsDirty) {
		_dirtyOverlayRects.push_back(Common::Rect(x, y, x + w, y + h));
	}

	OverlayColor *dst = _overlayBuffer + (y * _screenWidth + x);
	if (_screenWidth == pitch && pitch == w)
		memcpy(dst, buf, h * w * sizeof(OverlayColor));
	else {
		do {
			memcpy(dst, buf, w * sizeof(OverlayColor));
			buf += pitch;
			dst += _screenWidth;
		} while (--h);
	}
}

int16 OSystem_IPHONE::getOverlayHeight() {
	return _screenHeight;
}

int16 OSystem_IPHONE::getOverlayWidth() {
	return _screenWidth;
}

bool OSystem_IPHONE::showMouse(bool visible) {
	bool last = _mouseVisible;
	_mouseVisible = visible;
	_mouseDirty = true;

	return last;
}

void OSystem_IPHONE::warpMouse(int x, int y) {
	//printf("warpMouse()\n");

	_mouseX = x;
	_mouseY = y;
	_mouseDirty = true;
}

void OSystem_IPHONE::dirtyFullScreen() {
	if (!_fullScreenIsDirty) {
		_dirtyRects.clear();
		_dirtyRects.push_back(Common::Rect(0, 0, _screenWidth, _screenHeight));
		_fullScreenIsDirty = true;
	}
}

void OSystem_IPHONE::dirtyFullOverlayScreen() {
	if (!_fullScreenOverlayIsDirty) {
		_dirtyOverlayRects.clear();
		_dirtyOverlayRects.push_back(Common::Rect(0, 0, _screenWidth, _screenHeight));
		_fullScreenOverlayIsDirty = true;
	}
}

void OSystem_IPHONE::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale) {
	//printf("setMouseCursor(%i, %i)\n", hotspotX, hotspotY);

	if (_mouseBuf != NULL && (_mouseWidth != w || _mouseHeight != h)) {
		free(_mouseBuf);
		_mouseBuf = NULL;
	}

	if (_mouseBuf == NULL)
		_mouseBuf = (byte *)malloc(w * h);

	_mouseWidth = w;
	_mouseHeight = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColour = keycolor;

	memcpy(_mouseBuf, buf, w * h);

	_mouseDirty = true;
}

bool OSystem_IPHONE::pollEvent(Common::Event &event) {
	//printf("pollEvent()\n");

	long curTime = getMillis();

	if (_timerCallback && (curTime >= _timerCallbackNext)) {
		_timerCallback(_timerCallbackTimer);
		_timerCallbackNext = curTime + _timerCallbackTimer;
	}

	if (_needEventRestPeriod) {
		// Workaround: Some engines can't handle mouse-down and mouse-up events
		// appearing right after each other, without a call returning no input in between.
		_needEventRestPeriod = false;
		return false;
	}

	if (_queuedInputEvent.type != (Common::EventType)0) {
		event = _queuedInputEvent;
		_queuedInputEvent.type = (Common::EventType)0;
		return true;
	}

	int eventType;
	float xUnit, yUnit;

	if (iPhone_fetchEvent(&eventType, &xUnit, &yUnit)) {
		int x = 0;
		int y = 0;
		switch (_screenOrientation) {
			case kScreenOrientationPortrait:
				x = (int)(xUnit * _screenWidth);
				y = (int)(yUnit * _screenHeight);
				break;
			case kScreenOrientationLandscape:
				x = (int)(yUnit * _screenWidth);
				y = (int)((1.0 - xUnit) * _screenHeight);
				break;
			case kScreenOrientationFlippedLandscape:
				x = (int)((1.0 - yUnit) * _screenWidth);
				y = (int)(xUnit * _screenHeight);
				break;
		}

		switch ((InputEvent)eventType) {
			case kInputMouseDown:
				if (!handleEvent_mouseDown(event, x, y))
					return false;
				break;

			case kInputMouseUp:
			if (!handleEvent_mouseUp(event, x, y))
				return false;
				break;

			case kInputMouseDragged:
				if (!handleEvent_mouseDragged(event, x, y))
					return false;
				break;
			case kInputMouseSecondDragged:
				if (!handleEvent_mouseSecondDragged(event, x, y))
					return false;
				break;
			case kInputMouseSecondDown:
				_secondaryTapped = true;
				if (!handleEvent_secondMouseDown(event, x, y))
					return false;
				break;
			case kInputMouseSecondUp:
				_secondaryTapped = false;
				if (!handleEvent_secondMouseUp(event, x, y))
					return false;
				break;
			case kInputOrientationChanged:
				handleEvent_orientationChanged((int)xUnit);
				return false;
				break;

			case kInputApplicationSuspended:
				suspendLoop();
				return false;
				break;

			case kInputKeyPressed:
				handleEvent_keyPressed(event, (int)xUnit);
				break;

			case kInputSwipe:
				if (!handleEvent_swipe(event, (int)xUnit))
					return false;
				break;

			default:
				break;
		}

		return true;
	}
	return false;
}

bool OSystem_IPHONE::handleEvent_mouseDown(Common::Event &event, int x, int y) {
	//printf("Mouse down at (%u, %u)\n", x, y);

	// Workaround: kInputMouseSecondToggled isn't always sent when the
	// secondary finger is lifted. Need to make sure we get out of that mode.
	_secondaryTapped = false;

	if (_touchpadModeEnabled) {
		_lastPadX = x;
		_lastPadY = y;
	} else
		warpMouse(x, y);

	if (_mouseClickAndDragEnabled) {
		event.type = Common::EVENT_LBUTTONDOWN;
		event.mouse.x = _mouseX;
		event.mouse.y = _mouseY;
		return true;
	} else {
		_lastMouseDown = getMillis();
	}
	return false;
}

bool OSystem_IPHONE::handleEvent_mouseUp(Common::Event &event, int x, int y) {
	//printf("Mouse up at (%u, %u)\n", x, y);

	if (_secondaryTapped) {
		_secondaryTapped = false;
		if (!handleEvent_secondMouseUp(event, x, y))
			return false;
	}
	else if (_mouseClickAndDragEnabled) {
		event.type = Common::EVENT_LBUTTONUP;
		event.mouse.x = _mouseX;
		event.mouse.y = _mouseY;
	} else {
		if (getMillis() - _lastMouseDown < 250) {
			event.type = Common::EVENT_LBUTTONDOWN;
			event.mouse.x = _mouseX;
			event.mouse.y = _mouseY;

			_queuedInputEvent.type = Common::EVENT_LBUTTONUP;
			_queuedInputEvent.mouse.x = _mouseX;
			_queuedInputEvent.mouse.y = _mouseY;
			_lastMouseTap = getMillis();
			_needEventRestPeriod = true;
		} else
			return false;
	}

	return true;
}

bool OSystem_IPHONE::handleEvent_secondMouseDown(Common::Event &event, int x, int y) {
	_lastSecondaryDown = getMillis();
	_gestureStartX = x;
	_gestureStartY = y;

	if (_mouseClickAndDragEnabled) {
		event.type = Common::EVENT_LBUTTONUP;
		event.mouse.x = _mouseX;
		event.mouse.y = _mouseY;

		_queuedInputEvent.type = Common::EVENT_RBUTTONDOWN;
		_queuedInputEvent.mouse.x = _mouseX;
		_queuedInputEvent.mouse.y = _mouseY;
	}
	else
		return false;

	return true;
}

bool OSystem_IPHONE::handleEvent_secondMouseUp(Common::Event &event, int x, int y) {
	int curTime = getMillis();

	if (curTime - _lastSecondaryDown < 400 ) {
		//printf("Right tap!\n");
		if (curTime - _lastSecondaryTap < 400 && !_overlayVisible) {
			//printf("Right escape!\n");
			event.type = Common::EVENT_KEYDOWN;
			_queuedInputEvent.type = Common::EVENT_KEYUP;

			event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
			event.kbd.keycode = _queuedInputEvent.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = _queuedInputEvent.kbd.ascii = Common::ASCII_ESCAPE;
			_needEventRestPeriod = true;
			_lastSecondaryTap = 0;
		} else if (!_mouseClickAndDragEnabled) {
			//printf("Rightclick!\n");
			event.type = Common::EVENT_RBUTTONDOWN;
			event.mouse.x = _mouseX;
			event.mouse.y = _mouseY;
			_queuedInputEvent.type = Common::EVENT_RBUTTONUP;
			_queuedInputEvent.mouse.x = _mouseX;
			_queuedInputEvent.mouse.y = _mouseY;
			_lastSecondaryTap = curTime;
			_needEventRestPeriod = true;
		} else {
			//printf("Right nothing!\n");
			return false;
		}
	}
	if (_mouseClickAndDragEnabled) {
		event.type = Common::EVENT_RBUTTONUP;
		event.mouse.x = _mouseX;
		event.mouse.y = _mouseY;
	}

	return true;
}

bool OSystem_IPHONE::handleEvent_mouseDragged(Common::Event &event, int x, int y) {
	if (_lastDragPosX == x && _lastDragPosY == y)
		return false;

	_lastDragPosX = x;
	_lastDragPosY = y;

	//printf("Mouse dragged at (%u, %u)\n", x, y);
	int mouseNewPosX;
	int mouseNewPosY;
	if (_touchpadModeEnabled ) {
		int deltaX = _lastPadX - x;
		int deltaY = _lastPadY - y;
		_lastPadX = x;
		_lastPadY = y;

		mouseNewPosX = (int)(_mouseX - deltaX / 0.5f);
		mouseNewPosY = (int)(_mouseY - deltaY / 0.5f);

		if (mouseNewPosX < 0)
			mouseNewPosX = 0;
		else if (mouseNewPosX > _screenWidth)
			mouseNewPosX = _screenWidth;

		if (mouseNewPosY < 0)
			mouseNewPosY = 0;
		else if (mouseNewPosY > _screenHeight)
			mouseNewPosY = _screenHeight;

	} else {
		mouseNewPosX = x;
		mouseNewPosY = y;
	}

	event.type = Common::EVENT_MOUSEMOVE;
	event.mouse.x = mouseNewPosX;
	event.mouse.y = mouseNewPosY;
	warpMouse(mouseNewPosX, mouseNewPosY);

	return true;
}

bool OSystem_IPHONE::handleEvent_mouseSecondDragged(Common::Event &event, int x, int y) {
	if (_gestureStartX == -1 || _gestureStartY == -1) {
		return false;
	}

	static const int kNeededLength = 100;
	static const int kMaxDeviation = 20;

	int vecX = (x - _gestureStartX);
	int vecY = (y - _gestureStartY);
	
	int absX = abs(vecX);
	int absY = abs(vecY);

	//printf("(%d, %d)\n", vecX, vecY);

	if (absX >= kNeededLength || absY >= kNeededLength) { // Long enough gesture to react upon.
		_gestureStartX = -1;
		_gestureStartY = -1;

		if (absX < kMaxDeviation && vecY >= kNeededLength) {
			// Swipe down
			event.type = Common::EVENT_KEYDOWN;
			_queuedInputEvent.type = Common::EVENT_KEYUP;

			event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
			event.kbd.keycode = _queuedInputEvent.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = _queuedInputEvent.kbd.ascii = Common::ASCII_F5;
			_needEventRestPeriod = true;
			return true;
		}
		
		if (absX < kMaxDeviation && -vecY >= kNeededLength) {
			// Swipe up
			_mouseClickAndDragEnabled = !_mouseClickAndDragEnabled;
			const char *dialogMsg;
			if (_mouseClickAndDragEnabled) {
				_touchpadModeEnabled = false;
				dialogMsg = "Mouse-click-and-drag mode enabled.";
			} else
				dialogMsg = "Mouse-click-and-drag mode disabled.";
			GUI::TimedMessageDialog dialog(dialogMsg, 1500);
			dialog.runModal();
			return false;
		}
		
		if (absY < kMaxDeviation && vecX >= kNeededLength) {
			// Swipe right
			_touchpadModeEnabled = !_touchpadModeEnabled;
			const char *dialogMsg;
			if (_touchpadModeEnabled)
				dialogMsg = "Touchpad mode enabled.";
			else
				dialogMsg = "Touchpad mode disabled.";
			GUI::TimedMessageDialog dialog(dialogMsg, 1500);
			dialog.runModal();
			return false;

		}
		
		if (absY < kMaxDeviation && -vecX >= kNeededLength) {
			// Swipe left
			return false;
		}
	}

	return false;
}

void  OSystem_IPHONE::handleEvent_orientationChanged(int orientation) {
	//printf("Orientation: %i\n", orientation);

	ScreenOrientation newOrientation;
	switch (orientation) {
		case 1:
			newOrientation = kScreenOrientationPortrait;
			break;
		case 3:
			newOrientation = kScreenOrientationLandscape;
			break;
		case 4:
			newOrientation = kScreenOrientationFlippedLandscape;
			break;
		default:
			return;
	}


	if (_screenOrientation != newOrientation) {
		_screenOrientation = newOrientation;
		iPhone_initSurface(_screenWidth, _screenHeight);

		dirtyFullScreen();
		if (_overlayVisible)
			dirtyFullOverlayScreen();
		updateScreen();
	}
}

void  OSystem_IPHONE::handleEvent_keyPressed(Common::Event &event, int keyPressed) {
	int ascii = keyPressed;
	//printf("key: %i\n", keyPressed);

	// We remap some of the iPhone keyboard keys.
	// The first ten here are the row of symbols below the numeric keys.
	switch (keyPressed) {
		case 45:
			keyPressed = Common::KEYCODE_F1;
			ascii = Common::ASCII_F1;
			break;
		case 47:
			keyPressed = Common::KEYCODE_F2;
			ascii = Common::ASCII_F2;
			break;
		case 58:
			keyPressed = Common::KEYCODE_F3;
			ascii = Common::ASCII_F3;
			break;
		case 59:
			keyPressed = Common::KEYCODE_F4;
			ascii = Common::ASCII_F4;
			break;
		case 40:
			keyPressed = Common::KEYCODE_F5;
			ascii = Common::ASCII_F5;
			break;
		case 41:
			keyPressed = Common::KEYCODE_F6;
			ascii = Common::ASCII_F6;
			break;
		case 36:
			keyPressed = Common::KEYCODE_F7;
			ascii = Common::ASCII_F7;
			break;
		case 38:
			keyPressed = Common::KEYCODE_F8;
			ascii = Common::ASCII_F8;
			break;
		case 64:
			keyPressed = Common::KEYCODE_F9;
			ascii = Common::ASCII_F9;
			break;
		case 34:
			keyPressed = Common::KEYCODE_F10;
			ascii = Common::ASCII_F10;
			break;
		case 10:
			keyPressed = Common::KEYCODE_RETURN;
			ascii = Common::ASCII_RETURN;
			break;
	}
	event.type = Common::EVENT_KEYDOWN;
	_queuedInputEvent.type = Common::EVENT_KEYUP;

	event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
	event.kbd.keycode = _queuedInputEvent.kbd.keycode = (Common::KeyCode)keyPressed;
	event.kbd.ascii = _queuedInputEvent.kbd.ascii = ascii;
	_needEventRestPeriod = true;
}

bool OSystem_IPHONE::handleEvent_swipe(Common::Event &event, int direction) {
	Common::KeyCode keycode = Common::KEYCODE_INVALID;
	switch (_screenOrientation) {
		case kScreenOrientationPortrait:
			switch ((UIViewSwipeDirection)direction) {
				case kUIViewSwipeUp:
					keycode = Common::KEYCODE_UP;
					break;
				case kUIViewSwipeDown:
					keycode = Common::KEYCODE_DOWN;
					break;
				case kUIViewSwipeLeft:
					keycode = Common::KEYCODE_LEFT;
					break;
				case kUIViewSwipeRight:
					keycode = Common::KEYCODE_RIGHT;
					break;
				default:
					return false;
			}
			break;
		case kScreenOrientationLandscape:
			switch ((UIViewSwipeDirection)direction) {
				case kUIViewSwipeUp:
					keycode = Common::KEYCODE_LEFT;
					break;
				case kUIViewSwipeDown:
					keycode = Common::KEYCODE_RIGHT;
					break;
				case kUIViewSwipeLeft:
					keycode = Common::KEYCODE_DOWN;
					break;
				case kUIViewSwipeRight:
					keycode = Common::KEYCODE_UP;
					break;
				default:
					return false;
			}
			break;
		case kScreenOrientationFlippedLandscape:
			switch ((UIViewSwipeDirection)direction) {
				case kUIViewSwipeUp:
					keycode = Common::KEYCODE_RIGHT;
					break;
				case kUIViewSwipeDown:
					keycode = Common::KEYCODE_LEFT;
					break;
				case kUIViewSwipeLeft:
					keycode = Common::KEYCODE_UP;
					break;
				case kUIViewSwipeRight:
					keycode = Common::KEYCODE_DOWN;
					break;
				default:
					return false;
			}
			break;
	}

	event.kbd.keycode = _queuedInputEvent.kbd.keycode = keycode;
	event.kbd.ascii = _queuedInputEvent.kbd.ascii = 0;
	event.type = Common::EVENT_KEYDOWN;
	_queuedInputEvent.type = Common::EVENT_KEYUP;
	event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
	_needEventRestPeriod = true;

	return true;
}

void OSystem_IPHONE::suspendLoop() {
	bool done = false;
	int eventType;
	float xUnit, yUnit;
	uint32 startTime = getMillis();

	stopSoundsystem();

	while (!done) {
		if (iPhone_fetchEvent(&eventType, &xUnit, &yUnit))
			if ((InputEvent)eventType == kInputApplicationResumed)
				done = true;
		usleep(100000);
	}

	startSoundsystem();

	_timeSuspended += getMillis() - startTime;
}

uint32 OSystem_IPHONE::getMillis() {
	//printf("getMillis()\n");

	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return (uint32)(((currentTime.tv_sec - _startTime.tv_sec) * 1000) +
	                ((currentTime.tv_usec - _startTime.tv_usec) / 1000)) - _timeSuspended;
}

void OSystem_IPHONE::delayMillis(uint msecs) {
	//printf("delayMillis(%d)\n", msecs);
	usleep(msecs * 1000);
}

OSystem::MutexRef OSystem_IPHONE::createMutex(void) {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_t *mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	if (pthread_mutex_init(mutex, &attr) != 0) {
		printf("pthread_mutex_init() failed!\n");
		free(mutex);
		return NULL;
	}

	return (MutexRef)mutex;
}

void OSystem_IPHONE::lockMutex(MutexRef mutex) {
	if (pthread_mutex_lock((pthread_mutex_t *) mutex) != 0) {
		printf("pthread_mutex_lock() failed!\n");
	}
}

void OSystem_IPHONE::unlockMutex(MutexRef mutex) {
	if (pthread_mutex_unlock((pthread_mutex_t *) mutex) != 0) {
		printf("pthread_mutex_unlock() failed!\n");
	}
}

void OSystem_IPHONE::deleteMutex(MutexRef mutex) {
	if (pthread_mutex_destroy((pthread_mutex_t *) mutex) != 0) {
		printf("pthread_mutex_destroy() failed!\n");
	} else {
		free(mutex);
	}
}

void OSystem_IPHONE::AQBufferCallback(void *in, AudioQueueRef inQ, AudioQueueBufferRef outQB) {
	//printf("AQBufferCallback()\n");
	if (s_AudioQueue.frameCount > 0 && s_soundCallback != NULL) {
		outQB->mAudioDataByteSize = 4 * s_AudioQueue.frameCount;
		s_soundCallback(s_soundParam, (byte *)outQB->mAudioData, outQB->mAudioDataByteSize);
		AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
	} else {
		AudioQueueStop(s_AudioQueue.queue, false);
	}
}

void OSystem_IPHONE::mixCallback(void *sys, byte *samples, int len) {
	OSystem_IPHONE *this_ = (OSystem_IPHONE *)sys;
	assert(this_);

	if (this_->_mixer) {
		this_->_mixer->mixCallback(samples, len);
	}
}

void OSystem_IPHONE::setupMixer() {
	//printf("setSoundCallback()\n");
	_mixer = new Audio::MixerImpl(this);

	s_soundCallback = mixCallback;
	s_soundParam = this;

	startSoundsystem();
}

void OSystem_IPHONE::startSoundsystem() {
	s_AudioQueue.dataFormat.mSampleRate = AUDIO_SAMPLE_RATE;
	s_AudioQueue.dataFormat.mFormatID = kAudioFormatLinearPCM;
	s_AudioQueue.dataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	s_AudioQueue.dataFormat.mBytesPerPacket = 4;
	s_AudioQueue.dataFormat.mFramesPerPacket = 1;
	s_AudioQueue.dataFormat.mBytesPerFrame = 4;
	s_AudioQueue.dataFormat.mChannelsPerFrame = 2;
	s_AudioQueue.dataFormat.mBitsPerChannel = 16;
	s_AudioQueue.frameCount = WAVE_BUFFER_SIZE;

	if (AudioQueueNewOutput(&s_AudioQueue.dataFormat, AQBufferCallback, &s_AudioQueue, 0, kCFRunLoopCommonModes, 0, &s_AudioQueue.queue)) {
		printf("Couldn't set the AudioQueue callback!\n");
		_mixer->setReady(false);
		return;
	}

	uint32 bufferBytes = s_AudioQueue.frameCount * s_AudioQueue.dataFormat.mBytesPerFrame;

	for (int i = 0; i < AUDIO_BUFFERS; i++) {
		if (AudioQueueAllocateBuffer(s_AudioQueue.queue, bufferBytes, &s_AudioQueue.buffers[i])) {
			printf("Error allocating AudioQueue buffer!\n");
			_mixer->setReady(false);
			return;
		}

		AQBufferCallback(&s_AudioQueue, s_AudioQueue.queue, s_AudioQueue.buffers[i]);
	}

	AudioQueueSetParameter(s_AudioQueue.queue, kAudioQueueParam_Volume, 1.0);
	if (AudioQueueStart(s_AudioQueue.queue, NULL)) {
		printf("Error starting the AudioQueue!\n");
		_mixer->setReady(false);
		return;
	}

	_mixer->setOutputRate(AUDIO_SAMPLE_RATE);
	_mixer->setReady(true);
}

void OSystem_IPHONE::stopSoundsystem() {
	AudioQueueStop(s_AudioQueue.queue, true);

	for (int i = 0; i < AUDIO_BUFFERS; i++) {
		AudioQueueFreeBuffer(s_AudioQueue.queue, s_AudioQueue.buffers[i]);
	}

	AudioQueueDispose(s_AudioQueue.queue, true);
	_mixer->setReady(false);
}

int OSystem_IPHONE::getOutputSampleRate() const {
	return AUDIO_SAMPLE_RATE;
}

void OSystem_IPHONE::setTimerCallback(TimerProc callback, int interval) {
	//printf("setTimerCallback()\n");

	if (callback != NULL) {
		_timerCallbackTimer = interval;
		_timerCallbackNext = getMillis() + interval;
		_timerCallback = callback;
	} else
		_timerCallback = NULL;
}

void OSystem_IPHONE::quit() {
}

void OSystem_IPHONE::getTimeAndDate(struct tm &t) const {
	time_t curTime = time(0);
	t = *localtime(&curTime);
}

Common::SaveFileManager *OSystem_IPHONE::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

Audio::Mixer *OSystem_IPHONE::getMixer() {
	assert(_mixer);
	return _mixer;
}

Common::TimerManager *OSystem_IPHONE::getTimerManager() {
	assert(_timer);
	return _timer;
}

OSystem *OSystem_IPHONE_create() {
	return new OSystem_IPHONE();
}

Common::SeekableReadStream *OSystem_IPHONE::createConfigReadStream() {
#ifdef IPHONE_OFFICIAL
	char buf[256];
	strncpy(buf, iPhone_getDocumentsDir(), 256);
	strncat(buf, "/Preferences", 256 - strlen(buf) );
	Common::FSNode file(buf);
#else
	Common::FSNode file(SCUMMVM_PREFS_PATH);
#endif
	return file.createReadStream();
}

Common::WriteStream *OSystem_IPHONE::createConfigWriteStream() {
#ifdef IPHONE_OFFICIAL
	char buf[256];
	strncpy(buf, iPhone_getDocumentsDir(), 256);
	strncat(buf, "/Preferences", 256 - strlen(buf) );
	Common::FSNode file(buf);
#else
	Common::FSNode file(SCUMMVM_PREFS_PATH);
#endif
	return file.createWriteStream();
}

void OSystem_IPHONE::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// Get URL of the Resource directory of the .app bundle
	CFURLRef fileUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	if (fileUrl) {
		// Try to convert the URL to an absolute path
		UInt8 buf[MAXPATHLEN];
		if (CFURLGetFileSystemRepresentation(fileUrl, true, buf, sizeof(buf))) {
			// Success: Add it to the search path
			Common::String bundlePath((const char *)buf);
			s.add("__OSX_BUNDLE__", new Common::FSDirectory(bundlePath), priority);
		}
		CFRelease(fileUrl);
	}
}

void iphone_main(int argc, char *argv[]) {

	//OSystem_IPHONE::migrateApp();

	FILE *newfp = fopen("/var/mobile/.scummvm.log", "a");
	if (newfp != NULL) {
		fclose(stdout);
		fclose(stderr);
		*stdout = *newfp;
		*stderr = *newfp;
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);

		//extern int gDebugLevel;
		//gDebugLevel = 10;
	}

#ifdef IPHONE_OFFICIAL
	chdir( iPhone_getDocumentsDir() );
#else
	system("mkdir " SCUMMVM_ROOT_PATH);
	system("mkdir " SCUMMVM_SAVE_PATH);

	chdir("/var/mobile/");
#endif

	g_system = OSystem_IPHONE_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	scummvm_main(argc, argv);
	g_system->quit();       // TODO: Consider removing / replacing this!
}
