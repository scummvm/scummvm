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

#if defined(IPHONE_BACKEND)

#include <unistd.h>
#include <pthread.h>

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
#include "blit_arm.h"
#include <sys/time.h>

#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreSurface/CoreSurface.h>

const OSystem::GraphicsMode OSystem_IPHONE::s_supportedGraphicsModes[] = {
	{0, 0, 0}
};

AQCallbackStruct OSystem_IPHONE::s_AudioQueue;
SoundProc OSystem_IPHONE::s_soundCallback = NULL;
void *OSystem_IPHONE::s_soundParam = NULL;
bool OSystem_IPHONE::s_is113OrHigher = false;

OSystem_IPHONE::OSystem_IPHONE() :
	_savefile(NULL), _mixer(NULL), _timer(NULL), _offscreen(NULL),
	_overlayVisible(false), _overlayBuffer(NULL), _fullscreen(NULL),
	_mouseHeight(0), _mouseWidth(0), _mouseBuf(NULL), _lastMouseTap(0),
	_secondaryTapped(false), _lastSecondaryTap(0), _screenOrientation(kScreenOrientationFlippedLandscape),
	_needEventRestPeriod(false), _mouseClickAndDragEnabled(false),
	_gestureStartX(-1), _gestureStartY(-1), _fullScreenIsDirty(false),
	_mouseDirty(false), _timeSuspended(0)
{
	_queuedInputEvent.type = (Common::EventType)0;
	_lastDrawnMouseRect = Common::Rect(0, 0, 0, 0);
}

