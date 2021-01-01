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

#if defined(__ANDROID__)

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv(a)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/system_properties.h>
#include <time.h>
#include <unistd.h>

#include "common/util.h"
#include "common/textconsole.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/mutex.h"
#include "common/events.h"
#include "common/config-manager.h"

#include "backends/audiocd/default/default-audiocd.h"
#include "backends/events/default/default-events.h"
#include "backends/mutex/pthread/pthread-mutex.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"

#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/keymapper-defaults.h"
#include "backends/keymapper/standard-actions.h"

#include "backends/platform/android3d/jni-android.h"
#include "backends/platform/android3d/android.h"
#include "backends/platform/android3d/graphics.h"

const char *android_log_tag = "ResidualVM";

// This replaces the bionic libc assert functions with something that
// actually prints the assertion failure before aborting.
extern "C" {
	void __assert(const char *file, int line, const char *expr) {
		__android_log_assert(expr, android_log_tag,
								"Assertion failure: '%s' in %s:%d",
								 expr, file, line);
	}

	void __assert2(const char *file, int line, const char *func,
					const char *expr) {
		__android_log_assert(expr, android_log_tag,
								"Assertion failure: '%s' in %s:%d (%s)",
								 expr, file, line, func);
	}
}

#ifdef ANDROID_DEBUG_GL
static const char *getGlErrStr(GLenum error) {
	switch (error) {
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	}

	static char buf[40];
	snprintf(buf, sizeof(buf), "(Unknown GL error code 0x%x)", error);

	return buf;
}

void checkGlError(const char *expr, const char *file, int line) {
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		LOGE("GL ERROR: %s on %s (%s:%d)", getGlErrStr(error), expr, file, line);
}
#endif

OSystem_Android::OSystem_Android(int audio_sample_rate, int audio_buffer_size) :
	_audio_sample_rate(audio_sample_rate),
	_audio_buffer_size(audio_buffer_size),
	_screen_changeid(0),
	_mixer(0),
	_queuedEventTime(0),
	_event_queue_lock(0),
	_touch_pt_down(),
	_touch_pt_scroll(),
	_touch_pt_dt(),
	_eventScaleX(100),
	_eventScaleY(100),
	// TODO put these values in some option dlg?
	_touchpad_mode(true),
	_touchpad_scale(66),
	_dpad_scale(4),
	_fingersDown(0),
	_trackball_scale(2) {

	_fsFactory = new POSIXFilesystemFactory();

	LOGI("Running on: [%s] [%s] [%s] [%s] [%s] SDK:%s ABI:%s",
			getSystemProperty("ro.product.manufacturer").c_str(),
			getSystemProperty("ro.product.model").c_str(),
			getSystemProperty("ro.product.brand").c_str(),
			getSystemProperty("ro.build.fingerprint").c_str(),
			getSystemProperty("ro.build.display.id").c_str(),
			getSystemProperty("ro.build.version.sdk").c_str(),
			getSystemProperty("ro.product.cpu.abi").c_str());
}

OSystem_Android::~OSystem_Android() {
	ENTER();
	// _audiocdManager should be deleted before _mixer!
	// It is normally deleted in proper order in the OSystem destructor.
	// However, currently _mixer is deleted here (OSystem_Android)
	// and in the ModularBackend destructor,
	// hence unless _audiocdManager is deleted here first,
	// it will cause a crash for the Android app (arm64 v8a) upon exit
	// -- when the audio cd manager was actually used eg. audio cd test of the testbed
	// FIXME: A more proper fix would probably be to:
	//        - delete _mixer in the base class (OSystem) after _audiocdManager (this is already the current behavior)
	//	      - remove its deletion from OSystem_Android and ModularBackend (this is what needs to be fixed).
	delete _audiocdManager;
	_audiocdManager = 0;
	delete _mixer;
	_mixer = 0;
	delete _fsFactory;
	_fsFactory = 0;
	delete _timerManager;
	_timerManager = 0;

	delete _event_queue_lock;

	delete _savefileManager;
	_savefileManager = 0;
}

