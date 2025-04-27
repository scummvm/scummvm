/*****************************************************************
 *
 *  encount.h
 *
 *  Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *  HISTORY
 *
 *      1.0     07/14/94     BAR     refer to encount.cpp for details
 *
 *  MODULE DESCRIPTION:
 *
 *      definitions for DoEncounter and it's subroutines
 *   
 *  RELEVANT DOCUMENTATION:
 *
 *      n/a
 *   
 ****************************************************************/
#include "mgstat.h"                   
#include "invent.h"

#define	TRAP_PER_SECTOR		  2
#define	NARR_PER_SECTOR		  8
#define	MG_TRAP_COUNT		 60
#define	MG_NARR_COUNT		180
#define	MG_ENC_COUNT		(MG_TRAP_COUNT + MG_NARR_COUNT)

#define TRAP_PROB			 20			// 20% of the time, set a trap  ( 2 out of 10 times )
#define	NARR_PROB			  3			// 3 * number of steps
#define	PROB_FACTOR			 10			// raise percentages by a factor of ten
#define	RAND_FACTOR		    100			// 100%

#define	MG_ACT_HODJ			288
#define MG_ACT_PODJ			289

// Value of Two objects, MISH and MOSH
#define	ENC_MISHMOSH		2

// Input whether they want a boobytrap or narration
#define	ENC_BOOBYTRAP		0
#define ENC_NARRATION		1

// RETURN values for the DoEncounter function
#define	ENC_DO_NOTHING		0
#define ENC_LOSE_TURN		1
#define	ENC_GAIN_TURN		2

#define MAX_ACTIONS         5
                   

// CEncounterTable definition, used for both Booby Traps and Colour Narrations

class CEncounterTable {
public:
    int     m_iSectorCode ;         // MG_SECTOR_xxxx: sector code
    char    *m_lpszWavEFile;        // name of 8-bit sound file (*m_pItemSoundPath)
//    char  *m_lpszWavSFile;        // name of 16-bit sound file
    int     m_Actions[MAX_ACTIONS]; // string of action codes, set to NULL for Colour Narrations
    char    *m_lpszText ;           // text of booby trap
} ;


int DoEncounter( CWnd *pWnd, CPalette *pPalette, BOOL bHodj, CInventory *pInventory, 
					CInventory *pPawn, CInventory *pGeneral,
					 int EncType, int nNumSteps, 
					 int nPSector, int nOSector, BOOL *pArray );
