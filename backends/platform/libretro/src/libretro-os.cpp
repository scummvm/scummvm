/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <sys/time.h>
#include <unistd.h>

#include <libretro.h>
#include <retro_inline.h>
#include <retro_miscellaneous.h>
#include <features/features_cpu.h>

#include "audio/mixer_intern.h"
#include "backends/base-backend.h"
#include "base/commandLine.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/tokenizer.h"
#include "common/list.h"

#if defined(_WIN32)
#include "backends/fs/windows/windows-fs-factory.h"
#define FS_SYSTEM_FACTORY WindowsFilesystemFactory
#else
#include "backends/platform/libretro/include/libretro-fs-factory.h"
#define FS_SYSTEM_FACTORY LibRetroFilesystemFactory
#endif

#include "backends/saves/default/default-saves.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "graphics/colormasks.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#if defined(_WIN32)
#include <direct.h>
#ifdef _XBOX
#include <xtl.h>
#else
#include <windows.h>
#endif
#elif defined(__CELLOS_LV2__)
#include <sys/sys_time.h>
#elif (defined(GEKKO) && !defined(WIIU))
#include <ogc/lwp_watchdog.h>
#else
#include <time.h>
#endif

#include "backends/platform/libretro/include/libretro-threads.h"
#include "backends/platform/libretro/include/os.h"

extern retro_log_printf_t log_cb;

extern bool timing_inaccuracies_is_enabled(void);
extern bool consecutive_screen_updates_is_enabled(void);
extern void reset_performance_tuner(void);
extern void retro_osd_notification(const char* msg);
extern float frame_rate;

#include "common/mutex.h"

/**
 *  Dummy mutex implementation
 */
class LibretroMutexInternal final : public Common::MutexInternal {
public:
	LibretroMutexInternal() {};
	~LibretroMutexInternal() override {};

	bool lock() override {
		return 0;
	}
	bool unlock() override {
		return 0;
	};
};

Common::MutexInternal *createLibretroMutexInternal() {
	return new LibretroMutexInternal();
}

struct RetroPalette {
	unsigned char _colors[256 * 3];

	RetroPalette() {
		memset(_colors, 0, sizeof(_colors));
	}

	void set(const byte *colors, uint start, uint num) {
		memcpy(_colors + start * 3, colors, num * 3);
	}

	void get(byte *colors, uint start, uint num) const {
		memcpy(colors, _colors + start * 3, num * 3);
	}

	unsigned char *getColor(uint aIndex) const {
		return (unsigned char *)&_colors[aIndex * 3];
	}
};

