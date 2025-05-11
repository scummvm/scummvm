/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  savegame.h -
*
*  HISTORY
*
*       1.00        08/19/94    BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*
*
*  RELEVANT DOCUMENTATION:
*
*
*
****************************************************************/
#ifndef RESTGAME_H
#define RESTGAME_H

#include "bfc.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/boflib/error.h"

// prototypes
//
#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (CALLBACK *LPSAVEFUNC)(const CHAR *, CBfcMgr *, CWnd *, CPalette *, ERROR_CODE *);

BOOL CALLBACK RestoreGame(const CHAR *, CBfcMgr *, CWnd *, CPalette *, ERROR_CODE *);
#ifdef __cplusplus
}
#endif

#endif // RESTGAME_H
