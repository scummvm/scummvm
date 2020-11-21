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

#include "ac/inventoryiteminfo.h"
#include "util/stream.h"
#include "util/string_utils.h"

using namespace AGS::Common;

void InventoryItemInfo::ReadFromFile(Stream *in)
{
    in->Read(name, 25);
    pic = in->ReadInt32();
    cursorPic = in->ReadInt32();
    hotx = in->ReadInt32();
    hoty = in->ReadInt32();
    in->ReadArrayOfInt32(reserved, 5);
    flags = in->ReadInt8();
}

void InventoryItemInfo::WriteToFile(Stream *out)
{
    out->Write(name, 25);
    out->WriteInt32(pic);
    out->WriteInt32(cursorPic);
    out->WriteInt32(hotx);
    out->WriteInt32(hoty);
    out->WriteArrayOfInt32(reserved, 5);
    out->WriteInt8(flags);
}

void InventoryItemInfo::ReadFromSavegame(Stream *in)
{
    StrUtil::ReadString(name, in, 25);
    pic = in->ReadInt32();
    cursorPic = in->ReadInt32();
}

void InventoryItemInfo::WriteToSavegame(Stream *out) const
{
    StrUtil::WriteString(name, out);
    out->WriteInt32(pic);
    out->WriteInt32(cursorPic);
}
