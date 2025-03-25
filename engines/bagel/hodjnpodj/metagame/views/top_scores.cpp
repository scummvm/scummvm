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

#define FONT_SIZE		8

TopScores::TopScores() : Dialog("TopScores"),
	_okButton(RectWH(190, 325, 110, 20), this),
	m_pgtGTStruct(&g_engine->_grandTour),
	_settings(g_engine->_settings["Top 10"]),
	_textRect(RectWH(69, (_newRank * 20) + 90, 280, 20))
{
}

bool TopScores::msgAction(const ActionMessage &msg) {
	if (_newRank == -1) {
		if (msg._action == KEYBIND_SELECT ||
			msg._action == KEYBIND_ESCAPE) {
			close();
			return true;
		}
	}

	return false;
}

bool TopScores::msgGame(const GameMessage &msg) {
	if (msg._name == "GRAND_TOUR") {
		_newRank = msg._value;
		show();
		return true;
	} else if (msg._name == "BUTTON") {
		close();
		return true;
	}

	return false;
}

bool TopScores::msgKeypress(const KeypressMessage &msg) {
	if (_newRank == -1)
		return true;

	auto &astTopTenScores = g_engine->_topScores;
	auto &name = astTopTenScores[_newRank]._name;
	Common::Rect rTempRect;

	GfxSurface s = getSurface();
	s.setFontSize(FONT_SIZE);
	uint dwTextWidth = s.getStringWidth(astTopTenScores[_newRank]._name);
	uint tmWidth = s.getMaxCharWidth();

	rTempRect = Common::Rect((70 + dwTextWidth) - tmWidth - 1,
		(_newRank * 20) + 90 - 1,
		(70 + dwTextWidth + (tmWidth * 2)) + 1,
		(_newRank * 20) + 110 + 1);

	if (Common::isAlnum(msg.ascii)) {
		// Alphabetic or numbers
		if (name.size() < 30) {
			name.deleteLastChar();	// Remove the |
			name += msg.ascii;
			name += '|';
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_SPACE) {
		// Space
		if (name.size() < 30) {
			name.deleteLastChar();
			name += " |";
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_BACKSPACE) {
		// Backspace
		if (name.size() > 1) {
			name.deleteChar(name.size() - 2);
			redraw();
		}
	} else if (Common::KEYCODE_RETURN) {
		// Return key
		name.deleteLastChar();
		saveScores();

		_newRank = -1;
		redraw();
	}

	return true;
}

void TopScores::draw() {
	Dialog::draw();
	const int nLeft = 0, nTop = 0;
	GfxSurface s = getSurface();
	s.setFontSize(FONT_SIZE);

	const auto &scores = g_engine->_topScores;

	s.writeString("GRAND TOUR - TOP TEN SCORES", Common::Point(nLeft + 50, nTop + 50), BLACK);
	s.writeString("RANK", Common::Point(nLeft + 50, nTop + 70), BLACK);

	for (int i = 0; i < 10; i++) {
		Common::String num = Common::String::format("%d", i + 1);
		s.writeString(num, Common::Point(nLeft + 50, nTop + (i * 20) + 90), BLACK);

		if (i != _newRank)
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

	if (_newRank != -1) {
		s.writeString(scores[_newRank]._name, _textRect,
			g_engine->_bDonePodj ? RED : BLUE);

		s.writeString("Press Enter When Done.", Common::Point(75, 325), BLACK);
	}
}

void TopScores::show() {
	auto &astTopTenScores = g_engine->_topScores;
	int i;

	addView();
	getScores();

	for (i = 0; i < 10; i++) {
		if ((m_pgtGTStruct->nHodjSkillLevel != NOPLAY) &&
				(m_pgtGTStruct->nHodjScore > astTopTenScores[i]._score)) {
			int	j;

			_newRank = i;
			for (j = 10; j > i; j--) {
				astTopTenScores[j]._name = astTopTenScores[j - 1]._name;
				astTopTenScores[j]._score = astTopTenScores[j - 1]._score;
				astTopTenScores[j]._skillLevel = astTopTenScores[j - 1]._skillLevel;
			}
			astTopTenScores[i]._name = "HODJ|";
			astTopTenScores[i]._score = m_pgtGTStruct->nHodjScore;
			astTopTenScores[i]._skillLevel = m_pgtGTStruct->nHodjSkillLevel;
			break;
		}
	}

	if (_newRank == -1) {
		for (i = 0; i < 10; i++) {
			if ((m_pgtGTStruct->nPodjSkillLevel != NOPLAY) && (m_pgtGTStruct->nPodjScore > astTopTenScores[i]._score)) {
				int	j;

				_newRank = i;
				for (j = 10; j > i; j--) {
					astTopTenScores[j]._name = astTopTenScores[j - 1]._name;
					astTopTenScores[j]._score = astTopTenScores[j - 1]._score;
					astTopTenScores[j]._skillLevel = astTopTenScores[j - 1]._skillLevel;
				}
				astTopTenScores[i]._name = "PODJ|";
				astTopTenScores[i]._score = m_pgtGTStruct->nPodjScore;
				astTopTenScores[i]._skillLevel = m_pgtGTStruct->nPodjSkillLevel;


				g_engine->_bDonePodj = true;
				break;
			}
		}
	}
}

void TopScores::syncScores(bool isSaving) {
	Settings::Serializer s(_settings, isSaving);

	for (int i = 0; i < 10; i++) {
		s.sync(Common::String::format("name%d", i),
			g_engine->_topScores[i]._name);
		s.sync(Common::String::format("skillLevel%d", i),
			g_engine->_topScores[i]._skillLevel);
		s.sync(Common::String::format("score%d", i),
			g_engine->_topScores[i]._score);
	}
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
