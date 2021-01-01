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
#include "common/str.h"

#if defined(USE_TTS)

#include "common/array.h"
namespace Common {


/**
 * @defgroup common_text_speech Text-to-speech Manager
 * @ingroup common
 *
 * @brief The TTS module allows for speech synthesis.
 *
 * @{
 */

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
		 * it has close to no value for anything else then communicating
		 * directly with the TTS engine, which should probably be done only by
		 * the backends.
		 */
		void *getData() { return _data; };

		/**
		 * Sets the voice data, should probably be used only by the backends
		 * that are directly communicating with the TTS engine.
		 */
		void setData(void *data) { _data = data; };

		/**
		 * Returns the voice description. This description is really tts engine
		 * specific and might be not be available with some tts engines.
		 */
		String getDescription() { return _description; };

	protected:
		Gender _gender; ///< Gender of the voice
		Age _age; ///< Age of the voice
		void *_data; ///< Pointer to tts engine specific data about the voice
		String _description; ///< Description of the voice (gets displayed in GUI)
		int *_refCount; ///< Reference count (serves for proper feeing of _data)
};

struct TTSState {
	int _rate;
	int _pitch;
	int _volume;
	String _language;
	int _activeVoice;
	Array<TTSVoice> _availableVoices;
	TTSState *_next;
};

/**
 * The TextToSpeechManager allows speech synthesis.
 */
class TextToSpeechManager {
public:
	enum Action {
		INTERRUPT,
		INTERRUPT_NO_REPEAT,
		QUEUE,
		QUEUE_NO_REPEAT,
		DROP
	};
	/**
	 * The constructor sets the language to the translation manager language if
	 * USE_TRANSLATION is defined, or english when it isn't defined. It sets the rate,
	 * pitch and volume to their middle values.
	 */
	TextToSpeechManager();
	virtual ~TextToSpeechManager() {}

	/**
	 * Says the given string
	 *
	 * @param str The string to say
	 * @param action What to do if another string is just being said.
	 * Possible actions are:
	 *		INTERRUPT - interrupts the current speech
	 *		INTERRUPT_NO_REPEAT - interrupts the speech (deletes the whole queue),
	 *			if the str is the same as the string currently being said,
	 *			it lets the current string finish.
	 *		QUEUE - queues the speech
	 *		QUEUE_NO_REPEAT - queues the speech only if the str is different than
	 *			the last string in the queue (or the string, that is currently
	 *			being said if the queue is empty)
	 *		DROP - does nothing if there is anything being said at the moment
	 */
	virtual bool say(const U32String &str, Action action) { return false; }

	/**
	 * Says the given string, but strings can have a custom charset here.
	 * It will convert to UTF-32 before passing along to the intended method.
	 */
	bool say(const String &str, Action action, CodePage charset = kUtf8) {
		U32String textToSpeak(str, charset);
		return say(textToSpeak, action);
	}

	/**
	 * Interrupts what's being said and says the given string
	 *
	 * @param str The string to say
	 */
	bool say(const U32String &str) { return say(str, INTERRUPT_NO_REPEAT); }

	/**
	 * Interrupts what's being said and says the given string
	 *
	 * @param str The string to say
	 * @param charset The encoding of the string. It will be converted to UTF-32.
	 *	              It will use UTF-8 by default.
	 */
	bool say(const String &str, CodePage charset = kUtf8) {
		return say(str, INTERRUPT_NO_REPEAT, charset);
	}

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
	 *
	 * @note On linux, the speech resumes from the begining of the last speech being
	 * said, when pause() was called. On other platforms the speech resumes from
	 * exactly where it was paused();
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
	 * @param index The index of the voice inside the _ttsState->_availableVoices array
	 */
	virtual void setVoice(unsigned index) {}

	/**
	 * Returns the voice, that is used right now
	 */
	TTSVoice getVoice();

	/**
	 * Sets the speech rate
	 *
	 * @param rate Integer between -100 (slowest) and 100 (fastest), 0 is the default
	 */
	virtual void setRate(int rate) { _ttsState->_rate = rate; }

	/**
	 * Returns the current speech rate
	 */
	int getRate() { return _ttsState->_rate; }

	/**
	 * Sets the pitch
	 *
	 * @param pitch Integer between -100 (lowest) and 100 (highest), 0 is the default
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
	int getVolume() { return _ttsState->_volume; }

	/**
	 * Sets the speech language
	 *
	 * @param language The language identifier as defined by ISO 639-1
	 *
	 * @note After using this method, it is probably a good idea to use setVoice,
	 * because voices are usually language specific and so it is set to some platform
	 * specific default after switching languages.
	 */
	virtual void setLanguage(String language);

	/**
	 * Returns the current speech language
	 */
	String getLanguage() { return _ttsState->_language; }

	/**
	 * Returns array of available voices for the current language
	 */
	Array<TTSVoice> getVoicesArray() { return _ttsState->_availableVoices; }

	/**
	 * Returns array of indices of voices from the _availableVoices array, which
	 * have the needed gender.
	 *
	 * @param gender Gender, which indices should be returned
	 *
	 * @return Array of indices into _availableVoices
	 */
	Array<int> getVoiceIndicesByGender (TTSVoice::Gender gender);

	/**
	 * returns the index for the default voice.
	 */
	virtual int getDefaultVoice() { return 0; }

	/**
	 * Pushes the current state of the TTS
	 */
	void pushState();

	/**
	 * Pops the TTS state
	 */
	bool popState();

	/**
	 * Frees the _data field from TTSVoice
	 */
	virtual void freeVoiceData(void *data) {}

protected:
	TTSState *_ttsState;

	void clearState();
	virtual void updateVoices() {};
};

/** @} */

} // End of namespace Common

#endif	// USE_TTS

#endif // BACKENDS_TEXT_TO_SPEECH_ABSTRACT_H
