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

#include "mm/mm1/views/maps/trivia.h"
#include "mm/mm1/maps/map21.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

#define VAL1 159
#define VAL2 160

Trivia::Trivia() :
		AnswerEntry("Trivia", Common::Point(9, 5), 14) {
}

bool Trivia::msgValue(const ValueMessage &msg) {
	_question = STRING[Common::String::format(
		"maps.map21.questions.%d", msg._value)];
	_correctAnswer = STRING[Common::String::format(
		"maps.map21.questions.%d", msg._value)];

	open();
	return true;
}

void Trivia::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map19.ice_princess"]);
	AnswerEntry::draw();
}

void Trivia::answerEntered() {
	if (_answer.equalsIgnoreCase(_correctAnswer)) {
		send(InfoMessage(STRING["maps.map21.correct"]));
		g_globals->_party[0]._gems += 50;
		Sound::sound(SOUND_3);

	} else {
		g_maps->_mapPos.x = 15;
		g_events->send("Game", GameMessage("UPDATE"));
		send(InfoMessage(STRING["maps.map19.incorrect"]));
	}
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
