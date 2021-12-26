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

#ifndef BACKENDS_TEXT_TO_SPEECH_LINUX_H
#define BACKENDS_TEXT_TO_SPEECH_LINUX_H

#include "common/scummsys.h"

#if defined(USE_TTS) && defined(USE_SPEECH_DISPATCHER) && defined(POSIX)

#include "common/text-to-speech.h"
#include "common/str.h"
#include "common/ustr.h"
#include "common/list.h"
#include "common/mutex.h"

#include <pthread.h>

struct StartSpeechParams {
	pthread_mutex_t *mutex;
	Common::List<Common::String> *speechQueue;
};

class SpeechDispatcherManager : public Common::TextToSpeechManager {
public:
	enum SpeechState {
		READY,
		PAUSED,
		SPEAKING,
		BROKEN
	};

	enum SpeechEvent {
		SPEECH_ENDED,
		SPEECH_PAUSED,
		SPEECH_CANCELED,
		SPEECH_RESUMED,
		SPEECH_BEGUN
	};

	SpeechDispatcherManager();
	~SpeechDispatcherManager() override;

	bool say(const Common::U32String &str, Action action) override;

	bool stop() override;
	bool pause() override;
	bool resume() override;

	bool isSpeaking() override;
	bool isPaused() override;
	bool isReady() override;

	void setVoice(unsigned index) override;
	void setRate(int rate) override;
	void setPitch(int pitch) override;
	void setVolume(unsigned volume) override;
	void setLanguage(Common::String language) override;

	void updateState(SpeechEvent event);

	void freeVoiceData(void *data) override;

private:
	void init();
	void updateVoices() override;
	void createVoice(int typeNumber, Common::TTSVoice::Gender, Common::TTSVoice::Age, char *description);
	Common::String strToUtf8(Common::String str, Common::String charset);
	static void *startSpeech(void *p);

	StartSpeechParams _params;
	SpeechState _speechState;
	Common::List<Common::String> _speechQueue;
	pthread_mutex_t _speechMutex;
	pthread_t _thread;
	bool _threadCreated;
};

#endif

#endif // BACKENDS_UPDATES_LINUX_H
