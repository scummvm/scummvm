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
#include "rec.h"

#define DECIMAL_BASE		10			// used in atoi() c function

#define	TIME_LEFT_COL		44			// time left box
#define	TIME_LEFT_ROW		30
#define TIME_RIGHT_COL		220
#define TIME_RIGHT_ROW		52

#define	SCORE_LEFT_COL		459			// score pos
#define SCORE_LEFT_ROW		30
#define SCORE_RIGHT_COL		574
#define SCORE_RIGHT_ROW	   	52

#define MINUTE				60
#define	STATS_COLOR			RGB(255,255,255)

#define	INI_FNAME			"boffo.ini"
#define INI_SECTION			"Cryptograms"

#define	INI_LETTERSSOLVED	"LettersSolved"
#define	LSLVD_DEFAULT		6
#define MIN_LSLVD			0
#define MAX_LSLVD			20

#define	INI_TIME			"Time"
#define TIME_DEFAULT		180
#define MIN_TIME			15
#define MAX_TIME			601

#define INI_REC				"Record"
#define REC_DEFAULT			0

#define	STAT_TIMER_ID		987			// wm_timer messages
#define INTERVAL			1000		// one second intervals

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
