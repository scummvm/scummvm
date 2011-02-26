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

#include "base/main.h"
#include "common/config-manager.h"

#include "backends/platform/android/android.h"
#include "backends/platform/android/asset-archive.h"
#include "backends/platform/android/jni.h"

__attribute__ ((visibility("default")))
jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
	return JNI::onLoad(vm);
}

JavaVM *JNI::_vm = 0;
jobject JNI::_jobj = 0;

Common::Archive *JNI::_asset_archive = 0;
OSystem_Android *JNI::_system = 0;

jfieldID JNI::_FID_Event_type = 0;
jfieldID JNI::_FID_Event_synthetic = 0;
jfieldID JNI::_FID_Event_kbd_keycode = 0;
jfieldID JNI::_FID_Event_kbd_ascii = 0;
jfieldID JNI::_FID_Event_kbd_flags = 0;
jfieldID JNI::_FID_Event_mouse_x = 0;
jfieldID JNI::_FID_Event_mouse_y = 0;
jfieldID JNI::_FID_Event_mouse_relative = 0;
jfieldID JNI::_FID_ScummVM_nativeScummVM = 0;

jmethodID JNI::_MID_displayMessageOnOSD = 0;
jmethodID JNI::_MID_setWindowCaption = 0;
jmethodID JNI::_MID_initBackend = 0;
jmethodID JNI::_MID_audioSampleRate = 0;
jmethodID JNI::_MID_showVirtualKeyboard = 0;
jmethodID JNI::_MID_getSysArchives = 0;
jmethodID JNI::_MID_getPluginDirectories = 0;
jmethodID JNI::_MID_setupScummVMSurface = 0;
jmethodID JNI::_MID_destroyScummVMSurface = 0;
jmethodID JNI::_MID_swapBuffers = 0;

const JNINativeMethod JNI::_natives[] = {
	{ "create", "(Landroid/content/res/AssetManager;)V",
		(void *)JNI::create },
	{ "nativeDestroy", "()V",
		(void *)JNI::destroy },
	{ "scummVMMain", "([Ljava/lang/String;)I",
	 	(void *)JNI::main },
	{ "pushEvent", "(Lorg/inodes/gus/scummvm/Event;)V",
		(void *)JNI::pushEvent },
	{ "audioMixCallback", "([B)V",
		(void *)JNI::audioMixCallback },
	{ "setConfMan", "(Ljava/lang/String;I)V",
		(void *)JNI::setConfManInt },
	{ "setConfMan", "(Ljava/lang/String;Ljava/lang/String;)V",
		(void *)JNI::setConfManString },
	{ "enableZoning", "(Z)V",
		(void *)JNI::enableZoning },
	{ "setSurfaceSize", "(II)V",
		(void *)JNI::setSurfaceSize },
};

JNI::JNI() {
}

JNI::~JNI() {
}

jint JNI::onLoad(JavaVM *vm) {
	_vm = vm;

	JNIEnv *env;

	if (_vm->GetEnv((void **)&env, JNI_VERSION_1_2))
		return JNI_ERR;

	jclass cls = env->FindClass("org/inodes/gus/scummvm/ScummVM");
	if (cls == 0)
		return JNI_ERR;

	if (env->RegisterNatives(cls, _natives, ARRAYSIZE(_natives)) < 0)
		return JNI_ERR;

	_FID_ScummVM_nativeScummVM = env->GetFieldID(cls, "nativeScummVM", "J");
	if (_FID_ScummVM_nativeScummVM == 0)
		return JNI_ERR;

	jclass event = env->FindClass("org/inodes/gus/scummvm/Event");
	if (event == 0)
		return JNI_ERR;

	_FID_Event_type = env->GetFieldID(event, "type", "I");
	if (_FID_Event_type == 0)
		return JNI_ERR;

	_FID_Event_synthetic = env->GetFieldID(event, "synthetic", "Z");
	if (_FID_Event_synthetic == 0)
		return JNI_ERR;

	_FID_Event_kbd_keycode = env->GetFieldID(event, "kbd_keycode", "I");
	if (_FID_Event_kbd_keycode == 0)
		return JNI_ERR;

	_FID_Event_kbd_ascii = env->GetFieldID(event, "kbd_ascii", "I");
	if (_FID_Event_kbd_ascii == 0)
		return JNI_ERR;

	_FID_Event_kbd_flags = env->GetFieldID(event, "kbd_flags", "I");
	if (_FID_Event_kbd_flags == 0)
		return JNI_ERR;

	_FID_Event_mouse_x = env->GetFieldID(event, "mouse_x", "I");
	if (_FID_Event_mouse_x == 0)
		return JNI_ERR;

	_FID_Event_mouse_y = env->GetFieldID(event, "mouse_y", "I");
	if (_FID_Event_mouse_y == 0)
		return JNI_ERR;

	_FID_Event_mouse_relative = env->GetFieldID(event, "mouse_relative", "Z");
	if (_FID_Event_mouse_relative == 0)
		return JNI_ERR;

	return JNI_VERSION_1_2;
}

