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

#ifndef BACKENDS_PLATFORM_IOS7_IOS7_OSYS_MAIN_H
#define BACKENDS_PLATFORM_IOS7_IOS7_OSYS_MAIN_H

#include "graphics/surface.h"
#include "backends/platform/ios7/ios7_common.h"
#include "backends/modular-backend.h"
#include "common/events.h"
#include "common/str.h"
#include "common/ustr.h"
#include "audio/mixer_intern.h"
#include "backends/fs/posix/posix-fs-factory.h"
#include "graphics/palette.h"

#include <AudioToolbox/AudioQueue.h>

#define AUDIO_BUFFERS 3
#define WAVE_BUFFER_SIZE 2048
#define AUDIO_SAMPLE_RATE 44100

#define SCUMMVM_ROOT_PATH "/var/mobile/Library/ScummVM"
#define SCUMMVM_SAVE_PATH SCUMMVM_ROOT_PATH "/Savegames"
#define SCUMMVM_PREFS_PATH SCUMMVM_ROOT_PATH "/Preferences"

typedef void (*SoundProc)(void *param, byte *buf, int len);
typedef int (*TimerProc)(int interval);

struct AQCallbackStruct {
	AudioQueueRef queue;
	uint32 frameCount;
	AudioQueueBufferRef buffers[AUDIO_BUFFERS];
	AudioStreamBasicDescription dataFormat;
};

class OSystem_iOS7 : public EventsBaseBackend, public PaletteManager {
protected:
	static AQCallbackStruct s_AudioQueue;
	static SoundProc s_soundCallback;
	static void *s_soundParam;

	Audio::MixerImpl *_mixer;

	VideoContext *_videoContext;

	Graphics::Surface _framebuffer;

	// For signaling that screen format set up might have failed.
	TransactionError _gfxTransactionError;

	// For use with the game texture
	uint16  _gamePalette[256];
	// For use with the mouse texture
	uint16  _gamePaletteRGBA5551[256];

	CFTimeInterval _startTime;
	uint32 _timeSuspended;

	bool _mouseCursorPaletteEnabled;
	uint16 _mouseCursorPalette[256];
	Graphics::Surface _mouseBuffer;
	uint16 _mouseKeyColor;
	bool _mouseDirty;
	bool _mouseNeedTextureUpdate;

	long _lastMouseDown;
	long _lastMouseTap;
	long _queuedEventTime;
	Common::Event _queuedInputEvent;
	bool _secondaryTapped;
	long _lastSecondaryDown;
	long _lastSecondaryTap;
	int _gestureStartX, _gestureStartY;
	bool _mouseClickAndDragEnabled;
	bool _touchpadModeEnabled;
	int _lastPadX;
	int _lastPadY;
	int _lastDragPosX;
	int _lastDragPosY;

	int _timerCallbackNext;
	int _timerCallbackTimer;
	TimerProc _timerCallback;

	Common::Array<Common::Rect> _dirtyRects;
	Common::Array<Common::Rect> _dirtyOverlayRects;
	ScreenOrientation _screenOrientation;
	bool _fullScreenIsDirty;
	bool _fullScreenOverlayIsDirty;
	int _screenChangeCount;

	Common::String _lastErrorMessage;

#ifdef IPHONE_SANDBOXED
	Common::String _chrootBasePath;
#endif

public:

	OSystem_iOS7();
	virtual ~OSystem_iOS7();

	static OSystem_iOS7 *sharedInstance();

	void initBackend() override;

	void engineInit() override;
	void engineDone() override;

	bool hasFeature(Feature f) override;
	void setFeatureState(Feature f, bool enable) override;
	bool getFeatureState(Feature f) override;
	void initSize(uint width, uint height, const Graphics::PixelFormat *format) override;

	void beginGFXTransaction() override;
	TransactionError endGFXTransaction() override;

	int16 getHeight() override;
	int16 getWidth() override;

	bool touchpadModeEnabled() const;

#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override { return _framebuffer.format; }
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif

	PaletteManager *getPaletteManager() override { return this; }

	float getHiDPIScreenFactor() const override;

protected:
	// PaletteManager API
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;

public:
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	void updateScreen() override;
	Graphics::Surface *lockScreen() override;
	void unlockScreen() override;
	void setShakePos(int shakeXOffset, int shakeYOffset) override;

	void showOverlay() override;
	void hideOverlay() override;
	bool isOverlayVisible() const override { return _videoContext->overlayVisible; }
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayHeight() override;
	int16 getOverlayWidth() override;
	Graphics::PixelFormat getOverlayFormat() const override;

	bool showMouse(bool visible) override;

	void warpMouse(int x, int y) override;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor = 255, bool dontScale = false, const Graphics::PixelFormat *format = NULL) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override;

	bool pollEvent(Common::Event &event) override;
	uint32 getMillis(bool skipRecord = false) override;
	void delayMillis(uint msecs) override;
	Common::MutexInternal *createMutex() override;

	static void mixCallback(void *sys, byte *samples, int len);
	virtual void setupMixer(void);
	virtual void setTimerCallback(TimerProc callback, int interval);
	int getScreenChangeID() const override { return _screenChangeCount; }
	void quit() override;

	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;
	void getTimeAndDate(TimeDate &td, bool skipRecord = false) const override;

	Audio::Mixer *getMixer() override;

	void startSoundsystem();
	void stopSoundsystem();

	Common::String getDefaultConfigFileName() override;

	void logMessage(LogMessageType::Type type, const char *message) override;
	void fatalError() override;

	bool hasTextInClipboard() override;
	Common::U32String getTextFromClipboard() override;
	bool setTextInClipboard(const Common::U32String &text) override;

	bool openUrl(const Common::String &url) override;

	Common::String getSystemLanguage() const override;

	bool isConnectionLimited() override;

protected:
	void initVideoContext();
	void updateOutputSurface();
	void setShowKeyboard(bool);
	bool isKeyboardShown() const;

	void internUpdateScreen();
	void dirtyFullScreen();
	void dirtyFullOverlayScreen();
	void suspendLoop();
	void saveState();
	void restoreState();
	void clearState();
	void drawDirtyRect(const Common::Rect &dirtyRect);
	void updateMouseTexture();
	static void AQBufferCallback(void *in, AudioQueueRef inQ, AudioQueueBufferRef outQB);
	static int timerHandler(int t);

	bool handleEvent_swipe(Common::Event &event, int direction, int touches);
	bool handleEvent_tap(Common::Event &event, UIViewTapDescription type, int touches);
	void handleEvent_keyPressed(Common::Event &event, int keyPressed);
	void handleEvent_orientationChanged(int orientation);
	void handleEvent_applicationSuspended();
	void handleEvent_applicationResumed();
	void handleEvent_applicationSaveState();
	void handleEvent_applicationRestoreState();
	void handleEvent_applicationClearState();

	bool handleEvent_mouseDown(Common::Event &event, int x, int y);
	bool handleEvent_mouseUp(Common::Event &event, int x, int y);

	bool handleEvent_secondMouseDown(Common::Event &event, int x, int y);
	bool handleEvent_secondMouseUp(Common::Event &event, int x, int y);

	bool handleEvent_mouseDragged(Common::Event &event, int x, int y);
	bool handleEvent_mouseSecondDragged(Common::Event &event, int x, int y);

	void rebuildSurface();
};

#endif
