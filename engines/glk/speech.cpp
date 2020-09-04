/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/speech.h"
#include "common/debug.h"
#include "glk/glk.h"

#if defined(USE_TTS)
#include "common/config-manager.h"
#include "common/system.h"
#endif

namespace Glk {

SpeechManager *SpeechManager::_instance = nullptr;

SpeechManager* SpeechManager::getSpeechManagerInstance() {
#if defined(USE_TTS)
	if (!_instance)
		_instance = new SpeechManager();
	++_instance->_refCount;
	return _instance;
#else
	return nullptr;
#endif
}

void SpeechManager::releaseSpeechManagerInstance() {
	if (--_refCount == 0) {
		_instance = nullptr;
		delete this;
	}
}

void SpeechManager::syncSoundSettings() {
#if defined(USE_TTS)
	debugC(kDebugSpeech, "SpeechManager::syncSoundSettings");
	if (_instance && _instance->_ttsMan) {
		int volume = (ConfMan.getInt("speech_volume") * 100) / 256;
		if (ConfMan.hasKey("mute") && ConfMan.getBool("mute"))
			volume = 0;
		debugC(kDebugSpeech, "Set speech volume to %d", volume);
		_instance->_ttsMan->setVolume(volume);
	}
#endif
}

SpeechManager::SpeechManager() :
	_refCount(0)
#if defined(USE_TTS)
	, _ttsMan(nullptr), _lastSpeechSource(nullptr)
#endif
{
#if defined(USE_TTS)
	debugC(kDebugSpeech, "Initialize Glk::SpeechManager");
	if (_ttsMan != nullptr)
		return;
	_ttsMan = g_system->getTextToSpeechManager();
	if (_ttsMan != nullptr) {
		_ttsMan->pushState();
		// Language
		_ttsMan->setLanguage(ConfMan.get("language"));
		// Volume
		int volume = (ConfMan.getInt("speech_volume") * 100) / 256;
		if (ConfMan.hasKey("mute") && ConfMan.getBool("mute"))
			volume = 0;
		_ttsMan->setVolume(volume);
		// Voice
		unsigned voice;
		if(ConfMan.hasKey("tts_voice")) {
			voice = ConfMan.getInt("tts_voice");
			if (voice >= _ttsMan->getVoicesArray().size())
				voice = _ttsMan->getDefaultVoice();
		} else
			voice = _ttsMan->getDefaultVoice();
		_ttsMan->setVoice(voice);
	} else
		debugC(kDebugSpeech, "Text to Speech is not available");
#endif
}

SpeechManager::~SpeechManager() {
#if defined(USE_TTS)
	debugC(kDebugSpeech, "Destroy Glk::SpeechManager");
	if (_ttsMan != nullptr) {
		_ttsMan->popState();
		_ttsMan = nullptr;
	}
#endif
}

void SpeechManager::speak(const Common::U32String &text, Speech *speechSource) {
#if defined(USE_TTS)
	if (_ttsMan != nullptr) {
		// If the previous speech is from a different source, interrupt it.
		// Otherwise queeue the speech.
		Common::TextToSpeechManager::Action speechAction = Common::TextToSpeechManager::QUEUE;
		if (speechSource != _lastSpeechSource) {
			debugC(kDebugSpeech, "Changing speack text source.");
			// Should we interrupt the text from the other source?
			// Just queueing the text seems to provide a better experience.
			//speechAction = Common::TextToSpeechManager::INTERRUPT;
			_lastSpeechSource = speechSource;
		}
		//debugC(kDebugSpeech, "Speaking: \"%s\"", text.encode().c_str());
		_ttsMan->say(text, speechAction);
	}
#endif
}

void SpeechManager::stopSpeech(Speech *speechSource) {
#if defined(USE_TTS)
	debugC(kDebugSpeech, "SpeechManager::stopSpeech()");
	// Should we only interrupt the speech if it is from the given speech source.
	// If we do that we probably want to change speak to interrupt the speech when
	// called with a different speech source as the current one.
	if (_ttsMan != nullptr)
		_ttsMan->stop();
#endif
}

Speech::Speech() : _speechManager(nullptr) {
}

Speech::~Speech() {
	if (_speechManager) {
		warning("Unbalanced calls to gli_initialize_tts and gli_free_tts");
		_speechManager->releaseSpeechManagerInstance();
	}
}

void Speech::gli_initialize_tts(void) {
	debugC(kDebugSpeech, "gli_initialize_tts");
	if (!_speechManager)
		_speechManager = SpeechManager::getSpeechManagerInstance();
}

void Speech::gli_tts_flush(void) {
	debugC(kDebugSpeech, "gli_tts_flush");
	if (_speechManager && !_speechBuffer.empty())
		_speechManager->speak(_speechBuffer, this);
	_speechBuffer.clear();
}

void Speech::gli_tts_purge(void) {
	debugC(kDebugSpeech, "gli_tts_purge");
	if (_speechManager) {
		_speechBuffer.clear();
		_speechManager->stopSpeech(this);
	}
}

void Speech::gli_tts_speak(const uint32 *buf, size_t len) {
	debugC(1, kDebugSpeech, "gli_tts_speak(const uint32 *, size_t)");
	if (_speechManager) {
		for (uint i = 0 ; i < len ; ++i, ++buf) {
			// Should we automatically flush on new lines without waiting for the call to gli_tts_flush?
			// Should we also flush on '.', '?', and '!'?
			//if (*buf == '\n') {
			//	debugC(1, kDebugSpeech, "Flushing SpeechManager buffer on new line");
			//	gli_tts_flush();
			//} else {
			_speechBuffer += *buf;
			//}
		}
		//debugC(1, kDebugSpeech, "SpeechManager buffer: %s", _speechBuffer.encode().c_str());
	}
}

void Speech::gli_tts_speak(const char *buf, size_t len) {
	debugC(1, kDebugSpeech, "gli_tts_speak(const char *, size_t)");
	if (_speechManager) {
		for (uint i = 0 ; i < len ; ++i, ++buf) {
			// Should we automatically flush on new lines without waiting for the call to gli_tts_flush?
			// Should we also flush on '.', '?', and '!'?
			//if (*buf == '\n') {
			//	debugC(1, kDebugSpeech, "Flushing SpeechManager buffer on new line");
			//	gli_tts_flush();
			//} else {
			_speechBuffer += (uint32)*buf;
			//}
		}
		//debugC(1, kDebugSpeech, "SpeechManager buffer: %s", _speechBuffer.encode().c_str());
	}
}

void Speech::gli_free_tts(void) {
	debugC(kDebugSpeech, "gli_free_tts");
	if (_speechManager) {
		_speechManager->releaseSpeechManagerInstance();
		_speechManager = nullptr;
	}
}

} // End of namespace Glk




