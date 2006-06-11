/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 * Copyright (C) 2005 Won Star - GP32 Backend
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

#include "backends/gp32/gp32_osys.h"

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "backends/gp32/globals.h"

#include "common/rect.h"
#include "common/savefile.h"
#include "common/config-manager.h"

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{0, 0, 0}
};

OSystem_GP32::OSystem_GP32() :
		_screenWidth(0), _screenHeight(0), _gameScreen(NULL), _hwScreen(NULL),
		_overlayVisible(false), _forceFull(false), _adjustAspectRatio(false),
		/*_paletteDirtyStart(0), _paletteDirtyEnd(0),*/
		_mouseBuf(NULL), _mouseHeight(0), _mouseWidth(0), _mouseKeyColor(0),
		_mouseHotspotX(0), _mouseHotspotY(0) {
	NP("OSys::OSystem_GP32()");
	// allocate palette storage
	memset(_currentPalette, 0, 256 * sizeof(uint16));

	memset(&_km, 0, sizeof(_km));

	// HACK: bring mouse cursor to center
	_mouseX = 160;
	_mouseY = 120;
}

OSystem_GP32::~OSystem_GP32() {
	NP("OSys::~OSystem_GP32()");
}

bool OSystem_GP32::hasFeature(Feature f) {
	NP("OSys::hasFeature()");
	return false;
}

void OSystem_GP32::setFeatureState(Feature f, bool enable) {
	NP("OSys::setFeatureState()");
}

bool OSystem_GP32::getFeatureState(Feature f) {
	NP("OSys::getFeatureState()");
	return false;
}

const OSystem::GraphicsMode* OSystem_GP32::getSupportedGraphicsModes() const {
	NP("OSys::getSupportedGraphicsModes()");
	return s_supportedGraphicsModes;
}


int OSystem_GP32::getDefaultGraphicsMode() const {
	NP("OSys::getSupportedGraphicsModes()");
	return -1;
}

bool OSystem_GP32::setGraphicsMode(int mode) {
	NP("OSys::setGraphicsMode()");
	return true;
}

bool OSystem_GP32::setGraphicsMode(const char *name) {
	NP("OSys::setGraphicsMode()");
	return true;
}

int OSystem_GP32::getGraphicsMode() const {
	NP("OSys::getGraphicsMode()");
	return -1;
}

void OSystem_GP32::initSize(uint width, uint height) {
	NP("OSys::initSize()");

	if (width == _screenWidth && height == _screenHeight)
		return;

	_screenWidth = width;
	_screenHeight = height;

	if (height != 200)
		_adjustAspectRatio = false;

	_overlayWidth = width;
	_overlayHeight = height;
//	_overlayWidth = 320;
//	_overlayHeight = 240;

	// Create the surface that contains the 8 bit game data
	_gameScreen = new uint8[_screenWidth * _screenHeight];

	// Create the surface that contains the scaled graphics in 16 bit mode
	_tmpScreen = frameBuffer2;

	// Create the surface that is connected with hardware screen
	_hwScreen = frameBuffer1;

	_overlayBuffer = new OverlayColor[_overlayWidth * _overlayHeight];

	_km.x_max = _screenWidth - 1;
	_km.y_max = _screenHeight - 1;
	_km.x = _mouseX;
	_km.y = _mouseY;
	_km.delay_time = 25;
	_km.last_time = 0;

	// Clear Screen
	gp_fillRect(_hwScreen, 0, 0, 320, 240, 0xFFFF);
}

int16 OSystem_GP32::getHeight() {
	//NP("OSys::getHeight()");
	return _screenHeight;
}

int16 OSystem_GP32::getWidth() {
	//NP("OSys::getWidth()");
	return _screenWidth;
}

void OSystem_GP32::setPalette(const byte *colors, uint start, uint num) {
	NP("OSys::setPalette()");
	const byte *b = colors;
	uint i;
	uint16 *base = _currentPalette + start;
	for (i = 0; i < num; i++) {
		base[i] = gp_RGBTo16(b[0], b[1], b[2]);
		b += 4;
	}

//	if (start < _paletteDirtyStart)
//		_paletteDirtyStart = start;

//	if (start + num > _paletteDirtyEnd)
//		_paletteDirtyEnd = start + num;
}

