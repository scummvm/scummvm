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
#include "debug/debugmanager.h"
#include "debug/messagebuffer.h"

namespace AGS
{
namespace Engine
{

using namespace Common;

MessageBuffer::MessageBuffer(size_t buffer_limit)
    : _bufferLimit(buffer_limit)
    , _msgLost(0)
{
}

void MessageBuffer::PrintMessage(const DebugMessage &msg)
{
    if (_buffer.size() < _bufferLimit)
        _buffer.push_back(msg);
    else
        _msgLost++;
}

void MessageBuffer::Clear()
{
    _buffer.clear();
    _msgLost = 0;
}

void MessageBuffer::Send(const String &out_id)
{
    if (_buffer.empty())
        return;
    if (_msgLost > 0)
    {
        DebugGroup gr = DbgMgr.GetGroup(kDbgGroup_Main);
        DbgMgr.SendMessage(out_id, DebugMessage(String::FromFormat("WARNING: output %s lost exceeding buffer: %u debug messages\n", out_id.GetCStr(), (unsigned)_msgLost),
            gr.UID.ID, gr.OutputName, kDbgMsg_All));
    }
    for (std::vector<DebugMessage>::const_iterator it = _buffer.begin(); it != _buffer.end(); ++it)
    {
        DbgMgr.SendMessage(out_id, *it);
    }
}

void MessageBuffer::Flush(const String &out_id)
{
    Send(out_id);
    Clear();
}

} // namespace Engine
} // namespace AGS