void *OSystem_Android::timerThreadFunc(void *arg) {
	OSystem_Android *system = (OSystem_Android *)arg;
	DefaultTimerManager *timer = (DefaultTimerManager *)(system->_timerManager);

	// renice this thread to boost the audio thread
	if (setpriority(PRIO_PROCESS, 0, 19) < 0)
		LOGW("couldn't renice the timer thread");

	JNI::attachThread();

	struct timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = 10 * 1000 * 1000; // 10ms

	while (!system->_timer_thread_exit) {
		if (JNI::pause) {
			LOGD("timer thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("timer thread woke up");
		}

		timer->handler();
		nanosleep(&tv, 0);
	}

	JNI::detachThread();

	return 0;
}

void *OSystem_Android::audioThreadFunc(void *arg) {
	JNI::attachThread();

	OSystem_Android *system = (OSystem_Android *)arg;
	Audio::MixerImpl *mixer = system->_mixer;

	uint buf_size = system->_audio_buffer_size;

	JNIEnv *env = JNI::getEnv();

	jbyteArray bufa = env->NewByteArray(buf_size);

	bool paused = true;

	int offset, left, written, i;

	struct timespec tv_delay;
	tv_delay.tv_sec = 0;
	tv_delay.tv_nsec = 20 * 1000 * 1000;

	uint msecs_full = buf_size * 1000 / (mixer->getOutputRate() * 2 * 2);

	struct timespec tv_full;
	tv_full.tv_sec = 0;
	tv_full.tv_nsec = msecs_full * 1000 * 1000;

	uint silence_count = 33;

	while (!system->_audio_thread_exit) {
		if (JNI::pause) {
			JNI::setAudioStop();

			paused = true;
			silence_count = 33;

			LOGD("audio thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("audio thread woke up");
		}

		byte *buf = (byte *)env->GetPrimitiveArrayCritical(bufa, 0);
		assert(buf);

		int samples = mixer->mixCallback(buf, buf_size);

		bool silence = samples < 1;

		// looks stupid, and it is, but currently there's no way to detect
		// silence-only buffers from the mixer
		if (!silence) {
			silence = true;

			for (i = 0; i < samples; i += 2)
				// SID streams constant crap
				if (READ_UINT16(buf + i) > 32) {
					silence = false;
					break;
				}
		}

		env->ReleasePrimitiveArrayCritical(bufa, buf, 0);

		if (silence) {
			if (!paused)
				silence_count++;

			// only pause after a while to prevent toggle mania
			if (silence_count > 32) {
				if (!paused) {
					LOGD("AudioTrack pause");

					JNI::setAudioPause();
					paused = true;
				}

				nanosleep(&tv_full, 0);

				continue;
			}
		}

		if (paused) {
			LOGD("AudioTrack play");

			JNI::setAudioPlay();
			paused = false;

			silence_count = 0;
		}

		offset = 0;
		left = buf_size;
		written = 0;

		while (left > 0) {
			written = JNI::writeAudio(env, bufa, offset, left);

			if (written < 0) {
				LOGE("AudioTrack error: %d", written);
				break;
			}

			// buffer full
			if (written < left)
				nanosleep(&tv_delay, 0);

			offset += written;
			left -= written;
		}

		if (written < 0)
			break;

		// prepare the next buffer, and run into the blocking AudioTrack.write
	}

	JNI::setAudioStop();

	env->DeleteLocalRef(bufa);

	JNI::detachThread();

	return 0;
}

void OSystem_Android::initBackend() {
	ENTER();

	_main_thread = pthread_self();

	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);
	ConfMan.registerDefault("touchpad_mouse_mode", true);

	ConfMan.registerDefault("autosave_period", 0);
	ConfMan.setBool("FM_high_quality", false);
	ConfMan.setBool("FM_medium_quality", true);
	ConfMan.set("browser_lastpath", ConfMan.get("path"));

	if (ConfMan.hasKey("touchpad_mouse_mode"))
		_touchpad_mode = ConfMan.getBool("touchpad_mouse_mode");
	else
		ConfMan.setBool("touchpad_mouse_mode", true);

	// must happen before creating TimerManager to avoid race in
	// creating EventManager
	setupKeymapper();

	// BUG: "transient" ConfMan settings get nuked by the options
	// screen. Passing the savepath in this way makes it stick
	// (via ConfMan.registerDefault)
	_savefileManager = new DefaultSaveFileManager(ConfMan.get("savepath"));
	// TODO remove the debug message eventually
	LOGD("Setting DefaultSaveFileManager path to: %s", ConfMan.get("savepath").c_str());

	_mutexManager = new PthreadMutexManager();
	_timerManager = new DefaultTimerManager();

	_event_queue_lock = new Common::Mutex();

	gettimeofday(&_startTime, 0);

	_mixer = new Audio::MixerImpl(_audio_sample_rate);
	_mixer->setReady(true);

	_timer_thread_exit = false;
	pthread_create(&_timer_thread, 0, timerThreadFunc, this);

	_audio_thread_exit = false;
	pthread_create(&_audio_thread, 0, audioThreadFunc, this);

	_graphicsManager = new AndroidGraphicsManager();

	dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->initSurface();

	// renice this thread to boost the audio thread
	if (setpriority(PRIO_PROCESS, 0, 19) < 0)
		warning("couldn't renice the main thread");

	JNI::setReadyForEvents(true);

	_eventManager = new DefaultEventManager(this);
	_audiocdManager = new DefaultAudioCDManager();

	BaseBackend::initBackend();
}

bool OSystem_Android::hasFeature(Feature f) {
	if (f == kFeatureVirtualKeyboard ||
			f == kFeatureOpenUrl ||
			f == kFeatureTouchpadMode ||
			f == kFeatureOnScreenControl ||
			f == kFeatureClipboardSupport ||
			f == kFeatureOpenGLForGame ||
			f == kFeatureOverlaySupportsAlpha ||
			f == kFeatureClipboardSupport) {
		return true;
	}
	return ModularGraphicsBackend::hasFeature(f);
}

void OSystem_Android::setFeatureState(Feature f, bool enable) {
	ENTER("%d, %d", f, enable);

	switch (f) {
	case kFeatureVirtualKeyboard:
		_virtkeybd_on = enable;
		showVirtualKeyboard(enable);
		break;
	case kFeatureTouchpadMode:
		ConfMan.setBool("touchpad_mouse_mode", enable);
		_touchpad_mode = enable;
		break;
	case kFeatureOnScreenControl:
		ConfMan.setBool("onscreen_control", enable);
		JNI::showKeyboardControl(enable);
		break;
	default:
		ModularGraphicsBackend::setFeatureState(f, enable);
		break;
	}
}

bool OSystem_Android::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureVirtualKeyboard:
		return _virtkeybd_on;
	case kFeatureTouchpadMode:
		return ConfMan.getBool("touchpad_mouse_mode");
	case kFeatureOnScreenControl:
		return ConfMan.getBool("onscreen_control");
	default:
		return ModularGraphicsBackend::getFeatureState(f);
	}
}

