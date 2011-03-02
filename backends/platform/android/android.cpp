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

#if defined(__ANDROID__)

#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#include "common/util.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/mutex.h"
#include "common/events.h"
#include "common/config-manager.h"

#include "backends/keymapper/keymapper.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"

#include "backends/platform/android/jni.h"
#include "backends/platform/android/android.h"

const char *android_log_tag = "ScummVM";

// This replaces the bionic libc assert functions with something that
// actually prints the assertion failure before aborting.
extern "C" {
	void __assert(const char *file, int line, const char *expr) {
		__android_log_assert(expr, android_log_tag,
								"Assertion failure: '%s' in %s:%d",
								 expr, file, line);
	}

	void __assert2(const char *file, int line, const char *func, const char *expr) {
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

// floating point. use sparingly
template <class T>
static inline T scalef(T in, float numerator, float denominator) {
	return static_cast<float>(in) * numerator / denominator;
}

OSystem_Android::OSystem_Android(int audio_sample_rate, int audio_buffer_size) :
	_audio_sample_rate(audio_sample_rate),
	_audio_buffer_size(audio_buffer_size),
	_screen_changeid(0),
	_egl_surface_width(0),
	_egl_surface_height(0),
	_force_redraw(false),
	_game_texture(0),
	_overlay_texture(0),
	_mouse_texture(0),
	_use_mouse_palette(false),
	_show_mouse(false),
	_show_overlay(false),
	_enable_zoning(false),
	_savefile(0),
	_mixer(0),
	_timer(0),
	_fsFactory(new POSIXFilesystemFactory()),
	_shake_offset(0),
	_event_queue_lock(createMutex()) {
}

OSystem_Android::~OSystem_Android() {
	ENTER();

	delete _game_texture;
	delete _overlay_texture;
	delete _mouse_texture;

	delete _savefile;
	delete _timer;
	delete _mixer;
	delete _fsFactory;

	deleteMutex(_event_queue_lock);
}

void *OSystem_Android::timerThreadFunc(void *arg) {
	OSystem_Android *system = (OSystem_Android *)arg;
	DefaultTimerManager *timer = (DefaultTimerManager *)(system->_timer);

	// renice this thread to boost the audio thread
	if (setpriority(PRIO_PROCESS, 0, 19) < 0)
		LOGW("couldn't renice the timer thread");

	JNI::attachThread();

	struct timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = 100 * 1000 * 1000;	// 100ms

	while (!system->_timer_thread_exit) {
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

	byte *buf;
	int offset, left, written;
	int samples, i;

	struct timespec tv_delay;
	tv_delay.tv_sec = 0;
	tv_delay.tv_nsec = 20 * 1000 * 1000;

	uint msecs_full = buf_size * 1000 / (mixer->getOutputRate() * 2 * 2);

	struct timespec tv_full;
	tv_full.tv_sec = 0;
	tv_full.tv_nsec = msecs_full * 1000 * 1000;

	bool silence;
	uint silence_count = 33;

	while (!system->_audio_thread_exit) {
		buf = (byte *)env->GetPrimitiveArrayCritical(bufa, 0);
		assert(buf);

		samples = mixer->mixCallback(buf, buf_size);

		silence = samples < 1;

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

		// sleep a little, prepare the next buffer, and run into the
		// blocking AudioTrack.write
		nanosleep(&tv_delay, 0);
	}

	JNI::setAudioStop();

	env->DeleteLocalRef(bufa);

	JNI::detachThread();

	return 0;
}

void OSystem_Android::initBackend() {
	ENTER();

	_main_thread = pthread_self();

	ConfMan.setInt("autosave_period", 0);
	ConfMan.setBool("FM_high_quality", false);
	ConfMan.setBool("FM_medium_quality", true);

	// must happen before creating TimerManager to avoid race in
	// creating EventManager
	setupKeymapper();

	// BUG: "transient" ConfMan settings get nuked by the options
	// screen. Passing the savepath in this way makes it stick
	// (via ConfMan.registerDefault)
	_savefile = new DefaultSaveFileManager(ConfMan.get("savepath"));
	_timer = new DefaultTimerManager();

	gettimeofday(&_startTime, 0);

	_mixer = new Audio::MixerImpl(this, _audio_sample_rate);
	_mixer->setReady(true);

	_timer_thread_exit = false;
	pthread_create(&_timer_thread, 0, timerThreadFunc, this);

	_audio_thread_exit = false;
	pthread_create(&_audio_thread, 0, audioThreadFunc, this);

	setupSurface();

	// renice this thread to boost the audio thread
	if (setpriority(PRIO_PROCESS, 0, 19) < 0)
		warning("couldn't renice the main thread");

	JNI::setReadyForEvents(true);
}

void OSystem_Android::addPluginDirectories(Common::FSList &dirs) const {
	ENTER();

	JNI::getPluginDirectories(dirs);
}

bool OSystem_Android::hasFeature(Feature f) {
	return (f == kFeatureCursorHasPalette ||
			f == kFeatureVirtualKeyboard ||
			f == kFeatureOverlaySupportsAlpha);
}

void OSystem_Android::setFeatureState(Feature f, bool enable) {
	ENTER("%d, %d", f, enable);

	switch (f) {
	case kFeatureVirtualKeyboard:
		_virtkeybd_on = enable;
		showVirtualKeyboard(enable);
		break;
	default:
		break;
	}
}

bool OSystem_Android::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureVirtualKeyboard:
		return _virtkeybd_on;
	default:
		return false;
	}
}

void OSystem_Android::setupKeymapper() {
#ifdef ENABLE_KEYMAPPER
	using namespace Common;

	Keymapper *mapper = getEventManager()->getKeymapper();

	HardwareKeySet *keySet = new HardwareKeySet();

	keySet->addHardwareKey(
		new HardwareKey("n", KeyState(KEYCODE_n), "n (vk)",
				kTriggerLeftKeyType,
				kVirtualKeyboardActionType));

	mapper->registerHardwareKeySet(keySet);

	Keymap *globalMap = new Keymap("global");
	Action *act;

	act = new Action(globalMap, "VIRT", "Display keyboard",
						kVirtualKeyboardActionType);
	act->addKeyEvent(KeyState(KEYCODE_F7, ASCII_F7, 0));

	mapper->addGlobalKeymap(globalMap);

	mapper->pushKeymap("global");
#endif
}

bool OSystem_Android::pollEvent(Common::Event &event) {
	//ENTER();

	if (pthread_self() == _main_thread) {
		if (_screen_changeid != JNI::surface_changeid) {
			if (JNI::egl_surface_width > 0 && JNI::egl_surface_height > 0) {
				LOGD("initializing surface");

				JNI::deinitSurface();
				setupSurface();

				event.type = Common::EVENT_SCREEN_CHANGED;

				return true;
			}

			LOGD("deinitialiting surface");

			_screen_changeid = JNI::surface_changeid;
			JNI::deinitSurface();
		}
	}

	lockMutex(_event_queue_lock);

	if (_event_queue.empty()) {
		unlockMutex(_event_queue_lock);
		return false;
	}

	event = _event_queue.pop();
	unlockMutex(_event_queue_lock);

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		// TODO: only dirty/redraw move bounds
		_force_redraw = true;
		// fallthrough
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP: {
		// relative mouse hack
		if (event.kbd.flags == 1) {
			// Relative (trackball) mouse hack.
			const Common::Point& mouse_pos =
				getEventManager()->getMousePos();
			event.mouse.x += mouse_pos.x;
			event.mouse.y += mouse_pos.y;
			event.mouse.x = CLIP(event.mouse.x, (int16)0, _show_overlay ?
									getOverlayWidth() : getWidth());
			event.mouse.y = CLIP(event.mouse.y, (int16)0, _show_overlay ?
									getOverlayHeight() : getHeight());
		} else {
			// Touchscreen events need to be converted
			// from device to game coords first.
			const GLESTexture *tex;
			if (_show_overlay)
				tex = _overlay_texture;
			else
				tex = _game_texture;

			event.mouse.x = scalef(event.mouse.x, tex->width(),
									_egl_surface_width);
			event.mouse.y = scalef(event.mouse.y, tex->height(),
									_egl_surface_height);
			event.mouse.x -= _shake_offset;
		}
		break;
	}
	default:
		break;
	}

	return true;
}

