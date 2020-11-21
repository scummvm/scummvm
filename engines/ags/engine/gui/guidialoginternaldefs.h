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
//
//
//
//=============================================================================
#ifndef __AGS_EE_GUI__GUIDIALOGINTERNALDEFS_H
#define __AGS_EE_GUI__GUIDIALOGINTERNALDEFS_H

#include "gui/guidialogdefines.h"

#define _export
#ifdef WINAPI
#undef WINAPI
#endif
#define WINAPI
extern int ags_misbuttondown (int but);
#define mbutrelease(X) (!ags_misbuttondown(X))
#define TEXT_HT usetup.textheight

#endif // __AGS_EE_GUI__GUIDIALOGINTERNALDEFS_H
