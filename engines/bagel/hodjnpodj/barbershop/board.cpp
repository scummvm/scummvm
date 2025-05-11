/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved.
 *
 *
 * board.cpp
 *
 * HISTORY
 *
 *		1.0      06/15/94     JSC     Creation
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
#include "board.h"

CBoard::CBoard(CPaint *m_cPnt)
{
	int i;

	m_pFound = new CStack(m_cPnt->m_cFound, fnd);

	for ( i = 0; i < TAB_COUNT; i++ )
		m_pTab[i] = new CStack(m_cPnt->m_cTab[i], tab + i);

	m_pStock = new CStack(m_cPnt->m_cStock, stock);
	m_pUsed	 = new CStack(m_cPnt->m_cUsed, used);
}

CBoard::~CBoard()
{
	int i;

	if ( m_pUsed != NULL ) {
		delete m_pUsed;
		m_pUsed = NULL;
	}

	if ( m_pStock != NULL ) {
		delete m_pStock;
		m_pStock = NULL;
	}

	for ( i = 0; i < TAB_COUNT; i++ ) {
		if ( m_pTab[i] != NULL ) {
			delete m_pTab[i];
			m_pTab[i] = NULL;
		}
	}

	if ( m_pFound != NULL ) {
		delete m_pFound;
		m_pFound = NULL;
	}
}

CStack* CBoard::GetStack(loc nStack)
{
	int i;

	switch ( nStack ) {

		case fnd:
			return m_pFound;

		case stock:
			return m_pStock;

		case used:
			return m_pUsed;

		default:
			for ( i = 0; i < TAB_COUNT; i++ )
				if ( m_pTab[i]->GetID() == nStack )
					return m_pTab[i];
	}

	return NULL;
}

BOOL CBoard::IsTabStack(int nStack)
{
	switch ( nStack ) {

		case fnd:
			return FALSE;

		case stock:
			return FALSE;

		case used:
			return FALSE;

		default:
			return TRUE;
	}
}
