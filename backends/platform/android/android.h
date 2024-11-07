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

#ifndef _ANDROID_H_
#define _ANDROID_H_

#include "backends/platform/android/portdefs.h"
#include "common/fs.h"
#include "common/archive.h"
#include "common/mutex.h"
#include "common/ustr.h"
#include "audio/mixer_intern.h"
#include "backends/modular-backend.h"
#include "backends/plugins/posix/posix-provider.h"
#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/log/log.h"
#include "backends/platform/android/touchcontrols.h"

#include <pthread.h>

#include <android/log.h>

// toggles start
//#define ANDROID_DEBUG_ENTER
//#define ANDROID_DEBUG_GL
//#define ANDROID_DEBUG_GL_CALLS
// toggles end

extern const char *android_log_tag;

#define _ANDROID_LOG(prio, fmt, args...) __android_log_print(prio, android_log_tag, fmt, ## args)
#define LOGD(fmt, args...) _ANDROID_LOG(ANDROID_LOG_DEBUG, fmt, ##args)
#define LOGI(fmt, args...) _ANDROID_LOG(ANDROID_LOG_INFO, fmt, ##args)
#define LOGW(fmt, args...) _ANDROID_LOG(ANDROID_LOG_WARN, fmt, ##args)
#define LOGE(fmt, args...) _ANDROID_LOG(ANDROID_LOG_ERROR, fmt, ##args)

#define MAX_ANDROID_SCUMMVM_LOG_FILESIZE_IN_BYTES (100*1024)

#ifdef ANDROID_DEBUG_ENTER
#define ENTER(fmt, args...) LOGD("%s(" fmt ")", __FUNCTION__, ##args)
#else
#define ENTER(fmt, args...) do {  } while (false)
#endif

#ifdef ANDROID_DEBUG_GL
extern void checkGlError(const char *expr, const char *file, int line);

#ifdef ANDROID_DEBUG_GL_CALLS
#define GLCALLLOG(x, before) \
	do { \
		if (before) \
			LOGD("calling '%s' (%s:%d)", x, __FILE__, __LINE__); \
		else \
			LOGD("returned from '%s' (%s:%d)", x, __FILE__, __LINE__); \
	} while (false)
#else
#define GLCALLLOG(x, before) do {  } while (false)
#endif

