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

// Allow use of stuff in <time.h> and abort()
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_abort

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(USE_TTS)
#include "backends/text-to-speech/android/android-text-to-speech.h"

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

#include "common/ustr.h"

jmethodID AndroidTextToSpeechManager::_MID_shutdown = 0;
jmethodID AndroidTextToSpeechManager::_MID_getState = 0;
jmethodID AndroidTextToSpeechManager::_MID_setLanguage = 0;
jmethodID AndroidTextToSpeechManager::_MID_stop = 0;
jmethodID AndroidTextToSpeechManager::_MID_pause = 0;
jmethodID AndroidTextToSpeechManager::_MID_resume = 0;
jmethodID AndroidTextToSpeechManager::_MID_say = 0;

const JNINativeMethod AndroidTextToSpeechManager::_natives[] = {
	{ "updateVoices", "([Ljava/lang/String;)V",
		(void *)static_cast<void (*)(JNIEnv *, jobject, jobjectArray)>(AndroidTextToSpeechManager::updateVoices) },
};

bool AndroidTextToSpeechManager::_init = false;

void AndroidTextToSpeechManager::initJNI(JNIEnv *env) {
	if (_init) {
		return;
	}

	// We can't call error here as the backend is not built yet
#define FIND_METHOD(prefix, name, signature) do {                            \
    _MID_ ## prefix ## name = env->GetMethodID(cls, #name, signature);       \
        if (_MID_ ## prefix ## name == 0) {                                  \
            LOGE("Can't find method ID " #name);                             \
            abort();                                                         \
        }                                                                    \
    } while (0)

	jclass cls = env->FindClass("org/scummvm/scummvm/TextToSpeechManager");

	FIND_METHOD(, shutdown, "()V");
	FIND_METHOD(, getState, "()I");
	FIND_METHOD(, setLanguage, "(Ljava/lang/String;)V");
	FIND_METHOD(, stop, "()Z");
	FIND_METHOD(, pause, "()Z");
	FIND_METHOD(, resume, "()Z");
	FIND_METHOD(, say, "(Ljava/lang/String;IIIII)Z");

	if (env->RegisterNatives(cls, _natives, ARRAYSIZE(_natives)) < 0) {
		LOGE("Can't register natives for org/scummvm/scummvm/TextToSpeechManager");
		abort();
	}

	env->DeleteLocalRef(cls);

#undef FIND_FIELD
#undef FIND_METHOD

	_init = true;
}

AndroidTextToSpeechManager::AndroidTextToSpeechManager() : _tts(nullptr) {
	JNIEnv *env = JNI::getEnv();

	initJNI(env);

	jobject tts = JNI::getTTSManager();
	if (tts == nullptr) {
		return;
	}
	_tts = env->NewGlobalRef(tts);
	env->DeleteLocalRef(tts);
}

AndroidTextToSpeechManager::~AndroidTextToSpeechManager() {
	stop();
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_tts, _MID_shutdown);
	if (env->ExceptionCheck()) {
		LOGE("TextToSpeechManager::shutdown failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteGlobalRef(_tts);
	_tts = nullptr;

	clearState();
}

void AndroidTextToSpeechManager::updateVoices(JNIEnv *env, jobject obj, jobjectArray voices) {
	AndroidTextToSpeechManager *tts = (AndroidTextToSpeechManager *)g_system->getTextToSpeechManager();
	assert(tts);

	Common::String currentVoice;
	if (!tts->_ttsState->_availableVoices.empty())
		currentVoice = tts->_ttsState->_availableVoices[tts->_ttsState->_activeVoice].getDescription();
	int activeVoiceIndex = -1;

	tts->_ttsState->_availableVoices.clear();

	jsize size = env->GetArrayLength(voices);
	for (jsize i = 0; i < size; i++) {
		jstring name_obj   = (jstring)env->GetObjectArrayElement(voices, i);
		const char *name = env->GetStringUTFChars(name_obj, 0);
		if (name == nullptr) {
			env->DeleteLocalRef(name_obj);
			continue;
		}

		jsize *idx_p = new jsize;
		*idx_p = i;

		// Android doesn't provide any gender/age information on voices
		Common::TTSVoice voice(Common::TTSVoice::UNKNOWN_GENDER, Common::TTSVoice::UNKNOWN_AGE, idx_p, name);
		tts->_ttsState->_availableVoices.push_back(voice);

		if (name == currentVoice)
			activeVoiceIndex = i;

		env->ReleaseStringUTFChars(name_obj, name);
		env->DeleteLocalRef(name_obj);
	}

	if (activeVoiceIndex == -1 && size > 0) {
		activeVoiceIndex = 0;
	}
	if (activeVoiceIndex != -1) {
		tts->setVoice(activeVoiceIndex);
	}
}

void AndroidTextToSpeechManager::setVoice(unsigned index) {
	if (_ttsState->_availableVoices.empty())
		return;
	assert(index < _ttsState->_availableVoices.size());
	_ttsState->_activeVoice = index;
}

int AndroidTextToSpeechManager::getState() const {
	JNIEnv *env = JNI::getEnv();

	jint ret = env->CallIntMethod(_tts, _MID_getState);
	if (env->ExceptionCheck()) {
		LOGE("TextToSpeechManager::getState failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return BROKEN;
	}

	return ret;
}

void AndroidTextToSpeechManager::setLanguage(Common::String language) {
	Common::TextToSpeechManager::setLanguage(language);

	JNIEnv *env = JNI::getEnv();

	jstring language_obj = env->NewStringUTF(_ttsState->_language.c_str());

	env->CallVoidMethod(_tts, _MID_setLanguage, language_obj);

	env->DeleteLocalRef(language_obj);

	if (env->ExceptionCheck()) {
		LOGE("TextToSpeechManager::setLanguage failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

bool AndroidTextToSpeechManager::stop() {
	JNIEnv *env = JNI::getEnv();

	jboolean ret = env->CallBooleanMethod(_tts, _MID_stop);
	if (env->ExceptionCheck()) {
		LOGE("TextToSpeechManager::stop failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return false;
	}

	return ret;
}

bool AndroidTextToSpeechManager::pause() {
	JNIEnv *env = JNI::getEnv();

	jboolean ret = env->CallBooleanMethod(_tts, _MID_pause);
	if (env->ExceptionCheck()) {
		LOGE("TextToSpeechManager::pause failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return false;
	}

	return ret;
}

bool AndroidTextToSpeechManager::resume() {
	JNIEnv *env = JNI::getEnv();

	jboolean ret = env->CallBooleanMethod(_tts, _MID_resume);
	if (env->ExceptionCheck()) {
		LOGE("TextToSpeechManager::resume failed");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return false;
	}

	return ret;
}

bool AndroidTextToSpeechManager::say(const Common::U32String &str, Action action) {
	JNIEnv *env = JNI::getEnv();

	jstring str_obj = JNI::convertToJString(env, str);

	jsize voice = -1;
	if (!_ttsState->_availableVoices.empty()) {
		jsize *voice_p = (jsize *)_ttsState->_availableVoices[_ttsState->_activeVoice].getData();
		assert(voice_p != nullptr);
		voice = *voice_p;
	}

	jboolean ret = env->CallBooleanMethod(_tts, _MID_say, str_obj, (jint)action,
		_ttsState->_rate, _ttsState->_pitch,
		_ttsState->_volume, voice);

	env->DeleteLocalRef(str_obj);

	if (env->ExceptionCheck()) {
		LOGE("TextToSpeechManager::say failed");
		env->ExceptionDescribe();
		env->ExceptionClear();

		return false;
	}

	return ret;
}

#endif
