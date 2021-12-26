/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//=============================================================================
//
// AGS logging system is built with idea that the engine components should not
// be bothered with specifying particular output method. Instead they use
// generic logging interface, and the actual message printing is done by one
// or more registered handlers.
// Firstly this makes logging functions independent of running platform or
// back-end, secondly it grants end-users ability to configure output according
// to their preference.
//
// To make the logging work we need to register two sets of "entities":
// debug groups and output targets.
// Debug group is an arbitrary object with a name that describes message
// sender.
// Output target defines printing handler and a set of verbosity rules
// one per each known group.
//
// When the message is sent, it is tagged with one of the existing group IDs
// and a message type (debug info, warning, error). This message is sent onto
// each of the registered output targets, which do checks to find out whether
// the message is permitted to be sent further to the printing handler, or not.
//
//=============================================================================

#ifndef AGS_SHARED_DEBUGGING_DEBUG_MANAGER_H
#define AGS_SHARED_DEBUGGING_DEBUG_MANAGER_H

#include "ags/lib/std/memory.h"
#include "ags/lib/std/map.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/debugging/output_handler.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_types.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

// DebugGroup is a message sender definition, identified by DebugGroupID
// and providing OutputName that could be used when printing its messages.
// OutputName may or may not be same as DebugGroupID.SID.
struct DebugGroup {
	DebugGroupID    UID;
	String          OutputName;

	DebugGroup() {
	}
	DebugGroup(DebugGroupID id, String out_name) : UID(id), OutputName(out_name) {
	}
};

// DebugOutput is a slot for IOutputHandler with its own group filter
class DebugOutput {
public:
	DebugOutput(const String &id, IOutputHandler *handler, MessageType def_verbosity = kDbgMsg_All, bool enabled = true);

	String          GetID() const;
	IOutputHandler *GetHandler() const;

	bool            IsEnabled() const;
	void            SetEnabled(bool enable);
	// Setup group filter: either allow or disallow a group with the given ID
	void            SetGroupFilter(DebugGroupID id, MessageType verbosity);
	// Assign same verbosity level to all known groups
	void            SetAllGroupFilters(MessageType verbosity);
	// Clear all group filters; this efficiently disables everything
	void            ClearGroupFilters();
	// Try to resolve group filter unknown IDs
	void            ResolveGroupID(DebugGroupID id);
	// Test if given group id is permitted
	bool            TestGroup(DebugGroupID id, MessageType mt) const;

private:
	String          _id;
	IOutputHandler *_handler;
	bool            _enabled;
	MessageType     _defaultVerbosity;
	// Set of permitted groups' numeric IDs
	std::vector<MessageType> _groupFilter;
	// Set of unresolved groups, which numeric IDs are not yet known
	typedef std::unordered_map<String, MessageType, IgnoreCase_Hash, IgnoreCase_EqualTo> GroupNameToMTMap;
	GroupNameToMTMap _unresolvedGroups;
};

typedef std::shared_ptr<DebugOutput> PDebugOutput;


class DebugManager {
	friend class DebugOutput;

public:
	DebugManager();

	// Gets full group ID for any partial one; if the group is not registered returns unset ID
	DebugGroup   GetGroup(DebugGroupID id);
	// Gets output control interface for the given ID
	PDebugOutput GetOutput(const String &id);
	// Registers debugging group with the given string ID; numeric ID
	// will be assigned internally. Returns full ID pair.
	// If the group with such string id already exists, returns existing ID.
	DebugGroup RegisterGroup(const String &id, const String &out_name);
	// Registers output delegate for passing debug messages to;
	// if the output with such id already exists, replaces the old one
	PDebugOutput RegisterOutput(const String &id, IOutputHandler *handler, MessageType def_verbosity = kDbgMsg_All, bool enabled = true);
	// Unregisters all groups and all targets
	void UnregisterAll();
	// Unregisters debugging group with the given ID
	void UnregisterGroup(DebugGroupID id);
	// Unregisters output delegate with the given ID
	void UnregisterOutput(const String &id);

	// Output message of given group and message type
	void Print(DebugGroupID group_id, MessageType mt, const String &text);
	// Send message directly to the output with given id; the message
	// must pass the output's message filter though
	void SendMessage(const String &out_id, const DebugMessage &msg);

private:
	// OutputSlot struct wraps over output target and adds a flag which indicates
	// that this target is temporarily disabled (for internal use only)
	struct OutputSlot {
		PDebugOutput Target;
		bool          Suppressed;

		OutputSlot() : Suppressed(false) {
		}
	};

	typedef std::vector<DebugGroup> GroupVector;
	typedef std::unordered_map<String, DebugGroupID, IgnoreCase_Hash, IgnoreCase_EqualTo> GroupByStringMap;
	typedef std::unordered_map<String, OutputSlot, IgnoreCase_Hash, IgnoreCase_EqualTo> OutMap;

	void RegisterGroup(const DebugGroup &id);
	void SendMessage(OutputSlot &out, const DebugMessage &msg);

	uint32_t            _firstFreeGroupID;
	uint32_t            _lastGroupID;
	GroupVector         _groups;
	GroupByStringMap    _groupByStrLookup;
	OutMap              _outputs;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
