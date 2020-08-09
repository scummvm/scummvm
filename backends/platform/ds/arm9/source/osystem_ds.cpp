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


// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
// Allow use of stuff in <nds.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include <nds.h>
#include <filesystem.h>

#include "common/scummsys.h"
#include "common/system.h"

#include "common/util.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "osystem_ds.h"
#include "dsmain.h"
#include "blitters.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "graphics/surface.h"
#include "backends/fs/devoptab/devoptab-fs-factory.h"
#include "backends/keymapper/hardware-input.h"

#include "backends/audiocd/default/default-audiocd.h"
#include "backends/events/default/default-events.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"

#include <time.h>

OSystem_DS *OSystem_DS::_instance = NULL;

OSystem_DS::OSystem_DS()
	: _eventSource(NULL), _mixer(NULL), _isOverlayShown(true),
	_disableCursorPalette(true), _graphicsEnable(true), _gammaValue(0)
{
	_instance = this;

	nitroFSInit(NULL);
	_fsFactory = new DevoptabFilesystemFactory();
}

OSystem_DS::~OSystem_DS() {
	delete _mixer;
	_mixer = 0;
}

int OSystem_DS::timerHandler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

void OSystem_DS::initBackend() {
	ConfMan.setInt("autosave_period", 0);
	ConfMan.setBool("FM_medium_quality", true);

	_eventSource = new DSEventSource();
	_eventManager = new DefaultEventManager(_eventSource);

	_savefileManager = new DefaultSaveFileManager();
	_timerManager = new DefaultTimerManager();
    DS::setTimerCallback(&OSystem_DS::timerHandler, 10);

	_mixer = new Audio::MixerImpl(11025);
	_mixer->setReady(true);

	_overlay.create(256, 192, Graphics::PixelFormat(2, 5, 5, 5, 1, 0, 5, 10, 15));

	BaseBackend::initBackend();
}

bool OSystem_DS::hasFeature(Feature f) {
	return (f == kFeatureCursorPalette);
}

void OSystem_DS::setFeatureState(Feature f, bool enable) {
	if (f == kFeatureCursorPalette) {
		_disableCursorPalette = !enable;
		refreshCursor();
	}
}

bool OSystem_DS::getFeatureState(Feature f) {
	if (f == kFeatureCursorPalette)
		return !_disableCursorPalette;
	return false;
}

void OSystem_DS::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	_framebuffer.create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	// For Lost in Time, the title screen is displayed in 640x400.
	// In order to support this game, the screen mode is set, but
	// all draw calls are ignored until the game switches to 320x200.
	if ((width == 640) && (height == 400)) {
		_graphicsEnable = false;
	} else {
		_graphicsEnable = true;
		DS::setGameSize(width, height);
	}
}

int16 OSystem_DS::getHeight() {
	return _framebuffer.h;
}

int16 OSystem_DS::getWidth() {
	return _framebuffer.w;
}

void OSystem_DS::setPalette(const byte *colors, uint start, uint num) {
	for (unsigned int r = start; r < start + num; r++) {
		int red = *colors;
		int green = *(colors + 1);
		int blue = *(colors + 2);

		red >>= 3;
		green >>= 3;
		blue >>= 3;

		{
			u16 paletteValue = red | (green << 5) | (blue << 10);

			if (!_isOverlayShown) {
				int col = applyGamma(paletteValue);
				BG_PALETTE[r] = col;
#ifdef DISABLE_TEXT_CONSOLE
				BG_PALETTE_SUB[r] = col;
#endif
			}

			_palette[r] = paletteValue;
		}

		colors += 3;
	}
}

void OSystem_DS::setCursorPalette(const byte *colors, uint start, uint num) {

	for (unsigned int r = start; r < start + num; r++) {
		int red = *colors;
		int green = *(colors + 1);
		int blue = *(colors + 2);

		red >>= 3;
		green >>= 3;
		blue >>= 3;

		u16 paletteValue = red | (green << 5) | (blue << 10);
		_cursorPalette[r] = paletteValue;

		colors += 3;
	}

	_disableCursorPalette = false;
	refreshCursor();
}

void OSystem_DS::grabPalette(unsigned char *colors, uint start, uint num) const {
	for (unsigned int r = start; r < start + num; r++) {
		*colors++ = (BG_PALETTE[r] & 0x001F) << 3;
		*colors++ = (BG_PALETTE[r] & 0x03E0) >> 5 << 3;
		*colors++ = (BG_PALETTE[r] & 0x7C00) >> 10 << 3;
	}
}

void OSystem_DS::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	_framebuffer.copyRectToSurface(buf, pitch, x, y, w, h);
}

