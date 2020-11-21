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

#include "ac/global_textbox.h"
#include "ac/common.h"
#include "ac/gamesetupstruct.h"
#include "ac/string.h"
#include "ac/textbox.h"
#include "gui/guimain.h"
#include "gui/guitextbox.h"

using namespace AGS::Common;

extern GameSetupStruct game;

void SetTextBoxFont(int guin,int objn, int fontnum) {

    if ((guin<0) | (guin>=game.numgui)) quit("!SetTextBoxFont: invalid GUI number");
    if ((objn<0) | (objn>=guis[guin].GetControlCount())) quit("!SetTextBoxFont: invalid object number");
    if (guis[guin].GetControlType(objn) != kGUITextBox)
        quit("!SetTextBoxFont: specified control is not a text box");

    GUITextBox *guit = (GUITextBox*)guis[guin].GetControl(objn);
    TextBox_SetFont(guit, fontnum);
}

void GetTextBoxText(int guin, int objn, char*txbuf) {
    VALIDATE_STRING(txbuf);
    if ((guin<0) | (guin>=game.numgui)) quit("!GetTextBoxText: invalid GUI number");
    if ((objn<0) | (objn>=guis[guin].GetControlCount())) quit("!GetTextBoxText: invalid object number");
    if (guis[guin].GetControlType(objn)!=kGUITextBox)
        quit("!GetTextBoxText: specified control is not a text box");

    GUITextBox*guisl=(GUITextBox*)guis[guin].GetControl(objn);
    TextBox_GetText(guisl, txbuf);
}

void SetTextBoxText(int guin, int objn, const char* txbuf) {
    if ((guin<0) | (guin>=game.numgui)) quit("!SetTextBoxText: invalid GUI number");
    if ((objn<0) | (objn>=guis[guin].GetControlCount())) quit("!SetTextBoxText: invalid object number");
    if (guis[guin].GetControlType(objn)!=kGUITextBox)
        quit("!SetTextBoxText: specified control is not a text box");

    GUITextBox*guisl=(GUITextBox*)guis[guin].GetControl(objn);
    TextBox_SetText(guisl, txbuf);
}