void OSystem_Android::pushEvent(const Common::Event& event) {
	lockMutex(_event_queue_lock);

	// Try to combine multiple queued mouse move events
	if (event.type == Common::EVENT_MOUSEMOVE &&
			!_event_queue.empty() &&
			_event_queue.back().type == Common::EVENT_MOUSEMOVE) {
		Common::Event tail = _event_queue.back();
		if (event.kbd.flags) {
			// relative movement hack
			tail.mouse.x += event.mouse.x;
			tail.mouse.y += event.mouse.y;
		} else {
			// absolute position, clear relative flag
			tail.kbd.flags = 0;
			tail.mouse.x = event.mouse.x;
			tail.mouse.y = event.mouse.y;
		}
	} else {
	  _event_queue.push(event);
	}

	unlockMutex(_event_queue_lock);
}

uint32 OSystem_Android::getMillis() {
	timeval curTime;

	gettimeofday(&curTime, 0);

	return (uint32)(((curTime.tv_sec - _startTime.tv_sec) * 1000) + \
			((curTime.tv_usec - _startTime.tv_usec) / 1000));
}

void OSystem_Android::delayMillis(uint msecs) {
	usleep(msecs * 1000);
}

OSystem::MutexRef OSystem_Android::createMutex() {
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_t *mutex = new pthread_mutex_t;

	if (pthread_mutex_init(mutex, &attr) != 0) {
		warning("pthread_mutex_init() failed");

		delete mutex;

		return 0;
	}

	return (MutexRef)mutex;
}