void OSystem_GP32::grabPalette(byte *colors, uint start, uint num) {
	NP("OSys::grabPalette()");
}

void OSystem_GP32::copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) {
	NP("OSys::copyRectToScreen()");
	//Clip the coordinates
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
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
	
	byte *dst = _gameScreen + y * _screenWidth + x;

	if (_screenWidth == pitch && pitch == w) {
		memcpy(dst, src, h * w);
	} else {
		do {
			memcpy(dst, src, w);
			src += pitch;
			dst += _screenWidth;
		} while (--h);
	}
}

//TODO: Implement Dirty rect?
void OSystem_GP32::updateScreen() {
	uint16 *buffer;
	//TODO: adjust shakePos

	// draw gamescreen
	buffer = &_tmpScreen[240 - _screenHeight];
	for (int x = 0; x < _screenWidth; x++) {
		for (int y = 0; y < _screenHeight; y++) {
			*buffer++ = _currentPalette[_gameScreen[((_screenHeight - 1) - y) * _screenWidth + x]];
		}
		buffer += 240 - _screenHeight;
	}
	
	// draw overlay
	if (_overlayVisible) {
		buffer = &_tmpScreen[240 - _overlayHeight];
		for (int x = 0; x < _overlayWidth; x++) {
			for (int y = 0; y < _overlayHeight; y++) {
				*buffer++ = _overlayBuffer[((_overlayHeight - 1) - y) * _overlayWidth + x];
			}
			buffer += 240 - _overlayHeight;
		}
	}

	// draw mouse
	//adjust cursor position
	int mX = _mouseX - _mouseHotspotX;
	int mY = _mouseY - _mouseHotspotY;
	//if (_overlayVisible)
	//else
	if (_mouseVisible)
		for (int y = 0; y < _mouseHeight; y++) {
			for (int x = 0; x < _mouseWidth; x++) {
				if (mX + x < _screenWidth && mY + y < _screenHeight && mX + x >= 0 && mY + y >= 0)
					if (_mouseBuf[y * _mouseWidth + x] != _mouseKeyColor)
						gpd_drawPixel16(_tmpScreen, mX + x, mY + y, _currentPalette[_mouseBuf[y * _mouseWidth + x]]);
			}
		}

	//TODO: draw softkeyboard
	//gp_flipScreen();
	//_hwScreen = frameBuffer1;
	//_tmpScreen = frameBuffer2;
	memcpy(_hwScreen, _tmpScreen, LCD_WIDTH * LCD_HEIGHT * sizeof(uint16));
}

void OSystem_GP32::setShakePos(int shakeOffset) {
	NP("OSys::setShakePos()");
}

void OSystem_GP32::showOverlay() {
	NP("OSys::showOverlay()");
	_overlayVisible = true;
	clearOverlay();
}

void OSystem_GP32::hideOverlay() {
	NP("OSys::hideOverlay()");
	_overlayVisible = false;
	clearOverlay();
	_forceFull = true;
}

// Clear overlay with game screen
//TODO: Optimize?
void OSystem_GP32::clearOverlay() {
	NP("OSys::clearOverlay()");
	if (!_overlayVisible)
		return;

	uint8 *s = _gameScreen;
	OverlayColor *d = _overlayBuffer;
	uint8 c;
	for (int y = 0; y < _overlayHeight; y++) {
		for (int x = 0; x < _overlayWidth; x++) {
			c = *s;
			*d++ = _currentPalette[c];
			s++;
		}
	}

	_forceFull = true;
}

void OSystem_GP32::grabOverlay(OverlayColor *buf, int pitch) {
	NP("OSys::grabOverlay()");
	int h = _overlayHeight;
	OverlayColor *src = _overlayBuffer;

	do {
		memcpy(buf, src, _overlayWidth * sizeof(OverlayColor));
		src += _overlayWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_GP32::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	NP("OSys::copyRectToOverlay()");

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

	if (w > _overlayWidth - x) {
		w = _overlayWidth - x;
	}

	if (h > _overlayHeight - y) {
		h = _overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	
	OverlayColor *dst = _overlayBuffer + y * _overlayWidth + x;
	if (_overlayWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w * sizeof(OverlayColor));
	} else {
		do {
			memcpy(dst, buf, w * sizeof(OverlayColor));
			buf += pitch;
			dst += _overlayWidth;
		} while (--h);
	}
}

int16 OSystem_GP32::getOverlayHeight() {
	return getHeight();
}

int16 OSystem_GP32::getOverlayWidth() {
	return getWidth();
}

OverlayColor OSystem_GP32::RGBToColor(uint8 r, uint8 g, uint8 b) {
	return gp_RGBTo16(r, g, b);
}

void OSystem_GP32::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
	gp_16ToRGB(color, &r, &g, &b);
}

