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
//   __attribute__ ((format(__printf__, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_ftell

#include <EGL/egl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/system_properties.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>

#if defined(__arm__) || defined(__x86_64__) || defined(__i386__)
#include <cpu-features.h>
#endif

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"
#include "backends/fs/android/android-fs.h"
#include "backends/fs/android/android-fs-factory.h"
#include "backends/fs/posix/posix-iostream.h"

#include "backends/graphics/android/android-graphics.h"
#include "backends/graphics3d/android/android-graphics3d.h"

#include "backends/audiocd/default/default-audiocd.h"
#include "backends/events/default/default-events.h"
#include "backends/mutex/pthread/pthread-mutex.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"

#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/keymapper-defaults.h"
#include "backends/keymapper/standard-actions.h"

#include "common/util.h"
#include "common/textconsole.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/mutex.h"
#include "common/events.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "graphics/cursorman.h"

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

class AndroidSaveFileManager : public DefaultSaveFileManager {
public:
	AndroidSaveFileManager(const Common::Path &defaultSavepath) : DefaultSaveFileManager(defaultSavepath) {}

	bool removeSavefile(const Common::String &filename) override {
		Common::String path = getSavePath().join(filename).toString(Common::Path::kNativeSeparator);
		AbstractFSNode *node = AndroidFilesystemFactory::instance().makeFileNodePath(path);

		if (!node) {
			return false;
		}

		AndroidFSNode *anode = dynamic_cast<AndroidFSNode *>(node);

		if (!anode) {
			// This should never happen
			warning("Invalid node received");
			delete node;
			return false;
		}

		bool ret = anode->remove();

		delete anode;

		if (!ret) {
			setError(Common::kUnknownError, Common::String::format("Couldn't delete the save file: %s", path.c_str()));
		}
		return ret;
	}
};

OSystem_Android::OSystem_Android(int audio_sample_rate, int audio_buffer_size) :
	_audio_sample_rate(audio_sample_rate),
	_audio_buffer_size(audio_buffer_size),
	_screen_changeid(0),
	_mixer(0),
	_event_queue_lock(0),
	_touch_pt_down(),
	_touch_pt_scroll(),
	_touch_pt_dt(),
	_eventScaleX(100),
	_eventScaleY(100),
#if defined(USE_OPENGL) && defined(USE_GLAD)
	_gles2DL(nullptr),
#endif
	// TODO put these values in some option dlg?
	_touch_mode(TOUCH_MODE_TOUCHPAD),
	_touchpad_scale(66),
	_dpad_scale(4),