static INLINE void blit_uint8_uint16_fast(Graphics::Surface &aOut, const Graphics::Surface &aIn, const RetroPalette &aColors) {
	for (int i = 0; i < aIn.h; i++) {
		if (i >= aOut.h)
			continue;

		uint8_t *const in = (uint8_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + (i * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if (j >= aOut.w)
				continue;

			uint8 r, g, b;

			const uint8_t val = in[j];
			// if(val != 0xFFFFFFFF)
			{
				if (aIn.format.bytesPerPixel == 1) {
					unsigned char *col = aColors.getColor(val);
					r = *col++;
					g = *col++;
					b = *col++;
				} else
					aIn.format.colorToRGB(in[j], r, g, b);

				out[j] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static INLINE void blit_uint32_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, const RetroPalette &aColors) {
	for (int i = 0; i < aIn.h; i++) {
		if (i >= aOut.h)
			continue;

		uint32_t *const in = (uint32_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + (i * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if (j >= aOut.w)
				continue;

			uint8 r, g, b;

			// const uint32_t val = in[j];
			// if(val != 0xFFFFFFFF)
			{
				aIn.format.colorToRGB(in[j], r, g, b);
				out[j] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static INLINE void blit_uint16_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, const RetroPalette &aColors) {
	for (int i = 0; i < aIn.h; i++) {
		if (i >= aOut.h)
			continue;

		uint16_t *const in = (uint16_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + (i * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if (j >= aOut.w)
				continue;

			uint8 r, g, b;

			// const uint16_t val = in[j];
			// if(val != 0xFFFFFFFF)
			{
				aIn.format.colorToRGB(in[j], r, g, b);
				out[j] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static void blit_uint8_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, int aX, int aY, const RetroPalette &aColors, uint32 aKeyColor) {
	for (int i = 0; i < aIn.h; i++) {
		if ((i + aY) < 0 || (i + aY) >= aOut.h)
			continue;

		uint8_t *const in = (uint8_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + ((i + aY) * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if ((j + aX) < 0 || (j + aX) >= aOut.w)
				continue;

			uint8 r, g, b;

			const uint8_t val = in[j];
			if (val != aKeyColor) {
				unsigned char *col = aColors.getColor(val);
				r = *col++;
				g = *col++;
				b = *col++;
				out[j + aX] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static void blit_uint16_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, int aX, int aY, const RetroPalette &aColors, uint32 aKeyColor) {
	for (int i = 0; i < aIn.h; i++) {
		if ((i + aY) < 0 || (i + aY) >= aOut.h)
			continue;

		uint16_t *const in = (uint16_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + ((i + aY) * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if ((j + aX) < 0 || (j + aX) >= aOut.w)
				continue;

			uint8 r, g, b;

			const uint16_t val = in[j];
			if (val != aKeyColor) {
				aIn.format.colorToRGB(in[j], r, g, b);
				out[j + aX] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static void blit_uint32_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, int aX, int aY, const RetroPalette &aColors, uint32 aKeyColor) {
	for (int i = 0; i < aIn.h; i++) {
		if ((i + aY) < 0 || (i + aY) >= aOut.h)
			continue;

		uint32_t *const in = (uint32_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + ((i + aY) * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if ((j + aX) < 0 || (j + aX) >= aOut.w)
				continue;

			uint8 in_a, in_r, in_g, in_b;
			uint8 out_r, out_g, out_b;
			uint32_t blend_r, blend_g, blend_b;

			const uint32_t val = in[j];
			if (val != aKeyColor) {
				aIn.format.colorToARGB(in[j], in_a, in_r, in_g, in_b);

				if (in_a) {
					aOut.format.colorToRGB(out[j + aX], out_r, out_g, out_b);

					blend_r = ((in_r * in_a) + (out_r * (255 - in_a))) / 255;
					blend_g = ((in_g * in_a) + (out_g * (255 - in_a))) / 255;
					blend_b = ((in_b * in_a) + (out_b * (255 - in_a))) / 255;

					out[j + aX] = aOut.format.RGBToColor(blend_r, blend_g, blend_b);
				}
			}
		}
	}
}

static INLINE void copyRectToSurface(uint8_t *pixels, int out_pitch, const uint8_t *src, int pitch, int x, int y, int w, int h, int out_bpp) {
	uint8_t *dst = pixels + y * out_pitch + x * out_bpp;

	do {
		memcpy(dst, src, w * out_bpp);
		src += pitch;
		dst += out_pitch;
	} while (--h);
}

static Common::String s_systemDir;
static Common::String s_saveDir;

#ifdef FRONTEND_SUPPORTS_RGB565
#define SURF_BPP 2
#define SURF_RBITS 2
#define SURF_GBITS 5
#define SURF_BBITS 6
#define SURF_ABITS 5
#define SURF_ALOSS (8 - SURF_ABITS)
#define SURF_RLOSS (8 - SURF_RBITS)
#define SURF_GLOSS (8 - SURF_GBITS)
#define SURF_BLOSS (8 - SURF_BBITS)
#define SURF_RSHIFT 0
#define SURF_GSHIFT 11
#define SURF_BSHIFT 5
#define SURF_ASHIFT 0
#else
#define SURF_BPP 2
#define SURF_RBITS 5
#define SURF_GBITS 5
#define SURF_BBITS 5
#define SURF_ABITS 1
#define SURF_ALOSS (8 - SURF_ABITS)
#define SURF_RLOSS (8 - SURF_RBITS)
#define SURF_GLOSS (8 - SURF_GBITS)
#define SURF_BLOSS (8 - SURF_BBITS)
#define SURF_RSHIFT 10
#define SURF_GSHIFT 5
#define SURF_BSHIFT 0
#define SURF_ASHIFT 15
#endif

Common::List<Common::Event> _events;

class OSystem_RETRO : public EventsBaseBackend, public PaletteManager {
public:
	Graphics::Surface _screen;

	Graphics::Surface _gameScreen;
	RetroPalette _gamePalette;

	Graphics::Surface _overlay;
	bool _overlayVisible;
	bool _overlayInGUI;

	Graphics::Surface _mouseImage;
	RetroPalette _mousePalette;
	bool _mousePaletteEnabled;
	bool _mouseVisible;
	int _mouseX;
	int _mouseY;
	int _relMouseX;
	int _relMouseY;
	float _mouseXAcc;
	float _mouseYAcc;
	float _dpadXAcc;
	float _dpadYAcc;
	float _dpadXVel;
	float _dpadYVel;
	int _mouseHotspotX;
	int _mouseHotspotY;
	int _mouseKeyColor;
	bool _mouseDontScale;
	bool _mouseButtons[2];
	bool _joypadmouseButtons[2];
	bool _joypadkeyboardButtons[8];
	unsigned _joypadnumpadLast;
	bool _joypadnumpadActive;
	bool _ptrmouseButton;

	uint32 _startTime;
	uint8 _threadSwitchCaller;

	Audio::MixerImpl *_mixer;

	OSystem_RETRO() : _mousePaletteEnabled(false), _mouseVisible(false), _mouseX(0), _mouseY(0), _mouseXAcc(0.0), _mouseYAcc(0.0), _mouseHotspotX(0), _mouseHotspotY(0), _dpadXAcc(0.0), _dpadYAcc(0.0), _dpadXVel(0.0f), _dpadYVel(0.0f), _mouseKeyColor(0), _mouseDontScale(false), _joypadnumpadLast(8), _joypadnumpadActive(false), _mixer(0), _startTime(0), _threadSwitchCaller(0) {
		_fsFactory = new FS_SYSTEM_FACTORY();
		memset(_mouseButtons, 0, sizeof(_mouseButtons));
		memset(_joypadmouseButtons, 0, sizeof(_joypadmouseButtons));
		memset(_joypadkeyboardButtons, 0, sizeof(_joypadkeyboardButtons));

		_startTime = getMillis();

		if (s_systemDir.empty())
			s_systemDir = ".";

		if (s_saveDir.empty())
			s_saveDir = ".";
	}

	virtual ~OSystem_RETRO() {
		_gameScreen.free();
		_overlay.free();
		_mouseImage.free();
		_screen.free();

		delete _mixer;
	}

	virtual void initBackend() {
		_savefileManager = new DefaultSaveFileManager(s_saveDir);
#ifdef FRONTEND_SUPPORTS_RGB565
		_overlay.create(RES_W_OVERLAY, RES_H_OVERLAY, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
#else
		_overlay.create(RES_W_OVERLAY, RES_H_OVERLAY, Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15));
#endif
		_mixer = new Audio::MixerImpl(SAMPLE_RATE);

		_timerManager = new LibretroTimerManager(frame_rate);

		_mixer->setReady(true);

		EventsBaseBackend::initBackend();
	}

	virtual void engineInit() {
		Common::String engineId = ConfMan.get("engineid");
		if (engineId.equalsIgnoreCase("scumm") && ConfMan.getBool("original_gui")) {
			ConfMan.setBool("original_gui", false);
			log_cb(RETRO_LOG_INFO, "\"original_gui\" setting forced to false\n");
		}
	}

	virtual void engineDone() {
		reset_performance_tuner();
	}

	virtual bool hasFeature(Feature f) {
		return (f == OSystem::kFeatureCursorPalette);
	}

	virtual void setFeatureState(Feature f, bool enable) {
		if (f == kFeatureCursorPalette)
			_mousePaletteEnabled = enable;
	}

	virtual bool getFeatureState(Feature f) {
		return (f == kFeatureCursorPalette) ? _mousePaletteEnabled : false;
	}

	virtual const GraphicsMode *getSupportedGraphicsModes() const {
		static const OSystem::GraphicsMode s_noGraphicsModes[] = {{0, 0, 0}};
		return s_noGraphicsModes;
	}

	virtual int getDefaultGraphicsMode() const {
		return 0;
	}

	virtual bool isOverlayVisible() const {
		return false;
	}

	virtual bool setGraphicsMode(int mode) {
		return true;
	}

	virtual int getGraphicsMode() const {
		return 0;
	}

	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format) {
		_gameScreen.create(width, height, format ? *format : Graphics::PixelFormat::createFormatCLUT8());
	}

	virtual int16 getHeight() {
		return _gameScreen.h;
	}

	virtual int16 getWidth() {
		return _gameScreen.w;
	}

	virtual Graphics::PixelFormat getScreenFormat() const {
		return _gameScreen.format;
	}

	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const {
		Common::List<Graphics::PixelFormat> result;

#ifdef SCUMM_LITTLE_ENDIAN
		/* RGBA8888 */
		result.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#else
		/* ABGR8888 */
		result.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#endif
#ifdef FRONTEND_SUPPORTS_RGB565
		/* RGB565 - overlay */
		result.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
#endif
		/* RGB555 - fmtowns */
		result.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15));

		/* Palette - most games */
		result.push_back(Graphics::PixelFormat::createFormatCLUT8());

		return result;
	}

	virtual PaletteManager *getPaletteManager() {
		return this;
	}

protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num) {
		_gamePalette.set(colors, start, num);
	}

	virtual void grabPalette(byte *colors, uint start, uint num) const {
		_gamePalette.get(colors, start, num);
	}

public:
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
		const uint8_t *src = (const uint8_t *)buf;
		uint8_t *pix = (uint8_t *)_gameScreen.getPixels();
		copyRectToSurface(pix, _gameScreen.pitch, src, pitch, x, y, w, h, _gameScreen.format.bytesPerPixel);
	}

	virtual void updateScreen() {
		const Graphics::Surface &srcSurface = (_overlayInGUI) ? _overlay : _gameScreen;
		if (srcSurface.w && srcSurface.h) {
			switch (srcSurface.format.bytesPerPixel) {
			case 1:
			case 3:
				blit_uint8_uint16_fast(_screen, srcSurface, _gamePalette);
				break;
			case 2:
				blit_uint16_uint16(_screen, srcSurface, _gamePalette);
				break;
			case 4:
				blit_uint32_uint16(_screen, srcSurface, _gamePalette);
				break;
			}
		}

		// Draw Mouse
		if (_mouseVisible && _mouseImage.w && _mouseImage.h) {
			const int x = _mouseX - _mouseHotspotX;
			const int y = _mouseY - _mouseHotspotY;

			switch (_mouseImage.format.bytesPerPixel) {
			case 1:
			case 3:
				blit_uint8_uint16(_screen, _mouseImage, x, y, _mousePaletteEnabled ? _mousePalette : _gamePalette, _mouseKeyColor);
				break;
			case 2:
				blit_uint16_uint16(_screen, _mouseImage, x, y, _mousePaletteEnabled ? _mousePalette : _gamePalette, _mouseKeyColor);
				break;
			case 4:
				blit_uint32_uint16(_screen, _mouseImage, x, y, _mousePaletteEnabled ? _mousePalette : _gamePalette, _mouseKeyColor);
				break;
			}
		}

		/* Switch directly to main thread in case of consecutive updateScreen, to avoid losing frames.
		Non consecutive updateScreen are covered by thread switches triggered by pollEvent or delayMillis. */
		if (! timing_inaccuracies_is_enabled() && consecutive_screen_updates_is_enabled()) {
			if (_threadSwitchCaller & THREAD_SWITCH_UPDATE) {
				((LibretroTimerManager *)_timerManager)->switchThread();
			} else {
				_threadSwitchCaller = THREAD_SWITCH_UPDATE;
			}
		}
	}

	virtual Graphics::Surface *lockScreen() {
		return &_gameScreen;
	}

	virtual void unlockScreen() { /* EMPTY */ }

	virtual void setShakePos(int shakeXOffset, int shakeYOffset) {
		// TODO
	}

	virtual void showOverlay(bool inGUI) {
		_overlayVisible = true;
		_overlayInGUI = inGUI;
	}

	virtual void hideOverlay() {
		_overlayVisible = false;
		_overlayInGUI = false;
	}

	virtual void clearOverlay() {
		_overlay.fillRect(Common::Rect(_overlay.w, _overlay.h), 0);
	}

	virtual void grabOverlay(Graphics::Surface &surface) {
		const unsigned char *src = (unsigned char *)_overlay.getPixels();
		unsigned char *dst = (byte *)surface.getPixels();
		;
		unsigned i = RES_H_OVERLAY;

		do {
			memcpy(dst, src, RES_W_OVERLAY << 1);
			dst += surface.pitch;
			src += RES_W_OVERLAY << 1;
		} while (--i);
	}

	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
		const uint8_t *src = (const uint8_t *)buf;
		uint8_t *pix = (uint8_t *)_overlay.getPixels();
		copyRectToSurface(pix, _overlay.pitch, src, pitch, x, y, w, h, _overlay.format.bytesPerPixel);
	}

	virtual int16 getOverlayHeight() {
		return _overlay.h;
	}

	virtual int16 getOverlayWidth() {
		return _overlay.w;
	}

	virtual Graphics::PixelFormat getOverlayFormat() const {
		return _overlay.format;
	}

	virtual bool showMouse(bool visible) {
		const bool wasVisible = _mouseVisible;
		_mouseVisible = visible;
		return wasVisible;
	}

	virtual void warpMouse(int x, int y) {
		_mouseX = x;
		_mouseY = y;
	}

	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor = 255, bool dontScale = false, const Graphics::PixelFormat *format = NULL, const byte *mask = nullptr) {
		const Graphics::PixelFormat mformat = format ? *format : Graphics::PixelFormat::createFormatCLUT8();

		if (_mouseImage.w != w || _mouseImage.h != h || _mouseImage.format != mformat) {
			_mouseImage.create(w, h, mformat);
		}

		memcpy(_mouseImage.getPixels(), buf, h * _mouseImage.pitch);

		_mouseHotspotX = hotspotX;
		_mouseHotspotY = hotspotY;
		_mouseKeyColor = keycolor;
		_mouseDontScale = dontScale;
	}

	virtual void setCursorPalette(const byte *colors, uint start, uint num) {
		_mousePalette.set(colors, start, num);
		_mousePaletteEnabled = true;
	}

	uint8 getThreadSwitchCaller(){
		return _threadSwitchCaller;
	}

	virtual bool pollEvent(Common::Event &event) {
		_threadSwitchCaller = THREAD_SWITCH_POLL;
		((LibretroTimerManager *)_timerManager)->checkThread();
		((LibretroTimerManager *)_timerManager)->handler();
		if (!_events.empty()) {
			event = _events.front();
			_events.pop_front();
			return true;
		}

		return false;
	}

	virtual uint32 getMillis(bool skipRecord = false) {
#if (defined(GEKKO) && !defined(WIIU))
		return (ticks_to_microsecs(gettime()) / 1000.0) - _startTime;
#elif defined(WIIU)
		return ((cpu_features_get_time_usec()) / 1000) - _startTime;
#elif defined(__CELLOS_LV2__)
		return (sys_time_get_system_time() / 1000.0) - _startTime;
#else
		struct timeval t;
		gettimeofday(&t, 0);

		return ((t.tv_sec * 1000) + (t.tv_usec / 1000)) - _startTime;
#endif
	}

	virtual void delayMillis(uint msecs) {
		uint32 start_time = getMillis();
		uint32 elapsed_time = 0;

		_threadSwitchCaller = THREAD_SWITCH_DELAY;

		if (timing_inaccuracies_is_enabled()) {
			while (elapsed_time < msecs) {
				/* When remaining delay would take us past the next thread switch time, we switch immediately
				in order to burn as much as possible delay time in the main RetroArch thread as soon as possible. */
				if (msecs - elapsed_time >= ((LibretroTimerManager *)_timerManager)->timeToNextSwitch())
					((LibretroTimerManager *)_timerManager)->switchThread();
				else
					usleep(1000);

				/* Actual delay provided will be lower than requested: elapsed time is calculated cumulatively.
				i.e. the higher the requested delay, the higher the actual delay reduction */
				elapsed_time += getMillis() - start_time;
			}
		} else {
			while (elapsed_time < msecs) {
				/* if remaining delay is lower than last amount of time spent on main thread, burn it in emu thread
				to avoid exceeding requested delay */
				if (msecs - elapsed_time >= ((LibretroTimerManager *)_timerManager)->spentOnMainThread() && !((LibretroTimerManager *)_timerManager)->timeToNextSwitch())
					((LibretroTimerManager *)_timerManager)->switchThread();
				else
					usleep(1000);
				elapsed_time = getMillis() - start_time;
			}
		}

		((LibretroTimerManager *)_timerManager)->handler();
	}

	virtual Common::MutexInternal *createMutex(void) {
		return createLibretroMutexInternal();
	}

	virtual void quit() {
		// TODO:
	}

	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) {
		// TODO: NOTHING?
	}

	virtual void getTimeAndDate(TimeDate &t, bool skipRecord) const {
		time_t curTime = time(NULL);

#define YEAR0 1900
#define EPOCH_YR 1970
#define SECS_DAY (24L * 60L * 60L)
#define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year) (LEAPYEAR(year) ? 366 : 365)
		const int _ytab[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
		int year = EPOCH_YR;
		unsigned long dayclock = (unsigned long)curTime % SECS_DAY;
		unsigned long dayno = (unsigned long)curTime / SECS_DAY;
		t.tm_sec = dayclock % 60;
		t.tm_min = (dayclock % 3600) / 60;
		t.tm_hour = dayclock / 3600;
		t.tm_wday = (dayno + 4) % 7; /* day 0 was a thursday */
		while (dayno >= YEARSIZE(year)) {
			dayno -= YEARSIZE(year);
			year++;
		}
		t.tm_year = year - YEAR0;
		t.tm_mon = 0;
		while (dayno >= _ytab[LEAPYEAR(year)][t.tm_mon]) {
			dayno -= _ytab[LEAPYEAR(year)][t.tm_mon];
			t.tm_mon++;
		}
		t.tm_mday = dayno + 1;
	}

	virtual Audio::Mixer *getMixer() {
		return _mixer;
	}

	virtual Common::String getDefaultConfigFileName() {
		return s_systemDir + "/scummvm.ini";
	}

	virtual void logMessage(LogMessageType::Type type, const char *message) {
		retro_log_level loglevel = RETRO_LOG_INFO;
		switch (type) {
			case LogMessageType::kDebug:
				loglevel = RETRO_LOG_DEBUG;
				break;
			case LogMessageType::kWarning:
				loglevel = RETRO_LOG_WARN;
				break;
			case LogMessageType::kError:
				loglevel = RETRO_LOG_ERROR;
				break;
		}

		if (log_cb)
			log_cb(loglevel, "%s\n", message);
	}

	const Graphics::Surface &getScreen() {
		const Graphics::Surface &srcSurface = (_overlayInGUI) ? _overlay : _gameScreen;

		if (srcSurface.w != _screen.w || srcSurface.h != _screen.h) {
#ifdef FRONTEND_SUPPORTS_RGB565
			_screen.create(srcSurface.w, srcSurface.h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
#else
			_screen.create(srcSurface.w, srcSurface.h, Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15));
#endif
		}

		return _screen;
	}

#define ANALOG_RANGE 0x8000
#define BASE_CURSOR_SPEED 4
#define PI 3.141592653589793238

	void updateMouseXY(float deltaAcc, float * cumulativeXYAcc, int doing_x){
		int * mouseXY;
		int16 * screen_wh;
		int * relMouseXY;
		int cumulativeXYAcc_int;
		if (doing_x) {
			mouseXY = &_mouseX;
			screen_wh = &_screen.w;
			relMouseXY = &_relMouseX;
		} else {
			mouseXY = &_mouseY;
			screen_wh = &_screen.h;
			relMouseXY = &_relMouseY;
		}
		*cumulativeXYAcc += deltaAcc;
		cumulativeXYAcc_int = (int)*cumulativeXYAcc;
		if (cumulativeXYAcc_int != 0) {
			// Set mouse position
			*mouseXY += cumulativeXYAcc_int;
			*mouseXY = (*mouseXY < 0) ? 0 : *mouseXY;
			*mouseXY = (*mouseXY >= *screen_wh) ? *screen_wh : *mouseXY;
			// Update accumulator
			*cumulativeXYAcc -= (float)cumulativeXYAcc_int;
		}
		*relMouseXY = (int)deltaAcc;
	}

	void processMouse(retro_input_state_t aCallback, int device, float gampad_cursor_speed, float gamepad_acceleration_time, bool analog_response_is_quadratic, int analog_deadzone, float mouse_speed) {
		enum processMouse_status {
			STATUS_DOING_JOYSTICK  = (1 << 0),
			STATUS_DOING_MOUSE     = (1 << 1),
			STATUS_DOING_X         = (1 << 2),
			STATUS_DOING_Y         = (1 << 3)
		};
		uint8_t status = 0;
		int16_t joy_x, joy_y, joy_rx, joy_ry, x, y;
		float analog_amplitude_x, analog_amplitude_y;
		float deltaAcc;
		bool  down;
		float screen_adjusted_cursor_speed = (float)_screen.w / 320.0f; // Dpad cursor speed should always be based off a 320 wide screen, to keep speeds consistent
		float adjusted_cursor_speed = (float)BASE_CURSOR_SPEED * gampad_cursor_speed * screen_adjusted_cursor_speed;
		float inverse_acceleration_time = (gamepad_acceleration_time > 0.0) ? (1.0 / 60.0) * (1.0 / gamepad_acceleration_time) : 1.0;
		int dpad_cursor_offset;
		double rs_radius, rs_angle;
		unsigned numpad_index;

		static const uint32_t retroButtons[2] = {RETRO_DEVICE_ID_MOUSE_LEFT, RETRO_DEVICE_ID_MOUSE_RIGHT};
		static const Common::EventType eventID[2][2] = {{Common::EVENT_LBUTTONDOWN, Common::EVENT_LBUTTONUP}, {Common::EVENT_RBUTTONDOWN, Common::EVENT_RBUTTONUP}};

		static const unsigned gampad_key_map[8][4] = {
			{RETRO_DEVICE_ID_JOYPAD_X, (unsigned)Common::KEYCODE_ESCAPE, (unsigned)Common::ASCII_ESCAPE, 0},           // Esc
			{RETRO_DEVICE_ID_JOYPAD_Y, (unsigned)Common::KEYCODE_PERIOD, 46, 0},                                       // .
			{RETRO_DEVICE_ID_JOYPAD_L, (unsigned)Common::KEYCODE_RETURN, (unsigned)Common::ASCII_RETURN, 0},           // Enter
			{RETRO_DEVICE_ID_JOYPAD_R, (unsigned)Common::KEYCODE_KP5, 53, 0},                                          // Numpad 5
			{RETRO_DEVICE_ID_JOYPAD_L2, (unsigned)Common::KEYCODE_BACKSPACE, (unsigned)Common::ASCII_BACKSPACE, 0},    // Backspace
			{RETRO_DEVICE_ID_JOYPAD_L3, (unsigned)Common::KEYCODE_F10, (unsigned)Common::ASCII_F10, 0},                // F10
			{RETRO_DEVICE_ID_JOYPAD_R3, (unsigned)Common::KEYCODE_KP0, 48, 0},                                         // Numpad 0
			{RETRO_DEVICE_ID_JOYPAD_SELECT, (unsigned)Common::KEYCODE_F7, (unsigned)Common::ASCII_F7, RETROKMOD_CTRL}, // CTRL+F7 (virtual keyboard)
		};

		// Right stick circular wrap around: 1 -> 2 -> 3 -> 6 -> 9 -> 8 -> 7 -> 4
		static const unsigned gampad_numpad_map[8][2] = {
			{(unsigned)Common::KEYCODE_KP1, 49},
			{(unsigned)Common::KEYCODE_KP2, 50},
			{(unsigned)Common::KEYCODE_KP3, 51},
			{(unsigned)Common::KEYCODE_KP6, 54},
			{(unsigned)Common::KEYCODE_KP9, 57},
			{(unsigned)Common::KEYCODE_KP8, 56},
			{(unsigned)Common::KEYCODE_KP7, 55},
			{(unsigned)Common::KEYCODE_KP4, 52},
		};

		// Reduce gamepad cursor speed, if required
		if (device == RETRO_DEVICE_JOYPAD && aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2)) {
			adjusted_cursor_speed = adjusted_cursor_speed * (1.0f / 5.0f);
		}

		status = 0;
		x = aCallback(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
		y = aCallback(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
		joy_x = aCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X);
		joy_y = aCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y);

		// Left Analog X Axis
		if (joy_x > analog_deadzone || joy_x < -analog_deadzone) {
			status |= (STATUS_DOING_JOYSTICK | STATUS_DOING_X);
			if (joy_x > analog_deadzone) {
				// Reset accumulator when changing direction
				_mouseXAcc = (_mouseXAcc < 0.0) ? 0.0 : _mouseXAcc;
				joy_x = joy_x - analog_deadzone;
			}
			if (joy_x < -analog_deadzone) {
				// Reset accumulator when changing direction
				_mouseXAcc = (_mouseXAcc > 0.0) ? 0.0 : _mouseXAcc;
				joy_x = joy_x + analog_deadzone;
			}
			// Update accumulator
			analog_amplitude_x = (float)joy_x / (float)(ANALOG_RANGE - analog_deadzone);
			if (analog_response_is_quadratic) {
				if (analog_amplitude_x < 0.0)
					analog_amplitude_x = -(analog_amplitude_x * analog_amplitude_x);
				else
					analog_amplitude_x = analog_amplitude_x * analog_amplitude_x;
			}
			// printf("analog_amplitude_x: %f\n", analog_amplitude_x);
			deltaAcc = analog_amplitude_x * adjusted_cursor_speed;
			updateMouseXY(deltaAcc, &_mouseXAcc, 1);
		}

		// Left Analog Y Axis
		if (joy_y > analog_deadzone || joy_y < -analog_deadzone) {
			status |= (STATUS_DOING_JOYSTICK | STATUS_DOING_Y);
			if (joy_y > analog_deadzone) {
				// Reset accumulator when changing direction
				_mouseYAcc = (_mouseYAcc < 0.0) ? 0.0 : _mouseYAcc;
				joy_y = joy_y - analog_deadzone;
			}
			if (joy_y < -analog_deadzone) {
				// Reset accumulator when changing direction
				_mouseYAcc = (_mouseYAcc > 0.0) ? 0.0 : _mouseYAcc;
				joy_y = joy_y + analog_deadzone;
			}
			// Update accumulator
			analog_amplitude_y = (float)joy_y / (float)(ANALOG_RANGE - analog_deadzone);
			if (analog_response_is_quadratic) {
				if (analog_amplitude_y < 0.0)
					analog_amplitude_y = -(analog_amplitude_y * analog_amplitude_y);
				else
					analog_amplitude_y = analog_amplitude_y * analog_amplitude_y;
			}
			// printf("analog_amplitude_y: %f\n", analog_amplitude_y);
			deltaAcc = analog_amplitude_y * adjusted_cursor_speed;
			updateMouseXY(deltaAcc, &_mouseYAcc, 0);
		}

		if (device == RETRO_DEVICE_JOYPAD) {
			bool dpadLeft = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
			bool dpadRight = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
			bool dpadUp = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
			bool dpadDown = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);

			if (dpadLeft || dpadRight) {
				status |= (STATUS_DOING_JOYSTICK | STATUS_DOING_X);
				_dpadXVel = MIN(_dpadXVel + inverse_acceleration_time, 1.0f);

				if (dpadLeft) {
					deltaAcc = -(_dpadXVel * adjusted_cursor_speed);
					_dpadXAcc = _dpadXAcc < deltaAcc ? _dpadXAcc : 0.0f;
				} else { //dpadRight
					deltaAcc = _dpadXVel * adjusted_cursor_speed;
					_dpadXAcc = _dpadXAcc > deltaAcc ? _dpadXAcc : 0.0f;
				}

				updateMouseXY(deltaAcc, &_dpadXAcc, 1);
			} else {
				_dpadXVel = 0.0f;
			}


			if (dpadUp || dpadDown) {
				status |= (STATUS_DOING_JOYSTICK | STATUS_DOING_Y);
				_dpadYVel = MIN(_dpadYVel + inverse_acceleration_time, 1.0f);

				if (dpadUp) {
					deltaAcc = -(_dpadYVel * adjusted_cursor_speed);
					_dpadYAcc = _dpadYAcc < deltaAcc ? _dpadYAcc : 0.0f;
				} else { //dpadDown
					deltaAcc = _dpadYVel * adjusted_cursor_speed;
					_dpadYAcc = _dpadYAcc > deltaAcc ? _dpadYAcc : 0.0f;
				}

				updateMouseXY(deltaAcc, &_dpadYAcc, 0);


			} else {
				_dpadYVel = 0.0f;
			}

			if (aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START)) {
				Common::Event ev;
				ev.type = Common::EVENT_MAINMENU;
				_events.push_back(ev);
			}
		}

#if defined(WIIU) || defined(__SWITCH__)
		int p_x = aCallback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
		int p_y = aCallback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);
		int p_press = aCallback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED);
		int px = (int)((p_x + 0x7fff) * _screen.w / 0xffff);
		int py = (int)((p_y + 0x7fff) * _screen.h / 0xffff);
		// printf("(%d,%d) p:%d\n",px,py,pp);

		static int ptrhold = 0;

		if (p_press)
			ptrhold++;
		else
			ptrhold = 0;

		if (ptrhold > 0) {
			_mouseX = px;
			_mouseY = py;

			Common::Event ev;
			ev.type = Common::EVENT_MOUSEMOVE;
			ev.mouse.x = _mouseX;
			ev.mouse.y = _mouseY;
			_events.push_back(ev);
		}

		if (ptrhold > 10 && _ptrmouseButton == 0) {
			_ptrmouseButton = 1;
			Common::Event ev;
			ev.type = eventID[0][_ptrmouseButton ? 0 : 1];
			ev.mouse.x = _mouseX;
			ev.mouse.y = _mouseY;
			_events.push_back(ev);
		} else if (ptrhold == 0 && _ptrmouseButton == 1) {
			_ptrmouseButton = 0;
			Common::Event ev;
			ev.type = eventID[0][_ptrmouseButton ? 0 : 1];
			ev.mouse.x = _mouseX;
			ev.mouse.y = _mouseY;
			_events.push_back(ev);
		}

#endif

		if (status & STATUS_DOING_JOYSTICK) {
			Common::Event ev;
			ev.type = Common::EVENT_MOUSEMOVE;
			ev.mouse.x = _mouseX;
			ev.mouse.y = _mouseY;
			ev.relMouse.x = status & STATUS_DOING_X ? _relMouseX : 0;
			ev.relMouse.y = status & STATUS_DOING_Y ? _relMouseY : 0;
			_events.push_back(ev);
		}

		// Gampad mouse buttons
		down = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
		if (down != _joypadmouseButtons[0]) {
			_joypadmouseButtons[0] = down;

			Common::Event ev;
			ev.type = eventID[0][down ? 0 : 1];
			ev.mouse.x = _mouseX;
			ev.mouse.y = _mouseY;
			_events.push_back(ev);
		}

		down = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
		if (down != _joypadmouseButtons[1]) {
			_joypadmouseButtons[1] = down;

			Common::Event ev;
			ev.type = eventID[1][down ? 0 : 1];
			ev.mouse.x = _mouseX;
			ev.mouse.y = _mouseY;
			_events.push_back(ev);
		}

		// Gamepad keyboard buttons
		for (int i = 0; i < 8; i++) {
			down = aCallback(0, RETRO_DEVICE_JOYPAD, 0, gampad_key_map[i][0]);
			if (down != _joypadkeyboardButtons[i]) {
				_joypadkeyboardButtons[i] = down;
				bool state = down ? true : false;
				processKeyEvent(state, gampad_key_map[i][1], (uint32_t)gampad_key_map[i][2], (uint32_t)gampad_key_map[i][3]);
			}
		}

		// Gamepad right stick numpad emulation
		joy_rx = aCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X);
		joy_ry = aCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y);

		if (joy_rx > analog_deadzone)
			joy_rx = joy_rx - analog_deadzone;
		else if (joy_rx < -analog_deadzone)
			joy_rx = joy_rx + analog_deadzone;
		else
			joy_rx = 0;

		if (joy_ry > analog_deadzone)
			joy_ry = joy_ry - analog_deadzone;
		else if (joy_ry < -analog_deadzone)
			joy_ry = joy_ry + analog_deadzone;
		else
			joy_ry = 0;

		// This is very ugly, but I don't have time to make it nicer...
		if (joy_rx != 0 || joy_ry != 0) {
			analog_amplitude_x = (float)joy_rx / (float)(ANALOG_RANGE - analog_deadzone);
			analog_amplitude_y = (float)joy_ry / (float)(ANALOG_RANGE - analog_deadzone);

			// Convert to polar coordinates: part 1
			rs_radius = sqrt((double)(analog_amplitude_x * analog_amplitude_x) + (double)(analog_amplitude_y * analog_amplitude_y));

			// Check if radius is above threshold
			if (rs_radius > 0.5) {
				// Convert to polar coordinates: part 2
				rs_angle = atan2((double)analog_amplitude_y, (double)analog_amplitude_x);

				// Adjust rotation offset...
				rs_angle = (2.0 * PI) - (rs_angle + PI);
				rs_angle = fmod(rs_angle - (0.125 * PI), 2.0 * PI);
				if (rs_angle < 0)
					rs_angle += 2.0 * PI;

				// Convert angle into numpad key index
				numpad_index = (unsigned)((rs_angle / (2.0 * PI)) * 8.0);
				// Unnecessary safety check...
				numpad_index = (numpad_index > 7) ? 7 : numpad_index;
				// printf("numpad_index: %u\n", numpad_index);

				if (numpad_index != _joypadnumpadLast) {
					// Unset last key, if required
					if (_joypadnumpadActive)
						processKeyEvent(false, gampad_numpad_map[_joypadnumpadLast][0], (uint32_t)gampad_numpad_map[_joypadnumpadLast][1], 0);

					// Set new key
					processKeyEvent(true, gampad_numpad_map[numpad_index][0], (uint32_t)gampad_numpad_map[numpad_index][1], 0);

					_joypadnumpadLast = numpad_index;
					_joypadnumpadActive = true;
				}
			} else if (_joypadnumpadActive) {
				processKeyEvent(false, gampad_numpad_map[_joypadnumpadLast][0], (uint32_t)gampad_numpad_map[_joypadnumpadLast][1], 0);
				_joypadnumpadActive = false;
				_joypadnumpadLast = 8;
			}
		} else if (_joypadnumpadActive) {
			processKeyEvent(false, gampad_numpad_map[_joypadnumpadLast][0], (uint32_t)gampad_numpad_map[_joypadnumpadLast][1], 0);
			_joypadnumpadActive = false;
			_joypadnumpadLast = 8;
		}

		// Process input from physical mouse
		// > X Axis
		if (x != 0) {
			status |= (STATUS_DOING_MOUSE | STATUS_DOING_X);
			if (x > 0) {
				// Reset accumulator when changing direction
				_mouseXAcc = (_mouseXAcc < 0.0) ? 0.0 : _mouseXAcc;
			}
			if (x < 0) {
				// Reset accumulator when changing direction
				_mouseXAcc = (_mouseXAcc > 0.0) ? 0.0 : _mouseXAcc;
			}
			deltaAcc = (float)x * mouse_speed;
			updateMouseXY(deltaAcc, &_mouseXAcc, 1);
		}
		// > Y Axis
		if (y != 0) {
			status |= (STATUS_DOING_MOUSE | STATUS_DOING_Y);
			if (y > 0) {
				// Reset accumulator when changing direction
				_mouseYAcc = (_mouseYAcc < 0.0) ? 0.0 : _mouseYAcc;
			}
			if (y < 0) {
				// Reset accumulator when changing direction
				_mouseYAcc = (_mouseYAcc > 0.0) ? 0.0 : _mouseYAcc;
			}
			deltaAcc = (float)y * mouse_speed;
			updateMouseXY(deltaAcc, &_mouseYAcc, 0);
		}

		if (status & STATUS_DOING_MOUSE) {
			Common::Event ev;
			ev.type = Common::EVENT_MOUSEMOVE;
			ev.mouse.x = _mouseX;
			ev.mouse.y = _mouseY;
			ev.relMouse.x = status & STATUS_DOING_X ? _relMouseX : 0;
			ev.relMouse.y = status & STATUS_DOING_Y ? _relMouseY : 0;
			_events.push_back(ev);
		}

		for (int i = 0; i < 2; i++) {
			Common::Event ev;
			bool down = aCallback(0, RETRO_DEVICE_MOUSE, 0, retroButtons[i]);
			if (down != _mouseButtons[i]) {
				_mouseButtons[i] = down;

				ev.type = eventID[i][down ? 0 : 1];
				ev.mouse.x = _mouseX;
				ev.mouse.y = _mouseY;
				_events.push_back(ev);
			}
		}
	}

	void processKeyEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers) {
		int _keyflags = 0;
		_keyflags |= (key_modifiers & RETROKMOD_CTRL) ? Common::KBD_CTRL : 0;
		_keyflags |= (key_modifiers & RETROKMOD_ALT) ? Common::KBD_ALT : 0;
		_keyflags |= (key_modifiers & RETROKMOD_SHIFT) ? Common::KBD_SHIFT : 0;
		_keyflags |= (key_modifiers & RETROKMOD_META) ? Common::KBD_META : 0;
		_keyflags |= (key_modifiers & RETROKMOD_CAPSLOCK) ? Common::KBD_CAPS : 0;
		_keyflags |= (key_modifiers & RETROKMOD_NUMLOCK) ? Common::KBD_NUM : 0;
		_keyflags |= (key_modifiers & RETROKMOD_SCROLLOCK) ? Common::KBD_SCRL : 0;

		Common::Event ev;
		ev.type = down ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
		ev.kbd.keycode = (Common::KeyCode)keycode;
		ev.kbd.flags = _keyflags;
		ev.kbd.ascii = keycode;

		/* If shift was down then send upper case letter to engine */
		if (ev.kbd.ascii >= 97 && ev.kbd.ascii <= 122 && (_keyflags & Common::KBD_SHIFT))
			ev.kbd.ascii = ev.kbd.ascii & ~0x20;

		_events.push_back(ev);
	}

	bool parseGameName(const Common::String &gameName, Common::String &engineId,
	                   Common::String &gameId) {
		Common::StringTokenizer tokenizer(gameName, ":");
		Common::String token1, token2;

		if (!tokenizer.empty()) {
			token1 = tokenizer.nextToken();
		}

		if (!tokenizer.empty()) {
			token2 = tokenizer.nextToken();
		}

		if (!tokenizer.empty()) {
			return false; // Stray colon
		}

		if (!token1.empty() && !token2.empty()) {
			engineId = token1;
			gameId = token2;
			return true;
		} else if (!token1.empty()) {
			engineId.clear();
			gameId = token1;
			return true;
		}

		return false;
	}

	int TestGame(const char *filedata, bool autodetect) {
		Common::String game_id;
		Common::String engine_id;
		Common::String data = filedata;
		int res = TEST_GAME_KO_NOT_FOUND;

		PluginManager::instance().init();
		PluginManager::instance().loadAllPlugins();
		PluginManager::instance().loadDetectionPlugin();

		if (autodetect) {
			Common::FSNode dir(data);
			Common::FSList files;
			dir.getChildren(files, Common::FSNode::kListAll);

			DetectionResults detectionResults = EngineMan.detectGames(files);
			if (!detectionResults.listRecognizedGames().empty()) {
				res = TEST_GAME_OK_ID_AUTODETECTED;
			}

		} else {

			ConfMan.loadDefaultConfigFile(getDefaultConfigFileName().c_str());
			if (ConfMan.hasGameDomain(data)) {
				res = TEST_GAME_OK_TARGET_FOUND;
			} else {
				parseGameName(data, engine_id, game_id);

				QualifiedGameList games = EngineMan.findGamesMatching(engine_id, game_id);
				if (games.size() == 1) {
					res = TEST_GAME_OK_ID_FOUND;
				} else if (games.size() > 1) {
					res = TEST_GAME_KO_MULTIPLE_RESULTS;
				}
			}
		}

		PluginManager::instance().unloadDetectionPlugin();
		PluginManager::instance().unloadAllPlugins();
		PluginManager::destroy();
		return res;
	}

	void Quit() {
		Common::Event ev;
		ev.type = Common::EVENT_QUIT;
		dynamic_cast<OSystem_RETRO *>(g_system)->getEventManager()->pushEvent(ev);
	}

	void Reset() {
		dynamic_cast<OSystem_RETRO *>(g_system)->getEventManager()->resetQuit();
	}

	void destroy() {
		delete this;
	}

};

OSystem *retroBuildOS() {
	return new OSystem_RETRO();
}

const Graphics::Surface &getScreen() {
	return dynamic_cast<OSystem_RETRO *>(g_system)->getScreen();
}

void retroProcessMouse(retro_input_state_t aCallback, int device, float gamepad_cursor_speed, float gamepad_acceleration_time, bool analog_response_is_quadratic, int analog_deadzone, float mouse_speed) {
	dynamic_cast<OSystem_RETRO *>(g_system)->processMouse(aCallback, device, gamepad_cursor_speed, gamepad_acceleration_time, analog_response_is_quadratic, analog_deadzone, mouse_speed);
}

void retroQuit() {
	dynamic_cast<OSystem_RETRO *>(g_system)->Quit();
}

int retroTestGame(const char *game_id, bool autodetect) {
	return dynamic_cast<OSystem_RETRO *>(g_system)->TestGame(game_id, autodetect);
}

void retroSetSystemDir(const char *aPath) {
	s_systemDir = Common::String(aPath ? aPath : ".");
}

void retroSetSaveDir(const char *aPath) {
	s_saveDir = Common::String(aPath ? aPath : ".");
}

void retroKeyEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers) {
	dynamic_cast<OSystem_RETRO *>(g_system)->processKeyEvent(down, keycode, character, key_modifiers);
}

void retroReset() {
	dynamic_cast<OSystem_RETRO *>(g_system)->Reset();
}

uint8 getThreadSwitchCaller(){
	return dynamic_cast<OSystem_RETRO *>(g_system)->getThreadSwitchCaller();
}

void retroDestroy() {
	dynamic_cast<OSystem_RETRO *>(g_system)->destroy();
}
