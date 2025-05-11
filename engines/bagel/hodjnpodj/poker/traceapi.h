/* traceapi.h: Defines TRACER.DLL application interface
// This interface can be included by C or C++ code
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.
*/

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

HWND FAR PASCAL RunPoker( HWND, LPGAMESTRUCT );

BOOL FAR PASCAL FilterDllMsg(LPMSG lpMsg);
void FAR PASCAL ProcessDllIdle();

#ifdef __cplusplus
}
#endif
