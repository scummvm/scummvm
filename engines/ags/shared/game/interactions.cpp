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

#include <string.h>
#include "ac/common.h" // quit
#include "game/interactions.h"
#include "util/alignedstream.h"
#include "util/math.h"

using namespace AGS::Common;

InteractionVariable globalvars[MAX_GLOBAL_VARIABLES] = {InteractionVariable("Global 1", 0, 0)};
int numGlobalVars = 1;

namespace AGS
{
namespace Common
{

InteractionValue::InteractionValue()
{
    Type = kInterValLiteralInt;
    Value = 0;
    Extra = 0;
}

void InteractionValue::Read(Stream *in)
{
    Type  = (InterValType)in->ReadInt8();
    Value = in->ReadInt32();
    Extra = in->ReadInt32();
}

void InteractionValue::Write(Stream *out) const
{
    out->WriteInt8(Type);
    out->WriteInt32(Value);
    out->WriteInt32(Extra);
}

//-----------------------------------------------------------------------------

InteractionCommand::InteractionCommand()
    : Type(0)
    , Parent(nullptr)
{
}

InteractionCommand::InteractionCommand(const InteractionCommand &ic)
{
    *this = ic;
}

void InteractionCommand::Assign(const InteractionCommand &ic, InteractionCommandList *parent)
{
    Type = ic.Type;
    memcpy(Data, ic.Data, sizeof(Data));
    Children.reset(ic.Children.get() ? new InteractionCommandList(*ic.Children) : nullptr);
    Parent = parent;
}

void InteractionCommand::Reset()
{
    Type = 0;
    memset(Data, 0, sizeof(Data));
    Children.reset();
    Parent = nullptr;
}

void InteractionCommand::ReadValues_Aligned(Stream *in)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    for (int i = 0; i < MAX_ACTION_ARGS; ++i)
    {
        Data[i].Read(&align_s);
        align_s.Reset();
    }
}

void InteractionCommand::Read_v321(Stream *in, bool &has_children)
{
    in->ReadInt32(); // skip the 32-bit vtbl ptr (the old serialization peculiarity)
    Type = in->ReadInt32();
    ReadValues_Aligned(in);
    has_children = in->ReadInt32() != 0;
    in->ReadInt32(); // skip 32-bit Parent pointer
}

void InteractionCommand::WriteValues_Aligned(Stream *out) const
{
    AlignedStream align_s(out, Common::kAligned_Write);
    for (int i = 0; i < MAX_ACTION_ARGS; ++i)
    {
        Data[i].Write(&align_s);
        align_s.Reset();
    }
}

void InteractionCommand::Write_v321(Stream *out) const
{
    out->WriteInt32(0); // write dummy 32-bit vtbl ptr
    out->WriteInt32(Type);
    WriteValues_Aligned(out);
    out->WriteInt32(Children.get() ? 1 : 0);
    out->WriteInt32(Parent ? 1 : 0);
}

InteractionCommand &InteractionCommand::operator = (const InteractionCommand &ic)
{
    Type = ic.Type;
    memcpy(Data, ic.Data, sizeof(Data));
    Children.reset(ic.Children.get() ? new InteractionCommandList(*ic.Children) : nullptr);
    Parent = ic.Parent;
    return *this;
}

//-----------------------------------------------------------------------------

InteractionCommandList::InteractionCommandList()
    : TimesRun(0)
{
}

InteractionCommandList::InteractionCommandList(const InteractionCommandList &icmd_list)
{
    TimesRun = icmd_list.TimesRun;
    Cmds.resize(icmd_list.Cmds.size());
    for (size_t i = 0; i < icmd_list.Cmds.size(); ++i)
    {
        Cmds[i].Assign(icmd_list.Cmds[i], this);
    }
}

void InteractionCommandList::Reset()
{
    Cmds.clear();
    TimesRun = 0;
}

void InteractionCommandList::Read_Aligned(Stream *in, std::vector<bool> &cmd_children)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    for (size_t i = 0; i < Cmds.size(); ++i)
    {
        bool has_children;
        Cmds[i].Read_v321(&align_s, has_children);
        cmd_children[i] = has_children;
        align_s.Reset();
    }
}

