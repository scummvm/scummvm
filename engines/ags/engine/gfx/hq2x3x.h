//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AC_HQ2X3X_H
#define __AC_HQ2X3X_H

#include "core/platform.h"

#if AGS_PLATFORM_OS_ANDROID
void InitLUTs(){}
void hq2x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL ){}
void hq3x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL ){}
#else
void InitLUTs();
void hq2x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );
void hq3x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );
#endif

#endif // __AC_HQ2X3X_H