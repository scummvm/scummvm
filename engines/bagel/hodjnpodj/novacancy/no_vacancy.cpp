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

#include "common/system.h"
#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/novacancy/no_vacancy.h"
#include "bagel/hodjnpodj/novacancy/defines.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/views/main_menu.h"
#include "bagel/hodjnpodj/views/message_box.h"
#include "bagel/hodjnpodj/views/rules.h"

namespace Bagel {
namespace HodjNPodj {
namespace NoVacancy {

NoVacancy::NoVacancy() : MinigameView("NoVacancy", "novac/hnpnova.dll"),
		_scrollButton("Scroll", this, Common::Rect(
			SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
			SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
			SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1)),
		m_rNewGameButton(15, 4, 233, 20),
		m_rectGameArea(0, 0, GAME_WIDTH, GAME_HEIGHT),
		//set coordinates for paper, bottle,etc.
		Paper(PAPER_L, PAPER_T, PAPER_R, PAPER_B),
		aBrShoes(aBRSHOES_L, aBRSHOES_T, aBRSHOES_R, aBRSHOES_B),
		bBrShoes(bBRSHOES_L, bBRSHOES_T, bBRSHOES_R, bBRSHOES_B),
		BluShoes(BLSHOES_L, BLSHOES_T, BLSHOES_R, BLSHOES_B),
		Bottle(BOTTLE_L, BOTTLE_T, BOTTLE_R, BOTTLE_B),
		UmbrStand(STAND_L, STAND_T, STAND_R, STAND_B),
		Cat(HAT4_L, HAT4_T, HAT4_R, HAT4_B),
		Hat6(HAT6_L, HAT6_T, HAT6_R, HAT6_B),
		// Set the coordinates for the Dice Locations.
		m_rLDie(LDIE_LEFT, LDIE_TOP, LDIE_RIGHT, LDIE_BOTTOM),
		m_rRDie(RDIE_LEFT, RDIE_TOP, RDIE_RIGHT, RDIE_BOTTOM),
		m_pCLRollingDie(this),
		m_pCRRollingDie(this)
{
	// Set the coordinates for the Doors.
	m_rDoor1 = Common::Rect(DOOR1_LEFT, DOOR1_TOP, DOOR1_RIGHT, DOOR1_BOTTOM);
	m_rDoor[1] = Common::Rect(DOOR1_LEFT, DOOR1_TOP, DOOR1_RIGHT, DOOR1_BOTTOM);
	m_rDoor[2] = Common::Rect(DOOR2_LEFT, DOOR2_TOP, DOOR2_RIGHT, DOOR2_BOTTOM);
	m_rDoor[3] = Common::Rect(DOOR3_LEFT, DOOR3_TOP, DOOR3_RIGHT, DOOR3_BOTTOM);
	m_rDoor[4] = Common::Rect(DOOR4_LEFT, DOOR4_TOP, DOOR4_RIGHT, DOOR4_BOTTOM);
	m_rDoor[5] = Common::Rect(DOOR5_LEFT, DOOR5_TOP, DOOR5_RIGHT, DOOR5_BOTTOM);
	m_rDoor[6] = Common::Rect(DOOR6_LEFT, DOOR6_TOP, DOOR6_RIGHT, DOOR6_BOTTOM);
	m_rDoor[7] = Common::Rect(DOOR7_LEFT, DOOR7_TOP, DOOR7_RIGHT, DOOR7_BOTTOM);
	m_rDoor[8] = Common::Rect(DOOR8_LEFT, DOOR8_TOP, DOOR8_RIGHT, DOOR8_BOTTOM);
	m_rDoor[9] = Common::Rect(DOOR9_LEFT, DOOR9_TOP, DOOR9_RIGHT, DOOR9_BOTTOM);

	addResource(SCROLLUP_BMP, SCROLLUP);
	addResource(SCROLLDOWN_BMP, SCROLLDOWN);
}

bool NoVacancy::msgOpen(const OpenMessage &msg) {
	MinigameView::msgOpen(msg);

	resetFields();
	loadBitmaps();

	if (pGameParams->bMusicEnabled) {
		if (m_psndBkgndMusic = new CBofSound(this, "novac/sound/shotmac.mid", SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END)) {
			m_psndBkgndMusic->midiLoopPlaySegment(4000L, 31030L, 0L, FMT_MILLISEC);
		}
	} else {
		m_psndBkgndMusic = nullptr;
	}

	return true;
}

bool NoVacancy::msgClose(const CloseMessage &msg) {
	clearBitmaps();
	return MinigameView::msgClose(msg);
}

bool NoVacancy::msgAction(const ActionMessage &msg) {
	return true;
}

bool NoVacancy::msgKeypress(const KeypressMessage &msg) {
	return true;
}

bool NoVacancy::msgMouseDown(const MouseDownMessage &msg) {
	return true;
}

bool NoVacancy::msgGame(const GameMessage &msg) {
	if (msg._name == "NEW_GAME") {
		playGame();
		return true;
	}

	return false;
}

void NoVacancy::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);
}

bool NoVacancy::tick() {
	MinigameView::tick();
	return true;
}

void NoVacancy::resetFields() {
	m_bSound = true;

	m_bGameLoadUp = true;
	m_bPause = false;
	m_bGameActive = false;
	m_cActiveDoor = OPEN;			// The game has just begun and no door is active. The 0-th door is always open.
	m_cDoorCount = 0;				//	no door is closed.    

	m_cUnDoableThrows = 0;			//	no undoable throw has been registered yet.
	m_bDiceJustThrown = false;		// dice haven't been thrown yet
	m_bOneDieCase = false;			// always start w/ two dice on floor.     

	// Every door is preset to open, the 0-th door is ALWAYS open.
	Common::fill(m_iDoorStatus, m_iDoorStatus + 10, OPEN);
	Common::fill(m_bDoorBmpLoaded, m_bDoorBmpLoaded + 10, false);
}

void NoVacancy::loadBitmaps() {
	Image::BitmapDecoder decoder;
	Common::File f;
	static const int16 xDice[RIGHT + 1][7] =
		{{0,60,120,180,240,300,360},{0,58,116,174,232,290,348} };
	static const int16 yDice[RIGHT + 1][7] =
		{ {0,0,0,0,0,0,0},{62,62,62,62,62,62,62} };
	static const int16 dxDice[RIGHT + 1][7] =
		{ {60,60,60,60,60,60,60},{57,58,58,58,58,58,58} };
	static const int16 dyDice[RIGHT + 1][7] =
		{ {62,62,62,62,62,62,62},{60,61,61,61,61,61,61} };

	if (!f.open(BACKGROUND_BMP) || !decoder.loadStream(f))
		error("Could not load - %s", BACKGROUND_BMP);
	loadPalette(decoder.getPalette());

	_background.copyFrom(*decoder.getSurface());

	_scrollButton.loadBitmaps(SCROLLUP_BMP, SCROLLDOWN_BMP,
		nullptr, nullptr);

	m_pCLRollingDie.loadCels("novac/art/ld.bmp", NUM_LDIE_CELS);
	m_pCLRollingDie.linkSprite();

	m_pCRRollingDie.loadCels("novac/art/rd.bmp", NUM_RDIE_CELS);
	m_pCRRollingDie.linkSprite();

	pCMonolithDiceBmp.loadBitmap("novac/art/diestrp2.bmp");
	pCLDieBmp[0] = GfxSurface(pCMonolithDiceBmp, Common::Rect(
		xDice[LEFT][0], yDice[LEFT][0],
		dxDice[LEFT][0], dyDice[LEFT][0]), this);

	// flr under Ldie
	pCRDieBmp[0] = GfxSurface(pCMonolithDiceBmp,
		RectWH(xDice[RIGHT][0], yDice[RIGHT][0],
			dxDice[RIGHT][0], dyDice[RIGHT][0]), this);	//flr under Rdie
	for (int i = 1; i < 7; i++) {
		pCLDieBmp[i] = GfxSurface(pCMonolithDiceBmp,
			RectWH(xDice[LEFT][i], yDice[LEFT][i],
				dxDice[LEFT][i], dyDice[LEFT][i]), this);
		pCRDieBmp[i] = GfxSurface(pCMonolithDiceBmp,
			RectWH(xDice[RIGHT][i], yDice[RIGHT][i],
				dxDice[RIGHT][i], dyDice[RIGHT][i]), this);
	}
}

void NoVacancy::clearBitmaps() {
	_background.clear();
	_scrollButton.clear();
	m_pCLRollingDie.clear();
	m_pCRRollingDie.clear();
	m_pCLRollingDie.unlinkSprite();
	m_pCRRollingDie.unlinkSprite();

	pCMonolithDiceBmp.clear();
	for (int i = 0; i < 7; i++) {
		pCLDieBmp[i].clear();
		pCRDieBmp[i].clear();
	}
}

void NoVacancy::showMainMenu() {
	MainMenu::show(
		(pGameParams->bplayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0) |
		(m_bGameActive ? 0 : NO_RETURN),
		RULES_TEXT,
		pGameParams->bSoundEffectsEnabled ? RULES_WAV : NULL);
}

void NoVacancy::playGame() {
	m_LDie = getRandomNumber(1, 6);
	m_RDie = getRandomNumber(1, 6);

	// TODO: play game
}

} // namespace NoVacancy
} // namespace HodjNPodj
} // namespace Bagel
