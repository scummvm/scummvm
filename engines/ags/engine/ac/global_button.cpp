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

#include "ac/global_button.h"
#include "ac/common.h"
#include "ac/button.h"
#include "ac/gamesetupstruct.h"
#include "ac/string.h"
#include "gui/guimain.h"
#include "gui/guibutton.h"

using namespace AGS::Common;

extern GameSetupStruct game;

void SetButtonText(int guin,int objn, const char*newtx) {
    VALIDATE_STRING(newtx);
    if ((guin<0) | (guin>=game.numgui))
        quit("!SetButtonText: invalid GUI number");
    if ((objn<0) | (objn>=guis[guin].GetControlCount()))
        quit("!SetButtonText: invalid object number");
    if (guis[guin].GetControlType(objn)!=kGUIButton)
        quit("!SetButtonText: specified control is not a button");

    GUIButton*guil=(GUIButton*)guis[guin].GetControl(objn);
    Button_SetText(guil, newtx);
}


void AnimateButton(int guin, int objn, int view, int loop, int speed, int repeat) {
    if ((guin<0) | (guin>=game.numgui)) quit("!AnimateButton: invalid GUI number");
    if ((objn<0) | (objn>=guis[guin].GetControlCount())) quit("!AnimateButton: invalid object number");
    if (guis[guin].GetControlType(objn)!=kGUIButton)
        quit("!AnimateButton: specified control is not a button");

    Button_Animate((GUIButton*)guis[guin].GetControl(objn), view, loop, speed, repeat);
}


int GetButtonPic(int guin, int objn, int ptype) {
    if ((guin<0) | (guin>=game.numgui)) quit("!GetButtonPic: invalid GUI number");
    if ((objn<0) | (objn>=guis[guin].GetControlCount())) quit("!GetButtonPic: invalid object number");
    if (guis[guin].GetControlType(objn)!=kGUIButton)
        quit("!GetButtonPic: specified control is not a button");
    if ((ptype < 0) | (ptype > 3)) quit("!GetButtonPic: invalid pic type");

    GUIButton*guil=(GUIButton*)guis[guin].GetControl(objn);

    if (ptype == 0) {
        // currently displayed pic
        if (guil->CurrentImage < 0)
            return guil->Image;
        return guil->CurrentImage;
    }
    else if (ptype==1) {
        // nomal pic
        return guil->Image;
    }
    else if (ptype==2) {
        // mouseover pic
        return guil->MouseOverImage;
    }
    else { // pushed pic
        return guil->PushedImage;
    }

    quit("internal error in getbuttonpic");
}

void SetButtonPic(int guin,int objn,int ptype,int slotn) {
    if ((guin<0) | (guin>=game.numgui)) quit("!SetButtonPic: invalid GUI number");
    if ((objn<0) | (objn>=guis[guin].GetControlCount())) quit("!SetButtonPic: invalid object number");
    if (guis[guin].GetControlType(objn)!=kGUIButton)
        quit("!SetButtonPic: specified control is not a button");
    if ((ptype<1) | (ptype>3)) quit("!SetButtonPic: invalid pic type");

    GUIButton*guil=(GUIButton*)guis[guin].GetControl(objn);
    if (ptype==1) {
        Button_SetNormalGraphic(guil, slotn);
    }
    else if (ptype==2) {
        // mouseover pic
        Button_SetMouseOverGraphic(guil, slotn);
    }
    else { // pushed pic
        Button_SetPushedGraphic(guil, slotn);
    }
}
