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

#include "bagel/hodjnpodj/metagame/boardgame/minimap.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define	TEXT_COLOR				RGB(0, 0, 0)	// displayed text color
#define	ITEMDLG_TEXT_COLOR		RGB(128,0,128)
#define	ITEMDLG_BLURB_COLOR		RGB(0,0,255)

#define	MAP_COL			20				// first message box positioning
#define	MAP_ROW     	10

#define ZOOMMAP_WIDTH		640				// pixel dims
#define	ZOOMMAP_HEIGHT		480
#define BIGMAP_WIDTH		1925			// pixel dims
#define	BIGMAP_HEIGHT		1415

#define	MAP_DX				113				// Width of Dialog box
#define	MAP_DY				110
#define	PLAYER_BITMAP_DX	18
#define	PLAYER_BITMAP_DY	28
#define	PLAYER_OFFSET_X		 9
#define	PLAYER_OFFSET_Y		14

#define	NUM_NODES			20
#define IDD_ZOOMMAP			204
#define	BUTTON_DY			10

Minimap::Minimap() : View("Minimap"),
	_okButton(RectWH(GAME_WIDTH / 2 - 25,
		GAME_HEIGHT - 35, 50, 20), this) {
}

bool Minimap::msgOpen(const OpenMessage &msg) {
	View::msgOpen(msg);
	_background.loadBitmap("meta/art/minimap.bmp");
	_hodj.loadBitmap("meta/art/hodj.bmp");
	_podj.loadBitmap("meta/art/podj.bmp");

	return true;
}

bool Minimap::msgClose(const CloseMessage &msg) {
	View::msgClose(msg);
	_background.clear();
	return true;
}

bool Minimap::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT ||
		msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

bool Minimap::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		close();
		return true;
	}

	return false;
}

void Minimap::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);
	// TODO: Get Hodj/Podj position and draw on map
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
