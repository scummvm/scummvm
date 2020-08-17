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
	virtual ~SpeechDispatcherManager() override;

	virtual bool say(const Common::U32String &str, Action action) override;

	virtual bool stop() override;
	virtual bool pause() override;
	virtual bool resume() override;

	virtual bool isSpeaking() override;
	virtual bool isPaused() override;
	virtual bool isReady() override;

	virtual void setVoice(unsigned index) override;
	virtual void setRate(int rate) override;
	virtual void setPitch(int pitch) override;
	virtual void setVolume(unsigned volume) override;
	virtual void setLanguage(Common::String language) override;

	void updateState(SpeechEvent event);

	virtual void freeVoiceData(void *data) override;

private:
	void init();
	virtual void updateVoices() override;
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