//	_fingersDown(0),
	_firstPointerId(-1),
	_secondPointerId(-1),
	_thirdPointerId(-1),
	_trackball_scale(2),
	_joystick_scale(10),
	_engineRunning(false),
	_defaultConfigFileName(),
	_defaultLogFileName(),
	_systemPropertiesSummaryStr(""),
	_systemSDKdetectedStr(""),
	_logger(nullptr) {

	_systemPropertiesSummaryStr = Common::String::format("Running on: [%s] [%s] [%s] [%s] [%s] SDK:%s ABI:%s\n",
	                                                   getSystemProperty("ro.product.manufacturer").c_str(),
	                                                   getSystemProperty("ro.product.model").c_str(),
	                                                   getSystemProperty("ro.product.brand").c_str(),
	                                                   getSystemProperty("ro.build.fingerprint").c_str(),
	                                                   getSystemProperty("ro.build.display.id").c_str(),
	                                                   getSystemProperty("ro.build.version.sdk").c_str(),
	                                                   getSystemProperty("ro.product.cpu.abi").c_str()) ;

	LOGI("%s", _systemPropertiesSummaryStr.c_str());
	// JNI::getAndroidSDKVersionId() should be identical to the result from ("ro.build.version.sdk"),
	// though getting it via JNI is maybe the most reliable option (?)
	// Also __system_property_get which is used by getSystemProperty() is being deprecated in recent NDKs

	int sdkVersion = JNI::getAndroidSDKVersionId();

	_systemSDKdetectedStr = Common::String::format("SDK Version: %d\n", sdkVersion) ;
	LOGI("%s", _systemSDKdetectedStr.c_str());

	AndroidFilesystemFactory &fsFactory = AndroidFilesystemFactory::instance();
	if (sdkVersion >= 24) {
		fsFactory.initSAF();
	}
	_fsFactory = &fsFactory;
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
	_fsFactory = 0;
	AndroidFilesystemFactory::destroy();
	delete _timerManager;
	_timerManager = 0;

	delete _event_queue_lock;

	delete _savefileManager;
	_savefileManager = 0;

	// Uninitialize graphics manager now to avoid it to be done later when touch controls are destroyed
	delete _graphicsManager;
	_graphicsManager = 0;

	delete _logger;
	_logger = nullptr;
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

	if (!_logger)
		_logger = new Backends::Log::Log(this);

	if (_logger) {
		Common::WriteStream *logFile = createLogFileForAppending();
		if (logFile) {
			_logger->open(logFile);

			if (!_systemPropertiesSummaryStr.empty())
				_logger->print(_systemPropertiesSummaryStr.c_str());

			if (!_systemSDKdetectedStr.empty())
				_logger->print(_systemSDKdetectedStr.c_str());
		} else {
			LOGE("Error when opening log file for writing upon initializing backend");
			//_logger->close();
			_logger = nullptr;
		}
	}

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
	// slow down a bit virtual mouse speed (typical default seems to be "3") - eg. when controlling the virtual mouse cursor with DPAD keys
	// Also see declaration of support for feature kFeatureKbdMouseSpeed bellow
	ConfMan.registerDefault("kbdmouse_speed", 2);
	ConfMan.registerDefault("joystick_deadzone", 3);

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
		ConfMan.setPath("browser_lastpath", "/");
	}

	Common::String basePath = JNI::getScummVMBasePath();

	_savefileManager = new AndroidSaveFileManager(Common::Path(basePath, Common::Path::kNativeSeparator).joinInPlace("saves"));
	// TODO remove the debug message eventually
	LOGD("Setting DefaultSaveFileManager path to: %s", ConfMan.getPath("savepath").toString(Common::Path::kNativeSeparator).c_str());


	ConfMan.registerDefault("iconspath", Common::Path(basePath, Common::Path::kNativeSeparator).joinInPlace("icons"));
	// TODO remove the debug message eventually
	LOGD("Setting Default Icons and Shaders path to: %s", ConfMan.getPath("iconspath").toString(Common::Path::kNativeSeparator).c_str());

	_timerManager = new DefaultTimerManager();

	_event_queue_lock = new Common::Mutex();

	gettimeofday(&_startTime, 0);

	// The division by four happens because the Mixer stores the size in frame units
	// instead of bytes; this means that, since we have audio in stereo (2 channels)
	// with a word size of 16 bit (2 bytes), we have to divide the effective size by 4.
	_mixer = new Audio::MixerImpl(_audio_sample_rate, true, _audio_buffer_size / 4);
	_mixer->setReady(true);

	_timer_thread_exit = false;
	pthread_create(&_timer_thread, 0, timerThreadFunc, this);

	_audio_thread_exit = false;
	pthread_create(&_audio_thread, 0, audioThreadFunc, this);

	JNI::DPIValues dpi;
	JNI::getDPI(dpi);
	_touchControls.init(dpi[2]);

	_graphicsManager = new AndroidGraphicsManager();

	// renice this thread to boost the audio thread
	if (setpriority(PRIO_PROCESS, 0, 19) < 0)
		warning("couldn't renice the main thread");

	JNI::setReadyForEvents(true);

	_eventManager = new DefaultEventManager(this);
	_audiocdManager = new DefaultAudioCDManager();

	BaseBackend::initBackend();
}

void OSystem_Android::engineInit() {
	_engineRunning = true;
	updateOnScreenControls();

	JNI::setCurrentGame(ConfMan.getActiveDomainName());
}

void OSystem_Android::engineDone() {
	_engineRunning = false;
	updateOnScreenControls();
	JNI::setCurrentGame("");
}

void OSystem_Android::updateOnScreenControls() {
	int enableMask = SHOW_ON_SCREEN_ALL;
	if (!ConfMan.getBool("onscreen_control")) {
		enableMask = SHOW_ON_SCREEN_NONE;
	} else if (!_engineRunning) {
		// Don't show the menu icon if the engine is not running
		enableMask &= ~SHOW_ON_SCREEN_MENU;
	}
	JNI::showOnScreenControls(enableMask);
}

