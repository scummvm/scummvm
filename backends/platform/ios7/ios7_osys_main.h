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
#include "backends/keymapper/hardware-input.h"
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
#define MAX_IOS7_SCUMMVM_LOG_FILESIZE_IN_BYTES (100*1024)

typedef void (*SoundProc)(void *param, byte *buf, int len);
typedef int (*TimerProc)(int interval);

struct AQCallbackStruct {
	AudioQueueRef queue;
	uint32 frameCount;
	AudioQueueBufferRef buffers[AUDIO_BUFFERS];
	AudioStreamBasicDescription dataFormat;
};

class OSystem_iOS7 : public ModularGraphicsBackend, public EventsBaseBackend {
protected:
	static AQCallbackStruct s_AudioQueue;
	static SoundProc s_soundCallback;
	static void *s_soundParam;

	Audio::MixerImpl *_mixer;

	CFTimeInterval _startTime;
	uint32 _timeSuspended;

	long _lastMouseDown;
	long _lastMouseTap;
	long _queuedEventTime;
	Common::Event _queuedInputEvent;
	bool _secondaryTapped;
	long _lastSecondaryDown;
	long _lastSecondaryTap;
	bool _mouseClickAndDragEnabled;
	bool _touchpadModeEnabled;
	int _lastPadX;
	int _lastPadY;

	ScreenOrientation _screenOrientation;

	Common::String _lastErrorMessage;

	Common::String _chrootBasePath;

public:

	OSystem_iOS7();
	virtual ~OSystem_iOS7();

	static OSystem_iOS7 *sharedInstance();

	void initBackend() override;

	void engineInit() override;
	void engineDone() override;

	void updateStartSettings(const Common::String &executable, Common::String &command, Common::StringMap &settings, Common::StringArray& additionalArgs) override;

	bool hasFeature(Feature f) override;
	void setFeatureState(Feature f, bool enable) override;
	bool getFeatureState(Feature f) override;

	bool touchpadModeEnabled() const;

	uint createOpenGLContext();
	void destroyOpenGLContext();
	void refreshScreen() const;
	int getScreenWidth() const;
	int getScreenHeight() const;
	float getSystemHiDPIScreenFactor() const;

#if defined(USE_OPENGL) && defined(USE_GLAD)
	void *getOpenGLProcAddress(const char *name) const override;
#endif

public:
	bool pollEvent(Common::Event &event) override;
	uint32 getMillis(bool skipRecord = false) override;
	void delayMillis(uint msecs) override;
	Common::MutexInternal *createMutex() override;

	static void mixCallback(void *sys, byte *samples, int len);
	virtual void setupMixer(void);
	virtual void setTimerCallback(TimerProc callback, int interval);
	void quit() override;

	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;
	void getTimeAndDate(TimeDate &td, bool skipRecord = false) const override;

	Common::HardwareInputSet *getHardwareInputSet() override;

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
	void virtualController(bool connect);

	virtual Common::String getDefaultLogFileName() override { return Common::String("/scummvm.log"); }

	virtual GUI::OptionsContainerWidget* buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
	virtual void applyBackendSettings() override;
	virtual void registerDefaultSettings(const Common::String &target) const override;

protected:
	void updateOutputSurface();
	void setShowKeyboard(bool);
	bool isKeyboardShown() const;

	void suspendLoop();
	void saveState();
	void restoreState();
	void clearState();
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

	bool handleEvent_touchFirstDown(Common::Event &event, int x, int y);
	bool handleEvent_touchFirstUp(Common::Event &event, int x, int y);

	bool handleEvent_touchSecondDown(Common::Event &event, int x, int y);
	bool handleEvent_touchSecondUp(Common::Event &event, int x, int y);

	bool handleEvent_touchFirstDragged(Common::Event &event, int x, int y);
	bool handleEvent_touchSecondDragged(Common::Event &event, int x, int y);

	void handleEvent_mouseLeftButtonDown(Common::Event &event, int x, int y);
	void handleEvent_mouseLeftButtonUp(Common::Event &event, int x, int y);
	void handleEvent_mouseRightButtonDown(Common::Event &event, int x, int y);
	void handleEvent_mouseRightButtonUp(Common::Event &event, int x, int y);
	void handleEvent_mouseDelta(Common::Event &event, int deltaX, int deltaY);
	void handleEvent_mouseEvent(Common::Event &event, int relX, int relY);

	void rebuildSurface();
	float getMouseSpeed();
};

#endif
