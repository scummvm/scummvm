/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

//=============================================================================
//
// MessageBuffer, the IOutputHandler implementation that stores debug messages
// in a vector. Could be handy if you need to temporarily buffer debug log
// while specifying how to actually print it.
//
//=============================================================================

#ifndef AGS_ENGINE_DEBUGGING_MESSAGEBUFFER_H
#define AGS_ENGINE_DEBUGGING_MESSAGEBUFFER_H

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

#endif
