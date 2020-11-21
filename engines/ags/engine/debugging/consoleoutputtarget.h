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
// ConsoleOutputTarget prints messages onto in-game console GUI (available
// only if the game was compiled in debug mode).
//
//=============================================================================
#ifndef __AGS_EE_DEBUG__CONSOLEOUTPUTTARGET_H
#define __AGS_EE_DEBUG__CONSOLEOUTPUTTARGET_H

#include "debug/outputhandler.h"

namespace AGS
{
namespace Engine
{

using Common::String;
using Common::DebugMessage;

class ConsoleOutputTarget : public AGS::Common::IOutputHandler
{
public:
    ConsoleOutputTarget();
    virtual ~ConsoleOutputTarget();

    void PrintMessage(const DebugMessage &msg) override;
};

}   // namespace Engine
}   // namespace AGS

#endif // __AGS_EE_DEBUG__CONSOLEOUTPUTTARGET_H
