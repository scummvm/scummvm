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

#ifndef _ANDROID_JNI_H_
#define _ANDROID_JNI_H_

#include <jni.h>
#include <semaphore.h>
#include <pthread.h>

#include "common/fs.h"
#include "common/archive.h"
#include "common/array.h"
#include "common/ustr.h"
#include "engines/engine.h"

namespace Graphics {
	struct Surface;
}

class OSystem_Android;

class JNI {
private:
	JNI();
	virtual ~JNI();

public:
	static bool assets_updated;

	static bool pause;
	static sem_t pause_sem;

	static int surface_changeid;
	static int egl_surface_width;
	static int egl_surface_height;
	static int egl_bits_per_pixel;

	static bool virt_keyboard_state;

	static int32 gestures_insets[4];

	static jint onLoad(JavaVM *vm);

	static inline JNIEnv *getEnv() {
		JNIEnv *env = (JNIEnv*) pthread_getspecific(_env_tls);
		if (env != nullptr) {
			return env;
		}

		return fetchEnv();
	}

	static void attachThread();
	static void detachThread();

	static void setReadyForEvents(bool ready);
	static void wakeupForQuit();

	static void setWindowCaption(const Common::U32String &caption);

	/**
	 * Array members of DPIValues are xdpi, ydpi, density
	 **/
	typedef float DPIValues[3];
	static void getDPI(DPIValues &values);
	static void displayMessageOnOSD(const Common::U32String &msg);
	static bool openUrl(const Common::String &url);
	static bool hasTextInClipboard();
	static Common::U32String getTextFromClipboard();
	static bool setTextInClipboard(const Common::U32String &text);
	static bool isConnectionLimited();
	static void showVirtualKeyboard(bool enable);
	static void showOnScreenControls(int enableMask);
	static void setTouchMode(int touchMode);
	static int getTouchMode();
	static void setOrientation(int touchMode);
	static void addSysArchivesToSearchSet(Common::SearchSet &s, int priority);
	static Common::String getScummVMBasePath();
	static Common::String getScummVMConfigPath();
	static Common::String getScummVMLogPath();
	static jint getAndroidSDKVersionId();
	static void setCurrentGame(const Common::String &target);

	static inline bool haveSurface();
	static inline bool swapBuffers();
	static bool initSurface();
	static void deinitSurface();
	static int eglVersion() {
		if (_egl_version) {
			return _egl_version;
		}
		return fetchEGLVersion();
	}

	static void setAudioPause();
	static void setAudioPlay();
	static void setAudioStop();

	static inline int writeAudio(JNIEnv *env, jbyteArray &data, int offset,
									int size);

	static Common::Array<Common::String> getAllStorageLocations();

	static jobject getNewSAFTree(bool folder, bool writable, const Common::String &initURI, const Common::String &prompt);
	static Common::Array<jobject> getSAFTrees();
	static jobject findSAFTree(const Common::String &name);

private:
	static pthread_key_t _env_tls;

	static JavaVM *_vm;
	// back pointer to (java) peer instance
	static jobject _jobj;
	static jobject _jobj_audio_track;
	static jobject _jobj_egl;
	static jobject _jobj_egl_display;
	static jobject _jobj_egl_surface;
	// cached EGL version
	static int _egl_version;

	static Common::Archive *_asset_archive;
	static OSystem_Android *_system;

	static bool _ready_for_events;

	static jmethodID _MID_getDPI;
	static jmethodID _MID_displayMessageOnOSD;
	static jmethodID _MID_openUrl;
	static jmethodID _MID_hasTextInClipboard;
	static jmethodID _MID_getTextFromClipboard;
	static jmethodID _MID_setTextInClipboard;
	static jmethodID _MID_isConnectionLimited;
	static jmethodID _MID_setWindowCaption;
	static jmethodID _MID_showVirtualKeyboard;
	static jmethodID _MID_showOnScreenControls;
	static jmethodID _MID_setTouchMode;
	static jmethodID _MID_getTouchMode;
	static jmethodID _MID_setOrientation;
	static jmethodID _MID_getScummVMBasePath;
	static jmethodID _MID_getScummVMConfigPath;
	static jmethodID _MID_getScummVMLogPath;
	static jmethodID _MID_setCurrentGame;
	static jmethodID _MID_getSysArchives;
	static jmethodID _MID_getAllStorageLocations;
	static jmethodID _MID_initSurface;
	static jmethodID _MID_deinitSurface;
	static jmethodID _MID_eglVersion;
	static jmethodID _MID_getNewSAFTree;
	static jmethodID _MID_getSAFTrees;
	static jmethodID _MID_findSAFTree;

	static jmethodID _MID_EGL10_eglSwapBuffers;

	static jmethodID _MID_AudioTrack_flush;
	static jmethodID _MID_AudioTrack_pause;
	static jmethodID _MID_AudioTrack_play;
	static jmethodID _MID_AudioTrack_stop;
	static jmethodID _MID_AudioTrack_write;

	static const JNINativeMethod _natives[];

	static void throwByName(JNIEnv *env, const char *name, const char *msg);
	static void throwRuntimeException(JNIEnv *env, const char *msg);

	// natives for the dark side
	static void create(JNIEnv *env, jobject self, jobject asset_manager,
						jobject egl, jobject egl_display,
						jobject at, jint audio_sample_rate,
						jint audio_buffer_size,
						jboolean assets_updated_);
	static void destroy(JNIEnv *env, jobject self);

	static void setSurface(JNIEnv *env, jobject self, jint width, jint height, jint bpp);
	static jint main(JNIEnv *env, jobject self, jobjectArray args);

	static void pushEvent(JNIEnv *env, jobject self, int type, int arg1,
							int arg2, int arg3, int arg4, int arg5, int arg6);
	static void updateTouch(JNIEnv *env, jobject self, int action, int ptr, int x, int y);
	static void setupTouchMode(JNIEnv *env, jobject self, jint oldValue, jint newValue);
	static void syncVirtkeyboardState(JNIEnv *env, jobject self, jboolean newState);
	static void setPause(JNIEnv *env, jobject self, jboolean value);

	static void systemInsetsUpdated(JNIEnv *env, jobject self, jintArray gestureInsets, jintArray systemInsets, jintArray cutoutInsets);

	static jstring getNativeVersionInfo(JNIEnv *env, jobject self);
	static jstring convertToJString(JNIEnv *env, const Common::U32String &str);
	static Common::U32String convertFromJString(JNIEnv *env, const jstring &jstr);

	static PauseToken _pauseToken;

	static JNIEnv *fetchEnv();
	static int fetchEGLVersion();
};

inline bool JNI::haveSurface() {
	return _jobj_egl_surface != 0;
}

inline bool JNI::swapBuffers() {
	JNIEnv *env = JNI::getEnv();

	return env->CallBooleanMethod(_jobj_egl, _MID_EGL10_eglSwapBuffers,
									_jobj_egl_display, _jobj_egl_surface);
}

inline int JNI::writeAudio(JNIEnv *env, jbyteArray &data, int offset, int size) {
	return env->CallIntMethod(_jobj_audio_track, _MID_AudioTrack_write, data,
								offset, size);
}

#endif