OSystem_IPHONE::~OSystem_IPHONE() {
	AudioQueueDispose(s_AudioQueue.queue, true);

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
	_savefile = new DefaultSaveFileManager();
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

	if (_screenOrientation != kScreenOrientationPortrait)
		iPhone_initSurface(height, width, true);
	else
		iPhone_initSurface(width, height, false);

	dirtyFullScreen();
	_mouseVisible = false;
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
		_palette[i] = RGBToColor(b[0], b[1], b[2]);
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

void OSystem_IPHONE::addDirtyRect(int16 x, int16 y, int16 w, int16 h) {
	if (_fullScreenIsDirty) {
		return;
	}

	clipRectToScreen(x, y, w, h);
	_dirtyRects.push_back(Common::Rect(x, y, x + w, y + h));
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

	if (_dirtyRects.size() == 0 && !_mouseDirty)
		return;

	internUpdateScreen();
	_fullScreenIsDirty = false;

	//memcpy(iPhone_getSurface(), _fullscreen, _screenWidth * _screenHeight * 2);

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
		_mouseDirty = false;
		_lastDrawnMouseRect = mouseRect;
	}

	while (_dirtyRects.size()) {
		Common::Rect dirtyRect = _dirtyRects.remove_at(_dirtyRects.size() - 1);

		//printf("Drawing: (%i, %i) -> (%i, %i)\n", dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);
		int h = dirtyRect.bottom - dirtyRect.top;
		int w = dirtyRect.right - dirtyRect.left;

		if (_overlayVisible) {
			switch (_screenOrientation) {
				case kScreenOrientationPortrait: {
					uint16 *src = (uint16 *)&_overlayBuffer[dirtyRect.top * _screenWidth + dirtyRect.left];
					uint16 *dst = &_fullscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
					int x = (dirtyRect.right - dirtyRect.left) * 2;
					for (int y = h; y > 0; y--) {
						memcpy(dst, src, x);
						src += _screenWidth;
						dst += _screenWidth;
					}
					break;
				}
				case kScreenOrientationLandscape: {
					uint16 *src = (uint16 *)&_overlayBuffer[(dirtyRect.bottom - 1) * _screenWidth + dirtyRect.left];
					uint16 *dst = &_fullscreen[dirtyRect.left * _screenHeight + (_screenHeight - dirtyRect.bottom)];
					blitLandscapeScreenRect16bpp(dst, src, w, h, -_screenWidth, -_screenHeight);
					break;
				}
				case kScreenOrientationFlippedLandscape: {
					uint16 *src = (uint16 *)&_overlayBuffer[dirtyRect.top * _screenWidth + dirtyRect.left];
					uint16 *dst = &_fullscreen[(_screenWidth - dirtyRect.left - 1) * _screenHeight + dirtyRect.top];
					blitLandscapeScreenRect16bpp(dst, src, dirtyRect.right - dirtyRect.left, h, _screenWidth, _screenHeight);
					break;
				}
			}
		} else {
			switch (_screenOrientation) {
				case kScreenOrientationPortrait: {
					byte  *src = &_offscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
					uint16 *dst = &_fullscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
					for (int y = h; y > 0; y--) {
						for (int x = w; x > 0; x--)
							*dst++ = _palette[*src++];

						dst += _screenWidth - w;
						src += _screenWidth - w;
					}
					break;
				}
				case kScreenOrientationLandscape: {
					byte *src = &_offscreen[(dirtyRect.bottom - 1) * _screenWidth + dirtyRect.left];
					uint16 *dst = &_fullscreen[dirtyRect.left * _screenHeight + (_screenHeight - dirtyRect.bottom)];
					blitLandscapeScreenRect8bpp(dst, src, w, h, _palette, -_screenWidth, -_screenHeight);
					break;
				}
				case kScreenOrientationFlippedLandscape: {
					byte *src = &_offscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
					uint16 *dst = &_fullscreen[(_screenWidth - dirtyRect.left - 1) * _screenHeight + dirtyRect.top];
					blitLandscapeScreenRect8bpp(dst, src, w, h, _palette, _screenWidth, _screenHeight);
					break;
				}
			}
		}

		//draw mouse on top
		if (_mouseVisible && (dirtyRect.intersects(mouseRect))) {
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

			switch (_screenOrientation) {
				case kScreenOrientationPortrait: {
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
					break;
				}
				case kScreenOrientationLandscape: {
					byte *src = &_mouseBuf[(_mouseHeight - 1 - srcY) * _mouseWidth + srcX];
					uint16 *dst = &_fullscreen[left * _screenHeight + (_screenHeight - bottom + srcY)];
					for (int x = displayWidth; x > srcX; x--) {
						for (int y = displayHeight; y > srcY; y--) {
							if (*src != _mouseKeyColour)
								*dst = _palette[*src];
							dst++;
							src -= _mouseWidth;
						}
						dst -= -_screenHeight + displayHeight - srcY;
						src += 1 - (displayHeight - srcY) * -_mouseWidth;
					}
					break;
				}
				case kScreenOrientationFlippedLandscape: {
					byte *src = &_mouseBuf[srcY * _mouseWidth + srcX];
					uint16 *dst = &_fullscreen[(_screenWidth - left - 1) * _screenHeight + top];
					for (int x = displayWidth; x > srcX; x--) {
						for (int y = displayHeight; y > srcY; y--) {
							if (*src != _mouseKeyColour)
								*dst = _palette[*src];
							dst++;
							src += _mouseWidth;
						}
						dst -= _screenHeight + displayHeight - srcY;
						src += 1 - (displayHeight - srcY) * _mouseWidth;
					}
					break;
				}
			}
		}


		uint16 *surface = iPhone_getSurface();
		if (w == _screenWidth && h == _screenHeight)
			memcpy(surface, _fullscreen, _screenWidth * _screenHeight * 2);
		else {
			switch (_screenOrientation) {
				case kScreenOrientationPortrait: {
					int width = w * 2;
					int offset = dirtyRect.top * _screenWidth + dirtyRect.left;
					uint16 *fs = _fullscreen + offset;
					surface += offset;
					for (int y = h; y > 0; y--) {
						memcpy(surface, fs, width);
						surface += _screenWidth;
						fs += _screenWidth;
					}
					break;
				}
				case kScreenOrientationLandscape: {
					int height = h * 2;
					int offset = dirtyRect.left * _screenHeight + (_screenHeight - dirtyRect.bottom);
					uint16 *fs = _fullscreen + offset;
					surface += offset;
					for (int x = w; x > 0; x--) {
						memcpy(surface, fs, height);
						surface += _screenHeight;
						fs += _screenHeight;
					}
					break;
				}
				case kScreenOrientationFlippedLandscape: {
					int height = h * 2;
					int offset = ((_screenWidth - dirtyRect.left - 1) * _screenHeight + dirtyRect.top);
					uint16 *fs = _fullscreen + offset;
					surface += offset;
					for (int x = w; x > 0; x--) {
						memcpy(surface, fs, height);
						surface -= _screenHeight;
						fs -= _screenHeight;
					}
					break;
				}
			}
		}

		//iPhone_updateScreenRect(dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom );
	}
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
	printf("setShakePos(%i)\n", shakeOffset);
}