Common::Path OSystem_Android::getDefaultConfigFileName() {
	// if possible, skip JNI call which is more costly (performance wise)
	if (_defaultConfigFileName.empty()) {
		_defaultConfigFileName = JNI::getScummVMConfigPath();
	}
	return _defaultConfigFileName;
}

Common::Path OSystem_Android::getDefaultLogFileName() {
	if (_defaultLogFileName.empty()) {
		_defaultLogFileName = JNI::getScummVMLogPath();
	}
	return _defaultLogFileName;
}

Common::WriteStream *OSystem_Android::createLogFileForAppending() {
	Common::String logPath(getDefaultLogFileName().toString(Common::Path::kNativeSeparator));

	if (logPath.empty()) {
		__android_log_write(ANDROID_LOG_ERROR, android_log_tag, "Log file path is not known upon create attempt!");
		return nullptr;
	}

	FILE *scvmLogFilePtr = fopen(logPath.c_str(), "a");
	if (scvmLogFilePtr != nullptr) {
		// We check for log file size; if it's too big, we rewrite it.
		// This happens only upon app launch, in initBackend() when createLogFileForAppending() is called
		// NOTE: We don't check for file size each time we write a log message.
		long sz = ftell(scvmLogFilePtr);
		if (sz > MAX_ANDROID_SCUMMVM_LOG_FILESIZE_IN_BYTES) {
			fclose(scvmLogFilePtr);
			__android_log_write(ANDROID_LOG_WARN, android_log_tag, "Default log file is bigger than 100KB. It will be overwritten!");
			if (!logPath.empty()) {
				// Create the log file from scratch overwriting the previous one
				scvmLogFilePtr = fopen(logPath.c_str(), "w");
				if (scvmLogFilePtr == nullptr) {
					__android_log_write(ANDROID_LOG_ERROR, android_log_tag, "Could not open default log file for rewrite!");
					return nullptr;
				}
			} else {
				__android_log_write(ANDROID_LOG_ERROR, android_log_tag, "Log file path is not known upon rewrite attempt!");
				return nullptr;
			}
		}
	} else {
		__android_log_write(ANDROID_LOG_ERROR, android_log_tag, "Could not open default log file for writing/appending.");
		__android_log_write(ANDROID_LOG_ERROR, android_log_tag, logPath.c_str());
	}
	return new PosixIoStream(scvmLogFilePtr);
}

bool OSystem_Android::hasFeature(Feature f) {
	if (f == kFeatureFullscreenMode)
		return false;
	if (f == kFeatureVirtualKeyboard ||
			f == kFeatureOpenUrl ||
			f == kFeatureClipboardSupport ||
			f == kFeatureKbdMouseSpeed ||
			f == kFeatureJoystickDeadzone ||
			f == kFeatureTouchscreen) {
		return true;
	}
	/* Even if we are using the 2D graphics manager,
	 * we are at one initGraphics3d call of supporting GLES2 */
	if (f == kFeatureOpenGLForGame) return true;
	/* GLES2 always supports shaders */
	if (f == kFeatureShadersForGame) return true;

	if (f == kFeatureCpuNEON) {
#if defined(__aarch64__)
		// ARMv8 mandates NEON
		return true;
#elif defined(__arm__)
		return (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON);
#else
		return false;
#endif
	}

	if (f == kFeatureCpuSSE2) {
#if defined(__x86_64__)
		// x86_64 mandates SSE2
		return true;
#elif defined(__i386__) && defined(__SSE2__)
		// Android NDK mandates SSE2 starting with Jellybean but some people tried hacks
		// Allow to disable SSE2
		return true;
#else
		return false;
#endif
	}

	if (f == kFeatureCpuSSE41) {
#if defined(__x86_64__) || defined(__i386__)
		return (android_getCpuFeatures() & ANDROID_CPU_X86_FEATURE_SSE4_1);
#else
		return false;
#endif
	}

	if (f == kFeatureCpuAVX2) {
#if defined(__x86_64__)
		// No AVX2 in 32-bits
		return (android_getCpuFeatures() & ANDROID_CPU_X86_FEATURE_AVX2);
#else
		return false;
#endif
	}

	if (f == kFeatureCpuAltivec) {
		return false;
	}

	return ModularGraphicsBackend::hasFeature(f);
}