JNIEnv *JNI::getEnv() {
	JNIEnv *env = 0;

	jint res = _vm->GetEnv((void **)&env, JNI_VERSION_1_2);

	if (res != JNI_OK) {
		LOGE("GetEnv() failed: %d", res);
		abort();
	}

	return env;
}

void JNI::attachThread() {
	JNIEnv *env = 0;

	jint res = _vm->AttachCurrentThread(&env, 0);

	if (res != JNI_OK) {
		LOGE("AttachCurrentThread() failed: %d", res);
		abort();
	}
}

void JNI::detachThread() {
	jint res = _vm->DetachCurrentThread();

	if (res != JNI_OK) {
		LOGE("DetachCurrentThread() failed: %d", res);
		abort();
	}
}

void JNI::throwByName(JNIEnv *env, const char *name, const char *msg) {
	jclass cls = env->FindClass(name);

	// if cls is 0, an exception has already been thrown
	if (cls != 0)
		env->ThrowNew(cls, msg);

	env->DeleteLocalRef(cls);
}

// calls to the dark side

int JNI::getAudioSampleRate() {
	JNIEnv *env = JNI::getEnv();

	jint sample_rate = env->CallIntMethod(_jobj, _MID_audioSampleRate);

	if (env->ExceptionCheck()) {
		warning("Error finding audio sample rate - assuming 11025HZ");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return 11025;
	}

	return sample_rate;
}

void JNI::initBackend() {
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_jobj, _MID_initBackend);

	if (env->ExceptionCheck()) {
		error("Error in Java initBackend");

		env->ExceptionDescribe();
		env->ExceptionClear();

		// TODO now what?
	}
}

void JNI::getPluginDirectories(Common::FSList &dirs) {
	JNIEnv *env = JNI::getEnv();

	jobjectArray array =
		(jobjectArray)env->CallObjectMethod(_jobj, _MID_getPluginDirectories);

	if (env->ExceptionCheck()) {
		warning("Error finding plugin directories");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return;
	}

	jsize size = env->GetArrayLength(array);
	for (jsize i = 0; i < size; ++i) {
		jstring path_obj = (jstring)env->GetObjectArrayElement(array, i);

		if (path_obj == 0)
			continue;

		const char *path = env->GetStringUTFChars(path_obj, 0);

		if (path == 0) {
			warning("Error getting string characters from plugin directory");

			env->ExceptionClear();
			env->DeleteLocalRef(path_obj);

			continue;
		}

		dirs.push_back(Common::FSNode(path));

		env->ReleaseStringUTFChars(path_obj, path);
		env->DeleteLocalRef(path_obj);
	}
}

