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

#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/metagame/views/minigames.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define	BACKGROUND_BMP	"meta/art/zoommap.bmp"
#define FONT_SIZE		12

Minigames::MinigameRect Minigames::MINIGAME_RECTS[21] = {
	{  24, 243,  63, 263 },
	{ 495, 373, 529, 406 },
	{ 412, 185, 432, 201 },
	{ 526, 310, 597, 342 },
	{ 568, 376, 590, 417 },
	{ 201, 329, 254, 357 },
	{ 375, 136, 405, 153 },
	{  77, 253, 114, 302 },
	{ 314, 181, 337, 203 },
	{ 450,   6, 479,  34 },
	{ 482, 300, 505, 329 },
	{ 171,  75, 203,  94 },
	{ 255,  50, 271,  72 },
	{ 126, 254, 191, 278 },
	{ 467, 334, 519, 360 },
	{ 510, 189, 555, 226 },
	{ 292, 315, 341, 335 },
	{ 278,  70, 296,  88 },
	{ 349, 301, 382, 338 },
	{ 402, 203, 433, 250 }
};

const char *Minigames::MINIGAME_TEXTS[] = {
	"Click Here To Play Archeroids",
	"Click Here To Play Art Parts",
	"Click Here To Play Barbershop Quintet",
	"Click Here To Play Battlefish",
	"Click Here To Play Beacon",
	"Click Here To Play Cryptograms",
	"Click Here To Play Dam Furry Animals",
	"Click Here To Play Fuge",
	"Click Here To Play Garfunkel",
	"Click Here To Play Life",
	"Click Here To Play Mankala",
	"Click Here To Play Maze O' Doom",
	"Click Here To Play No Vacancy",
	"Click Here To Play Pack-Rat",
	"Click Here To Play Peggleboz",
	"Click Here To Play Riddles",
	"Click Here To Play TH GESNG GAM",
	"Click Here To Play Poker",
	"Click Here To Play Word Search",
	"Click Here To Go To Main Menu"
};

Minigames::Minigames() : View("Minigames"),
		_exitButton("MinigamesExit", "Main Menu",
			Common::Rect((GAME_WIDTH / 2) - 50, 450,
			(GAME_WIDTH / 2) + 50, 470)),
		_textRect(0, 428, 640, 450) {
}

bool Minigames::msgFocus(const FocusMessage &msg) {
	g_events->showCursor(true);

	bool showExit = msg._priorView->getName() == "TitleMenu";
	_exitButton.setParent(showExit ? this : nullptr);

	Common::File f;
	Image::BitmapDecoder decoder;
	if (!f.open(BACKGROUND_BMP) || !decoder.loadStream(f))
		error("Error loading - %s", BACKGROUND_BMP);

	const Graphics::Surface *surf = decoder.getSurface();
	_background.copyFrom(surf);
	_background.setTransparentColor(255);

	blackScreen();

	Common::Rect r(0, 0, surf->w, surf->h);
	r.moveTo((GAME_WIDTH - surf->w) / 2, (GAME_HEIGHT - surf->h) / 2);
	setBounds(r);

	return View::msgFocus(msg);
}

bool Minigames::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE && _exitButton.isVisible()) {
		replaceView("Title");
		return true;
	}

	return View::msgAction(msg);
}

bool Minigames::msgGame(const GameMessage &msg) {
	
	return false;
}

bool Minigames::msgMouseMove(const MouseMoveMessage &msg) {
	_mousePos = msg._pos;
	redraw();
	return true;
}

bool Minigames::msgMouseUp(const MouseUpMessage &msg) {
	int selectedMinigame = getSelectedMinigame();
	warning("TODO: Selection %d", selectedMinigame);
	return View::msgMouseUp(msg);
}

void Minigames::draw() {
	GfxSurface s = getSurface();
	s.setFontSize(FONT_SIZE);

	s.clear();
	s.blitFrom(_background);
}

int Minigames::getSelectedMinigame() const {
	for (int i = 0; i < MINIGAMES_HOTSPOTS_COUNT; ++i) {
		const MinigameRect &r = MINIGAME_RECTS[i];
		if (_mousePos.x >= r.left && _mousePos.x < r.right &&
				_mousePos.y >= r.top && _mousePos.y < r.bottom)
			return i;
	}

	return -1;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
