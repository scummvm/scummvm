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

#include "backends/platform/android/jni-android.h"
#include "backends/platform/android/android.h"
#include "backends/platform/android/graphics.h"

const char *android_log_tag = "ScummVM";

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
//	_fingersDown(0),
	_firstPointerId(-1),
	_secondPointerId(-1),
	_thirdPointerId(-1),
	_trackball_scale(2),
	_joystick_scale(10) {

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

//
// When launching ScummVM (from ScummVMActivity) order of business is as follows:
// 1. scummvm_main() (base/main.cpp)
// 1.1. call system.initBackend() (from scummvm_main() (base/main.cpp))
//       According to comments in main.cpp:
//         "Init the backend. Must take place after all config data (including the command line params) was read."
// 1.2. call setupGraphics(system); (from scummvm_main() (base/main.cpp))
// 1.3. call launcherDialog() (from scummvm_main() (base/main.cpp))
//         Upon calling launcherDialog() the transient domain configuration options are cleared!
//       According to comments in main.cpp:
//         "Those that affect the graphics mode and the others (like bootparam etc.) should not blindly be passed to the first game launched from the launcher."
void OSystem_Android::initBackend() {
	ENTER();

	_main_thread = pthread_self();

	// Warning: ConfMan.registerDefault() can be used for a Session of ScummVM
	//          but:
	//              1. The values will NOT persist to storage
	//                 ie. they won't get saved to scummvm.ini
	//              2. The values will NOT be reflected on the GUI
	//                 and they cannot be recovered after exiting scummvm and re-launching
	//          Also, if after a ConfMan.registerDefault(), we subsequently use ConfMan.hasKey()
	//          here or anywhere else in ScummVM, it WILL NOT return true.
	//			As noted in ConfigManager::hasKey() implementation: (common/config_manager.cpp)
	//			// Search the domains in the following order:
	//              // 1) the transient domain,
	//              // 2) the active game domain (if any),
	//              // 3) the application domain.
	//         -->  // The defaults domain is explicitly *not* checked. <--
	//
	// So for at least some of these keys,
	// we need to additionally check with hasKey() if they are persisted
	// and set them explicitly that way.
	// TODO Maybe the registerDefault only has meaning for "savepath"
	//      and similar key/values retrieved from "Command Line"
	//      so that they won't get "nuked"
	//      and maintained for the duration ScummVM app session (until we exit the app)
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);
	ConfMan.registerDefault("filtering", false);
	ConfMan.registerDefault("autosave_period", 0);

	// explicitly set this, since fullscreen cannot be changed from GUI
	// and for Android it should be persisted (and ConfMan.hasKey("fullscreen") check should return true for it)
	// Also in Options::dialogBuild() (gui/options.cpp), since Android does not have kFeatureFullscreenMode (see hasFeature() below)
	//      the state of the checkbox in the GUI is set to true (and disabled)
	ConfMan.setBool("fullscreen", true);

	// Aspect ratio can be changed from the GUI.
	// However we set it explicitly here (in addition to the registerDefault command above)
	//         if it's not already set in the persistent config file
	if (!ConfMan.hasKey("aspect_ratio")) {
		ConfMan.setBool("aspect_ratio", true);
	}

	if (!ConfMan.hasKey("filtering")) {
		ConfMan.setBool("filtering", false);
	}

	// Note: About the stretch mode setting
	//       If not explicitly set in the config file
	//       the default used by OSystem::setStretchMode() (common/system.h)
	//       is the one returned by getDefaultStretchMode() (backends/graphics/opengl-graphics.cpp)
	//       which currently is STRETCH_FIT

	if (!ConfMan.hasKey("autosave_period")) {
		ConfMan.setInt("autosave_period", 0);
	}

	// The swap_menu_and_back is a deprecated configuration key
	// It is no longer relevant, after introducing the keymapper functionality
	// since the behaviour of the menu and back buttons is now handled by the keymapper.
	// We now ignore it completely

	ConfMan.setBool("FM_high_quality", false);
	ConfMan.setBool("FM_medium_quality", true);

	// We need a relaxed delay for the remapping timeout since handling touch interface and virtual keyboard can be slow
	// and especially in some occasions when we need to pull down (hide) the keyboard and map a system key (like the AC_Back) button.
	// 8 seconds should be enough
	ConfMan.registerDefault("remap_timeout_delay_ms", 8000);
	if (!ConfMan.hasKey("remap_timeout_delay_ms")) {
		ConfMan.setInt("remap_timeout_delay_ms", 8000);
	}

	if (!ConfMan.hasKey("browser_lastpath")) {
		ConfMan.set("browser_lastpath", "/");
	}

	// BUG: "transient" ConfMan settings get nuked by the options
	// screen. Passing the savepath in this way makes it stick
	// (via ConfMan.registerDefault() which is called from DefaultSaveFileManager constructor (backends/saves/default/default-saves.cpp))
	// Note: The aforementioned bug is probably the one reported here:
	//  https://bugs.scummvm.org/ticket/3712
	//  and maybe here:
	//  https://bugs.scummvm.org/ticket/7389
	// However, we do NOT set the savepath key explicitly for ConfMan
	//          and thus the savepath will only be persisted as "default" config
	//          for the rest of the app session (until exit).
	//          It will NOT be reflected on the GUI, if it's not set explicitly by the user there
	// TODO Why do we need it not shown on the GUI though?
	//      Btw, this is a ScummVM thing, the "defaults" do not show they values on our GUI)
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

	// renice this thread to boost the audio thread
	if (setpriority(PRIO_PROCESS, 0, 19) < 0)
		warning("couldn't renice the main thread");

	JNI::setReadyForEvents(true);

	_eventManager = new DefaultEventManager(this);
	_audiocdManager = new DefaultAudioCDManager();

	BaseBackend::initBackend();
}