void InteractionCommandList::Read_v321(Stream *in)
{
    size_t cmd_count = in->ReadInt32();
    TimesRun = in->ReadInt32();

    std::vector<bool> cmd_children;
    Cmds.resize(cmd_count);
    cmd_children.resize(cmd_count);
    Read_Aligned(in, cmd_children);

    for (size_t i = 0; i < cmd_count; ++i)
    {
        if (cmd_children[i])
        {
            Cmds[i].Children.reset(new InteractionCommandList());
            Cmds[i].Children->Read_v321(in);
        }
        Cmds[i].Parent = this;
    }
}

void InteractionCommandList::Write_Aligned(Stream *out) const
{
    AlignedStream align_s(out, Common::kAligned_Write);
    for (InterCmdVector::const_iterator it = Cmds.begin(); it != Cmds.end(); ++it)
    {
        it->Write_v321(&align_s);
        align_s.Reset();
    }
}

void InteractionCommandList::Write_v321(Stream *out) const
{
    size_t cmd_count = Cmds.size();
    out->WriteInt32(cmd_count);
    out->WriteInt32(TimesRun);

    Write_Aligned(out);

    for (size_t i = 0; i < cmd_count; ++i)
    {
        if (Cmds[i].Children.get() != nullptr)
            Cmds[i].Children->Write_v321(out);
    }
}

//-----------------------------------------------------------------------------

InteractionEvent::InteractionEvent()
    : Type(0)
    , TimesRun(0)
{
}

InteractionEvent::InteractionEvent(const InteractionEvent &ie)
{
    *this = ie;
}

InteractionEvent &InteractionEvent::operator = (const InteractionEvent &ie)
{
    Type = ie.Type;
    TimesRun = ie.TimesRun;
    Response.reset(ie.Response.get() ? new InteractionCommandList(*ie.Response) : nullptr);
    return *this;
}

//-----------------------------------------------------------------------------

Interaction::Interaction()
{
}

Interaction::Interaction(const Interaction &ni)
{
    *this = ni;
}

Interaction &Interaction::operator =(const Interaction &ni)
{
    Events.resize(ni.Events.size());
    for (size_t i = 0; i < ni.Events.size(); ++i)
    {
        Events[i] = InteractionEvent(ni.Events[i]);
    }
    return *this;
}

void Interaction::CopyTimesRun(const Interaction &inter)
{
    assert(Events.size() == inter.Events.size());
    size_t count = Math::Min(Events.size(), inter.Events.size());
    for (size_t i = 0; i < count; ++i)
    {
        Events[i].TimesRun = inter.Events[i].TimesRun;
    }
}

void Interaction::Reset()
{
    Events.clear();
}

Interaction *Interaction::CreateFromStream(Stream *in)
{
    if (in->ReadInt32() != kInteractionVersion_Initial)
        return nullptr; // unsupported format

    const size_t evt_count = in->ReadInt32();
    if (evt_count > MAX_NEWINTERACTION_EVENTS)
        quit("Can't deserialize interaction: too many events");

    int types[MAX_NEWINTERACTION_EVENTS];
    int load_response[MAX_NEWINTERACTION_EVENTS];
    in->ReadArrayOfInt32(types, evt_count);
    in->ReadArrayOfInt32(load_response, evt_count);

    Interaction *inter = new Interaction();
    inter->Events.resize(evt_count);
    for (size_t i = 0; i < evt_count; ++i)
    {
        InteractionEvent &evt = inter->Events[i];
        evt.Type = types[i];
        if (load_response[i] != 0)
        {
            evt.Response.reset(new InteractionCommandList());
            evt.Response->Read_v321(in);
        }
    }
    return inter;
}

