/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * card.cpp
 *
 * HISTORY
 *
 *      1.00      06/13/94	JOS    Initial Design
 *
 * MODULE DESCRIPTION:
 *
 *      [Describe the purpose of this module]
 *
 * LOCALS:
 *
 *      MyFunction()            Description of MyFunction
 *
 * GLOBALS:
 *
 *      MyFunction()            Description of MyFunction
 *
 * RELEVANT DOCUMENTATION:
 *
 *      [Specifications, documents, test plans, etc./]
 *
 * FILES USED:
 *
 *      [Discuss files created, or used]
 *
 ****************************************************************/
#include "stdafx.h"
#include "card.h"

OSpr::OSpr() :CSprite()
{
	m_cCard = NULL;
}

CCard::CCard()
{
	m_enSuit	= (suit) suit_none;
	m_nPip		= 0;
	m_pPrevCard = NULL;
	m_pNextCard = NULL;
	m_pSprite	= NULL;
	m_pStack	= NULL;
	m_bIsBack	= FALSE;
	m_cOrigin	= CPoint(0,0);
}

CCard::CCard(int nValue)
{
	if ( nValue >= SUITS * PIPS ) {					// Is this a joker card?
		m_enSuit	= manicurist;					// Yes - assign joker suit
		m_nPip		= MANI_CARD;					//  and joker pip
	} else {
		m_enSuit = (suit) (nValue % SUITS);			// No - assign usual suit
		m_nPip		= nValue % PIPS;
	}

	m_pPrevCard = NULL;
	m_pNextCard = NULL;
	m_pSprite	= NULL;
	m_pStack	= NULL;
	m_bIsBack	= FALSE;
	m_cOrigin	= CPoint(0,0);
}

CCard::CCard(suit enSuit, int nPip)
{
	m_enSuit 	= enSuit;
	m_nPip 		= nPip;
	m_pPrevCard = NULL;
	m_pNextCard = NULL;
	m_pSprite	= NULL;
	m_pStack	= NULL;
	m_bIsBack	= FALSE;
	m_cOrigin	= CPoint(0,0);
}

CCard::~CCard()
{
}

int CCard::GetValue() 
{ 
	if ( m_enSuit == manicurist ) {	// Is Joker suit?
		return m_nPip;				// return it's special pip
	} else {
		return (m_nPip + (m_enSuit * PIPS)); 
	}
}

CCard& CCard::operator=(const CCard& cCard)
{
	m_enSuit	= cCard.m_enSuit;		// Private members
	m_nPip		= cCard.m_nPip;

	m_pPrevCard = cCard.m_pPrevCard;	// Public members
	m_pNextCard = cCard.m_pNextCard;
	m_pStack	= cCard.m_pStack;
	m_bIsBack	= cCard.m_bIsBack;
	m_pSprite	= cCard.m_pSprite;
	m_cOrigin	= cCard.m_cOrigin;

	return (*this);
}
