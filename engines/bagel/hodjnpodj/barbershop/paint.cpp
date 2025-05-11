/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * pnt_card.cpp					Paints cards as visual sprites
 *
 * HISTORY
 *
 *      1.00      04/19/94     JOS    Initial Design
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

#include "paint.h"
#include "barb.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"

/*****************************************************************
 *
 * CPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
CPaint::CPaint(CDC *pDC)
{
	CBitmap	*pBmpCardSet = NULL;
	CBitmap	*pCard = NULL;
	BOOL	bSuccess;
	int 	i, j;

	for ( i = 0; i < CARDS_PER_COL; i++ ) {	// Initialize master alphabet list
		pBmpCardSet = FetchBitmap(pDC, (CPalette**) NULL, lpszDeck[i]);
		ASSERT(pBmpCardSet);

		for ( j = 0; j < CARDS_PER_ROW; j ++ ) {
			pCard = ExtractBitmap(			// fetch the proper card
						pDC,
						pBmpCardSet,
						pGamePalette,
						j * BITMAP_WTH,
						0,
						BITMAP_WTH,
						BITMAP_HEI
						);

			ASSERT(pCard);

			m_cCardSet[(i * CARDS_PER_ROW) + j] = new OSpr();	// Initize the individual letter of the alphabet list
			bSuccess = (*m_cCardSet[(i * CARDS_PER_ROW) + j]).LoadSprite(pCard, pGamePalette);
			ASSERT(bSuccess);

			(*m_cCardSet[(i * CARDS_PER_ROW) + j]).SetHotspot(CARD_HOT_X, CARD_HOT_Y);
			(*m_cCardSet[(i * CARDS_PER_ROW) + j]).SetMobile(TRUE);
			(*m_cCardSet[(i * CARDS_PER_ROW) + j]).SetMasked(TRUE);
		}  // end for

		if ( pBmpCardSet != NULL ) {
			delete pBmpCardSet;
			pBmpCardSet = NULL;
		} // end if
	}  // end for

	m_nCardBack = CARD_BACK1;
	m_nUsedInd = 0;	// Used in Stack() proc; for indenting cards placed on used stack

	m_cFound.SetRect(
				FND_LEF,
				FND_TOP,
				FND_RIG,
				FND_BOT
				);

	for ( i = 0; i < TAB_COUNT; i++ )
		m_cTab[i].SetRect(
				TAB_LEF + (TAB_OFFSET * i),
				TAB_TOP,
				TAB_RIG + (TAB_OFFSET * i),
				TAB_BOT
				);

	m_cStock.SetRect(
				STOC_LEF,
				STOC_TOP,
				STOC_RIG,
				STOC_BOT
				);

	m_cUsed.SetRect(
				STOC_LEF + USED_OFFSET,
				STOC_TOP,
				STOC_RIG + USED_OFFSET,
				STOC_BOT
				);
}

/*****************************************************************
 *
 * ~CPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
CPaint::~CPaint()
{
	int i;

	CSprite::FlushSpriteChain();
	for ( i = 0; i < CARD_SET; i++)
		delete m_cCardSet[i];			// each letter in the alpha
}

/*****************************************************************
 *
 * Board
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaint::Board(CDC *pDC, CBoard *pBoard)
{
	int 	i, j;
	CCard	*pCard[TAB_COUNT];

	/************************************************************
	* Goes thru each stack on the tableau, dealing cards one at	*
	* a time rotating to next stack, in turn.					*
	************************************************************/
	for ( i = 0; i < TAB_COUNT; i++ )
		pCard[i] = pBoard->GetStack((loc) (tab + (loc) i))->Bottom();

	for ( i = 0; i < TABLEAU; i++ ) {
		j = i % TAB_COUNT;

		m_pSprite = new OSpr();
		if ( pCard[j] == pCard[j]->m_pStack->Top() ) {
   			m_cCardSet[pCard[j]->GetValue()]->DuplicateSprite(pDC, (CSprite*) m_pSprite);
			m_pSprite->m_cCard = pCard[j];
			(*m_pSprite).LinkSprite();

			pCard[j]->m_bIsBack = FALSE;
		} else {
   			m_cCardSet[m_nCardBack]->DuplicateSprite(pDC, (CSprite*) m_pSprite);
			m_pSprite->m_cCard = pCard[j];
			(*m_pSprite).LinkSprite();

			pCard[j]->m_bIsBack = TRUE;
		} // end if

		pCard[j]->m_pSprite	= m_pSprite;
		Stack(pDC, pCard[j], (int) i / TAB_COUNT);
		pCard[j] = pCard[j]->m_pNextCard;			// Advance to next card
	} // end for

	/************************************************************
	* used any random pCard index, didn't matter, so 0 is used.	*
	************************************************************/
	pCard[0]	= pBoard->GetStack(stock)->Bottom();		// Get first card in stack
	i			= 0;
	while ( pCard[0] != NULL ) {
		m_pSprite = new OSpr();				// set up visual sprite
		m_cCardSet[m_nCardBack]->DuplicateSprite(pDC, (CSprite*) m_pSprite);
		(*m_pSprite).LinkSprite();

		m_pSprite->m_cCard	= pCard[0];		// update internal card struct
		pCard[0]->m_pSprite	= m_pSprite;
		pCard[0]->m_bIsBack	= TRUE;

		Stack(pDC, pCard[0], i);					// paint it on screen
		pCard[0] = pCard[0]->m_pNextCard;			// Advance to next card
		i++;
	} // end while
}

