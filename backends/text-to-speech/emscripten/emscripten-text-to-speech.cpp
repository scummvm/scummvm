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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#include "backends/text-to-speech/emscripten/emscripten-text-to-speech.h"

#if defined(USE_TTS) && defined(EMSCRIPTEN)
#include <emscripten.h>

#include "common/config-manager.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/ustr.h"
#include "common/debug.h"

EM_JS(void, ttsInit, (), {
	/* 
	* Voices can come from the browser, the operating system or cloud services. This means we sometimes get
	* an incomplete or empty list on first call getVoices().
	* Best practice is to listen to the 'voiceschanged' event and update the list of voices when it fires.
	*/
	globalThis['ttsVoiceMap'] = {};
	globalThis['ttsUtteranceQueue'] = [];
	const refreshVoices = () => {
		globalThis['ttsVoiceMap'] = {};
		var cnt = 0;
		voices = window.speechSynthesis.getVoices();
		Array.from(voices).forEach((voice) => {
			if (!(voice.lang in globalThis['ttsVoiceMap'])) {
				globalThis['ttsVoiceMap'][voice.lang] = {};
			}
			globalThis['ttsVoiceMap'][voice.lang][voice.name] = voice;
			cnt++;
		});
		console.log("Found %d voices",cnt);
	};

	if ('onvoiceschanged' in speechSynthesis) {
		speechSynthesis.onvoiceschanged = refreshVoices;
	} 
	refreshVoices();
});

EmscriptenTextToSpeechManager::EmscriptenTextToSpeechManager() {
	ttsInit();
#ifdef USE_TRANSLATION
	setLanguage(TransMan.getCurrentLanguage());
#else
	setLanguage("en");
#endif
}

EmscriptenTextToSpeechManager::~EmscriptenTextToSpeechManager() {
	stop();
}


EM_JS(bool, _ttsSay, (const char *text, const char *voice_name, const char *voice_lang, int pitch, int rate, int volume, int action), {
	voice_name = UTF8ToString(voice_name);
	voice_lang = UTF8ToString(voice_lang);
	if (!(voice_lang in globalThis['ttsVoiceMap'] && voice_name in globalThis['ttsVoiceMap'][voice_lang])){
		console.error("_ttsSay: Voice not found");
		return false;
	}
	text = UTF8ToString(text);
	if (text === "") {
		return false;
	}
	/* 
	* Possible actions are:
	*	INTERRUPT - interrupts the current speech
	*	INTERRUPT_NO_REPEAT - interrupts the speech (deletes the whole queue),
	*		if the str is the same as the string currently being said,
	*		it lets the current string finish.
	*	QUEUE - queues the speech
	*	QUEUE_NO_REPEAT - queues the speech only if the str is different than
	*		the last string in the queue (or the string, that is currently
	*		being said if the queue is empty)
	*	DROP - does nothing if there is anything being said at the moment
	*/
	const Actions = Object.freeze({
		INTERRUPT: 0,
		INTERRUPT_NO_REPEAT: 1,
		QUEUE: 2,
		QUEUE_NO_REPEAT: 3
	});
	console.assert(action <= 3,"_ttsSay: Illegal Action: %d",action);// DROP is handled on the native side so we should only have 0-3.

	if (action == Actions.QUEUE_NO_REPEAT && 
			globalThis['ttsUtteranceQueue'].length > 0 && globalThis['ttsUtteranceQueue'][globalThis['ttsUtteranceQueue'].length-1].text == text) {
		console.debug("_ttsSay: Skipping duplicate utterance (QUEUE_NO_REPEAT)");
		return false;
	}
	//  INTERRUPT_NO_REPEAT with a matching string - empty queue but let the current string finish
	if (action == Actions.INTERRUPT_NO_REPEAT && globalThis['ttsUtteranceQueue'].length > 0 && globalThis['ttsUtteranceQueue'][0].text == text){
		globalThis['ttsUtteranceQueue'] = globalThis['ttsUtteranceQueue'].slice(0,1);
		return false;
	}
	// interrupt or INTERRUPT_NO_REPEAT with a non-matching string (or no string talking) - empty queue, cancel all talking
	if (action == Actions.INTERRUPT || action == Actions.INTERRUPT_NO_REPEAT ) {
		globalThis['ttsUtteranceQueue'] = [];//globalThis['ttsUtteranceQueue'].slice(0,1);
		window.speechSynthesis.cancel();
		
	}
	// queue and speak next utterance
	voice = globalThis['ttsVoiceMap'][voice_lang][voice_name];
	const utterance = new SpeechSynthesisUtterance(text);
	utterance.onend = function(event) { // this is triggered when an utterance completes speaking 
		if (globalThis['ttsUtteranceQueue'][0] == event.target){
			globalThis['ttsUtteranceQueue'].shift(); //remove utterance that was just spoken
		}
		if (globalThis['ttsUtteranceQueue'].length > 0 && !window.speechSynthesis.speaking){ // speak next utterance if nothing is being spoken
			window.speechSynthesis.speak(globalThis['ttsUtteranceQueue'][0]);
		}
	};
	utterance.onerror = function(event) { // this includes canceled utterances (so not just errors)
		if (globalThis['ttsUtteranceQueue'][0] == event.target){
			globalThis['ttsUtteranceQueue'].shift(); //remove utterance that was just spoken
		}
		if (globalThis['ttsUtteranceQueue'].length > 0 && !window.speechSynthesis.speaking){ // speak next utterance if nothing is being spoken
			window.speechSynthesis.speak(globalThis['ttsUtteranceQueue'][0]);
		}
	};
	/* 
	 * TODO: we could do INTERRUPT_NO_REPEAT and INTERRUPT handling on boundaries, but it's not reliable
	 * 		 remote voices don't have onboundary event: https://issues.chromium.org/issues/41195426
     * 
	 * 	utterance.onboundary = function(event){
	 *	    console.log(event);
	 *	};
	*/
	utterance.voice = voice;
	utterance.volume = volume / 100; // linearly adjust 0 to 100 -> 0 to 1
	utterance.pitch = (pitch + 100) / 100; // linearly adjust -100 to 100 (0 default) -> 0 to 2 (1 default)
	utterance.rate = rate > 0 ? 1 + (rate / (100 - 9)) : 0.1 + (rate + 100) / (100 / 0.9); // linearly adjust -100 to 100 (0 default)  -> 0.1 to 10 (1 default)
	
	console.debug("Pushing to queue: %s",text);
	globalThis['ttsUtteranceQueue'].push(utterance);
	if (globalThis['ttsUtteranceQueue'].length == 1){
		console.debug("Speaking %s",text);
		window.speechSynthesis.speak(utterance);
	}
	return true;
});

