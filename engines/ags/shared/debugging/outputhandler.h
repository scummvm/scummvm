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
// IOutputHandler is a debug printing interface. Its implementations can be
// registered as potential output for the debug log.
//
//=============================================================================
#ifndef __AGS_CN_DEBUG__OUTPUTHANDLER_H
#define __AGS_CN_DEBUG__OUTPUTHANDLER_H

#include "debug/out.h"
#include "util/string.h"

namespace AGS
{
namespace Common
{

struct DebugMessage
{
    String       Text;
    uint32_t     GroupID;
    String       GroupName;
    MessageType  MT;

    DebugMessage() : GroupID(kDbgGroup_None), MT(kDbgMsg_None) {}
    DebugMessage(const String &text, uint32_t group_id, const String &group_name, MessageType mt)
        : Text(text)
        , GroupID(group_id)
        , GroupName(group_name)
        , MT(mt)
    {}
};

class IOutputHandler
{
public:
    virtual ~IOutputHandler() = default;
    
    // Print the given text sent from the debug group.
    // Implementations are free to decide which message components are to be printed, and how.
    virtual void PrintMessage(const DebugMessage &msg) = 0;
};

}   // namespace Common
}   // namespace AGS

#endif // __AGS_CN_DEBUG__OUTPUTHANDLER_H