Common::KeymapperDefaultBindings *OSystem_Android::getKeymapperDefaultBindings() {
	Common::KeymapperDefaultBindings *keymapperDefaultBindings = new Common::KeymapperDefaultBindings();

	// The swap_menu_and_back is a legacy configuration key
	// It is only checked here for compatibility with old config files
	// where it may have been set as "true"
	// TODO Why not just ignore it entirely anyway?
	if (ConfMan.hasKey("swap_menu_and_back")  && ConfMan.getBool("swap_menu_and_back")) {
		keymapperDefaultBindings->setDefaultBinding(Common::kGlobalKeymapName, "MENU", "AC_BACK");
		keymapperDefaultBindings->setDefaultBinding("engine-default", Common::kStandardActionSkip, "MENU");
		keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, "CLOS", "MENU");
	} else {
		keymapperDefaultBindings->setDefaultBinding(Common::kGlobalKeymapName, "MENU", "MENU");
		keymapperDefaultBindings->setDefaultBinding("engine-default", Common::kStandardActionSkip, "AC_BACK");
		keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, "CLOS", "AC_BACK");
	}

	return keymapperDefaultBindings;
}

uint32 OSystem_Android::getMillis(bool skipRecord) {
	timeval curTime;

	gettimeofday(&curTime, 0);

	return (uint32)(((curTime.tv_sec - _startTime.tv_sec) * 1000) +
			((curTime.tv_usec - _startTime.tv_usec) / 1000));
}

