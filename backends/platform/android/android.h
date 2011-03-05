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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef _ANDROID_H_
#define _ANDROID_H_

#if defined(__ANDROID__)

#include "common/fs.h"
#include "common/archive.h"
#include "audio/mixer_intern.h"
#include "graphics/surface.h"
#include "backends/base-backend.h"
#include "backends/plugins/posix/posix-provider.h"
#include "backends/fs/posix/posix-fs-factory.h"

#include "backends/platform/android/texture.h"

#include <pthread.h>

#include <android/log.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

// toggles start
//#define ANDROID_DEBUG_ENTER
//#define ANDROID_DEBUG_GL
// toggles end

extern const char *android_log_tag;

#define _ANDROID_LOG(prio, fmt, args...) __android_log_print(prio, android_log_tag, fmt, ## args)
#define LOGD(fmt, args...) _ANDROID_LOG(ANDROID_LOG_DEBUG, fmt, ##args)
#define LOGI(fmt, args...) _ANDROID_LOG(ANDROID_LOG_INFO, fmt, ##args)
#define LOGW(fmt, args...) _ANDROID_LOG(ANDROID_LOG_WARN, fmt, ##args)
#define LOGE(fmt, args...) _ANDROID_LOG(ANDROID_LOG_ERROR, fmt, ##args)

#ifdef ANDROID_DEBUG_ENTER
#define ENTER(fmt, args...) LOGD("%s(" fmt ")", __FUNCTION__, ##args)
#else
#define ENTER(fmt, args...) do {  } while (false)
#endif

#ifdef ANDROID_DEBUG_GL
extern void checkGlError(const char *expr, const char *file, int line);

#define GLCALL(x) \
	do { \
		(x); \
		checkGlError(#x, __FILE__, __LINE__); \
	} while (false)

#define GLTHREADCHECK \
	do { \
		assert(pthread_self() == _main_thread); \
	} while (false)

#else
#define GLCALL(x) do { (x); } while (false)
#define GLTHREADCHECK do {  } while (false)
#endif

#ifdef DYNAMIC_MODULES
class AndroidPluginProvider : public POSIXPluginProvider {
protected:
	virtual void addCustomDirectories(Common::FSList &dirs) const;
};
#endif

class OSystem_Android : public BaseBackend, public PaletteManager {
private:
	// passed from the dark side
	int _audio_sample_rate;
	int _audio_buffer_size;

	int _screen_changeid;
	int _egl_surface_width;
	int _egl_surface_height;

	bool _force_redraw;

	// Game layer
	GLESTexture *_game_texture;
	int _shake_offset;
	Common::Rect _focus_rect;

	// Overlay layer
	GLES4444Texture *_overlay_texture;
	bool _show_overlay;

	// Mouse layer
	GLESTexture *_mouse_texture;
	GLESPaletteTexture *_mouse_texture_palette;
	GLESTexture *_mouse_texture_rgb;
	Common::Point _mouse_hotspot;
	int _mouse_targetscale;
	bool _show_mouse;
	bool _use_mouse_palette;

	Common::Queue<Common::Event> _event_queue;
	MutexRef _event_queue_lock;

	pthread_t _main_thread;

	bool _timer_thread_exit;
	pthread_t _timer_thread;
	static void *timerThreadFunc(void *arg);

	bool _audio_thread_exit;
	pthread_t _audio_thread;
	static void *audioThreadFunc(void *arg);

	bool _enable_zoning;
	bool _virtkeybd_on;

	Common::SaveFileManager *_savefile;
	Audio::MixerImpl *_mixer;
	Common::TimerManager *_timer;
	FilesystemFactory *_fsFactory;
	timeval _startTime;

	void initSurface();
	void deinitSurface();
	void initViewport();

#ifdef USE_RGB_COLOR
	Common::String getPixelFormatName(const Graphics::PixelFormat &format) const;
	void initTexture(GLESTexture **texture, uint width, uint height,
						const Graphics::PixelFormat *format, bool alphaPalette);
#endif

	void setupKeymapper();
	void setCursorPaletteInternal(const byte *colors, uint start, uint num);

public:
	OSystem_Android(int audio_sample_rate, int audio_buffer_size);
	virtual ~OSystem_Android();

	virtual void initBackend();
	void addPluginDirectories(Common::FSList &dirs) const;
	void enableZoning(bool enable) { _enable_zoning = enable; }

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);

	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	bool setGraphicsMode(const char *name);
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;

#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif

	virtual void initSize(uint width, uint height,
							const Graphics::PixelFormat *format);
	virtual int getScreenChangeID() const;

	virtual int16 getHeight();
	virtual int16 getWidth();

	virtual PaletteManager *getPaletteManager() {
		return this;
	}

protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);

public:
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y,
									int w, int h);
	virtual void updateScreen();
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void setShakePos(int shakeOffset);
	virtual void fillScreen(uint32 col);
	virtual void setFocusRectangle(const Common::Rect& rect);
	virtual void clearFocusRectangle();

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch,
									int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();

	// RGBA 4444
	virtual Graphics::PixelFormat getOverlayFormat() const {
		Graphics::PixelFormat format;

		format.bytesPerPixel = 2;
		format.rLoss = 8 - 4;
		format.gLoss = 8 - 4;
		format.bLoss = 8 - 4;
		format.aLoss = 8 - 4;
		format.rShift = 3 * 4;
		format.gShift = 2 * 4;
		format.bShift = 1 * 4;
		format.aShift = 0 * 4;

		return format;
	}

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX,
								int hotspotY, uint32 keycolor,
								int cursorTargetScale,
								const Graphics::PixelFormat *format);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);
	virtual void disableCursorPalette(bool disable);

	virtual bool pollEvent(Common::Event &event);
	void pushEvent(const Common::Event& event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual void quit();

	virtual void setWindowCaption(const char *caption);
	virtual void displayMessageOnOSD(const char *msg);
	virtual void showVirtualKeyboard(bool enable);

	virtual Common::SaveFileManager *getSavefileManager();
	virtual Audio::Mixer *getMixer();
	virtual void getTimeAndDate(TimeDate &t) const;
	virtual Common::TimerManager *getTimerManager();
	virtual FilesystemFactory *getFilesystemFactory();
	virtual void logMessage(LogMessageType::Type type, const char *message);
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s,
											int priority = 0);
};

#endif
#endif

