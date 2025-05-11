/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * rec.h
 *
 * HISTORY
 *
 *      1.00      04/19/94     Jos     Creation
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


#ifndef __game_deck_H__
#define __game_deck_H__

#include "card.h"

#define	DECK		62
#define	STOCK		27

#define NOT_USED	-1
#define USED		0

class CBoard;

class CDeck {
public:	// func

	CDeck();		// constructor
	~CDeck();		// destructor

	void	Shuffle();
	void	Deal(CBoard*);

public:	// vars
	CCard	m_cDeck[DECK];
};
#endif // __game_rec_H__
