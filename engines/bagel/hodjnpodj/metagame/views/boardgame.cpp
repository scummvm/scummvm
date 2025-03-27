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

#include "bagel/hodjnpodj/metagame/views/boardgame.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

constexpr bool bHomeWriteLocked = false;
constexpr bool bPathsDiffer = false;

Boardgame::Boardgame() : Dialog("Boardgame"),
		_pHSHButton("HHARD", "Hard",          Common::Rect(72, 208, 240, 232), this),
		_pHSMButton("HMEDIUM", "Medium",      Common::Rect(72, 234, 240, 258), this),
		_pHSLButton("HEASY", "Easy",          Common::Rect(72, 260, 240, 284), this),
		_pPSHButton("PHARD", "Tough Opponent",     Common::Rect(262, 208, 440, 232), this),
		_pPSMButton("PMEDIUM", "Average Opponent", Common::Rect(262, 234, 440, 258), this),
		_pPSLButton("PEASY", "Unskilled Opponent", Common::Rect(262, 260, 440, 284), this),
		
		_pGTLButton("GLONG", "Long Game",     Common::Rect(262, 140, 440, 164), this),
		_pGTMButton("GMEDIUM", "Medium Game", Common::Rect(262, 114, 440, 138), this),
		_pGTSButton("GSHORT", "Short Game",   Common::Rect(262, 88, 440, 112), this),
		
		_pPCButton("PLAYERS1", "One Player",  Common::Rect(72, 88, 240, 112), this),
		_pPHButton("PLAYERS2", "Two Players", Common::Rect(72, 114, 240, 138), this),

		_pPlayButton("Play", "Play", Common::Rect(101, 310, 211, 340), this),
		_pCancelButton("Cancel", "Main Menu", Common::Rect(291, 310, 401, 340), this)
{
}

bool Boardgame::msgOpen(const OpenMessage &msg) {
	Dialog::msgOpen(msg);
	lpMetaGame->initBFCInfo();

	return true;
}

bool Boardgame::msgClose(const CloseMessage &msg) {
	Dialog::msgClose(msg);
	return true;
}

bool Boardgame::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		replaceView("TitleMenu", true);
		return true;
	}

	return false;
}

bool Boardgame::msgGame(const GameMessage &msg) {
	return false;
}

void Boardgame::draw() {
	Dialog::draw();
	GfxSurface s = getSurface();
	
	s.writeString("Number of Players", Common::Point(72, 63));
	s.writeString("Game Duration", Common::Point(262, 63));
	s.writeString("Hodj's Skill Level", Common::Point(72, 184));
	s.writeString("Podj's Skill Level", Common::Point(262, 184));
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
