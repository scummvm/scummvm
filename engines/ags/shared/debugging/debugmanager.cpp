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

#include <stdarg.h>
#include "debug/debugmanager.h"
#include "util/string_types.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

DebugOutput::DebugOutput(const String &id, IOutputHandler *handler, MessageType def_verbosity, bool enabled)
	: _id(id)
	, _handler(handler)
	, _enabled(enabled)
	, _defaultVerbosity(def_verbosity) {
	_groupFilter.resize(DbgMgr._lastGroupID + 1, _defaultVerbosity);
}

String DebugOutput::GetID() const {
	return _id;
}

IOutputHandler *DebugOutput::GetHandler() const {
	return _handler;
}

bool DebugOutput::IsEnabled() const {
	return _enabled;
}

void DebugOutput::SetEnabled(bool enable) {
	_enabled = enable;
}

void DebugOutput::SetGroupFilter(DebugGroupID id, MessageType verbosity) {
	uint32_t key = DbgMgr.GetGroup(id).UID.ID;
	if (key != kDbgGroup_None)
		_groupFilter[key] = verbosity;
	else
		_unresolvedGroups.insert(std::make_pair(id.SID, verbosity));
}

void DebugOutput::SetAllGroupFilters(MessageType verbosity) {
	for (auto &group : _groupFilter)
		group = verbosity;
	for (auto &group : _unresolvedGroups)
		group.second = verbosity;
}

void DebugOutput::ClearGroupFilters() {
	for (auto &gf : _groupFilter)
		gf = kDbgMsg_None;
	_unresolvedGroups.clear();
}

void DebugOutput::ResolveGroupID(DebugGroupID id) {
	if (!id.IsValid())
		return;

	DebugGroupID real_id = DbgMgr.GetGroup(id).UID;
	if (real_id.IsValid()) {
		if (_groupFilter.size() <= id.ID)
			_groupFilter.resize(id.ID + 1, _defaultVerbosity);
		GroupNameToMTMap::const_iterator it = _unresolvedGroups.find(real_id.SID);
		if (it != _unresolvedGroups.end()) {
			_groupFilter[real_id.ID] = it->second;
			_unresolvedGroups.erase(it);
		}
	}
}

bool DebugOutput::TestGroup(DebugGroupID id, MessageType mt) const {
	DebugGroupID real_id = DbgMgr.GetGroup(id).UID;
	if (real_id.ID == kDbgGroup_None || real_id.ID >= _groupFilter.size())
		return false;
	return (_groupFilter[real_id.ID] >= mt) != 0;
}

DebugManager::DebugManager() {
	// Add hardcoded groups
	RegisterGroup(DebugGroup(DebugGroupID(kDbgGroup_Main, "main"), ""));
	RegisterGroup(DebugGroup(DebugGroupID(kDbgGroup_Game, "game"), "Game"));
	RegisterGroup(DebugGroup(DebugGroupID(kDbgGroup_Script, "script"), "Script"));
	RegisterGroup(DebugGroup(DebugGroupID(kDbgGroup_SprCache, "sprcache"), "Sprite cache"));
	RegisterGroup(DebugGroup(DebugGroupID(kDbgGroup_ManObj, "manobj"), "Managed obj"));
	_firstFreeGroupID = _groups.size();
	_lastGroupID = _firstFreeGroupID;
}

DebugGroup DebugManager::GetGroup(DebugGroupID id) {
	if (id.ID != kDbgGroup_None) {
		return id.ID < _groups.size() ? _groups[id.ID] : DebugGroup();
	} else if (!id.SID.IsEmpty()) {
		GroupByStringMap::const_iterator it = _groupByStrLookup.find(id.SID);
		return it != _groupByStrLookup.end() ? _groups[it->second.ID] : DebugGroup();
	}
	return DebugGroup();
}

PDebugOutput DebugManager::GetOutput(const String &id) {
	OutMap::const_iterator it = _outputs.find(id);
	return it != _outputs.end() ? it->second.Target : PDebugOutput();
}

DebugGroup DebugManager::RegisterGroup(const String &id, const String &out_name) {
	DebugGroup group = GetGroup(id);
	if (group.UID.IsValid())
		return group;
	group = DebugGroup(DebugGroupID(++DbgMgr._lastGroupID, id), out_name);
	_groups.push_back(group);
	_groupByStrLookup[group.UID.SID] = group.UID;

	// Resolve group reference on every output target
	for (OutMap::const_iterator it = _outputs.begin(); it != _outputs.end(); ++it) {
		it->second.Target->ResolveGroupID(group.UID);
	}
	return group;
}

void DebugManager::RegisterGroup(const DebugGroup &group) {
	_groups.push_back(group);
	_groupByStrLookup[group.UID.SID] = group.UID;
}

PDebugOutput DebugManager::RegisterOutput(const String &id, IOutputHandler *handler, MessageType def_verbosity, bool enabled) {
	_outputs[id].Target = PDebugOutput(new DebugOutput(id, handler, def_verbosity, enabled));
	_outputs[id].Suppressed = false;
	return _outputs[id].Target;
}

void DebugManager::UnregisterAll() {
	_lastGroupID = _firstFreeGroupID;
	_groups.clear();
	_groupByStrLookup.clear();
	_outputs.clear();
}

void DebugManager::UnregisterGroup(DebugGroupID id) {
	DebugGroup group = GetGroup(id);
	if (!group.UID.IsValid())
		return;
	_groups[group.UID.ID] = DebugGroup();
	_groupByStrLookup.erase(group.UID.SID);
}

void DebugManager::UnregisterOutput(const String &id) {
	_outputs.erase(id);
}

void DebugManager::Print(DebugGroupID group_id, MessageType mt, const String &text) {
	const DebugGroup &group = GetGroup(group_id);
	DebugMessage msg(text, group.UID.ID, group.OutputName, mt);

	for (OutMap::iterator it = _outputs.begin(); it != _outputs.end(); ++it) {
		SendMessage(it->second, msg);
	}
}

void DebugManager::SendMessage(const String &out_id, const DebugMessage &msg) {
	OutMap::iterator it = _outputs.find(out_id);
	if (it != _outputs.end())
		SendMessage(it->second, msg);
}

void DebugManager::SendMessage(OutputSlot &out, const DebugMessage &msg) {
	IOutputHandler *handler = out.Target->GetHandler();
	if (!handler || !out.Target->IsEnabled() || out.Suppressed)
		return;
	if (!out.Target->TestGroup(msg.GroupID, msg.MT))
		return;
	// We suppress current target before the call so that if it makes
	// a call to output system itself, message would not print to the
	// same target
	out.Suppressed = true;
	handler->PrintMessage(msg);
	out.Suppressed = false;
}

// TODO: move this to the dynamically allocated engine object whenever it is implemented
DebugManager DbgMgr;


namespace Debug {

void Printf(const char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	DbgMgr.Print(kDbgGroup_Main, kDbgMsg_Default, String::FromFormatV(fmt, argptr));
	va_end(argptr);
}

void Printf(MessageType mt, const char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	DbgMgr.Print(kDbgGroup_Main, mt, String::FromFormatV(fmt, argptr));
	va_end(argptr);
}

void Printf(DebugGroupID group, MessageType mt, const char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	DbgMgr.Print(group, mt, String::FromFormatV(fmt, argptr));
	va_end(argptr);
}

} // namespace Debug

} // namespace Shared
} // namespace AGS
} // namespace AGS3