bool EmscriptenTextToSpeechManager::say(const Common::U32String &str, Action action) {
	assert(_ttsState->_enabled);

	Common::String strUtf8 = str.encode();
	debug(5, "Saying %s (%d)", strUtf8.c_str(), action);

	if (isSpeaking() && action == DROP) {
		debug(5, "EmscriptenTextToSpeechManager::say - Not saying '%s' as action=DROP and already speaking", strUtf8.c_str());
		return true;
	}

	char *voice_name = ((char **)_ttsState->_availableVoices[_ttsState->_activeVoice].getData())[0];
	char *voice_lang = ((char **)_ttsState->_availableVoices[_ttsState->_activeVoice].getData())[1];
	return _ttsSay(strUtf8.c_str(), voice_name, voice_lang, _ttsState->_pitch, _ttsState->_rate, _ttsState->_volume, action);
}

EM_JS(char **, _ttsGetVoices, (), {
	voices = Array.from(Object.values(globalThis['ttsVoiceMap'])).map(Object.values).flat() // flatten voice map
		.sort((a,b) => a.default ===  b.default ? a.name.localeCompare(b.name):a.default?-1:1) // first default, then alphabetically
		.map(voice=>[voice.name,voice.lang])
		.flat();
	voices.push(""); // we need this to find the end of the array on the native side.

	// convert the strings to C strings
	var c_strings = voices.map((s) => {
		var size = lengthBytesUTF8(s) + 1;
		var ret = Module._malloc(size);
		stringToUTF8Array(s, HEAP8, ret, size);
		return ret;
	});

	var ret_arr = Module._malloc(c_strings.length * 4); // 4-bytes per pointer
	c_strings.forEach((ptr, i) => { Module.setValue(ret_arr + i * 4, ptr, "i32"); }); // populate return array
	return ret_arr;
});