void OSystem_Android::setFeatureState(Feature f, bool enable) {
	ENTER("%d, %d", f, enable);

	switch (f) {
	case kFeatureVirtualKeyboard:
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

	// By default DPAD directions will be used for virtual mouse in GUI context
	// If the user wants to remap them, they will be able to navigate to Global Options -> Keymaps and do so.
	// In some devices (eg. Android TV) with only the remote control as DPAD input, it is impossible to navigate the launcher GUI,
	// if the DPAD actions are mapped to "UP", "DOWN", "LEFT", "RIGHT" directions (GUI context) and not mouse cursor movement.
	// TODO If/when full key-based (ie. non-mouse) navigation of the ScummVM GUI is implemented,
	// we can revert back to the core behavior of DPAD being mapped to "up", "down", "left", "right" directions.
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSEUP", "JOY_LEFT_STICK_Y-");
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSEUP", "JOY_UP");
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSEDOWN", "JOY_LEFT_STICK_Y+");
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSEDOWN", "JOY_DOWN");
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSELEFT", "JOY_LEFT_STICK_X-");
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSELEFT", "JOY_LEFT");
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSERIGHT", "JOY_LEFT_STICK_X+");
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSERIGHT", "JOY_RIGHT");
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSESLOW", "JOY_RIGHT_SHOULDER");
	keymapperDefaultBindings->addDefaultBinding(Common::kGlobalKeymapName, "VMOUSESLOW", "AUDIOPLAYPAUSE");
	keymapperDefaultBindings->addDefaultBinding(Common::kGuiKeymapName, Common::kStandardActionInteract, "JOY_A");
	keymapperDefaultBindings->addDefaultBinding(Common::kGuiKeymapName, Common::kStandardActionInteract, "JOY_CENTER");
	keymapperDefaultBindings->addDefaultBinding(Common::kGuiKeymapName, Common::kStandardActionInteract, "SELECT");
	// NOTE using nullptr as the third argument clears the bindings for the action.
	keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, Common::kStandardActionMoveUp, "UP");
	keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, Common::kStandardActionMoveDown, "DOWN");
	keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, Common::kStandardActionMoveLeft, "LEFT");
	keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, Common::kStandardActionMoveRight, "RIGHT");

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

Common::MutexInternal *OSystem_Android::createMutex() {
	return createPthreadMutexInternal();
}

void OSystem_Android::quit() {
	ENTER();

	_audio_thread_exit = true;
	_timer_thread_exit = true;

	JNI::wakeupForQuit();
	JNI::setReadyForEvents(false);

	pthread_join(_audio_thread, 0);
	pthread_join(_timer_thread, 0);
}

void OSystem_Android::setWindowCaption(const Common::U32String &caption) {
	JNI::setWindowCaption(caption);
}

Audio::Mixer *OSystem_Android::getMixer() {
	assert(_mixer);
	return _mixer;
}

void OSystem_Android::getTimeAndDate(TimeDate &td, bool skipRecord) const {
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

	// Then log into file (via the logger)
	if (_logger)
		_logger->print(message);

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

const OSystem::GraphicsMode *OSystem_Android::getSupportedGraphicsModes() const {
	// We only support one mode
	static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
		{ "default", "Default", 0 },
		{ 0, 0, 0 },
	};

	return s_supportedGraphicsModes;
}

int OSystem_Android::getDefaultGraphicsMode() const {
	// We only support one mode
	return 0;
}

