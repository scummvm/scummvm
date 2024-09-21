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

//include <string.h>
#include "ags/shared/ac/common.h" // quit
#include "ags/shared/game/interactions.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/math.h"
#include "common/util.h"

namespace AGS3 {

using namespace AGS::Shared;

namespace AGS {
namespace Shared {

InteractionValue::InteractionValue() {
	Type = kInterValLiteralInt;
	Value = 0;
	Extra = 0;
}

void InteractionValue::clear() {
	Type = kInterValInvalid;
	Value = 0;
	Extra = 0;
}

void InteractionValue::Read(Stream *in) {
	Type = (InterValType)in->ReadInt8();
	in->Seek(3); // alignment padding to int32
	Value = in->ReadInt32();
	Extra = in->ReadInt32();
}

void InteractionValue::Write(Stream *out) const {
	out->WriteInt8(Type);
	out->WriteByteCount(0, 3); // alignment padding to int32
	out->WriteInt32(Value);
	out->WriteInt32(Extra);
}

//-----------------------------------------------------------------------------

InteractionCommand::InteractionCommand()
	: Type(0)
	, Parent(nullptr) {
}

InteractionCommand::InteractionCommand(const InteractionCommand &ic) {
	*this = ic;
}

void InteractionCommand::Assign(const InteractionCommand &ic, InteractionCommandList *parent) {
	Type = ic.Type;
	memcpy(Data, ic.Data, sizeof(Data));
	Children.reset(ic.Children.get() ? new InteractionCommandList(*ic.Children) : nullptr);
	Parent = parent;
}

void InteractionCommand::Reset() {
	Type = 0;
	for (uint8 i = 0; i < ARRAYSIZE(Data); i++) Data[i].clear();
	Children.reset();
	Parent = nullptr;
}

void InteractionCommand::ReadValues(Stream *in) {
	for (int i = 0; i < MAX_ACTION_ARGS; ++i) {
		Data[i].Read(in);
	}
}

void InteractionCommand::Read(Stream *in, bool &has_children) {
	in->ReadInt32(); // skip the 32-bit vtbl ptr (the old serialization peculiarity)
	Type = in->ReadInt32();
	ReadValues(in);
	has_children = in->ReadInt32() != 0;
	in->ReadInt32(); // skip 32-bit Parent pointer
}

void InteractionCommand::WriteValues(Stream *out) const {
	for (int i = 0; i < MAX_ACTION_ARGS; ++i) {
		Data[i].Write(out);
	}
}

void InteractionCommand::Write(Stream *out) const {
	out->WriteInt32(0); // write dummy 32-bit vtbl ptr
	out->WriteInt32(Type);
	WriteValues(out);
	out->WriteInt32(Children.get() ? 1 : 0);  // notify that has children
	out->WriteInt32(0);  // skip 32-bit Parent pointer
}

InteractionCommand &InteractionCommand::operator=(const InteractionCommand &ic) {
	if (this == &ic) {
		return *this; // prevent self-assignment
	}

	Type = ic.Type;
	memcpy(Data, ic.Data, sizeof(Data));
	Children.reset(ic.Children.get() ? new InteractionCommandList(*ic.Children) : nullptr);
	Parent = ic.Parent;
	return *this;
}

//-----------------------------------------------------------------------------

InteractionCommandList::InteractionCommandList()
	: TimesRun(0) {
}

InteractionCommandList::InteractionCommandList(const InteractionCommandList &icmd_list) {
	TimesRun = icmd_list.TimesRun;
	Cmds.resize(icmd_list.Cmds.size());
	for (size_t i = 0; i < icmd_list.Cmds.size(); ++i) {
		Cmds[i].Assign(icmd_list.Cmds[i], this);
	}
}

void InteractionCommandList::Reset() {
	Cmds.clear();
	TimesRun = 0;
}

void InteractionCommandList::ReadCommands(Stream *in, std::vector<bool> &cmd_children) {
	for (size_t i = 0; i < Cmds.size(); ++i) {
		bool has_children;
		Cmds[i].Read(in, has_children);
		cmd_children[i] = has_children;
	}
}

void InteractionCommandList::Read(Stream *in) {
	size_t cmd_count = in->ReadInt32();
	TimesRun = in->ReadInt32();

	std::vector<bool> cmd_children;
	Cmds.resize(cmd_count);
	cmd_children.resize(cmd_count);
	ReadCommands(in, cmd_children);

	for (size_t i = 0; i < cmd_count; ++i) {
		if (cmd_children[i]) {
			Cmds[i].Children.reset(new InteractionCommandList());
			Cmds[i].Children->Read(in);
		}
		Cmds[i].Parent = this;
	}
}

void InteractionCommandList::WriteCommands(Stream *out) const {
	for (InterCmdVector::const_iterator it = Cmds.begin(); it != Cmds.end(); ++it) {
		it->Write(out);
	}
}

void InteractionCommandList::Write(Stream *out) const {
	size_t cmd_count = Cmds.size();
	out->WriteInt32(cmd_count);
	out->WriteInt32(TimesRun);

	WriteCommands(out);

	for (size_t i = 0; i < cmd_count; ++i) {
		if (Cmds[i].Children.get() != nullptr)
			Cmds[i].Children->Write(out);
	}
}

//-----------------------------------------------------------------------------

InteractionEvent::InteractionEvent()
	: Type(0)
	, TimesRun(0) {
}

InteractionEvent::InteractionEvent(const InteractionEvent &ie) {
	*this = ie;
}

InteractionEvent &InteractionEvent::operator = (const InteractionEvent &ie) {
	Type = ie.Type;
	TimesRun = ie.TimesRun;
	Response.reset(ie.Response.get() ? new InteractionCommandList(*ie.Response) : nullptr);
	return *this;
}

//-----------------------------------------------------------------------------

Interaction::Interaction() {
}

Interaction::Interaction(const Interaction &ni) {
	*this = ni;
}

Interaction &Interaction::operator =(const Interaction &ni) {
	Events.resize(ni.Events.size());
	for (size_t i = 0; i < ni.Events.size(); ++i) {
		Events[i] = InteractionEvent(ni.Events[i]);
	}
	return *this;
}

void Interaction::CopyTimesRun(const Interaction &inter) {
	assert(Events.size() == inter.Events.size());
	size_t count = MIN(Events.size(), inter.Events.size());
	for (size_t i = 0; i < count; ++i) {
		Events[i].TimesRun = inter.Events[i].TimesRun;
	}
}

void Interaction::Reset() {
	Events.clear();
}

Interaction *Interaction::CreateFromStream(Stream *in) {
	if (in->ReadInt32() != kInteractionVersion_Initial)
		return nullptr; // unsupported format

	const size_t evt_count = in->ReadInt32();
	if (evt_count > MAX_NEWINTERACTION_EVENTS)
		quit("Can't deserialize interaction: too many events");

	int32_t types[MAX_NEWINTERACTION_EVENTS];
	int32_t load_response[MAX_NEWINTERACTION_EVENTS];
	in->ReadArrayOfInt32(types, evt_count);
	in->ReadArrayOfInt32(load_response, evt_count);

	Interaction *inter = new Interaction();
	inter->Events.resize(evt_count);
	for (size_t i = 0; i < evt_count; ++i) {
		InteractionEvent &evt = inter->Events[i];
		evt.Type = types[i];
		if (load_response[i] != 0) {
			evt.Response.reset(new InteractionCommandList());
			evt.Response->Read(in);
		}
	}
	return inter;
}

void Interaction::Write(Stream *out) const {
	out->WriteInt32(kInteractionVersion_Initial);  // Version
	const size_t evt_count = Events.size();
	out->WriteInt32(evt_count);
	for (size_t i = 0; i < evt_count; ++i) {
		out->WriteInt32(Events[i].Type);
	}

	// The pointer is only checked against NULL to determine whether the event exists
	for (size_t i = 0; i < evt_count; ++i) {
		out->WriteInt32(Events[i].Response.get() ? 1 : 0);
	}

	for (size_t i = 0; i < evt_count; ++i) {
		if (Events[i].Response.get())
			Events[i].Response->Write(out);
	}
}

void Interaction::ReadFromSavedgame_v321(Stream *in) {
	const size_t evt_count = in->ReadInt32();
	if (evt_count > MAX_NEWINTERACTION_EVENTS)
		quit("Can't deserialize interaction: too many events");

	Events.resize(evt_count);
	// Read required amount and skip the remaining placeholders
	for (size_t i = 0; i < evt_count; ++i) {
		Events[i].Type = in->ReadInt32();
	}
	in->Seek((MAX_NEWINTERACTION_EVENTS - evt_count) * sizeof(int32_t));
	ReadTimesRunFromSave_v321(in);
	// Skip an array of dummy 32-bit pointers (nasty legacy format)
	in->Seek(MAX_NEWINTERACTION_EVENTS * sizeof(int32_t));
}

void Interaction::WriteToSavedgame_v321(Stream *out) const {
	const size_t evt_count = Events.size();
	out->WriteInt32(evt_count);

	for (size_t i = 0; i < evt_count; ++i) {
		out->WriteInt32(Events[i].Type);
	}
	out->WriteByteCount(0, (MAX_NEWINTERACTION_EVENTS - evt_count) * sizeof(int32_t));
	WriteTimesRunToSave_v321(out);
	// Array of dummy 32-bit pointers (nasty legacy format)
	out->WriteByteCount(0, MAX_NEWINTERACTION_EVENTS * sizeof(int32_t));
}

void Interaction::ReadTimesRunFromSave_v321(Stream *in) {
	const size_t evt_count = Events.size();
	// Read required amount and skip the remaining placeholders
	for (size_t i = 0; i < evt_count; ++i) {
		Events[i].TimesRun = in->ReadInt32();
	}
	in->Seek((MAX_NEWINTERACTION_EVENTS - evt_count) * sizeof(int32_t));
}

void Interaction::WriteTimesRunToSave_v321(Stream *out) const {
	const size_t evt_count = Events.size();
	for (size_t i = 0; i < Events.size(); ++i) {
		out->WriteInt32(Events[i].TimesRun);
	}
	out->WriteByteCount(0, (MAX_NEWINTERACTION_EVENTS - evt_count) * sizeof(int32_t));
}

//-----------------------------------------------------------------------------

#define INTER_VAR_NAME_LENGTH 23

InteractionVariable::InteractionVariable()
	: Type(0)
	, Value(0) {
}

InteractionVariable::InteractionVariable(const String &name, char type, int val)
	: Name(name)
	, Type(type)
	, Value(val) {
}

void InteractionVariable::Read(Stream *in) {
	Name.ReadCount(in, INTER_VAR_NAME_LENGTH);
	Type = in->ReadInt8();
	Value = in->ReadInt32();
}

void InteractionVariable::Write(Shared::Stream *out) const {
	out->Write(Name.GetCStr(), INTER_VAR_NAME_LENGTH);
	out->WriteInt8(Type);
	out->WriteInt32(Value);
}

//-----------------------------------------------------------------------------

InteractionScripts *InteractionScripts::CreateFromStream(Stream *in) {
	const size_t evt_count = in->ReadInt32();
	if (evt_count > MAX_NEWINTERACTION_EVENTS) {
		quit("Can't deserialize interaction scripts: too many events");
		return nullptr;
	}

	InteractionScripts *scripts = new InteractionScripts();
	for (size_t i = 0; i < evt_count; ++i) {
		String name = String::FromStream(in);
		scripts->ScriptFuncNames.push_back(name);
	}
	return scripts;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