void EmscriptenTextToSpeechManager::updateVoices() {
	_ttsState->_availableVoices.clear();
	char **ttsVoices = _ttsGetVoices();
	char **iter = ttsVoices;
	Common::Array<char *> names;
	while (strcmp(*iter, "") != 0) {
		char *c_name = *iter++;
		char *c_lang = *iter++;
		Common::String language = Common::String(c_lang);
		if (_ttsState->_language == language.substr(0, 2)) {
			int idx = -1;
			for (int i = 0; i < names.size(); i++) {
				if (strcmp(names[i], c_name) == 0) {
					idx = i;
					break;
				}
			}
			names.push_back(c_name);
			Common::String name;
			// some systems have the same voice multiple times for the same language (e.g. en-US and en-GB),
			// in that case we should add the locale to the name
			if (idx == -1) {
				name = Common::String(c_name);
			} else {
				name = Common::String::format("%s (%s)", c_name, language.substr(3, 2).c_str());
				// some systems have identical name/language/locale pairs multiple times (seems a bug), we just skip that case (e.g. macOS Safari for "Samantha (en_US)" )
				char *other_name = ((char **)_ttsState->_availableVoices[idx].getData())[0];
				char *other_lang = ((char **)_ttsState->_availableVoices[idx].getData())[1];
				Common::String other_new = Common::String::format("%s (%s)", other_name, Common::String(other_lang).substr(3, 2).c_str());
				if (other_new == name) {
					warning("Skipping duplicate voice %s %s", c_name, c_lang);
					continue;
				} else {
					warning("Adding duplicate voice %s %s", _ttsState->_availableVoices[idx].getDescription().c_str(), name.c_str());
					_ttsState->_availableVoices[idx].setDescription(other_new);
				}
			}
			char **data_p = new char *[] { c_name, c_lang };
			Common::TTSVoice voice(Common::TTSVoice::UNKNOWN_GENDER, Common::TTSVoice::UNKNOWN_AGE, (void *)data_p, name);
			_ttsState->_availableVoices.push_back(voice);
		}
	}
	free(ttsVoices);

	if (_ttsState->_availableVoices.empty()) {
		warning("No voice is available for language: %s", _ttsState->_language.c_str());
	}
}

EM_JS(void, _ttsStop, (), {
	window.speechSynthesis.cancel();
});

bool EmscriptenTextToSpeechManager::stop() {
	_ttsStop();
	return true;
}

EM_ASYNC_JS(void, _ttsPause, (), {
	if(window.speechSynthesis.paused){
	} else if(window.speechSynthesis.speaking && globalThis['ttsUtteranceQueue'].length > 0){
		// browsers don't pause immediately, so we have to wait for the pause event if there's something being spoken
		await (async () => {
			return new Promise((resolve, reject) => {
				setTimeout(() => { resolve(); }, 300);
				globalThis['ttsUtteranceQueue'][0].onpause = (event) =>{ resolve(event)};
				window.speechSynthesis.pause();
			});
		})();
	} else {
		assert(globalThis['ttsUtteranceQueue'].length == 0);
		window.speechSynthesis.pause();
	}
	return;
});

bool EmscriptenTextToSpeechManager::pause() {
	if (isPaused())
		return false;
	_ttsPause();
	return true;
}

EM_JS(void, _ttsResume, (), {
	window.speechSynthesis.resume();
});

bool EmscriptenTextToSpeechManager::resume() {
	if (!isPaused())
		return false;
	_ttsResume();
	return true;
}

EM_JS(bool, _ttsIsSpeaking, (), {
	return window.speechSynthesis.speaking;
});

bool EmscriptenTextToSpeechManager::isSpeaking() {
	return _ttsIsSpeaking();
}

EM_JS(bool, _ttsIsPaused, (), {
	console.debug("_ttsIsPaused: Checking if speech synthesis is paused %s",window.speechSynthesis.paused ? "true" : "false");
	return window.speechSynthesis.paused;
});

bool EmscriptenTextToSpeechManager::isPaused() {
	return _ttsIsPaused();
}

bool EmscriptenTextToSpeechManager::isReady() {
	if (_ttsState->_availableVoices.empty())
		return false;
	if (!isPaused() && !isSpeaking())
		return true;
	else
		return false;
}

void EmscriptenTextToSpeechManager::setVoice(unsigned index) {
	assert(!_ttsState->_enabled || index < _ttsState->_availableVoices.size());
	_ttsState->_activeVoice = index;
	return;
}

void EmscriptenTextToSpeechManager::setRate(int rate) {
	assert(rate >= -100 && rate <= 100);
	_ttsState->_rate = rate;
}

void EmscriptenTextToSpeechManager::setPitch(int pitch) {
	assert(pitch >= -100 && pitch <= 100);
	_ttsState->_pitch = pitch;
}

void EmscriptenTextToSpeechManager::setVolume(unsigned volume) {
	assert(volume <= 100);
	_ttsState->_volume = volume;
}

void EmscriptenTextToSpeechManager::setLanguage(Common::String language) {
	debug(5, "EmscriptenTextToSpeechManager::setLanguage to %s", language.c_str());
	if (_ttsState->_language != language.substr(0, 2) || _ttsState->_availableVoices.empty()) {
		debug(5, "EmscriptenTextToSpeechManager::setLanguage - Update voices");
		updateVoices();
		setVoice(0);
	}
	Common::TextToSpeechManager::setLanguage(language);
}

void EmscriptenTextToSpeechManager::freeVoiceData(void *data) {
	free(((char **)data)[0]);
	free(((char **)data)[1]);
	free(data);
}

#endif