bool OSystem_Android::setGraphicsMode(int mode, uint flags) {
	bool render3d = flags & OSystem::kGfxModeRender3d;

	// Very hacky way to set up the old graphics manager state, in case we
	// switch from SDL->OpenGL or OpenGL->SDL.
	//
	// This is a probably temporary workaround to fix bugs like #5799
	// "SDL/OpenGL: Crash when switching renderer backend".
	//
	// It's also used to restore state from 3D to 2D GFX manager
	AndroidCommonGraphics *androidGraphicsManager = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager);
	AndroidCommonGraphics::State gfxManagerState = androidGraphicsManager->getState();
	bool supports3D = _graphicsManager->hasFeature(kFeatureOpenGLForGame);

	bool switchedManager = false;

	// If the new mode and the current mode are not from the same graphics
	// manager, delete and create the new mode graphics manager
	debug(5, "requesting 3D: %d, supporting 3D: %d", render3d, supports3D);
	if (render3d && !supports3D) {
		debug(5, "switching to 3D graphics");
		delete _graphicsManager;
		_graphicsManager = nullptr;
		AndroidGraphics3dManager *manager = new AndroidGraphics3dManager();
		_graphicsManager = manager;
		androidGraphicsManager = manager;
		switchedManager = true;
	} else if (!render3d && supports3D) {
		debug(5, "switching to 2D graphics");
		delete _graphicsManager;
		_graphicsManager = nullptr;
		AndroidGraphicsManager *manager = new AndroidGraphicsManager();
		_graphicsManager = manager;
		androidGraphicsManager = manager;
		switchedManager = true;
	}

	androidGraphicsManager->syncVirtkeyboardState(_virtkeybd_on);

	if (switchedManager) {
		// Setup the graphics mode and size first
		// This is needed so that we can check the supported pixel formats when
		// restoring the state.
		_graphicsManager->beginGFXTransaction();
		if (!_graphicsManager->setGraphicsMode(mode, flags))
			return false;
		_graphicsManager->initSize(gfxManagerState.screenWidth, gfxManagerState.screenHeight);
		_graphicsManager->endGFXTransaction();

		// This failing will probably have bad consequences...
		if (!androidGraphicsManager->setState(gfxManagerState)) {
			return false;
		}

		// Next setup the cursor again
		CursorMan.pushCursor(0, 0, 0, 0, 0, 0);
		CursorMan.popCursor();

		// Next setup cursor palette if needed
		if (_graphicsManager->getFeatureState(kFeatureCursorPalette)) {
			CursorMan.pushCursorPalette(0, 0, 0);
			CursorMan.popCursorPalette();
		}

		_graphicsManager->beginGFXTransaction();
		return true;
	} else {
		return _graphicsManager->setGraphicsMode(mode, flags);
	}
}

int OSystem_Android::getGraphicsMode() const {
	// We only support one mode
	return 0;
}

#if defined(USE_OPENGL) && defined(USE_GLAD)
void *OSystem_Android::getOpenGLProcAddress(const char *name) const {
	// eglGetProcAddress exists since Android 2.3 (API Level 9)
	// EGL 1.5+ supports loading core functions too: try to optimize
	if (JNI::eglVersion() >= 0x00010005) {
		return (void *)eglGetProcAddress(name);
	}

	if (!_gles2DL) {
		_gles2DL = dlopen("libGLESv2.so", RTLD_NOW | RTLD_LOCAL);
		if (!_gles2DL) {
			error("Can't load libGLESv2.so with old EGL context");
		}
	}

	void *ptr = dlsym(_gles2DL, name);
	if (!ptr) {
		ptr = (void *)eglGetProcAddress(name);
	}
	return ptr;
}
#endif

