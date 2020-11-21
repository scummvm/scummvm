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

#include <stdio.h>
#include <utility>
#include "script/script_common.h"  // current_line
#include "util/string.h"

using namespace AGS::Common;

// Returns full script error message and callstack (if possible)
extern std::pair<String, String> cc_error_at_line(const char *error_msg);
// Returns script error message without location or callstack
extern String cc_error_without_line(const char *error_msg);

int ccError = 0;
int ccErrorLine = 0;
String ccErrorString;
String ccErrorCallStack;
bool ccErrorIsUserError = false;
const char *ccCurScriptName = "";

void cc_error(const char *descr, ...)
{
    ccErrorIsUserError = false;
    if (descr[0] == '!')
    {
        ccErrorIsUserError = true;
        descr++;
    }

    va_list ap;
    va_start(ap, descr);
    String displbuf = String::FromFormatV(descr, ap);
    va_end(ap);

    if (currentline > 0)
    {
        // [IKM] Implementation is project-specific
        std::pair<String, String> errinfo = cc_error_at_line(displbuf);
        ccErrorString = errinfo.first;
        ccErrorCallStack = errinfo.second;
    }
    else
    {
        ccErrorString = cc_error_without_line(displbuf);
        ccErrorCallStack = "";
    }

    ccError = 1;
    ccErrorLine = currentline;
}
