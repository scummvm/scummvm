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

#if defined(USE_TTS)
#include "common/config-manager.h"
#include "common/system.h"
#include "common/debug.h"
#include "glk/glk.h"
#endif

namespace Glk {

void Speech::gli_initialize_tts(void) {
#if defined(USE_TTS)
	debugC(kDebugSpeech, "gli_initialize_tts()");
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
	}
#endif
}

void Speech::gli_tts_flush(void) {
#if defined(USE_TTS)
	debugC(kDebugSpeech, "gli_tts_flush()");
	if (_ttsMan != nullptr && !_speechBuffer.empty()) {
		// Curently the TextToSpeechManager takes a String, which does not properly support
		// UTF-32. So convert to UTF-8.
		Common::String text = _speechBuffer.encode();
		debugC(kDebugSpeech, "Speaking: \"%s\"", text.c_str());
		_ttsMan->say(text, Common::TextToSpeechManager::QUEUE, "utf-8");
		_speechBuffer.clear();
	}
#endif
}

void Speech::gli_tts_purge(void) {
#if defined(USE_TTS)
	debugC(kDebugSpeech, "gli_tts_purge()");
	if (_ttsMan != nullptr) {
		_speechBuffer.clear();
		_ttsMan->stop();
	}
#endif
}

void Speech::gli_tts_speak(const uint32 *buf, size_t len) {
#if defined(USE_TTS)
	debugC(1, kDebugSpeech, "gli_tts_speak()");
	if (_ttsMan != nullptr) {
		for (int i = 0 ; i < len ; ++i, ++buf) {
			// Should we automatically flush on new lines without waiting for the call to gli_tts_flush?
			// Should we also flush on '.', '?', and '!'?
//			if (*buf == '\n') {
//				debugC(1, kDebugSpeech, "flush speech buffer on new line");
//				gli_tts_flush();
//			} else {
				_speechBuffer += *buf;
				debugC(1, kDebugSpeech, "speech buffer: %s", _speechBuffer.encode().c_str());
//			}
		}
	}
#endif
}

void Speech::gli_free_tts(void) {
#if defined(USE_TTS)
	debugC(kDebugSpeech, "gli_free_tts()");
	if (_ttsMan != nullptr) {
		_ttsMan->popState();
		_ttsMan = nullptr;
	}
#endif
}

} // End of namespace Glk