void OSystem_Android::lockMutex(MutexRef mutex) {
	if (pthread_mutex_lock((pthread_mutex_t *)mutex) != 0)
		warning("pthread_mutex_lock() failed");
}

void OSystem_Android::unlockMutex(MutexRef mutex) {
	if (pthread_mutex_unlock((pthread_mutex_t *)mutex) != 0)
		warning("pthread_mutex_unlock() failed");
}

void OSystem_Android::deleteMutex(MutexRef mutex) {
	pthread_mutex_t *m = (pthread_mutex_t *)mutex;

	if (pthread_mutex_destroy(m) != 0)
		warning("pthread_mutex_destroy() failed");
	else
		delete m;
}

void OSystem_Android::quit() {
	ENTER();

	JNI::setReadyForEvents(false);

	_audio_thread_exit = true;
	pthread_join(_audio_thread, 0);

	_timer_thread_exit = true;
	pthread_join(_timer_thread, 0);

	JNI::deinitSurface();
}

void OSystem_Android::setWindowCaption(const char *caption) {
	ENTER("%s", caption);

	JNI::setWindowCaption(caption);
}

void OSystem_Android::displayMessageOnOSD(const char *msg) {
	ENTER("%s", msg);

	JNI::displayMessageOnOSD(msg);
}

void OSystem_Android::showVirtualKeyboard(bool enable) {
	ENTER("%d", enable);

	JNI::showVirtualKeyboard(enable);
}

Common::SaveFileManager *OSystem_Android::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

Audio::Mixer *OSystem_Android::getMixer() {
	assert(_mixer);
	return _mixer;
}

Common::TimerManager *OSystem_Android::getTimerManager() {
	assert(_timer);
	return _timer;
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
}

FilesystemFactory *OSystem_Android::getFilesystemFactory() {
	return _fsFactory;
}

void OSystem_Android::addSysArchivesToSearchSet(Common::SearchSet &s,
												int priority) {
	ENTER("");

	JNI::addSysArchivesToSearchSet(s, priority);
}

void OSystem_Android::logMessage(LogMessageType::Type type, const char *message) {
	switch (type) {
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

#ifdef DYNAMIC_MODULES
void AndroidPluginProvider::addCustomDirectories(Common::FSList &dirs) const {
	((OSystem_Android *)g_system)->addPluginDirectories(dirs);
}
#endif

#endif

