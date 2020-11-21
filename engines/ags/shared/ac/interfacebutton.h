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

#ifndef __AC_INTERFACEBUTTON_H
#define __AC_INTERFACEBUTTON_H

#define MAXBUTTON       20
#define IBFLG_ENABLED   1
#define IBFLG_INVBOX    2
struct InterfaceButton {
    int x, y, pic, overpic, pushpic, leftclick;
    int rightclick; // if inv, then leftclick = wid, rightclick = hit
    int reserved_for_future;
    char flags;
    void set(int xx, int yy, int picc, int overpicc, int actionn);
};

#endif // __AC_INTERFACEBUTTON_H