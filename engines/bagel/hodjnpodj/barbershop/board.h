/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * board.h
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
#ifndef __game_board_H__
#define __game_board_H__

#include "paint.h"
#include "stack.h"

class CBoard {
public:		// functions
	CBoard(CPaint*);
	~CBoard();

	CStack* GetStack(loc);
	BOOL	IsTabStack(int);

public:		// vars
	CStack	*m_pFound;
	CStack	*m_pTab[TAB_COUNT];
	CStack	*m_pStock;
	CStack	*m_pUsed;
};

#endif // __game_board_H__
