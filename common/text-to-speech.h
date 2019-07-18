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


/**
 * Text to speech voice class.
 */
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
		TTSVoice();

		TTSVoice(Gender gender, Age age, void *data, String description) ;

		TTSVoice(const TTSVoice& voice);

		~TTSVoice();

		TTSVoice& operator=(const TTSVoice& voice);

		/**
		 * Returns the gender of the used voice.
		 *
		 * @note The gender information is really platform specific, it may not be
		 * possible to find it out on some platforms. Sometimes it can be set by
		 * the user in the TTS engine configuration and so the information might be
		 * wrong
		 */
		Gender getGender() { return _gender; };

		/**
		 * Sets the voice gender, should probably be used only by the backends
		 * that are directly communicating with the TTS engine.
		 */
		void setGender(Gender gender) { _gender = gender; };

		/**
		 * Returns the age of the used voice.
		 *
		 * @note The age information is really platform specific, it may not be
		 * possible to find it out on some platforms. Sometimes it can be set by
		 * the user in the TTS engine configuration and so the information might be
		 * wrong
		 */
		Age getAge() { return _age; };

		/**
		 * Sets the voice age, should probably be used only by the backends
		 * that are directly communicating with the TTS engine.
		 */
		void setAge(Age age) { _age = age; };

		/**
		 * Returns the data about the voice, this is engine specific variable,
		 * it has close to no value for anything else then communicating with
		 * directly with the TTS engine, which should probably be done only by
		 * the backends.
		 */
		void setData(void *data) { _data = data; };

		/**
		 * Sets the voice age, should probably be used only by the backends
		 * that are directly communicating with the TTS engine.
		 */
		void *getData() { return _data; };

		/**
		 * Returns the voice description. This description is really tts engine
		 * specific and might be not be availaible with some tts engines.
		 */
		String getDescription() { return _description; };

	protected:
		Gender _gender;
		Age _age;
		void *_data;
		String _description;
		int *_refCount;
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
 */
class TextToSpeechManager {
public:
	/**
	 * The constructor sets the language to the translation manager language if 
	 * USE_TRANSLATION is defined, or english when it isn't defined. It sets the rate,
	 * pitch and volume to their middle values.
	 */
	TextToSpeechManager();
	virtual ~TextToSpeechManager();

	/**
	 * Says the given string
	 *
	 * @param str The string to say
	 */
	virtual bool say(String str, String charset = "") { return false; }

	/**
	 * Stops the speech
	 */
	virtual bool stop() { return false; }

	/**
	 * Pauses the speech
	 */
	virtual bool pause() { return false; }

	/**
	 * Resumes the speech
	 */
	virtual bool resume() { return false; }

	/**
	 * Returns true, if the TTS engine is speaking
	 */
	virtual bool isSpeaking() { return false; }

	/**
	 * Returns true, if the TTS engine is paused
	 */
	virtual bool isPaused() { return false; }

	/**
	 * Returns true, if the TTS engine is ready to speak (isn't speaking and isn't paused)
	 */
	virtual bool isReady() { return false; }
	
	/**
	 * Sets a voice to be used by the TTS.
	 *
	 * @param index The index of the voice inside the _ttsState->_availaibleVoices array
	 */
	virtual void setVoice(unsigned index) {}

	/**
	 * Returns the voice, that is used right now
	 */
	TTSVoice getVoice() { return _ttsState->_availaibleVoices[_ttsState->_activeVoice]; }

	/**
	 * Sets the speech rate
	 *
	 * @param rate Integer between -100 (slowest) and 100 (fastest)
	 */
	virtual void setRate(int rate) { _ttsState->_rate = rate; }

	/**
	 * Returns the current speech rate
	 */
	int getRate() { return _ttsState->_rate; }

	/**
	 * Sets the pitch
	 *
	 * @param pitch Integer between -100 (lowest) and 100 (highest)
	 */
	virtual void setPitch(int pitch) { _ttsState->_pitch = pitch; }

	/**
	 * Returns current speech pitch
	 */
	int getPitch() { return _ttsState->_pitch; }

	/**
	 * Sets the speech volume
	 *
	 * @param volume Volume as a percentage (0 means muted, 100 means as loud as possible)
	 */
	virtual void setVolume(unsigned volume) { _ttsState->_volume = volume; }

	/**
	 * Returns the current voice volume
	 */
	virtual int getVolume() { return _ttsState->_volume; }

	/**
	 * Sets the speech language
	 *
	 * @param language The language identifier as defined by ISO (2 characters long string)
	 *
	 * @note After using this method, it is probably a good idea to use setVoice,
	 * because voices are usually language specific and so it is set to some platform
	 * specific default after switching languages.
	 */
	virtual void setLanguage(String language) { _ttsState->_language = language; }

	/**
	 * Returns the current speech language
	 */
	String getLanguage() { return _ttsState->_language; }

	/**
	 * Returns array of availaible voices for the current language
	 */
	Array<TTSVoice> getVoicesArray() { return _ttsState->_availaibleVoices; }

	/**
	 * Pushes the current state of the TTS
	 */
	void pushState();

	/**
	 * Pops the TTS state
	 */
	virtual bool popState() { return true; }

	virtual void freeVoiceData(void *data) {}

protected:
	TTSState *_ttsState;

	virtual void updateVoices() {};
};

} // End of namespace Common

#endif

#endif // BACKENDS_TEXT_TO_SPEECH_ABSTRACT_H
