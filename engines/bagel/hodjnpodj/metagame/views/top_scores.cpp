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

#include "bagel/hodjnpodj/metagame/views/top_scores.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

TopScores::TopScores() : Dialog("TopScores"),
	_okButton(RectWH(190, 325, 110, 20), this) {
}

bool TopScores::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT ||
		msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

bool TopScores::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		close();
		return true;
	}

	return false;
}

void TopScores::draw() {
	Dialog::draw();
	const int nLeft = 0, nTop = 0;
	GfxSurface s = getSurface();
	s.setFontSize(8);

	const auto &scores = g_engine->_topScores;
	const int newRank = g_engine->_newRank;

	s.writeString("GRAND TOUR - TOP TEN SCORES", Common::Point(nLeft + 50, nTop + 50), BLACK);
	s.writeString("RANK", Common::Point(nLeft + 50, nTop + 70), BLACK);

	for (int i = 0; i < 10; i++) {
		Common::String num = Common::String::format("%d", i + 1);
		s.writeString(num, Common::Point(nLeft + 50, nTop + (i * 20) + 90), BLACK);

		if (i != newRank)
			s.writeString(scores[i]._name,
				Common::Point(nLeft + 70, nTop + (i * 20) + 90), BLACK);

		switch (scores[i]._skillLevel) {
		case SKILLLEVEL_LOW:
			s.writeString("Easy", Common::Point(nLeft + 350, nTop + (i * 20) + 90), PURPLE);
			break;
		case SKILLLEVEL_MEDIUM:
			s.writeString("Medium", Common::Point(nLeft + 350, nTop + (i * 20) + 90), PURPLE);
			break;
		default:
			s.writeString("Hard", Common::Point(nLeft + 350, nTop + (i * 20) + 90), PURPLE);
			break;
		}

		s.writeString(Common::String::format("%d", scores[i]._score),
			Common::Point(nLeft + 425, nTop + (i * 20) + 90), PURPLE);
	}

	if (newRank > -1) {
		s.writeString(scores[newRank]._name,
			Common::Point(nLeft + 69, nTop + (newRank * 20) + 90),
			g_engine->_bDonePodj ? RED : BLUE);

		s.writeString("Press Enter When Done.", Common::Point(75, 325), BLACK);
	}
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
