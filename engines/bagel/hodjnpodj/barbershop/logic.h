/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * logic.h
 *
 * HISTORY
 *
 *      1.00      03/28/94     XXX     [description of changes]
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

#include "stack.h"
#include "board.h"
#include "card.h"

class CLogic {
public:
	CLogic();
	~CLogic();

	BOOL	IsMoveOk(CCard*,CStack*);
	BOOL	IsGameOver(CBoard *pBoard);
};
