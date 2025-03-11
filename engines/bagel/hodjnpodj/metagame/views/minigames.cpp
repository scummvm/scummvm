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
#define FONT_SIZE		10

Minigames::MinigameRect Minigames::MINIGAME_RECTS[MINIGAMES_HOTSPOTS_COUNT] = {
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

int Minigames::MINIGAME_IDS[MINIGAMES_HOTSPOTS_COUNT] = {
	MG_GAME_ARCHEROIDS,
	MG_GAME_ARTPARTS,
	MG_GAME_BARBERSHOP,
	MG_GAME_BATTLEFISH,
	MG_GAME_BEACON,
	MG_GAME_CRYPTOGRAMS,
	MG_GAME_DAMFURRY,
	MG_GAME_FUGE,
	MG_GAME_GARFUNKEL,
	MG_GAME_LIFE,
	MG_GAME_MANKALA,
	MG_GAME_MAZEODOOM,
	MG_GAME_NOVACANCY,
	MG_GAME_PACRAT,
	MG_GAME_PEGGLEBOZ,
	MG_GAME_RIDDLES,
	MG_GAME_THGESNGGME,
	MG_GAME_VIDEOPOKER,
	MG_GAME_WORDSEARCH,
	-1
};

const char *Minigames::MINIGAME_TEXTS[MINIGAMES_HOTSPOTS_COUNT] = {
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
		_textRect(0, 428, GAME_WIDTH, 450) {
	setBounds(Common::Rect(0, 0, GAME_WIDTH, GAME_HEIGHT));
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
	if (msg._name == "BUTTON" && msg._stringValue == "MinigamesExit") {
		replaceView("TitleMenu");
		return true;
	}

	return false;
}

bool Minigames::msgMouseMove(const MouseMoveMessage &msg) {
	_mousePos = msg._pos;
	redraw();
	return true;
}

bool Minigames::msgMouseUp(const MouseUpMessage &msg) {
	int index = getSelectedMinigame();

	if (MINIGAME_IDS[index] == -1) {
		replaceView("TitleMenu");
	} else {
		pGameParams->bPlayingMetagame = false;
		g_engine->selectMinigame(MINIGAME_IDS[index]);
	}

	return View::msgMouseUp(msg);
}

void Minigames::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);

	int index = getSelectedMinigame();
	if (index != -1) {
		const Common::Rect rTemp1(
			(MINIGAME_RECTS[index].left - 5 + 2),
			(MINIGAME_RECTS[index].top - 5 + 2),
			(MINIGAME_RECTS[index].right + 5 + 2),
			(MINIGAME_RECTS[index].bottom + 5 + 2));
		const Common::Rect rTemp2(
			(MINIGAME_RECTS[index].left - 5),
			(MINIGAME_RECTS[index].top - 5),
			(MINIGAME_RECTS[index].right + 5),
			(MINIGAME_RECTS[index].bottom + 5));

		const byte CYAN = getPaletteIndex(RGB(0, 255, 255));
		const byte RED = getPaletteIndex(RGB(255, 0, 0));
		s.frameRect(rTemp1, BLACK);
		s.frameRect(rTemp2, CYAN);

		// FIXME: The original used bold cyan text for tooltips.
		// But since there's no facility in ScummVM to make the
		// loaded font bold, I chose a red color for contrast
		s.setFontSize(FONT_SIZE);
		const char *text = MINIGAME_TEXTS[index];
		s.writeShadowedString(text, _textRect,
			RED, Graphics::kTextAlignCenter);
	}
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
