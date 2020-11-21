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

#include "ac/gamestate.h"
#include "ac/global_invwindow.h"
#include "ac/global_translation.h"
#include "ac/properties.h"
#include "gui/guiinv.h"
#include "script/executingscript.h"

extern ExecutingScript*curscript;
extern GameState play;

void sc_invscreen() {
    curscript->queue_action(ePSAInvScreen, 0, "InventoryScreen");
}

void SetInvDimensions(int ww,int hh) {
    play.inv_item_wid = ww;
    play.inv_item_hit = hh;
    play.inv_numdisp = 0;
    // backwards compatibility
    for (int i = 0; i < numguiinv; i++) {
        guiinv[i].ItemWidth = ww;
        guiinv[i].ItemHeight = hh;
        guiinv[i].OnResized();
    }
    guis_need_update = 1;
}
