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

#include "debug/debugmanager.h"
#include "debug/messagebuffer.h"

namespace AGS {
namespace Engine {

using namespace Common;

MessageBuffer::MessageBuffer(size_t buffer_limit)
	: _bufferLimit(buffer_limit)
	, _msgLost(0) {
}

void MessageBuffer::PrintMessage(const DebugMessage &msg) {
	if (_buffer.size() < _bufferLimit)
		_buffer.push_back(msg);
	else
		_msgLost++;
}

void MessageBuffer::Clear() {
	_buffer.clear();
	_msgLost = 0;
}

void MessageBuffer::Send(const String &out_id) {
	if (_buffer.empty())
		return;
	if (_msgLost > 0) {
		DebugGroup gr = DbgMgr.GetGroup(kDbgGroup_Main);
		DbgMgr.SendMessage(out_id, DebugMessage(String::FromFormat("WARNING: output %s lost exceeding buffer: %u debug messages\n", out_id.GetCStr(), (unsigned)_msgLost),
		                                        gr.UID.ID, gr.OutputName, kDbgMsg_All));
	}
	for (std::vector<DebugMessage>::const_iterator it = _buffer.begin(); it != _buffer.end(); ++it) {
		DbgMgr.SendMessage(out_id, *it);
	}
}

void MessageBuffer::Flush(const String &out_id) {
	Send(out_id);
	Clear();
}

} // namespace Engine
} // namespace AGS
