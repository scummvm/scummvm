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

#ifndef BACKENDS_TEXT_TO_SPEECH_ANDROID_H
#define BACKENDS_TEXT_TO_SPEECH_ANDROID_H

#include <jni.h>

#include "common/scummsys.h"

#if defined(USE_TTS)

#include "common/text-to-speech.h"
#include "common/str.h"
#include "common/ustr.h"
#include "common/list.h"


class AndroidTextToSpeechManager final : public Common::TextToSpeechManager {
public:
	enum SpeechState {
		BROKEN,
		READY,
		SPEAKING,
		PAUSED,
	};

	static void initJNI(JNIEnv *env);

	AndroidTextToSpeechManager();
	~AndroidTextToSpeechManager() override;

	bool say(const Common::U32String &str, Action action) override;

	bool stop() override;
	bool pause() override;
	bool resume() override;

	bool isSpeaking() override { return getState() == SPEAKING; }
	bool isPaused() override { return getState() == PAUSED; }
	bool isReady() override { return getState() == READY; }

	void setVoice(unsigned index) override;
	void freeVoiceData(void *data) override { delete (int *)data; }

	void setLanguage(Common::String language) override;

private:
	static void updateVoices(JNIEnv *env, jobject obj, jobjectArray voices);
	void updateVoices() override { setLanguage(_ttsState->_language); }

	int getState() const;

	jobject _tts;

	static jmethodID _MID_shutdown;
	static jmethodID _MID_getState;
	static jmethodID _MID_setLanguage;
	static jmethodID _MID_stop;
	static jmethodID _MID_pause;
	static jmethodID _MID_resume;
	static jmethodID _MID_say;

	static const JNINativeMethod _natives[];

	static bool _init;
};


#endif

#endif // BACKENDS_UPDATES_ANDROID_H
