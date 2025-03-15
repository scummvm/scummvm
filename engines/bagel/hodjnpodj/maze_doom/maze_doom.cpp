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
#include "bagel/hodjnpodj/maze_doom/maze_doom.h"
#include "bagel/hodjnpodj/maze_doom/defines.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/views/main_menu.h"
#include "bagel/hodjnpodj/views/rules.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

MazeDoom::MazeDoom() : MinigameView("MazeDoom", "mazedoom/hnpmaze.dll"),
		_scrollButton("Scroll", this, Common::Rect(
			SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
			SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
			SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1)
		),
		pPlayerSprite(this),
		_timeRect(RectWH(TIME_LOCATION_X, TIME_LOCATION_Y,
			TIME_WIDTH, TIME_HEIGHT)) {
	addResource(IDB_LOCALE_BMP, Common::WinResourceID("idb_locale_bmp"));
	addResource(IDB_BLANK_BMP, Common::WinResourceID("idb_blank_bmp"));
	addResource(IDB_PARTS_BMP, IDB_PARTS);
	addResource(IDB_HODJ_UP_BMP, IDB_HODJ_UP);
	addResource(IDB_HODJ_DOWN_BMP, IDB_HODJ_DOWN);
	addResource(IDB_HODJ_LEFT_BMP, IDB_HODJ_LEFT);
	addResource(IDB_HODJ_RIGHT_BMP, IDB_HODJ_RIGHT);
	addResource(IDB_PODJ_UP_BMP, IDB_PODJ_UP);
	addResource(IDB_PODJ_DOWN_BMP, IDB_PODJ_DOWN);
	addResource(IDB_PODJ_LEFT_BMP, IDB_PODJ_LEFT);
	addResource(IDB_PODJ_RIGHT_BMP, IDB_PODJ_RIGHT);
}

bool MazeDoom::msgOpen(const OpenMessage &msg) {
	MinigameView::msgOpen(msg);

	setupHodjPodj();
	loadBitmaps();
	setupSettings();
	initializeMaze();	// Set the surrounding wall and start/end squares 
	createMaze();		// Create a maze layout given the initialized maze

	return true;
}

bool MazeDoom::msgClose(const CloseMessage &msg) {
	MinigameView::msgClose(msg);

	_background.clear();
	_scrollButton.clear();
	_mazeBitmap.clear();
	_partsBitmap.clear();

	return true;
}

bool MazeDoom::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		// Show the main menu
		showMainMenu();
		return true;
	}

	return false;
}

void MazeDoom::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);
}

void MazeDoom::setupHodjPodj() {
	if (pGameParams->bPlayingHodj) {
		_upBmp = IDB_HODJ_UP_BMP;
		_downBmp = IDB_HODJ_DOWN_BMP;
		_leftBmp = IDB_HODJ_LEFT_BMP;
		_rightBmp = IDB_HODJ_RIGHT_BMP;
	} else {
		_upBmp = IDB_PODJ_UP_BMP;
		_downBmp = IDB_PODJ_DOWN_BMP;
		_leftBmp = IDB_PODJ_LEFT_BMP;
		_rightBmp = IDB_PODJ_RIGHT_BMP;
	}
}

void MazeDoom::loadBitmaps() {
	Image::BitmapDecoder decoder;
	Common::File f;

	if (!f.open(MAIN_SCREEN) || !decoder.loadStream(f))
		error("Could not load - %s", MAIN_SCREEN);
	loadPalette(decoder.getPalette());

	_background.copyFrom(*decoder.getSurface());

	_scrollButton.loadBitmaps(SCROLLUP_BMP, SCROLLDOWN_BMP,
		nullptr, nullptr);
	_mazeBitmap.create(NUM_COLUMNS * SQ_SIZE_X,
		NUM_ROWS * SQ_SIZE_Y);

	// Load up the various bitmaps for wall, edge, booby traps, etc.
	_partsBitmap.loadBitmap(IDB_PARTS_BMP);

	pWallBitmap = GfxSurface(_partsBitmap,
		RectWH(WALL_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT), this);
	pPathBitmap = GfxSurface(_partsBitmap,
		RectWH(PATH_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT), this);
	pStartBitmap = GfxSurface(_partsBitmap,
		RectWH(START_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT), this);

	pLeftEdgeBmp = GfxSurface(_partsBitmap,
		RectWH(0, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT), this);
	pRightEdgeBmp = GfxSurface(_partsBitmap,
		RectWH(EDGE_WIDTH, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT), this);
	pTopEdgeBmp = GfxSurface(_partsBitmap,
		RectWH(EDGE_WIDTH * 2, EDGE_Y, EDGE_HEIGHT, EDGE_WIDTH), this);
	pBottomEdgeBmp = GfxSurface(_partsBitmap,
		RectWH((EDGE_WIDTH * 2) + EDGE_HEIGHT, EDGE_Y,
			EDGE_HEIGHT, EDGE_WIDTH), this);

	for (int i = 0; i < NUM_TRAP_MAPS; i++) {
		TrapBitmap[i] = GfxSurface(_partsBitmap,
			RectWH(TRAP_WIDTH * i, 0, TRAP_WIDTH, TRAP_HEIGHT), this);
	}

	pPlayerSprite.loadCels(_leftBmp, NUM_CELS);
	pPlayerSprite.setTransparentColor(WHITE);

	pLocaleBitmap.loadBitmap(IDB_LOCALE_BMP);
	pBlankBitmap.loadBitmap(IDB_BLANK_BMP);
}

void MazeDoom::setupSettings() {
	if (pGameParams->bPlayingMetagame) {
		if (pGameParams->nSkillLevel == Metagame::SKILLLEVEL_LOW) {
			// Total Wussy
			m_nDifficulty = MIN_DIFFICULTY;
			m_nTime = 60;
		} else if (pGameParams->nSkillLevel == Metagame::SKILLLEVEL_MEDIUM) {
			// Big Sissy
			m_nDifficulty = 2;
			m_nTime = 60;
		} else {
			// Minor Whimp
			m_nDifficulty = 4;
			m_nTime = 60;
		}
	} else {                                      // Use Defaults 
		m_nDifficulty = 6;                      // Miner
		m_nTime = 180;
	}

	tempDifficulty = m_nDifficulty;
	tempTime = m_nTime;
	nSeconds = m_nTime % 60;
	nMinutes = m_nTime / 60;
}

void MazeDoom::showMainMenu() {
	MainMenu::show(
		pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
		RULES_TEXT,
		pGameParams->bSoundEffectsEnabled ? RULES_WAV : NULL);
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
