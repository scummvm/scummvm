/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved.
 *
 *
 * rec.cpp
 *
 * HISTORY
 *
 *		1.0      04/19/94     Jos     Creation
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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "rec.h"


CCryptRecord::CCryptRecord()
{
}

CCryptRecord::~CCryptRecord()
{
}


BOOL CCryptRecord::GetRecord(int nID)
{
	OFSTRUCT    ofstFileStat;
	char 		chBuf;
	int 		i;

	if ( (m_hfCryptFile = LZOpenFile(CRYPT_TXT_FILE, &ofstFileStat, OF_READ)) == -1 )
		return FALSE;

	m_nID = 0;
	for ( m_nID = 0; m_nID < nID; m_nID++ ) {			// Locate record
		LZSeek(
			m_hfCryptFile,
			RECORD_LEN * sizeof(char),
			1
			);											// Advance past initial quote mark
	}

	LZSeek(m_hfCryptFile, sizeof(char), 1); 			// Advance past initial quote mark

	for ( i = 0; ; i++ ) {								// Grab cryptogram
		if ( i >= MAX_GRAM_LEN )
			return FALSE;

		LZRead(m_hfCryptFile, &chBuf, sizeof(char));
		if ( chBuf == '\\' ) {
			LZRead(m_hfCryptFile, &chBuf, sizeof(char));

			m_lpszGram[i] = chBuf;
			i++;
		} else if ( chBuf == '\"' ) {
			m_lpszGram[i] = '\0';
			break;
		}

		m_lpszGram[i] = chBuf;
	}

	_fstrupr(m_lpszGram);
	LZSeek(m_hfCryptFile, 2 * sizeof(char), 1);			// Advance past dilimiting comma and initial quote mark

	for ( i = 0; ; i++ ) {			// Grab source
		if ( i >= MAX_SOURCE_LEN )
			return FALSE;

		 LZRead(m_hfCryptFile, &chBuf, sizeof(char));
		if ( chBuf == '\\' ) {
			LZRead(m_hfCryptFile, &chBuf, sizeof(char));
			m_lpszSource[i] = chBuf;
		} else if ( chBuf == '\"' ) {
			m_lpszSource[i] = '\0';
			break;
		}

		m_lpszSource[i] = chBuf;
	}

	_fstrupr(m_lpszSource);
	LZClose(m_hfCryptFile);
	return TRUE;
}
