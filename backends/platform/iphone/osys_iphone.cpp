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

#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreSurface/CoreSurface.h>
#include <unistd.h>
#include <pthread.h>

#include "common/system.h"
#include "common/scummsys.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/events.h"

#include "base/main.h"

#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/intern.h"
#include "sound/mixer.h"

#include "osys_iphone.h"
#include "iphone_common.h"

const OSystem::GraphicsMode OSystem_IPHONE::s_supportedGraphicsModes[] = {
	{0, 0, 0}
};

OSystem_IPHONE::OSystem_IPHONE() :
	_savefile(NULL), _mixer(NULL), _timer(NULL), _offscreen(NULL),
	_overlayVisible(false), _overlayBuffer(NULL), _fullscreen(NULL),
	_mouseHeight(0), _mouseWidth(0), _mouseBuf(NULL), _lastMouseTap(0),
	_secondaryTapped(false), _lastSecondaryTap(0)
{	
	_queuedInputEvent.type = (Common::EventType)0;
}

OSystem_IPHONE::~OSystem_IPHONE() {
	delete _savefile;
	delete _mixer;
	delete _timer;
	delete _offscreen;
	delete _fullscreen;
}

void OSystem_IPHONE::initBackend() {
	_savefile = new DefaultSaveFileManager();
	_mixer = new Audio::Mixer();
	_timer = new DefaultTimerManager();

	setSoundCallback(Audio::Mixer::mixCallback, _mixer);
	
	gettimeofday(&_startTime, NULL);

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

	if (_offscreen)
		free(_offscreen);
	
	_offscreen = (byte *)malloc(width * height);
	bzero(_offscreen, width * height);

	if (_overlayBuffer)
		free(_overlayBuffer);

	int fullSize = _screenWidth * _screenHeight * sizeof(OverlayColor);
	_overlayBuffer = (OverlayColor *)malloc(fullSize);
	clearOverlay();
	
	if (_fullscreen) {
		free(_fullscreen);
	}
	_fullscreen = (uint16 *)malloc(fullSize);
	bzero(_fullscreen, fullSize);
	
	iPhone_initSurface(height, width);
	
	_mouseVisible = false;
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

	for (uint i = start; i < num; ++i) {
		_palette[i] = RGBToColor(b[0], b[1], b[2]);
		b += 4;
	}
}

void OSystem_IPHONE::grabPalette(byte *colors, uint start, uint num) {
	//printf("grabPalette()\n");
}

void OSystem_IPHONE::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	//printf("copyRectToScreen()\n");
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
	
	
	byte *dst = _offscreen + y * _screenWidth + x;
	if (_screenWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w);
	} else {
		do {
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _screenWidth;
		} while (--h);
	}
}

void OSystem_IPHONE::updateScreen() {
	//printf("updateScreen()\n");

	//uint16* screen = iPhone_getSurface();

	int row;	
	if (_overlayVisible) {
		for (int x = _screenWidth; x >= 1; x--) {
			row = (_screenWidth - x) * _screenHeight;
			for (int y = 0; y < _screenHeight; y++) {	
				_fullscreen[row + y] = _overlayBuffer[y * _screenWidth + x];
			}
		}
	} else {
		for (int x = _screenWidth; x >= 1; x--) {
			row = (_screenWidth - x) * _screenHeight;
			for (int y = 0; y < _screenHeight; y++) {
				_fullscreen[row + y] = _palette[_offscreen[y * _screenWidth + x]];
			}
		}		
	}

	//draw mouse on top
	if (_mouseVisible) {
		for (uint x = _mouseWidth; x >= 1; x--) {
			int mx = _mouseX + x; // + _mouseHotspotX;
			row = (_screenWidth - mx) * _screenHeight;
			if (mx >= 0 && mx < _screenWidth) {
				for (uint y = 0; y < _mouseHeight; ++y) {
					if (_mouseBuf[y * _mouseWidth + x] != _mouseKeyColour) {
						int my = _mouseY + y; // + _mouseHotspotY;

						if ( my >= 0 && my < _screenHeight)
							_fullscreen[row + my] = _palette[_mouseBuf[y * _mouseWidth + x]];
					}
				}				
			}
		}
	}

	memcpy(iPhone_getSurface(), _fullscreen, (_screenWidth * _screenHeight) * 2);
	iPhone_updateScreen();
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
	updateScreen();
}