bool OSystem_GP32::showMouse(bool visible) {
	NP("OSys::showMouse()");
	if (_mouseVisible == visible)
		return visible;

	bool last = _mouseVisible;
	_mouseVisible = visible;

	updateScreen();

	return last;
}

void OSystem_GP32::warpMouse(int x, int y) {
	NP("OSys::warpMouse()");
	//assert(x > 0 && x < _screenWidth);
	//assert(y > 0 && y < _screenHeight);
	_mouseX = x;
	_mouseY = y;
}

void OSystem_GP32::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale) {
	NP("OSys::setMouseCursor()");
	_mouseWidth = w;
	_mouseHeight = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColor = keycolor;

	if (_mouseBuf)
		free(_mouseBuf);

	_mouseBuf = (byte *)malloc(w * h);
	memcpy(_mouseBuf, buf, w * h);
}

void OSystem_GP32::handleKbdMouse() {
	uint32 curTime = getMillis();
	if (curTime >= _km.last_time + _km.delay_time) {
		_km.last_time = curTime;
		if (_km.x_down_count == 1) {
			_km.x_down_time = curTime;
			_km.x_down_count = 2;
		}
		if (_km.y_down_count == 1) {
			_km.y_down_time = curTime;
			_km.y_down_count = 2;
		}

		if (_km.x_vel || _km.y_vel) {
			if (_km.x_down_count) {
				if (curTime > _km.x_down_time + _km.delay_time * 12) {
					if (_km.x_vel > 0)
						_km.x_vel++;
					else
						_km.x_vel--;
				} else if (curTime > _km.x_down_time + _km.delay_time * 8) {
					if (_km.x_vel > 0)
						_km.x_vel = 5;
					else
						_km.x_vel = -5;
				}
			}
			if (_km.y_down_count) {
				if (curTime > _km.y_down_time + _km.delay_time * 12) {
					if (_km.y_vel > 0)
						_km.y_vel++;
					else
						_km.y_vel--;
				} else if (curTime > _km.y_down_time + _km.delay_time * 8) {
					if (_km.y_vel > 0)
						_km.y_vel = 5;
					else
						_km.y_vel = -5;
				}
			}

			//GPDEBUG("%d %d - %d %d", _km.x, _km.y, _km.x_vel, _km.y_vel);
			_km.x += _km.x_vel;
			_km.y += _km.y_vel;

			if (_km.x < 0) {
				_km.x = 0;
				_km.x_vel = -1;
				_km.x_down_count = 1;
			} else if (_km.x > _km.x_max) {
				_km.x = _km.x_max;
				_km.x_vel = 1;
				_km.x_down_count = 1;
			}

			if (_km.y < 0) {
				_km.y = 0;
				_km.y_vel = -1;
				_km.y_down_count = 1;
			} else if (_km.y > _km.y_max) {
				_km.y = _km.y_max;
				_km.y_vel = 1;
				_km.y_down_count = 1;
			}

			warpMouse(_km.x, _km.y);
		}
	}
}

void OSystem_GP32::fillMouseEvent(Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;

	// Update the "keyboard mouse" coords
	_km.x = x;
	_km.y = y;

	// Optionally perform aspect ratio adjusting
	//if (_adjustAspectRatio)
	//	event.mouse.y = aspect2Real(event.mouse.y);
}

