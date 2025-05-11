/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * stack.cpp
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
#include "stack.h"

CStack::CStack(CRect cRect, int nID)
{
	m_pTopCard		= NULL;
	m_pBottomCard	= NULL;
	m_nID			= nID;
	m_nSize			= 0;
	m_cRect			= cRect;
}

CStack::~CStack()
{
}

void CStack::Reset()
{
	m_pTopCard		= NULL;
	m_pBottomCard	= NULL;
	m_nSize			= 0;
}

void CStack::Push(CCard *pCard)
{
	if ( pCard == NULL )
		return;

	/*********************************************************
	* Update card pointers aligning them with current stack. *
	*********************************************************/
	pCard->m_pPrevCard	= m_pTopCard;	// link new top prev to old top card
	pCard->m_pNextCard	= NULL;			// new top next card now does not point anywhere

	pCard->m_pStack		= this;			// reassign proper stack location

	/*************************
	* Update stack pointers. *
	*************************/
	if ( m_pTopCard != NULL )
		m_pTopCard->m_pNextCard = pCard;    // create forward chain for next card

	m_pTopCard = pCard;						// link old top card to new top

	if ( m_pBottomCard == NULL )
		m_pBottomCard = pCard;

	m_nSize++;
}

CCard* CStack::Pop()
{
	CCard *pReturnCard;

	/**************************************************
	* Undo any links of current card to other stacks. *
	**************************************************/
	pReturnCard = m_pTopCard;

	if ( m_pTopCard == NULL ) {
		m_nSize = 0;
		return NULL;
	}

	if ( m_pTopCard->m_pPrevCard != NULL ) {
		m_pTopCard->m_pPrevCard->m_pNextCard = NULL;
		m_pTopCard = m_pTopCard->m_pPrevCard;

		if ( m_pTopCard->m_pNextCard != NULL )
			m_pTopCard->m_pNextCard->m_pPrevCard = m_pTopCard->m_pPrevCard;	// make cur prev card point to prev of prev card :^) (if any)

		m_nSize--;
	} else {
		m_pTopCard		= NULL;
		m_pBottomCard	= NULL;
		m_nSize			= 0;
	}

	return (pReturnCard);
}

void CStack::Unlink(CCard *pCard)
{
	/**************************************************
	* Undo any links of current card to other stacks. *
	**************************************************/
	if ( pCard->m_pNextCard != NULL ) {
		pCard->m_pNextCard->m_pPrevCard = pCard->m_pPrevCard;	// make cur prev card point to prev of prev card :^) (if any)
		pCard->m_pNextCard = NULL;
	}

	if ( pCard->m_pPrevCard != NULL ) {
		pCard->m_pPrevCard->m_pNextCard = pCard->m_pNextCard;	// make cur of prev of next pointer point to next of the next (if any)
		this->m_pTopCard = pCard->m_pPrevCard;					// Top of stack points to the prev card
		pCard->m_pPrevCard = NULL;								// clear the card "prev card" pointer
	} else {
		m_pTopCard		= NULL;
		m_pBottomCard	= NULL;
	}

	/**************************************************
	* Card's stack pointer should not point anywhere. *
	**************************************************/
	pCard->m_pStack = NULL;										// clear card "stack" pointer
}

int CStack::Size()
{
	return m_nSize;
}