void JNI::setWindowCaption(const char *caption) {
	JNIEnv *env = JNI::getEnv();
	jstring java_caption = env->NewStringUTF(caption);

	env->CallVoidMethod(_jobj, _MID_setWindowCaption, java_caption);

	if (env->ExceptionCheck()) {
		warning("Failed to set window caption");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteLocalRef(java_caption);
}

void JNI::displayMessageOnOSD(const char *msg) {
	JNIEnv *env = JNI::getEnv();
	jstring java_msg = env->NewStringUTF(msg);

	env->CallVoidMethod(_jobj, _MID_displayMessageOnOSD, java_msg);

	if (env->ExceptionCheck()) {
		warning("Failed to display OSD message");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteLocalRef(java_msg);
}

void JNI::showVirtualKeyboard(bool enable) {
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_jobj, _MID_showVirtualKeyboard, enable);

	if (env->ExceptionCheck()) {
		error("Error trying to show virtual keyboard");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

void JNI::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	JNIEnv *env = JNI::getEnv();

	s.add("ASSET", _asset_archive, priority, false);

	jobjectArray array =
		(jobjectArray)env->CallObjectMethod(_jobj, _MID_getSysArchives);

	if (env->ExceptionCheck()) {
		warning("Error finding system archive path");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return;
	}

	jsize size = env->GetArrayLength(array);
	for (jsize i = 0; i < size; ++i) {
		jstring path_obj = (jstring)env->GetObjectArrayElement(array, i);
		const char *path = env->GetStringUTFChars(path_obj, 0);

		if (path != 0) {
			s.addDirectory(path, path, priority);
			env->ReleaseStringUTFChars(path_obj, path);
		}

		env->DeleteLocalRef(path_obj);
	}
}

// natives for the dark side

void JNI::create(JNIEnv *env, jobject self, jobject am) {
	assert(!_system);

	_asset_archive = new AndroidAssetArchive(am);
	assert(_asset_archive);

	_system = new OSystem_Android();
	assert(_system);

	// weak global ref to allow class to be unloaded
	// ... except dalvik implements NewWeakGlobalRef only on froyo
	//_jobj = env->NewWeakGlobalRef(self);
	_jobj = env->NewGlobalRef(self);

	jclass cls = env->GetObjectClass(_jobj);

#define FIND_METHOD(name, signature) do {							\
		_MID_ ## name = env->GetMethodID(cls, #name, signature);	\
		if (_MID_ ## name == 0)										\
			return;													\
	} while (0)

	FIND_METHOD(setWindowCaption, "(Ljava/lang/String;)V");
	FIND_METHOD(displayMessageOnOSD, "(Ljava/lang/String;)V");
	FIND_METHOD(initBackend, "()V");
	FIND_METHOD(audioSampleRate, "()I");
	FIND_METHOD(showVirtualKeyboard, "(Z)V");
	FIND_METHOD(getSysArchives, "()[Ljava/lang/String;");
	FIND_METHOD(getPluginDirectories, "()[Ljava/lang/String;");
	FIND_METHOD(setupScummVMSurface, "()V");
	FIND_METHOD(destroyScummVMSurface, "()V");
	FIND_METHOD(swapBuffers, "()Z");

#undef FIND_METHOD

	env->SetLongField(self, _FID_ScummVM_nativeScummVM, (jlong)_system);

	g_system = _system;
}

void JNI::destroy(JNIEnv *env, jobject self) {
	assert(_system);

	OSystem_Android *tmp = _system;
	g_system = 0;
	_system = 0;
	delete tmp;

	delete _asset_archive;
	_asset_archive = 0;

	// see above
	//JNI::getEnv()->DeleteWeakGlobalRef(_jobj);
	JNI::getEnv()->DeleteGlobalRef(_jobj);
}

jint JNI::main(JNIEnv *env, jobject self, jobjectArray args) {
	assert(_system);

	const int MAX_NARGS = 32;
	int res = -1;

	int argc = env->GetArrayLength(args);
	if (argc > MAX_NARGS) {
		throwByName(env, "java/lang/IllegalArgumentException",
					"too many arguments");
		return 0;
	}

	char *argv[MAX_NARGS];

	// note use in cleanup loop below
	int nargs;

	for (nargs = 0; nargs < argc; ++nargs) {
		jstring arg = (jstring)env->GetObjectArrayElement(args, nargs);

		if (arg == 0) {
			argv[nargs] = 0;
		} else {
			const char *cstr = env->GetStringUTFChars(arg, 0);

			argv[nargs] = const_cast<char *>(cstr);

			// exception already thrown?
			if (cstr == 0)
				goto cleanup;
		}

		env->DeleteLocalRef(arg);
	}

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new AndroidPluginProvider());
#endif

	LOGI("Entering scummvm_main with %d args", argc);

	res = scummvm_main(argc, argv);

	LOGI("Exiting scummvm_main");

	_system->quit();

cleanup:
	nargs--;

	for (int i = 0; i < nargs; ++i) {
		if (argv[i] == 0)
			continue;

		jstring arg = (jstring)env->GetObjectArrayElement(args, nargs);

		// Exception already thrown?
		if (arg == 0)
			return res;

		env->ReleaseStringUTFChars(arg, argv[i]);
		env->DeleteLocalRef(arg);
	}

	return res;
}

void JNI::pushEvent(JNIEnv *env, jobject self, jobject java_event) {
	assert(_system);

	Common::Event event;
	event.type = (Common::EventType)env->GetIntField(java_event,
														_FID_Event_type);

	event.synthetic =
		env->GetBooleanField(java_event, _FID_Event_synthetic);

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
		event.kbd.keycode = (Common::KeyCode)env->GetIntField(
			java_event, _FID_Event_kbd_keycode);
		event.kbd.ascii = static_cast<int>(env->GetIntField(
			java_event, _FID_Event_kbd_ascii));
		event.kbd.flags = static_cast<int>(env->GetIntField(
			java_event, _FID_Event_kbd_flags));
		break;
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP:
		event.mouse.x =
			env->GetIntField(java_event, _FID_Event_mouse_x);
		event.mouse.y =
			env->GetIntField(java_event, _FID_Event_mouse_y);
		// This is a terrible hack.	 We stash "relativeness"
		// in the kbd.flags field until pollEvent() can work
		// it out.
		event.kbd.flags = env->GetBooleanField(
			java_event, _FID_Event_mouse_relative) ? 1 : 0;
		break;
	default:
		break;
	}

	_system->pushEvent(event);
}

