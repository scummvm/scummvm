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
// Script Editor run-time engine component (c) 1998 Chris Jones
// script chunk format:
// 00h  1 dword  version - should be 2
// 04h  1 dword  sizeof(scriptblock)
// 08h  1 dword  number of ScriptBlocks
// 0Ch  n STRUCTs ScriptBlocks
//
//=============================================================================

#include <stdlib.h>
#include "script/cc_instance.h"
#include "script/cc_error.h"
#include "util/file.h"
#include "util/stream.h"

namespace AGS { namespace Common { class RoomStruct; } }
using namespace AGS::Common;

extern void quit(const char *);
extern int currentline; // in script/script_common

std::pair<String, String> cc_error_at_line(const char *error_msg)
{
    ccInstance *sci = ccInstance::GetCurrentInstance();
    if (!sci)
    {
        return std::make_pair(String::FromFormat("Error (line %d): %s", currentline, error_msg), String());
    }
    else
    {
        return std::make_pair(String::FromFormat("Error: %s\n", error_msg), ccInstance::GetCurrentInstance()->GetCallStack(5));
    }
}

String cc_error_without_line(const char *error_msg)
{
    return String::FromFormat("Runtime error: %s", error_msg);
}
