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

#ifndef _WII_OSYSTEM_H_
#define _WII_OSYSTEM_H_

#include <gctypes.h>
#include <gccore.h>

#include <gxflux/gfx.h>

#include "base/main.h"
#include "common/fs.h"
#include "common/rect.h"
#include "common/events.h"
#include "backends/base-backend.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "audio/mixer_intern.h"

#ifdef __cplusplus
extern "C" {
#endif

extern bool reset_btn_pressed;
extern bool power_btn_pressed;

#ifdef DEBUG_WII_MEMSTATS
extern void wii_memstats(void);
#endif

#ifdef __cplusplus
}
#endif

class OSystem_Wii final : public EventsBaseBackend, public PaletteManager {
private:
	s64 _startup_time;

	bool _cursorDontScale;
	bool _cursorPaletteDisabled;
	u16 *_cursorPalette;
	bool _cursorPaletteDirty;

	bool _gameRunning;
	u16 _gameWidth, _gameHeight;
	u8 *_gamePixels;
	Graphics::Surface _surface;
	gfx_screen_coords_t _coordsGame;
	gfx_tex_t _texGame;
	bool _gameDirty;

	bool _overlayVisible;
	u16 _overlayWidth, _overlayHeight;
	u32 _overlaySize;
	uint16 *_overlayPixels;
	gfx_screen_coords_t _coordsOverlay;
	gfx_tex_t _texOverlay;
	bool _overlayDirty;

	u32 _lastScreenUpdate;
	u16 _currentWidth, _currentHeight;
	f32 _currentXScale, _currentYScale;

	s32 _configGraphicsMode;
	s32 _actualGraphicsMode;
	bool _bilinearFilter;
	const Graphics::PixelFormat _pfRGB565;
	const Graphics::PixelFormat _pfRGB3444;
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat _pfGame;
	Graphics::PixelFormat _pfGameTexture;
	Graphics::PixelFormat _pfCursor;
#endif

	bool _consoleVisible;
	bool _optionsDlgActive;
	bool _fullscreen;
	bool _arCorrection;

	bool _mouseVisible;
	s32 _mouseX, _mouseY;
	s32 _mouseHotspotX, _mouseHotspotY;
	u16 _mouseKeyColor;
	gfx_tex_t _texMouse;

	bool _kbd_active;

	bool _event_quit;

	u32 _lastPadCheck;
	u8 _padSensitivity;
	u8 _padAcceleration;

	void initGfx();
	void deinitGfx();
	void updateScreenResolution();
	void switchVideoMode(int mode);
	bool needsScreenUpdate();

	void initSfx();
	void deinitSfx();

	void initEvents();
	void deinitEvents();
	void updateEventScreenResolution();
	bool pollKeyboard(Common::Event &event);

	void showOptionsDialog();

protected:
	Audio::MixerImpl *_mixer;

public:
	enum {
		gmStandard = 0,
		gmStandardFiltered,
		gmDoubleStrike,
		gmDoubleStrikeFiltered
	};

	OSystem_Wii();
	virtual ~OSystem_Wii();

	virtual void initBackend() override;
	virtual void engineInit() override;
	virtual void engineDone() override;

	virtual bool hasFeature(Feature f) override;
	virtual void setFeatureState(Feature f, bool enable) override;
	virtual bool getFeatureState(Feature f) override;
	virtual const GraphicsMode *getSupportedGraphicsModes() const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool setGraphicsMode(int mode, uint flags = kGfxModeNoFlags) override;
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const override;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif
	virtual int getGraphicsMode() const override;
	virtual void initSize(uint width, uint height,
							const Graphics::PixelFormat *format) override;
	virtual int16 getWidth() override;
	virtual int16 getHeight() override;

	virtual PaletteManager *getPaletteManager() override { return this; }
protected:
	virtual void setPalette(const byte *colors, uint start, uint num) override;
	virtual void grabPalette(byte *colors, uint start, uint num) const override;
public:
	virtual void setCursorPalette(const byte *colors, uint start, uint num) override;
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y,
									int w, int h) override;
	virtual void updateScreen() override;
	virtual Graphics::Surface *lockScreen() override;
	virtual void unlockScreen() override;
	virtual void setShakePos(int shakeXOffset, int shakeYOffset) override;

	virtual void showOverlay() override;
	virtual void hideOverlay() override;
	virtual bool isOverlayVisible() const override { return _overlayVisible; }
	virtual void clearOverlay() override;
	virtual void grabOverlay(void *buf, int pitch) override;
	virtual void copyRectToOverlay(const void *buf, int pitch,
									int x, int y, int w, int h) override;
	virtual int16 getOverlayWidth() override;
	virtual int16 getOverlayHeight() override;
	virtual Graphics::PixelFormat getOverlayFormat() const override;

	virtual bool showMouse(bool visible) override;

	virtual void warpMouse(int x, int y) override;
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX,
								int hotspotY, uint32 keycolor,
								bool dontScale,
								const Graphics::PixelFormat *format) override;

	virtual bool pollEvent(Common::Event &event) override;
	virtual uint32 getMillis(bool skipRecord = false) override;
	virtual void delayMillis(uint msecs) override;

	virtual MutexRef createMutex() override;
	virtual void lockMutex(MutexRef mutex) override;
	virtual void unlockMutex(MutexRef mutex) override;
	virtual void deleteMutex(MutexRef mutex) override;

	typedef void (*SoundProc)(void *param, byte *buf, int len);

	virtual void quit() override;

	virtual Audio::Mixer *getMixer() override;
	virtual FilesystemFactory *getFilesystemFactory() override;
	virtual void getTimeAndDate(TimeDate &t) const override;

	virtual void logMessage(LogMessageType::Type type, const char *message) override;

#ifndef GAMECUBE
	virtual Common::String getSystemLanguage() const override;
#endif // GAMECUBE
};

#endif
