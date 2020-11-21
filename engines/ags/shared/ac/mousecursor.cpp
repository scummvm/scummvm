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

#include "ac/mousecursor.h"
#include "util/stream.h"

using AGS::Common::Stream;

MouseCursor::MouseCursor() { pic = 2054; hotx = 0; hoty = 0; name[0] = 0; flags = 0; view = -1; }

void MouseCursor::ReadFromFile(Stream *in)
{
    pic = in->ReadInt32();
    hotx = in->ReadInt16();
    hoty = in->ReadInt16();
    view = in->ReadInt16();
    in->Read(name, 10);
    flags = in->ReadInt8();
}

void MouseCursor::WriteToFile(Stream *out)
{
    out->WriteInt32(pic);
    out->WriteInt16(hotx);
    out->WriteInt16(hoty);
    out->WriteInt16(view);
    out->Write(name, 10);
    out->WriteInt8(flags);
}

void MouseCursor::ReadFromSavegame(Stream *in)
{
    pic = in->ReadInt32();
    hotx = in->ReadInt32();
    hoty = in->ReadInt32();
    view = in->ReadInt32();
    flags = in->ReadInt32();
}

void MouseCursor::WriteToSavegame(Stream *out) const
{
    out->WriteInt32(pic);
    out->WriteInt32(hotx);
    out->WriteInt32(hoty);
    out->WriteInt32(view);
    out->WriteInt32(flags);
}