void OSystem_IPHONE::setShakePos(int shakeOffset) {
}

void OSystem_IPHONE::showOverlay() {
	//printf("showOverlay()\n");
	_overlayVisible = true;
}

void OSystem_IPHONE::hideOverlay() {
	//printf("hideOverlay()\n");
	_overlayVisible = false;
}

void OSystem_IPHONE::clearOverlay() {
	//printf("clearOverlay()\n");
	bzero(_overlayBuffer, _screenWidth * _screenHeight * sizeof(OverlayColor));
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

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight - y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	
	OverlayColor *dst = _overlayBuffer + (y * _screenWidth + x);
	if (_screenWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w * sizeof(OverlayColor));
	} else {
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

OverlayColor OSystem_IPHONE::RGBToColor(uint8 r, uint8 g, uint8 b)
{
	return (r & 0xF8) << 8 | (g & 0xFC) << 3 | (b >> 3);
}	

void OSystem_IPHONE::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b)
{
	r = ((color & 0xF800) >> 11) << 3;
	g = ((color & 0x07e0) >> 5) << 2;
	b = (color & 0x001F) << 3;
}

bool OSystem_IPHONE::showMouse(bool visible) {
	bool last = _mouseVisible;
	_mouseVisible = visible;
	return last;
}

void OSystem_IPHONE::warpMouse(int x, int y) {
	//printf("warpMouse()\n");
	_mouseX = x;
	_mouseY = y;
}

void OSystem_IPHONE::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale) {
	//printf("setMouseCursor()\n");

	if (_mouseBuf != NULL && (_mouseWidth != w || _mouseHeight != h)) {
		free(_mouseBuf);
		_mouseBuf = NULL;
	}
	
	if (_mouseBuf == NULL) {
		_mouseBuf = (byte *)malloc(w * h);
	}

	_mouseWidth = w;
	_mouseHeight = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColour = keycolor;
	
	memcpy(_mouseBuf, buf, w * h);
}