void CPaint::Refresh(CDC *pDC, CBoard *pBoard)
{
	int 	i;
	int		nCardPos;							// card pos in stack
	CStack	*pStack;
	CCard	*pCard;
	CPoint	cPos;

	m_pSprite = (OSpr*) OSpr::GetSpriteChain();
	if ( m_pSprite == NULL )					// any sprites to refresh?
		return;									// no

	OSpr::ClearBackgrounds();

	pStack		= pBoard->GetStack(fnd);  		// refresh foundation
	pCard		= pStack->Bottom();
	nCardPos	= 0;
	while ( pCard != NULL ) {
		if ( pCard->m_pSprite != NULL ) {
			pCard->m_pSprite->RefreshSprite(pDC);
		}
		pCard = pCard->m_pNextCard;
		nCardPos++;
	}

	for ( i = tab; i < stock; i++ ) {			// refresh tableau
		pStack	= pBoard->GetStack((loc) i);
		pCard	= pStack->Bottom();
		nCardPos = 0;

		while ( pCard != NULL ) {
			if ( pCard->m_pSprite != NULL ) {
				pCard->m_pSprite->RefreshSprite(pDC);
			}
			pCard = pCard->m_pNextCard;
			nCardPos++;
		} // end while
	} // end for

	pStack		= pBoard->GetStack(stock);		// refresh stock top card only
	pCard		= pStack->Bottom();
	nCardPos	= 0;
	while ( pCard != NULL ) {
		if ( pCard->m_pSprite != NULL ) {
			pCard->m_pSprite->RefreshSprite(pDC);
		}
		pCard = pCard->m_pNextCard;
		nCardPos++;
	}

	pStack		= pBoard->GetStack(used);		// refresh used stack
	pCard		= pStack->Bottom();
	nCardPos	= 0;
	while ( pCard != NULL ) {
		if ( pCard->m_pSprite != NULL ) {
			pCard->m_pSprite->RefreshSprite(pDC);
		}
		pCard = pCard->m_pNextCard;
		nCardPos++;
	}
}

CCard* CPaint::IsOnCard(CPoint cPoint)
{
	if ( (m_pSprite = (OSpr *) m_pSprite->Touched(cPoint)) == NULL )
		return NULL;

	return m_pSprite->m_cCard->m_pStack->Top();
}

void CPaint::MoveCard(CDC *pDC, CCard *pCard, CPoint point)
{
	point.x -= pCard->m_pSprite->GetHotspot().x;	// causes the point to appear in the
	point.y -= pCard->m_pSprite->GetHotspot().y;	// center of the card.
	pCard->m_pSprite->PaintSprite(pDC, point);
}

void CPaint::UpdateCard(CDC *pDC, CCard *pCard)
{
	pCard->m_pSprite->PaintSprite(pDC, pCard->m_cOrigin);	// paint card @ cur pos
}

void CPaint::FlipCard(CDC *pDC, CCard *pCard)
{
	/**********************************
	* Get rid of current card bitmap. *
	**********************************/
	pCard->m_pSprite->RefreshBackground(pDC);	// ...repaint background
	pCard->m_pSprite->UnlinkSprite();			// unlink from refresh chain
	delete pCard->m_pSprite;

	/******************************************************************
	* Reverse card. If it is a back, flip it forward, and vise versa. *
	******************************************************************/
	pCard->m_pSprite = new OSpr();
	if ( pCard->m_bIsBack == TRUE ) {
		(*m_cCardSet[pCard->GetValue()]).DuplicateSprite(pDC, (CSprite*) pCard->m_pSprite);
		pCard->m_bIsBack = FALSE;
	} else {
		(*m_cCardSet[m_nCardBack]).DuplicateSprite(pDC, (CSprite*) pCard->m_pSprite);
		pCard->m_bIsBack = TRUE;
	}

	pCard->m_pSprite->LinkSprite();
	pCard->m_pSprite->m_cCard = pCard;						// update internal rep
}

