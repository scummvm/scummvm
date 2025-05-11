/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * undo.h
 *
 * HISTORY
 *
 *      1.00      07/28/94     Josquin     Initial Design
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

#ifndef __game_undo_H__
#define __game_undo_H__

#include "paint.h"
#include "board.h"

#define NONE	-1

#define UNDO_LEF		432			// Undo hotspot rect
#define UNDO_TOP		80
#define UNDO_RIG		(UNDO_LEF + 98)
#define UNDO_BOT		(UNDO_TOP + 94)


class CUndo {
public:
	CUndo();
	~CUndo();

	void	Record(CStack*, CCard*);
	void	Record(int);
	void	Reset();
	BOOL	Undo(CDC*, CBoard*, CPaint*);

	CRect	m_cUndoRect;
private:
	CStack	*m_pStack;
	CCard	*m_pCard;
	int		m_nStock;
};

#endif // __game_undo_H__