bool OSystem_Android::hasFeature(Feature f) {
	if (f == kFeatureFullscreenMode)
		return false;
	if (f == kFeatureVirtualKeyboard ||
			f == kFeatureOpenUrl ||
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
		JNI::showVirtualKeyboard(enable);
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
	default:
		return ModularGraphicsBackend::getFeatureState(f);
	}
}

// TODO Re-eval if we need this here
Common::HardwareInputSet *OSystem_Android::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));
	inputSet->addHardwareInputSet(new KeyboardHardwareInputSet(defaultKeys, defaultModifiers));
	inputSet->addHardwareInputSet(new JoystickHardwareInputSet(defaultJoystickButtons, defaultJoystickAxes));

	return inputSet;
}

// TODO Re-eval if we need this here
Common::KeymapArray OSystem_Android::getGlobalKeymaps() {
	Common::KeymapArray globalMaps = BaseBackend::getGlobalKeymaps();
	return globalMaps;
}

Common::KeymapperDefaultBindings *OSystem_Android::getKeymapperDefaultBindings() {
	Common::KeymapperDefaultBindings *keymapperDefaultBindings = new Common::KeymapperDefaultBindings();

	// The swap_menu_and_back is a legacy configuration key
	// We now ignore it entirely (it as always false -- ie. back short press is AC_BACK)

	//
	// Note: setDefaultBinding maps a hw input to a keymapId_actionId combo.
	//
	// Clarifications/Quote by developer bgK (via Discord, Oct 3, 2020)
	// bgK: [With the introduction of the ScummVM keymapper we have] "standard actions" defined in "standard-actions.h".
	//      The engines use those as much as possible when defining keymaps.
	//      Then, the backends can override the default bindings to make use of the platform specific keys.
	//
	//
	keymapperDefaultBindings->setDefaultBinding(Common::kGlobalKeymapName, "MENU", "MENU");
	//
	// We want the AC_BACK key to be the default (until overridden explicitly by the user or a game engine)
	// mapped key for the standard SKIP action.
	//
	// bgK: "engine-default" is for the default keymap used by games that don't define their own keymap.
	//      [We] want Common::kStandardActionsKeymapName to override the action for all the keymaps
	//      Common::kStandardActionsKeymapName is used as a fallback if there are no keymap specific bindings defined.
	//      So it should be enough on its own.
	// [ie. we don't have to set default binding for "engine-default", as well]
	// ["engine-default" is used for to create a Keymap sequence of type kKeymapTypeGame in engines/metaengine.cpp initKeymaps() for an engine]
	// [In initKeymaps() is where the default key Esc is mapped to Skip action for game engines]
	//
	// [kStandardActionsKeymapName is defined  as (constant char*) in ./backends/keymapper/keymap, and utilised in getActionDefaultMappings()]
	// ["If no keymap-specific default mapping was found, look for a standard action binding"]
	keymapperDefaultBindings->setDefaultBinding(Common::kStandardActionsKeymapName, Common::kStandardActionSkip, "AC_BACK");

	// The "CLOS" action ID is not a typo.
	// See: backends/keymapper/remap-widget.cpp:	kCloseCmd        = 'CLOS'
	keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, "CLOS", "AC_BACK");

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
}

void OSystem_Android::setWindowCaption(const Common::U32String &caption) {
	JNI::setWindowCaption(caption);
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
