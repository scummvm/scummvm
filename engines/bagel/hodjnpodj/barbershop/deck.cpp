/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved.
 *
 *
 * deck.cpp
 *
 * HISTORY
 *
 *		1.0      04/19/94     JSC		First writing
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
#include <time.h>
#include <iostream.h>

#include "deck.h"
#include "board.h"
#include "stack.h"

CDeck::CDeck()
{
	int i;

	for ( i = 0; i < DECK; i++ )
		m_cDeck[i] = CCard(i);			// Reset deck

	srand((unsigned) time(NULL));		// seed the random number generator
}

CDeck::~CDeck()
{
}

void CDeck::Shuffle()
{
	int 	nNewDeck[DECK];
	int 	nCard;
	int		i;

	for ( i = 0; i < DECK; i++ )
		nNewDeck[i] = NOT_USED;

	/******************************
	* Find an unused encrypt map. *
	******************************/
	for ( i = 0; i < DECK; i++) {

		#ifndef REVEAL  						// change ifndef to ifdef for debugging purposes
		do {
			nCard = rand() % DECK;
		} while ( nNewDeck[nCard] == USED );	// find unshuffled card

		nNewDeck[nCard] = USED;					// mark card as shuffled

		#else
		nCard = i % 2;
		if ( nCard == 1 )
			nCard = CUST_CARD;
		#endif

		m_cDeck[i] = CCard(nCard);				// put card into it's new location
	}
}

void CDeck::Deal(CBoard *pBoard)
{
	int 	nStack, nCard, i;

	for ( nStack = fnd; nStack <= used; nStack++)
		pBoard->GetStack((loc) nStack)->Reset();


	for ( nStack = tab, i = 1, nCard = 0; nStack < stock; nStack++, i++ )
		for (; nCard < (i * (TAB_STACK)); nCard++ )
			pBoard->GetStack((loc) nStack)->Push(&m_cDeck[nCard]);

	for ( ; nCard < (STOCK + TABLEAU); nCard++ )
		pBoard->GetStack(stock)->Push(&m_cDeck[nCard]);
}
