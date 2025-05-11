/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * [file name].h
 *
 * HISTORY
 *
 *      1.00      03/28/94     Josquin     Creation
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

#include "rec.h"
#include "pnt_gram.h"
#include "stats.h"

#define SPLASHSPEC		".\\art\\crypt.BMP"	// bitmap file for the splash screen

#define NOT_USED		-1
#define	MAP				2
#define	DECRYPT_MAP		0
#define	ENCRYPT_MAP		1

class CCryptogram {
private:
	CCryptRecord	*m_cRecordGram;
	int				m_nCryptMap[MAP][ALPHABET];

public:
	CCryptogram(CDC *pDC);
	~CCryptogram();

	BOOL		DrawGram(CDC *pDC);
	BOOL		HandleUserUpdate(CDC *pDC, CPoint cpointClicked);
	BOOL		HandleUserUpdate(CDC *pDC, UINT nChar);
	void		Encrypt();
	void		CreateCryptMap(int nLettersSolved);
	int			UpdateCryptMap(int nOldType, int nNewType);
	BOOL		IsSolved();
	int			LettersSolved();
	void		MarkSolvedLetters(CDC *pDC);
	void		SolveCryptogram(CDC *pDC);
	void		DrawSource(CDC *pDC);

	char	 	m_chEncryptGram[MAX_GRAM_LEN];
	CPaintGram	*m_cPaintGram;
	CStats		*m_cStats;
	BOOL 		bIsGameOver;
};

// Globals!
extern CPalette		*pGamePalette;
