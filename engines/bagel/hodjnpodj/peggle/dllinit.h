/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * dllinit.h
 *
 * HISTORY
 *
 *	1.0 5/13/94 GTB		
 *
 * MODULE DESCRIPTION:
 *
 *
 * LOCALS:
 *
 *
 * GLOBALS:
 *
 *
 * RELEVANT DOCUMENTATION:
 *
 *      n/a
 *
 * FILES USED:
 *
 ****************************************************************/
#ifndef __dllinit_H__
#define __dllinit_H__


#include "gamedll.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

HWND FAR PASCAL _export RunPeggle( HWND, LPGAMESTRUCT =NULL );

BOOL FAR PASCAL _export FilterDllMsg(LPMSG lpMsg);
void FAR PASCAL _export ProcessDllIdle();

#ifdef __cplusplus
}
#endif

#endif // __dllinit_H__