bool OSystem_IPHONE::pollEvent(Common::Event &event) {
	//printf("pollEvent()\n");
	
	if (_queuedInputEvent.type != (Common::EventType)0) {
		event = _queuedInputEvent;
		_queuedInputEvent.type = (Common::EventType)0;
		return true;
	}

	int eventType;
	float xUnit, yUnit;

	if (iPhone_fetchEvent(&eventType, &xUnit, &yUnit)) {
		int x = (int)((1.0 - yUnit) * _screenWidth);
		int y = (int)(xUnit * _screenHeight);

		long curTime = getMillis();

		switch ((InputEvent)eventType) {
			case kInputMouseDown:
				//printf("Mouse down at (%u, %u)\n", x, y);
				_lastMouseDown = curTime;
				_mouseX = x;
				_mouseY = y;
				return false;

				break;
			case kInputMouseUp:
				//printf("Mouse up at (%u, %u)\n", x, y);

				if (curTime - _lastMouseDown < 250) {
					event.type = Common::EVENT_LBUTTONDOWN;
					event.mouse.x = _mouseX;
					event.mouse.y = _mouseY;

					_queuedInputEvent.type = Common::EVENT_LBUTTONUP;
					_queuedInputEvent.mouse.x = _mouseX;
					_queuedInputEvent.mouse.y = _mouseY;
					_lastMouseTap = curTime;
					
					// if (curTime - _lastMouseTap < 250 && !_overlayVisible) {
					// 	event.type = Common::EVENT_KEYDOWN;
					// 	_queuedInputEvent.type = Common::EVENT_KEYUP;
					// 
					// 	event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
					// 	event.kbd.keycode = _queuedInputEvent.kbd.keycode = Common::KEYCODE_ESCAPE;
					// 	event.kbd.ascii = _queuedInputEvent.kbd.ascii = 27;		
					// 							
					// 	_lastMouseTap = 0;
					// } else {
					// 
					// }
				} else {
					return false;
				}

				break;
			case kInputMouseDragged:
				//printf("Mouse dragged at (%u, %u)\n", x, y);
				if (_secondaryTapped) {
					int vecX = (x - _gestureStartX);
					int vecY = (y - _gestureStartY);
					int lengthSq =  vecX * vecX + vecY * vecY;
					//printf("Lengthsq: %u\n", lengthSq);

					if (lengthSq > 5000) { // Long enough gesture to react upon.
						_gestureStartX = x;
						_gestureStartY = y;
						
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
							event.kbd.ascii = _queuedInputEvent.kbd.ascii = 27;
						} else if (vecXNorm > -0.50 && vecXNorm < 0.50 && vecYNorm < -0.75) {
							// Swipe up
							event.type = Common::EVENT_KEYDOWN;
							_queuedInputEvent.type = Common::EVENT_KEYUP;

							event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
							event.kbd.keycode = _queuedInputEvent.kbd.keycode = Common::KEYCODE_1;
							event.kbd.ascii = _queuedInputEvent.kbd.ascii = 27;
						} else if (vecXNorm > 0.75 && vecYNorm >  -0.5 && vecYNorm < 0.5) {
							// Swipe right
							return false;
						} else if (vecXNorm < -0.75 && vecYNorm >  -0.5 && vecYNorm < 0.5) {
							// Swipe left
							return false;
						} else {
							return false;
						}				
					} else {
						return false;						
					}
				} else {
					event.type = Common::EVENT_MOUSEMOVE;
					event.mouse.x = x;
					event.mouse.y = y;
					_mouseX = x;
					_mouseY = y;					
				}
				break;
			case kInputMouseSecondToggled:
				_secondaryTapped = !_secondaryTapped;
				//printf("Mouse second at (%u, %u). State now %s.\n", x, y, _secondaryTapped ? "on" : "off");
				if (_secondaryTapped) {
					_lastSecondaryDown = curTime;
					_gestureStartX = x;
					_gestureStartY = y;
					return false;
				} else if (curTime - _lastSecondaryDown < 250 ) {
					if (curTime - _lastSecondaryTap < 250 && !_overlayVisible) {
						event.type = Common::EVENT_KEYDOWN;
						_queuedInputEvent.type = Common::EVENT_KEYUP;

						event.kbd.flags = _queuedInputEvent.kbd.flags = 0;
						event.kbd.keycode = _queuedInputEvent.kbd.keycode = Common::KEYCODE_ESCAPE;
						event.kbd.ascii = _queuedInputEvent.kbd.ascii = 27;		
				
						_lastSecondaryTap = 0;
					} else {
						event.type = Common::EVENT_RBUTTONDOWN;
						event.mouse.x = _mouseX;
						event.mouse.y = _mouseY;
						_queuedInputEvent.type = Common::EVENT_RBUTTONUP;
						_queuedInputEvent.mouse.x = _mouseX;
						_queuedInputEvent.mouse.y = _mouseY;
						_lastSecondaryTap = curTime;
					}		
				} else {
					return false;
				}
				break;
			default:
				break;
		}

		return true;
	}
	return false;
}

uint32 OSystem_IPHONE::getMillis() {
	//printf("getMillis()\n");

	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return (uint32)(((currentTime.tv_sec - _startTime.tv_sec) * 1000) +
	                ((currentTime.tv_usec - _startTime.tv_usec) / 1000));
}

void OSystem_IPHONE::delayMillis(uint msecs) {
	//printf("delayMillis(%d)\n", msecs);
	usleep(msecs * 1000);
}

OSystem::MutexRef OSystem_IPHONE::createMutex(void) {
	pthread_mutex_t *mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	return (MutexRef)mutex;
}

void OSystem_IPHONE::lockMutex(MutexRef mutex) {
	pthread_mutex_lock((pthread_mutex_t *) mutex);
}

void OSystem_IPHONE::unlockMutex(MutexRef mutex) {
	pthread_mutex_unlock((pthread_mutex_t *) mutex);
}

void OSystem_IPHONE::deleteMutex(MutexRef mutex) {
	pthread_mutex_destroy((pthread_mutex_t *) mutex);
	free(mutex);
}

bool OSystem_IPHONE::setSoundCallback(SoundProc proc, void *param) {
	return true;
}

void OSystem_IPHONE::clearSoundCallback() {
}

int OSystem_IPHONE::getOutputSampleRate() const {
	return 22050;
}

void OSystem_IPHONE::quit() {
	//exit(0);
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

void iphone_main(int argc, char *argv[]) {
	g_system = OSystem_IPHONE_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	scummvm_main(argc, argv);
	g_system->quit();       // TODO: Consider removing / replacing this!
}

#endif