void OSystem_DS::updateScreen() {
	if (_isOverlayShown) {
		u16 *back = (u16 *)_overlay.getPixels();
		dmaCopyHalfWords(3, back, BG_GFX, 256 * 192 * 2);
	} else if (!_graphicsEnable) {
		return;
	} else if (DS::isCpuScalerEnabled()) {
		u16 *base = BG_GFX + 0x10000;
		Rescale_320x256xPAL8_To_256x256x1555(
			base,
			(const u8 *)_framebuffer.getPixels(),
			256,
			_framebuffer.pitch,
			BG_PALETTE,
			_framebuffer.h );
	} else {
		// The DS video RAM doesn't support 8-bit writes because Nintendo wanted
		// to save a few pennies/euro cents on the hardware.

		u16 *bg = BG_GFX + 0x10000;
		s32 stride = 512;

		u16 *src = (u16 *)_framebuffer.getPixels();

		for (int dy = 0; dy < _framebuffer.h; dy++) {
			DC_FlushRange(src, _framebuffer.w << 1);

			u16 *dest1 = bg + (dy * (stride >> 1));
			DC_FlushRange(dest1, _framebuffer.w << 1);

#ifdef DISABLE_TEXT_CONSOLE
			u16 *dest2 = (u16 *)BG_GFX_SUB + (dy << 8);
			DC_FlushRange(dest2, _framebuffer.w << 1);

			dmaCopyHalfWordsAsynch(2, src, dest2, _framebuffer.w);
#endif
			dmaCopyHalfWordsAsynch(3, src, dest1, _framebuffer.w);

			while (dmaBusy(2) || dmaBusy(3));

			src += _framebuffer.pitch >> 1;
		}
	}
}

void OSystem_DS::setShakePos(int shakeXOffset, int shakeYOffset) {
	DS::setShakePos(shakeXOffset, shakeYOffset);
}

void OSystem_DS::showOverlay() {
	dmaFillHalfWords(0, BG_GFX, 256 * 192 * 2);
	videoBgEnable(2);
	lcdMainOnBottom();
	_isOverlayShown = true;
}

void OSystem_DS::hideOverlay() {
	videoBgDisable(2);
	DS::displayMode8Bit();
	_isOverlayShown = false;
}

bool OSystem_DS::isOverlayVisible() const {
	return _isOverlayShown;
}

void OSystem_DS::clearOverlay() {
	memset(_overlay.getPixels(), 0, _overlay.pitch * _overlay.h);
}

void OSystem_DS::grabOverlay(void *buf, int pitch) {
	byte *dst = (byte *)buf;

	for (int y = 0; y < _overlay.h; ++y) {
		memcpy(dst, _overlay.getBasePtr(0, y), _overlay.w * _overlay.format.bytesPerPixel);
		dst += pitch;
	}
}

void OSystem_DS::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_overlay.copyRectToSurface(buf, pitch, x, y, w, h);
}

int16 OSystem_DS::getOverlayHeight() {
	return _overlay.h;
}

int16 OSystem_DS::getOverlayWidth() {
	return _overlay.w;
}

Graphics::PixelFormat OSystem_DS::getOverlayFormat() const {
	return _overlay.format;
}

Common::Point OSystem_DS::transformPoint(uint16 x, uint16 y) {
	return DS::transformPoint(x, y, _isOverlayShown);
}

bool OSystem_DS::showMouse(bool visible) {
	DS::setShowCursor(visible);
	return true;
}

void OSystem_DS::warpMouse(int x, int y) {
	DS::warpMouse(x, y, _isOverlayShown);
}

void OSystem_DS::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, u32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	if ((w > 0) && (w < 64) && (h > 0) && (h < 64)) {
		memcpy(_cursorImage, buf, w * h);
		_cursorW = w;
		_cursorH = h;
		_cursorHotX = hotspotX;
		_cursorHotY = hotspotY;
		_cursorKey = keycolor;
		// TODO: The old target scales was saved, but never used. Should the
		// new "do not scale" logic be implemented?
		//_cursorScale = targetCursorScale;
		refreshCursor();
	}
}

void OSystem_DS::refreshCursor() {
	DS::setCursorIcon(_cursorImage, _cursorW, _cursorH, _cursorKey, _cursorHotX, _cursorHotY);
}

uint32 OSystem_DS::getMillis(bool skipRecord) {
	return DS::getMillis();
}

void OSystem_DS::delayMillis(uint msecs) {
	int st = getMillis();

	while (st + msecs >= getMillis());

	DS::doTimerCallback();
}


