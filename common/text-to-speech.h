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

#ifndef BACKENDS_TEXT_TO_SPEECH_ABSTRACT_H
#define BACKENDS_TEXT_TO_SPEECH_ABSTRACT_H

#include "common/scummsys.h"

#if defined(USE_TTS)

#include "common/array.h"
#include "common/debug.h"
namespace Common {

class TTSVoice {
	friend class TextToSpeechManager;

	public:
		enum Gender {
			MALE,
			FEMALE,
			UNKNOWN_GENDER
		};

		enum Age {
			CHILD,
			ADULT,
			UNKNOWN_AGE
		};

	public:
		TTSVoice()
			: _gender(UNKNOWN_GENDER)
			, _age(UNKNOWN_AGE)
			, _data(nullptr)
			, _description("") {}
		TTSVoice(Gender gender, Age age, void *data, String description) 
			: _gender(gender)
			, _age(age)
			, _data(data)
			, _description(description) {}
		Gender getGender() { return _gender; };
		void setGender(Gender gender) { _gender = gender; };
		Age getAge() { return _age; };
		void setAge(Age age) { _age = age; };
		void setData(void *data) { _data = data; };
		void *getData() { return _data; };
		String getDescription() { return _description; };

	protected:
		Gender _gender;
		Age _age;
		void *_data;
		String _description;
};

struct TTSState {
	int _rate;
	int _pitch;
	int _volume;
	String _language;
	int _activeVoice;
	Array<TTSVoice> _availaibleVoices;
	TTSState *_next;
};

/**
 * The TextToSpeechManager allows speech synthesis.
 *
 */
class TextToSpeechManager {
public:
	TextToSpeechManager();
	virtual ~TextToSpeechManager();

	virtual bool say(String str) { return false; }

	virtual bool stop() { return false; }
	virtual bool pause() { return false; }
	virtual bool resume() { return false; }

	virtual bool isSpeaking() { return false; }
	virtual bool isPaused() { return false; }
	virtual bool isReady() { return false; }
	
	virtual void setVoice(unsigned index) {}
	TTSVoice getVoice() { return _ttsState->_availaibleVoices[_ttsState->_activeVoice]; }

	virtual void setRate(int rate) {}
	int getRate() { return _ttsState->_rate; }

	virtual void setPitch(int pitch) {}
	int getPitch() { return _ttsState->_pitch; }

	virtual void setVolume(unsigned volume) {}
	virtual int getVolume() { return _ttsState->_volume; }

	virtual void setLanguage(String language) {}
	String getLanguage() { return _ttsState->_language; }

	Array<TTSVoice> getVoicesArray() { return _ttsState->_availaibleVoices; }

	void pushState();
	virtual bool popState() { return true; }

protected:
	TTSState *_ttsState;

	virtual void updateVoices() {};
};

} // End of namespace Common

#endif

#endif // BACKENDS_TEXT_TO_SPEECH_ABSTRACT_H
