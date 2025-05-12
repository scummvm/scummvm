/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  life.h
*
*  HISTORY
*
*		1.10	08/04/94	JSC		upgraded to net resources
*		1.00	02/28/94	JSC		Added life.h
*
*  MODULE DESCRIPTION:
*
*       Header for user configuration dialog
*
*  RELEVANT DOCUMENTATION:
*
*
*
****************************************************************/
#ifndef __game_life_H__
#define __game_life_H__

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "grid.h"

#define	VILLAGES		5	// Constants
#define	MONTHS			12
#define	FASTEST			0	// Changing this doesn't effectly change it
							// everywhere.  speed of evolution.
#define	LARGE		32468 	//= 2^15 - 300
							//	when to reset counter increments

#define	WAV_EVOLVE		".\\sound\\sound178.wav"	// Wave sounds
#define WAV_GAMEOVER	".\\sound\\sound283.wav"
#define WAV_NEWVILLAGE	".\\sound\\colony.wav"
#define WAV_DEATH		".\\sound\\sound259.wav"
#define	WAV_CANTDO		".\\sound\\sorry.wav"

class CCalendar {
public:
	CSprite	*pMonthSprite[MONTHS];
	BOOL	bIsOn[MONTHS];
};

class CLife {
public:
	// Constructor
	CLife(CDC *pDC);
	// Destructor
	~CLife();
	void	change_board(UINT, CPoint, CDC*, BOOL);
	void	evolution(CDC *pDC);
	void	NewGame(CDC *pDC);
	void	DisplayMonth(int nMonth, CDC *pDC);
	void	ResetMonths(CDC *pDC);
	int		ColonyPlaced() { return (*pColony).m_nColony_count; }

public:
	colony 		*pColony;
	double		m_dScore;
	int			m_nCumLife;
	int			m_nYears;
	BOOL		m_bIsEvolving;
	BOOL		m_bPrePlace;

	CText		*pYears, *pYearsText1, *pYearsText2,
				*pColonyStat, *pColonyStatText1,
				*pScore, *pColonyPlaced, *pTime;

	CSprite		*pBaseSprite[VILLAGES];
	CCalendar	m_cCalendar;
};

#endif // __game_life_H__
