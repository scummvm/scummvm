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

#include "mm/mm1/views_enh/interactions/trivia.h"
#include "mm/mm1/maps/map21.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Trivia::Trivia() : InteractionQuery("Trivia", 14) {
	_title = STRING["maps.emap21.title"];
}

bool Trivia::msgGame(const GameMessage &msg) {
	if (msg._name == "TRIVIA") {
		_question = STRING[Common::String::format(
			"maps.map21.questions.%d", msg._value)];
		_correctAnswer = STRING[Common::String::format(
			"maps.map21.answers.%d", msg._value)];

		addText(_question);
		open();
		return true;
	}

	return false;
}

void Trivia::answerEntered() {
	if (_answer.equalsIgnoreCase(_correctAnswer)) {
		send(InfoMessage(STRING["maps.map21.correct"]));
		g_globals->_party[0]._gems += 50;
		Sound::sound(SOUND_3);

	} else {
		g_maps->_mapPos.x = 15;
		g_maps->_currentMap->updateGame();
		send(InfoMessage(STRING["maps.map21.incorrect"]));
	}
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
