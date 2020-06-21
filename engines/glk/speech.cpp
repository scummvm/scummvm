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

SpeechManager::SpeechManager() :
	_refCount(0)
#if defined(USE_TTS)
	, _ttsMan(nullptr), _lastSpeechSource(nullptr), _nextSpeechAction(Common::TextToSpeechManager::QUEUE)
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
		if (ConfMan.hasKey("mute", "scummvm") && ConfMan.getBool("mute", "scummvm"))
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

void SpeechManager::flushSpeech(Speech * source) {
#if defined(USE_TTS)
	if (source != _lastSpeechSource) {
		debugC(kDebugSpeech, "SpeechManager::flushSpeech() called with a different source than the last text source");
		purgeSpeech(source);
	} else {
		if (_ttsMan != nullptr && !_speechBuffer.empty()) {
			// Curently the TextToSpeechManager takes a String, which does not properly support
			// UTF-32. So convert to UTF-8.
			Common::String text = _speechBuffer.encode();
			debugC(kDebugSpeech, "Speaking: \"%s\"", text.c_str());
			_ttsMan->say(text, _nextSpeechAction, "utf-8");
			_speechBuffer.clear();
			_nextSpeechAction = Common::TextToSpeechManager::QUEUE;
		}
	}
#endif
}

void SpeechManager::purgeSpeech(Speech *source) {
#if defined(USE_TTS)
	debugC(kDebugSpeech, "SpeechManager::purgeSpeech()");
	if (_ttsMan != nullptr) {
		_speechBuffer.clear();
		_ttsMan->stop();
	}
	_nextSpeechAction = Common::TextToSpeechManager::QUEUE;
	_lastSpeechSource = source;
#endif
}

void SpeechManager::addSpeech(const uint32 *buf, size_t len, Speech *source) {
#if defined(USE_TTS)
	if (source != _lastSpeechSource) {
		debugC(kDebugSpeech, "Flushing SpeechManager buffer for a different speech source");
		if (!_speechBuffer.empty()) {
			// Flush the pending speech, but allow interupting it if we flush the text
			// for the new source before it has finished.
			flushSpeech(_lastSpeechSource);
			_nextSpeechAction = Common::TextToSpeechManager::INTERRUPT;
		}
		_lastSpeechSource = source;
	}
	if (_ttsMan != nullptr) {
		debugC(1, kDebugSpeech, "SpeechManager add speech");
		for (int i = 0 ; i < len ; ++i, ++buf) {
			// Should we automatically flush on new lines without waiting for the call to gli_tts_flush?
			// Should we also flush on '.', '?', and '!'?
			//if (*buf == '\n') {
			//	debugC(1, kDebugSpeech, "Flushing SpeechManager buffer on new line");
			//	gli_tts_flush();
			//} else {
			_speechBuffer += *buf;
			//}
		}
//		debugC(1, kDebugSpeech, "SpeechManager buffer: %s", _speechBuffer.encode().c_str());
	}
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
	if (_speechManager)
		_speechManager->flushSpeech(this);
}

void Speech::gli_tts_purge(void) {
	debugC(kDebugSpeech, "gli_tts_purge");
	if (_speechManager)
		_speechManager->purgeSpeech(this);
}

void Speech::gli_tts_speak(const uint32 *buf, size_t len) {
	debugC(1, kDebugSpeech, "gli_tts_speak");
	if (_speechManager)
		_speechManager->addSpeech(buf, len, this);
}

void Speech::gli_free_tts(void) {
	debugC(kDebugSpeech, "gli_free_tts");
	if (_speechManager) {
		_speechManager->releaseSpeechManagerInstance();
		_speechManager = nullptr;
	}
}

} // End of namespace Glk




