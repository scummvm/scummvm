/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved.
 *
 *
 * cstats.cpp
 *
 * HISTORY
 *
 *		1.0      05/05/94     Josquin     Creation
 *
 * MODULE DESCRIPTION:
 *
 *      [Describe the function of the module]
 *
 * CONSTRUCTORS:
 *
 *      [list constructors with one line descriptions]
 *
 * DESTRUCTORS:
 *
 *      [list destructors with one line descriptions]
 *
 * PUBLIC:
 *
 *      [list public routines with one line descriptions]
 *
 * PUBLIC GLOBAL:
 *
 *      [list global public routines with one line descriptions]
 *
 * PROTECTED:
 *
 *      [list protected routines with one line descriptions]
 *
 * PRIVATE:
 *
 *      [list private routines with one line descriptions]
 *
 * MEMBERS:
 *
 *      [list members of the class with one line descriptions]
 *
 * RELEVANT DOCUMENTATION:
 *
 *      [Specifications, documents, test plans, etc.]
 *
 ****************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "stats.h"

CStats::CStats()
{
	m_pTime	= NULL;
	m_pScore = NULL;

	m_nLettersSolved = GetPrivateProfileInt(
								INI_SECTION,
								INI_LETTERSSOLVED,
								LSLVD_DEFAULT,
								INI_FILENAME
								);

	if ( m_nLettersSolved < MIN_LSLVD || m_nLettersSolved > MAX_LSLVD )
		m_nLettersSolved = LSLVD_DEFAULT;

	m_nScore = 0;

	m_nTime = GetPrivateProfileInt(
		   					INI_SECTION,
		   					INI_TIME,
		   					TIME_DEFAULT,
		   					INI_FILENAME
		   					);


	if ( m_nTime < MIN_TIME || m_nTime > MAX_TIME )
		m_nTime = TIME_DEFAULT;

	m_nCountDown = m_nTime;

	m_nIsUsedGram = GetPrivateProfileInt(
	  						INI_SECTION,
		   					INI_REC,
		   					REC_DEFAULT,
		   					INI_FILENAME
	   						);
}


CStats::~CStats()
{
	if ( m_pScore != NULL ) {
		delete m_pScore;
		m_pScore = NULL;
	}

	if ( m_pTime != NULL ) {
		delete m_pTime;
		m_pTime = NULL;
	}
}


int CStats::ResetGame()
{
    //char 	chResetUsedGram;
	char	tmpBuf[5];
    int		nID;
    //int 	i;

    /*************************
    * Reset crytogram stats. *
    *************************/
   	m_nCountDown 	= m_nTime;
	m_nScore 		= 0;

	#ifdef REVEAL
    /************************************************************
	* Need to find a new crytogram for use.						*
    * Want to determine if we have seen all of the cyrptograms. *
    ************************************************************/
    for ( i = 0, chResetUsedGram = SEEN; i < CRYPT_RECS; i++ ) {	// Scan entire used list looking for one unused slot
    	if ( m_chIsUsedGram[i] == NOT_SEEN ) {						// Is this an unused slot?
    		chResetUsedGram = NOT_SEEN;								// Yes - break out of here
    		break;
    	}
    }

    if ( chResetUsedGram == SEEN ) 									// Have we seen all cryptograms?
		for ( i = 0; i < CRYPT_RECS; i++ )							// Yes - reset list
			m_chIsUsedGram[i] = NOT_SEEN;
	#endif

	do {															// Get random unused cryptogram
		nID = brand() % CRYPT_RECS;
	} while ( m_nIsUsedGram == nID );

	m_nIsUsedGram = nID;											// Mark as used

	WritePrivateProfileString(
						INI_SECTION,
	   					INI_REC,
	   					itoa(m_nIsUsedGram, tmpBuf, DECIMAL_BASE),
						INI_FILENAME
						);											// Save used list back

	return nID;
}


void CStats::SaveStats(int nLttrsSlvd, int nTime)
{
	char	tmpBuf[5];


    m_nLettersSolved	= nLttrsSlvd;
    m_nTime				= nTime;

	WritePrivateProfileString(
						INI_SECTION,
						INI_LETTERSSOLVED,
						itoa(m_nLettersSolved, tmpBuf, DECIMAL_BASE),
						INI_FILENAME
						);

	WritePrivateProfileString(
						INI_SECTION,
						INI_TIME,
						itoa(m_nTime, tmpBuf, DECIMAL_BASE),
						INI_FILENAME
						);
}
