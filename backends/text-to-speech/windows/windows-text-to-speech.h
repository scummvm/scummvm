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

#ifndef BACKENDS_TEXT_TO_SPEECH_WINDOWS_H
#define BACKENDS_TEXT_TO_SPEECH_WINDOWS_H

#include "common/scummsys.h"

#if defined(USE_TTS) && defined(WIN32)

#include "common/text-to-speech.h"
#include "common/str.h"
#include "common/ustr.h"
#include "common/list.h"


class WindowsTextToSpeechManager final : public Common::TextToSpeechManager {
public:
	enum SpeechState {
		READY,
		PAUSED,
		SPEAKING,
		BROKEN,
		NO_VOICE
	};

	struct SpeechParameters {
		Common::List<WCHAR *> *queue;
		SpeechState *state;
		HANDLE *mutex;
	};

	WindowsTextToSpeechManager();
	virtual ~WindowsTextToSpeechManager() override;

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

	virtual void freeVoiceData(void *data) override;

private:
	void init();
	virtual void updateVoices() override;
	void createVoice(void *cpVoiceToken);
	Common::String lcidToLocale(LCID locale);
	SpeechState _speechState;
	Common::String _lastSaid;
	HANDLE _thread;
	Common::List<WCHAR *> _speechQueue;
	SpeechParameters _threadParams;
	HANDLE _speechMutex;
};


#endif

#endif // BACKENDS_UPDATES_WINDOWS_H