bool OSystem_GP32::pollEvent(Event &event) {
	NP("OSys::pollEvent()");
	GP32BtnEvent ev;

	handleKbdMouse();

	if (!gp_pollButtonEvent(&ev))
		return false;
	
	switch(ev.type) {
	case BUTTON_DOWN:
		if (ev.button == GPC_VK_LEFT) {
			_km.x_vel = -1;
			_km.x_down_count = 1;
		}
		if (ev.button == GPC_VK_RIGHT) {
			_km.x_vel =  1;
			_km.x_down_count = 1;
		}
		if (ev.button == GPC_VK_UP) {
			_km.y_vel = -1;
			_km.y_down_count = 1;
		}
		if (ev.button == GPC_VK_DOWN) {
			_km.y_vel =  1;
			_km.y_down_count = 1;
		}
		if (ev.button == GPC_VK_START) {	// START = menu/enter
			event.type = EVENT_KEYDOWN;
			if (_overlayVisible)
				event.kbd.keycode = event.kbd.ascii = 13;
			else
				event.kbd.keycode = event.kbd.ascii = 319;
			return true;
		}
		if (ev.button == GPC_VK_SELECT) {	// SELECT = pause
			event.type = EVENT_KEYDOWN;
			event.kbd.keycode = event.kbd.ascii = 32;
			return true;
		}
		if (ev.button == GPC_VK_FL) {
			event.type = EVENT_KEYDOWN;
			event.kbd.keycode = event.kbd.ascii = '0';
			return true;
		}
		if (ev.button == GPC_VK_FR) { // R = ESC
			event.type = EVENT_KEYDOWN;
			event.kbd.keycode = event.kbd.ascii = 27;
			return true;
		}
		if (ev.button == GPC_VK_FA) {
			event.type = EVENT_LBUTTONDOWN;
			fillMouseEvent(event, _km.x, _km.y);
			return true;
		}
		if (ev.button == GPC_VK_FB) {
			event.type = EVENT_RBUTTONDOWN;
			fillMouseEvent(event, _km.x, _km.y);
			return true;
		}
		break;
	case BUTTON_UP:
		if (ev.button == GPC_VK_LEFT) {
			if (_km.x_vel < 0) {
				_km.x_vel = 0;
				_km.x_down_count = 0;
			}
			event.type = EVENT_MOUSEMOVE;
			fillMouseEvent(event, _km.x, _km.y);
			return true;
		}
		if (ev.button == GPC_VK_RIGHT) {
			if (_km.x_vel > 0) {
				_km.x_vel = 0;
				_km.x_down_count = 0;
			}
			event.type = EVENT_MOUSEMOVE;
			fillMouseEvent(event, _km.x, _km.y);
			return true;
		}
		if (ev.button == GPC_VK_UP) {
			if (_km.y_vel < 0) {
				_km.y_vel = 0;
				_km.y_down_count = 0;
			}
			event.type = EVENT_MOUSEMOVE;
			fillMouseEvent(event, _km.x, _km.y);
			return true;
		}
		if (ev.button == GPC_VK_DOWN) {
			if (_km.y_vel > 0) {
				_km.y_vel = 0;
				_km.y_down_count = 0;
			}
			event.type = EVENT_MOUSEMOVE;
			fillMouseEvent(event, _km.x, _km.y);
			return true;
		}

		if (ev.button == GPC_VK_START) {
			event.type = EVENT_KEYUP;
			if (_overlayVisible)
				event.kbd.keycode = event.kbd.ascii = 13;
			else
				event.kbd.keycode = event.kbd.ascii = 319;
			return true;
		}
		if (ev.button == GPC_VK_SELECT) {
			event.type = EVENT_KEYUP;
			event.kbd.keycode = event.kbd.ascii = 32;
			return true;
		}
		if (ev.button == GPC_VK_FL) {
			event.type = EVENT_KEYUP;
			event.kbd.keycode = event.kbd.ascii = '0';
			return true;
		}
		if (ev.button == GPC_VK_FR) {
			event.type = EVENT_KEYUP;
			event.kbd.keycode = event.kbd.ascii = 27;
			return true;
		}
		if (ev.button == GPC_VK_FA) {
			event.type = EVENT_LBUTTONUP;
			fillMouseEvent(event, _km.x, _km.y);
			return true;
		}
		if (ev.button == GPC_VK_FB) {
			event.type = EVENT_RBUTTONUP;
			fillMouseEvent(event, _km.x, _km.y);
			return true;
		}
		break;
	default:
		error("Unknown Event!");
	}

	if (gp_getButtonPressed(GPC_VK_LEFT) ||
		gp_getButtonPressed(GPC_VK_RIGHT) ||
		gp_getButtonPressed(GPC_VK_UP) ||
		gp_getButtonPressed(GPC_VK_DOWN)) {
		event.type = EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);
		return true;
	}
	return false;
}

