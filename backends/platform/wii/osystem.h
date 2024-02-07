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
#include "graphics/paletteman.h"
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
	bool _overlayInGUI;

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
	void updateMousePalette();

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

	void initBackend() override;
	void engineInit() override;
	void engineDone() override;

	bool hasFeature(Feature f) override;
	void setFeatureState(Feature f, bool enable) override;
	bool getFeatureState(Feature f) override;
	const GraphicsMode *getSupportedGraphicsModes() const override;
	int getDefaultGraphicsMode() const override;
	bool setGraphicsMode(int mode, uint flags = kGfxModeNoFlags) override;
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override;
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif
	int getGraphicsMode() const override;
	virtual void initSize(uint width, uint height,
							const Graphics::PixelFormat *format) override;
	int16 getWidth() override;
	int16 getHeight() override;

	PaletteManager *getPaletteManager() override { return this; }
protected:
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;
public:
	void setCursorPalette(const byte *colors, uint start, uint num) override;
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y,
									int w, int h) override;
	void updateScreen() override;
	Graphics::Surface *lockScreen() override;
	void unlockScreen() override;
	void setShakePos(int shakeXOffset, int shakeYOffset) override;

	void showOverlay(bool inGUI) override;
	void hideOverlay() override;
	bool isOverlayVisible() const override { return _overlayVisible; }
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) override;
	virtual void copyRectToOverlay(const void *buf, int pitch,
									int x, int y, int w, int h) override;
	int16 getOverlayWidth() override;
	int16 getOverlayHeight() override;
	Graphics::PixelFormat getOverlayFormat() const override;

	bool showMouse(bool visible) override;

	void warpMouse(int x, int y) override;
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX,
								int hotspotY, uint32 keycolor,
								bool dontScale,
								const Graphics::PixelFormat *format, const byte *mask) override;

	bool pollEvent(Common::Event &event) override;
	uint32 getMillis(bool skipRecord = false) override;
	void delayMillis(uint msecs) override;

	Common::MutexInternal *createMutex() override;

	typedef void (*SoundProc)(void *param, byte *buf, int len);

	void quit() override;

	Audio::Mixer *getMixer() override;
	FilesystemFactory *getFilesystemFactory() override;
	void getTimeAndDate(TimeDate &td, bool skipRecord = false) const override;

	void logMessage(LogMessageType::Type type, const char *message) override;

#ifndef GAMECUBE
	Common::String getSystemLanguage() const override;
#endif // GAMECUBE
};

#endif
