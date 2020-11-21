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
// 'C'-style script compiler
//
//=============================================================================

#ifndef __CC_ERROR_H
#define __CC_ERROR_H

#include "util/string.h"

extern void cc_error(const char *, ...);

// error reporting
extern int ccError;             // set to non-zero if error occurs
extern int ccErrorLine;         // line number of the error
extern AGS::Common::String ccErrorString; // description of the error
extern AGS::Common::String ccErrorCallStack; // callstack where error happened
extern bool ccErrorIsUserError;
extern const char *ccCurScriptName; // name of currently compiling script

#endif // __CC_ERROR_H
