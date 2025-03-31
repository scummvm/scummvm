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

#include "bagel/hodjnpodj/metagame/boardgame/boardgame_options.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

constexpr bool bHomeWriteLocked = false;
constexpr bool bPathsDiffer = false;

BoardgameOptions::BoardgameOptions() : Dialog("BoardgameOptions"),
		_pHSHButton("HHARD", "Hard",          Common::Rect(72, 208, 240, 232), this),
		_pHSMButton("HMEDIUM", "Medium",      Common::Rect(72, 234, 240, 258), this),
		_pHSLButton("HEASY", "Easy",          Common::Rect(72, 260, 240, 284), this),
		_pPSHButton("PHARD", "Tough Opponent",     Common::Rect(262, 208, 440, 232), this),
		_pPSMButton("PMEDIUM", "Average Opponent", Common::Rect(262, 234, 440, 258), this),
		_pPSLButton("PEASY", "Unskilled Opponent", Common::Rect(262, 260, 440, 284), this),
		
		_pGTLButton("GLONG", "Long Game",     Common::Rect(262, 140, 440, 164), this),
		_pGTMButton("GMEDIUM", "Medium Game", Common::Rect(262, 114, 440, 138), this),
		_pGTSButton("GSHORT", "Short Game",   Common::Rect(262, 88, 440, 112), this),
		
		_pPCButton("COMPUTER", "One Player",  Common::Rect(72, 88, 240, 112), this),
		_pPHButton("HUMAN", "Two Players", Common::Rect(72, 114, 240, 138), this),

		_pPlayButton("PLAY", "Play", Common::Rect(101, 310, 211, 340), this),
		_pCancelButton("CANCEL", "Main Menu", Common::Rect(291, 310, 401, 340), this)
{
}

bool BoardgameOptions::msgOpen(const OpenMessage &msg) {
	Dialog::msgOpen(msg);
	lpMetaGame->initBFCInfo();

	m_nHodjSkillLevel = lpMetaGame->m_cHodj.m_iSkillLevel;
	m_nPodjSkillLevel = lpMetaGame->m_cPodj.m_iSkillLevel;
	m_bPodjIsComputer = lpMetaGame->m_cPodj.m_bComputer;
	m_nGameTime = lpMetaGame->m_iGameTime;
	updateRadioButtons();

	return true;
}

bool BoardgameOptions::msgClose(const CloseMessage &msg) {
	Dialog::msgClose(msg);
	return true;
}

bool BoardgameOptions::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		replaceView("TitleMenu", true);
		return true;
	}

	return false;
}

bool BoardgameOptions::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		if (msg._stringValue == "PLAY") {
			lpMetaGame->m_cHodj.m_iSkillLevel = m_nHodjSkillLevel;
			lpMetaGame->m_cPodj.m_iSkillLevel = m_nPodjSkillLevel;
			lpMetaGame->m_cHodj.m_bComputer = false;
			lpMetaGame->m_cPodj.m_bComputer = m_bPodjIsComputer;
			lpMetaGame->m_iGameTime = m_nGameTime;

			return true;
		} else if (msg._stringValue == "CANCEL") {
			close();
			return true;
		}
	} else if (msg._name == "RADIOBUTTON") {
		if (msg._stringValue == "HHARD") {
			m_nHodjSkillLevel = SKILLLEVEL_HIGH;
		} else if (msg._stringValue == "HMEDIUM") {
			m_nHodjSkillLevel = SKILLLEVEL_MEDIUM;
		} else if (msg._stringValue == "HEASY") {
			m_nHodjSkillLevel = SKILLLEVEL_LOW;
		} else if (msg._stringValue == "PHARD") {
			m_nPodjSkillLevel = SKILLLEVEL_HIGH;
		} else if (msg._stringValue == "PMEDIUM") {
			m_nPodjSkillLevel = SKILLLEVEL_MEDIUM;
		} else if (msg._stringValue == "PEASY") {
			m_nPodjSkillLevel = SKILLLEVEL_LOW;
		} else if (msg._stringValue == "GLONG") {
			m_nGameTime = LONG_GAME;
		} else if (msg._stringValue == "GMEDIUM") {
			m_nGameTime = MEDIUM_GAME;
		} else if (msg._stringValue == "GSHORT") {
			m_nGameTime = SHORT_GAME;
		} else if (msg._stringValue == "COMPUTER") {
			m_bPodjIsComputer = true;
		} else if (msg._stringValue == "HUMAN") {
			m_bPodjIsComputer = false;
		}

		updateRadioButtons();
		redraw();
		return true;
	}

	return false;
}

void BoardgameOptions::draw() {
	Dialog::draw();
	GfxSurface s = getSurface();
	
	s.writeString("Number of Players", Common::Point(72, 63));
	s.writeString("Game Duration", Common::Point(262, 63));
	s.writeString("Hodj's Skill Level", Common::Point(72, 184));
	s.writeString("Podj's Skill Level", Common::Point(262, 184));
}

void BoardgameOptions::updateRadioButtons() {
	_pHSHButton.setCheck(m_nHodjSkillLevel == SKILLLEVEL_HIGH);
	_pHSMButton.setCheck(m_nHodjSkillLevel == SKILLLEVEL_MEDIUM);
	_pHSLButton.setCheck(m_nHodjSkillLevel == SKILLLEVEL_LOW);

	_pPSHButton.setCheck(m_nPodjSkillLevel == SKILLLEVEL_HIGH);
	_pPSMButton.setCheck(m_nPodjSkillLevel == SKILLLEVEL_MEDIUM);
	_pPSLButton.setCheck(m_nPodjSkillLevel == SKILLLEVEL_LOW);

	_pGTLButton.setCheck(m_nGameTime == LONG_GAME);
	_pGTMButton.setCheck(m_nGameTime == MEDIUM_GAME);
	_pGTSButton.setCheck(m_nGameTime == SHORT_GAME);

	if (m_bPodjIsComputer) {
		_pPCButton.setCheck(true);
		_pPHButton.setCheck(false);
		_pPSHButton.setText("Tough Opponent");
		_pPSMButton.setText("Average Opponent");
		_pPSLButton.setText("Unskilled Opponent");
	} else {
		_pPCButton.setCheck(false);
		_pPHButton.setCheck(true);
		_pPSHButton.setText("Hard");
		_pPSMButton.setText("Medium");
		_pPSLButton.setText("Easy");
	}
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