void OSystem_IPHONE::showOverlay() {
	//printf("showOverlay()\n");
	_overlayVisible = true;
	dirtyFullScreen();
}

void OSystem_IPHONE::hideOverlay() {
	//printf("hideOverlay()\n");
	_overlayVisible = false;
	dirtyFullScreen();
}

void OSystem_IPHONE::clearOverlay() {
	//printf("clearOverlay()\n");
	bzero(_overlayBuffer, _screenWidth * _screenHeight * sizeof(OverlayColor));
	dirtyFullScreen();
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

	if (!_fullScreenIsDirty) {
		_dirtyRects.push_back(Common::Rect(x, y, x + w, y + h));
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

void OSystem_IPHONE::dirtyMouseCursor() {
	addDirtyRect(_mouseX - _mouseHotspotX, _mouseY - _mouseHotspotY, _mouseX + _mouseWidth - _mouseHotspotX, _mouseY + _mouseHeight - _mouseHotspotY);
}

void OSystem_IPHONE::dirtyFullScreen() {
	if (!_fullScreenIsDirty) {
		_dirtyRects.clear();
		_dirtyRects.push_back(Common::Rect(0, 0, _screenWidth, _screenHeight));
		_fullScreenIsDirty = true;
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
		int x;
		int y;
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
				//printf("Mouse down at (%u, %u)\n", x, y);

				// Workaround: kInputMouseSecondToggled isn't always sent when the
				// secondary finger is lifted. Need to make sure we get out of that mode.
				_secondaryTapped = false;

				warpMouse(x, y);
				// event.type = Common::EVENT_MOUSEMOVE;
				// event.mouse.x = _mouseX;
				// event.mouse.y = _mouseY;

				if (_mouseClickAndDragEnabled) {
					event.type = Common::EVENT_LBUTTONDOWN;
					event.mouse.x = _mouseX;
					event.mouse.y = _mouseY;
					return true;
				} else {
					_lastMouseDown = curTime;
				}
				return false;
				break;
			case kInputMouseUp:
				//printf("Mouse up at (%u, %u)\n", x, y);
				if (_mouseClickAndDragEnabled) {
					event.type = Common::EVENT_LBUTTONUP;
					event.mouse.x = _mouseX;
					event.mouse.y = _mouseY;
				} else {
					if (curTime - _lastMouseDown < 250) {
						event.type = Common::EVENT_LBUTTONDOWN;
						event.mouse.x = _mouseX;
						event.mouse.y = _mouseY;

						_queuedInputEvent.type = Common::EVENT_LBUTTONUP;
						_queuedInputEvent.mouse.x = _mouseX;
						_queuedInputEvent.mouse.y = _mouseY;
						_lastMouseTap = curTime;
						_needEventRestPeriod = true;
					} else
						return false;
				}

				break;
			case kInputMouseDragged:
				//printf("Mouse dragged at (%u, %u)\n", x, y);
				if (_secondaryTapped) {
					 if (_gestureStartX == -1 || _gestureStartY == -1) {
						return false;
					 }

					int vecX = (x - _gestureStartX);
					int vecY = (y - _gestureStartY);
					int lengthSq =  vecX * vecX + vecY * vecY;
					//printf("Lengthsq: %u\n", lengthSq);

					if (lengthSq > 15000) { // Long enough gesture to react upon.
						_gestureStartX = -1;
						_gestureStartY = -1;

						float vecLength = sqrt(lengthSq);
						float vecXNorm = vecX / vecLength;
						float vecYNorm = vecY / vecLength;

						//printf("Swipe vector: (%.2f, %.2f)\n", vecXNorm, vecYNorm);

						if (vecXNorm > -0.50 && vecXNorm < 0.50 && vecYNorm > 0.75) {
							// Swipe down
							event.type = Common::EVENT_KEYDOWN;
							_queuedInputEvent.type = Common::EVENT_KEYUP;

							event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
							event.kbd.keycode = _queuedInputEvent.kbd.keycode = Common::KEYCODE_F5;
							event.kbd.ascii = _queuedInputEvent.kbd.ascii = Common::ASCII_F5;
							_needEventRestPeriod = true;
						} else if (vecXNorm > -0.50 && vecXNorm < 0.50 && vecYNorm < -0.75) {
							// Swipe up
							_mouseClickAndDragEnabled = !_mouseClickAndDragEnabled;
							const char *dialogMsg;
							if (_mouseClickAndDragEnabled)
								dialogMsg = "Mouse-click-and-drag mode enabled.";
							else
								dialogMsg = "Mouse-click-and-drag mode disabled.";
							GUI::TimedMessageDialog dialog(dialogMsg, 1500);
							dialog.runModal();
							return false;
						} else if (vecXNorm > 0.75 && vecYNorm >  -0.5 && vecYNorm < 0.5) {
							// Swipe right
							// _secondaryTapped = !_secondaryTapped;
							// _gestureStartX = x;
							// _gestureStartY = y;
							//
							// GUI::TimedMessageDialog dialog("Forcing toggle of pressed state.", 1500);
							// dialog.runModal();
							return false;
						} else if (vecXNorm < -0.75 && vecYNorm >  -0.5 && vecYNorm < 0.5) {
							// Swipe left
							return false;
						} else
							return false;
					} else
						return false;
				} else {
					event.type = Common::EVENT_MOUSEMOVE;
					event.mouse.x = x;
					event.mouse.y = y;
					warpMouse(x, y);
				}
				break;
			case kInputMouseSecondToggled:
				_secondaryTapped = !_secondaryTapped;
				//printf("Mouse second at (%u, %u). State now %s.\n", x, y, _secondaryTapped ? "on" : "off");
				if (_secondaryTapped) {
					_lastSecondaryDown = curTime;
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
				} else {
					if (curTime - _lastSecondaryDown < 250 ) {
						if (curTime - _lastSecondaryTap < 250 && !_overlayVisible) {
							event.type = Common::EVENT_KEYDOWN;
							_queuedInputEvent.type = Common::EVENT_KEYUP;

							event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
							event.kbd.keycode = _queuedInputEvent.kbd.keycode = Common::KEYCODE_ESCAPE;
							event.kbd.ascii = _queuedInputEvent.kbd.ascii = Common::ASCII_ESCAPE;
							_needEventRestPeriod = true;
							_lastSecondaryTap = 0;
						} else if (!_mouseClickAndDragEnabled) {
							event.type = Common::EVENT_RBUTTONDOWN;
							event.mouse.x = _mouseX;
							event.mouse.y = _mouseY;
							_queuedInputEvent.type = Common::EVENT_RBUTTONUP;
							_queuedInputEvent.mouse.x = _mouseX;
							_queuedInputEvent.mouse.y = _mouseY;
							_lastSecondaryTap = curTime;
							_needEventRestPeriod = true;
						}
					}
					if (_mouseClickAndDragEnabled) {
						event.type = Common::EVENT_RBUTTONUP;
						event.mouse.x = _mouseX;
						event.mouse.y = _mouseY;
					}
				}
				break;
			case kInputOrientationChanged:
				//printf("Orientation: %i", (int)xUnit);

				ScreenOrientation newOrientation;
				switch ((int)xUnit) {
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
						return false;
				}


				if (_screenOrientation != newOrientation) {
					_screenOrientation = newOrientation;
					if (_screenOrientation != kScreenOrientationPortrait)
						iPhone_initSurface(_screenHeight, _screenWidth, true);
					else
						iPhone_initSurface(_screenWidth, _screenHeight, false);

					dirtyFullScreen();
					updateScreen();
				}
				break;

			case kInputApplicationSuspended:
				suspendLoop();
				break;

			case kInputKeyPressed: {
				int keyPressed = (int)xUnit;
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
				break;
			}

			case kInputSwipe: {
				Common::KeyCode keycode = Common::KEYCODE_INVALID;
				switch (_screenOrientation) {
					case kScreenOrientationPortrait:
						switch ((UIViewSwipeDirection)xUnit) {
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
						switch ((UIViewSwipeDirection)xUnit) {
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
						switch ((UIViewSwipeDirection)xUnit) {
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
				break;
			}

			default:
				break;
		}

		return true;
	}
	return false;
}

void OSystem_IPHONE::suspendLoop() {
	bool done = false;
	int eventType;
	float xUnit, yUnit;
	uint32 startTime = getMillis();

	AudioQueueStop(s_AudioQueue.queue, true);

	while (!done) {
		if (iPhone_fetchEvent(&eventType, &xUnit, &yUnit))
			if ((InputEvent)eventType == kInputApplicationResumed)
				done = true;
		usleep(100000);
	}

	AudioQueueStart(s_AudioQueue.queue, NULL);
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
	} else
		AudioQueueStop(s_AudioQueue.queue, false);
}

void OSystem_IPHONE::mixCallback(void *sys, byte *samples, int len) {
	OSystem_IPHONE *this_ = (OSystem_IPHONE *)sys;
	assert(this_);

	if (this_->_mixer)
		this_->_mixer->mixCallback(samples, len);
}

void OSystem_IPHONE::setupMixer() {
	//printf("setSoundCallback()\n");
	_mixer = new Audio::MixerImpl(this);

	s_soundCallback = mixCallback;
	s_soundParam = this;


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

void OSystem_IPHONE::setWindowCaption(const char *caption) {
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

const char* OSystem_IPHONE::getConfigPath() {
	return SCUMMVM_PREFS_PATH;
}

const char* OSystem_IPHONE::getSavePath() {
	return SCUMMVM_SAVE_PATH;
}

void OSystem_IPHONE::migrateApp() {
	// Migrate to the new 1.1.3 directory structure, if needed.
	
	FilesystemNode file("/var/mobile");
	if (file.exists() && file.isDirectory()) {
		// We have 1.1.3 or above.
		s_is113OrHigher = true;
		file = FilesystemNode(SCUMMVM_ROOT_PATH);
		if (!file.exists()) {
			system("mkdir " SCUMMVM_ROOT_PATH);
			system("mkdir " SCUMMVM_SAVE_PATH);
			
			// Copy over the prefs file
			system("cp " SCUMMVM_OLD_PREFS_PATH " " SCUMMVM_PREFS_PATH);

			file = FilesystemNode(SCUMMVM_OLD_SAVE_PATH);
			// Copy over old savegames to the new directory.
			if (file.exists() && file.isDirectory())			
				system("cp " SCUMMVM_OLD_SAVE_PATH "/* " SCUMMVM_SAVE_PATH "/");
		}
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

	system("mkdir " SCUMMVM_ROOT_PATH);
	system("mkdir " SCUMMVM_SAVE_PATH);

	g_system = OSystem_IPHONE_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	scummvm_main(argc, argv);
	g_system->quit();       // TODO: Consider removing / replacing this!
}

#endif

