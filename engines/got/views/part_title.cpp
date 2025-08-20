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

#include "got/views/part_title.h"
#include "got/metaengine.h"
#include "got/vars.h"

namespace Got {
namespace Views {

void PartTitle::draw() {
	GfxSurface s = getSurface();
	s.clear();
	s.print(Common::Point(13 * 8, 13 * 8), "God of Thunder", 14);
#ifdef USE_TTS
	Common::String ttsMessage = "God of Thunder: ";
#endif

	switch (_G(area)) {
	case 1:
		s.print(Common::Point(8 * 8, 15 * 8), "Part I: Serpent Surprise", 32);
#ifdef USE_TTS
		ttsMessage += "Part 1: Serpent Surprise";
#endif
		break;
	case 2:
		s.print(Common::Point(7 * 8, 15 * 8), "Part II: Non-Stick Nognir", 32);
#ifdef USE_TTS
		ttsMessage += "Part 2: Non-Stick Nognir";
#endif
		break;
	case 3:
		s.print(Common::Point(7 * 8, 15 * 8), "Part III: Lookin' for Loki", 32);
#ifdef USE_TTS
		ttsMessage += "Part 3: Lookin' for Loki";
#endif
		break;
	default:
		break;
	}
#ifdef USE_TTS
	sayText(ttsMessage);
#endif
}

bool PartTitle::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE)
		done();

	return true;
}

bool PartTitle::tick() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (++_timeoutCtr >= 80 && (!ttsMan || !ttsMan->isSpeaking())) {
		_timeoutCtr = 0;
		done();
	}

	return true;
}

void PartTitle::done() {
#ifdef USE_TTS
	stopTextToSpeech();
#endif
	replaceView("Game", true, true);
}

} // namespace Views
} // namespace Got
