/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * barb.h
 *
 * HISTORY
 *
 *		1.01	08/22/94	JSC		Documented
 *      1.00	03/28/94	JSC		First writing
 *
 * MODULE DESCRIPTION:
 *
 *	Class definition for Barbershop Quintet mini-game.
 *
 * RELEVANT DOCUMENTATION:
 *
 *	This is the main object for the mini-game.  Handles most
 *	higher level functions for the game object.
 *
 ****************************************************************/

#include "deck.h"
#include "board.h"
#include "paint.h"
#include "logic.h"
#include "undo.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/boflib/sound.h"

#define NOT_USED		-1
#define USED			0

//
// sounds
//
#define STOCKCARDS	".\\sound\\carddeal.wav"
#define UNDO		".\\sound\\sound355.wav"
#define WIN_SOUND	".\\sound\\applause.wav"
#define LOSE_SOUND	".\\sound\\sosorry.wav"

//
// related to stock card flipping sounds
//
#define RECYCLE_SOUND_FREQ	3

class CBarber {
private:
	CDeck		*m_cDck;		// Handles card shuffling
	CBoard		*m_cBrd;		// contains position info for all card stacks on screen
	CLogic		*m_pLogic;		// determines whether a given move is valid or not
	CUndo		*m_pUndo;		// records moves made by user and handles move undoing
	CSound		*m_pSound;

public:
	CBarber(CDC*, CSound*);
	~CBarber();

	void	NewGame(CDC*);
	void	Refresh(CDC*);
	void	OnLButtonDown(CWnd*, CPalette*, CPoint);
	void	OnLButtonDblClk(CWnd*, CPalette*, CPoint);
	void	OnMouseMove(CDC*, CPoint);
	void	OnLButtonUp(CWnd*);
	BOOL	IsInRect(CRect cStk, CRect cCrd);
	BOOL	IsGameOver(CWnd*);
	BOOL	IsNewBack(int);
	void	ChangeBack(CDC*, int);
	int		Score();

	CPaint	*m_cPnt;			// used to visually paint cards on screen
	CCard	*m_pCrd;			// the card being moved
	BOOL	m_bIsGameOver;		// tells if game has ended
	BOOL	m_bIsWin;			// tells if game was won or lost
};

// Globals!
extern CPalette			*pGamePalette;
