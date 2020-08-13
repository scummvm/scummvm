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
	_graphicsMode(GFX_HWSCALE), _stretchMode(100),
	_disableCursorPalette(true), _graphicsEnable(true),
	_callbackTimer(10), _currentTimeMillis(0)
{
	_instance = this;

	nitroFSInit(NULL);
	_fsFactory = new DevoptabFilesystemFactory();
}

OSystem_DS::~OSystem_DS() {
	delete _mixer;
	_mixer = 0;
}

void timerTickHandler() {
	OSystem_DS *system = OSystem_DS::instance();
	if (system->_callbackTimer > 0) {
		system->_callbackTimer--;
	}
	system->_currentTimeMillis++;
}

void OSystem_DS::initBackend() {
	DS::initHardware();

	defaultExceptionHandler();

	ConfMan.setInt("autosave_period", 0);
	ConfMan.setBool("FM_medium_quality", true);

	_eventSource = new DSEventSource();
	_eventManager = new DefaultEventManager(_eventSource);

	_savefileManager = new DefaultSaveFileManager();
	_timerManager = new DefaultTimerManager();
	timerStart(0, ClockDivider_1, (u16)TIMER_FREQ(1000), timerTickHandler);
	REG_IME = 1;

	_mixer = new Audio::MixerImpl(11025);
	_mixer->setReady(true);

	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);
	_cursorSprite = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_Bmp);

	_overlay.create(256, 192, Graphics::PixelFormat(2, 5, 5, 5, 1, 0, 5, 10, 15));

	BaseBackend::initBackend();
}

bool OSystem_DS::hasFeature(Feature f) {
	return (f == kFeatureCursorPalette) || (f == kFeatureStretchMode);
}

void OSystem_DS::setFeatureState(Feature f, bool enable) {
	if (f == kFeatureCursorPalette) {
		_disableCursorPalette = !enable;
		refreshCursor(_cursorSprite, _cursor, !_disableCursorPalette ? _cursorPalette : _palette);
	}
}

bool OSystem_DS::getFeatureState(Feature f) {
	if (f == kFeatureCursorPalette)
		return !_disableCursorPalette;
	return false;
}

static const OSystem::GraphicsMode graphicsModes[] = {
	{ "NONE",  _s("Unscaled"),                                  GFX_NOSCALE },
	{ "HW",    _s("Hardware scale (fast, but low quality)"),    GFX_HWSCALE },
	{ "SW",    _s("Software scale (good quality, but slower)"), GFX_SWSCALE },
	{ nullptr, nullptr,                                         0           }
};

const OSystem::GraphicsMode *OSystem_DS::getSupportedGraphicsModes() const {
	return graphicsModes;
}

int OSystem_DS::getDefaultGraphicsMode() const {
	return GFX_HWSCALE;
}

bool OSystem_DS::setGraphicsMode(int mode) {
	switch (mode) {
	case GFX_NOSCALE:
	case GFX_HWSCALE:
	case GFX_SWSCALE:
		_graphicsMode = mode;
		return true;
	default:
		return false;
	}
}

int OSystem_DS::getGraphicsMode() const {
	return _graphicsMode;
}

static const OSystem::GraphicsMode stretchModes[] = {
	{ "100",   "100%",  100 },
	{ "150",   "150%",  150 },
	{ "200",   "200%",  200 },
	{ nullptr, nullptr, 0   }
};

const OSystem::GraphicsMode *OSystem_DS::getSupportedStretchModes() const {
	return stretchModes;
}

int OSystem_DS::getDefaultStretchMode() const {
	return 100;
}

bool OSystem_DS::setStretchMode(int mode) {
	_stretchMode = mode;
	DS::setTopScreenZoom(mode);
	return true;
}

int OSystem_DS::getStretchMode() const {
	return _stretchMode;
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
				BG_PALETTE[r] = paletteValue;
#ifdef DISABLE_TEXT_CONSOLE
				BG_PALETTE_SUB[r] = paletteValue;
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
	refreshCursor(_cursorSprite, _cursor, !_disableCursorPalette ? _cursorPalette : _palette);
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
	oamSet(&oamMain, 0, _cursorPos.x - _cursorHotX, _cursorPos.y - _cursorHotY, 0, 15, SpriteSize_64x64,
	       SpriteColorFormat_Bmp, _cursorSprite, 0, false, !_cursorVisible, false, false, false);
	oamUpdate(&oamMain);

	if (_isOverlayShown) {
		u16 *back = (u16 *)_overlay.getPixels();
		dmaCopyHalfWords(3, back, BG_GFX, 256 * 192 * 2);
	} else if (!_graphicsEnable) {
		return;
	} else if (_graphicsMode == GFX_SWSCALE) {
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
	const bool last = _cursorVisible;
	_cursorVisible = visible;
	return last;
}

void OSystem_DS::warpMouse(int x, int y) {
	_cursorPos = DS::warpMouse(x, y, _isOverlayShown);
}

void OSystem_DS::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, u32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	if (!buf || w == 0 || h == 0 || (format && *format != Graphics::PixelFormat::createFormatCLUT8()))
		return;

	if (_cursor.w != w || _cursor.h != h)
		_cursor.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	_cursor.copyRectToSurface(buf, w, 0, 0, w, h);
	_cursorHotX = hotspotX;
	_cursorHotY = hotspotY;
	_cursorKey = keycolor;
	refreshCursor(_cursorSprite, _cursor, !_disableCursorPalette ? _cursorPalette : _palette);
}

void OSystem_DS::refreshCursor(u16 *dst, const Graphics::Surface &src, const uint16 *palette) {
	uint w = MIN<uint>(src.w, 64);
	uint h = MIN<uint>(src.h, 64);

	dmaFillHalfWords(0, dst, 64 * 64 * 2);

	for (uint y = 0; y < h; y++) {
		const uint8 *row = (const uint8 *)src.getBasePtr(0, y);
		for (uint x = 0; x < w; x++) {
			uint8 color = *row++;

			if (color != _cursorKey)
				dst[y * 64 + x] = palette[color] | 0x8000;
		}
	}
}

uint32 OSystem_DS::getMillis(bool skipRecord) {
	return _currentTimeMillis;
}

void OSystem_DS::delayMillis(uint msecs) {
	int st = getMillis();

	while (st + msecs >= getMillis());

	doTimerCallback();
}

void OSystem_DS::doTimerCallback(int interval) {
	DefaultTimerManager *tm = (DefaultTimerManager *)getTimerManager();
	if (_callbackTimer <= 0) {
		_callbackTimer += interval;
		tm->handler();
	}
}

void OSystem_DS::getTimeAndDate(TimeDate &td) const {
	time_t curTime = time(0);
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

Common::String OSystem_DS::getSystemLanguage() const {
	switch (PersonalData->language) {
		case 0: return "ja_JP";
		case 1: return "en_US";
		case 2: return "fr_FR";
		case 3: return "de_DE";
		case 4: return "it_IT";
		case 5: return "es_ES";
		case 6: return "zh_CN";
		default: return "en_US";
	}
}
