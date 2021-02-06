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
// Debug output interface provides functions which send a formatted message
// tagged with group ID and message type to the registered output handlers.
//
// Depending on configuration this message may be printed by any of those
// handlers, or none of them. The calling unit should not worry about where the
// message goes.
//
//-----------------------------------------------------------------------------
//
// On using message types.
//
// Please keep in mind, that there are different levels of errors. AGS logging
// system allows to classify debug message by two parameters: debug group and
// message type. Sometimes message type alone is not enough. Debug groups can
// also be used to distinct messages that has less (or higher) importance.
//
// For example, there are engine errors and user (game-dev) mistakes. Script
// commands that cannot be executed under given circumstances are user
// mistakes, and usually are not as severe as internal engine errors. This is
// why it is advised to use a separate debug group for mistakes like that to
// distinct them from reports on the internal engine's problems and make
// verbosity configuration flexible.
//
// kDbgMsg_Debug - is the most mundane type of message (if the printing function
// argument list does not specify message type, it is probably kDbgMsg_Debug).
// You can use it for almost anything, from noting some process steps to
// displaying current object state. If certain messages are meant to be printed
// very often, consider using another distinct debug group so that it may be
// disabled to reduce log verbosity.
//
// kDbgMsg_Info - is a type for important notifications, such as initialization
// and stopping of engine components.
//
// kDbgMsg_Warn - this is suggested for more significant cases, when you find
// out that something is not right, but is not immediately affecting engine
// processing. For example: certain object was constructed in a way that
// would make them behave unexpectedly, or certain common files are missing but
// it is not clear yet whether the game will be accessing them.
// In other words: use kDbgMsg_Warn when there is no problem right away, but
// you are *anticipating* that one may happen under certain circumstances.
//
// kDbgMsg_Error - use this kind of message is for actual serious problems.
// If certain operation assumes both positive and negative results are
// acceptable, it is preferred to report such negative result with simple
// kDbgMsg_Debug message. kDbgMsg_Error is for negative results that are not
// considered acceptable for normal run.
//
// kDbgMsg_Fatal - is the message type to be reported when the program or
// component abortion is imminent.
//
//=============================================================================
#ifndef __AGS_CN_DEBUG__OUT_H
#define __AGS_CN_DEBUG__OUT_H

#include "util/string.h"

namespace AGS
{
namespace Common
{

// Message types provide distinction for debug messages by their intent.
enum MessageType
{
    kDbgMsg_None                = 0,
    // Alerts may be informative messages with topmost level of importance,
    // such as reporting engine startup and shutdown.
    kDbgMsg_Alert               ,
    // Fatal errors are ones that make program abort immediately.
    kDbgMsg_Fatal               ,
    // Error messages are about engine not being able to perform requested
    // operation in a situation when that will affect game playability and
    // further execution.
    kDbgMsg_Error               ,
    // Warnings are made when unexpected or non-standart behavior
    // is detected in program, which is not immediately critical,
    // but may be a symptom of a bigger problem.
    kDbgMsg_Warn                ,
    // General information messages.
    kDbgMsg_Info                ,
    // Debug reason is for arbitrary information about events and current
    // game state.
    kDbgMsg_Debug               ,


    // Convenient aliases
    kDbgMsg_Default             = kDbgMsg_Debug,
    kDbgMsg_All                 = kDbgMsg_Debug
};

// This enumeration is a list of common hard-coded groups, but more could
// be added via debugging configuration interface (see 'debug/debug.h').
enum CommonDebugGroup
{
    kDbgGroup_None = -1,
    // Main debug group is for reporting general engine status and issues
    kDbgGroup_Main = 0,
    // Game group is for logging game logic state and issues
    kDbgGroup_Game,
    // Log from the game script
    kDbgGroup_Script,
    // Sprite cache logging
    kDbgGroup_SprCache,
    // Group for debugging managed object state (can slow engine down!)
    kDbgGroup_ManObj
};

// Debug group identifier defining either numeric or string id, or both
struct DebugGroupID
{
    uint32_t    ID;
    String      SID;

    DebugGroupID() : ID(kDbgGroup_None) {}
    DebugGroupID(uint32_t id, const String &sid = "") : ID(id), SID(sid) {}
    DebugGroupID(const String &sid) : ID(kDbgGroup_None), SID(sid) {}
    // Tells if any of the id components is valid
    bool IsValid() const { return ID != kDbgGroup_None || !SID.IsEmpty(); }
    // Tells if both id components are properly set
    bool IsComplete() const { return ID != kDbgGroup_None && !SID.IsEmpty(); }
};

namespace Debug
{
    //
    // Debug output
    //
    // Output formatted message of default group and default type
    void Printf(const char *fmt, ...);
    // Output formatted message of default group and given type
    void Printf(MessageType mt, const char *fmt, ...);
    // Output formatted message of given group and type
    void Printf(DebugGroupID group_id, MessageType mt, const char *fmt, ...);

}   // namespace Debug

}   // namespace Common
}   // namespace AGS

#endif // __AGS_CN_DEBUG__OUT_H
