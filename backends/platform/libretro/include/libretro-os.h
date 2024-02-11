/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
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
#ifndef BACKENDS_LIBRETRO_OS_H
#define BACKENDS_LIBRETRO_OS_H

#include "audio/mixer_intern.h"
#include "base/main.h"
#include "backends/base-backend.h"
#include "common/system.h"
#include "common/mutex.h"
#include "common/list.h"
#include "common/events.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#define BASE_CURSOR_SPEED 4
#define CURSOR_STATUS_DOING_JOYSTICK  (1 << 0)
#define CURSOR_STATUS_DOING_MOUSE     (1 << 1)
#define CURSOR_STATUS_DOING_X         (1 << 2)
#define CURSOR_STATUS_DOING_Y         (1 << 3)
#define CURSOR_STATUS_DOING_SLOWER    (1 << 4)

#define LIBRETRO_G_SYSTEM dynamic_cast<OSystem_libretro *>(g_system)

/**
 *  Dummy mutex implementation
 */
class LibretroMutexInternal final : public Common::MutexInternal {
public:
	LibretroMutexInternal() {};
	~LibretroMutexInternal() override {};
	bool lock() override { return 0; }
	bool unlock() override { return 0; };
};

class LibretroPalette {
public:
	const byte *_prevColorsSource;
	unsigned char _colors[256 * 3];
	LibretroPalette(void);
	~LibretroPalette(void) {};
	void set(const byte *colors, uint start, uint num);
	void get(byte *colors, uint start, uint num) const;
	unsigned char *getColor(uint aIndex) const;
	void reset(void) { _prevColorsSource = NULL; }
};

class OSystem_libretro : public EventsBaseBackend, public PaletteManager {
private:
	int _mouseX;
	int _mouseY;
	int _relMouseX;
	int _relMouseY;
	int _mouseHotspotX;
	int _mouseHotspotY;
	int _mouseKeyColor;
	float _mouseXAcc;
	float _mouseYAcc;
	float _dpadXAcc;
	float _dpadYAcc;
	float _dpadXVel;
	float _dpadYVel;
	float _adjusted_cursor_speed;
	float _inverse_acceleration_time;
	uint32 _startTime;
	uint8 _threadSwitchCaller;
	uint8 _cursorStatus;
	Common::String s_systemDir;
	Common::String s_saveDir;
	Common::String s_playlistDir;
	Common::List<Common::Event> _events;

public:
	Audio::MixerImpl *_mixer;
	Graphics::Surface _screen;
	Graphics::Surface _gameScreen;
	Graphics::Surface _overlay;
	Graphics::Surface _mouseImage;
	LibretroPalette _mousePalette;
	LibretroPalette _gamePalette;
	bool _overlayVisible;
	bool _overlayInGUI;
	bool _mouseDontScale;
	bool _mouseButtons[2];
	bool _ptrmouseButton;
	bool _mousePaletteEnabled;
	bool _mouseVisible;

	/* Base */
	OSystem_libretro(void);
	~OSystem_libretro(void) override;
	void initBackend(void) override;
	void engineInit(void) override;
	void engineDone(void) override;
	bool hasFeature(Feature f) override;
	void setFeatureState(Feature f, bool enable) override;
	bool getFeatureState(Feature f) override;
	void refreshRetroSettings(void);
	void destroy(void);
	void quit() override {}
private:
	bool checkPathSetting(const char *setting, Common::String const &defaultPath, bool isDirectory = true);
	void setLibretroDir(const char * path, Common::String &var);

	/* Graphics */
public:
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
	const GraphicsMode *getSupportedGraphicsModes(void) const override;
	void initSize(uint width, uint height, const Graphics::PixelFormat *format) override;
	int16 getHeight(void) override;
	int16 getWidth(void) override;
	Graphics::PixelFormat getScreenFormat(void) const override;
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	void updateScreen(void) override;
	void showOverlay(bool inGUI) override;
	void hideOverlay(void) override;
	void clearOverlay(void) override;
	void grabOverlay(Graphics::Surface &surface) override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayHeight(void) override;
	int16 getOverlayWidth(void) override;
	Graphics::PixelFormat getOverlayFormat() const override;
	const Graphics::Surface &getScreen(void);
	bool showMouse(bool visible) override;
	void warpMouse(int x, int y) override;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor = 255, bool dontScale = false, const Graphics::PixelFormat *format = NULL, const byte *mask = nullptr) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override;
	int getDefaultGraphicsMode() const override { return 0; }
	bool isOverlayVisible() const override { return false; }
	bool setGraphicsMode(int mode, uint flags = kGfxModeNoFlags) override { return true; }
	int getGraphicsMode() const override { return 0; }
	PaletteManager *getPaletteManager() override { return this; }
	Graphics::Surface *lockScreen() override { return &_gameScreen; }
	void unlockScreen() override {}
	GUI::OptionsContainerWidget *buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
	void applyBackendSettings() override;
protected:
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;

	/* Events */
public:
	bool pollEvent(Common::Event &event) override;
	uint8 getThreadSwitchCaller(void);
	uint32 getMillis(bool skipRecord = false) override;
	void delayMillis(uint msecs) override;
	Common::MutexInternal *createMutex(void) override;
	void requestQuit(void);
	void resetQuit(void);

	/* Utils */
	void getTimeAndDate(TimeDate &t, bool skipRecord) const override;
	Audio::Mixer *getMixer(void) override;
	Common::Path getDefaultConfigFileName(void) override;
	void logMessage(LogMessageType::Type type, const char *message) override;
	int testGame(const char *filedata, bool autodetect);
	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;
	const char * const *buildHelpDialogData() override;
	Common::String getSaveDir(void);
private:
	bool parseGameName(const Common::String &gameName, Common::String &engineId, Common::String &gameId);

	/* Inputs */
public:
	void processInputs(void);
	void processKeyEvent(bool down, unsigned keycode, uint32 character, uint16 key_modifiers);
	void setShakePos(int shakeXOffset, int shakeYOffset) override {}
private:
	void updateMouseXY(float deltaAcc, float * cumulativeXYAcc, int doing_x);
	void getMouseXYFromAnalog(bool is_x, int16 coor);
	void getMouseXYFromButton(bool is_x, int16 sign);
};

#endif