void Interaction::Write(Stream *out) const
{
    out->WriteInt32(kInteractionVersion_Initial);  // Version
    const size_t evt_count = Events.size();
    out->WriteInt32(evt_count);
    for (size_t i = 0; i < evt_count; ++i)
    {
        out->WriteInt32(Events[i].Type);
    }

    // The pointer is only checked against NULL to determine whether the event exists
    for (size_t i = 0; i < evt_count; ++i)
    {
        out->WriteInt32 (Events[i].Response.get() ? 1 : 0);
    }

    for (size_t i = 0; i < evt_count; ++i)
    {
        if (Events[i].Response.get())
            Events[i].Response->Write_v321(out);
    }
}

void Interaction::ReadFromSavedgame_v321(Stream *in)
{
    const size_t evt_count = in->ReadInt32();
    if (evt_count > MAX_NEWINTERACTION_EVENTS)
        quit("Can't deserialize interaction: too many events");

    Events.resize(evt_count);
    for (size_t i = 0; i < evt_count; ++i)
    {
        Events[i].Type = in->ReadInt32();
    }
    const size_t padding = (MAX_NEWINTERACTION_EVENTS - evt_count);
    for (size_t i = 0; i < padding; ++i)
        in->ReadInt32(); // cannot skip when reading aligned structs
    ReadTimesRunFromSave_v321(in);

    // Skip an array of dummy 32-bit pointers
    for (size_t i = 0; i < MAX_NEWINTERACTION_EVENTS; ++i)
        in->ReadInt32();
}

void Interaction::WriteToSavedgame_v321(Stream *out) const
{
    const size_t evt_count = Events.size();
    out->WriteInt32(evt_count);

    for (size_t i = 0; i < evt_count; ++i)
    {
        out->WriteInt32(Events[i].Type);
    }
    out->WriteByteCount(0, (MAX_NEWINTERACTION_EVENTS - evt_count) * sizeof(int32_t));
    WriteTimesRunToSave_v321(out);

    // Array of dummy 32-bit pointers
    out->WriteByteCount(0, MAX_NEWINTERACTION_EVENTS * sizeof(int32_t));
}

void Interaction::ReadTimesRunFromSave_v321(Stream *in)
{
    const size_t evt_count = Events.size();
    for (size_t i = 0; i < evt_count; ++i)
    {
        Events[i].TimesRun = in->ReadInt32();
    }
    const size_t padding = (MAX_NEWINTERACTION_EVENTS - evt_count);
    for (size_t i = 0; i < padding; ++i)
        in->ReadInt32(); // cannot skip when reading aligned structs
}

void Interaction::WriteTimesRunToSave_v321(Stream *out) const
{
    const size_t evt_count = Events.size();
    for (size_t i = 0; i < Events.size(); ++i)
    {
        out->WriteInt32(Events[i].TimesRun);
    }
    out->WriteByteCount(0, (MAX_NEWINTERACTION_EVENTS - evt_count) * sizeof(int32_t));
}

//-----------------------------------------------------------------------------

#define INTER_VAR_NAME_LENGTH 23

InteractionVariable::InteractionVariable()
    : Type(0)
    , Value(0)
{
}

InteractionVariable::InteractionVariable(const String &name, char type, int val)
    : Name(name)
    , Type(type)
    , Value(val)
{
}

void InteractionVariable::Read(Stream *in)
{
    Name.ReadCount(in, INTER_VAR_NAME_LENGTH);
    Type  = in->ReadInt8();
    Value = in->ReadInt32();
}

void InteractionVariable::Write(Common::Stream *out) const
{
    out->Write(Name, INTER_VAR_NAME_LENGTH);
    out->WriteInt8(Type);
    out->WriteInt32(Value);
}

//-----------------------------------------------------------------------------

InteractionScripts *InteractionScripts::CreateFromStream(Stream *in)
{
    const size_t evt_count = in->ReadInt32();
    if (evt_count > MAX_NEWINTERACTION_EVENTS)
    {
        quit("Can't deserialize interaction scripts: too many events");
        return nullptr;
    }

    InteractionScripts *scripts = new InteractionScripts();
    for (size_t i = 0; i < evt_count; ++i)
    {
        String name = String::FromStream(in);
        scripts->ScriptFuncNames.push_back(name);
    }
    return scripts;
}

} // namespace Common
} // namespace AGS