#define GLCALL(x) \
	do { \
		GLCALLLOG(#x, true); \
		(x); \
		GLCALLLOG(#x, false); \
		checkGlError(#x, __FILE__, __LINE__); \
	} while (false)

#define GLTHREADCHECK \
	do { \
		assert(dynamic_cast<OSystem_Android *>(g_system)->isRunningInMainThread()); \
	} while (false)

#else
#define GLCALL(x) do { (x); } while (false)
#define GLTHREADCHECK do {  } while (false)
#endif

void *androidGLgetProcAddress(const char *name);

class OSystem_Android : public ModularGraphicsBackend, Common::EventSource {
private:
	static const int kQueuedInputEventDelay = 50;

	struct EventWithDelay : public Common::Event {
		/** An original timestamp that identifies this event and the delayed ones connected to it that will follow */
		uint32 originTimeMillis;

		/** The time which the delay starts counting from. It can be set to be later than originTimeMillis */
		uint32 referTimeMillis;

		/** The delay for the event to be handled */
		uint32 delayMillis;

		/** The connected EventType of the "connected" event that should be handled before this one */
		Common::EventType connectedType;

		/** A status flag indicating whether the "connected" event was handled */
		bool connectedTypeExecuted;

		EventWithDelay() : originTimeMillis(0), referTimeMillis(0), delayMillis(0), connectedType(Common::EVENT_INVALID), connectedTypeExecuted(false) {
		}

		void reset() {
			originTimeMillis = 0;
			referTimeMillis = 0;
			delayMillis = 0;
			connectedType = Common::EVENT_INVALID;
			connectedTypeExecuted = false;
		}
	};

	// passed from the dark side
	int _audio_sample_rate;
	int _audio_buffer_size;

	int _screen_changeid;

	pthread_t _main_thread;

	bool _timer_thread_exit;
	pthread_t _timer_thread;

	bool _audio_thread_exit;
	pthread_t _audio_thread;

	bool _virtkeybd_on;

	Audio::MixerImpl *_mixer;
	timeval _startTime;

	Common::Queue<Common::Event> _event_queue;
	EventWithDelay _delayedMouseBtnUpEvent;
	EventWithDelay _delayedMouseBtnDownEvent;
	Common::Mutex *_event_queue_lock;

	Common::Point _touch_pt_down, _touch_pt_scroll, _touch_pt_dt, _touch_pt_multi;
	int _eventScaleX;
	int _eventScaleY;
	int _touch_mode;
	int _touchpad_scale;  // Used in events.cpp
	int _trackball_scale; // Used in events.cpp
	int _dpad_scale;      // Used in events.cpp
	int _joystick_scale;  // TODO This seems currently unused. Is it needed?
//	int _fingersDown;
	int _firstPointerId;
	int _secondPointerId;
	int _thirdPointerId;

	TouchControls _touchControls;

	bool _engineRunning;

	Common::Path _defaultConfigFileName;
	Common::Path _defaultLogFileName;
	Common::String _systemPropertiesSummaryStr;
	Common::String _systemSDKdetectedStr;

	Backends::Log::Log *_logger;

#if defined(USE_OPENGL) && defined(USE_GLAD)
	// Cached dlopen object
	mutable void *_gles2DL;
#endif

	static void *timerThreadFunc(void *arg);
	static void *audioThreadFunc(void *arg);
	Common::String getSystemProperty(const char *name) const;

	Common::WriteStream *createLogFileForAppending();

public:
	enum {
		TOUCH_MODE_TOUCHPAD = 0,
		TOUCH_MODE_MOUSE = 1,
		TOUCH_MODE_GAMEPAD = 2,
		TOUCH_MODE_MAX = 3
	};

	enum {
		SCREEN_ORIENTATION_UNSPECIFIED = 0xffffffff,
		SCREEN_ORIENTATION_LANDSCAPE = 0,
		SCREEN_ORIENTATION_PORTRAIT = 1
	};

	enum {
		SHOW_ON_SCREEN_NONE = 0,
		SHOW_ON_SCREEN_MENU = 1,
		SHOW_ON_SCREEN_INPUT_MODE = 2,
		SHOW_ON_SCREEN_ALL = 0xffffffff,
	};

	OSystem_Android(int audio_sample_rate, int audio_buffer_size);
	virtual ~OSystem_Android();

	void initBackend() override;
	void engineInit() override;
	void engineDone() override;

	bool hasFeature(OSystem::Feature f) override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) override;

	void pushEvent(int type, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
	void pushEvent(const Common::Event &event);
	void pushEvent(const Common::Event &event1, const Common::Event &event2);
	void pushDelayedTouchMouseBtnEvents();

	TouchControls &getTouchControls() { return _touchControls; }
	void applyTouchSettings(bool _3dMode, bool overlayShown);
	void setupTouchMode(int oldValue, int newValue);

	void applyOrientationSettings();

	void updateOnScreenControls();

	bool pollEvent(Common::Event &event) override;
	Common::HardwareInputSet *getHardwareInputSet() override;
	Common::KeymapArray getGlobalKeymaps() override;
	Common::KeymapperDefaultBindings *getKeymapperDefaultBindings() override;

	Common::Path getDefaultConfigFileName() override;
	Common::Path getDefaultLogFileName() override;

	void registerDefaultSettings(const Common::String &target) const override;
	GUI::OptionsContainerWidget *buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
	void applyBackendSettings() override;

	uint32 getMillis(bool skipRecord = false) override;
	void delayMillis(uint msecs) override;
	Common::MutexInternal *createMutex() override;

	void quit() override;

	void setWindowCaption(const Common::U32String &caption) override;

	Audio::Mixer *getMixer() override;
	void getTimeAndDate(TimeDate &td, bool skipRecord = false) const override;
	void logMessage(LogMessageType::Type type, const char *message) override;
	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;
	bool openUrl(const Common::String &url) override;
	bool hasTextInClipboard() override;
	Common::U32String getTextFromClipboard() override;
	bool setTextInClipboard(const Common::U32String &text) override;
	bool isConnectionLimited() override;
	Common::String getSystemLanguage() const override;

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	int getDefaultGraphicsMode() const override;
	bool setGraphicsMode(int mode, uint flags) override;
	int getGraphicsMode() const override;

	OpenGL::ContextType getOpenGLType() const override { return OpenGL::kContextGLES2; }
#if defined(USE_OPENGL) && defined(USE_GLAD)
	void *getOpenGLProcAddress(const char *name) const override;
#endif

#ifdef ANDROID_DEBUG_GL
	bool isRunningInMainThread() { return pthread_self() == _main_thread; }
#endif

	virtual const char * const *buildHelpDialogData() override;
};

#endif