static const char * const helpTabs[] = {

_s("Getting help"),
"",
_s(
"## Help, I'm lost!\n"
"\n"
"First, make sure you have the games and necessary game files ready. Check the **Where to Get the Games** section under the **General** tab. Once obtained, follow the steps outlined in the **Adding Games** tab to finish adding them on this device. Take a moment to review this process carefully, as some users encountered challenges here owing to recent Android changes.\n"
"\n"
"Need more help? Refer to our [online documentation for Android](https://docs.scummvm.org/en/latest/other_platforms/android.html). Got questions? Swing by our [support forums](https://forums.scummvm.org/viewforum.php?f=17) or hop on our [Discord server](https://discord.gg/4cDsMNtcpG), which includes an [Android support channel](https://discord.com/channels/581224060529148060/1135579923185139862).\n"
"\n"
"Oh, and heads up, many of our supported games are intentionally tricky, sometimes mind-bogglingly so. If you're stuck in a game, think about checking out a game walkthrough. Good luck!\n"
),

_s("Touch Controls"),
"android-help.zip",
_s(
"## Touch control modes\n"
"The touch control mode can be changed by tapping or clicking on the controller icon in the upper right corner"
"\n"
"### Direct mouse \n"
"\n"
"The touch controls are direct. The pointer jumps to where the finger touches the screen (default for menus).\n"
"\n"
"  ![Direct mouse mode](mouse.png \"Direct mouse mode\"){w=10em}\n"
"\n"
"### Touchpad emulation \n"
"\n"
"The touch controls are indirect, like on a laptop touchpad.\n"
"\n"
"  ![Touchpad mode](touchpad.png \"Touchpad mode\"){w=10em}\n"
"\n"
"### Gamepad emulation \n"
"\n"
"Fingers must be placed on lower left and right of the screen to emulate a directional pad and action buttons.\n"
"\n"
"  ![Gamepad mode](gamepad.png \"Gamepad mode\"){w=10em}\n"
"\n"
"To select the preferred touch mode for menus, 2D games, and 3D games, go to **Global Options > Backend > Choose the preferred touch mode**.\n"
"\n"
"## Touch actions \n"
"\n"
"### Two finger scroll \n"
"\n"
"To scroll, slide two fingers up or down the screen"
"\n"
"### Two finger tap\n"
"\n"
"To do a two finger tap, hold one finger down and then tap with a second finger.\n"
"\n"
"### Three finger tap\n"
"\n"
"To do a three finger tap, start with holding down one finger and progressively touch down the other two fingers, one at a time, while still holding down the previous fingers. Imagine you are impatiently tapping your fingers on a surface, but then slow down that movement so it is rhythmic, but not too slow.\n"
"\n"
"### Immersive Sticky fullscreen mode\n"
"\n"
"Swipe from the edge to reveal the system bars.  They remain semi-transparent and disappear after a few seconds unless you interact with them.\n"
"\n"
"### Global Main Menu\n"
"\n"
"To open the Global Main Menu, tap on the menu icon at the top right of the screen.\n"
"\n"
"  ![Menu icon](menu.png \"Menu icon\"){w=10em}\n"
"\n"
"## Virtual keyboard\n"
"\n"
"To open the virtual keyboard, long press on the controller icon at the top right of the screen, or tap on any editable text field. To hide the virtual keyboard, tap the controller icon again, or tap outside the text field.\n"
"\n"
"\n"
"  ![Keybpard icon](keyboard.png \"Keyboard icon\"){w=10em}\n"
"\n"
	),

_s("Adding Games"),
"android-help.zip",
_s(
"## Adding Games \n"
"\n"
"1. Select **Add Game...** from the launcher. \n"
"\n"
"2. Inside the ScummVM file browser, select **Go Up** until you reach the root folder which has the **<Add a new folder>** option. \n"
"\n"
"  ![ScummVM file browser root](browser-root.png \"ScummVM file browser root\"){w=70%}\n"
"\n"
"3. Double-tap **<Add a new folder>**. In your device's file browser, navigate to the folder containing all your game folders. For example, **SD Card > ScummVMgames**. \n"
"\n"
"4. Select **Use this folder**. \n"
"\n"
"  ![OS selectable folder](fs-folder.png \"OS selectable folder\"){w=70%}\n"
"\n"
"5. Select **ALLOW** to give ScummVM permission to access the folder. \n"
"\n"
"  ![OS access permission dialog](fs-permission.png \"OS access permission\"){w=70%}\n"
"\n"
"6. In the ScummVM file browser, double-tap to browse through your added folder. Add a game by selecting the sub-folder containing the game files, then tap **Choose**. \n"
"\n"
"  ![SAF folder added](browser-folder-in-list.png \"SAF folder added\"){w=70%}\n"
"\n"
"Step 2 and 3 are done only once. To add more games, repeat Steps 1 and 6. \n"
"\n"
"See our [Android documentation](https://docs.scummvm.org/en/latest/other_platforms/android.html) for more information.\n"
	),

0 // End of list
};

const char * const *OSystem_Android::buildHelpDialogData() {
	return helpTabs;
}
