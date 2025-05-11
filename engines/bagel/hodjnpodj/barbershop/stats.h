/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * cstats.h
 *
 * HISTORY
 *
 *      1.00      05/05/94     Josquin     Initial Design
 *
 * MODULE DESCRIPTION:
 *
 *      [Describe the function of the module]
 *
 * RELEVANT DOCUMENTATION:
 *
 *      [Specifications, documents, test plans, etc.]
 *
 ****************************************************************/

#include "text.h"
//#include "rec.h"

#define	STATS_COLOR			RGB(255,255,255)

#define SCORE_FACTOR		2
#define	SCORE_JACKPOT		100

class CStats {
public:
	CStats();
	~CStats();

	void 	SaveStats(int nLttrsSlvd, int nTime);
	int		ResetGame();

	CText	*m_pScore;
	CText	*m_pTime;

	int		m_nLettersSolved;
	int		m_nScore;
	int		m_nTime;
	int		m_nCountDown;
	char	m_nIsUsedGram;
};