/*****************************************************************
 *
 * Stack
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Indents top card properly relative to it's current stack.
 * Assigns pCard->m_cOrigin to correct position.
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaint::Stack(CDC *pDC, CCard *pCard, int nSize)
{
	int	i;

	if ( pCard == NULL )
		return;

	if ( nSize == -1 ) {				// funct overloaded?
		i = pCard->m_pStack->Size() - 1;// no, use stack size info
	} else {
		i = nSize;						// yes, use info from param
	}

	switch ( pCard->m_pStack->GetID() ) { 	 	// Update card visual coordinates

		case fnd:
			pCard->m_cOrigin = pCard->m_pStack->m_cRect.TopLeft();
			pCard->m_cOrigin.x += ((int) i / STACK_INDENT) * STACK_DX;
			pCard->m_cOrigin.y += ((int) i / STACK_INDENT) * STACK_DY;
			break;

		case stock:
			pCard->m_cOrigin = pCard->m_pStack->m_cRect.TopLeft();
			pCard->m_cOrigin.x += ((int) i / STACK_INDENT) * STACK_DX;
			pCard->m_cOrigin.y += ((int) i / STACK_INDENT) * STACK_DY;
			break;

		case used:
			if ( pCard->m_pStack->Bottom() == pCard )	// bottom card?
				m_nUsedInd = 0;							// reset stack offset counter

			pCard->m_cOrigin = pCard->m_pStack->m_cRect.TopLeft();
			pCard->m_cOrigin.x += ((int) m_nUsedInd % USED_INDENT) * USED_DX;
			pCard->m_cOrigin.y += ((int) m_nUsedInd % USED_INDENT) * USED_DY;

			m_nUsedInd++;
			break;

		default:
			if ( pCard->m_pPrevCard != NULL ) {  // card face req pos?
				if ( pCard->m_pPrevCard->m_bIsBack == FALSE ) {
					pCard->m_cOrigin.x = pCard->m_pPrevCard->m_cOrigin.x + STACK_FACE_DX;
					pCard->m_cOrigin.y = pCard->m_pPrevCard->m_cOrigin.y + STACK_FACE_DY;
					break;
				} // end if
			}

			//
			// Card back requires positioning
			//
			pCard->m_cOrigin = pCard->m_pStack->m_cRect.TopLeft();
			pCard->m_cOrigin.x += ((int) i / TAB_INDENT) * TAB_DX;
			pCard->m_cOrigin.y += ((int) i / TAB_INDENT) * TAB_DY;
			break;
	}

	pCard->m_pSprite->PaintSprite(pDC, pCard->m_cOrigin);
}

/*****************************************************************
 *
 * IsNewBack
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
BOOL CPaint::IsNewBack(int nBack)
{
	if ( m_nCardBack == nBack )	{	// any change?
		return FALSE;
	} else {
		return TRUE;
	}
}

/*****************************************************************
 *
 * ChangeBack
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaint::ChangeBack(CDC *pDC, CBoard *pBoard, int nBack)
{
	int 	i;
	CStack	*pStack;
	CCard	*pCard;

	if ( m_nCardBack == nBack )	{	// any change?
		return;						// no - just return
	} else {
		m_nCardBack = nBack;
	}

	for ( i = fnd; i <= used; i++ ) {			// refresh tableau
		pStack	= pBoard->GetStack((loc) i);
		pCard	= pStack->Bottom();

		while ( pCard != NULL ) {
			if ( pCard->m_bIsBack == FALSE ) {
				pCard = pCard->m_pNextCard;
				continue;
			}

			pCard->m_pSprite->UnlinkSprite();			// unlink from refresh chain
			delete pCard->m_pSprite;

			pCard->m_pSprite = new OSpr();
			(*m_cCardSet[m_nCardBack]).DuplicateSprite(pDC, (CSprite*) pCard->m_pSprite);
			pCard->m_pSprite->LinkSprite();
			pCard->m_pSprite->SetPosition(pCard->m_cOrigin);
			pCard->m_pSprite->m_cCard = pCard;						// update internal rep

			pCard = pCard->m_pNextCard;
		} // end while
	} // end for
}
