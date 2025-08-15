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

#ifndef HODJNPODJ_NOVACANCY_MAIN_H
#define HODJNPODJ_NOVACANCY_MAIN_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace NoVacancy {

char *GetStringFromResource(UINT);

class CMainWindow : public CFrameWnd {
public:
	CMainWindow(VOID);
	VOID        PlayGame(VOID);
	VOID        PaintScreen(VOID);
	VOID        LoadIniSettings(VOID);
	VOID        SaveIniSettings(VOID);


protected:
	VOID        GameReset(VOID);
	VOID        GamePause(VOID);
	VOID        GameResume(VOID);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
	VOID        HandleError(ERROR_CODE);
	VOID        DeleteSprite(CSprite *);

	VOID        OnPaint(VOID);
	VOID        OnTimer(UINT_PTR);
	VOID        OnMouseMove(UINT, CPoint);
	virtual VOID        OnLButtonDown(UINT, CPoint);
	virtual VOID        OnRButtonDown(UINT, CPoint);
	VOID        OnSysChar(UINT, UINT, UINT);
	virtual  VOID        OnSysKeyDown(UINT, UINT, UINT);
	virtual VOID        OnKeyDown(UINT, UINT, UINT);
	VOID        OnActivate(UINT, CWnd *, BOOL) override;
	VOID        OnClose(VOID);
	virtual LRESULT     OnMCINotify(WPARAM, LPARAM);
	virtual LRESULT     OnMMIONotify(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()

	CRect       m_rNewGameButton,
	            m_rectGameArea;
	CPalette   *m_pGamePalette;
	CBmpButton *m_pScrollButton;

	CRect m_rLDie;                  //the rectangles defined by the two dice.
	CRect m_rRDie;

	//assume all doors are equi-sized and equidistant.
	CRect m_rDoor1;                //the first door
	CRect m_rDoor[10];            //rectangles representing doors 0 thru 9  ; #0 is unused.
	BOOL m_bDieUpdate;
	BOOL m_iDoorStatus[10];        //status of every door.
	BOOL m_bGameJustBegun;
	BOOL m_bDiceJustThrown;
	BOOL m_bDoorBmpLoaded[10];
	BOOL m_bGameLoadUp;

	BYTE m_cDoorToOpen;
	BYTE m_LDie;
	BYTE m_RDie;
	BYTE m_cUnDoableThrows;

	short m_iMoveValid;             //    flag to indicate the legal status of the last move. It is a ternary indicator :
	//      1 => Perfectly legal move, and the dice van be rolled if desired.
	//      0 => Legal move but not complete to roll the dice. The dice are disabled.
	//      -1=> Illegal move; stop and correct yourself. The dice are disabled.
	BYTE m_cDoorCount;            //    keeps track of all closed doors, irrespective of the  validity of  the move
	BYTE m_cActiveDoor;           //    the door whose status has just been changed (as result of mouse action).
	BOOL m_bOneDieCase;        //   this variable is always FALSE unless doors 7, 8, and 9 are FIEXD.

	//other objects for animation...
	CRect Paper,
	      Glass,
	      aBrShoes,
	      bBrShoes,
	      BluShoes,
	      Bottle,
	      Cat,
	      Hat6,
	      UmbrStand;

	BOOL        m_bGameActive;
	BOOL        m_bPause;
	BOOL        m_bIgnoreScrollClick;
	BOOL        m_bSound;
	CSound *m_psndBkgndMusic;

	CSprite *m_pCLRollingDie;
	CSprite *m_pCRRollingDie;
	CSprite *m_pCSingleRollingDie;      //unused in v.1.5 or later.

private:
	BOOL m_bDiceBmpsLoaded;
	CString gWndClass;

	void AnimateDice(void);
	short LegalizeMove(short i);
	BOOL IsThrowDoable(BYTE dice_sum);
};


#define LEFT 0
#define RIGHT 1

#define FIXED           2
#define CLOSED      1
#define OPEN             0

#define  toggle(V)  (V=!(V))
#define is_locked(door)     (m_iDoorStatus[door]==FIXED)

} // namespace NoVacancy
} // namespace HodjNPodj
} // namespace Bagel

#endif
