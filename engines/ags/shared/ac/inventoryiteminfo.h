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

#ifndef __AC_INVENTORYITEMINFO_H
#define __AC_INVENTORYITEMINFO_H

namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

#define IFLG_STARTWITH 1
struct InventoryItemInfo {
    char name[25];
    int  pic;
    int  cursorPic, hotx, hoty;
    int  reserved[5];
    char flags;

    void ReadFromFile(Common::Stream *in);
    void WriteToFile(Common::Stream *out);
    void ReadFromSavegame(Common::Stream *in);
    void WriteToSavegame(Common::Stream *out) const;
};

#endif // __AC_INVENTORYITEMINFO_H