uint32 OSystem_GP32::getMillis() {
	return GpTickCountGet();
}

void OSystem_GP32::delayMillis(uint msecs) {
	int startTime = GpTickCountGet();
	while (GpTickCountGet() < startTime + msecs);
}

// Add a new callback timer
//FIXME: Add to member
int _timerInterval;
int (*_timerCallback)(int);

static void _timerCallbackVoid() {
	//NP("timer running");
	_timerCallback(_timerInterval);	//FIXME ?? (*_timercallback)(_timerinterval);
}

void OSystem_GP32::setTimerCallback(TimerProc callback, int interval) {
	NP("OSys::setTimerCallback()");

	int timerNo = 1;

	if (callback == NULL) {
		GpTimerKill(timerNo);
		return;
	}

	if (GpTimerOptSet(timerNo, interval, 0, _timerCallbackVoid) == GPOS_ERR_ALREADY_USED) {
		error("Timer slot is already used");
	}

	_timerInterval = interval;
	_timerCallback = callback;

	GpTimerSet(timerNo);
}

OSystem::MutexRef OSystem_GP32::createMutex() {
//	NP("OSys::createMutex()");
	return NULL;
}

void OSystem_GP32::lockMutex(MutexRef mutex) {
//	NP("OSys::lockMutex()");
}

void OSystem_GP32::unlockMutex(MutexRef mutex) {
//	NP("OSys::unlockMutex()");
}

void OSystem_GP32::deleteMutex(MutexRef mutex) {
//	NP("OSys::deleteMutex()");
}

bool OSystem_GP32::setSoundCallback(SoundProc proc, void *param) {
	NP("OSys::setSoundCallback()");

	GPSOUNDBUF gpSoundBuf;

	ConfMan.setBool("FM_medium_quality", (g_vars.fmQuality == FM_QUALITY_MED));
	ConfMan.setBool("FM_high_quality", (g_vars.fmQuality == FM_QUALITY_HI));
	//ConfMan.set("output_rate", (int)g_vars.sampleRate);

	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");
		
	_samplesPerSec = (int)g_vars.sampleRate; //hack
	
	if (_samplesPerSec == 0) {
		return false;
	}

	if (_samplesPerSec < 0)
		_samplesPerSec = SAMPLES_PER_SEC;

	// Originally, we always used 2048 samples. This loop will produce the
	// same result at 22050 Hz, and should hopefully produce something
	// sensible for other frequencies. Note that it must be a power of two.

	uint32 samples = 0x8000;

	for (;;) {
		if ((1000 * samples) / _samplesPerSec < 100)
			break;
		samples >>= 1;
	}

	switch(_samplesPerSec) {
		case 44100:
		case 22050:
		case 11025:
			break;
		default:
			_samplesPerSec = 11025;
	}

	gpSoundBuf.freq = _samplesPerSec;
	gpSoundBuf.format = 16;
	gpSoundBuf.channels = 2;
	gpSoundBuf.samples = samples;
	gpSoundBuf.userdata = param;
	gpSoundBuf.callback = proc;
	gp_soundBufStart(&gpSoundBuf);

	// For Safety...
	GPDEBUG("_samplesPerSec = %d, samples = %d", _samplesPerSec, samples);
	gp_delay(1000);
	return true;
}

void OSystem_GP32::clearSoundCallback() {
	NP("OSys::clearSoundCallback()");
	if (_samplesPerSec != 0)
		gp_soundBufStop();
}

int OSystem_GP32::getOutputSampleRate() const {
	NP("OSys::getOutputSampleRate()");
	return _samplesPerSec;
}

void OSystem_GP32::quit() {
	NP("OSys::quit()");
	clearSoundCallback();
	setTimerCallback(0, 0);
	exit(0);
}

void OSystem_GP32::setWindowCaption(const char *caption) {
	NP("OSys::setWindowCaption(%s)", caption);
}

void OSystem_GP32::displayMessageOnOSD(const char *msg) {
	NP("OSys::displayMessageOnOSD(%s)", msg);
}

OSystem *OSystem_GP32_create() {
	NP("OSys::OSystem_GP32_create()");
	return new OSystem_GP32();
}
