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
#include "bagel/hodjnpodj/maze_doom/globals.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/views/main_menu.h"
#include "bagel/hodjnpodj/views/rules.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

#define	WALL_X		0
#define	WALL_Y		22
#define	PATH_WIDTH	24
#define	PATH_HEIGHT	24
#define	PATH_X		48
#define	START_X		24
#define	EDGE_Y		46
#define	EDGE_WIDTH	5
#define	EDGE_HEIGHT	24
#define	TRAP_WIDTH	22
#define	TRAP_HEIGHT	22

// Border info              
#define SIDE_BORDER 	 20
#define TOP_BORDER		 28	 
#define BOTTOM_BORDER	 16	 
#define HILITE_BORDER	  3

// Dimensions constants
#define ART_WIDTH	600
#define ART_HEIGHT	432

#define SQ_SIZE_X	24
#define SQ_SIZE_Y   24 
#define EDGE_SIZE	 5

// Starting value defaults
#define MAX_DIFFICULTY	10	//8
#define MIN_DIFFICULTY	 1	//0 

// Timer constants         
#define MIN_TIME			 15						//  15 Seconds
#define MAX_TIME			180						// 180 Seconds = 3 minute max
#define TIMER_MIN			  0						// Increment scrollbar in steps of 15 Secs
#define TIMER_MAX			 10						// if Time > MAX_TIME, No Time Limit

#define NUM_COLUMNS		25
#define NUM_ROWS		19
#define NUM_NEIGHBORS	 9							// The "clump" area is 3 X 3 grid spaces

#define	MIN_TRAPS		 4

#define NUM_CELS		 8

#define PATH	0
#define WALL	1
#define TRAP	2
#define START	3
#define EXIT	4

#define HODJ	0
#define PODJ	4

// Timer stuff
#define GAME_TIMER 		1
#define CLICK_TIME	 1000		// Every Second, update timer clock 

// Rules files
#define	RULES_TEXT		"MAZEOD.TXT"
#define	RULES_WAV		".\\SOUND\\MAZEOD.WAV"

// Sound files                          
#define WIN_SOUND	".\\sound\\fanfare2.wav"
#define LOSE_SOUND	".\\sound\\buzzer.wav"
#define HIT_SOUND	".\\sound\\thud.wav"
#define TRAP_SOUND	".\\sound\\boing.wav"

#define GAME_THEME	".\\sound\\mazeod.mid"

// Backdrop bitmaps
#define MAINSCREEN	".\\ART\\DOOM2.BMP"

// New Game button area
#define	NEWGAME_LOCATION_X	 15
#define	NEWGAME_LOCATION_Y	  0
#define	NEWGAME_WIDTH		217
#define NEWGAME_HEIGHT		 20

// Time Display area
#define	TIME_LOCATION_X		420
#define	TIME_LOCATION_Y		  4
#define	TIME_WIDTH			195
#define TIME_HEIGHT			 15

MazeDoom::MazeDoom() : MinigameView("MazeDoom", "mazedoom/hnpmaze.dll"),
		_scrollButton("Scroll", this, Common::Rect(
			SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
			SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
			SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1)
		),
		pPlayerSprite(this) {
	addResource(IDB_LOCALE_BMP, "idb_locale_bmp");
	addResource(IDB_BLANK_BMP, "idb_blank_bmp");
	addResource(IDB_PARTS_BMP, IDB_PARTS);
}

bool MazeDoom::msgOpen(const OpenMessage &msg) {
	MinigameView::msgOpen(msg);

	setupHodjPodj();
	loadBitmaps();

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

void MazeDoom::showMainMenu() {
	MainMenu::show(
		pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
		RULES_TEXT,
		pGameParams->bSoundEffectsEnabled ? RULES_WAV : NULL);
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
