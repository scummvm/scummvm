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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/tts.h"
#include "common/system.h"
#include "common/text-to-speech.h"
#include "common/config-manager.h"

namespace Sci {

SciTTS::SciTTS() : _curMessage("") {
	_ttsMan = ConfMan.getBool("tts_enabled") ? g_system->getTextToSpeechManager() : nullptr;
	if (_ttsMan != nullptr)
		_ttsMan->setLanguage(ConfMan.get("language"));
}

void SciTTS::button(const Common::String &text) {
	if (_ttsMan != nullptr && shouldPerformTTS(text))
		_ttsMan->say(getMessage(text), Common::TextToSpeechManager::QUEUE_NO_REPEAT);
}

void SciTTS::text(const Common::String &text) {
	if (_ttsMan != nullptr && shouldPerformTTS(text))
		_ttsMan->say(getMessage(text), Common::TextToSpeechManager::INTERRUPT);
}

void SciTTS::stop() {
	if (_ttsMan != nullptr)
		_ttsMan->stop();
}

void SciTTS::setMessage(const Common::String &text) {
	if (text.size() > 0)
		_curMessage = text;
}

Common::String SciTTS::getMessage(const Common::String &text) {
	Common::String message = text;

	// If the current message contains a substring of the text to be displayed,
	// minus the first letter, prefer the message instead. The first letter is
	// chopped off in messages in games such as KQ6 and is replaced with tabs in
	// KQ6 or spaces in KQ5, so that a calligraphic first letter is drawn instead.
	if (_curMessage.size() > 0 && text.size() > 0 && text.hasSuffix(_curMessage.substr(1)))
		message = _curMessage;
	else
		message = text;

	// Strip color code characters in SCI1.1
	if (getSciVersion() == SCI_VERSION_1_1) {
		int32 index = message.find('|');

		while (index >= 0) {
			do {
				message.deleteChar(index);
			} while (message.size() > 0 && message[index] != '|');

			if (message.size() > 0)
				message.deleteChar(index);

			index = message.find('|');
		}
	}

	return message;
}

bool SciTTS::shouldPerformTTS(const Common::String &message) const {
	SciGameId gameId = g_sci->getGameId();
	uint16 roomNumber = g_sci->getEngineState()->currentRoomNumber();

	// Check if it's an actual message, by checking for the
	// existence of any vowel.
	// For example, when talking to the alien in SQ5 room 500, a
	// series of symbols is shown, as part of a joke.
	if (!message.contains('a') &&
		!message.contains('e') &&
		!message.contains('i') &&
		!message.contains('o') &&
		!message.contains('u'))
		return false;

	// Skip TTS for QFG4 room 140 (character creation screen).
	if (gameId == GID_QFG4 && roomNumber == 140)
		return false;

	return true;
}

} // End of namespace Sci
