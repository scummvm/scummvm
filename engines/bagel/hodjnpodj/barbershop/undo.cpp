/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * undo.cpp					undoing card moves
 *
 * HISTORY
 *
 *      1.00      07/27/94     JSC     Initial Design
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
#include "undo.h"

/*****************************************************************
 *
 * [function name]
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
CUndo::CUndo()
{
	m_cUndoRect	= CRect(
						UNDO_LEF,
						UNDO_TOP,
						UNDO_RIG,
						UNDO_BOT
						);
	m_pStack	= NULL;
	m_pCard		= NULL;
	m_nStock	= NONE;
}

/*****************************************************************
 *
 * [function name]
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
CUndo::~CUndo()
{
}

/*****************************************************************
 *
 * [function name]
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
void CUndo::Record(CStack *pStack, CCard *pCard)
{
	m_pStack	= pStack;
	m_pCard		= pCard;
	m_nStock	= NONE;
}

/*****************************************************************
 *
 * [function name]
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
void CUndo::Record(int nCardsFlipped)
{
	m_nStock	= nCardsFlipped;

	m_pStack	= NULL;
	m_pCard		= NULL;
}

/*****************************************************************
 *
 * Reset
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Resets undo parameters
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
void CUndo::Reset()
{
	m_pStack	= NULL;
	m_pCard		= NULL;
	m_nStock	= NONE;
}

/*****************************************************************
 *
 * Undo
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *		Puts card back into original position.
 *      Returns TRUE if undo was possible, else FALSE.
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
BOOL CUndo::Undo(CDC *pDC, CBoard *pBoard, CPaint *pPaint)
{
	CCard	*pCard;
	CPoint	cPos;
	int 	i;

	if ( m_nStock != NONE ) {		// undo stock card flip?
		for ( i = 0; i < m_nStock; i++ ) {
			//
			// update interal representation
			//
			pCard = pBoard->GetStack((loc) used)->Pop();
			pBoard->GetStack((loc) stock)->Push(pCard);

			//
			// update visual rep
			//
			pPaint->FlipCard(pDC, pCard);
			pPaint->Stack(pDC, pCard);
			pPaint->m_nUsedInd--;
		} // end for

		Reset();					// clear undo
		return TRUE;
	} // end if

	if ( m_pStack == NULL )			// basic card undo?
		return FALSE;				// No - nothing can be undone


	m_pCard->m_pStack->Pop();		// undo move in internal rep
	m_pStack->Push(m_pCard);		// undo move in internal rep

	if ( m_pStack->GetID() == used ) {		// card in used stack?
	//
	// messy code to handle special visual used stacking order
	//
		if ( m_pCard->m_pPrevCard == NULL ) {
			pPaint->m_nUsedInd = 0;
		} else {
			if ( m_pCard->m_pPrevCard->m_cOrigin.x == m_pCard->m_pStack->m_cRect.TopLeft().x ) {
				pPaint->m_nUsedInd = 1;
			} else if ( m_pCard->m_pPrevCard->m_cOrigin.x == m_pCard->m_pStack->m_cRect.TopLeft().x + (1 * USED_DX) ) {
				pPaint->m_nUsedInd = 2;
			} else {
				pPaint->m_nUsedInd = 0;
			} // end if
		} // end if

		pPaint->Stack(pDC, m_pCard);	// draw card apro stack
		pPaint->m_nUsedInd = 0;
	} else {
		pPaint->Stack(pDC, m_pCard);	// draw card apro stack
	} // end if

	Reset();						// clear undo
	return TRUE;
}