void JNI::audioMixCallback(JNIEnv *env, jobject self, jbyteArray jbuf) {
	assert(_system);

	jsize len = env->GetArrayLength(jbuf);
	jbyte *buf = env->GetByteArrayElements(jbuf, 0);

	if (buf == 0) {
		warning("Unable to get Java audio byte array. Skipping");
		return;
	}

	Audio::MixerImpl *mixer =
		static_cast<Audio::MixerImpl *>(_system->getMixer());
	assert(mixer);
	mixer->mixCallback(reinterpret_cast<byte *>(buf), len);

	env->ReleaseByteArrayElements(jbuf, buf, 0);
}

void JNI::setConfManInt(JNIEnv *env, jclass cls, jstring key_obj, jint value) {
	ENTER("%p, %d", key_obj, (int)value);

	const char *key = env->GetStringUTFChars(key_obj, 0);

	if (key == 0)
		return;

	ConfMan.setInt(key, value);

	env->ReleaseStringUTFChars(key_obj, key);
}

void JNI::setConfManString(JNIEnv *env, jclass cls, jstring key_obj,
							jstring value_obj) {
	ENTER("%p, %p", key_obj, value_obj);

	const char *key = env->GetStringUTFChars(key_obj, 0);

	if (key == 0)
		return;

	const char *value = env->GetStringUTFChars(value_obj, 0);

	if (value == 0) {
		env->ReleaseStringUTFChars(key_obj, key);
		return;
	}

	ConfMan.set(key, value);

	env->ReleaseStringUTFChars(value_obj, value);
	env->ReleaseStringUTFChars(key_obj, key);
}

void JNI::enableZoning(JNIEnv *env, jobject self, jboolean enable) {
	assert(_system);

	_system->enableZoning(enable);
}

void JNI::setSurfaceSize(JNIEnv *env, jobject self, jint width, jint height) {
	assert(_system);

	_system->setSurfaceSize(width, height);
}

#endif

