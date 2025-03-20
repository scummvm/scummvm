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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_NOVACANCY_H
#define HODJNPODJ_NOVACANCY_H

#include "bagel/hodjnpodj/views/minigame_view.h"
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace NoVacancy {

class NoVacancy : public MinigameView {
private:
	BmpButton _scrollButton;
	Common::Rect m_rNewGameButton,
		m_rectGameArea;

	Common::Rect m_rLDie;		// The rectangles defined by the two dice.
	Common::Rect m_rRDie;

	// Assume all doors are equi-sized and equidistant.
	Common::Rect m_rDoor1;		// The first door                 
	Common::Rect m_rDoor[10];	// Rectangles representing doors 0 thru 9  ; #0 is unused.
	bool m_bDieUpdate;
	bool m_iDoorStatus[10];		// Status of every door.
	bool m_bGameJustBegun;
	bool m_bDiceJustThrown;
	bool m_bDoorBmpLoaded[10];
	bool m_bGameLoadUp;

	byte m_cDoorToOpen;
	byte m_LDie;
	byte m_RDie;
	byte m_cUnDoableThrows;

	short m_iMoveValid;             //	  flag to indicate the legal status of the last move. It is a ternary indicator : 
	//		1 => Perfectly legal move, and the dice van be rolled if desired.
	//		0 => Legal move but not complete to roll the dice. The dice are disabled.
	//		-1=> Illegal move; stop and correct yourself. The dice are disabled.
	byte m_cDoorCount;            //	keeps track of all closed doors, irrespective of the  validity of  the move      
	byte m_cActiveDoor;           //    the door whose status has just been changed (as result of mouse action).   
	bool m_bOneDieCase;        //	this variable is always FALSE unless doors 7, 8, and 9 are FIEXD.

	//other objects for animation...
	Common::Rect Paper,
		Glass,
		aBrShoes,
		bBrShoes,
		BluShoes,
		Bottle,
		Cat,
		Hat6,
		UmbrStand;

	bool        m_bGameActive;
	bool        m_bPause;
	bool        m_bIgnoreScrollClick;
	bool        m_bSound;
	CSound *m_psndBkgndMusic;

	GfxSurface m_pCLRollingDie;
	GfxSurface m_pCRRollingDie;
//	GfxSurface m_pCSingleRollingDie;


private:
	bool m_bDiceBmpsLoaded;

protected:
	void showMainMenu() override;

public:
	NoVacancy();
	virtual ~NoVacancy() {
	}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace NoVacancy
} // namespace HodjNPodj
} // namespace Bagel

#endif
