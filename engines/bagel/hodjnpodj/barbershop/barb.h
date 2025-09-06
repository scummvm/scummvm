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

#ifndef HODJNPODJ_BARBERSHOP_BARB_H
#define HODJNPODJ_BARBERSHOP_BARB_H

#include "bagel/hodjnpodj/barbershop/deck.h"
#include "bagel/hodjnpodj/barbershop/board.h"
#include "bagel/hodjnpodj/barbershop/paint.h"
#include "bagel/hodjnpodj/barbershop/logic.h"
#include "bagel/hodjnpodj/barbershop/undo.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

#define NOT_USED        -1
#define USED            0

//
// sounds
//
#define STOCKCARDS  ".\\sound\\carddeal.wav"
#define UNDO        ".\\sound\\sound355.wav"
#define WIN_SOUND   ".\\sound\\applause.wav"
#define LOSE_SOUND  ".\\sound\\sosorry.wav"

//
// related to stock card flipping sounds
//
#define RECYCLE_SOUND_FREQ  3

class CBarber {
private:
	CDeck       *m_cDck;        // Handles card shuffling
	CBoard      *m_cBrd;        // contains position info for all card stacks on screen
	CLogic      *m_pLogic;      // determines whether a given move is valid or not
	CUndo       *m_pUndo;       // records moves made by user and handles move undoing
	CSound      *m_pSound;

public:
	CBarber(CDC*, CSound*);
	~CBarber();

	void    NewGame(CDC*);
	void    Refresh(CDC*);
	void    OnLButtonDown(CWnd*, CPalette*, CPoint);
	void    OnLButtonDblClk(CWnd*, CPalette*, CPoint);
	void    OnMouseMove(CDC*, CPoint);
	void    OnLButtonUp(CWnd*);
	bool    IsInRect(CRect cStk, CRect cCrd);
	bool    IsGameOver(CWnd*);
	bool    IsNewBack(int);
	void    ChangeBack(CDC*, int);
	int     Score();

	CPaint  *m_cPnt;            // used to visually paint cards on screen
	CCard   *m_pCrd;            // the card being moved
	bool    m_bIsGameOver;      // tells if game has ended
	bool    m_bIsWin;           // tells if game was won or lost
};

// Globals!
extern CPalette         *pGamePalette;

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel

#endif
