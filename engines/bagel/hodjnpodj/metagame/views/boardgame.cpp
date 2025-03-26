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
		_pPlayButton("Play", "Play", RectWH(45, 155, 50, 15), this),
		_pCancelButton("Cancel", "Main Menu", RectWH(130, 155, 50, 15), this),

		_pHSHButton("HHARD", "Hard", RectWH(32, 104, 76, 12), this),
		_pHSMButton("HMEDIUM", "Medium", RectWH(32, 117, 76, 12), this),
		_pHSLButton("HEASY", "Easy", RectWH(32, 130, 76, 12), this),
		_pPSHButton("PHARD", "Tough Opponent", RectWH(117, 104, 76, 12), this),
		_pPSMButton("PMEDIUM", "Average Opponent", RectWH(117, 117, 76, 12), this),
		_pPSLButton("PEASY", "Unskilled Opponent", RectWH(117, 130, 76, 12), this),
		
		_pGTLButton("GLONG", "Long Game", RectWH(117, 70, 76, 12), this),
		_pGTMButton("GMEDIUM", "Medium Game", RectWH(117, 57, 76, 12), this),
		_pGTSButton("GSHORT", "Short Game", RectWH(117, 44, 76, 12), this),
		
		_pPCButton("PLAYERS1", "One Player", RectWH(32, 44, 76, 12), this),
		_pPHButton("PLAYERS2", "Two Players", RectWH(32, 57, 76, 12), this)
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

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
