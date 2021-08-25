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

#include "sci/sci.h"
#include "sci/engine/speech.h"
#include "common/system.h"
#include "common/text-to-speech.h"
#include "common/config-manager.h"


namespace Sci {

void ttsSetLang() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr)
		ttsMan->setLanguage(ConfMan.get("language"));
}

Common::String textKeeper[10];

void ttsPickQ(const char *text) {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (textKeeper[0] != text && !(text == textKeeper[1] && textKeeper[0] == "")) {
		if (ttsMan != nullptr && g_sci->getGameId() == GID_LAURABOW2 && g_sci->isDemo() == false)
			ttsMan->say(text, Common::TextToSpeechManager::INTERRUPT);

			for (int i = 5; i < 10; ++i)
				textKeeper[i] = textKeeper[i - 5];

			textKeeper[0] = text;

			for (int i = 1; i < 5; ++i)
				textKeeper[i] = textKeeper[i + 4];
	}
}

void ttsButton(const char *text) {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr && ConfMan.getBool("tts_enabled") && (g_sci->getGameId() == GID_CASTLEBRAIN || g_sci->getGameId() == GID_LAURABOW2))
		ttsMan->say(text, Common::TextToSpeechManager::QUEUE_NO_REPEAT);
}

void ttsBox(const char *text) {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr && ConfMan.getBool("tts_enabled") && (g_sci->getGameId() == GID_LAURABOW2 || g_sci->getGameId() == GID_CASTLEBRAIN))
		ttsMan->say(text, Common::TextToSpeechManager::INTERRUPT);
}

const char *thelastText;

void ttsDisplay(const char *text) {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (text != thelastText && ttsMan != nullptr) {
		if ((g_sci->getGameId() == GID_LAURABOW2 || g_sci->getGameId() == GID_CASTLEBRAIN) && g_sci->isDemo() == true)
			ttsMan->say(text, Common::TextToSpeechManager::QUEUE_NO_REPEAT);
		thelastText = text;
	}
}

} // End of namespace Sci