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
// IOutputHandler is a debug printing interface. Its implementations can be
// registered as potential output for the debug log.
//
//=============================================================================

#ifndef AGS_SHARED_DEBUGGING_OUTPUTHANDLER_H
#define AGS_SHARED_DEBUGGING_OUTPUTHANDLER_H

#include "debug/out.h"
#include "util/string.h"

namespace AGS3 {
namespace AGS {
namespace Common {

struct DebugMessage {
	String       Text;
	uint32_t     GroupID;
	String       GroupName;
	MessageType  MT;

	DebugMessage() : GroupID(kDbgGroup_None), MT(kDbgMsg_None) {
	}
	DebugMessage(const String &text, uint32_t group_id, const String &group_name, MessageType mt)
		: Text(text)
		, GroupID(group_id)
		, GroupName(group_name)
		, MT(mt) {
	}
};

class IOutputHandler {
public:
	virtual ~IOutputHandler() = default;

	// Print the given text sent from the debug group.
	// Implementations are free to decide which message components are to be printed, and how.
	virtual void PrintMessage(const DebugMessage &msg) = 0;
};

} // namespace Common
} // namespace AGS
} // namespace AGS3

#endif
