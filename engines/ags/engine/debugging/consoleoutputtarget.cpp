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

#include <string.h>
#include "consoleoutputtarget.h"
#include "debug/debug_log.h"

namespace AGS
{
namespace Engine
{

ConsoleOutputTarget::ConsoleOutputTarget()
{
}

ConsoleOutputTarget::~ConsoleOutputTarget() = default;

void ConsoleOutputTarget::PrintMessage(const DebugMessage &msg)
{
    // limit number of characters for console
    // TODO: is there a way to find out how many characters can fit in?
    debug_line[last_debug_line] = msg.Text.Left(99);

    last_debug_line = (last_debug_line + 1) % DEBUG_CONSOLE_NUMLINES;
    if (last_debug_line == first_debug_line)
        first_debug_line = (first_debug_line + 1) % DEBUG_CONSOLE_NUMLINES;
}

} // namespace Engine
} // namespace AGS