void OSystem_Android::delayMillis(uint msecs) {
	usleep(msecs * 1000);
}

void OSystem_Android::quit() {
	ENTER();

	JNI::setReadyForEvents(false);

	_audio_thread_exit = true;
	pthread_join(_audio_thread, 0);

	_timer_thread_exit = true;
	pthread_join(_timer_thread, 0);

	dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->deinitSurface();
}

void OSystem_Android::setWindowCaption(const Common::U32String &caption) {
	JNI::setWindowCaption(caption);
}

void OSystem_Android::showVirtualKeyboard(bool enable) {
	ENTER("%d", enable);

	JNI::showVirtualKeyboard(enable);
}

Audio::Mixer *OSystem_Android::getMixer() {
	assert(_mixer);
	return _mixer;
}

void OSystem_Android::getTimeAndDate(TimeDate &td) const {
	struct tm tm;
	const time_t curTime = time(0);

	localtime_r(&curTime, &tm);
	td.tm_sec = tm.tm_sec;
	td.tm_min = tm.tm_min;
	td.tm_hour = tm.tm_hour;
	td.tm_mday = tm.tm_mday;
	td.tm_mon = tm.tm_mon;
	td.tm_year = tm.tm_year;
	td.tm_wday = tm.tm_wday;
}

void OSystem_Android::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	ENTER("");

	JNI::addSysArchivesToSearchSet(s, priority);
}

void OSystem_Android::logMessage(LogMessageType::Type type, const char *message) {
	switch (type) {
	case LogMessageType::kInfo:
		__android_log_write(ANDROID_LOG_INFO, android_log_tag, message);
		break;

	case LogMessageType::kDebug:
		__android_log_write(ANDROID_LOG_DEBUG, android_log_tag, message);
		break;

	case LogMessageType::kWarning:
		__android_log_write(ANDROID_LOG_WARN, android_log_tag, message);
		break;

	case LogMessageType::kError:
		__android_log_write(ANDROID_LOG_ERROR, android_log_tag, message);
		break;
	}
}

Common::String OSystem_Android::getSystemLanguage() const {
	return Common::String::format("%s_%s",
							getSystemProperty("persist.sys.language").c_str(),
							getSystemProperty("persist.sys.country").c_str());
}

bool OSystem_Android::openUrl(const Common::String &url) {
	return JNI::openUrl(url);
}

bool OSystem_Android::hasTextInClipboard() {
	return JNI::hasTextInClipboard();
}

Common::U32String OSystem_Android::getTextFromClipboard() {
	return JNI::getTextFromClipboard();
}

bool OSystem_Android::setTextInClipboard(const Common::U32String &text) {
	return JNI::setTextInClipboard(text);
}

bool OSystem_Android::isConnectionLimited() {
	return JNI::isConnectionLimited();
}

Common::String OSystem_Android::getSystemProperty(const char *name) const {
	char value[PROP_VALUE_MAX];

	int len = __system_property_get(name, value);

	return Common::String(value, len);
}

#endif
