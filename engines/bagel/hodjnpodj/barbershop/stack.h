/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * stack.h
 *
 * HISTORY
 *
 *      1.00      04/19/94     Josquin     Initial Design
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

#ifndef __game_stack_H__
#define __game_stack_H__

#include "card.h"

#define STACKS	8


class CStack {
private:	// vars
	int		m_nID;
	CCard	*m_pTopCard;
	CCard	*m_pBottomCard;
	int		m_nSize;

public:		// functions

	CStack(CRect, int nID);
	~CStack();

	void	Reset();
	void	Unlink(CCard*);
	void	Push(CCard*);
	CCard*	Pop();
	CCard*	Top() const { return (m_pTopCard); }
	CCard*	Bottom() const { return (m_pBottomCard); }
	BOOL	IsEmpty() { return (BOOL)(m_pTopCard == NULL); }
	int		GetID() { return (m_nID); }
	BOOL	IsTab() { return (BOOL)(m_nID >= tab && m_nID < stock); }
	int 	Size();

public:
	CRect	m_cRect;
};

#endif // __game_stack_H__