void OSystem_DS::getTimeAndDate(TimeDate &td) const {
	time_t curTime;
#if 0
	curTime = time(0);
#else
	curTime = 0xABCD1234 + DS::getMillis() / 1000;
#endif
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
	td.tm_wday = t.tm_wday;
}

OSystem::MutexRef OSystem_DS::createMutex(void) {
	return NULL;
}

void OSystem_DS::lockMutex(MutexRef mutex) {
}

void OSystem_DS::unlockMutex(MutexRef mutex) {
}

void OSystem_DS::deleteMutex(MutexRef mutex) {
}

void OSystem_DS::quit() {
}

Graphics::Surface *OSystem_DS::lockScreen() {
	return &_framebuffer;
}

void OSystem_DS::unlockScreen() {
	// No need to do anything here.  The screen will be updated in updateScreen().
}

void OSystem_DS::setFocusRectangle(const Common::Rect& rect) {
	DS::setTalkPos(rect.left + rect.width() / 2, rect.top + rect.height() / 2);
}

void OSystem_DS::clearFocusRectangle() {

}

void OSystem_DS::engineInit() {
#ifdef DISABLE_TEXT_CONSOLE
	videoBgEnableSub(3);
#endif
}

void OSystem_DS::engineDone() {
#ifdef DISABLE_TEXT_CONSOLE
	videoBgDisableSub(3);
#endif
}

void OSystem_DS::logMessage(LogMessageType::Type type, const char *message) {
#ifndef DISABLE_TEXT_CONSOLE
	printf("%s", message);
#endif
}

u16 OSystem_DS::applyGamma(u16 color) {
	// Attempt to do gamma correction (or something like it) to palette entries
	// to improve the contrast of the image on the original DS screen.

	// Split the color into it's component channels
	int r = color & 0x001F;
	int g = (color & 0x03E0) >> 5;
	int b = (color & 0x7C00) >> 10;

	// Caluclate the scaling factor for this color based on it's brightness
	int scale = ((23 - ((r + g + b) >> 2)) * _gammaValue) >> 3;

	// Scale the three components by the scaling factor, with clamping
	r = r + ((r * scale) >> 4);
	if (r > 31) r = 31;

	g = g + ((g * scale) >> 4);
	if (g > 31) g = 31;

	b = b + ((b * scale) >> 4);
	if (b > 31) b = 31;

	// Stick them back together into a 555 color value
	return 0x8000 | r | (g << 5) | (b << 10);
}

static const Common::HardwareInputTableEntry ndsJoystickButtons[] = {
    { "JOY_A",              Common::JOYSTICK_BUTTON_A,              _s("A")           },
    { "JOY_B",              Common::JOYSTICK_BUTTON_B,              _s("B")           },
    { "JOY_X",              Common::JOYSTICK_BUTTON_X,              _s("X")           },
    { "JOY_Y",              Common::JOYSTICK_BUTTON_Y,              _s("Y")           },
    { "JOY_BACK",           Common::JOYSTICK_BUTTON_BACK,           _s("Select")      },
    { "JOY_START",          Common::JOYSTICK_BUTTON_START,          _s("Start")       },
    { "JOY_LEFT_SHOULDER",  Common::JOYSTICK_BUTTON_LEFT_SHOULDER,  _s("L")           },
    { "JOY_RIGHT_SHOULDER", Common::JOYSTICK_BUTTON_RIGHT_SHOULDER, _s("R")           },
    { "JOY_UP",             Common::JOYSTICK_BUTTON_DPAD_UP,        _s("D-pad Up")    },
    { "JOY_DOWN",           Common::JOYSTICK_BUTTON_DPAD_DOWN,      _s("D-pad Down")  },
    { "JOY_LEFT",           Common::JOYSTICK_BUTTON_DPAD_LEFT,      _s("D-pad Left")  },
    { "JOY_RIGHT",          Common::JOYSTICK_BUTTON_DPAD_RIGHT,     _s("D-pad Right") },
    { nullptr,              0,                                      nullptr           }
};

static const Common::AxisTableEntry ndsJoystickAxes[] = {
    { nullptr, 0, Common::kAxisTypeFull, nullptr }
};

const Common::HardwareInputTableEntry ndsMouseButtons[] = {
    { "MOUSE_LEFT", Common::MOUSE_BUTTON_LEFT, _s("Touch") },
    { nullptr,      0,                         nullptr     }
};

Common::HardwareInputSet *OSystem_DS::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();
	// Touch input sends mouse events for now, so we need to declare we have a mouse...
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(ndsMouseButtons));
	inputSet->addHardwareInputSet(new JoystickHardwareInputSet(ndsJoystickButtons, ndsJoystickAxes));

	return inputSet;
}
