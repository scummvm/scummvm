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
// MessageBuffer, the IOutputHandler implementation that stores debug messages
// in a vector. Could be handy if you need to temporarily buffer debug log
// while specifying how to actually print it.
//
//=============================================================================
#ifndef __AGS_EE_DEBUG__MESSAGEBUFFER_H
#define __AGS_EE_DEBUG__MESSAGEBUFFER_H

#include <vector>
#include "debug/outputhandler.h"

namespace AGS
{
namespace Engine
{

using Common::String;
using Common::DebugMessage;

class MessageBuffer : public AGS::Common::IOutputHandler
{
public:
    MessageBuffer(size_t buffer_limit = 1024);

    void PrintMessage(const DebugMessage &msg) override;

    // Clears buffer
    void         Clear();
    // Sends buffered messages into given output target
    void         Send(const String &out_id);
    // Sends buffered messages into given output target and clears buffer
    void         Flush(const String &out_id);

private:
    const size_t    _bufferLimit;
    std::vector<DebugMessage> _buffer;
    size_t          _msgLost;
};

}   // namespace Engine
}   // namespace AGS

#endif // __AGS_EE_DEBUG__MESSAGEBUFFER_H
