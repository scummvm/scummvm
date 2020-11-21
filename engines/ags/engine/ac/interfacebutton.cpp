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

#include "ac/interfacebutton.h"

void InterfaceButton::set(int xx, int yy, int picc, int overpicc, int actionn) {
    x = xx; y = yy; pic = picc; overpic = overpicc; leftclick = actionn; pushpic = 0;
    rightclick = 0; flags = IBFLG_ENABLED;
    reserved_for_future = 0;
}